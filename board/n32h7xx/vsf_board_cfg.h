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
 * Hal Driver Configurations                                                  *
 *----------------------------------------------------------------------------*/

#define VSF_HAL_USE_I2C                                 DISABLED
#define VSF_HAL_USE_SDIO                                DISABLED

/*============================ INCLUDES ======================================*/

// retrieve compiler information
#define __VSF_HEADER_ONLY_SHOW_COMPILER_INFO__
#include "utilities/compiler/compiler.h"

/*============================ MACROS ========================================*/

/*----------------------------------------------------------------------------*
 * Architecture Configurations                                                *
 *----------------------------------------------------------------------------*/

// thread reg of cortex-m is r9, which will be used in elf-loader
//  note that r9 should not be used by compiler, if elf-loader is enabled
//  for IAR, add compiler option: --lock_regs=r9
#define VSF_ARCH_USE_THREAD_REG                         ENABLED

#define VSF_BOARD_ARCH_STR                              "CortexM7"
#define VSF_BOARD_ARCH_APP_FORMAT                       "romfs"

/*----------------------------------------------------------------------------*
 * Kernel Configurations                                                      *
 *----------------------------------------------------------------------------*/

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

/*----------------------------------------------------------------------------*
 * Components Configurations                                                  *
 *----------------------------------------------------------------------------*/

#ifndef VSF_USE_HEAP
#   define VSF_USE_HEAP                                 ENABLED
#endif
#   define VSF_HEAP_CFG_MCB_MAGIC_EN                    ENABLED
#   define VSF_HEAP_SIZE                                (256 * 1024)

#define VSF_USBH_USE_HCD_DWCOTG                         ENABLED
#   define VSF_USBH_USE_HUB                             ENABLED
#   define VSF_USBH_CFG_ENABLE_ROOT_HUB                 DISABLED

#define VSF_USBD_USE_DCD_DWCOTG                         ENABLED
#   define VSF_USBD_CFG_SPEED                           USB_SPEED_HIGH

#define VSF_BOARD_SEPERATE_USB_HOST_DEVICE

// debug stream is implemented in vsf_board.c
#if VSF_HAL_USE_DEBUG_STREAM == DISABLED && VSF_HAL_USE_USART == ENABLED
#   define VSF_CFG_DEBUG_STREAM_TX_T                    vsf_stream_t
#   define VSF_CFG_DEBUG_STREAM_RX_T                    vsf_mem_stream_t
#endif

#define VSF_USE_USB_HOST                                ENABLED
#define VSF_USE_USB_DEVICE                              ENABLED
#   define VSF_USBH_USE_HCD_DWCOTG                      ENABLED
#   define VSF_USBH_USE_HUB                             ENABLED
#   define VSF_USBH_CFG_ENABLE_ROOT_HUB                 DISABLED
#   define VSF_USBD_CFG_SPEED_HIGH

#define VSF_USE_LWIP                                    ENABLED
#define VSF_USE_UI                                      DISABLED

#define VSF_USE_AUDIO                                   DISABLED
#   define VSF_AUDIO_USE_PLAYBACK                       ENABLED
#   define VSF_AUDIO_USE_DUMMY                          ENABLED

#define VSF_LINUX_CFG_STACKSIZE                         (8 * 1024)
#define VSF_KERNEL_CFG_THREAD_STACK_LARGE               ENABLED
#define VSF_LINUX_CFG_HEAP_SIZE                         (512 * 1024)
#define VSF_HEAP_CFG_MCB_ALIGN_BIT                      9

/*----------------------------------------------------------------------------*
 * Application Configurations                                                 *
 *----------------------------------------------------------------------------*/

#define APP_CFG_USBH_ARCH_PRIO                          vsf_arch_prio_0


#define APP_MSCBOOT_CFG_FLASH                           vsf_hw_flash0
#define APP_MSCBOOT_CFG_FLASH_ADDR                      0x15000000
#define APP_MSCBOOT_CFG_ERASE_ALIGN                     (4 * 1024)

#define APP_BOOT1_KEY_IS_DOWN                           (0)

#define APP_MSCBOOT_CFG_BOOTLOADER_SIZE                 0
#define APP_MSCBOOT_CFG_BOOTLOADER_ADDR                 0
#define APP_MSCBOOT_CFG_FW_SIZE                         (1024 * 1024)
#define APP_MSCBOOT_CFG_FW_ADDR                         (APP_MSCBOOT_CFG_BOOTLOADER_ADDR + APP_MSCBOOT_CFG_BOOTLOADER_SIZE)
#define APP_MSCBOOT_CFG_ROMFS_SIZE                      (512 * 1024)
#define APP_MSCBOOT_CFG_ROMFS_ADDR                      (APP_MSCBOOT_CFG_FW_ADDR + APP_MSCBOOT_CFG_FW_SIZE)
#define APP_MSCBOOT_CFG_ROOT_SIZE                       ((512 - 128) * 1024)
#define APP_MSCBOOT_CFG_ROOT_ADDR                       (APP_MSCBOOT_CFG_ROMFS_ADDR + APP_MSCBOOT_CFG_ROMFS_SIZE)

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#endif      // __VSF_BOARD_CFG_H__
