cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0076 NEW)

#if(NOT DEFINED ENV{VSF_PATH})
    get_filename_component(__VSF_PATH ../../../vsf ABSOLUTE)
    set(ENV{VSF_PATH} ${__VSF_PATH})
#endif()
message(STATUS "VSF_PATH: $ENV{VSF_PATH}")

# target, board and application settings
set(VSF_TARGET RP2040)
set(BOARD_PATH ../../../board/rp2040)
set(APPLICATION_PATH ../../../application/kernel/helloworld)
set(PICO_BARE_METAL 1)

include(pico_sdk_import.cmake)
project(vsf_demo C CXX ASM)
pico_sdk_init()

# VSF_USE configurations

# include vsf cmake script
set(VSF_CONFIG_PATH ${APPLICATION_PATH})
get_filename_component(ABS_BOARD_PATH ${BOARD_PATH} ABSOLUTE)
list(APPEND VSF_TARGET_INCLUDE_DIRECTORIES
    ${ABS_BOARD_PATH}
)
include($ENV{VSF_PATH}/script/cmake/vsf.cmake)

# executable is added in vsf.cmake, so pico_add_extra_outputs can be used here
pico_add_extra_outputs(vsf_demo)

# application configurations
target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
    __VSF__
    __VSF_CPP__
)

# linker script
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    LINKER:-T${CMAKE_CURRENT_SOURCE_DIR}/memmap_default.ld
    LINKER:-Map=output.map
)

# add user application source code/include directories
add_subdirectory(${BOARD_PATH} ${CMAKE_CURRENT_BINARY_DIR}/board_bin)
add_subdirectory(${APPLICATION_PATH} ${CMAKE_CURRENT_BINARY_DIR}/application_bin)
