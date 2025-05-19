add_library(joybus-pio INTERFACE)

target_include_directories(joybus-pio INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_sources(joybus-pio INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/gamecube.pio
)

pico_generate_pio_header(joybus-pio ${CMAKE_CURRENT_LIST_DIR}/src/gamecube.pio)

target_link_libraries(joybus-pio INTERFACE
    pico_stdlib
    hardware_pio
)
