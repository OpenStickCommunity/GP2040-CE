add_library(QuadratureEncoder
src/QuadratureEncoder.cpp
src/QuadratureTurntable.cpp
)
target_include_directories(QuadratureEncoder PUBLIC
 src
 )

pico_generate_pio_header(QuadratureEncoder ${CMAKE_CURRENT_LIST_DIR}/src/quadrature_capture.pio)

target_link_libraries(QuadratureEncoder PUBLIC
pico_stdlib
hardware_pio
hardware_dma
hardware_clocks
hardware_irq
hardware_sync
)
