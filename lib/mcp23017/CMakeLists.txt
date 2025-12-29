add_library(mcp23017 INTERFACE)
target_sources(mcp23017 INTERFACE ${CMAKE_CURRENT_LIST_DIR}/mcp23017.cpp)
target_include_directories(mcp23017 INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(mcp23017 INTERFACE hardware_i2c)
