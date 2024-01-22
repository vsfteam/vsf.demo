cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0076 NEW)

# set VSF_PATH and VSF_SRC_PATH
if(NOT DEFINED ENV{VSF_PATH})
    get_filename_component(__VSF_PATH ../../../../../../vsf ABSOLUTE)
    set(ENV{VSF_PATH} ${__VSF_PATH})
endif()
message(STATUS "VSF_PATH: $ENV{VSF_PATH}")

# compiler configurations

# arm-none-eabi-gcc will not pass compiler checker of cmake, so include before project
# refer to arm-none-eabi-gcc BUG: https://answers.launchpad.net/gcc-arm-embedded/+question/675869
set(CMAKE_C_FLAGS 
    "-Os -fms-extensions -nostartfiles -e _start -msingle-pic-base -mpic-register=r9 -mno-pic-data-is-text-relative -fPIC -shared -z max-page-size=4 -nostartfiles -nodefaultlibs -nolibc -nostdlib"
    CACHE INTERNAL "C compiler common flags"
)
set(CMAKE_CXX_FLAGS 
    "-Os -fms-extensions -nostartfiles -e _start -msingle-pic-base -mpic-register=r9 -mno-pic-data-is-text-relative -fPIC -shared -z max-page-size=4 -nostartfiles -nodefaultlibs -nolibc -nostdlib++"
    CACHE INTERNAL "C++ compiler common flags"
)
include($ENV{VSF_PATH}/script/cmake/compilers/gnuarmemb.cmake)

project(pikapython C CXX ASM)

# target & configurations
set(VSF_TARGET CortexM4)
set(VSF_APPLET 1)
include($ENV{VSF_PATH}/script/cmake/vsf.cmake)

# linker configurations
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    LINKER:-T${CMAKE_CURRENT_SOURCE_DIR}/../linker.ld
    LINKER:-Map=output.map
)

# add user application source code/include directories
vsf_add_include_directories(
    ../include
)
include(pikapython.cmake)