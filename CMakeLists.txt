cmake_minimum_required(VERSION 3.10)

project(heir)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g -fsanitize=address,undefined -Wall -Wextra")

set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

add_subdirectory(test)
