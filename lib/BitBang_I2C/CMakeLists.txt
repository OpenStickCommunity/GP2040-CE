add_library(BitBang_I2C BitBang_I2C.c)
target_include_directories(BitBang_I2C INTERFACE .)
target_link_libraries(BitBang_I2C 
pico_stdlib
hardware_i2c
hardware_spi
)