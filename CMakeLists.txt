cmake_minimum_required(VERSION 4.0)
project(RM_Arithmetic_coding)

set(CMAKE_CXX_STANDARD 20)

include_directories(include/ordered-map/include)

add_executable(RM_Arithmetic_coding main.cpp)
