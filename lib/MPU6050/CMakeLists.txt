add_library(MPU6050 MPU6050.cpp)
target_link_libraries(MPU6050 PUBLIC BitBang_I2C)
target_include_directories(MPU6050 INTERFACE .)
target_include_directories(MPU6050 PUBLIC
BitBang_I2C
)