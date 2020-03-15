project("Battle Paddle")

set(VERSION_MAJOR "0")
set(VERSION_MINOR "0")
set(VERSION_PATCH "0")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_BUILD_TYPE release)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

cmake_minimum_required(VERSION 2.6)

include_directories(include)

if(WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/external/WinDivert)
    link_directories(${CMAKE_SOURCE_DIR}/external/WinDivert)
endif(WIN32)

find_package(Threads)

add_subdirectory(source)
