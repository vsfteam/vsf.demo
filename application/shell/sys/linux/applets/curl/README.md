# curl applet

## Submodule
- vsf
  - PLOOC
- application/shell/sys/linux/applets/curl/raw

## Note
Please use curl 7.81.0, because later version is imcompatible with arm-none-eabi-gcc 13.2.1.
The BUG is below:

```c
#define STRCONST(x) x,sizeof(x)-1
#define test(a,b,c) testx(a,b,c)

void testx(int a, char *b, int c)
{

}

int main(int argc, char **argv)
{
    test(1, STRCONST("abc"));
    return 0;
}

error: macro "test" requires 3 arguments, but only 2 given
```

## Patch
Apply ram_usage.patch to raw.

## Compiler and Utilities
- arm-none-eabi-gcc: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
- cmake: https://github.com/Kitware/CMake/releases
- ninja: https://github.com/ninja-build/ninja/releases

## Commands(current directory is application/shell/sys/linux/applets/curl)
```
> mkdir build
> cd build
> cmake -G Ninja ..
> ninja
```
