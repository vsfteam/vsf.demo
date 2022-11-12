# vsf.demo -- demos for vsf

[English](README.md) |

vsf.demo is used as demonstrations for VSF platform, each demo will be of one dedicated module or function include vsf_usr_cfg.h. Usually one demo has one source code main.c, but some demo will need more source files even submodule. To switch between demos, please switch in project source codes, include directories and pre-defined symbols.

## Catalogue
- application : applications in the same catalogue as vsf/source
- board : board supporting package
- project : projects
- vsf : VSF submodule

## Submodule
### Obligatory submodules 
- VSF
  - PLOOC

### Submodules for specific arch, eg for ARM cortex chips:
- VSF
  - CMSIS(source/utilities/compiler/arm/3rd-party/CMSIS)

### Submodules for specific chip, eg for AIC8800M:
- VSF
  - AIC8800M_SDK_vsf(source/hal/driver/AIC/AIC8800/vendor)

### Submodules for specific application
Listed in the comments of the main source file, "Denpendency" section

## [how to use(Chinese)](https://www.bilibili.com/video/BV1Ug411z7wH/?vd_source=4e65ae526a55e90072dbc16c85a4e3da)