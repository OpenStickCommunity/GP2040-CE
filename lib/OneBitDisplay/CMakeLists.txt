add_library(OneBitDisplay
OneBitDisplay.cpp
)
target_include_directories(OneBitDisplay INTERFACE 
.
fonts
)
target_link_libraries(OneBitDisplay 
pico_stdlib
BitBang_I2C
)