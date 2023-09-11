
#define __BTSTACK_FILE__ "BTAdapter.cpp"

//stdlib
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

//btstack
#include <btstack.h>
#include "GATT.h"

//private
#include "BTHelper.h"
#include "descriptor.h"
#include <vector>
#include <tuple>

/*
########################################################################################################### 
  Packet Handler Callbacks
###########################################################################################################
  Call Tree:
    handleHIDPacket: Handles HID device packets to setup the HID ID and serve reports when allowed

    handleHCIPacket: Discconect
*/

static uint16_t hidCID = 0; // HID device ID handle from local HCI
static uint8_t TDS_Data[] = {0x1,0x1}; // TDS opcode and org code, maybe needs 18 octet uuid service procedure section?

void handleHIDPacket(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t packet_size) {
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
    default:
      printf("Unknown HID subevent: %x\n", subevent);
      break;
  }
}

void handleHCIPacket(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t packet_size) {
  printEventType(packet);
  switch (hci_event_packet_get_type(packet)) {
    case HCI_EVENT_DISCONNECTION_COMPLETE: 
      printf("Disconnected, reason: %x\n", hci_event_disconnection_complete_get_reason(packet));
      hidCID = 0;
      // gap_advertisements_enable(1);
      break;
    case HCI_EVENT_COMMAND_COMPLETE:
      printf("Command: %x %x\n", hci_event_command_complete_get_command_opcode(packet), hci_event_command_complete_get_return_parameters(packet));
      break;
    case HCI_EVENT_ENCRYPTION_CHANGE:
      printf("Encryption: %x %x\n", hci_event_encryption_change_get_encryption_enabled(packet), hci_event_encryption_change_get_status(packet));
      break;
    case HCI_EVENT_COMMAND_STATUS: 
      printf("Status: %x\n", hci_event_command_status_get_status(packet));
      break;
    case HCI_EVENT_LINK_KEY_REQUEST:
      {
        bd_addr_t addr;
        hci_event_link_key_request_get_bd_addr(packet, addr);
        hci_connection_t * conn = hci_connection_for_bd_addr_and_type(addr, BD_ADDR_TYPE_ACL);

        printf("Link Key: ");
        if(conn->link_key_type == INVALID_LINK_KEY){
          printf("INVALID\n");
        } else {
          for(const auto& c : conn->link_key) printf("%x", c);
          printf("\n");
        }
        
      }
      
    default: break;
  }
}

void handleSMPacket(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t packet_size) {
  printEventType(packet);
  
  switch (hci_event_packet_get_type(packet)) {
    case SM_EVENT_JUST_WORKS_REQUEST: 
      printf("Just Works requested\n");
      sm_just_works_confirm(sm_event_just_works_request_get_handle(packet)); 
      gap_discoverable_control(1);
      break;
    default: break;
  }
}

// dynamic GATT entry read/write callbacks, 
// we're supposed to get a write call to the TDS Control Point 
// when the host is ready to connect... but we don't
uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size){
  if (att_handle == ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TDS_CONTROL_POINT_01_VALUE_HANDLE){
    printf("Read TDS CONTROL: %x %x\n", offset, buffer_size);
    return att_read_callback_handle_blob(TDS_Data, (uint16_t)sizeof(TDS_Data), offset, buffer, buffer_size);
  }
  else if(att_handle == ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TDS_CONTROL_POINT_01_CLIENT_CONFIGURATION_HANDLE){
    printf("Read TDS CONTROL CONFIG: %x %x\n", offset, buffer_size);
    return att_read_callback_handle_blob(TDS_Data, (uint16_t)sizeof(TDS_Data), offset, buffer, buffer_size);
  }
  printf("Read Unknown: %x\n", att_handle);
  return 0;
}

static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){
  if (att_handle == ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TDS_CONTROL_POINT_01_VALUE_HANDLE){
    printf("Write TDS CONTROL: %x %x %x\n", offset, buffer_size, transaction_mode);
    return 0;
  }
  else if(att_handle == ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TDS_CONTROL_POINT_01_CLIENT_CONFIGURATION_HANDLE){
    printf("Write TDS CONTROL CONFIG: %x %x %x\n", offset, buffer_size, transaction_mode);
    return 0;
  }
  printf("Write Unknown: %x\n", att_handle);
  return 0;
}
/*
########################################################################################################### 
  Setup
###########################################################################################################
*/

// SDP record buffers
static uint8_t didServiceBuffer[200]; 
static uint8_t hidServiceBuffer[600]; 
static btstack_packet_callback_registration_t hciCallback;
static btstack_packet_callback_registration_t smCallback;

void setupClassic() {
  l2cap_init();
  sdp_init();
  sm_init();
  sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
  sm_set_authentication_requirements(SM_AUTHREQ_BONDING | SM_AUTHREQ_SECURE_CONNECTION );

  // setup discoverability via non-BLE Bluetooth, this is SLOW and should be replaced with BLE handoff
  gap_set_local_name(hid_device_name);
  gap_set_class_of_device(GAMEPAD_DEVICE_ID);
  gap_set_default_link_policy_settings( LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE );
  gap_set_allow_role_switch(true);

  gap_discoverable_control(0);

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

constexpr uint8_t TDS_PROVIDER = 0x2;
constexpr uint8_t TDS_STATE_ON = 0x8;
constexpr uint8_t TDS_STATE_UNV = 0x10;
constexpr uint8_t TDS_DATA_SIZE = 0x0;

constexpr auto buildHandoffAdvert(){
  std::array<uint8_t, 31> result = {};
  int offset = 0;
  // Flags: (general discoverable)
  BUILD_DAT_ROW(BLUETOOTH_DATA_TYPE_FLAGS, 0x2);
  // Name:
  BUILD_STR_ROW(BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, "GP-Gamepad");
  // 16-bit Service UUIDs
  BUILD_DAT_ROW(BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, ORG_BLUETOOTH_SERVICE_TRANSPORT_DISCOVERY & 0xff, ORG_BLUETOOTH_SERVICE_TRANSPORT_DISCOVERY >> 8);
  // Appearance HID - Gamepad
  //BUILD_DAT_ROW(appearance, BLUETOOTH_DATA_TYPE_APPEARANCE, 0xC4, 0x03)
  // TDS handoff data
  BUILD_DAT_ROW(BLUETOOTH_DATA_TYPE_TRANSPORT_DISCOVERY_DATA, 0x1, (TDS_PROVIDER | TDS_STATE_ON), 0x4, 
    0x3, 0x1, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE & 0xff, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE >> 8, // 16bit UUID, required
    //0x4, 0x8, 0x08, 0x25, 0x00
    )

  return std::make_tuple(result,offset);
}

constexpr auto cleanHandoffAdvert(){
  constexpr auto t = buildHandoffAdvert();
  constexpr auto data = std::get<0>(t);
  constexpr auto size = std::get<1>(t);

  std::array<uint8_t, size> result = {};
  for(int i = 0; i < size; ++i) result[i] = data[i];
  return result;
}

constexpr auto advData = cleanHandoffAdvert();

// This sets up the BLE Transport Discovery Service advertisement for the BR/EDR classic connection.
// This allows device pairing much more quickly and reliably than the somewhat deprecated classic SDP discovery 
void setupHandoff(){
  // TODO: setup att/gatt data
  att_server_init(profile_data, att_read_callback, att_write_callback);

  // setup LE advertisements
  constexpr uint16_t adv_int_min = 0x0030;
  constexpr uint16_t adv_int_max = 0x0030;
  constexpr uint8_t event_type = 0x00; // connectable, undirected
  constexpr uint8_t address_type = 0x0; // public address
  bd_addr_t null_addr; // will be filled in automatically
  memset(null_addr, 0, 6);
  gap_advertisements_set_params(adv_int_min, adv_int_max, event_type, address_type, null_addr, 0x07, 0x00);
  gap_advertisements_set_data(advData.size(), (uint8_t*)advData.data());
  gap_advertisements_enable(1);

  smCallback.callback = &handleSMPacket;
  sm_add_event_handler(&smCallback);
}

/*
########################################################################################################### 
  Main logic
###########################################################################################################
*/

// this limits the rate we send at, useful to raise it when analyzing traffic
// theoretically setting this too low could cause a mysterious buffer to overflow
#define GAMEPAD_PERIOD_MS 2
static btstack_timer_source_t loopTimer; // event timer for send loop
static uint8_t reportBuffer[17];

extern void btTickGamepad(uint8_t (&reportBuffer)[17]);

void sendReport() {
  // we could send hid_device_request_can_send_now_event and 
  // call sendReport from HID_SUBEVENT_CAN_SEND_NOW, but we don't actually have to
  // may cause crashes on disconnect, currently reconnecting doesn't work regardless
  hid_device_send_interrupt_message(hidCID, &reportBuffer[0], sizeof(reportBuffer));
}

void adapterLoop(btstack_timer_source_t * ts){
  btTickGamepad(reportBuffer);

  if (hidCID != 0){
    sendReport();
  }

  // do it again
  btstack_run_loop_set_timer(ts, GAMEPAD_PERIOD_MS);
  btstack_run_loop_add_timer(ts);
}

int btstack_main(int argc, const char * argv[]);
int btstack_main(int argc, const char * argv[]){
  (void)argc; (void)argv;

  // setup the L2CAP channel framework and footprint
  setupClassic();
  setupHandoff();

  // actually turn on Bluetooth
  hci_power_control(HCI_POWER_ON);

  // start our send loop
  loopTimer.process = &adapterLoop;
  btstack_run_loop_set_timer(&loopTimer, GAMEPAD_PERIOD_MS);
  btstack_run_loop_add_timer(&loopTimer);
  return 0;
}