cmake_minimum_required(VERSION 3.20)
project(steamclient_rdata_patch LANGUAGES C CXX)

if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "steamclient_rdata_patch targets Windows x64")
endif()

include(FetchContent)
FetchContent_Declare(
  minhook_source
  GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
  GIT_TAG v1.3.4
  GIT_SHALLOW TRUE
)
FetchContent_GetProperties(minhook_source)
if(NOT minhook_source_POPULATED)
  FetchContent_Populate(minhook_source)
endif()

add_library(steamclient_rdata_patch SHARED
  main.cpp
  ${minhook_source_SOURCE_DIR}/src/buffer.c
  ${minhook_source_SOURCE_DIR}/src/hook.c
  ${minhook_source_SOURCE_DIR}/src/trampoline.c
  ${minhook_source_SOURCE_DIR}/src/hde/hde64.c
)
target_compile_features(steamclient_rdata_patch PRIVATE cxx_std_17)
target_include_directories(steamclient_rdata_patch PRIVATE
  include
  ${minhook_source_SOURCE_DIR}/include
  ${minhook_source_SOURCE_DIR}/src
  ${minhook_source_SOURCE_DIR}/src/hde
)
target_link_libraries(steamclient_rdata_patch PRIVATE kernel32 advapi32)

install(TARGETS steamclient_rdata_patch RUNTIME DESTINATION .)
