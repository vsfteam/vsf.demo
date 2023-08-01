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

// define VSF_BOARD_CFG_DISTBUS_USART to N to use the COMN as distbus transport
// if not defined, hal_distbus will not be used
// This is useful if you want to connect to a external board running distbus_hal
//   slave, and the hardware resources on the board can be conntrolled by
//   hal_distbus host.
//#define VSF_BOARD_CFG_DISTBUS_USART                     1

/*----------------------------------------------------------------------------*
 * Architecture Configurations                                                *
 *----------------------------------------------------------------------------*/

#define VSF_ASSERT                                      assert

#define VSF_ARCH_LIMIT_NO_SET_STACK
#define VSF_OS_CFG_ADD_EVTQ_TO_IDLE                     ENABLED
// cpu usage is not supported in windows/linux/macos
#define VSF_KERNEL_CFG_CPU_USAGE                        DISABLED

#ifdef VSF_BOARD_CFG_DISTBUS_USART
#   define VSF_LINUX_USART_CFG_TX_BLOCK                 ENABLED
// do not use linux_usart as hw_usart if VSF_BOARD_CFG_DISTBUS_USART is enabled,
//  because hw_usart will be hal_distbus_usart
#   define VSF_LINUX_USART_CFG_USE_AS_HW_USART          DISABLED
#endif

/*----------------------------------------------------------------------------*
 * Hal Driver Configurations                                                  *
 *----------------------------------------------------------------------------*/

#define VSF_HAL_USE_GPIO                                ENABLED
#define VSF_HAL_USE_USART                               ENABLED
#define VSF_HAL_USE_USBD                                DISABLED
#define VSF_HAL_USE_DEBUG_STREAM                        ENABLED
#   define VSF_DEBUG_STREAM_CFG_HW_PRIORITY             vsf_arch_prio_0
#define VSF_HAL_USE_DISTBUS                             ENABLED
#   define VSF_HAL_DISTBUS_USE_IO                       ENABLED
#   define VSF_HAL_DISTBUS_USE_GPIO                     ENABLED
#   define VSF_HAL_DISTBUS_USE_ADC                      ENABLED
#   define VSF_HAL_DISTBUS_USE_PWM                      ENABLED
#   define VSF_HAL_DISTBUS_USE_I2C                      ENABLED
#   define VSF_HAL_DISTBUS_USE_SPI                      ENABLED
#   define VSF_HAL_DISTBUS_USE_USART                    ENABLED
#   define VSF_HAL_DISTBUS_USE_MMC                      ENABLED
#   define VSF_HAL_DISTBUS_USE_I2S                      ENABLED
#   define VSF_HAL_DISTBUS_USE_USBD                     VSF_HAL_USE_USBD
#   define VSF_HAL_DISTBUS_USE_USBH                     ENABLED

/*----------------------------------------------------------------------------*
 * Components Configurations                                                  *
 *----------------------------------------------------------------------------*/

#define VSF_DISP_USE_SDL2                               ENABLED
#define VSF_FS_USE_LINFS                                ENABLED
#define VSF_USE_HEAP                                    ENABLED
#define VSF_USE_DISTBUS                                 ENABLED
// acutally VSF_DISTBUS_TRANSPORT_USE_STREAM is not in vsf, but in application/app/distbus
#   define VSF_DISTBUS_TRANSPORT_USE_STREAM             ENABLED
#define VSF_USE_USB_DEVICE                              VSF_HAL_USE_USBD

#define VSF_USBH_USE_HCD_LIBUSB                         ENABLED
#   define VSF_LIBUSB_HCD_CFG_DEV_NUM                   1
#   define VSF_LIBUSB_HCD_DEV0_VID                      0x0A5C      // BCM20702 bthci
#   define VSF_LIBUSB_HCD_DEV0_PID                      0x21E8
#define VSF_USBH_CFG_EDA_PRIORITY                       vsf_prio_0
#define VSF_USBH_CFG_ENABLE_ROOT_HUB                    DISABLED
#define VSF_USBH_USE_HUB                                DISABLED

#define VSF_LINUX_HOSTFS_TYPE                           "linfs"
#define VSF_LINUX_CFG_WRAPPER                           ENABLED
#define VSF_LINUX_SOCKET_CFG_WRAPPER                    ENABLED
#define VSF_LINUX_LIBC_CFG_WRAPPER                      ENABLED

/*----------------------------------------------------------------------------*
 * Application Configurations                                                 *
 *----------------------------------------------------------------------------*/

#define APP_DISP_SDL2_HEIGHT                            800
#define APP_DISP_SDL2_WIDTH                             480
#define APP_DISP_SDL2_COLOR                             VSF_DISP_COLOR_RGB565
#define APP_DISP_SDL2_AMPLIFIER                         1

#define APP_CFG_USBH_ARCH_PRIO                          vsf_arch_prio_0

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#endif      // __VSF_BOARD_CFG_H__
