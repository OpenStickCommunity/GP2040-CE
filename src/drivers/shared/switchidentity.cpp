#include "drivers/shared/switchidentity.h"
#include "storagemanager.h"
#include "pico/rand.h"

static constexpr uint64_t FNV_OFFSET_BASIS = 0xCBF29CE484222325ull;
static constexpr uint64_t FNV_PRIME = 0x100000001B3ull;
static constexpr uint64_t SPLITMIX_INCREMENT = 0x9E3779B97F4A7C15ull;
static constexpr uint64_t SPLITMIX_MULTIPLIER_1 = 0xBF58476D1CE4E5B9ull;
static constexpr uint64_t SPLITMIX_MULTIPLIER_2 = 0x94D049BB133111EBull;

void getSwitchIdentityBytes(uint8_t domain, uint8_t * out, size_t len)
{
    const Config & config = Storage::getInstance().getConfig();
    if (!config.has_switchIdentitySeed || config.switchIdentitySeed.size == 0) {
        for (size_t i = 0; i < len; i++)
            out[i] = get_rand_32() & 0xFF;
        return;
    }

    uint64_t state = FNV_OFFSET_BASIS ^ domain;
    for (size_t i = 0; i < config.switchIdentitySeed.size; i++)
        state = (state ^ config.switchIdentitySeed.bytes[i]) * FNV_PRIME;

    for (size_t i = 0; i < len; i++) {
        state += SPLITMIX_INCREMENT;
        uint64_t z = state;
        z = (z ^ (z >> 30)) * SPLITMIX_MULTIPLIER_1;
        z = (z ^ (z >> 27)) * SPLITMIX_MULTIPLIER_2;
        out[i] = (z ^ (z >> 31)) & 0xFF;
    }
}
