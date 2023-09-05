add_library(BTAdapter BTAdapter.cpp)
target_include_directories(BTAdapter INTERFACE .)
target_link_libraries(BTAdapter 
pico_stdlib
pico_btstack_classic
pico_btstack_cyw43
pico_cyw43_arch_none
)
target_include_directories(BTAdapter PRIVATE ${CMAKE_CURRENT_LIST_DIR})