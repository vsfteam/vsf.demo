# openocd applet

## Submodule
- vsf
  - PLOOC
- application/shell/sys/linux/applets/openocd/raw
  - jimtcl

## patch
apply jimtcl.patch to raw/jimtcl.

## Compiler and Utilities
- arm-none-eabi-gcc: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
- cmake: https://github.com/Kitware/CMake/releases
- ninja: https://github.com/ninja-build/ninja/releases

## Commands(current directory is application/shell/sys/linux/applets/openocd)
```
> mkdir build
> cd build
> cmake -G Ninja ..
> ninja
```

## License
Codes in this directory are under GPLv2 license.