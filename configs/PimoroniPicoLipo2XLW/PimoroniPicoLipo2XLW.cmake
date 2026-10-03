set(PICO_BOARD pico2_w)
set(PICO_PLATFORM rp2350-arm-s)

# RP2350B has 48 GPIOs; required for GP43 (battery ADC) to be addressable.
# The stock pico2_w board header targets RP2350A (30 GPIOs), so override here.
set(PICO_NUM_GPIOS 48)

# NOTE (v1): Flash is declared as 4 MB to the firmware (via PICO_BOARD=pico2_w)
# even though the physical chip is 16 MB. Declaring 16 MB causes the RP2350
# boot/partition path to erase the FlashPROM EEPROM region on reboot, regardless
# of where the EEPROM is placed (verified at both 0x101F8000 and 0x10FF8000).
# Full-flash support is deferred.
