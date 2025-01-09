add_library(AnimationStation
src/Effects/Chase.cpp
src/Effects/Rainbow.cpp
src/Effects/StaticColor.cpp
src/Effects/RandomColor.cpp
src/Effects/SMPulseColour.cpp
src/Effects/SMWave.cpp
src/Effects/SMKnightRider.cpp
src/Effects/SMCircleColour.cpp
src/AnimationStation.cpp
src/Animation.cpp
src/SpecialMoveSystem.cpp
)
target_include_directories(AnimationStation PUBLIC 
src
src/Effects
)
target_link_libraries(AnimationStation PUBLIC
pico_stdlib
hardware_clocks
hardware_timer
NeoPico
)
