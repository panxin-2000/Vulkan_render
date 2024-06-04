cmake_minimum_required(VERSION 3.27)
project(hello_mac)

set(CMAKE_CXX_STANDARD 14)

#find_package(Eigen3 REQUIRED)
#INCLUDE_DIRECTORIES ( "$ENV{EIGEN3_INCLUDE_DIR}" )

include_directories(/opt/homebrew/Cellar/eigen/3.4.0_1/include/eigen3)
add_executable(hello_mac main.cpp )
