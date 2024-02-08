cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0076 NEW)

# set VSF_PATH and VSF_SRC_PATH
if(NOT DEFINED ENV{VSF_PATH})
    get_filename_component(__VSF_PATH ../../../../../../vsf ABSOLUTE)
    set(ENV{VSF_PATH} ${__VSF_PATH})
endif()
message(STATUS "VSF_PATH: $ENV{VSF_PATH}")

# compiler configurations

set(arm_cortex_target CortexM0 CortexM3 CortexM4 CortexM7)
set(thead_riscv_target E902 E907)

if(APPLET_COMPILER_GCC)
    if(${APPLET_TARGET} IN_LIST arm_cortex_target)
        set(CMAKE_C_FLAGS
            "-msingle-pic-base -mpic-register=r9 -mno-pic-data-is-text-relative ${CMAKE_C_FLAGS}"
            CACHE INTERNAL "C compiler common flags"
        )
        set(CMAKE_CXX_FLAGS
            "-msingle-pic-base -mpic-register=r9 -mno-pic-data-is-text-relative ${CMAKE_CXX_FLAGS}"
            CACHE INTERNAL "C++ compiler common flags"
        )
        set(GCC_PREFIX      arm-none-eabi)
        set(GCC_SPEC        nano)
    elseif(${APPLET_TARGET} IN_LIST thead_riscv_target)
        if(APPLET_COMPILER_GOTPI)
            message(FATAL_ERROR "-shared not support, please use APPLET_COMPILER_EMBPI")
        endif()
        set(APPLET_COMPILER_EMBPI 1)
        set(GCC_PREFIX      riscv64-unknown-elf)
    else()
        message(FATAL_ERROR "Target not supported")
    endif()

    if(NOT APPLET_COMPILER_EMBPI AND NOT APPLET_COMPILER_GOTPI)
        set(APPLET_COMPILER_GOTPI 1)
    endif()
    if(APPLET_COMPILER_GOTPI)
        set(CMAKE_C_FLAGS
            "-shared -nodefaultlibs -nolibc -nostdlib ${CMAKE_C_FLAGS}"
            CACHE INTERNAL "C compiler common flags"
        )
        set(CMAKE_CXX_FLAGS
            "-shared -nodefaultlibs -nolibc -nostdlib++ ${CMAKE_CXX_FLAGS}"
            CACHE INTERNAL "C++ compiler common flags"
        )
    elseif(APPLET_COMPILER_EMBPI)
        # for embedded position independency, vsf_linux_applet_lib will be included,
        #   so it's OK to use libs in compiler, bacause the API in these library are weak version,
        #   and will be over-written by the same strong APIs in VSF is required.
    endif()

    set(CMAKE_C_FLAGS
        "-Os -fms-extensions -nostartfiles -e _start -fPIC -z max-page-size=4 ${CMAKE_C_FLAGS}"
        CACHE INTERNAL "C compiler common flags"
    )
    set(CMAKE_CXX_FLAGS
        "-Os -fms-extensions -nostartfiles -e _start -fPIC -z max-page-size=4 ${CMAKE_CXX_FLAGS}"
        CACHE INTERNAL "C++ compiler common flags"
    )

    include($ENV{VSF_PATH}/script/cmake/compilers/gcc_common.cmake)
elseif(APPLET_COMPILER_LLVM)
    if(NOT DEFINED LLVM_TOOLCHAIN_PATH)
        message(WARNING "LLVM_TOOLCHAIN_PATH not defined, use default: -DLLVM_TOOLCHAIN_PATH=\"E:/Software/armllvm16\"")
        set(LLVM_TOOLCHAIN_PATH "E:/Software/armllvm17")
    endif()
    if(NOT DEFINED LLVM_TOOLCHAIN_EXE_SUFIX)
        message(WARNING "LLVM_TOOLCHAIN_EXE_SUFIX not defined, use default: -DLLVM_TOOLCHAIN_EXE_SUFIX=\".exe\"")
        set(LLVM_TOOLCHAIN_EXE_SUFIX ".exe")
    endif()
    if(NOT DEFINED LLVM_TOOLCHAIN_SYSROOT)
        message(WARNING "LLVM_TOOLCHAIN_SYSROOT not defined, use default: -DLLVM_TOOLCHAIN_EXE_SUFIX=\"${LLVM_TOOLCHAIN_PATH}/lib/clang-runtimes/arm-none-eabi/armv7m_soft_nofp\"")
        set(LLVM_TOOLCHAIN_SYSROOT "${LLVM_TOOLCHAIN_PATH}/lib/clang-runtimes/arm-none-eabi/armv7m_soft_nofp")
    endif()

    if(APPLET_COMPILER_LLVM_EMBPI)
        # embedded position independency
        set(CMAKE_C_FLAGS
            "-Oz -mthumb -fno-builtin-printf -fno-builtin-fprintf -fropi -frwpi"
            CACHE INTERNAL "C compiler common flags"
        )
        set(CMAKE_CXX_FLAGS
            "-Oz -mthumb -fno-builtin-printf -fno-builtin-fprintf -fropi -frwpi"
            CACHE INTERNAL "C++ compiler common flags"
        )
    elseif(APPLET_COMPILER_LLVM_GOTPI)
        # GOT-base position independency
        set(CMAKE_C_FLAGS
            "-Oz -mthumb -fno-builtin-printf -fno-builtin-fprintf -fPIC -mno-pic-data-is-text-relative"
            CACHE INTERNAL "C compiler common flags"
        )
        set(CMAKE_CXX_FLAGS
            "-Oz -mthumb -fno-builtin-printf -fno-builtin-fprintf -fPIC -mno-pic-data-is-text-relative"
            CACHE INTERNAL "C++ compiler common flags"
        )
    else()
        message(FATAL_ERROR "Either APPLET_COMPILER_LLVM_EMBPI or APPLET_COMPILER_LLVM_GOTPI should be set to 1")
    endif()

    include($ENV{VSF_PATH}/script/cmake/compilers/armllvm.cmake)
else()
    message(FATAL_ERROR "compiler is not set.")
endif()

