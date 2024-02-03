cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0076 NEW)

set(APPLET_COMPILER_GCC 1)
set(APPLET_TARGET CortexM4)
include(../applet_compiler.cmake)
project(pikapython C CXX ASM)
include(../applet_common.cmake)

# add user application source code/include directories
vsf_add_include_directories(
    ../include
)
include(pikapython.cmake)