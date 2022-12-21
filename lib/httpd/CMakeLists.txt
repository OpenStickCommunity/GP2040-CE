add_library(httpd
fs.c
fsdata.c
httpd.c
)
target_include_directories(httpd INTERFACE 
.
)
target_link_libraries(httpd 
pico_stdlib
lwip-port
)