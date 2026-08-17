cmake_minimum_required(VERSION 3.20)

project(dynamic_fps LANGUAGES CXX)

if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "dynamic_fps targets Windows x64")
endif()

add_library(dynamic_fps SHARED
    src/plugin.cpp
    src/rtss.cpp
    src/streamline.cpp
)

target_compile_features(dynamic_fps PRIVATE cxx_std_17)
target_include_directories(dynamic_fps PRIVATE include)
target_compile_definitions(dynamic_fps PRIVATE
    WIN32_LEAN_AND_MEAN
    UNICODE
    _UNICODE
)
target_link_libraries(dynamic_fps PRIVATE kernel32)

set_target_properties(dynamic_fps PROPERTIES
    PREFIX ""
    OUTPUT_NAME "dynamic_fps"
)

install(TARGETS dynamic_fps RUNTIME DESTINATION .)
install(FILES config.ini DESTINATION .)

if(MSVC)
    target_compile_options(dynamic_fps PRIVATE /W4 /WX /utf-8)
endif()
