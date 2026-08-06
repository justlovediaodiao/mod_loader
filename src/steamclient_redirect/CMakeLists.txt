cmake_minimum_required(VERSION 3.20)
project(steamclient_redirect LANGUAGES CXX)

if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "steamclient_redirect targets Windows x64")
endif()

add_library(steamclient_redirect SHARED main.cpp)
target_compile_features(steamclient_redirect PRIVATE cxx_std_17)
target_include_directories(steamclient_redirect PRIVATE include)
target_link_libraries(steamclient_redirect PRIVATE kernel32)

install(TARGETS steamclient_redirect RUNTIME DESTINATION .)
install(FILES README.md DESTINATION .)
