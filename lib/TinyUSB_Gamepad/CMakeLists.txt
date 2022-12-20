add_library(TinyUSB_Gamepad
src/hid_driver.cpp
src/net_driver.cpp
src/tusb_driver.cpp
src/usb_descriptors.cpp
src/xinput_driver.cpp
)
target_include_directories(TinyUSB_Gamepad PUBLIC 
src
${CMAKE_SOURCE_DIR}/headers
)
target_link_libraries(TinyUSB_Gamepad 
pico_stdlib
tinyusb_device
MPG
rndis
)