cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0076 NEW)

if(NOT DEFINED ENV{VSF_PATH})
    get_filename_component(__VSF_PATH ../../../vsf ABSOLUTE)
    set(ENV{VSF_PATH} ${__VSF_PATH})
endif()
get_filename_component(__PROJ_PATH ./ ABSOLUTE)
message(STATUS "VSF_PATH: $ENV{VSF_PATH}")

get_filename_component(LINUX_APPLET_PATH ../../../application/shell/sys/linux/applets ABSOLUTE)
message(STATUS "LINUX_APPLET_PATH: ${LINUX_APPLET_PATH}")

# board and application settings
set(BOARD_PATH ../../../board/sf32lb5xx)
set(APPLICATION_PATH ../../../application/shell/sys/linux/kernel)

# compiler configurations
# arm-none-eabi-gcc will not pass compiler checker of cmake, so include before project
set(CMAKE_C_FLAGS
    "-Ofast -Wno-maybe-uninitialized -Wno-unused-variable -Wno-attributes -Wno-unknown-pragmas -Wno-unused-label -Wno-discarded-qualifiers -Wno-comment -Wno-pointer-to-int-cast -Wno-unused-but-set-variable -Wno-implicit-function-declaration -Wno-parentheses -Wno-incompatible-pointer-types -Wno-tautological-compare -Wno-dangling-pointer -Wno-pointer-sign"
    CACHE INTERNAL "C compiler common flags"
)
set(CMAKE_CXX_FLAGS
    "-Ofast -Wno-maybe-uninitialized -Wno-unused-variable -Wno-attributes -Wno-unknown-pragmas -Wno-unused-label -Wno-discarded-qualifiers -Wno-comment -Wno-pointer-to-int-cast -Wno-unused-but-set-variable -Wno-implicit-function-declaration -Wno-parentheses -Wno-incompatible-pointer-types -Wno-tautological-compare -Wno-dangling-pointer -Wno-pointer-sign"
    CACHE INTERNAL "C++ compiler common flags"
)
include($ENV{VSF_PATH}/script/cmake/compilers/gnuarmemb.cmake)

project(vsf_demo C CXX ASM)
set(VSF_TARGET SF32LB520U36)

# VSF_USE configurations
set(VSF_USE_LINUX 1)
    set(VSF_LINUX_USE_SIMPLE_LIBC 1)
    set(VSF_LINUX_USE_SDL2 1)
    set(VSF_LINUX_USE_MICROPYTHON 0)
    set(VSF_LINUX_USE_ALSA 0)
set(VSF_USE_LITTLEFS 1)
set(VSF_USE_QRCODE 1)
set(VSF_USE_FREETYPE 0)
set(VSF_USE_LWIP 1)
set(VSF_USE_MBEDTLS 1)

# include vsf cmake script
set(VSF_CONFIG_PATH ${APPLICATION_PATH})
get_filename_component(ABS_BOARD_PATH ${BOARD_PATH} ABSOLUTE)
list(APPEND VSF_TARGET_INCLUDE_DIRECTORIES
    ${ABS_BOARD_PATH}
)
include($ENV{VSF_PATH}/script/cmake/vsf.cmake)

# application configurations
vsf_add_compile_definitions(
    __VSF__
    __VSF_CPP__

    SF32LB52X
    SOC_BF0_HCPU
    VSF_COMPATIBILITY=DISABLED
)

if(VSF_USE_MBEDTLS)
    vsf_add_compile_definitions(
        __unix__
    )
endif()
if(VSF_USE_LWIP)
    vsf_add_include_directories(
        ${APPLICATION_PATH}/config/lwip
    )
endif()

if(VSF_LINUX_USE_ALSA)
    get_filename_component(ALSA_APPLET_PATH ${LINUX_APPLET_PATH}/alsa ABSOLUTE)
    include(${ALSA_APPLET_PATH}/alsa.cmake)
endif()

if(VSF_LINUX_USE_SDL2)
    vsf_add_compile_definitions(
        VSF_LINUX_USE_SDL2=ENABLED
        VSF_SDL2_USE_VPLT=ENABLED
    )
    get_filename_component(SDL2_APPLET_PATH ${LINUX_APPLET_PATH}/sdl ABSOLUTE)
    include(${SDL2_APPLET_PATH}/sdl2.cmake)

#    vsf_add_compile_definitions(
#        APP_USE_SDLPAL=ENABLED
#    )
#    include(${SDL2_APPLET_PATH}/sdlpal.cmake)

    # -fno-short-enums required by SDL2
    target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC -fno-short-enums)
endif()

if(VSF_LINUX_USE_MICROPYTHON)
    vsf_add_compile_definitions(
        VSF_LINUX_USE_MICROPYTHON=ENABLED
        VSF_FREETYPE_USE_STDIO=ENABLED
    )

    get_filename_component(MICROPYTHON_APPLET_PATH ${LINUX_APPLET_PATH}/micropython ABSOLUTE)
    set(MICROPYTHON_USE_PYGAME 1)
        set(PYGAME_RENDER_USE_LIBCG 1)
    set(mpy_variant standard)
    include(${MICROPYTHON_APPLET_PATH}/micropython.cmake)
endif()

# linker script
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    LINKER:-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
    LINKER:-Map=output.map
)

# add user application source code/include directories
add_subdirectory(${BOARD_PATH} ${CMAKE_CURRENT_BINARY_DIR}/board_bin)
add_subdirectory(${APPLICATION_PATH} ${CMAKE_CURRENT_BINARY_DIR}/application_bin)

# type conversion and show size
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    SUFFIX ".elf"
)
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin
)
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    USES_TERMINAL
)