cmake_minimum_required(VERSION 3.20)
project(steam_achievement LANGUAGES CXX)

if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "steam_achievement targets Windows x64")
endif()

add_library(steam_achievement SHARED main.cpp)
target_compile_features(steam_achievement PRIVATE cxx_std_17)
target_include_directories(steam_achievement PRIVATE include)
target_link_libraries(steam_achievement PRIVATE kernel32)

install(TARGETS steam_achievement RUNTIME DESTINATION .)
install(FILES config.ini DESTINATION .)
