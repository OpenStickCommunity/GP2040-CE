add_library(SNESpad SNESpad.cpp)
target_link_libraries(SNESpad PUBLIC pico_stdlib)
target_include_directories(SNESpad INTERFACE .)
target_include_directories(SNESpad PUBLIC
pico_stdlib
)
