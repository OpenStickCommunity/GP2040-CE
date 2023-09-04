add_library(I2CIF INTERFACE)
target_include_directories(I2CIF INTERFACE .)
target_link_libraries(I2CIF INTERFACE
pico_stdlib
hardware_i2c
hardware_spi
)