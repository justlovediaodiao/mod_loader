cmake_minimum_required(VERSION 3.20)
project(unity_fps_limit LANGUAGES CXX)

if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "unity_fps_limit targets Windows x64")
endif()

add_library(unity_fps_limit SHARED main.cpp)
target_compile_features(unity_fps_limit PRIVATE cxx_std_17)
target_include_directories(unity_fps_limit PRIVATE include)
target_link_libraries(unity_fps_limit PRIVATE kernel32)

install(TARGETS unity_fps_limit RUNTIME DESTINATION .)
install(FILES config.ini DESTINATION .)
