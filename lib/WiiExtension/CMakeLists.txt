add_library(WiiExtension WiiExtension.cpp)
target_link_libraries(WiiExtension PUBLIC BitBang_I2C)
target_include_directories(WiiExtension INTERFACE .)
target_include_directories(WiiExtension PUBLIC
BitBang_I2C
)
