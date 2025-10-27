cmake_minimum_required(VERSION 3.10)

project(heir)

set(CMAKE_BUILD_PARALLEL_LEVEL 8)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g -fsanitize=address,undefined -Wall -Wextra")

set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(LIB_DIR ${PROJECT_SOURCE_DIR}/src)

add_subdirectory(test)
