/*
 * SPDX-License-Identifier: MIT
 * Switch Pro Controller Bluetooth driver for GP2040-CE
 * Ported from retro-pico-switch by DavidPagels
 */

#define __BTSTACK_FILE__ "SwitchBluetoothDriver.cpp"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/rand.h"

#include "btstack.h"
#include "btstack_event.h"
#include "btstack_run_loop.h"

#include "drivers/switchbt/SwitchBluetoothDriver.h"
#include "drivers/switchbt/SwitchBTConsts.h"
#include "drivers/switchbt/BtStackUtils.h"

#include <cstring>

// ============================================================================
// Module State
// ============================================================================

static SwitchBTState connectionState = SwitchBTState::IDLE;
static uint16_t hid_cid = 0;
static bool btInitialized = false;
static bd_addr_t bt_addr;

// Host (Switch) address for reconnection
static bd_addr_t host_addr;
static bool has_host_addr = false;
static bool reconnect_pending = false;  // Flag to retry reconnection in main loop
static uint32_t reconnect_start_time = 0;  // When we started trying to reconnect
static bool deep_sleep_active = false;  // True when CYW43 is deinitialized
#define RECONNECT_TIMEOUT_MS 5000  // Go to sleep after 5 seconds of failed reconnection

// ============================================================================
// Flash Storage with Delayed Write (similar to FlashPROM)
// ============================================================================

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/timer.h"
#include "pico/multicore.h"

#define BT_PAIRING_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define BT_PAIRING_MAGIC 0x53574254  // "SWBT"
#define BT_FLASH_WRITE_DELAY_MS 500  // Delay before writing to flash

typedef struct {
    uint32_t magic;
    uint8_t host_mac[6];
    uint8_t padding[2];
} __attribute__((packed)) BTPairingData;

static volatile alarm_id_t flash_write_alarm = 0;
static bd_addr_t pending_host_addr;  // Address to save when alarm fires

// Alarm callback - runs after delay, does the actual flash write
static int64_t flash_write_callback(alarm_id_t id, void* user_data) {
    (void)id;
    (void)user_data;

    BTPairingData data;
    data.magic = BT_PAIRING_MAGIC;
    memcpy(data.host_mac, pending_host_addr, 6);
    data.padding[0] = 0;
    data.padding[1] = 0;

    // Safely write to flash - use multicore lockout like FlashPROM
    multicore_lockout_start_blocking();
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(BT_PAIRING_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(BT_PAIRING_FLASH_OFFSET, (const uint8_t*)&data, sizeof(data));
    restore_interrupts(ints);
    multicore_lockout_end_blocking();

    flash_write_alarm = 0;
    return 0;  // Don't reschedule
}

// Schedule a delayed flash write
static void schedule_flash_save(const bd_addr_t addr) {
    memcpy(pending_host_addr, addr, 6);

    // Cancel any pending write
    if (flash_write_alarm != 0) {
        cancel_alarm(flash_write_alarm);
    }

    // Schedule write for later
    flash_write_alarm = add_alarm_in_ms(BT_FLASH_WRITE_DELAY_MS, flash_write_callback, nullptr, true);
}

// Forward declaration
static void reinit_bluetooth(void);

// Enter deep sleep - deinitialize CYW43 to save power
static void enter_deep_sleep(void) {
    if (deep_sleep_active) return;

    // Deinitialize the CYW43 (Bluetooth/WiFi chip)
    cyw43_arch_deinit();
    deep_sleep_active = true;
    btInitialized = false;
}

// Wake from deep sleep - reinitialize everything
static void wake_from_deep_sleep(void) {
    if (!deep_sleep_active) return;

    deep_sleep_active = false;
    reinit_bluetooth();
}

// Load saved host from flash
static void load_paired_host(void) {
    const BTPairingData* data = (const BTPairingData*)(XIP_BASE + BT_PAIRING_FLASH_OFFSET);

    if (data->magic == BT_PAIRING_MAGIC) {
        // Validate MAC - not all zeros or all 0xFF
        bool all_zero = true;
        bool all_ff = true;
        for (int i = 0; i < 6; i++) {
            if (data->host_mac[i] != 0x00) all_zero = false;
            if (data->host_mac[i] != 0xFF) all_ff = false;
        }
        if (!all_zero && !all_ff) {
            memcpy(host_addr, data->host_mac, 6);
            has_host_addr = true;
        }
    }
}


// SDP service buffers
static uint8_t hid_service_buffer[700];
static uint8_t pnp_service_buffer[200];

// Report buffers
static uint8_t report[64];
static uint8_t switchRequestReport[64];

// State variables
static uint8_t timer = 0;
static uint32_t timestamp = 0;
static bool imu_enabled = false;
static bool vibration_enabled = false;
static uint8_t vibration_idx = 0;
static uint8_t vibration_report = 0;
static uint8_t player_number = 0;

// Current input state for report generation
static SwitchBTInput currentInput = {0};


// ============================================================================
// Report Generation (from retro-pico-switch SwitchCommon)
// ============================================================================

static void set_empty_report() {
    memset(report, 0x00, sizeof(report));
}

static void set_empty_switch_request_report() {
    memset(switchRequestReport, 0x00, sizeof(switchRequestReport));
}

static void set_timer() {
    if (timestamp == 0) {
        timestamp = to_ms_since_boot(get_absolute_time());
        report[2] = 0x00;
        return;
    }
    uint32_t now = to_ms_since_boot(get_absolute_time());
    uint32_t delta_t = (now - timestamp);
    uint32_t elapsed_ticks = (delta_t * 4);
    timer = (timer + elapsed_ticks) & 0xFF;
    report[2] = timer;
    timestamp = now;
}

static void set_standard_input_report() {
    set_timer();

    // Battery connection
    report[3] = 0x80;

    // Button byte 0: Y, X, B, A, SR, SL, R, ZR
    report[4] = 0;
    if (currentInput.buttons & SWBT_GAMEPAD_MASK_B3) report[4] |= SWITCH_MASK_Y;
    if (currentInput.buttons & SWBT_GAMEPAD_MASK_B4) report[4] |= SWITCH_MASK_X;
    if (currentInput.buttons & SWBT_GAMEPAD_MASK_B1) report[4] |= SWITCH_MASK_B;
    if (currentInput.buttons & SWBT_GAMEPAD_MASK_B2) report[4] |= SWITCH_MASK_A;
    if (currentInput.buttons & SWBT_GAMEPAD_MASK_R1) report[4] |= SWITCH_MASK_R;
    if (currentInput.buttons & SWBT_GAMEPAD_MASK_R2) report[4] |= SWITCH_MASK_ZR;

    // Button byte 1: Minus, Plus, R3, L3, Home, Capture
    report[5] = 0;
    if (currentInput.buttons & SWBT_GAMEPAD_MASK_S1) report[5] |= SWITCH_MASK_MINUS;
    if (currentInput.buttons & SWBT_GAMEPAD_MASK_S2) report[5] |= SWITCH_MASK_PLUS;
    if (currentInput.buttons & SWBT_GAMEPAD_MASK_R3) report[5] |= SWITCH_MASK_R3;
    if (currentInput.buttons & SWBT_GAMEPAD_MASK_L3) report[5] |= SWITCH_MASK_L3;
    if (currentInput.buttons & SWBT_GAMEPAD_MASK_A1) report[5] |= SWITCH_MASK_HOME;
    if (currentInput.buttons & SWBT_GAMEPAD_MASK_A2) report[5] |= SWITCH_MASK_CAPTURE;

    // Button byte 2: L, ZL (upper nibble) - D-pad mapped to left analog instead
    report[6] = 0;
    // D-pad is NOT mapped to HAT - it goes to left analog stick below
    if (currentInput.buttons & SWBT_GAMEPAD_MASK_L1) report[6] |= SWITCH_MASK_L;
    if (currentInput.buttons & SWBT_GAMEPAD_MASK_L2) report[6] |= SWITCH_MASK_ZL;

    // Left stick (12-bit, packed into 3 bytes)
    // Convert from 16-bit (0-65535) to 12-bit (0-4095)
    // D-pad overrides left analog stick when pressed
    uint16_t lx = 2048;  // Center (0x800)
    uint16_t ly = 2048;  // Center

    // Check if D-pad is pressed - if so, use D-pad as left analog
    if (currentInput.dpad & (SWBT_GAMEPAD_MASK_UP | SWBT_GAMEPAD_MASK_DOWN |
                              SWBT_GAMEPAD_MASK_LEFT | SWBT_GAMEPAD_MASK_RIGHT)) {
        if (currentInput.dpad & SWBT_GAMEPAD_MASK_LEFT)  lx = 0;
        if (currentInput.dpad & SWBT_GAMEPAD_MASK_RIGHT) lx = 4095;
        if (currentInput.dpad & SWBT_GAMEPAD_MASK_UP)    ly = 4095;  // Inverted (like analog stick)
        if (currentInput.dpad & SWBT_GAMEPAD_MASK_DOWN)  ly = 0;
    } else {
        // No D-pad pressed, use actual analog stick values
        lx = currentInput.lx >> 4;
        ly = (65535 - currentInput.ly) >> 4;  // Invert Y
    }
    report[7] = lx & 0xFF;
    report[8] = ((lx >> 8) & 0x0F) | ((ly & 0x0F) << 4);
    report[9] = ly >> 4;

    // Right stick
    uint16_t rx = currentInput.rx >> 4;
    uint16_t ry = (65535 - currentInput.ry) >> 4;  // Invert Y
    report[10] = rx & 0xFF;
    report[11] = ((rx >> 8) & 0x0F) | ((ry & 0x0F) << 4);
    report[12] = ry >> 4;

    // Vibration report
    report[13] = vibration_report;
}

static void set_subcommand_reply() {
    report[1] = 0x21;
    if (vibration_enabled) {
        vibration_idx = (vibration_idx + 1) % 4;
        vibration_report = VIB_OPTS[vibration_idx];
    }
    set_standard_input_report();
}

static void set_full_input_report() {
    report[1] = 0x30;
    set_standard_input_report();

    // IMU data (if enabled)
    if (imu_enabled) {
        uint8_t imu_data[36] = {
            0x75, 0xFD, 0xFD, 0xFF, 0x09, 0x10, 0x21, 0x00, 0xD5, 0xFF, 0xE0, 0xFF,
            0x72, 0xFD, 0xF9, 0xFF, 0x0A, 0x10, 0x22, 0x00, 0xD5, 0xFF, 0xE0, 0xFF,
            0x76, 0xFD, 0xFC, 0xFF, 0x09, 0x10, 0x23, 0x00, 0xD5, 0xFF, 0xE0, 0xFF
        };
        memcpy(report + 14, imu_data, sizeof(imu_data));
    }
}

// ============================================================================
// Subcommand Handlers
// ============================================================================

static void set_bt() {
    report[14] = 0x81;
    report[15] = 0x01;
    report[16] = 0x03;
}

static void set_device_info() {
    report[14] = 0x82;  // ACK
    report[15] = 0x02;  // Subcommand
    report[16] = 0x03;  // FW version major
    report[17] = 0x48;  // FW version minor
    report[18] = 0x03;  // Controller type (Pro Controller)
    report[19] = 0x02;  // Unknown, always 2
    memcpy(report + 20, bt_addr, 6);  // MAC address
    report[26] = 0x01;  // Unknown, always 1
    report[27] = 0x01;  // Colors from SPI
}

static void set_shipment() {
    report[14] = 0x80;
    report[15] = 0x08;
}

static void toggle_imu() {
    imu_enabled = (switchRequestReport[11] == 0x01);
    report[14] = 0x80;
    report[15] = 0x40;
}

static void imu_sensitivity() {
    report[14] = 0x80;
    report[15] = 0x41;
}

static void spi_read() {
    uint8_t addr_top = switchRequestReport[12];
    uint8_t addr_bottom = switchRequestReport[11];
    uint8_t read_length = switchRequestReport[15];

    report[14] = 0x90;  // ACK
    report[15] = 0x10;  // Subcommand
    report[16] = addr_bottom;
    report[17] = addr_top;
    report[20] = read_length;

    // Stick parameters
    uint8_t params[18] = {
        0x0F, 0x30, 0x61, 0x96, 0x30, 0xF3,
        0xD4, 0x14, 0x54, 0x41, 0x15, 0x54,
        0xC7, 0x79, 0x9C, 0x33, 0x36, 0x63
    };

    if (addr_top == 0x60 && addr_bottom == 0x00) {
        // Serial number - set to 0xFF (no serial)
        memset(report + 21, 0xff, 16);
    } else if (addr_top == 0x60 && addr_bottom == 0x50) {
        // Colors
        memset(report + 21, 0x32, 3);   // Body gray
        memset(report + 24, 0xff, 3);   // Buttons
        memset(report + 27, 0xff, 7);   // Grips
    } else if (addr_top == 0x60 && addr_bottom == 0x80) {
        // Six-axis factory parameters
        report[21] = 0x50;
        report[22] = 0xFD;
        report[23] = 0x00;
        report[24] = 0x00;
        report[25] = 0xC6;
        report[26] = 0x0F;
        memcpy(report + 27, params, sizeof(params));
    } else if (addr_top == 0x60 && addr_bottom == 0x98) {
        // Stick params 2
        memcpy(report + 21, params, sizeof(params));
    } else if (addr_top == 0x80 && addr_bottom == 0x10) {
        // User calibration (none)
        memset(report + 21, 0xff, 3);
    } else if (addr_top == 0x60 && addr_bottom == 0x3D) {
        // Factory stick calibration
        uint8_t l_cal[9] = {0xD4, 0x75, 0x61, 0xE5, 0x87, 0x7C, 0xEC, 0x55, 0x61};
        uint8_t r_cal[9] = {0x5D, 0xD8, 0x7F, 0x18, 0xE6, 0x61, 0x86, 0x65, 0x5D};
        memcpy(report + 21, l_cal, 9);
        memcpy(report + 30, r_cal, 9);
        report[39] = 0xFF;
        memset(report + 40, 0x32, 3);  // Body
        memset(report + 43, 0xff, 3);  // Buttons
    } else if (addr_top == 0x60 && addr_bottom == 0x20) {
        // Six-axis calibration
        uint8_t sa_cal[24] = {
            0xcc, 0x00, 0x40, 0x00, 0x91, 0x01,
            0x00, 0x40, 0x00, 0x40, 0x00, 0x40,
            0xe7, 0xff, 0x0e, 0x00, 0xdc, 0xff,
            0x3b, 0x34, 0x3b, 0x34, 0x3b, 0x34
        };
        memcpy(report + 21, sa_cal, sizeof(sa_cal));
    } else {
        memset(report + 21, 0xFF, read_length);
    }
}

static void set_mode() {
    report[14] = 0x80;
    report[15] = 0x03;
}

static void set_trigger_buttons() {
    report[14] = 0x83;
    report[15] = 0x04;
}

static void enable_vibration() {
    report[14] = 0x80;
    report[15] = 0x48;
    vibration_enabled = true;
    vibration_idx = 0;
    vibration_report = VIB_OPTS[0];
}

static void set_player_lights() {
    report[14] = 0x80;
    report[15] = 0x30;
    uint8_t bitfield = switchRequestReport[11];
    if (bitfield == 0x01 || bitfield == 0x10) player_number = 1;
    else if (bitfield == 0x03 || bitfield == 0x30) player_number = 2;
    else if (bitfield == 0x07 || bitfield == 0x70) player_number = 3;
    else if (bitfield == 0x0F || bitfield == 0xF0) player_number = 4;
}

static void set_nfc_ir_state() {
    report[14] = 0x80;
    report[15] = 0x22;
}

static void set_nfc_ir_config() {
    report[14] = 0xA0;
    report[15] = 0x21;
    uint8_t params[8] = {0x01, 0x00, 0xFF, 0x00, 0x08, 0x00, 0x1B, 0x01};
    memcpy(report + 16, params, sizeof(params));
    report[49] = 0xC8;
}

static uint8_t* generate_report() {
    set_empty_report();
    report[0] = 0xa1;  // HID Input report header

    switch (switchRequestReport[10]) {
        case BLUETOOTH_PAIR_REQUEST:
            set_subcommand_reply();
            set_bt();
            break;
        case REQUEST_DEVICE_INFO:
            set_subcommand_reply();
            set_device_info();
            break;
        case SET_SHIPMENT:
            set_subcommand_reply();
            set_shipment();
            break;
        case SPI_READ:
            set_subcommand_reply();
            spi_read();
            break;
        case SET_MODE:
            set_subcommand_reply();
            set_mode();
            break;
        case TRIGGER_BUTTONS:
            set_subcommand_reply();
            set_trigger_buttons();
            break;
        case TOGGLE_IMU:
            set_subcommand_reply();
            toggle_imu();
            break;
        case IMU_SENSITIVITY:
            set_subcommand_reply();
            imu_sensitivity();
            break;
        case ENABLE_VIBRATION:
            set_subcommand_reply();
            enable_vibration();
            break;
        case SET_PLAYER:
            set_subcommand_reply();
            set_player_lights();
            break;
        case SET_NFC_IR_STATE:
            set_subcommand_reply();
            set_nfc_ir_state();
            break;
        case SET_NFC_IR_CONFIG:
            set_subcommand_reply();
            set_nfc_ir_config();
            break;
        default:
            set_full_input_report();
            break;
    }
    return report;
}

// ============================================================================
// BTstack Packet Handler
// ============================================================================

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void)channel;
    (void)size;

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }

    switch (packet[0]) {
        case HCI_EVENT_HID_META:
            switch (hci_event_hid_meta_get_subevent_code(packet)) {
                case HID_SUBEVENT_CONNECTION_OPENED: {
                    uint8_t status = hid_subevent_connection_opened_get_status(packet);
                    if (status) {
                        hid_cid = 0;
                        // Connection failed - schedule retry in main loop
                        if (has_host_addr) {
                            reconnect_pending = true;
                            connectionState = SwitchBTState::RECONNECTING;
                        } else {
                            connectionState = SwitchBTState::DISCOVERABLE;
                        }
                        return;
                    }
                    hid_cid = hid_subevent_connection_opened_get_hid_cid(packet);
                    connectionState = SwitchBTState::CONNECTED;
                    reconnect_pending = false;

                    // Get connected host address
                    bd_addr_t connected_addr;
                    hid_subevent_connection_opened_get_bd_addr(packet, connected_addr);

                    // Check if this is a new host - schedule delayed save
                    if (!has_host_addr || memcmp(host_addr, connected_addr, 6) != 0) {
                        memcpy(host_addr, connected_addr, 6);
                        has_host_addr = true;
                        schedule_flash_save(connected_addr);  // Delayed write
                    }

                    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
                    hid_device_request_can_send_now_event(hid_cid);
                    break;
                }
                case HID_SUBEVENT_CONNECTION_CLOSED:
                    hid_cid = 0;
                    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);

                    // Schedule reconnection in main loop (with timeout)
                    if (has_host_addr) {
                        reconnect_pending = true;
                        reconnect_start_time = to_ms_since_boot(get_absolute_time());
                        connectionState = SwitchBTState::RECONNECTING;
                    } else {
                        connectionState = SwitchBTState::DISCOVERABLE;
                    }
                    break;
                case HID_SUBEVENT_CAN_SEND_NOW:
                    if (hid_cid != 0) {
                        uint8_t *rep = generate_report();
                        hid_device_send_interrupt_message(hid_cid, rep, 50);
                        set_empty_switch_request_report();
                        hid_device_request_can_send_now_event(hid_cid);
                    }
                    break;
            }
            break;
    }
}

static void hid_report_callback(uint16_t cid, hid_report_type_t report_type,
                                 uint16_t report_id, int report_size, uint8_t *report_data) {
    (void)cid;
    (void)report_type;

    if (report_id == 0x01 || report_id == 0x10 || report_id == 0x11) {
        // BTstack doesn't include report_id in report_data, but our code expects it
        // So we shift everything by 1 and put report_id at position 0
        switchRequestReport[0] = (uint8_t)report_id;
        int copy_size = (report_size < 63) ? report_size : 63;
        memcpy(switchRequestReport + 1, report_data, copy_size);
    }
}

// ============================================================================
// BTstack Setup (shared between init and reinit)
// ============================================================================

static void setup_btstack(void) {
    // Initialize L2CAP
    l2cap_init();

    // Configure GAP
    gap_discoverable_control(1);
    gap_set_class_of_device(0x2508);
    gap_set_local_name("Pro Controller");
    gap_set_default_link_policy_settings(LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE);
    gap_set_allow_role_switch(true);

    // Set chipset and BD address
    hci_set_chipset(btstack_chipset_cyw43_instance());
    hci_set_bd_addr(bt_addr);

    // Initialize SDP
    sdp_init();

    // Create HID SDP record
    hid_sdp_record_t hid_sdp_record = {
        .hid_device_subclass = 0x2508,
        .hid_country_code = 33,
        .hid_virtual_cable = 1,
        .hid_remote_wake = 1,
        .hid_reconnect_initiate = 1,
        .hid_normally_connectable = 0,
        .hid_boot_device = 0,
        .hid_ssr_host_max_latency = 0xFFFF,
        .hid_ssr_host_min_timeout = 0xFFFF,
        .hid_supervision_timeout = 3200,
        .hid_descriptor = switch_bt_report_descriptor,
        .hid_descriptor_size = sizeof(switch_bt_report_descriptor),
        .device_name = "Wireless Gamepad"
    };

    memset(hid_service_buffer, 0, sizeof(hid_service_buffer));
    create_sdp_hid_record(hid_service_buffer, &hid_sdp_record);
    sdp_register_service(hid_service_buffer);

    // Create PnP SDP record (Nintendo Pro Controller)
    memset(pnp_service_buffer, 0, sizeof(pnp_service_buffer));
    create_sdp_pnp_record(pnp_service_buffer, DEVICE_ID_VENDOR_ID_SOURCE_USB,
                          0x057E, 0x2009, 0x0001);
    sdp_register_service(pnp_service_buffer);

    // Initialize HID device
    hid_device_init(1, sizeof(switch_bt_report_descriptor), switch_bt_report_descriptor);

    // Register callbacks
    static btstack_packet_callback_registration_t hci_event_callback_registration;
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    hid_device_register_packet_handler(&packet_handler);
    hid_device_register_report_data_callback(&hid_report_callback);

    // Power on
    hci_power_control(HCI_POWER_ON);

    // Poll until HCI is working (give BTstack time to initialize)
    for (int i = 0; i < 100; i++) {
        cyw43_arch_poll();
        sleep_ms(10);
    }

    btInitialized = true;
}

// ============================================================================
// Public API
// ============================================================================

void switchbt_init(void) {
    connectionState = SwitchBTState::INITIALIZING;

    // Try to load saved host from flash
    load_paired_host();

    // Generate random MAC address starting with Nintendo prefix 7c:bb:8a
    bd_addr_t newAddr = {
        0x7c, 0xbb, 0x8a,
        (uint8_t)(get_rand_32() & 0xff),
        (uint8_t)(get_rand_32() & 0xff),
        (uint8_t)(get_rand_32() & 0xff)
    };
    memcpy(bt_addr, newAddr, 6);

    // Initialize CYW43
    if (cyw43_arch_init()) {
        // Rapid blink to indicate error
        while (1) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            sleep_ms(100);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            sleep_ms(100);
        }
    }

    // Setup BTstack (shared initialization)
    setup_btstack();

    // If we have a saved host, try to reconnect; otherwise wait for pairing
    if (has_host_addr) {
        connectionState = SwitchBTState::RECONNECTING;
        reconnect_pending = true;
        reconnect_start_time = to_ms_since_boot(get_absolute_time());
    } else {
        connectionState = SwitchBTState::DISCOVERABLE;
    }

    // Double blink = init complete
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(100);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    sleep_ms(100);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(100);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
}

// Reinitialize Bluetooth after deep sleep (uses same MAC address from initial pairing)
static void reinit_bluetooth(void) {
    // Initialize CYW43
    if (cyw43_arch_init()) {
        btInitialized = false;
        return;
    }

    // Setup BTstack (shared initialization)
    setup_btstack();
    hid_cid = 0;  // Reset connection ID

    // Single blink = reinit complete
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(100);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
}

bool switchbt_process(const SwitchBTInput* input) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    // Store input for report generation
    if (input) {
        memcpy(&currentInput, input, sizeof(SwitchBTInput));
    }

    // Check for any button press (used to wake from sleep)
    bool anyButtonPressed = input && (input->buttons != 0 || input->dpad != 0);

    // Handle SLEEPING state (deep sleep) - wait for button press
    if (connectionState == SwitchBTState::SLEEPING) {
        // CYW43 is deinitialized in deep sleep, so no LED control
        // Just poll for button press

        if (anyButtonPressed && has_host_addr) {
            // Wake from deep sleep and reinitialize Bluetooth
            wake_from_deep_sleep();
            connectionState = SwitchBTState::RECONNECTING;
            reconnect_pending = true;
            reconnect_start_time = now;
        }

        // Small delay to save CPU power while sleeping
        sleep_ms(10);
        return false;
    }

    if (!btInitialized) return false;

    // Poll the async context multiple times to ensure BTstack processes events
    for (int i = 0; i < 10; i++) {
        cyw43_arch_poll();
    }

    // Handle reconnection retries with timeout
    static uint32_t lastReconnectAttempt = 0;

    if (connectionState == SwitchBTState::RECONNECTING && reconnect_pending && has_host_addr && hid_cid == 0) {
        // Check if we've timed out
        if (reconnect_start_time > 0 && (now - reconnect_start_time > RECONNECT_TIMEOUT_MS)) {
            // Enter deep sleep after timeout (deinitializes CYW43 to save power)
            reconnect_pending = false;
            connectionState = SwitchBTState::SLEEPING;
            enter_deep_sleep();
            return false;
        }

        // Retry every 1 second (faster retries during the 5-second window)
        if (now - lastReconnectAttempt > 1000) {
            lastReconnectAttempt = now;
            hid_device_connect(host_addr, &hid_cid);
        }
    }

    static uint32_t lastBlink = 0;
    static bool ledOn = false;

    if (connectionState == SwitchBTState::RECONNECTING) {
        // Fast blink when reconnecting
        if (now - lastBlink > 150) {
            ledOn = !ledOn;
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, ledOn);
            lastBlink = now;
        }
        return false;
    }

    if (connectionState == SwitchBTState::DISCOVERABLE) {
        // Slow blink when waiting for pairing
        if (now - lastBlink > 500) {
            ledOn = !ledOn;
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, ledOn);
            lastBlink = now;
        }
        return false;
    }

    if (connectionState == SwitchBTState::CONNECTED) {
        // Solid LED when connected
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        return true;
    }

    return false;
}

void switchbt_clear_pairing(void) {
    has_host_addr = false;
    reconnect_pending = false;
    memset(host_addr, 0, 6);

    // Cancel any pending write and erase flash
    if (flash_write_alarm != 0) {
        cancel_alarm(flash_write_alarm);
        flash_write_alarm = 0;
    }

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(BT_PAIRING_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);
}

SwitchBTState switchbt_get_state(void) {
    return connectionState;
}

bool switchbt_is_connected(void) {
    return connectionState == SwitchBTState::CONNECTED;
}
