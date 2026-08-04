cmake_minimum_required(VERSION 3.20)
project(mod_loader LANGUAGES CXX)

if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "mod_loader targets Windows x64")
endif()

add_library(mod_loader SHARED proxy.cpp proxy_generated.cpp proxy.def)
target_compile_features(mod_loader PRIVATE cxx_std_17)
target_include_directories(mod_loader PRIVATE include)
target_link_libraries(mod_loader PRIVATE kernel32)

if(MSVC)
  target_compile_options(mod_loader PRIVATE /GS-)
endif()

install(TARGETS mod_loader RUNTIME DESTINATION .)