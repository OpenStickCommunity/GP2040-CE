
#define __BTSTACK_FILE__ "BTAdapter.cpp"

//#include "BTAdapter.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <btstack.h>

#include "BTHelper.h"
#include "descriptor.h"

#include "ble/gatt-service/hids_device.h"

void sendReport();
void adapterLoop(btstack_timer_source_t * ts);
int btstack_main(int argc, const char * argv[]);

/*
########################################################################################################### 
  Device Configuration
###########################################################################################################
*/

#define GAMEPAD_DEVICE_ID 0x2508
#define GAMEPAD_PERIOD_MS 50
#define REPORT_ID 0x01

static const char hid_device_name[] = "GP2040-CE Gamepad";

// This is one of the two SDP record we give to the host.
// Be very carefuly touching this, if you break it you may corrupt your BT cache (see Here Be Dragons)
const hid_sdp_record_t hidParams = {
  GAMEPAD_DEVICE_ID, // hid_device_subclass: gamepad
  33, //* hid_country_code (0 in gamepad, 33 in keyboard)
  1, //* hid_virtual_cable
  0, // hid_remote_wake (added) (0? 1 in keyboard, 0 in xbone dump)
  1, //* hid_reconnect_initiate (0 in gamepad, 1 in keyboard)
  true, // hid_normally_connectable (added) (true in keyboard)
  false, //* hid_boot_device (changed to bool)
  1600, // host_max_latency (added, based on keyboard, xbone doesn't have this)
  3200, // host_min_timeout (added, based on keyboard, xbone doesn't have this)
  3200, // supervision_timeout (added, keyboard and xbone both set it to this)
  xboneReport, //* hid_descriptor
  sizeof(xboneReport), //* hid_descriptor_size
  hid_device_name //* device_name
};

// SDP record buffers
static uint8_t didServiceBuffer[200]; 
static uint8_t hidServiceBuffer[600]; 

static btstack_packet_callback_registration_t hciCallback;
static btstack_timer_source_t loopTimer; // event timer for send loop
static uint16_t hidCID; // HID device ID handle from local HCI
static XBoneData currentData; // Actual Controller data, and report constructor

/*
########################################################################################################### 
  Packet Handler Callbacks
###########################################################################################################
  Call Tree:
    handleHIDPacket: Handles HID device packets to setup the HID ID and serve reports when allowed

    handleHCIPacket: Discconect
*/

void handleHIDPacket(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t packet_size) {
  printf("HID: ");
  printEventType(packet);
  if(hci_event_packet_get_type(packet) != HCI_EVENT_HID_META) return;

  uint8_t status;
  const uint8_t subevent = hci_event_hid_meta_get_subevent_code(packet);
  switch (subevent){
    case HID_SUBEVENT_CONNECTION_OPENED:
      status = hid_subevent_connection_opened_get_status(packet);
      if (status) {
        // outgoing connection failed
        printf("Connection failed, status 0x%x\n", status);
        hidCID = 0;
        return;
      }
      
      hidCID = hid_subevent_connection_opened_get_hid_cid(packet);
      printf("HID Connected\n", status);
      break;
    case HID_SUBEVENT_CONNECTION_CLOSED:
      printf("HID Disconnected\n");
      hidCID = 0;
      break;
    /*case HID_SUBEVENT_CAN_SEND_NOW:  
      if(hidCID!=0){ // Solves crash when disconnecting gamepad on android
        sendReport();
      }
      break;*/
    default:
      printf("Unknown HID subevent: %x\n", subevent);
      break;
  }
}

void handleHCIPacket(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t packet_size) {
  printf("HCI: ");
  printEventType(packet);
  switch (hci_event_packet_get_type(packet)) {
    case HCI_EVENT_DISCONNECTION_COMPLETE: 
      printf("Disconnected\n");
      hidCID = 0;
      // gap_advertisements_enable(1);
      break;
    default: break;
  }
}

/*
########################################################################################################### 
  Setup
###########################################################################################################
*/

void setupClassic() {
  l2cap_init();
  sdp_init();

  // setup discoverability via non-BLE Bluetooth, this is SLOW and should be replaced with BLE handoff
  gap_set_local_name(hid_device_name);
  gap_set_class_of_device(GAMEPAD_DEVICE_ID);
  gap_set_default_link_policy_settings( LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE );
  gap_set_allow_role_switch(true);
  gap_discoverable_control(1);

  // setup SDP HID record
  memset(hidServiceBuffer, 0, sizeof(hidServiceBuffer));
  hid_create_sdp_record(hidServiceBuffer, sdp_create_service_record_handle(), &hidParams);
  sdp_register_service(hidServiceBuffer);
  printf("HID service record size: %u\n", de_get_len(hidServiceBuffer));

  // xbone SDP PnP record, otherwise this appears as a generic HID, and nothing can use it
  device_id_create_sdp_record(didServiceBuffer, sdp_create_service_record_handle(), 0x0002, 0x045e, 0x02e0, 0x0903);
  sdp_register_service(didServiceBuffer);
  printf("DID service record size: %u\n", de_get_len(didServiceBuffer));

  // setup HID Device service
  // the report isn't actually used afaik, Windows seems to get all of this data from the SDP records
  // but it also sets up the L2CAP HID channel descriptors
  hid_device_init(0, sizeof(xboneReport), xboneReport);

  // callback for disconnect only, has way more use with LE
  hciCallback.callback = &handleHCIPacket;
  hci_add_event_handler(&hciCallback);

  // callback for HID setup, we don't ask before sending packets so it doesn't get touched in the loop
  hid_device_register_packet_handler(&handleHIDPacket);
}

/*
########################################################################################################### 
  Main logic
###########################################################################################################
*/

int btstack_main(int argc, const char * argv[]){
  (void)argc; (void)argv;

  // setup the L2CAP channel framework and footprint
  setupClassic();

  // actually turn on Bluetooth
  hci_power_control(HCI_POWER_ON);

  // start our send loop
  loopTimer.process = &adapterLoop;
  btstack_run_loop_set_timer(&loopTimer, GAMEPAD_PERIOD_MS);
  btstack_run_loop_add_timer(&loopTimer);
  return 0;
}

void sendReport() {
  printf("...\n");

  uint8_t report[17]; 
  currentData.makeReportOne(report);

  // we could send hid_device_request_can_send_now_event and 
  // call sendReport from HID_SUBEVENT_CAN_SEND_NOW, but we don't actually have to
  // may cause crashes on disconnect, currently reconnecting doesn't work regardless
  hid_device_send_interrupt_message(hidCID, &report[0], sizeof(report));
}

void adapterLoop(btstack_timer_source_t * ts){
  if (hidCID != 0){
    sendReport();
  }

  // do it again
  btstack_run_loop_set_timer(ts, GAMEPAD_PERIOD_MS);
  btstack_run_loop_add_timer(ts);
}

// API for main loop to update the device state
void updateBluetoothInputs(uint16_t leftX, uint16_t leftY, uint16_t rightX, uint16_t rightY, uint16_t buttons){
  currentData.X = leftX;
  currentData.Y = leftY;
  currentData.Rx = rightX;
  currentData.Ry = rightY;
  currentData.Buttons = buttons;
}