cmake_minimum_required(VERSION 3.10)

include_directories(${INCLUDE_DIR})

add_executable(fibonacci_test fibonacci.cpp)
add_executable(dom_test dom_test.cpp)
add_executable(loop_test loop_test.cpp)
add_executable(live_ins_test live_ins_test.cpp)
add_executable(regalloc_test test_regalloc.cpp)
add_executable(inline_test inline_test.cpp)

target_link_libraries(fibonacci_test IR)
target_link_libraries(dom_test IR)
target_link_libraries(loop_test IR)
target_link_libraries(live_ins_test IR)
target_link_libraries(regalloc_test IR)
target_link_libraries(inline_test IR)
