# vsf.demo -- vsf 应用层 demo

[English](README.md) |

vsf.demo 仓库用于存放 VSF 的应用层示例以及开发板的出厂测试程序。应用层示例尽可能做到最简，并且包含 vsf_usr_cfg.h 用户层配置文件。应用层示例一般只有一个 main.c ，当然也可能包含多个 c 源代码，甚至包含子仓库。工程独立于应用之外，切换应用的时候，只需要简单切换三要素即可（应用的源代码，包含路径，预定义宏）。

## 目录结构
- application ：各种应用，目录结构类似 vsf/source
- board ：板级移植
- project ：各种工程
- vsf ： VSF 子仓库

## 子仓库
### 必须的子仓库如下：
- VSF
  - PLOOC

### 根据实际目标芯片，可能会有一些芯片相关的子仓库。比如，对于 ARM 的 cortex 芯片，需要如下子仓库
- VSF
  - CMSIS(source/utilities/compiler/arm/3rd-party/CMSIS)

### 对于一些芯片，使用子仓库的方式，引用芯片 SDK 的，需要克隆对应的子仓库。比如， AIC8800M ，需要克隆如下子仓库
- VSF
  - AIC8800M_SDK_vsf(source/hal/driver/AIC/AIC8800/vendor)

### 应用使用到的子仓库，会写在应用主源代码文件里说明部分的 Dependency 节里

## [使用说明](https://www.bilibili.com/video/BV1Ug411z7wH/?vd_source=4e65ae526a55e90072dbc16c85a4e3da)