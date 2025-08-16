add_library(QMI8658 QMI8658.c)
target_link_libraries(QMI8658 PUBLIC PicoPeripherals)
target_include_directories(QMI8658 INTERFACE .)
target_include_directories(QMI8658 PUBLIC . PicoPeripherals)
