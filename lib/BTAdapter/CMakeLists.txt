add_library(BTAdapter BTAdapter.cpp)
target_include_directories(BTAdapter INTERFACE .)
target_link_libraries(BTAdapter 
pico_stdlib
hardware_i2c
hardware_spi
)