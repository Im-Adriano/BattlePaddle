cmake_minimum_required (VERSION 2.6)
project (main)
find_package(Threads)
SET(CMAKE_CXX_FLAGS -pthread)
file(GLOB SRC
    "*.hpp"
    "*.cpp"
)
add_executable(main ${SRC})