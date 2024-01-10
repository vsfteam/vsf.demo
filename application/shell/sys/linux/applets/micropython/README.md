# micropython applet

## Submodule
- vsf
  - PLOOC
- application/shell/sys/linux/applets/micropython/raw

## patch
- modify heap_size in ports/unix/main.c, to fit the ram of current target.

## Compiler and Utilities
- arm-none-eabi-gcc: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
- cmake: https://github.com/Kitware/CMake/releases
- ninja: https://github.com/ninja-build/ninja/releases

## Commands(current directory is application/shell/sys/linux/applets/micropython)
> mkdir build
> cd build
> cmake -G Ninja ..
> ninja
