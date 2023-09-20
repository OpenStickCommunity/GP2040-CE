add_library(ADS1219 ADS1219.cpp)
target_link_libraries(ADS1219 PUBLIC hardware_i2c)
target_include_directories(ADS1219 INTERFACE .)
