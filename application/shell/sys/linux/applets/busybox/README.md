# busybox applet

## Submodule
- vsf
  - PLOOC
- application/shell/sys/linux/applets/busybox/raw

## patch
- apply busybox.patch to application/shell/sys/linux/applets/busybox/raw

## Compiler and Utilities
- arm-none-eabi-gcc: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
- cmake: https://github.com/Kitware/CMake/releases
- ninja: https://github.com/ninja-build/ninja/releases

## Commands(current directory is application/shell/sys/linux/applets/busybox)
> mkdir build
> cd build
> cmake -G Ninja ..
> ninja
