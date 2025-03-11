/*****************************************************************************
 *   Copyright(C)2009-2022 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/


//! \note User Level Board Configuration

#ifndef __VSF_BOARD_CFG_H__
#define __VSF_BOARD_CFG_H__

/*============================ MACROS ========================================*/

/*----------------------------------------------------------------------------*
 * Architecture Configurations                                                *
 *----------------------------------------------------------------------------*/

// thread reg of cortex-m is r9, which will be used in elf-loader
//  note that r9 should not be used by compiler, if elf-loader is enabled
//  for IAR, add compiler option: --lock_regs=r9
#define VSF_ARCH_USE_THREAD_REG                         ENABLED
#define VSF_SYSTIMER_FREQ                               (240UL * 1000 * 1000)

#define VSF_BOARD_ARCH_STR                              "CortexM4"
#define VSF_BOARD_ARCH_APP_FORMAT                       "romfs"

// enable below for AIC8800MBWP
#define VSF_BOARD_CFG_HEAP_IN_PSRAM                     DISABLED

/*----------------------------------------------------------------------------*
 * Hal Driver Configurations                                                  *
 *----------------------------------------------------------------------------*/

#define VSF_BOARD_HAS_USB_HOST                          1
#define VSF_BOARD_HAS_USB_DEVICE                        1

// Use unused interrupt as SWI
#define VSF_DEV_SWI_NUM                                 4
#define VSF_DEV_SWI_LIST                                WDT3_IRQn, WDT2_IRQn, WDT1_IRQn, I2CS_IRQn
#define SWI0_IRQHandler                                 WDT3_IRQHandler
#define SWI1_IRQHandler                                 WDT2_IRQHandler
#define SWI2_IRQHandler                                 WDT1_IRQHandler
#define SWI3_IRQHandler                                 I2CS_IRQHandler

// actually general hal components will be enabled by default
#ifndef VSF_HAL_USE_RNG
#   define VSF_HAL_USE_RNG                              ENABLED
#endif
#define VSF_HAL_USE_GPIO                                ENABLED
#   define VSF_HW_GPIO_CFG_MULTI_CLASS                  ENABLED
#ifndef VSF_HAL_USE_SPI
#   define VSF_HAL_USE_SPI                              ENABLED
#endif
#define VSF_HAL_USE_SDIO                                ENABLED
#ifndef VSF_HAL_USE_I2S
#   define VSF_HAL_USE_I2S                              ENABLED
#endif
#ifndef VSF_HAL_USE_USART
#   define VSF_HAL_USE_USART                            ENABLED
#endif
#ifndef VSF_HAL_USE_DEBUG_STREAM
#   define VSF_HAL_USE_DEBUG_STREAM                     ENABLED
#endif

/*----------------------------------------------------------------------------*
 * Kernel Configurations                                                      *
 *----------------------------------------------------------------------------*/

/*============================ INCLUDES ======================================*/

// retrieve compiler information
#define __VSF_HEADER_ONLY_SHOW_COMPILER_INFO__
#include "utilities/compiler/compiler.h"

#if __IS_COMPILER_IAR__
// for checking stack in jmpbuf
#   define VSF_KERNEL_CFG_THREAD_STACK_CHECK            ENABLED
#   define VSF_KERNEL_GET_STACK_FROM_JMPBUF(__JMPBUF)   ((*(__JMPBUF))[4] & 0xFFFFFFFF)
#elif __IS_COMPILER_GCC__ || __IS_COMPILER_LLVM__
// strtoxxx in newlib has dependency issues, use implementation in simple_stdlib.
//  useful only if VSF_USE_LINUX/VSF_LINUX_USE_SIMPLE_LIBC/VSF_LINUX_USE_SIMPLE_STDLIB are enabled
#   define VSF_LINUX_SIMPLE_STDLIB_USE_STRTOXX          ENABLED
#endif

// configure pool and heap to avoid heap allocating in interrupt
#define VSF_OS_CFG_EVTQ_POOL_SIZE                       128
#define VSF_POOL_CFG_FEED_ON_HEAP                       DISABLED

// enable VSF_EDA_QUEUE_CFG_REGION and VSF_EDA_QUEUE_CFG_SUPPORT_ISR needed by
//  AIC8800_SDK
#define VSF_EDA_QUEUE_CFG_REGION                        ENABLED
#define VSF_EDA_QUEUE_CFG_SUPPORT_ISR                   ENABLED

/*----------------------------------------------------------------------------*
 * Components Configurations                                                  *
 *----------------------------------------------------------------------------*/

#define VSF_USE_LWIP                                    ENABLED

#ifndef VSF_USE_HEAP
#   define VSF_USE_HEAP                                 ENABLED
#endif
#   define VSF_HEAP_CFG_MCB_MAGIC_EN                    ENABLED
#   if VSF_BOARD_CFG_HEAP_IN_PSRAM == ENABLED
#       define VSF_HEAP_CFG_MCB_ALIGN_BIT               8
#       define VSF_HEAP_SIZE                            0x8000
#   else
#       define VSF_HEAP_CFG_MCB_ALIGN_BIT               4
#       define VSF_HEAP_ADDR                            0x00100000
#       define VSF_HEAP_SIZE                            0x60000
#   endif

#define VSF_USBH_USE_HCD_DWCOTG                         ENABLED
#   define VSF_USBH_USE_HUB                             ENABLED
//  if VSF_DWCOTG_HCD_WORKAROUND_ALIGN_BUFFER_SIZE is not defined,
//   usb_heap will be used to allocate buffer when buffer from user is not suitable for DMA.
//#   define VSF_DWCOTG_HCD_WORKAROUND_ALIGN_BUFFER_SIZE  4096
#   define VSF_USBH_CFG_ENABLE_ROOT_HUB                 DISABLED
// usbh memory MUST be in 0x001A0000 - 0x001C7FFF
#define vsf_usbh_malloc                                 __vsf_usbh_malloc
#define vsf_usbh_malloc_aligned                         __vsf_usbh_malloc_aligned
#define vsf_usbh_free                                   __vsf_usbh_free

#define VSF_USBD_USE_DCD_DWCOTG                         ENABLED
#   define VSF_USBD_CFG_SPEED                           USB_SPEED_HIGH
// #if VSF_USBD_CFG_SPEED == USB_SPEED_HIGH will not work
#   define VSF_USBD_CFG_SPEED_HIGH

// debug stream is implemented in vsf_board.c
#if VSF_HAL_USE_DEBUG_STREAM == DISABLED && VSF_HAL_USE_USART == ENABLED
#   define VSF_CFG_DEBUG_STREAM_TX_T                    vsf_stream_t
#   define VSF_CFG_DEBUG_STREAM_RX_T                    vsf_mem_stream_t
#endif

#define VSF_AUDIO_USE_AIC1000A                          ENABLED
#define VSF_DISP_USE_MIPI_SPI_LCD                       ENABLED

//lwip in opensource SDK does not support netlink
#define VSF_LINUX_SOCKET_USE_NETLINK                    DISABLED

/*----------------------------------------------------------------------------*
 * Application Configurations                                                 *
 *----------------------------------------------------------------------------*/

#define APP_CFG_USBH_ARCH_PRIO                          vsf_arch_prio_0

// flash layout for linux:
//  bootloader: 32K @ 0
//  fw: 2M(flash_size) - 32K(bootloader) - 512K(romfs_size) - 64K(/root) - 4K(chip_config) @ 32K
//  romfs: 512K @ 2M(flash_size) - 512K(romfs_size) - 64K(/root) - 4K(chip_config)
//  /root: 64K @ 2M(flash_size) - 64K(/root) - 4K(chip_config)
//  chip_config: 4K @ 2M(flash_size) - 4K
#define __APP_FLASH_SIZE                                (2048 * 1024)
//#define __APP_BOOTLOADER_SIZE                           (32 * 1024)
#define __APP_BOOTLOADER_SIZE                           (0 * 1024)
#define __APP_ROMFS_SIZE                                (768 * 1024)
#define __APP_ROOT_SIZE                                 (64 * 1024)
#define __APP_WIFI_CFG_SIZE                             (4 * 1024)
#define __APP_APP_SIZE                                  (__APP_FLASH_SIZE - __APP_BOOTLOADER_SIZE - __APP_ROMFS_SIZE - __APP_ROOT_SIZE - __APP_WIFI_CFG_SIZE)

#define APP_MSCBOOT_CFG_FW_SIZE                         __APP_APP_SIZE
#define APP_MSCBOOT_CFG_FW_ADDR                         __APP_BOOTLOADER_SIZE
#define APP_MSCBOOT_CFG_ROMFS_SIZE                      __APP_ROMFS_SIZE
#define APP_MSCBOOT_CFG_ROMFS_ADDR                      (APP_MSCBOOT_CFG_FW_ADDR + APP_MSCBOOT_CFG_FW_SIZE)
#define APP_MSCBOOT_CFG_ROOT_SIZE                       __APP_ROOT_SIZE
#define APP_MSCBOOT_CFG_ROOT_ADDR                       (APP_MSCBOOT_CFG_ROMFS_ADDR + APP_MSCBOOT_CFG_ROMFS_SIZE)
#define APP_MSCBOOT_CFG_FLASH                           vsf_hw_flash0
#define APP_MSCBOOT_CFG_FLASH_ADDR                      0x08000000
#define APP_MSCBOOT_CFG_ERASE_ALIGN                     (4 * 1024)
#define APP_MSCBOOT_CFG_ERASE_BLOCK_SIZE                (4 * 1024)
#define APP_MSCBOOT_CFG_WRITE_ALIGN                     (256)
#define APP_MSCBOOT_CFG_WRITE_BLOCK_SIZE                (0)
#define APP_MSCBOOT_CFG_READ_ALIGN                      (0)
#define APP_MSCBOOT_CFG_READ_BLOCK_SIZE                 (0)

#define APP_BOOT0_KEY_PORT                              vsf_hw_gpio1
#define APP_BOOT0_KEY_PIN                               6
#define APP_BOOT0_KEY_IS_DOWN                           (!(vsf_gpio_read(&APP_BOOT0_KEY_PORT) & (1 << APP_BOOT0_KEY_PIN)))

#define APP_BOOT1_KEY_PORT                              vsf_hw_gpio1
#define APP_BOOT1_KEY_PIN                               7
#define APP_BOOT1_KEY_IS_DOWN                           (!(vsf_gpio_read(&APP_BOOT1_KEY_PORT) & (1 << APP_BOOT1_KEY_PIN)))

#define app_mscboot_init()
#define app_mscboot_check()                             APP_BOOT0_KEY_IS_DOWN
#define app_mscboot_fini()
#define app_mscboot_boot()                                                      \
    do {                                                                        \
        uint32_t *image = (uint32_t *)(APP_MSCBOOT_CFG_FLASH_ADDR + APP_MSCBOOT_CFG_FW_ADDR);\
        vsf_arch_set_stack(image[0]);                                           \
        ((void (*)(void))(image[1]))();                                         \
    } while (0)

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#endif      // __VSF_BOARD_CFG_H__
