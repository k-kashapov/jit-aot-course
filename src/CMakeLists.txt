include_directories(${INCLUDE_DIR})

file(GLOB LIB_FILES *.cpp)

add_library(IR ${LIB_FILES})
