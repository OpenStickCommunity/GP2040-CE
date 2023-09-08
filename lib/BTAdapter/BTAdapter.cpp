
#define __BTSTACK_FILE__ "BTAdapter.cpp"

//#include "BTAdapter.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <btstack.h>

#include "BTHelper.h"

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
#define GAMEPAD_PERIOD_MS 5
#define REPORT_ID 0x01

static const char hid_device_name[] = "GP2040-CE Gamepad";
static const char hid_sevice_name[] = "GP2040-CE Gamepad";

const uint8_t gamepadReport[] = {
  // Preamble
  0x05, 0x01,                   //  USAGE_PAGE (Generic Desktop)
  0x09, 0x05,                   //  USAGE (Game Pad)
  0xa1, 0x01,                   //  COLLECTION (Application)

  // Joystick 1
  0xA1, 0x00,                   //    COLLECTION (Physical)
  0x09, 0x30,                   //      USAGE (X)
  0x09, 0x31,                   //      USAGE (Y)
  0x15, 0x00,                   //      LOGICAL_MINIMUM (0)
  0x27, 0xFF, 0xFF, 0x00, 0x00, //      LOGICAL_MAXIMUM (65535)
  0x95, 0x02,                   //      REPORT_COUNT (2)
  0x75, 0x10,                   //      REPORT_SIZE (16)
  0x81, 0x02,                   //      INPUT (Data, Var, Abs)
  0xc0,                         //    END_COLLECTION

  // Joystick 2
  0xA1, 0x00,                   //    COLLECTION (Physical)
  0x09, 0x33,                   //      USAGE (rX)
  0x09, 0x34,                   //      USAGE (rY)
  0x15, 0x00,                   //      LOGICAL_MINIMUM (0)
  0x27, 0xFF, 0xFF, 0x00, 0x00, //      LOGICAL_MAXIMUM (65535)
  0x95, 0x02,                   //      REPORT_COUNT (2)
  0x75, 0x10,                   //      REPORT_SIZE (16)
  0x81, 0x02,                   //      INPUT (Data, Var, Abs)
  0xc0,                         //    END_COLLECTION
  
  // Buttons
  0x05, 0x09,                   //    USAGE_PAGE (Button)
  0x19, 0x01,                   //    USAGE_MINIMUM (Button 1)
  0x29, 0x10,                   //    USAGE_MAXIMUM (Button 16)
  0x15, 0x00,                   //    LOGICAL_MINIMUM (0)
  0x25, 0x01,                   //    LOGICAL_MAXIMUM (1)
  0x95, 0x10,                   //    REPORT_COUNT (16)
  0x75, 0x01,                   //    REPORT_SIZE (1)
  0x81, 0x02,                   //    INPUT (Data, Var, Abs)    
   
  // Closing
  0xc0                          //  END_COLLECTION
};

const uint8_t advertisedData[] = {
  // Flags general discoverable
  0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x6,
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
  0, //* hid_reconnect_initiate (0 in gamepad, 1 in keyboard)
  true, // hid_normally_connectable (added) (true in keyboard)
  false, //* hid_boot_device (changed to bool)
  1600, // host_max_latency (added, based on keyboard)
  3200, // host_min_timeout (added, based on keyboard)
  3200, // supervision_timeout (added, based on keyboard)
  gamepadReport, //* hid_descriptor
  sizeof(gamepadReport), //* hid_descriptor size
  hid_device_name //* device_name
};

static uint8_t didServiceBuffer[600]; 
static uint8_t hidServiceBuffer[600]; 
static btstack_packet_callback_registration_t hciCallback;
static btstack_packet_callback_registration_t smCallback;
static btstack_timer_source_t loopTimer;
static uint16_t hidCID;
static hci_con_handle_t leConnectionHandle = HCI_CON_HANDLE_INVALID;

struct GamePadData {
  uint16_t lx, ly, rx, ry;
  uint8_t but_a, but_b;
};

static GamePadData currentData;

/*
########################################################################################################### 
  Packet Handler Callbacks
###########################################################################################################
  Call Tree:
    handleHIDPacket: Handles HID device packets to setup the HID ID and serve reports when allowed

    handleHCIPacket:
      LE Discconect
      handleLeMeta: Handles BLE connection state setup and migrates to Classic
    
    handleSMPacket: Serves authentication confirmation on BLE connection setup
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
      printf("Unknown HID subevent: %x\n", subevent);
      break;
  }
}

void handleLeMeta(uint8_t * packet) {
  switch(hci_event_le_meta_get_subevent_code(packet)){
    case HCI_SUBEVENT_LE_CONNECTION_COMPLETE: 
    printf("LE Connected\n");
      leConnectionHandle = hci_subevent_le_connection_complete_get_connection_handle(packet);
      // handshake onto other mode?
       
      // gap_advertisements_enable(0); 
      break;
    default: break;
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
    case HCI_EVENT_LE_META: handleLeMeta(packet); break;
    default: break;
  }
}

void handleSMPacket(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t packet_size) {
  printf("SM:  ");
  printEventType(packet);

  switch (hci_event_packet_get_type(packet)) {
    case SM_EVENT_JUST_WORKS_REQUEST:
      printf("Just Works requested\n");
      sm_just_works_confirm(sm_event_just_works_request_get_handle(packet)); 
      break;
    default:
      break;
  }
}

/*
########################################################################################################### 
  Setup
###########################################################################################################
*/

// LE is used to advertise the device, the device is automatically migrated to classic on connection
void setupLE() {
  sm_init();
  sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
  sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION );

  uint16_t adv_int_min = 0x0030;
  uint16_t adv_int_max = 0x0030;
  uint8_t adv_type = 0;
  bd_addr_t null_addr;
  memset(null_addr, 0, 6);
  gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
  gap_advertisements_set_data(sizeof(advertisedData), (uint8_t*)advertisedData);
  gap_advertisements_enable(1);
}

void setupCallbacks() {
  hciCallback.callback = &handleHCIPacket;
  hci_add_event_handler(&hciCallback);

  smCallback.callback = &handleSMPacket;
  sm_add_event_handler(&smCallback);

  hid_device_register_packet_handler(&handleHIDPacket);
}

void setupClassic() {
  sdp_init();

  gap_set_local_name(hid_sevice_name);
  gap_ssp_set_io_capability(SSP_IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
  gap_set_class_of_device(GAMEPAD_DEVICE_ID);
  gap_set_default_link_policy_settings( LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE );
  gap_set_allow_role_switch(true);
  gap_discoverable_control(1);

  memset(hidServiceBuffer, 0, sizeof(hidServiceBuffer));
  hid_create_sdp_record(hidServiceBuffer, 0x10001, &hidParams);
  sdp_register_service(hidServiceBuffer);
  printf("HID service record size: %u\n", de_get_len(hidServiceBuffer));

#if 0
  device_id_create_sdp_record(didServiceBuffer, 0x10003, DEVICE_ID_VENDOR_ID_SOURCE_BLUETOOTH, BLUETOOTH_COMPANY_ID_BLUEKITCHEN_GMBH, 1, 1);
  sdp_register_service(didServiceBuffer);
  printf("DID service record size: %u\n", de_get_len(didServiceBuffer));
#endif

  // setup HID Device service
  hid_device_init(0, sizeof(gamepadReport), gamepadReport);
}

/*
########################################################################################################### 
  Main logic
###########################################################################################################
*/

int btstack_main(int argc, const char * argv[]){
  (void)argc; (void)argv;

  currentData.lx = 0;
  currentData.ly = 0;
  currentData.rx = 0;
  currentData.ry = 0;
  currentData.but_a = 1;
  currentData.but_b = 1;
  //currentData.lt = 0;
  //currentData.rt = 0;

  l2cap_init();
  setupLE();
  setupClassic();
  setupCallbacks();

  hci_power_control(HCI_POWER_ON);

  loopTimer.process = &adapterLoop;
  btstack_run_loop_set_timer(&loopTimer, GAMEPAD_PERIOD_MS);
  btstack_run_loop_add_timer(&loopTimer);
  return 0;
}

void updateData(uint8_t but_a, uint8_t but_b, uint8_t lt, uint8_t rt, uint16_t lx, uint16_t rx) {
  currentData.but_a = but_a;
  currentData.but_b = but_b;
  //currentData.lt = lt;
  //currentData.rt = rt;
  currentData.lx = lx;
  //currentData.ly = ly;
  currentData.rx = rx;
  //currentData.ry = ry;
}

void updateJoystick() {
  currentData.lx += 100;
  if (currentData.lx > 60000){
    currentData.lx = 0;
  }
  currentData.ly = -currentData.lx;
  currentData.rx = currentData.lx;
  currentData.ry = -currentData.lx;

  currentData.but_a = currentData.but_a << 1;
  currentData.but_b = currentData.but_b >> 1;

  if( currentData.but_a == 0 ) currentData.but_a = 1;
  if( currentData.but_b == 0 ) currentData.but_b = 128;
}

void sendReport() {
  updateJoystick();
  uint8_t report[] = {
    0xa1, 
    currentData.lx & 0xff, currentData.lx >> 8, 
    currentData.ly & 0xff, currentData.ly >> 8,
    currentData.rx & 0xff, currentData.rx >> 8, 
    currentData.ry & 0xff, currentData.ry >> 8,
    currentData.but_a, currentData.but_b,
    0
  };
  printf("...\n");


  hid_device_send_interrupt_message(hidCID, &report[0], sizeof(report));
}

void adapterLoop(btstack_timer_source_t * ts){
  if (hidCID != 0)
    hid_device_request_can_send_now_event(hidCID);

  btstack_run_loop_set_timer(ts, GAMEPAD_PERIOD_MS);
  btstack_run_loop_add_timer(ts);
}