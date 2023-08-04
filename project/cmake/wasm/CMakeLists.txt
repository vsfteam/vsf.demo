cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED ENV{VSF_PATH})
    get_filename_component(__VSF_PATH ../../../vsf/ ABSOLUTE)
    set(ENV{VSF_PATH} ${__VSF_PATH})
endif()
message(STATUS "VSF_PATH: $ENV{VSF_PATH}")

# board and application settings
set(BOARD_PATH ../../../board/wasm)
set(APPLICATION_PATH ../../../application/shell/sys/linux/kernel)

# add -g for debug
# remove -g, remove --emrun, add -O3 for release
set(CMAKE_C_FLAGS 
    "-g"
    CACHE INTERNAL "C compiler common flags"
)
set(CMAKE_CXX_FLAGS 
    "-g"
    CACHE INTERNAL "CXX compiler common flags"
)

project(vsf_template)
set(VSF_TARGET webassembly/emscripten)

# compiler configurations
# if compiler is not set, cmake will find one

# VSF_USE configurations
# include below to use linux
set(VSF_USE_LINUX 1)
    set(VSF_LINUX_USE_SIMPLE_LIBC 1)
set(VSF_USE_DISP 1)
    set(VSF_DISP_USE_SDL2 1)

# include vsf cmake script
set(VSF_CONFIG_PATH ${APPLICATION_PATH})
list(APPEND VSF_TARGET_INCLUDE_DIRECTORIES
    ${BOARD_PATH}
)
include($ENV{VSF_PATH}/script/cmake/vsf.cmake)

# application configurations
target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
    __VSF__
    __VSF_CPP__
)
target_link_options(${CMAKE_PROJECT_NAME} PUBLIC
    --emrun
)
set(CMAKE_EXECUTABLE_SUFFIX ".html")

# add user application source code/include directories
add_subdirectory(${BOARD_PATH} ${CMAKE_CURRENT_BINARY_DIR}/board_bin)
add_subdirectory(${APPLICATION_PATH} ${CMAKE_CURRENT_BINARY_DIR}/application_bin)
