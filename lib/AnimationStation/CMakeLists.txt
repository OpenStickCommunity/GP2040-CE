add_library(AnimationStation
src/AnimationStation.cpp
src/Animation.cpp
)
target_include_directories(AnimationStation PUBLIC 
src
)
target_link_libraries(AnimationStation PUBLIC
pico_stdlib
hardware_clocks
hardware_timer
NeoPico
)
