add_library(ADS1256 ADS1256.cpp)
target_link_libraries(ADS1256 PUBLIC PicoPeripherals)
target_include_directories(ADS1256 INTERFACE .)
target_include_directories(ADS1256 PUBLIC . PicoPeripherals)
