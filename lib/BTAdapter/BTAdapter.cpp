
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
static const char hid_sevice_name[] = "GP2040-CE Gamepad";


const hid_sdp_record_t hidParams = {
  GAMEPAD_DEVICE_ID, // hid_device_subclass: gamepad
  33, //* hid_country_code (0 in gamepad, 33 in keyboard)
  1, //* hid_virtual_cable
  0, // hid_remote_wake (added) (0? its 1 in keyboard)
  1, //* hid_reconnect_initiate (0 in gamepad, 1 in keyboard)
  true, // hid_normally_connectable (added) (true in keyboard)
  false, //* hid_boot_device (changed to bool)
  1600, // host_max_latency (added, based on keyboard)
  3200, // host_min_timeout (added, based on keyboard)
  3200, // supervision_timeout (added, based on keyboard)
  xboneReport, //* hid_descriptor
  sizeof(xboneReport), //* hid_descriptor size
  hid_device_name //* device_name
};

static uint8_t didServiceBuffer[600]; 
static uint8_t hidServiceBuffer[600]; 
static btstack_packet_callback_registration_t hciCallback;
static btstack_timer_source_t loopTimer;
static uint16_t hidCID;
static hci_con_handle_t leConnectionHandle = HCI_CON_HANDLE_INVALID;



static XBoneData currentData;

/*
########################################################################################################### 
  Packet Handler Callbacks
###########################################################################################################
  Call Tree:
    handleHIDPacket: Handles HID device packets to setup the HID ID and serve reports when allowed

    handleHCIPacket:
      LE Discconect
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
    case HID_SUBEVENT_CAN_SEND_NOW:  
      if(hidCID!=0){ //Solves crash when disconnecting gamepad on android
        sendReport();
      }
      break;
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
      leConnectionHandle = HCI_CON_HANDLE_INVALID;
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

void setupCallbacks() {
  hciCallback.callback = &handleHCIPacket;
  hci_add_event_handler(&hciCallback);

  hid_device_register_packet_handler(&handleHIDPacket);
}

void setupClassic() {
  sdp_init();

  gap_set_local_name(hid_sevice_name);
  //gap_ssp_set_io_capability(SSP_IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
  gap_set_class_of_device(GAMEPAD_DEVICE_ID);
  gap_set_default_link_policy_settings( LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE );
  gap_set_allow_role_switch(true);
  gap_discoverable_control(1);

  memset(hidServiceBuffer, 0, sizeof(hidServiceBuffer));
  hid_create_sdp_record(hidServiceBuffer, sdp_create_service_record_handle(), &hidParams);
  sdp_register_service(hidServiceBuffer);
  printf("HID service record size: %u\n", de_get_len(hidServiceBuffer));

#if 1 // forge xbone PnP
  device_id_create_sdp_record(didServiceBuffer, sdp_create_service_record_handle(), 0x0002, 0x045e, 0x02e0, 0x0903);
  sdp_register_service(didServiceBuffer);
  printf("DID service record size: %u\n", de_get_len(didServiceBuffer));
#endif

  // setup HID Device service
  hid_device_init(0, sizeof(xboneReport), xboneReport);
}

/*
########################################################################################################### 
  Main logic
###########################################################################################################
*/

int btstack_main(int argc, const char * argv[]){
  (void)argc; (void)argv;

  l2cap_init();
  setupClassic();
  setupCallbacks();

  hci_power_control(HCI_POWER_ON);

  loopTimer.process = &adapterLoop;
  btstack_run_loop_set_timer(&loopTimer, GAMEPAD_PERIOD_MS);
  btstack_run_loop_add_timer(&loopTimer);
  return 0;
}

void fakeUpdateData() {
  currentData.X += 1000;
  if (currentData.X > 60000){
    currentData.X = 0;
  }
  currentData.Y = -currentData.X;
  currentData.Rx = currentData.X;
  currentData.Ry = -currentData.X;

  currentData.Buttons = currentData.Buttons ^ 1;
}

void sendReport() {
  printf("...\n");

  fakeUpdateData();
  uint8_t report[17]; 
  currentData.makeReportOne(report);

  hid_device_send_interrupt_message(hidCID, &report[0], sizeof(report));
}

void adapterLoop(btstack_timer_source_t * ts){
  if (hidCID != 0){
    sendReport();
    //hid_device_request_can_send_now_event(hidCID);
  }

  btstack_run_loop_set_timer(ts, GAMEPAD_PERIOD_MS);
  btstack_run_loop_add_timer(ts);
}