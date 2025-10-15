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
/*============================ MACROS ========================================*/

/*----------------------------------------------------------------------------*
 * Board Configurations                                                       *
 *----------------------------------------------------------------------------*/

#if !defined(__Emulation__) || !defined(__EMULATION_DISTBUS__) || !defined(__VSF_HOSTOS_BYPASS__)
#   define VSF_HAL_USE_I2C                              DISABLED
#   define VSF_HAL_USE_SDIO                             DISABLED
#endif

/*----------------------------------------------------------------------------*
 * Architecture Configurations                                                *
 *----------------------------------------------------------------------------*/

#define VSF_ARCH_LIMIT_NO_SET_STACK
#define VSF_OS_CFG_ADD_EVTQ_TO_IDLE                     ENABLED
// cpu usage is not supported in windows/linux/macos
#define VSF_KERNEL_CFG_CPU_USAGE                        DISABLED

#define VSF_BOARD_ARCH_STR                              "x86_64"
#define VSF_BOARD_ARCH_APP_FORMAT                       "pe"

/*----------------------------------------------------------------------------*
 * Hal Driver Configurations                                                  *
 *----------------------------------------------------------------------------*/

#define VSF_HAL_USE_FLASH                               DISABLED

#define VSF_HAL_USE_DEBUG_STREAM                        ENABLED
#   define VSF_DEBUG_STREAM_CFG_HW_PRIORITY             vsf_arch_prio_0
#define VSF_HAL_USE_DISTBUS                             ENABLED
#   define VSF_HAL_DISTBUS_USE_GPIO                     ENABLED
#   define VSF_HAL_DISTBUS_USE_ADC                      ENABLED
#   define VSF_HAL_DISTBUS_USE_PWM                      ENABLED
#   define VSF_HAL_DISTBUS_USE_I2C                      ENABLED
#   define VSF_HAL_DISTBUS_USE_SPI                      ENABLED
#   define VSF_HAL_DISTBUS_USE_USART                    ENABLED
#   define VSF_HAL_DISTBUS_USE_SDIO                     ENABLED
#   define VSF_HAL_DISTBUS_USE_I2S                      ENABLED
#   define VSF_HAL_DISTBUS_USE_USBD                     ENABLED
#   define VSF_HAL_DISTBUS_USE_USBH                     ENABLED

/*----------------------------------------------------------------------------*
 * Components Configurations                                                  *
 *----------------------------------------------------------------------------*/

#define VSF_USE_UI                                      ENABLED
#   define VSF_DISP_USE_WINGDI                          ENABLED
#   define VSF_BOARD_DISP_HEIGHT                        480
#   define VSF_BOARD_DISP_WIDTH                         800
#   define VSF_BOARD_DISP_COLOR                         VSF_DISP_COLOR_RGB565
#   define VSF_TGUI_CFG_PFB_LINENUM                     20
#   define VSF_TGUI_CFG_DISP_COLOR                      VSF_TGUI_COLOR_RGB_565
#define VSF_USE_AUDIO                                   ENABLED
#   define VSF_AUDIO_USE_PLAYBACK                       ENABLED
#   define VSF_AUDIO_USE_WINSOUND                       ENABLED
#define VSF_FS_USE_WINFS                                ENABLED
#define VSF_USE_HEAP                                    ENABLED
#define VSF_USE_DISTBUS                                 ENABLED

#define VSF_BOARD_HAS_USB_HOST                          1
#define VSF_USBH_USE_HCD_WINUSB                         ENABLED
#   define VSF_WINUSB_HCD_CFG_DEV_NUM                   1
#   define VSF_WINUSB_HCD_DEV0_VID                      0x0A5C      // BCM20702 bthci
#   define VSF_WINUSB_HCD_DEV0_PID                      0x21E8
#define VSF_USBH_CFG_EDA_PRIORITY                       vsf_prio_0
#define VSF_USBH_CFG_ENABLE_ROOT_HUB                    DISABLED
#define VSF_USBH_USE_HUB                                DISABLED

#define VSF_LINUX_CFG_HEAP_SIZE                         (16 * 1024 * 1024)
#define VSF_HEAP_CFG_MCB_ALIGN_BIT                      9

/*----------------------------------------------------------------------------*
 * Application Configurations                                                 *
 *----------------------------------------------------------------------------*/

#define APP_CFG_WINSOUND_ARCH_PRIO                      vsf_arch_prio_0
#define APP_CFG_USBH_ARCH_PRIO                          vsf_arch_prio_0

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#endif      // __VSF_BOARD_CFG_H__
