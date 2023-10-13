add_library(WiiExtension 
    WiiExtension.cpp
    extensions/ExtensionBase.cpp
    extensions/ClassicExtension.cpp
    extensions/DrumExtension.cpp
    extensions/GuitarExtension.cpp
    extensions/NunchuckExtension.cpp
    extensions/TaikoExtension.cpp
    extensions/TurntableExtension.cpp
)
target_link_libraries(WiiExtension PUBLIC pico_stdlib hardware_i2c)
target_include_directories(WiiExtension INTERFACE .)
target_include_directories(WiiExtension PUBLIC .)
