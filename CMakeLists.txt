cmake_minimum_required (VERSION 2.6)
project (main)
SET( CMAKE_CXX_STANDARD 20)
#find_package(Threads)
#SET(CMAKE_CXX_FLAGS -pthread)
SET( CMAKE_BUILD_TYPE debug )

file(GLOB SRC
    "*.hpp"
    "*.cpp"
)
add_executable(main ${SRC})

if (WIN32)
    find_library(WINDIVERT_LIB WinDivert WinDivert)
    
    target_link_libraries(main ${WINDIVERT_LIB})
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
endif(WIN32)