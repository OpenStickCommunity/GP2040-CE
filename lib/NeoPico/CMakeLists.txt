add_library(NeoPico
src/NeoPico.cpp
)
target_include_directories(NeoPico PUBLIC
 src
 )

pico_generate_pio_header(NeoPico ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/src/generated)

target_link_libraries(NeoPico PUBLIC
pico_stdlib
hardware_pio
hardware_clocks
hardware_timer
)
