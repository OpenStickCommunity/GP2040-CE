
#define __BTSTACK_FILE__ "BTAdapter.cpp"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <btstack.h>

#include "BTHelper.h"

#include "ble/gatt-service/hids_device.h"



#define GAMEPAD_DEVICE_ID 0x2508
#define GAMEPAD_PERIOD_MS 5

static const char hid_device_name[] = "GP2040-CE Gamepad";

const uint8_t gamepadReport[] = {
  0x05, 0x01,                   //  USAGE_PAGE (Generic Desktop)
  0x09, 0x05,                   //  USAGE (Game Pad)
  0xa1, 0x01,                   //  COLLECTION (Application)
  0x85, 0x01,                   //    REPORT_ID (1)
  0xa1, 0x02,                   //    COLLECTION (Logical)

  0x05, 0x01,                   //      USAGE_PAGE (Generic Desktop)
  0x09, 0x30,                   //      USAGE (X)
  0x09, 0x31,                   //      USAGE (Y)
  0x15, 0x81,                   //      LOGICAL_MINIMUM (-127)
  0x25, 0x7f,                   //      LOGICAL_MAXIMUM (127)
  0x95, 0x02,                   //      REPORT_COUNT (2)
  0x75, 0x08,                   //      REPORT_SIZE (8)
  0x81, 0x02,                   //      INPUT (Data, Var, Abs)

  0x05, 0x09,                   //      USAGE_PAGE (Button)
  0x19, 0x01,                   //      USAGE_MINIMUM (Button 1)
  0x29, 0x08,                   //      USAGE_MAXIMUM (Button 8)
  0x15, 0x00,                   //      LOGICAL_MINIMUM (0)
  0x25, 0x01,                   //      LOGICAL_MAXIMUM (1)
  0x95, 0x08,                   //      REPORT_COUNT (8)
  0x75, 0x01,                   //      REPORT_SIZE (1)
  0x81, 0x02,                   //      INPUT (Data, Var, Abs)      

  0xc0,                         //    END_COLLECTION
  0xc0                          //  END_COLLECTION
};

const uint8_t advertisedData[] = {
    // Flags general discoverable
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x2,
    // Name
    0x0f, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'H', 'I', 'D', ' ', 'C', 'o', 'n', 't', 'r', 'o', 'l', 'l', 'e', 'r',
    // 16-bit Service UUIDs
    0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE & 0xff, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE >> 8,
    // Appearance HID - Gamepad
    0x03, BLUETOOTH_DATA_TYPE_APPEARANCE, 0xC4, 0x03,
};

const hid_sdp_record_t hidParams = {
  GAMEPAD_DEVICE_ID, // hid_device_subclass: gamepad
  33, //* hid_country_code (0 in gamepad, 33 in keyboard)
  0, //* hid_virtual_cable
  1, // hid_remote_wake (added) (0? its 1 in keyboard)
  1, //* hid_reconnect_initiate (0 in gamepad, 1 in keyboard)
  true, // hid_normally_connectable (added) (true in keyboard)
  false, //* hid_boot_device (changed to bool)
  1600, // host_max_latency (added, based on keyboard)
  3200, // host_min_timeout (added, based on keyboard)
  3200, // supervision_timeout (added, based on keyboard)
  gamepadReport, //* hid_descriptor
  sizeof(gamepadReport), //* hid_descriptor size
  hid_device_name //* device_name
};

static uint8_t hidServiceBuffer[300]; 
static btstack_packet_callback_registration_t hciCallback;
static btstack_timer_source_t loopTimer;
static uint16_t hidCID;

static void sendReport(){
  uint8_t report[] = {0xa1, 0x30,0, 0,0};  
  hid_device_send_interrupt_message(hidCID, &report[0], sizeof(report));
}

static void adapterLoop(btstack_timer_source_t * ts){
  if (hidCID != 0)
    hid_device_request_can_send_now_event(hidCID);

  btstack_run_loop_set_timer(ts, GAMEPAD_PERIOD_MS);
  btstack_run_loop_add_timer(ts);
}

void handleHID(uint8_t *packet){
  uint8_t status;
  switch (hci_event_hid_meta_get_subevent_code(packet)){
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
      hid_device_request_can_send_now_event(hidCID);
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
      break;
  }
}

void handleLeMeta(uint8_t *packet) {
  switch(hci_event_le_meta_get_subevent_code(packet)){
    case HCI_SUBEVENT_LE_CONNECTION_COMPLETE: 
      // handshake onto other mode?
       
      //gap_advertisements_enable(0); 
      break;
    default: break;
  }
}

static void handlePacket(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t packet_size){
  (void)channel; (void)packet_size;

  if (packet_type != HCI_EVENT_PACKET) return;

  printEventType(packet);
  
  switch (hci_event_packet_get_type(packet)) {
    case HCI_EVENT_HID_META: handleHID(packet); break;
    case HCI_EVENT_LE_META: handleLeMeta(packet); break;
    default: break;
  }
}


static void setupBT() {
  gap_set_local_name(hid_device_name);
  //gap_ssp_set_io_capability(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
  gap_set_class_of_device(GAMEPAD_DEVICE_ID);
  gap_set_default_link_policy_settings( LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE );
  gap_set_allow_role_switch(true);
  gap_discoverable_control(1);

  l2cap_init();

  sdp_init();

  memset(hidServiceBuffer, 0, sizeof(hidServiceBuffer));
  hid_create_sdp_record(hidServiceBuffer, 0x10001, &hidParams);
  sdp_register_service(hidServiceBuffer);
  printf("HID service record size: %u\n", de_get_len(hidServiceBuffer));

  // setup HID Device service
  hid_device_init(0, sizeof(gamepadReport), gamepadReport);

  hid_device_register_packet_handler(&handlePacket);

  hciCallback.callback = &handlePacket;
  hci_add_event_handler(&hciCallback);

  uint16_t adv_int_min = 0x0030;
  uint16_t adv_int_max = 0x0030;
  uint8_t adv_type = 0;
  bd_addr_t null_addr;
  memset(null_addr, 0, 6);
  gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
  gap_advertisements_set_data(sizeof(advertisedData), (uint8_t*)advertisedData);
  gap_advertisements_enable(1);
}


int btstack_main(int argc, const char * argv[]);
int btstack_main(int argc, const char * argv[]){
  (void)argc; (void)argv;

  setupBT();
  hci_power_control(HCI_POWER_ON);

  loopTimer.process = &adapterLoop;
  btstack_run_loop_set_timer(&loopTimer, GAMEPAD_PERIOD_MS);
  btstack_run_loop_add_timer(&loopTimer);
  return 0;
}