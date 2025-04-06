cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0076 NEW)

# set VSF_PATH and VSF_SRC_PATH
if(NOT DEFINED ENV{VSF_PATH})
    get_filename_component(__VSF_PATH ../../../../../../vsf ABSOLUTE)
    set(ENV{VSF_PATH} ${__VSF_PATH})
endif()
message(STATUS "VSF_PATH: $ENV{VSF_PATH}")

get_filename_component(LINUX_APPLET_PATH ../ ABSOLUTE)
message(STATUS "LINUX_APPLET_PATH: ${LINUX_APPLET_PATH}")

# target & configurations
set(VSF_TARGET CortexM4)
set(VSF_APPLET 1)
    set(VSF_APPLET_USE_GETOPT 0)
    set(VSF_APPLET_USE_REGEX 0)
    set(VSF_APPLET_USE_CTYPE 0)
set(VSF_USE_LINUX 1)
    set(VSF_LINUX_USE_SIMPLE_LIBC 1)

# compiler configurations
set(CMAKE_C_FLAGS
"-Os"
CACHE INTERNAL "C compiler common flags"
)
set(CMAKE_CXX_FLAGS
"-Os"
CACHE INTERNAL "C++ compiler common flags"
)
include($ENV{VSF_PATH}/script/cmake/compilers/gnuarmemb.cmake)

# project and vsf.cmake
project(busybox C CXX ASM)
include($ENV{VSF_PATH}/script/cmake/vsf.cmake)
vsf_add_include_directories(
    ../include
)

# linker
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    LINKER:-T${LINUX_APPLET_PATH}/linker.ld
    LINKER:-Map=output.map
)
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
)

# application
vsf_add_compile_definitions(
#    __linux__           # DO NOT define __linux__ until mount/mount2 are implemented for busybox
    BB_GLOBAL_CONST=    # avoid busybox optimization, which will cause hardfault on CortexM, refer to BB_GLOBAL_CONST in libbb.h
)
get_filename_component(BUSYBOX_PATH ${LINUX_APPLET_PATH}/busybox ABSOLUTE)
include(${BUSYBOX_PATH}/busybox.cmake)