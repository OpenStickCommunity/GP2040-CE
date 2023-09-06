
#define __BTSTACK_FILE__ "BTAdapter.cpp"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <btstack.h>

static const char hid_device_name[] = "Bluetooth Gamepad";
static const char service_name[] = "Wireless Gamepad";

// no idea how big this needs to be
// 250 for gamepad with reportMap[44] (but outdated/missing options)
// 300 for keyboard with reportMap[65]
static uint8_t hid_service_buffer[300]; 

#define GAMEPAD_DEVICE_ID 0x2508


// static uint8_t device_id_sdp_service_buffer[100]; // unused in gamepad?
static btstack_packet_callback_registration_t hci_event_callback_registration;
static uint16_t hid_cid;

const uint8_t reportMap[] = {
  0x05, 0x01,                   // USAGE_PAGE (Generic Desktop)
  0x09, 0x05,                   // USAGE (Game Pad)
  0xa1, 0x01,                   // COLLECTION (Application)
  0xa1, 0x02,                   //    COLLECTION (Logical)
  0x85, 0x30,                    //      REPORT_ID (48)
  
  0x75, 0x08,                   //      REPORT_SIZE (8)
  0x95, 0x02,                   //      REPORT_COUNT (2)
  0x05, 0x01,                   //      USAGE_PAGE (Generic Desktop)
  0x09, 0x30,                   //      USAGE (X)
  0x09, 0x31,                   //      USAGE (Y)
  0x15, 0x81,                   //      LOGICAL_MINIMUM (-127)
  0x25, 0x7f,                   //      LOGICAL_MAXIMUM (127)
  0x81, 0x02,                   //      INPUT (Data, Var, Abs)
  
  0x75, 0x01,                   //      REPORT_SIZE (1)
  0x95, 0x08,                   //      REPORT_COUNT (8)
  0x15, 0x00,                   //      LOGICAL_MINIMUM (0)
  0x25, 0x01,                   //      LOGICAL_MAXIMUM (1)
  0x05, 0x09,                   //      USAGE_PAGE (Button)
  0x19, 0x01,                   //      USAGE_MINIMUM (Button 1)
  0x29, 0x08,                   //      USAGE_MAXIMUM (Button 8)
  0x81, 0x02,                   //      INPUT (Data, Var, Abs)      
  0xc0,                         //   END_COLLECTION
  0xc0                          // END_COLLECTION
};

// * implies it is adapted from gamepad example
const hid_sdp_record_t hid_params = {
  GAMEPAD_DEVICE_ID, // hid_device_subclass: gamepad
  0, //* hid_country_code (0 in gamepad, 33 in keyboard)
  0, //* hid_virtual_cable
  0, // hid_remote_wake (added) (0? its 1 in keyboard)
  0, //* hid_reconnect_initiate (0 in gamepad, 1 in keyboard)
  false, // hid_normally_connectable (added) (true in keyboard)
  false, //* hid_boot_device (changed to bool)
  1600, // host_max_latency (added, based on keyboard)
  3200, // host_min_timeout (added, based on keyboard)
  3200, // supervision_timeout (added, based on keyboard)
  reportMap, //* hid_descriptor
  sizeof(reportMap), //* hid_descriptor size
  service_name //* device_name
};

typedef struct
{
  uint8_t left_x;
  uint8_t left_y;
  uint8_t buttons;
}
gamepad_report_t;

static gamepad_report_t joystick;

static void send_report_joystick(){
  //Dummy report, presses two buttons and moves the stick to the down-right corner
  joystick.left_x = 120;
  joystick.left_y = 120;
  joystick.buttons |= 0x1;
  joystick.buttons |= 0x4;
  uint8_t report[] = {0xa1, 0x30,joystick.left_x, joystick.left_y,joystick.buttons};  
  //send report
  hid_device_send_interrupt_message(hid_cid, &report[0], sizeof(report));
}

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t packet_size){
  UNUSED(channel);
  UNUSED(packet_size);
  uint8_t status;
  if(packet_type == HCI_EVENT_PACKET) {
    switch (packet[0]){
      case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
        break;

      case HCI_EVENT_USER_CONFIRMATION_REQUEST:
        // ssp: inform about user confirmation request
        //log_info("SSP User Confirmation Request with numeric value '%06"PRIu32"'\n", hci_event_user_confirmation_request_get_numeric_value(packet));
        //log_info("SSP User Confirmation Auto accept\n");
        break;
      case HCI_EVENT_HID_META:
        switch (hci_event_hid_meta_get_subevent_code(packet)){
          case HID_SUBEVENT_CONNECTION_OPENED:
            status = hid_subevent_connection_opened_get_status(packet);
            if (status) {
              // outgoing connection failed
              printf("Connection failed, status 0x%x\n", status);
              hid_cid = 0;
              return;
            }
            hid_cid = hid_subevent_connection_opened_get_hid_cid(packet);
            //log_info("HID Connected\n");
            hid_device_request_can_send_now_event(hid_cid);
            break;
          case HID_SUBEVENT_CONNECTION_CLOSED:
            //log_info("HID Disconnected\n");
            hid_cid = 0;
            break;
          case HID_SUBEVENT_CAN_SEND_NOW:  
            if(hid_cid!=0){ //Solves crash when disconnecting gamepad on android
              send_report_joystick();
              hid_device_request_can_send_now_event(hid_cid);
            }
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
  }
}


static void setup_sdp() {
  hci_event_callback_registration.callback = &packet_handler;
  hci_add_event_handler(&hci_event_callback_registration);

  l2cap_init();
  sdp_init();
  memset(hid_service_buffer, 0, sizeof(hid_service_buffer));

  // service record handle is 0x10000 in gamepad, 0x10001 in keyboard
  hid_create_sdp_record(hid_service_buffer, 0x10000, &hid_params);
  sdp_register_service(hid_service_buffer);
  hid_device_init(0, sizeof(reportMap), reportMap);
  hid_device_register_packet_handler(&packet_handler);
}


int btstack_main(int argc, const char * argv[]);
int btstack_main(int argc, const char * argv[]){
  (void)argc;
  (void)argv;

  // gap setup
  gap_discoverable_control(1);
  gap_set_class_of_device(GAMEPAD_DEVICE_ID);
  gap_set_local_name(hid_device_name);

  setup_sdp();

  hci_power_control(HCI_POWER_ON);
  return 0;
}