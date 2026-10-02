#include "host/usbh.h"
#include "class/hid/hid.h"
#include "class/hid/hid_host.h"
#include "drivers/ps4/PS4AuthUSBListener.h"
#include "CRC32.h"
#include "peripheralmanager.h"
#include "usbhostmanager.h"
#include "pico/time.h"

static const uint8_t output_0xf3[] = { 0x0, 0x38, 0x38, 0, 0, 0, 0 };

// A control transfer that has not completed within this window is treated as
// lost (failed completions with len == 0 are dropped by USBHostManager, and a
// dongle can reset mid-sequence). Normal transfers finish in a few ms.
static const uint32_t PS4_AUTH_CB_TIMEOUT_MS = 500;

// After this many consecutive timeouts, stop retrying the current nonce and
// wait for the console to start a new round, instead of re-sending to a
// wedged dongle (and holding the shared host control slot) indefinitely.
static const uint8_t PS4_AUTH_MAX_TIMEOUT_RETRIES = 3;

// After this many consecutive signing errors from the bound dongle, try the
// next candidate dongle (one error may be transient, e.g. a bad CRC).
static const uint8_t PS4_AUTH_MAX_SIGNING_ERRORS = 2;

// Delay between PS4_GET_SIGNING_STATE polls while the dongle reports
// "not ready", so polling does not monopolise the shared control slot.
static const uint32_t PS4_SIGNING_POLL_INTERVAL_MS = 10;

// Signing-state byte returned by the dongle in PS4_GET_SIGNING_STATE
static const uint8_t PS4_SIGNING_READY = 0;
static const uint8_t PS4_SIGNING_ERROR = 1;

// Every HID interface on the bus could be a PS4-family dongle interface.
static_assert(PS4_MAX_DONGLE_CANDIDATES >= CFG_TUH_HID,
              "PS4_MAX_DONGLE_CANDIDATES must cover every host HID interface");

static inline uint32_t now_ms() {
    return to_ms_since_boot(get_absolute_time());
}

void PS4AuthUSBListener::setup() {
    ps_dev_addr = 0xFF;
    ps_instance = 0xFF;
    ps4AuthData = nullptr;
    num_candidates = 0;
    timeout_retries = 0;
    signing_errors = 0;
    need_definition = false;
    rotations = 0;
    rotation_nonce_id = 0;
    resetHostData();
}

void PS4AuthUSBListener::process() {
    if ( ps4AuthData == nullptr )
        return;

    if ( awaiting_cb == true ) {
        if ( (now_ms() - awaiting_since_ms) < PS4_AUTH_CB_TIMEOUT_MS )
            return;
        // Completion never arrived: restart the dongle exchange. If the console
        // is still waiting on this nonce, no_nonce re-sends it from page 0.
        // Once the signature is being read back (sending_nonce), the nonce in
        // ps4_auth_buffer has been partly overwritten and cannot be re-sent,
        // so wait for the console's next round instead.
        // Must be read before resetHostData(), which sets dongle_state to no_nonce.
        bool nonce_intact = (dongle_state != PS4State::sending_nonce);
        uint8_t retries = timeout_retries + 1;
        resetHostData();
        if ( retries >= PS4_AUTH_MAX_TIMEOUT_RETRIES ) {
            retries = 0;
            dongleFailed(nonce_intact); // may bind another dongle
        } else if ( !nonce_intact ) {
            ps4AuthData->passthrough_state = GPAuthState::auth_idle_state;
        }
        timeout_retries = retries;
    }

    // PS4_DEFINITION is requested once per newly bound dongle. It is sent from
    // here rather than from bind() because bind() can run inside unmount(),
    // while tinyusb still holds the control slot for the removed device.
    if ( need_definition ) {
        memset(report_buffer, 0, PS4_ENDPOINT_SIZE);
        report_buffer[0] = PS4AuthReport::PS4_DEFINITION;
        if ( host_get_report(PS4AuthReport::PS4_DEFINITION, report_buffer, 48) ) {
            need_definition = false;
        }
        return; // one control transfer per pass
    }

    switch ( dongle_state ) {
        case PS4State::no_nonce:
            // Once Console is ready with the nonce, begin!
            if ( ps4AuthData->passthrough_state == GPAuthState::send_auth_console_to_dongle ) {
                memcpy(report_buffer, output_0xf3, sizeof(output_0xf3));
                host_get_report(PS4AuthReport::PS4_RESET_AUTH, report_buffer, sizeof(output_0xf3) + 1);
            }
            break;
        case PS4State::receiving_nonce:
            report_buffer[0] = PS4AuthReport::PS4_SET_AUTH_PAYLOAD; // [0xF0, ID, Page, 0, nonce(54 or 32 with 0 padding), CRC32 of data]
            report_buffer[1] = ps4AuthData->nonce_id;
            report_buffer[2] = nonce_page;
            report_buffer[3] = 0;
            if ( nonce_page == 4 ) {
                noncelen = 32; // from 4 to 64 - 24 - 4
                memcpy(&report_buffer[4], &ps4AuthData->ps4_auth_buffer[nonce_page*56], noncelen);
                memset(&report_buffer[4+noncelen], 0, 24); // zero padding  
            } else {
                noncelen = 56;
                memcpy(&report_buffer[4], &ps4AuthData->ps4_auth_buffer[nonce_page*56], noncelen);
            }
            crc32 = CRC32::calculate(report_buffer, 60);
            memcpy(&report_buffer[60], &crc32, sizeof(uint32_t));
            // Only move to the next page once this one is actually queued;
            // otherwise the same page is retried on the next pass.
            if ( host_set_report(PS4AuthReport::PS4_SET_AUTH_PAYLOAD, report_buffer, 64) ) {
                nonce_page++;
            }
            break;
        case PS4State::signed_nonce_ready:
            if ( (int32_t)(now_ms() - poll_after_ms) < 0 )
                break; // dongle said "not ready"; wait before polling again
            report_buffer[0] = PS4AuthReport::PS4_GET_SIGNING_STATE;
            report_buffer[1] = ps4AuthData->nonce_id;
            memset(&report_buffer[2], 0, 14);
            host_get_report(PS4AuthReport::PS4_GET_SIGNING_STATE, report_buffer, 16);
            break;
        case PS4State::sending_nonce:
            report_buffer[0] = PS4AuthReport::PS4_GET_SIGNATURE_NONCE;
            report_buffer[1] = ps4AuthData->nonce_id;    // nonce_id
            report_buffer[2] = nonce_chunk; // next_part
            memset(&report_buffer[3], 0, 61); // zero rest of memory
            // Nonce Part is reset during callback; advance only once queued.
            if ( host_get_report(PS4AuthReport::PS4_GET_SIGNATURE_NONCE, report_buffer, 64) ) {
                nonce_chunk++;
            }
            break;
        default:
            break;
    };
}

void PS4AuthUSBListener::resetHostData() {
    nonce_page = 0; // no nonce yet
    nonce_chunk = 0; // which part of the nonce are we getting from send?
    awaiting_cb = false;
    awaiting_since_ms = 0;
    poll_after_ms = 0;
    dongle_state = PS4State::no_nonce;
}

// tuh_hid_*_report() returns false when the transfer could not be queued
// (the host's single control slot is busy, or the device is gone). Only wait
// for a completion if one was actually requested; otherwise process() retries.
bool PS4AuthUSBListener::host_get_report(uint8_t report_id, void* report, uint16_t len) {
    awaiting_cb = tuh_hid_get_report(ps_dev_addr, ps_instance, report_id, HID_REPORT_TYPE_FEATURE, report, len);
    if ( awaiting_cb )
        awaiting_since_ms = now_ms();
    return awaiting_cb;
}

bool PS4AuthUSBListener::host_set_report(uint8_t report_id, void* report, uint16_t len) {
    awaiting_cb = tuh_hid_set_report(ps_dev_addr, ps_instance, report_id, HID_REPORT_TYPE_FEATURE, report, len);
    if ( awaiting_cb )
        awaiting_since_ms = now_ms();
    return awaiting_cb;
}

void PS4AuthUSBListener::bind(uint8_t dev_addr, uint8_t instance) {
    ps_dev_addr = dev_addr;
    ps_instance = instance;
    timeout_retries = 0;
    signing_errors = 0;
    resetHostData();
    ps4AuthData->dongle_ready = true;

    // Request PS4_DEFINITION as soon as it is connected; sent by process().
    need_definition = true;
}

// Move the bound device's interfaces to the back of the candidate list and
// bind the next device. Returns false if there is no other device to try.
// Number of distinct candidate devices other than the bound one.
uint8_t PS4AuthUSBListener::otherCandidateDevices() const {
    uint8_t count = 0;
    for (uint8_t i = 0; i < num_candidates; i++) {
        if ( candidates[i].dev_addr == ps_dev_addr )
            continue;
        bool seen = false;
        for (uint8_t j = 0; j < i; j++) {
            if ( candidates[j].dev_addr == candidates[i].dev_addr ) {
                seen = true;
                break;
            }
        }
        if ( !seen )
            count++;
    }
    return count;
}

bool PS4AuthUSBListener::rotateCandidate() {
    uint8_t bound = ps_dev_addr;
    if ( otherCandidateDevices() == 0 )
        return false;

    // Stable partition: other devices first, then the bound device's interfaces.
    struct { uint8_t dev_addr; uint8_t instance; } reordered[PS4_MAX_DONGLE_CANDIDATES];
    uint8_t n = 0;
    for (uint8_t i = 0; i < num_candidates; i++) {
        if ( candidates[i].dev_addr != bound ) {
            reordered[n].dev_addr = candidates[i].dev_addr;
            reordered[n].instance = candidates[i].instance;
            n++;
        }
    }
    for (uint8_t i = 0; i < num_candidates; i++) {
        if ( candidates[i].dev_addr == bound ) {
            reordered[n].dev_addr = candidates[i].dev_addr;
            reordered[n].instance = candidates[i].instance;
            n++;
        }
    }
    for (uint8_t i = 0; i < n; i++) {
        candidates[i].dev_addr = reordered[i].dev_addr;
        candidates[i].instance = reordered[i].instance;
    }

    bind(candidates[0].dev_addr, candidates[0].instance);
    return true;
}

// The bound dongle repeatedly failed (timeouts or signing errors). Try another
// connected dongle; if the console's nonce is still intact, the new dongle
// picks it up immediately, otherwise wait for the console's next round.
//
// This only catches failures the dongle reports. A dongle that signs
// successfully but is rejected by the console (e.g. a PS4-only dongle in PS5
// mode) is not detected here.
void PS4AuthUSBListener::dongleFailed(bool nonce_intact) {
    // Try each other device at most once per console nonce, so two dead
    // dongles cannot be rotated between forever. A new nonce from the console
    // starts a fresh pass.
    if ( ps4AuthData->nonce_id != rotation_nonce_id ) {
        rotation_nonce_id = ps4AuthData->nonce_id;
        rotations = 0;
    }
    bool rotated = false;
    if ( rotations < otherCandidateDevices() ) {
        rotated = rotateCandidate();
        if ( rotated )
            rotations++;
    }
    if ( !rotated || !nonce_intact ) {
        ps4AuthData->passthrough_state = GPAuthState::auth_idle_state;
    }
}

void PS4AuthUSBListener::mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    // Only a PS4 interface has vendor IDs F0, F1, F2, and F3. A full DS4-style
    // descriptor lists the vendor reports after many input/output reports, so
    // parse enough entries to reach them.
    static const uint8_t MAX_REPORT_INFO = 32;
    tuh_hid_report_info_t report_info[MAX_REPORT_INFO];
    uint8_t report_count = tuh_hid_parse_report_descriptor(report_info, MAX_REPORT_INFO, desc_report, desc_len);
    bool isPS4Dongle = false;
    for(uint8_t i = 0; i < report_count; i++) {
        if ( report_info[i].usage_page == 0xFFF0 && 
                (report_info[i].report_id == 0xF3) ) {
            isPS4Dongle = true;
            break;
        }
    }
    if (isPS4Dongle == false )
        return;

    // Remember every PS4-family dongle interface, not just the first, so the
    // listener can move to another one if the bound dongle is unplugged.
    bool known = false;
    for (uint8_t i = 0; i < num_candidates; i++) {
        if ( candidates[i].dev_addr == dev_addr && candidates[i].instance == instance ) {
            known = true;
            break;
        }
    }
    if ( !known && num_candidates < PS4_MAX_DONGLE_CANDIDATES ) {
        candidates[num_candidates].dev_addr = dev_addr;
        candidates[num_candidates].instance = instance;
        num_candidates++;
    }

    // Bind the first dongle seen (also prevents Magic-X double mount: its
    // second interface is only kept as a candidate).
    if ( ps4AuthData->dongle_ready == false ) {
        bind(dev_addr, instance);
    }
}

void PS4AuthUSBListener::unmount(uint8_t dev_addr) {
    // Drop every candidate interface on this device.
    uint8_t kept = 0;
    for (uint8_t i = 0; i < num_candidates; i++) {
        if ( candidates[i].dev_addr != dev_addr ) {
            candidates[kept++] = candidates[i];
        }
    }
    num_candidates = kept;

    if ( ps4AuthData->dongle_ready == false ||
        (dev_addr != ps_dev_addr) ) {
        return;
    }

    ps_dev_addr = 0xFF;
    ps_instance = 0xFF;
    need_definition = false;
    resetHostData();
    ps4AuthData->dongle_ready = false;

    // Fall back to another dongle that is still plugged in. Without this the
    // remaining dongles are never bound, because their mount() already ran.
    if ( num_candidates > 0 ) {
        bind(candidates[0].dev_addr, candidates[0].instance);
    }
}

void PS4AuthUSBListener::set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    // Ignore completions we are not waiting for (e.g. one that arrives after
    // the timeout in process() already restarted the exchange).
    if ( ps4AuthData->dongle_ready == false || awaiting_cb == false ||
        (dev_addr != ps_dev_addr) || (instance != ps_instance) ) {
        return;
    }

    timeout_retries = 0; // dongle is responding

    switch(report_id) {
        case PS4AuthReport::PS4_SET_AUTH_PAYLOAD:
            if (nonce_page == 5) {
                nonce_page = 0;
                dongle_state = PS4State::signed_nonce_ready;
            }
            break;
        default:
            break;
    };
    awaiting_cb = false;
}

void PS4AuthUSBListener::get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    // Ignore completions we are not waiting for (see set_report_complete).
    if ( ps4AuthData->dongle_ready == false || awaiting_cb == false ||
        (dev_addr != ps_dev_addr) || (instance != ps_instance) ) {
        return;
    }
    
    timeout_retries = 0; // dongle is responding

    switch(report_id) {
        case PS4AuthReport::PS4_DEFINITION:
            break;
        case PS4AuthReport::PS4_RESET_AUTH:
            nonce_page = 0;
            nonce_chunk = 0;
            dongle_state = PS4State::receiving_nonce;
            break;
        case PS4AuthReport::PS4_GET_SIGNING_STATE:
            // report_buffer[2]: 0 = ready, 1 = error in signing, 16 = not ready (keep polling)
            if (report_buffer[2] == PS4_SIGNING_READY) {
                signing_errors = 0;
                dongle_state = PS4State::sending_nonce;
            } else if (report_buffer[2] == PS4_SIGNING_ERROR) {
                // Abandon this nonce: re-sending it to the same dongle would
                // just fail again. After repeated errors try another dongle,
                // which can take the (still intact) nonce straight away.
                resetHostData();
                if ( ++signing_errors >= PS4_AUTH_MAX_SIGNING_ERRORS ) {
                    signing_errors = 0;
                    dongleFailed(true);
                } else {
                    ps4AuthData->passthrough_state = GPAuthState::auth_idle_state;
                }
                // Must return immediately: dongleFailed() may have bound a
                // different dongle, so ps_dev_addr / state no longer refer to
                // the device this completion came from.
                return;
            } else {
                poll_after_ms = now_ms() + PS4_SIGNING_POLL_INTERVAL_MS; // not ready yet
            }
            break;
        case PS4AuthReport::PS4_GET_SIGNATURE_NONCE:
            // probably should mutex lock
            if (nonce_chunk == 0 || nonce_chunk > 19) {
                break; // out-of-sequence completion; never index before the buffer
            }
            memcpy(&ps4AuthData->ps4_auth_buffer[(nonce_chunk-1)*56], &report_buffer[4], 56);
            if (nonce_chunk == 19) {
                nonce_chunk = 0;
                dongle_state = PS4State::no_nonce; // something we don't support
                ps4AuthData->passthrough_state = GPAuthState::send_auth_dongle_to_console;
            }
            break;
        default:
            break;
    };
    awaiting_cb = false;
}
