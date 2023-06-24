add_library(DS4Crypto
src/ds4crypto.cpp
)
target_include_directories(DS4Crypto INTERFACE
include
)
target_include_directories(DS4Crypto PUBLIC
# For our header files
${CMAKE_CURRENT_SOURCE_DIR}/include
# For the project-wide mbedtls_config.h
${CMAKE_SOURCE_DIR}/headers
)
target_link_libraries(DS4Crypto
pico_stdlib
pico_mbedtls
pico_rand
)
