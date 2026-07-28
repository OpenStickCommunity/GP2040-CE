#ifndef _MAYFLASHS5AUTH_H_
#define _MAYFLASHS5AUTH_H_

#include "drivers/shared/gpauthdriver.h"

typedef enum {
    ps5_auth_idle = 0,
    ps5_auth_send_f0_from_console,
    //ps5_auth_send_f0_final_from_console,
    ps5_auth_recv_f1_from_dongle,
    ps5_auth_recv_f2_delay_500mS,
    ps5_auth_recv_f2_signing_state,
    ps5_set_bluetooth_mode,
    ps5_set_test_command,
} PS5AuthState; // PS5 Auth State for F0/F1/F2 auth

typedef enum {
    PS5_AUTH_NOT_READY = 0x11,
    PS5_AUTH_READY = 0x12,
    PS5_AUTH_DONE = 0x40,
    PS5_AUTH_REFRESH_NOT_READY = 0x02,
    PS5_AUTH_REFRESH_READY = 0x52,
} PS5AuthResponse;

#define PS5_AUTH_DATALEN 56

// Console feature writes are forwarded to the S5 verbatim. The console emits
// 0x0A, 0x08 and 0x80 within a few milliseconds of each other, so a single
// pending slot would drop two of the three.
#define PS5_SET_QUEUE_DEPTH 8

typedef struct {
    uint8_t  report_id;
    uint16_t len;            // including the report id byte
    uint8_t  data[64];
} PS5PendingSet;

// PS5 Auth Buffer from Console
typedef struct {
    const uint8_t auth_type;
    const uint8_t frame_id;
    const uint8_t auth_index;
    const uint8_t auth_data[PS5_AUTH_DATALEN];
    const uint8_t auth_crc32[4];
} ConsolePS5AuthBuffer;

typedef struct PS5AuthData {
    uint8_t hash_pending_buffer[64];
    uint8_t hash_finish_buffer[64];
    //uint8_t auth_buffer[64];
    //uint8_t console_buffer[64];
    uint8_t auth_f1_buffer[224]; // 56 * 4
    uint8_t auth_f1_raw[4][64];
    uint8_t auth_f2_raw[16];
    bool auth_f2_valid;
    uint8_t auth_f1_get_index; // index for getting f1
    uint8_t console_f0_buffer[224]; // 56 * 4
    uint8_t auth_f0_raw[4][64];
    uint8_t console_f0_get_index; // index for getting f0
    uint8_t console_f0_recv_count; // index for last received f0 from buffer (stagger)
    uint8_t console_f0_type; // 0x01 or 0x02 (final chunk)
    //uint8_t status_buffer[15]; // for F2 status
    uint8_t auth_frame_id; // PS5 indicates which auth we are on
    bool auth_f1_ready; // is the F1 ready from the dongle?
    bool auth_f1_done; // are we all done for the final F2 response?
    uint8_t send_hid_buffer[64];
    uint8_t mayflash_buffer[64];
    uint64_t auth_recv_f2_us;
    bool dongle_mounted;
    bool dongle_ready;
    bool pair_info_ready;
    uint8_t init_stage;
    bool hash_pending;
    bool hash_ready;
    uint32_t hash_request_count;
    uint32_t hash_response_count;
    uint32_t console_report_count;
    uint32_t auth_console_f0_raw_count;
    uint32_t auth_console_f0_count;
    uint32_t auth_console_f0_crc_valid_count;
    uint32_t auth_console_f0_submit_count;
    uint32_t auth_console_f0_complete_count;
    uint8_t auth_console_f0_accept_mask;
    uint8_t auth_console_f0_submit_mask;
    uint8_t auth_console_f0_last_type;
    uint32_t auth_dongle_f1_count;
    uint32_t auth_dongle_f1_crc_valid_count;
    uint32_t auth_dongle_f1_meta_valid_count;
    uint32_t auth_console_f1_count;
    uint32_t auth_console_f2_count;
    uint8_t auth_dongle_f2_status;
    uint8_t proxy_get_request;
    uint16_t proxy_get_len;      // length the console asked for
    bool proxy_get_stalled;      // the dongle refused it; pass that on
    bool proxy_get_pending;
    bool proxy_get_inflight;
    bool proxy_response_ready;
    uint16_t proxy_response_len;
    uint8_t proxy_response[64];
    PS5AuthState ps5_auth_state;
    uint8_t MAC_pair_report[15];     // 16-byte pair information for BT MAC Address
    uint8_t calibration_report[40];  // calibration info from S5
    uint8_t firmware_report[63];     // firmware reported by Mayflash S5 dongle
    bool S5_reports_ready; // did we get all reports back from the S5?
    uint8_t set_bluetooth_mode;
    uint8_t set_testcommand[2];
    PS5PendingSet set_queue[PS5_SET_QUEUE_DEPTH];
    uint8_t set_queue_head;
    uint8_t set_queue_tail;
    uint32_t set_queue_dropped;
} PS5AuthData;

class MayflashS5Auth : public GPAuthDriver {
public:
    virtual void initialize();
    virtual bool available();
    void process();
    PS5AuthData * getAuthData() { return &ps5AuthData; }
private:
    PS5AuthData ps5AuthData;
};

#endif
