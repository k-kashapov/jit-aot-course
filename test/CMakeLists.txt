cmake_minimum_required(VERSION 3.10)

include_directories(${INCLUDE_DIR})

set(FIBONACCI_SOURCE_FILES fibonacci.cpp)

add_executable(fibonacci_test ${FIBONACCI_SOURCE_FILES})

file(GLOB LIB_FILES ${LIB_DIR}/*.cpp)
add_executable(dom_test dom_test.cpp ${LIB_FILES})
