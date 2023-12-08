add_library(lwip-port
${PICO_SDK_PATH}/lib/tinyusb/lib/networking/dhserver.c
${PICO_SDK_PATH}/lib/tinyusb/lib/networking/dnserver.c
)
target_include_directories(lwip-port INTERFACE 
.
arch
${PICO_SDK_PATH}/lib/tinyusb/lib/networking
)
target_link_libraries(lwip-port 
pico_stdlib
pico_lwip
)
target_include_directories(pico_lwip INTERFACE
.
)