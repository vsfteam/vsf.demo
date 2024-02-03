cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0076 NEW)

set(APPLET_COMPILER_GCC 1)
set(APPLET_TARGET CortexM4)
include(../applet_compiler.cmake)
project(busybox C CXX ASM)
include(../applet_common.cmake)

# add user application source code/include directories
vsf_add_compile_definitions(
#    __linux__           # DO NOT define __linux__ until mount/mount2 are implemented for busybox
    BB_GLOBAL_CONST=    # avoid busybox optimization, which will cause hardfault on CortexM, refer to BB_GLOBAL_CONST in libbb.h
)
vsf_add_include_directories(
    ../include
)
include(busybox.cmake)
