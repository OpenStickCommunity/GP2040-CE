add_library(FlashPROM
src/FlashPROM.cpp
)
target_include_directories(FlashPROM INTERFACE 
src
)
target_link_libraries(FlashPROM 
pico_stdlib
pico_multicore
hardware_flash
)