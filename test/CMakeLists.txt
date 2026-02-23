cmake_minimum_required(VERSION 3.10)

include_directories(${INCLUDE_DIR})

file(GLOB LIB_FILES ${LIB_DIR}/*.cpp)
add_executable(fibonacci_test fibonacci.cpp ${LIB_FILES})
add_executable(dom_test dom_test.cpp ${LIB_FILES})
add_executable(loop_test loop_test.cpp ${LIB_FILES})
add_executable(live_ins_test live_ins_test.cpp ${LIB_FILES})

target_link_libraries(fibonacci_test IR)
target_link_libraries(dom_test IR)
target_link_libraries(loop_test IR)
target_link_libraries(live_ins_test IR)
