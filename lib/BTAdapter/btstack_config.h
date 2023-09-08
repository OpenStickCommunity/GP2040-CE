#ifndef _BTADAPTER_BTSTACK_CONFIG_H
#define _BTADAPTER_BTSTACK_CONFIG_H

#include <stdint.h>

// Taken from Standalone PICO example, adapted with Classic BT example

#ifndef ENABLE_CLASSIC
#error Please link to pico_btstack_classic
#endif

// BTstack features that can be enabled
//
#ifdef ENABLE_BLE
#define ENABLE_LE_PERIPHERAL
#define ENABLE_LE_CENTRAL
#define ENABLE_L2CAP_LE_CREDIT_BASED_FLOW_CONTROL_MODE
#endif

#define ENABLE_LOG_INFO
#define ENABLE_LOG_ERROR
#define ENABLE_PRINTF_HEXDUMP

// for the client
#define MAX_NR_GATT_CLIENTS 1 // both


// BTstack configuration. buffers, sizes, ...
#define HCI_OUTGOING_PRE_BUFFER_SIZE 4 // both
#define HCI_ACL_PAYLOAD_SIZE (1691 + 4) // (255 + 4) in std, (1691 + 4) in classic
#define HCI_ACL_CHUNK_SIZE_ALIGNMENT 4 // both

 // classic only
#define MAX_NR_AVDTP_CONNECTIONS 1
#define MAX_NR_AVDTP_STREAM_ENDPOINTS 1
#define MAX_NR_AVRCP_CONNECTIONS 2
#define MAX_NR_BNEP_CHANNELS 1
#define MAX_NR_BNEP_SERVICES 1
#define MAX_NR_BTSTACK_LINK_KEY_DB_MEMORY_ENTRIES  2


#define MAX_NR_HCI_CONNECTIONS 1 // 1 in std, 2 in classic

// classic only
#define MAX_NR_HID_HOST_CONNECTIONS 1
#define MAX_NR_HIDS_CLIENTS 1
#define MAX_NR_HFP_CONNECTIONS 1
#define MAX_NR_L2CAP_CHANNELS  4
#define MAX_NR_L2CAP_SERVICES  3
#define MAX_NR_RFCOMM_CHANNELS 1
#define MAX_NR_RFCOMM_MULTIPLEXERS 1
#define MAX_NR_RFCOMM_SERVICES 1
#define MAX_NR_SERVICE_RECORD_ITEMS 4


#define MAX_NR_SM_LOOKUP_ENTRIES 3 // both
#define MAX_NR_WHITELIST_ENTRIES 16 // both
#define MAX_NR_LE_DEVICE_DB_ENTRIES 16 // both

// Limit number of ACL/SCO Buffer to use by stack to avoid cyw43 shared bus overrun
#define MAX_NR_CONTROLLER_ACL_BUFFERS 3 // both
#define MAX_NR_CONTROLLER_SCO_PACKETS 3 // both

// Enable and configure HCI Controller to Host Flow Control to avoid cyw43 shared bus overrun
#define ENABLE_HCI_CONTROLLER_TO_HOST_FLOW_CONTROL // both
#define HCI_HOST_ACL_PACKET_LEN 1024 // (255+4) in std, 1024 in classic
#define HCI_HOST_ACL_PACKET_NUM 3 // both
#define HCI_HOST_SCO_PACKET_LEN 120 // both
#define HCI_HOST_SCO_PACKET_NUM 3 // both

// Link Key DB and LE Device DB using TLV on top of Flash Sector interface
#define NVM_NUM_DEVICE_DB_ENTRIES 16 // both
#define NVM_NUM_LINK_KEYS 16 // both

// We don't give btstack a malloc, so use a fixed-size ATT DB.
#define MAX_ATT_DB_SIZE 512 // both

// BTstack HAL configuration
#define HAVE_EMBEDDED_TIME_MS // both
// map btstack_assert onto Pico SDK assert()
#define HAVE_ASSERT // both

// Some USB dongles take longer to respond to HCI reset (e.g. BCM20702A).
#define HCI_RESET_RESEND_TIMEOUT_MS 1000 // both
#define ENABLE_SOFTWARE_AES128 // both
#define ENABLE_MICRO_ECC_FOR_LE_SECURE_CONNECTIONS // both

 // classic only
#define HAVE_BTSTACK_STDIN

#define ENABLE_L2CAP_ENHANCED_RETRANSMISSION_MODE


#endif // MICROPY_INCLUDED_EXTMOD_BTSTACK_BTSTACK_CONFIG_H
