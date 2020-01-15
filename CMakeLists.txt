cmake_minimum_required (VERSION 2.6)
project (main)
find_package(Threads)
#SET(CMAKE_CXX_FLAGS -pthread)
SET( CMAKE_BUILD_TYPE Release )

file(GLOB SRC
    "*.hpp"
    "*.cpp"
)
add_executable(main ${SRC})

if (WIN32)
    find_library(WINDIVERT_LIB WinDivert WinDivert)
    
    target_link_libraries(main wsock32 ws2_32 ${WINDIVERT_LIB})
    add_custom_command(TARGET main POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${PROJECT_SOURCE_DIR}/WinDivert/WinDivert.dll"      # <--this is in-file
        $<TARGET_FILE_DIR:main>)                 # <--this is out-file path
    add_custom_command(TARGET main POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${PROJECT_SOURCE_DIR}/WinDivert/WinDivert32.sys"      # <--this is in-file
        $<TARGET_FILE_DIR:main>)                 # <--this is out-file path
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
    message("WINDOWS")
endif(WIN32)