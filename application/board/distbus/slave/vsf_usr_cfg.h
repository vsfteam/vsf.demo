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

/*============================ INCLUDES ======================================*/

#include "vsf_board_cfg.h"

/*============================ MACROS ========================================*/

#define APP_DISTBUS_CFG_POOL_NUM                        16
//#define APP_DISTBUS_CFG_DEBUG                           ENABLED

#undef VSF_HAL_USE_DAC
#define VSF_HAL_USE_DAC                                 DISABLED
// enable hal_distbus for constants and structures
#define VSF_HAL_USE_DISTBUS                             ENABLED
#   define VSF_HAL_DISTBUS_USE_IO                       ENABLED
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

#define VSF_USE_USB_DEVICE                              ENABLED
#define VSF_USE_USB_HOST                                ENABLED

#define VSF_USE_SIMPLE_STREAM                           ENABLED
#define VSF_USE_TRACE                                   ENABLED

#define VSF_USE_DISTBUS                                 ENABLED
#   define VSF_HAL_DISTBUS_CFG_MTU                      512

#if APP_DISTBUS_CFG_DEBUG == ENABLED
#   define VSF_DISTBUS_TRANSPORT_USE_STREAM             ENABLED
#else
#   define VSF_DISTBUS_TRANSPORT_USE_USBD_CDCACM        ENABLED
#endif

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ INCLUDES ======================================*/

#include "transport/vsf_distbus_transport_cfg.h"

#endif
/* EOF */
