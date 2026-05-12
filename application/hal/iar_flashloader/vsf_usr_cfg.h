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


//! \note User Level Application Configuration

#ifndef __VSF_USR_CFG_H__
#define __VSF_USR_CFG_H__

/*============================ MACROS ========================================*/

// IAR flashloader runs stand-alone in RAM under C-SPY control. It has
// no scheduler, no streams and no trace sink: just the flash HAL plus
// the 16-bit CRC the XIP driver uses for per-block integrity.
//
// IMPORTANT: these DISABLE defines MUST appear BEFORE #include
// "vsf_board_cfg.h". The board cfg chain-includes utilities/compiler.h
// which pulls in vsf_cfg.h; vsf_cfg.h then has its own default
// `#ifndef VSF_USE_KERNEL #define ... ENABLED #endif` block that
// latches VSF_USE_KERNEL to ENABLED if we have not defined it yet.
// Defining here first both wins the race and silences IAR Pa181
// (incompatible redefinition) warnings.
#define VSF_USE_KERNEL                                  DISABLED
#define VSF_USE_SIMPLE_STREAM                           DISABLED
#define VSF_USE_TRACE                                   DISABLED
#define VSF_USE_HEAP                                    DISABLED
#define VSF_USE_USB_DEVICE                              DISABLED
#define VSF_USE_USB_HOST                                DISABLED

/*============================ MACROS ========================================*/

// This file is MCU/board agnostic. Any config that depends on the
// concrete hardware (flash instance, CRC variant, chip peripherals,
// debug stream, ...) belongs in board/<xxx>/vsf_board_cfg.h, NOT here.

#define VSF_USE_HAL                                     ENABLED
#define VSF_HAL_USE_FLASH                               ENABLED

// Flashloader only needs the flash HAL. Explicitly disable every other
// HAL driver so nothing in hal/driver/common/* gets compiled in (their
// defaults in vsf_hal_cfg.h are ENABLED), preventing chain-pulls of BSP
// helpers like isr_reg_handler / drv_timer / etc.
//
// Using #ifndef so the board_cfg's own DISABLEDs (EXTI/I2S/SDIO) keep
// taking precedence and IAR does not emit Pa181 incompatible
// redefinition warnings.
#ifndef VSF_HAL_USE_GPIO
#   define VSF_HAL_USE_GPIO                             DISABLED
#endif
#ifndef VSF_HAL_USE_EXTI
#   define VSF_HAL_USE_EXTI                             DISABLED
#endif
#ifndef VSF_HAL_USE_PM
#   define VSF_HAL_USE_PM                               DISABLED
#endif
#ifndef VSF_HAL_USE_USART
#   define VSF_HAL_USE_USART                            DISABLED
#endif
#ifndef VSF_HAL_USE_SPI
#   define VSF_HAL_USE_SPI                              DISABLED
#endif
#ifndef VSF_HAL_USE_FIFO2REQ_SPI
#   define VSF_HAL_USE_FIFO2REQ_SPI                     DISABLED
#endif
#ifndef VSF_HAL_USE_MULTIPLEX_CS_SPI
#   define VSF_HAL_USE_MULTIPLEX_CS_SPI                 DISABLED
#endif
#ifndef VSF_HAL_USE_I2C
#   define VSF_HAL_USE_I2C                              DISABLED
#endif
#ifndef VSF_HAL_USE_MULTIPLEX_I2C
#   define VSF_HAL_USE_MULTIPLEX_I2C                    DISABLED
#endif
#ifndef VSF_HAL_USE_MERGER_REQUEST_I2C
#   define VSF_HAL_USE_MERGER_REQUEST_I2C               DISABLED
#endif
#ifndef VSF_HAL_USE_GPIO_I2C
#   define VSF_HAL_USE_GPIO_I2C                         DISABLED
#endif
#ifndef VSF_HAL_USE_I2S
#   define VSF_HAL_USE_I2S                              DISABLED
#endif
#ifndef VSF_HAL_USE_TIMER
#   define VSF_HAL_USE_TIMER                            DISABLED
#endif
#ifndef VSF_HAL_USE_ADC
#   define VSF_HAL_USE_ADC                              DISABLED
#endif
#ifndef VSF_HAL_USE_DAC
#   define VSF_HAL_USE_DAC                              DISABLED
#endif
#ifndef VSF_HAL_USE_DMA
#   define VSF_HAL_USE_DMA                              DISABLED
#endif
#ifndef VSF_HAL_USE_ETH
#   define VSF_HAL_USE_ETH                              DISABLED
#endif
#ifndef VSF_HAL_USE_WDT
#   define VSF_HAL_USE_WDT                              DISABLED
#endif
#ifndef VSF_HAL_USE_RTC
#   define VSF_HAL_USE_RTC                              DISABLED
#endif
#ifndef VSF_HAL_USE_USBD
#   define VSF_HAL_USE_USBD                             DISABLED
#endif
#ifndef VSF_HAL_USE_USBH
#   define VSF_HAL_USE_USBH                             DISABLED
#endif
#ifndef VSF_HAL_USE_PWM
#   define VSF_HAL_USE_PWM                              DISABLED
#endif
#ifndef VSF_HAL_USE_SDIO
#   define VSF_HAL_USE_SDIO                             DISABLED
#endif
#ifndef VSF_HAL_USE_FB
#   define VSF_HAL_USE_FB                               DISABLED
#endif
#ifndef VSF_HAL_USE_RNG
#   define VSF_HAL_USE_RNG                              DISABLED
#endif
#ifndef VSF_HAL_USE_MULTIPLEX_RNG
#   define VSF_HAL_USE_MULTIPLEX_RNG                    DISABLED
#endif

/*============================ INCLUDES ======================================*/

#include "vsf_board_cfg.h"

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#endif
/* EOF */
