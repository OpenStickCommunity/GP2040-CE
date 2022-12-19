cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)
project(GP2040-CE C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
# initialize the Raspberry Pi Pico SDK
pico_sdk_init()
add_subdirectory(lib)
add_subdirectory(MPG)

add_executable(${PROJECT_NAME}
src/gp2040.cpp
src/gp2040aux.cpp
src/gamepad.cpp
src/configmanager.cpp
src/storagemanager.cpp
lib/FlashPROM/src/FlashPROM.cpp
MPG/src/GamepadDebouncer.cpp
MPG/src/MPG.cpp
MPG/src/MPGS.cpp
MPG/src/GamepadDescriptors.cpp
lib/CRC32/src/CRC32.cpp
${PICO_SDK_PATH}/lib/tinyusb/src/class/net/ecm_rndis_device.c
${PICO_SDK_PATH}/lib/tinyusb/src/class/net/ncm_device.c
${PICO_SDK_PATH}/lib/tinyusb/src/class/hid/hid_device.c
${PICO_SDK_PATH}/lib/tinyusb/src/device/usbd.c
${PICO_SDK_PATH}/lib/tinyusb/src/device/usbd_control.c
${PICO_SDK_PATH}/lib/tinyusb/src/tusb.c
lib/TinyUSB_Gamepad/src/tusb_driver.cpp
lib/TinyUSB_Gamepad/src/usb_descriptors.cpp
lib/TinyUSB_Gamepad/src/net_driver.cpp
lib/TinyUSB_Gamepad/src/hid_driver.cpp
lib/TinyUSB_Gamepad/src/xinput_driver.cpp
src/configs/webconfig.cpp
src/addons/analog.cpp
src/addons/turbo.cpp
src/addons/jslider.cpp
src/addons/i2cdisplay.cpp
src/addons/neopicoleds.cpp
src/addons/playerleds.cpp
lib/NeoPico/src/NeoPico.cpp
lib/AnimationStation/src/AnimationStation.cpp
lib/AnimationStation/src/Animation.cpp
lib/AnimationStation/src/AnimationStation.hpp
lib/AnimationStation/src/Animation.hpp
lib/AnimationStation/src/Effects/Chase.cpp
lib/AnimationStation/src/Effects/Rainbow.cpp
lib/AnimationStation/src/Effects/StaticColor.cpp
lib/AnimationStation/src/Effects/StaticTheme.cpp
lib/OneBitDisplay/obd.inl
lib/OneBitDisplay/OneBitDisplay.cpp
lib/BitBang_I2C/BitBang_I2C.c
${PICO_SDK_PATH}/lib/tinyusb/lib/networking/rndis_reports.c
lib/rndis/rndis.c
lib/lwip-port/server/dhserver.c
lib/lwip-port/server/dnserver.c
${PICO_SDK_PATH}/lib/lwip/src/core/pbuf.c
${PICO_SDK_PATH}/lib/lwip/src/core/memp.c
${PICO_SDK_PATH}/lib/lwip/src/core/mem.c
${PICO_SDK_PATH}/lib/lwip/src/core/stats.c
${PICO_SDK_PATH}/lib/lwip/src/core/ipv4/etharp.c
${PICO_SDK_PATH}/lib/lwip/src/core/init.c
${PICO_SDK_PATH}/lib/lwip/src/core/netif.c
lib/httpd/httpd.c
${PICO_SDK_PATH}/lib/lwip/src/core/ipv4/ip4.c
${PICO_SDK_PATH}/lib/lwip/src/core/ip.c
${PICO_SDK_PATH}/lib/lwip/src/core/tcp_in.c
${PICO_SDK_PATH}/lib/lwip/src/core/tcp.c
${PICO_SDK_PATH}/lib/lwip/src/core/tcp_out.c
${PICO_SDK_PATH}/lib/lwip/src/core/def.c
${PICO_SDK_PATH}/lib/lwip/src/core/inet_chksum.c
${PICO_SDK_PATH}/lib/lwip/src/core/timeouts.c
${PICO_SDK_PATH}/lib/lwip/src/core/ipv4/ip4_frag.c
${PICO_SDK_PATH}/lib/lwip/src/core/ipv4/icmp.c
${PICO_SDK_PATH}/lib/lwip/src/core/ipv4/ip4_addr.c
${PICO_SDK_PATH}/lib/lwip/src/core/udp.c
lib/httpd/fs.c
${PICO_SDK_PATH}/lib/lwip/src/netif/ethernet.c
)
target_link_libraries(MPG)
target_link_libraries(ADS1219)
target_include_directories(${PROJECT_NAME} PUBLIC
configs/Pico
src
src/addons
headers
headers/addons
lib
lib/OneBitDisplay
lib/BitBang_I2C
lib/AnimationStation/src
lib/AnimationStation/src/Effects
lib/NeoPico/src
lib/PlayerLEDs
lib/PlayerLEDs/include
lib/PlayerLEDs/src
lib/TinyUSB_Gamepad/include
lib/TinyUSB_Gamepad/src
lib/FlashPROM/include
lib/FlashPROM/src
${PICO_SDK_PATH}/lib/tinyusb/lib/networking
lib/CRC32/src
${PICO_SDK_PATH}/lib/tinyusb/src/device/
${PICO_SDK_PATH}/lib/tinyusb/src/class/hid/
${PICO_SDK_PATH}/lib/tinyusb/src/
${PICO_SDK_PATH}/lib/tinyusb/src/class/
${PICO_SDK_PATH}/lib/tinyusb/src/class/net/
${PICO_SDK_PATH}/lib/tinyusb/lib/networking
${PICO_SDK_PATH}/lib/lwip/src/include/
${PICO_SDK_PATH}/lib/lwip/src/include/netif
lib/lwip-port/
lib/lwip-port/server
lib/rndis
lib/httpd
configs/${PROJECT_NAME}
)

link_libraries(${PROJECT_NAME} 
    pico_stdlib
    pico_multicore
    pico_time
    hardware_i2c
    hardware_spi
    hardware_pio
    tinyusb_device
    tinyusb_board
    hardware_flash
    hardware_adc
    hardware_pwm
    hardware_clocks
    )

pico_add_extra_outputs(${PROJECT_NAME})

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

        install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    DESTINATION .
)