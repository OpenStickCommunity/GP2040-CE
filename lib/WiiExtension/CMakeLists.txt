add_library(WiiExtension WiiExtension.cpp)
target_include_directories(WiiExtension INTERFACE .)
target_link_libraries(WiiExtension PUBLIC
pico_stdlib
hardware_i2c
)
