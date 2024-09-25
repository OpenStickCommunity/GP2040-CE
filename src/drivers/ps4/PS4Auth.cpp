#include "host/usbh.h"
#include "class/hid/hid.h"
#include "class/hid/hid_host.h"
#include "drivers/ps4/PS4Auth.h"
#include "drivers/ps4/PS4AuthUSBListener.h"
#include "CRC32.h"
#include "peripheralmanager.h"
#include "storagemanager.h"
#include "usbhostmanager.h"

#include "enums.pb.h"

#include "mbedtls/error.h"
#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"

#define NEW_CONFIG_MPI(name, buf, size) \
    mbedtls_mpi_uint *bytes ## name = new mbedtls_mpi_uint[size / sizeof(mbedtls_mpi_uint)]; \
    mbedtls_mpi name = { .s=1, .n=size / sizeof(mbedtls_mpi_uint), .p=bytes ## name }; \
    memcpy(bytes ## name, buf, size);

#define DELETE_CONFIG_MPI(name) delete bytes ## name;

static inline int rng(void*p_rng, unsigned char* p, size_t len) {
    (void) p_rng;
    p[0] = rand();
    return 0;
}

void PS4Auth::initialize() {
    if ( !available() ) {
        return;
    }

    listener = nullptr;
    ps4AuthData.dongle_ready = false;
    ps4AuthData.nonce_id = 0;
    ps4AuthData.passthrough_state = GPAuthState::auth_idle_state;
    memset(ps4AuthData.ps4_auth_buffer, 0, 1064);
    if (authType == InputModeAuthType::INPUT_MODE_AUTH_TYPE_KEYS ) {
        keyModeInitialize();
    } else if (authType == InputModeAuthType::INPUT_MODE_AUTH_TYPE_USB ) {
        listener = new PS4AuthUSBListener();
        ((PS4AuthUSBListener*)listener)->setup();
        ((PS4AuthUSBListener*)listener)->setAuthData(&ps4AuthData);
    }
}

bool PS4Auth::available() {
     // Move options over to their own ps4 data structure or gamepad?
    if ( authType == InputModeAuthType::INPUT_MODE_AUTH_TYPE_KEYS ) {
        const PS4Options& options = Storage::getInstance().getAddonOptions().ps4Options;
        return options.serial.size == sizeof(options.serial.bytes)
            && options.signature.size == sizeof(options.signature.bytes)
            && options.rsaN.size == sizeof(options.rsaN.bytes)
            && options.rsaE.size == sizeof(options.rsaE.bytes)
            && options.rsaP.size == sizeof(options.rsaP.bytes)
            && options.rsaQ.size == sizeof(options.rsaQ.bytes);
    } else if ( authType == InputModeAuthType::INPUT_MODE_AUTH_TYPE_USB ) {
        return (PeripheralManager::getInstance().isUSBEnabled(0));
    }
    return false;
}

void PS4Auth::process() {
    if (authType == InputModeAuthType::INPUT_MODE_AUTH_TYPE_KEYS ) {
        keyModeProcess();
    } else if (authType == InputModeAuthType::INPUT_MODE_AUTH_TYPE_USB ) {
        ((PS4AuthUSBListener*)listener)->process(); 	// process HOST with client data
    }
}

// Init if we're in ps4 key mode
void PS4Auth::keyModeInitialize() {
    ps4AuthData.valid_rsa = false;
    const PS4Options& options = Storage::getInstance().getAddonOptions().ps4Options;
    NEW_CONFIG_MPI(N, options.rsaN.bytes, options.rsaN.size)
    NEW_CONFIG_MPI(E, options.rsaE.bytes, options.rsaE.size)
    NEW_CONFIG_MPI(P, options.rsaP.bytes, options.rsaP.size)
    NEW_CONFIG_MPI(Q, options.rsaQ.bytes, options.rsaQ.size)
    mbedtls_rsa_init(&ps4AuthData.rsa_context, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
    if (mbedtls_rsa_import(&ps4AuthData.rsa_context, &N, &P, &Q, nullptr, &E) == 0 &&
            mbedtls_rsa_complete(&ps4AuthData.rsa_context) == 0) {
        ps4AuthData.valid_rsa = true;
    }
    DELETE_CONFIG_MPI(N)
    DELETE_CONFIG_MPI(E)
    DELETE_CONFIG_MPI(P)
    DELETE_CONFIG_MPI(Q)

    // Reset our random seed
    srand(0);
}

// Process if we are using ps4 keys
void PS4Auth::keyModeProcess() {
    // Do not run if RSA is invalid
    if (!ps4AuthData.valid_rsa) {
        return;
    }

    // Check to see if the PS4 Authentication needs work
    if ( ps4AuthData.passthrough_state == GPAuthState::send_auth_console_to_dongle ) {
        const PS4Options& options = Storage::getInstance().getAddonOptions().ps4Options;
        int rss_error = 0;
        uint8_t hashed_nonce[32];
        // Sign our nonce into hashed_nonce
        if ( mbedtls_sha256_ret(ps4AuthData.ps4_auth_buffer, 256, hashed_nonce, 0) < 0 ) {
            return;
        }
        rss_error = mbedtls_rsa_rsassa_pss_sign(&ps4AuthData.rsa_context, rng, nullptr,
                MBEDTLS_RSA_PRIVATE, MBEDTLS_MD_SHA256,
                32, hashed_nonce,
                ps4AuthData.ps4_auth_buffer);
        if ( rss_error < 0 ) {
            return; // If we could not sign with our key, return (error)
        }
        // copy the parts into our authentication buffer
        size_t offset = 256;
        memcpy(&ps4AuthData.ps4_auth_buffer[offset], options.serial.bytes, 16);
        offset += 16;
        mbedtls_rsa_export_raw(
            &ps4AuthData.rsa_context,
            &ps4AuthData.ps4_auth_buffer[offset], 256,
            nullptr, 0,
            nullptr, 0,
            nullptr, 0,
            &ps4AuthData.ps4_auth_buffer[offset+256], 256
        );
        offset += 512;
        memcpy(&ps4AuthData.ps4_auth_buffer[offset], options.signature.bytes, 256);
        offset += 256;
        memset(&ps4AuthData.ps4_auth_buffer[offset], 0, 24);
        ps4AuthData.passthrough_state = GPAuthState::send_auth_dongle_to_console;
    }
}

void PS4Auth::resetAuth() {
    if (authType == InputModeAuthType::INPUT_MODE_AUTH_TYPE_USB ) {
        ((PS4AuthUSBListener*)listener)->resetHostData();
    }
    ps4AuthData.passthrough_state = GPAuthState::auth_idle_state;
}
