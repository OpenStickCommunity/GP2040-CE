add_library(ADS1115 ADS1115.cpp)
target_link_libraries(ADS1115 PUBLIC PicoPeripherals)
target_include_directories(ADS1115 INTERFACE .)
target_include_directories(ADS1115 PUBLIC . PicoPeripherals)
