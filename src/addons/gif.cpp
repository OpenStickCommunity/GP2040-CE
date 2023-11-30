#include "configs/base64.h"
#include "addons/gif.h"
#include <hardware/flash.h>
#include "FlashPROM.h"
#include "FlashPROM.cpp"
#include "storagemanager.h"

extern char __flash_binary_end;

bool GifAddon::available() {
    const DisplayOptions& displayOptions = Storage::getInstance().getDisplayOptions();
    const GifOptions& gifOptions = Storage::getInstance().getAddonOptions().gifOptions;
    return displayOptions.enabled && gifOptions.enabled;
};

void GifAddon::setup() {
    setCurrentFrameFor(0);
};

bool GifAddon::setupForWrite(size_t numberOfBytes)
{
    if (get_core_num() != 0)
        return false;

    const uint32_t numberOfFrames = numberOfBytes / sizeof(SplashFrame);

    const size_t framesMaxAddress = EEPROM_ADDRESS_START - XIP_BASE - GIF_EEPROM_DISTANCE;
    const size_t alignedFramesMaxAddress = (framesMaxAddress / FLASH_SECTOR_SIZE == 0)
        ? framesMaxAddress
        : framesMaxAddress - framesMaxAddress % FLASH_SECTOR_SIZE;

    const size_t framesMinAddress = (size_t) &__flash_binary_end  + GIF_CODE_DISTANCE - XIP_BASE;
    const size_t alignedFramesMinAddress = (framesMinAddress / FLASH_SECTOR_SIZE == 0)
        ? framesMinAddress
        : framesMinAddress + FLASH_SECTOR_SIZE - framesMinAddress % FLASH_SECTOR_SIZE;

    const size_t availableMemory = alignedFramesMaxAddress - alignedFramesMinAddress;

    if (availableMemory <= 0) return false;

    const size_t framesSize = sizeof(SplashFrame) * numberOfFrames;
    const size_t numberOfSectors = (framesSize % FLASH_SECTOR_SIZE == 0)
        ? framesSize / FLASH_SECTOR_SIZE 
        : framesSize / FLASH_SECTOR_SIZE + 1;

    const size_t necessaryMemory = numberOfSectors * FLASH_SECTOR_SIZE;

    if (necessaryMemory > availableMemory) return false;

    const size_t startingAddress = alignedFramesMaxAddress - necessaryMemory;

    while (is_spin_locked(flashLock));

    multicore_lockout_start_blocking();
    uint32_t interrupts = spin_lock_blocking(flashLock);

    flash_range_erase(startingAddress, necessaryMemory);

    multicore_lockout_end_blocking();
    spin_unlock(flashLock, interrupts);

    GifOptions& gifOptions = Storage::getInstance().getAddonOptions().gifOptions;
    gifOptions.startingGifAddress = startingAddress + XIP_BASE;
    gifOptions.numberOfFrames = numberOfFrames;
    Storage::getInstance().save();

    return true;
}

bool GifAddon::writePage(std::string& base64, size_t pageIndex)
{
    if (get_core_num() != 0)
        return false;

    GifOptions& gifOptions = Storage::getInstance().getAddonOptions().gifOptions;
    const size_t writeAddress = gifOptions.startingGifAddress - XIP_BASE + (pageIndex * FLASH_PAGE_SIZE);
    std::string decoded;
    Base64::Decode(base64, decoded);
    const uint8_t * frames = reinterpret_cast<const uint8_t*>(decoded.c_str());

    while (is_spin_locked(flashLock));

    multicore_lockout_start_blocking();
    uint32_t interrupts = spin_lock_blocking(flashLock);

    flash_range_program(writeAddress, frames, FLASH_PAGE_SIZE);

    multicore_lockout_end_blocking();
    spin_unlock(flashLock, interrupts);

    return true;
}

void GifAddon::checkBeforeDraw(int32_t currentMsTime)
{
    if (currentFrame->delay <= 0) return;

    const GifOptions& gifOptions = Storage::getInstance().getAddonOptions().gifOptions;

    int32_t elapsedTime = currentMsTime - lastDrawTimestamp;
    if (elapsedTime < currentFrame->delay) return;

    lastDrawTimestamp = currentMsTime;
    currentFrameIndex = currentFrameIndex >= gifOptions.numberOfFrames - 1
        ? 0 
        : currentFrameIndex + 1;
    setCurrentFrameFor(currentFrameIndex);
}

void GifAddon::setCurrentFrameFor(uint32_t index)
{
    const GifOptions& gifOptions = Storage::getInstance().getAddonOptions().gifOptions;
    const uint32_t offset = index * sizeof(SplashFrame);
    if (gifOptions.startingGifAddress == GIF_STARTING_GIF_ADDRESS) return;
    
    currentFrame = (SplashFrame*) (gifOptions.startingGifAddress + offset);
};
