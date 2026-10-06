#pragma once
#include <stdint.h>

bool amiiboKeyValid(const uint8_t *key, bool locked);
bool amiiboRandomize(const uint8_t *original540, uint8_t *output540,
    const uint8_t *unfixed80, const uint8_t *locked80, const uint8_t *uid7);
