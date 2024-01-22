# micropython applet

## Submodule
- vsf
  - PLOOC
  - source/component/3rd-party/mbedtls/raw

## Source
Download pikapython source code for linux at https://pikapython.com/. Use linux as Platform Secection.
Extract the downloaded file to raw directory.

## Compiler and Utilities
- arm-none-eabi-gcc: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
- cmake: https://github.com/Kitware/CMake/releases
- ninja: https://github.com/ninja-build/ninja/releases

## Commands(current directory is application/shell/sys/linux/applets/micropython)
```
> mkdir build
> cd build
> cmake -G Ninja ..
> ninja
```
