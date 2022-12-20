add_library(NeoPico
src/NeoPico.cpp
)
target_include_directories(NeoPico INTERFACE src)

pico_generate_pio_header(NeoPico ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

target_link_libraries(NeoPico PUBLIC
pico_stdlib
hardware_pio
hardware_clocks
)
pico_add_extra_outputs(NeoPico)