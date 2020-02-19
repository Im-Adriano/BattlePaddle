cmake_minimum_required(VERSION 2.6)
project(main)
SET(CMAKE_CXX_STANDARD 17)
find_package(Threads)
SET(CMAKE_BUILD_TYPE release)

file(GLOB SRC
        "*.hpp"
        "*.cpp"
        )
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Os -s -DCMAKE_GENERATOR_PLATFORM=x64")
SET(BUILD_SHARED_LIBS OFF)


if (UNIX)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    SET(CMAKE_EXE_LINKER_FLAGS "-static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive")

    add_executable(main ${SRC})
endif (UNIX)


if (WIN32)
    add_library(main SHARED ${SRC})

    find_library(WINDIVERT_LIB WinDivert WinDivert)
    find_library(IPHLPAPI_LIB iphlpapi iphlpapi)
    target_link_libraries(main ${WINDIVERT_LIB} ${IPHLPAPI_LIB})
    add_custom_command(TARGET main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/WinDivert/WinDivert.dll"
            $<TARGET_FILE_DIR:main>)
    add_custom_command(TARGET main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/WinDivert/WinDivert32.sys"
            $<TARGET_FILE_DIR:main>)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
    add_custom_command(TARGET main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/WinDivert/WinDivert64.sys"
            $<TARGET_FILE_DIR:main>)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
    message("WINDOWS")
endif (WIN32)