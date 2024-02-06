cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0076 NEW)

# set VSF_PATH and VSF_SRC_PATH
if(NOT DEFINED ENV{VSF_PATH})
    get_filename_component(__VSF_PATH ../../../../../../vsf ABSOLUTE)
    set(ENV{VSF_PATH} ${__VSF_PATH})
endif()
message(STATUS "VSF_PATH: $ENV{VSF_PATH}")

# target & configurations
set(VSF_TARGET ${APPLET_TARGET})
set(VSF_APPLET 1)
include($ENV{VSF_PATH}/script/cmake/vsf.cmake)

# tweak for dedicated compiler/options
if(APPLET_COMPILER_LLVM)
    if(APPLET_COMPILER_LLVM_EMBPI)
        vsf_add_sources(
            $ENV{VSF_PATH}/source/shell/sys/linux/lib/vsf_linux_applet_lib.c
        )
        vsf_add_compile_definitions(
            __VSF_APPLET_EMBPI__
        )
    elseif(APPLET_COMPILER_LLVM_GOTPI)
        vsf_add_compile_definitions(
            __VSF_APPLET_GOTPI__
        )
    else()
        message(FATAL "Please select embpi or gotpi by setting APPLET_COMPILER_LLVM_EMBPI/APPLET_COMPILER_LLVM_GOTPI to 1")
    endif()
endif()

# linker configurations
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    LINKER:-T${CMAKE_CURRENT_SOURCE_DIR}/../linker.ld
    LINKER:-Map=output.map
)
