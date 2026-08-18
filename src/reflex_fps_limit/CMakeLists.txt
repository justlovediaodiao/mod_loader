cmake_minimum_required(VERSION 3.20)

project(reflex_fps_limit LANGUAGES C CXX)

if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "reflex_fps_limit targets Windows x64")
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

add_library(reflex_fps_limit SHARED
    main.cpp
    ${minhook_source_SOURCE_DIR}/src/buffer.c
    ${minhook_source_SOURCE_DIR}/src/hook.c
    ${minhook_source_SOURCE_DIR}/src/trampoline.c
    ${minhook_source_SOURCE_DIR}/src/hde/hde64.c
)

target_compile_features(reflex_fps_limit PRIVATE cxx_std_17)
target_include_directories(reflex_fps_limit PRIVATE
    include
    ${minhook_source_SOURCE_DIR}/include
    ${minhook_source_SOURCE_DIR}/src
    ${minhook_source_SOURCE_DIR}/src/hde
)
target_compile_definitions(reflex_fps_limit PRIVATE
    WIN32_LEAN_AND_MEAN
    UNICODE
    _UNICODE
)
target_link_libraries(reflex_fps_limit PRIVATE kernel32)

set_target_properties(reflex_fps_limit PROPERTIES
    PREFIX ""
    OUTPUT_NAME "reflex_fps_limit"
)

install(TARGETS reflex_fps_limit RUNTIME DESTINATION .)
install(FILES config.ini DESTINATION .)
