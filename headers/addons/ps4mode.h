#ifndef PS4MODE_H_
#define PS4MODE_H_

#include "gpaddon.h"
#include "storagemanager.h"

#ifndef PS4MODE_ADDON_ENABLED
#define PS4MODE_ADDON_ENABLED 0
#endif

// Turbo Module Name
#define PS4ModeName "PS4Mode"

class PS4ModeAddon : public GPAddon {
public:
    virtual bool available();
	virtual void setup();       // TURBO Button Setup
    virtual void preprocess() {}
	virtual void process();     // TURBO Setting of buttons (Enable/Disable)
    virtual std::string name() { return PS4ModeName; }
private:
	struct mbedtls_rsa_context rsa_context;
    mbedtls_mpi_uint bytesN[64] = {};
    mbedtls_mpi_uint bytesE[1] = {};
    mbedtls_mpi_uint bytesD[64] = {};
    mbedtls_mpi_uint bytesP[32] = {};
    mbedtls_mpi_uint bytesQ[32] = {};
    mbedtls_mpi_uint bytesDP[32] = {};
    mbedtls_mpi_uint bytesDQ[32] = {};
    mbedtls_mpi_uint bytesQP[32] = {};
    mbedtls_mpi_uint bytesRN[64] = {};
    uint8_t hashed_nonce[32] = {};
    ConfigLegacy::PS4Options ps4Options;
};

#endif  // PS4MODE_H_
