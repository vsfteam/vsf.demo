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

/*============================ INCLUDES ======================================*/

// retrive compiler information
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
#define VSF_SYSTIMER_FREQ                               (240UL * 1000 * 1000)

/*----------------------------------------------------------------------------*
 * Hal Driver Configurations                                                  *
 *----------------------------------------------------------------------------*/

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
#ifndef VSF_HAL_USE_MMC
#   define VSF_HAL_USE_MMC                              ENABLED
#endif
#ifndef VSF_HAL_USE_I2S
#   define VSF_HAL_USE_I2S                              ENABLED
#endif
#ifndef VSF_HAL_USE_USART
#   define VSF_HAL_USE_USART                            ENABLED
#endif
// debug port on aic8800m_uikit is uart1, so disable debug stream
//  debug stream is implement in vsf_board.c using uart1
#ifndef VSF_HAL_USE_DEBUG_STREAM
#   if VSF_HAL_USE_USART == ENABLED
#       define VSF_HAL_USE_DEBUG_STREAM                 DISABLED
#   else
#       define VSF_HAL_USE_DEBUG_STREAM                 ENABLED
#   endif
#endif

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
#define VSF_OS_CFG_DEFAULT_TASK_FRAME_POOL_SIZE         32
#define VSF_POOL_CFG_FEED_ON_HEAP                       DISABLED

// enable VSF_EDA_QUEUE_CFG_REGION and VSF_EDA_QUEUE_CFG_SUPPORT_ISR needed by
//  AIC8800_SDK
#define VSF_EDA_QUEUE_CFG_REGION                        ENABLED
#define VSF_EDA_QUEUE_CFG_SUPPORT_ISR                   ENABLED

/*----------------------------------------------------------------------------*
 * Components Configurations                                                  *
 *----------------------------------------------------------------------------*/

#ifndef VSF_USE_HEAP
#   define VSF_USE_HEAP                                 ENABLED
#endif
#   define VSF_HEAP_CFG_MCB_MAGIC_EN                    ENABLED
#   define VSF_HEAP_CFG_MCB_ALIGN_BIT                   4
#   define VSF_HEAP_ADDR                                0x00100000
#   define VSF_HEAP_SIZE                                0x50000

#define VSF_USBH_USE_HCD_DWCOTG                         ENABLED
//  AIC8800 need this
#   define VSF_DWCOTG_HCD_WORKAROUND_ALIGN_BUFFER_SIZE  4096
#   define VSF_USBH_CFG_ENABLE_ROOT_HUB                 DISABLED
// usbh memory MUST be in 0x001A0000 - 0x001C7FFF
#define vsf_usbh_malloc                                 __vsf_usbh_malloc
#define vsf_usbh_malloc_aligned                         __vsf_usbh_malloc_aligned
#define vsf_usbh_free                                   __vsf_usbh_free

#define VSF_USBD_USE_DCD_DWCOTG                         ENABLED
#   define VSF_USBD_CFG_SPEED                           USB_SPEED_HIGH

// debug stream is implemented in vsf_board.c
#if VSF_HAL_USE_DEBUG_STREAM == DISABLED && VSF_HAL_USE_USART == ENABLED
#   define VSF_CFG_DEBUG_STREAM_TX_T                    vsf_stream_t
#   define VSF_CFG_DEBUG_STREAM_RX_T                    vsf_mem_stream_t
#endif

#define VSF_AUDIO_USE_AIC1000A                          ENABLED
#define VSF_AUDIO_CFG_EDA_PRIORITY                      vsf_prio_highest
#define VSF_DISP_USE_MIPI_SPI_LCD                       ENABLED

/*----------------------------------------------------------------------------*
 * Application Configurations                                                 *
 *----------------------------------------------------------------------------*/

#define APP_CFG_USBH_ARCH_PRIO                          vsf_arch_prio_0

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#endif      // __VSF_BOARD_CFG_H__
