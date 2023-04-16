add_library(GPDaughter GPDaughter.cpp)
target_link_libraries(GPDaughter PUBLIC BitBang_I2C)
target_include_directories(GPDaughter INTERFACE .)
target_include_directories(GPDaughter PUBLIC
BitBang_I2C
)
