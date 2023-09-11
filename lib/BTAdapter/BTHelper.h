#ifndef BT_HELPER_H_
#define BT_HELPER_H_

#include <array>
#include <string_view>

void printEventType(uint8_t *packet) {

  const unsigned int event_type = hci_event_packet_get_type(packet);
  const char* messages[] = {
    "NOP",
    "INQUIRY_COMPLETE",
    "INQUIRY_RESULT",
    "CONNECTION_COMPLETE",

    "CONNECTION_REQUEST",
    "DISCONNECTION_COMPLETE",
    "AUTHENTICATION_COMPLETE",
    "REMOTE_NAME_REQUEST_COMPLETE",

    "ENCRYPTION_CHANGE",
    "CHANGE_CONNECTION_LINK_KEY_COMPLETE",
    "MASTER_LINK_KEY_COMPLETE",
    "READ_REMOTE_SUPPORTED_FEATURES_COMPLETE",

    "READ_REMOTE_VERSION_INFORMATION_COMPLETE",
    "QOS_SETUP_COMPLETE",
    "COMMAND_COMPLETE", // ignored
    "COMMAND_STATUS",

    "HARDWARE_ERROR",
    "FLUSH_OCCURRED",
    "ROLE_CHANGE",
    0, // ignored

    "MODE_CHANGE",
    "RETURN_LINK_KEYS",
    "PIN_CODE_REQUEST",
    "LINK_KEY_REQUEST",

    "LINK_KEY_NOTIFICATION",
    "LOOPBACK_COMMAND",
    "DATA_BUFFER_OVERFLOW",
    "MAX_SLOTS_CHANGED",

    "READ_CLOCK_OFFSET_COMPLETE",
    "CONNECTION_PACKET_TYPE_CHANGED",
    "QOS_VIOLATION",
    "", // 0x1f not defined

    "PAGE_SCAN_REPETITION_MODE_CHANGE",
    "FLOW_SPECIFICATION_COMPLETE",
    "INQUIRY_RESULT_WITH_RSSI",
    "READ_REMOTE_EXTENDED_FEATURES_COMPLETE",

    "","","","", // 0x24..0x2b not defined
    "","","","",

    "SYNCHRONOUS_CONNECTION_COMPLETE",
    "SYNCHRONOUS_CONNECTION_CHANGED",
    "SNIFF_SUBRATING",
    "EXTENDED_INQUIRY_RESPONSE",

    "ENCRYPTION_KEY_REFRESH_COMPLETE",
    "IO_CAPABILITY_REQUEST",
    "IO_CAPABILITY_RESPONSE",
    "USER_CONFIRMATION_REQUEST",

    "USER_PASSKEY_REQUEST",
    "REMOTE_OOB_DATA_REQUEST",
    "SIMPLE_PAIRING_COMPLETE",
    "", // 0x37 not defined

    "LINK_SUPERVISION_TIMEOUT_CHANGED",
    "ENHANCED_FLUSH_COMPLETE",
    "", // 0x03a not defined
    "USER_PASSKEY_NOTIFICATION",
  };

  if(event_type < 60){
    if( messages[event_type] ){
      printf("Type: HCI_EVENT_%s\n", messages[event_type]);
    }
    return;
  }

  switch (event_type) {
    case BTSTACK_EVENT_STATE: printf("Type: BTSTACK_EVENT_STATE\n"); break;
    case BTSTACK_EVENT_SCAN_MODE_CHANGED: printf("Type: BTSTACK_EVENT_SCAN_MODE_CHANGED\n"); break;
    case BTSTACK_EVENT_NR_CONNECTIONS_CHANGED: printf("Type: BTSTACK_EVENT_NR_CONNECTIONS_CHANGED\n"); break;

    case HCI_EVENT_TRANSPORT_PACKET_SENT: /*printf("Type: HCI_EVENT_TRANSPORT_PACKET_SENT\n");*/ break;
    case HCI_EVENT_HID_META: printf("Type: HCI_EVENT_HID_META\n"); break;
    case HCI_EVENT_VENDOR_SPECIFIC: printf("Type: HCI_EVENT_VENDOR_SPECIFIC\n"); break;
    
    case GAP_EVENT_SECURITY_LEVEL: printf("Type: GAP_EVENT_SECURITY_LEVEL\n"); break;
    case GAP_EVENT_PAIRING_STARTED: printf("Type: GAP_EVENT_PAIRING_STARTED\n"); break;
    case GAP_EVENT_PAIRING_COMPLETE: printf("Type: GAP_EVENT_PAIRING_COMPLETE\n"); break;
    


    case HCI_EVENT_LE_META: 
      switch(hci_event_le_meta_get_subevent_code(packet)){
        case HCI_SUBEVENT_LE_CONNECTION_COMPLETE: printf("Type: HCI_EVENT_LE_META - Sub: CONNECTION_COMPLETE\n");  break;
        case HCI_SUBEVENT_LE_ADVERTISING_REPORT: printf("Type: HCI_EVENT_LE_META - Sub: ADVERTISING_REPORT\n");  break;
        case HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE: printf("Type: HCI_EVENT_LE_META - Sub: CONNECTION_UPDATE_COMPLETE\n");  break;
        case HCI_SUBEVENT_LE_READ_REMOTE_FEATURES_COMPLETE: printf("Type: HCI_EVENT_LE_META - Sub: READ_REMOTE_FEATURES_COMPLETE\n");  break;

        case HCI_SUBEVENT_LE_LONG_TERM_KEY_REQUEST: printf("Type: HCI_EVENT_LE_META - Sub: LONG_TERM_KEY_REQUEST\n");  break;
        case HCI_SUBEVENT_LE_REMOTE_CONNECTION_PARAMETER_REQUEST: printf("Type: HCI_EVENT_LE_META - Sub: REMOTE_CONNECTION_PARAMETER_REQUEST\n");  break;
        case HCI_SUBEVENT_LE_DATA_LENGTH_CHANGE: printf("Type: HCI_EVENT_LE_META - Sub: DATA_LENGTH_CHANGE\n");  break;
        case HCI_SUBEVENT_LE_READ_LOCAL_P256_PUBLIC_KEY_COMPLETE: printf("Type: HCI_EVENT_LE_META - Sub: READ_LOCAL_P256_PUBLIC_KEY_COMPLETE\n");  break;

        case HCI_SUBEVENT_LE_GENERATE_DHKEY_COMPLETE: printf("Type: HCI_EVENT_LE_META - Sub: GENERATE_DHKEY_COMPLETE\n");  break;
        case HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE_V1: printf("Type: HCI_EVENT_LE_META - Sub: ENHANCED_CONNECTION_COMPLETE_V1\n");  break;
        case HCI_SUBEVENT_LE_DIRECT_ADVERTISING_REPORT: printf("Type: HCI_EVENT_LE_META - Sub: DIRECT_ADVERTISING_REPORT\n");  break;
        case HCI_SUBEVENT_LE_PHY_UPDATE_COMPLETE: printf("Type: HCI_EVENT_LE_META - Sub: PHY_UPDATE_COMPLETE\n");  break;

        case HCI_SUBEVENT_LE_EXTENDED_ADVERTISING_REPORT: printf("Type: HCI_EVENT_LE_META - Sub: EXTENDED_ADVERTISING_REPORT\n");  break;
        case HCI_SUBEVENT_LE_PERIODIC_ADVERTISING_SYNC_ESTABLISHMENT: printf("Type: HCI_EVENT_LE_META - Sub: PERIODIC_ADVERTISING_SYNC_ESTABLISHMENT\n");  break;
        case HCI_SUBEVENT_LE_PERIODIC_ADVERTISING_REPORT: printf("Type: HCI_EVENT_LE_META - Sub: PERIODIC_ADVERTISING_REPORT\n");  break;
        case HCI_SUBEVENT_LE_PERIODIC_ADVERTISING_SYNC_LOST: printf("Type: HCI_EVENT_LE_META - Sub: PERIODIC_ADVERTISING_SYNC_LOST\n");  break;

        case HCI_SUBEVENT_LE_SCAN_TIMEOUT: printf("Type: HCI_EVENT_LE_META - Sub: SCAN_TIMEOUT\n");  break;
        case HCI_SUBEVENT_LE_ADVERTISING_SET_TERMINATED: printf("Type: HCI_EVENT_LE_META - Sub: ADVERTISING_SET_TERMINATED\n");  break;
        case HCI_SUBEVENT_LE_SCAN_REQUEST_RECEIVED: printf("Type: HCI_EVENT_LE_META - Sub: SCAN_REQUEST_RECEIVED\n");  break;
        case HCI_SUBEVENT_LE_CHANNEL_SELECTION_ALGORITHM: printf("Type: HCI_EVENT_LE_META - Sub: CHANNEL_SELECTION_ALGORITHM\n");  break;

        case HCI_SUBEVENT_LE_CONNECTIONLESS_IQ_REPORT: printf("Type: HCI_EVENT_LE_META - Sub: CONNECTIONLESS_IQ_REPORT\n");  break;
        case HCI_SUBEVENT_LE_CONNECTION_IQ_REPORT: printf("Type: HCI_EVENT_LE_META - Sub: CONNECTION_IQ_REPORT\n");  break;
        case HCI_SUBEVENT_LE_PERIODIC_ADVERTISING_SYNC_TRANSFER_RECEIVED: printf("Type: HCI_EVENT_LE_META - Sub: PERIODIC_ADVERTISING_SYNC_TRANSFER_RECEIVED\n");  break;

        default: printf("Type: HCI_EVENT_LE_META - Sub: %x\n", event_type); break;
      }
      break;

    case HCI_EVENT_HIDS_META: 
      switch(hci_event_le_meta_get_subevent_code(packet)){
        case HIDS_SUBEVENT_CAN_SEND_NOW: /*printf("Type: HCI_EVENT_HIDS_META - Sub: CAN_SEND_NOW\n");*/  break;
        case HIDS_SUBEVENT_PROTOCOL_MODE: printf("Type: HCI_EVENT_HIDS_META - Sub: PROTOCOL_MODE\n");  break;
        case HIDS_SUBEVENT_BOOT_MOUSE_INPUT_REPORT_ENABLE: printf("Type: HCI_EVENT_HIDS_META - Sub: BOOT_MOUSE_INPUT_REPORT_ENABLE\n");  break;
        case HIDS_SUBEVENT_BOOT_KEYBOARD_INPUT_REPORT_ENABLE: printf("Type: HCI_EVENT_HIDS_META - Sub: BOOT_KEYBOARD_INPUT_REPORT_ENABLE\n");  break;
        
        case HIDS_SUBEVENT_INPUT_REPORT_ENABLE: printf("Type: HCI_EVENT_HIDS_META - Sub: INPUT_REPORT_ENABLE\n");  break;
        case HIDS_SUBEVENT_OUTPUT_REPORT_ENABLE: printf("Type: HCI_EVENT_HIDS_META - Sub: OUTPUT_REPORT_ENABLE\n");  break;
        case HIDS_SUBEVENT_FEATURE_REPORT_ENABLE: printf("Type: HCI_EVENT_HIDS_META - Sub: FEATURE_REPORT_ENABLE\n");  break;
        case HIDS_SUBEVENT_SUSPEND: printf("Type: HCI_EVENT_HIDS_META - Sub: SUSPEND\n");  break;

        case HIDS_SUBEVENT_EXIT_SUSPEND: printf("Type: HCI_EVENT_HIDS_META - Sub: EXIT_SUSPEND\n");  break;
        default: printf("Type: HCI_EVENT_HIDS_META - Sub: %x\n", event_type); break;
      }
      break;

    case SM_EVENT_JUST_WORKS_REQUEST: printf("Type: SM_EVENT_JUST_WORKS_REQUEST\n"); break;
    case SM_EVENT_PAIRING_STARTED: printf("Type: SM_EVENT_PAIRING_STARTED\n"); break;
    case SM_EVENT_IDENTITY_RESOLVING_STARTED: printf("Type: SM_EVENT_IDENTITY_RESOLVING_STARTED\n"); break;
    case SM_EVENT_IDENTITY_RESOLVING_FAILED: printf("Type: SM_EVENT_IDENTITY_RESOLVING_FAILED\n"); break;
    case SM_EVENT_IDENTITY_RESOLVING_SUCCEEDED: printf("Type: SM_EVENT_IDENTITY_RESOLVING_SUCCEEDED\n"); break;
    case SM_EVENT_IDENTITY_CREATED: printf("Type: SM_EVENT_IDENTITY_CREATED\n"); break;
    case SM_EVENT_PAIRING_COMPLETE: 
      switch (sm_event_pairing_complete_get_status(packet)){
                case ERROR_CODE_SUCCESS:
                    printf("Pairing complete, success\n");
                    break;
                case ERROR_CODE_CONNECTION_TIMEOUT:
                    printf("Pairing failed, timeout\n");
                    break;
                case ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION:
                    printf("Pairing failed, disconnected\n");
                    break;
                case ERROR_CODE_AUTHENTICATION_FAILURE:
                    printf("Pairing failed, authentication failure with reason = %u\n", sm_event_pairing_complete_get_reason(packet));
                    break;
                default:
                    break;
            }
            break;
    case SM_EVENT_REENCRYPTION_STARTED: printf("Type: SM_EVENT_REENCRYPTION_STARTED\n"); break;
    case SM_EVENT_REENCRYPTION_COMPLETE: printf("Type: SM_EVENT_REENCRYPTION_COMPLETE\n"); break;

    default: printf("Type: %x\n", event_type); break;
  }
}


#define BUILD_STR_ROW(tid, input_str) {\
  constexpr auto data = std::string_view(input_str); \
  static_assert(data.size()+1 <= 255, "BLE AD row is too long"); \
  result[offset++] = (uint8_t)data.size() + 1; \
  result[offset++] = tid; \
  for(std::size_t i=0; i < data.size(); ++i) result[offset++] = data[i]; \
}

#define NUMARGS(...)  ()
#define BUILD_DAT_ROW(...) { \
  constexpr uint8_t data[] = {__VA_ARGS__}; \
  static_assert(sizeof(data) <= 255, "BLE AD row is too long"); \
  result[offset++] = (uint8_t)sizeof(data); \
  for(std::size_t i=0; i < sizeof(data); ++i) result[offset++] = data[i]; \
}


#endif