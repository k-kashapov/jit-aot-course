cmake_minimum_required(VERSION 3.10)

include_directories(${INCLUDE_DIR})

set(SOURCE_FILES fibonacci.cpp)

add_executable(fibonacci_test ${SOURCE_FILES})
# target_link_libraries(fibonacci_test ir)
install(TARGETS fibonacci_test DESTINATION bin)
