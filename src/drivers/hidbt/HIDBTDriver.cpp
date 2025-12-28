/*
 * SPDX-License-Identifier: MIT
 * Generic HID Bluetooth Gamepad driver
 * Simpler than Switch - no subcommand handling needed
 */

#define __BTSTACK_FILE__ "HIDBTDriver.cpp"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/rand.h"
#include "pico/unique_id.h"

#include "btstack.h"
#include "btstack_event.h"
#include "btstack_run_loop.h"

#include "drivers/hidbt/HIDBTDriver.h"

#include <cstring>

// ============================================================================
// External LED Configuration
// ============================================================================

#define EXTERNAL_LED_PIN 22  // GP22 for external status LED

// ============================================================================
// Module State
// ============================================================================

static HIDBTState connectionState = HIDBTState::IDLE;
static uint16_t hid_cid = 0;
static bool btInitialized = false;
static bd_addr_t bt_addr;

// Host address for reconnection
static bd_addr_t host_addr;
static bool has_host_addr = false;
static bool reconnect_pending = false;
static uint32_t reconnect_start_time = 0;
static bool deep_sleep_active = false;
// Flag for deferred flash save (can't call from interrupt context)
static bool flash_save_pending = false;
static bd_addr_t flash_save_addr;
// PC/generic HID reconnection takes longer than Switch - PCs don't actively poll for gamepads
// After timeout, go to discoverable mode instead of sleeping so PC can reconnect when ready
#define RECONNECT_TIMEOUT_MS 10000

// ============================================================================
// Flash Storage (separate from Switch BT to maintain independent pairings)
// ============================================================================

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/timer.h"
#include "pico/multicore.h"

// Use a DIFFERENT flash sector than Switch BT and BTstack link keys
// BTstack uses 0x1F6000-0x1F7FFF, Switch BT uses 0x1F7000, we use 0x1F5000
#define HIDBT_PAIRING_FLASH_OFFSET (0x1F6000 - FLASH_SECTOR_SIZE)
#define HIDBT_PAIRING_MAGIC 0x48494442  // "HIDB"
#define HIDBT_FLASH_WRITE_DELAY_MS 500

typedef struct {
    uint32_t magic;
    uint8_t host_mac[6];      // PC's MAC address
} __attribute__((packed)) HIDBTPairingData;

static volatile alarm_id_t flash_write_alarm = 0;
static bd_addr_t pending_host_addr;

static int64_t flash_write_callback(alarm_id_t id, void* user_data) {
    (void)id;
    (void)user_data;

    HIDBTPairingData data;
    data.magic = HIDBT_PAIRING_MAGIC;
    memcpy(data.host_mac, pending_host_addr, 6);

    multicore_lockout_start_blocking();
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(HIDBT_PAIRING_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(HIDBT_PAIRING_FLASH_OFFSET, (const uint8_t*)&data, sizeof(data));
    restore_interrupts(ints);
    multicore_lockout_end_blocking();

    flash_write_alarm = 0;
    return 0;
}

static void save_pairing_to_flash(const bd_addr_t addr) {
    HIDBTPairingData data;
    data.magic = HIDBT_PAIRING_MAGIC;
    memcpy(data.host_mac, addr, 6);

    multicore_lockout_start_blocking();
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(HIDBT_PAIRING_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(HIDBT_PAIRING_FLASH_OFFSET, (const uint8_t*)&data, sizeof(data));
    restore_interrupts(ints);
    multicore_lockout_end_blocking();
}

static void schedule_flash_save(const bd_addr_t addr) {
    memcpy(pending_host_addr, addr, 6);
    if (flash_write_alarm != 0) {
        cancel_alarm(flash_write_alarm);
    }
    flash_write_alarm = add_alarm_in_ms(HIDBT_FLASH_WRITE_DELAY_MS, flash_write_callback, nullptr, true);
}

static void reinit_bluetooth(void);

static void enter_deep_sleep(void) {
    if (deep_sleep_active) return;
    cyw43_arch_deinit();
    deep_sleep_active = true;
    btInitialized = false;
}

static void wake_from_deep_sleep(void) {
    if (!deep_sleep_active) return;
    deep_sleep_active = false;
    reinit_bluetooth();
}

static void load_paired_host(void) {
    const HIDBTPairingData* data = (const HIDBTPairingData*)(XIP_BASE + HIDBT_PAIRING_FLASH_OFFSET);

    if (data->magic == HIDBT_PAIRING_MAGIC) {
        // Check host MAC validity
        bool all_zero = true, all_ff = true;
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

// ============================================================================
// Generic HID Gamepad Descriptor
// ============================================================================

// Standard HID gamepad report descriptor
// Report format: buttons (16 bits) + hat (4 bits) + padding (4 bits) + 4 axes (8 bits each)
static const uint8_t hidbt_report_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    
    // 16 buttons
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (Button 1)
    0x29, 0x10,        //   Usage Maximum (Button 16)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x10,        //   Report Count (16)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    
    // Hat switch (D-pad)
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x39,        //   Usage (Hat switch)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x07,        //   Logical Maximum (7)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x65, 0x14,        //   Unit (Eng Rot:Angular Pos)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x42,        //   Input (Data,Var,Abs,Null)
    
    // Padding (4 bits)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x03,        //   Input (Const,Var,Abs)
    
    // 4 axes (X, Y, Z, Rz) - 8 bits each
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x35,        //   Usage (Rz)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    
    0xC0               // End Collection
};

// SDP service buffers
static uint8_t hid_service_buffer[700];
static uint8_t pnp_service_buffer[200];

// Report buffer: report_id(1) + buttons(2) + hat+pad(1) + axes(4) = 8 bytes
static uint8_t report[9];

// Current input state
static HIDBTInput currentInput = {0};

// ============================================================================
// Report Generation
// ============================================================================

static uint8_t dpad_to_hat(uint8_t dpad) {
    // Convert D-pad bits to hat switch value (0-7, 8=neutral)
    switch (dpad & 0x0F) {
        case 0x01: return 0;  // Up
        case 0x03: return 1;  // Up-Right
        case 0x02: return 2;  // Right
        case 0x06: return 3;  // Down-Right
        case 0x04: return 4;  // Down
        case 0x0C: return 5;  // Down-Left
        case 0x08: return 6;  // Left
        case 0x09: return 7;  // Up-Left
        default:   return 8;  // Neutral (no direction)
    }
}

static void generate_report(void) {
    memset(report, 0, sizeof(report));
    
    // Report ID
    report[0] = 0xA1;  // HID input report header
    report[1] = 0x01;  // Report ID
    
    // Buttons (16 bits) - map GP2040 buttons to generic gamepad
    uint16_t buttons = 0;
    if (currentInput.buttons & HIDBT_GAMEPAD_MASK_B1) buttons |= (1 << 0);   // A
    if (currentInput.buttons & HIDBT_GAMEPAD_MASK_B2) buttons |= (1 << 1);   // B
    if (currentInput.buttons & HIDBT_GAMEPAD_MASK_B3) buttons |= (1 << 2);   // X
    if (currentInput.buttons & HIDBT_GAMEPAD_MASK_B4) buttons |= (1 << 3);   // Y
    if (currentInput.buttons & HIDBT_GAMEPAD_MASK_L1) buttons |= (1 << 4);   // LB
    if (currentInput.buttons & HIDBT_GAMEPAD_MASK_R1) buttons |= (1 << 5);   // RB
    if (currentInput.buttons & HIDBT_GAMEPAD_MASK_L2) buttons |= (1 << 6);   // LT
    if (currentInput.buttons & HIDBT_GAMEPAD_MASK_R2) buttons |= (1 << 7);   // RT
    if (currentInput.buttons & HIDBT_GAMEPAD_MASK_S1) buttons |= (1 << 8);   // Back/Select
    if (currentInput.buttons & HIDBT_GAMEPAD_MASK_S2) buttons |= (1 << 9);   // Start
    if (currentInput.buttons & HIDBT_GAMEPAD_MASK_L3) buttons |= (1 << 10);  // L3
    if (currentInput.buttons & HIDBT_GAMEPAD_MASK_R3) buttons |= (1 << 11);  // R3
    if (currentInput.buttons & HIDBT_GAMEPAD_MASK_A1) buttons |= (1 << 12);  // Home
    if (currentInput.buttons & HIDBT_GAMEPAD_MASK_A2) buttons |= (1 << 13);  // Capture
    
    report[2] = buttons & 0xFF;
    report[3] = (buttons >> 8) & 0xFF;
    
    // Get D-pad mode from input struct
    uint8_t dpadMode = currentInput.dpadMode;
    
    // Hat switch (D-pad) - only in digital mode
    uint8_t hat = 8;  // Neutral by default
    if (dpadMode == HIDBT_DPAD_MODE_DIGITAL) {
        hat = dpad_to_hat(currentInput.dpad);
    }
    report[4] = hat & 0x0F;  // Lower 4 bits = hat, upper 4 bits = padding
    
    // Axes (convert from 16-bit to 8-bit)
    uint8_t lx = currentInput.lx >> 8;
    uint8_t ly = currentInput.ly >> 8;
    uint8_t rx = currentInput.rx >> 8;
    uint8_t ry = currentInput.ry >> 8;
    
    // D-pad as Left Analog mode
    if (dpadMode == HIDBT_DPAD_MODE_LEFT_ANALOG) {
        if (currentInput.dpad & (HIDBT_GAMEPAD_MASK_UP | HIDBT_GAMEPAD_MASK_DOWN |
                                  HIDBT_GAMEPAD_MASK_LEFT | HIDBT_GAMEPAD_MASK_RIGHT)) {
            lx = 128;  // Center
            ly = 128;
            if (currentInput.dpad & HIDBT_GAMEPAD_MASK_LEFT)  lx = 0;
            if (currentInput.dpad & HIDBT_GAMEPAD_MASK_RIGHT) lx = 255;
            if (currentInput.dpad & HIDBT_GAMEPAD_MASK_UP)    ly = 0;
            if (currentInput.dpad & HIDBT_GAMEPAD_MASK_DOWN)  ly = 255;
        }
    }
    
    // D-pad as Right Analog mode
    if (dpadMode == HIDBT_DPAD_MODE_RIGHT_ANALOG) {
        if (currentInput.dpad & (HIDBT_GAMEPAD_MASK_UP | HIDBT_GAMEPAD_MASK_DOWN |
                                  HIDBT_GAMEPAD_MASK_LEFT | HIDBT_GAMEPAD_MASK_RIGHT)) {
            rx = 128;  // Center
            ry = 128;
            if (currentInput.dpad & HIDBT_GAMEPAD_MASK_LEFT)  rx = 0;
            if (currentInput.dpad & HIDBT_GAMEPAD_MASK_RIGHT) rx = 255;
            if (currentInput.dpad & HIDBT_GAMEPAD_MASK_UP)    ry = 0;
            if (currentInput.dpad & HIDBT_GAMEPAD_MASK_DOWN)  ry = 255;
        }
    }
    
    report[5] = lx;
    report[6] = ly;
    report[7] = rx;
    report[8] = ry;
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
                        if (has_host_addr) {
                            reconnect_pending = true;
                            connectionState = HIDBTState::RECONNECTING;
                        } else {
                            connectionState = HIDBTState::DISCOVERABLE;
                        }
                        return;
                    }
                    hid_cid = hid_subevent_connection_opened_get_hid_cid(packet);
                    connectionState = HIDBTState::CONNECTED;
                    reconnect_pending = false;

                    bd_addr_t connected_addr;
                    hid_subevent_connection_opened_get_bd_addr(packet, connected_addr);

                    // Always save pairing - PC needs our MAC to reconnect
                    memcpy(host_addr, connected_addr, 6);
                    has_host_addr = true;
                    // Set flag for save from main loop (safer than alarm)
                    memcpy(flash_save_addr, connected_addr, 6);
                    flash_save_pending = true;

                    gpio_put(EXTERNAL_LED_PIN, 1);
                    hid_device_request_can_send_now_event(hid_cid);
                    break;
                }
                case HID_SUBEVENT_CONNECTION_CLOSED:
                    hid_cid = 0;
                    gpio_put(EXTERNAL_LED_PIN, 0);

                    if (has_host_addr) {
                        reconnect_pending = true;
                        reconnect_start_time = to_ms_since_boot(get_absolute_time());
                        connectionState = HIDBTState::RECONNECTING;
                    } else {
                        connectionState = HIDBTState::DISCOVERABLE;
                    }
                    break;
                case HID_SUBEVENT_CAN_SEND_NOW:
                    if (hid_cid != 0) {
                        generate_report();
                        hid_device_send_interrupt_message(hid_cid, report, sizeof(report));
                        hid_device_request_can_send_now_event(hid_cid);
                    }
                    break;
            }
            break;
    }
}

// ============================================================================
// BTstack Setup
// ============================================================================

// Forward declaration for btstack chipset
extern "C" {
    void chipset_set_bd_addr_command(bd_addr_t addr, uint8_t *hci_cmd_buffer);
    const btstack_chipset_t * btstack_chipset_cyw43_instance(void);
}

static void setup_btstack(void) {
    l2cap_init();

    gap_discoverable_control(1);
    gap_set_class_of_device(0x2508);  // Gamepad
    gap_set_local_name("GP2040 Gamepad");
    gap_set_default_link_policy_settings(LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE);
    gap_set_allow_role_switch(true);

    hci_set_chipset(btstack_chipset_cyw43_instance());
    hci_set_bd_addr(bt_addr);

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
        .hid_descriptor = hidbt_report_descriptor,
        .hid_descriptor_size = sizeof(hidbt_report_descriptor),
        .device_name = "GP2040 Gamepad"
    };

    memset(hid_service_buffer, 0, sizeof(hid_service_buffer));
    hid_create_sdp_record(hid_service_buffer, sdp_create_service_record_handle(), &hid_sdp_record);
    sdp_register_service(hid_service_buffer);

    // Create PnP SDP record (Generic gamepad)
    memset(pnp_service_buffer, 0, sizeof(pnp_service_buffer));
    device_id_create_sdp_record(pnp_service_buffer, sdp_create_service_record_handle(),
                                 DEVICE_ID_VENDOR_ID_SOURCE_USB, 0x1209, 0x2040, 0x0001);
    sdp_register_service(pnp_service_buffer);

    hid_device_init(1, sizeof(hidbt_report_descriptor), hidbt_report_descriptor);

    static btstack_packet_callback_registration_t hci_event_callback_registration;
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    hid_device_register_packet_handler(&packet_handler);

    hci_power_control(HCI_POWER_ON);

    for (int i = 0; i < 100; i++) {
        cyw43_arch_poll();
        sleep_ms(10);
    }

    btInitialized = true;
}

// ============================================================================
// Public API
// ============================================================================

void hidbt_init(void) {
    connectionState = HIDBTState::INITIALIZING;

    // Initialize external LED GPIO
    gpio_init(EXTERNAL_LED_PIN);
    gpio_set_dir(EXTERNAL_LED_PIN, GPIO_OUT);
    gpio_put(EXTERNAL_LED_PIN, 0);

    // Generate MAC from Pico's unique ID - ALWAYS the same for this device
    // This ensures PC can always reconnect since our MAC never changes
    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    
    // Use different bytes than Switch BT to get a different MAC for HID mode
    bt_addr[0] = 0x00;  // Generic prefix
    bt_addr[1] = 0x1A;
    bt_addr[2] = 0x7D;
    bt_addr[3] = board_id.id[5];  // Use different bytes than Switch BT
    bt_addr[4] = board_id.id[6];
    bt_addr[5] = board_id.id[7];

    // Load any saved host pairing
    load_paired_host();

    if (cyw43_arch_init()) {
        while (1) {
            gpio_put(EXTERNAL_LED_PIN, 1);
            sleep_ms(100);
            gpio_put(EXTERNAL_LED_PIN, 0);
            sleep_ms(100);
        }
    }

    setup_btstack();

    if (has_host_addr) {
        connectionState = HIDBTState::RECONNECTING;
        reconnect_pending = true;
        reconnect_start_time = to_ms_since_boot(get_absolute_time());
    } else {
        connectionState = HIDBTState::DISCOVERABLE;
    }

    // Double blink = init complete
    gpio_put(EXTERNAL_LED_PIN, 1);
    sleep_ms(100);
    gpio_put(EXTERNAL_LED_PIN, 0);
    sleep_ms(100);
    gpio_put(EXTERNAL_LED_PIN, 1);
    sleep_ms(100);
    gpio_put(EXTERNAL_LED_PIN, 0);
}

static void reinit_bluetooth(void) {
    if (cyw43_arch_init()) {
        btInitialized = false;
        return;
    }

    setup_btstack();
    hid_cid = 0;

    gpio_put(EXTERNAL_LED_PIN, 1);
    sleep_ms(100);
    gpio_put(EXTERNAL_LED_PIN, 0);
}

bool hidbt_process(const HIDBTInput* input) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    // Handle deferred flash save (must be done from main loop, not interrupt)
    if (flash_save_pending) {
        flash_save_pending = false;
        save_pairing_to_flash(flash_save_addr);
    }

    if (input) {
        memcpy(&currentInput, input, sizeof(HIDBTInput));
    }

    bool anyButtonPressed = input && (input->buttons != 0 || input->dpad != 0);

    // Handle SLEEPING state
    if (connectionState == HIDBTState::SLEEPING) {
        static uint32_t lastSleepFlash = 0;
        if (now - lastSleepFlash > 10000) {
            gpio_put(EXTERNAL_LED_PIN, 1);
            sleep_ms(50);
            gpio_put(EXTERNAL_LED_PIN, 0);
            lastSleepFlash = now;
        }

        if (anyButtonPressed && has_host_addr) {
            gpio_put(EXTERNAL_LED_PIN, 1);
            wake_from_deep_sleep();
            connectionState = HIDBTState::RECONNECTING;
            reconnect_pending = true;
            reconnect_start_time = now;
        }

        sleep_ms(10);
        return false;
    }

    if (!btInitialized) return false;

    for (int i = 0; i < 10; i++) {
        cyw43_arch_poll();
    }

    static uint32_t lastReconnectAttempt = 0;

    if (connectionState == HIDBTState::RECONNECTING && reconnect_pending && has_host_addr && hid_cid == 0) {
        if (reconnect_start_time > 0 && (now - reconnect_start_time > RECONNECT_TIMEOUT_MS)) {
            // For PC/generic HID, stay discoverable instead of sleeping
            // PCs often reconnect on their own schedule, not immediately
            reconnect_pending = false;
            connectionState = HIDBTState::DISCOVERABLE;
            return false;
        }

        if (now - lastReconnectAttempt > 1000) {
            lastReconnectAttempt = now;
            hid_device_connect(host_addr, &hid_cid);
        }
    }

    static uint32_t lastBlink = 0;
    static bool ledOn = false;

    if (connectionState == HIDBTState::RECONNECTING) {
        if (now - lastBlink > 150) {
            ledOn = !ledOn;
            gpio_put(EXTERNAL_LED_PIN, ledOn);
            lastBlink = now;
        }
        return false;
    }

    if (connectionState == HIDBTState::DISCOVERABLE) {
        if (now - lastBlink > 500) {
            ledOn = !ledOn;
            gpio_put(EXTERNAL_LED_PIN, ledOn);
            lastBlink = now;
        }
        return false;
    }

    if (connectionState == HIDBTState::CONNECTED) {
        gpio_put(EXTERNAL_LED_PIN, 1);
        return true;
    }

    return false;
}

void hidbt_clear_pairing(void) {
    has_host_addr = false;
    reconnect_pending = false;
    memset(host_addr, 0, 6);

    if (flash_write_alarm != 0) {
        cancel_alarm(flash_write_alarm);
        flash_write_alarm = 0;
    }

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(HIDBT_PAIRING_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);
}

HIDBTState hidbt_get_state(void) {
    return connectionState;
}

bool hidbt_is_connected(void) {
    return connectionState == HIDBTState::CONNECTED;
}


