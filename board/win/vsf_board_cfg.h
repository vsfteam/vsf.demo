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
 * Architecture Configurations                                                *
 *----------------------------------------------------------------------------*/

#define VSF_ARCH_LIMIT_NO_SET_STACK
#define VSF_OS_CFG_ADD_EVTQ_TO_IDLE                     ENABLED
// cpu usage is not supported in windows/linux/macos
#define VSF_KERNEL_CFG_CPU_USAGE                        DISABLED

/*----------------------------------------------------------------------------*
 * Hal Driver Configurations                                                  *
 *----------------------------------------------------------------------------*/

#define VSF_HAL_USE_USART                               ENABLED
#define VSF_HAL_USE_DEBUG_STREAM                        ENABLED
#   define VSF_DEBUG_STREAM_CFG_HW_PRIORITY             vsf_arch_prio_0

/*----------------------------------------------------------------------------*
 * Components Configurations                                                  *
 *----------------------------------------------------------------------------*/

#define VSF_DISP_USE_WINGDI                             ENABLED
#define VSF_AUDIO_USE_WINSOUND                          ENABLED
#define VSF_FS_USE_WINFS                                ENABLED
#define VSF_USE_HEAP                                    ENABLED

#define VSF_USBH_USE_HCD_WINUSB                         ENABLED
#   define VSF_WINUSB_HCD_CFG_DEV_NUM                   1
#   define VSF_WINUSB_HCD_DEV0_VID                      0x0A5C      // BCM20702 bthci
#   define VSF_WINUSB_HCD_DEV0_PID                      0x21E8
#define VSF_USBH_CFG_EDA_PRIORITY                       vsf_prio_0
#define VSF_USBH_CFG_ENABLE_ROOT_HUB                    DISABLED
#define VSF_USBH_USE_HUB                                DISABLED

/*----------------------------------------------------------------------------*
 * Application Configurations                                                 *
 *----------------------------------------------------------------------------*/

#define APP_DISP_WINGDI_HEIGHT                          800
#define APP_DISP_WINGDI_WIDTH                           480
#define APP_DISP_WINGDI_COLOR                           VSF_DISP_COLOR_RGB565

#define APP_CFG_WINSOUND_ARCH_PRIO                      vsf_arch_prio_0
#define APP_CFG_USBH_ARCH_PRIO                          vsf_arch_prio_0

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#endif      // __VSF_BOARD_CFG_H__
