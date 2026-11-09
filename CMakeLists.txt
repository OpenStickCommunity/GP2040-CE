add_library(httpd
fs.c
fsdata.c
${PICO_SDK_PATH}/lib/lwip/src/apps/http/httpd.c
)
target_include_directories(httpd INTERFACE 
.
)
target_link_libraries(httpd
pico_stdlib
lwip-port
pico_lwip
)