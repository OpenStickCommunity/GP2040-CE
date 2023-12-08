add_library(rndis
rndis.c
${PICO_SDK_PATH}/lib/tinyusb/lib/networking/rndis_reports.c
)
target_include_directories(rndis PUBLIC 
${CMAKE_CURRENT_LIST_DIR}
${CMAKE_SOURCE_DIR}/headers
${PICO_SDK_PATH}/lib/tinyusb/lib/networking
)
target_link_libraries(rndis 
pico_stdlib
tinyusb_device
lwip-port
httpd
)

