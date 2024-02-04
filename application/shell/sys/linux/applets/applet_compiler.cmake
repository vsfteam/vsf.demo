cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0076 NEW)

# set VSF_PATH and VSF_SRC_PATH
if(NOT DEFINED ENV{VSF_PATH})
    get_filename_component(__VSF_PATH ../../../../../../vsf ABSOLUTE)
    set(ENV{VSF_PATH} ${__VSF_PATH})
endif()
message(STATUS "VSF_PATH: $ENV{VSF_PATH}")

# compiler configurations

if(APPLET_COMPILER_GCC)
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
        message(FATAL "Either APPLET_COMPILER_LLVM_EMBPI or APPLET_COMPILER_LLVM_GOTPI should be set to 1")
    endif()

    include($ENV{VSF_PATH}/script/cmake/compilers/armllvm.cmake)
else()
    message(FATAL "compiler is not set.")
endif()

