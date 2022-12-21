add_library(lwip-port
server/dhserver.c
server/dnserver.c
)
target_include_directories(lwip-port INTERFACE 
.
server
arch
)
target_link_libraries(lwip-port 
pico_stdlib
pico_lwip
)
target_include_directories(pico_lwip INTERFACE
.
)