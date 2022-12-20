add_library(rndis
rndis.c
rndis_reports.c
)
target_include_directories(rndis PUBLIC 
${CMAKE_CURRENT_LIST_DIR}
${CMAKE_SOURCE_DIR}/headers
)
target_link_libraries(rndis 
pico_stdlib
tinyusb_device
lwip-port
httpd
)

