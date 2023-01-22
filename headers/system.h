#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <cstdint>

namespace System {
    // Returns the size of on-board flash memory in bytes
    uint32_t getTotalFlash();
    // Returns the amount of on-board flash memory used by the firmware in bytes
    uint32_t getUsedFlash();
    // Returns the amount of memory used for static allocations in bytes
    uint32_t getStaticAllocs();
    // Returns the total size of heap memory in bytes
    uint32_t getTotalHeap();
    // Returns the about of heap memory currently allocated in bytes
    uint32_t getUsedHeap();
}

#endif
