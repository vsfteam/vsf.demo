cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0076 NEW)

#if(NOT DEFINED ENV{VSF_PATH})
    get_filename_component(__VSF_PATH ../../../vsf ABSOLUTE)
    set(ENV{VSF_PATH} ${__VSF_PATH})
#endif()
message(STATUS "VSF_PATH: $ENV{VSF_PATH}")

# PICO_SDK_PATH is inside the vsf submodule
set(__PICO_SDK_PATH ${__VSF_PATH}/source/hal/driver/RaspberryPi/RP2040/vendor)
set(ENV{PICO_SDK_PATH} ${__PICO_SDK_PATH})
message(STATUS "PICO_SDK_PATH: $ENV{PICO_SDK_PATH}")

# target, board and application settings
set(VSF_TARGET RP2040)
set(BOARD_PATH ../../../board/rp2040)
set(APPLICATION_PATH ../../../application/component/vsf-test)
set(PICO_BARE_METAL 1)

include(pico_sdk_import.cmake)
project(vsf_demo C CXX ASM)
pico_sdk_init()

# include vsf cmake script
set(VSF_CONFIG_PATH ${APPLICATION_PATH})
get_filename_component(ABS_BOARD_PATH ${BOARD_PATH} ABSOLUTE)

# Pico SDK 2.x reorganized include paths relative to what VSF RP2040.cmake expects.
# Add the correct paths here so they take precedence.
list(APPEND VSF_TARGET_INCLUDE_DIRECTORIES
    ${ABS_BOARD_PATH}
    ${PICO_SDK_PATH}/src/common/pico_base_headers/include
    ${PICO_SDK_PATH}/src/rp2040/pico_platform/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_platform_compiler/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_platform_sections/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_platform_panic/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_platform_common/include
    ${PICO_SDK_PATH}/src/rp2_common/cmsis/stub/CMSIS/Device/RP2040/Include
)
# Linux profile needs the cmake-side VSF_USE_LINUX flag set before vsf.cmake
# pulls in shell/sys/linux/CMakeLists.txt (which gates on if(VSF_USE_LINUX)).
if("${VSF_KERNEL_PROFILE}" STREQUAL "VSF_TEST_KERNEL_PROFILE_LINUX")
    set(VSF_USE_LINUX 1)
    set(VSF_LINUX_USE_SIMPLE_LIBC 1)
endif()
include($ENV{VSF_PATH}/script/cmake/vsf.cmake)

# executable is added in vsf.cmake, so pico_add_extra_outputs can be used here
pico_add_extra_outputs(vsf_demo)

# application configurations
target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
    __VSF__
    __VSF_CPP__
    PICO_RP2040=1
    VSF_TRACE_CFG_COLOR_EN=DISABLED
    VSF_TRACE_CFG_PROTECT_LEVEL=none
)

# linker script
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    LINKER:-T${CMAKE_CURRENT_SOURCE_DIR}/memmap_default.ld
    LINKER:-Map=output.map
)
# Linux profile: VSF simple_libc defines stdio functions that overlap with
# newlib. Allow the first definition (VSF's) to win.
if("${VSF_KERNEL_PROFILE}" STREQUAL "VSF_TEST_KERNEL_PROFILE_LINUX")
    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
        LINKER:--allow-multiple-definition
    )
endif()

# add board and application source code
add_subdirectory(${BOARD_PATH} ${CMAKE_CURRENT_BINARY_DIR}/board_bin)
add_subdirectory(${APPLICATION_PATH} ${CMAKE_CURRENT_BINARY_DIR}/application_bin)

# download via CMSIS-DAP (DAPLink, SWD)
add_custom_target(download_swd
    COMMAND openocd
        -f interface/cmsis-dap.cfg
        -f target/rp2040.cfg
        -c "adapter speed 5000"
        -c "program ${CMAKE_PROJECT_NAME}.elf verify reset exit"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS vsf_demo
    COMMENT "Flashing ${CMAKE_PROJECT_NAME}.elf via OpenOCD + DAPLink"
    VERBATIM
)
