cmake_minimum_required(VERSION 3.10)

include_directories(${INCLUDE_DIR})

add_executable(fibonacci_test fibonacci.cpp)

file(GLOB LIB_FILES ${LIB_DIR}/*.cpp)
add_executable(dom_test dom_test.cpp ${LIB_FILES})
add_executable(loop_test loop_test.cpp ${LIB_FILES})
