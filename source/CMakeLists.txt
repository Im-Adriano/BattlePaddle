cmake_policy(SET CMP0009 NEW)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Os -s -DCMAKE_GENERATOR_PLATFORM=x64 -fPIC")
set(BUILD_SHARED_LIBS OFF)

add_subdirectory(bpLib)

if (UNIX)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(CMAKE_EXE_LINKER_FLAGS "-static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive")
    add_executable(BP main.cpp)
    add_library(_BP SHARED shared_main.cpp)
    target_link_libraries(BP BP_LIB)
    target_link_libraries(_BP BP_LIB)
endif (UNIX)

if (WIN32)
    find_library(WINDIVERT_LIB NAME WinDivert PATHS ${CMAKE_SOURCE_DIR}/external/WinDivert)
    find_library(IPHLPAPI_LIB iphlpapi)

    add_executable(BP main.cpp)
    add_library(_BP SHARED shared_main.cpp)

    target_link_libraries(BP BP_LIB)
    target_link_libraries(_BP BP_LIB)
    target_link_libraries(BP ${WINDIVERT_LIB} ${IPHLPAPI_LIB})
    target_link_libraries(_BP ${WINDIVERT_LIB} ${IPHLPAPI_LIB})


    add_custom_command(TARGET BP POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/external/WinDivert/WinDivert.dll"
            $<TARGET_FILE_DIR:BP>)
    add_custom_command(TARGET BP POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/external/WinDivert/WinDivert64.sys"
            $<TARGET_FILE_DIR:BP>)


    add_custom_command(TARGET _BP POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/external/WinDivert/WinDivert.dll"
            $<TARGET_FILE_DIR:_BP>)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
    add_custom_command(TARGET _BP POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/external/WinDivert/WinDivert64.sys"
            $<TARGET_FILE_DIR:_BP>)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)


    message("WINDOWS")
endif (WIN32)