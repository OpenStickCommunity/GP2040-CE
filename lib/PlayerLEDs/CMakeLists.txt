add_library(PlayerLEDs
src/PlayerLEDs.cpp
)
target_include_directories(PlayerLEDs PUBLIC 
src
)
target_link_libraries(PlayerLEDs 
pico_stdlib
hardware_pwm
)