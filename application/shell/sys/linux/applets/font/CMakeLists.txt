cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0076 NEW)

set(APPLET_COMPILER_GCC 1)
set(APPLET_TARGET CortexM4)
include(../applet_compiler.cmake)
project(linux_font C CXX ASM)
include(../applet_common.cmake)

# add user application source code/include directories
vsf_add_compile_definitions(
    ${VSF_TARGET_DEFINITIONS}
    __VSF__
    alloca=__builtin_alloca
)
vsf_add_include_directories(
    ${VSF_COMMON_INCLUDE_DIRECTORIES}
    ../include
)
vsf_add_sources(
    font_vplt.c
    raw/font_8x8.c
    raw/font_8x16.c
)
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    LINKER:-e,_start
)
