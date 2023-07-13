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

#ifndef __VSF_BOARD_H__
#define __VSF_BOARD_H__

/*============================ INCLUDES ======================================*/

#include "vsf.h"

/*============================ MACROS ========================================*/
/*============================ TYPES =========================================*/

#if VSF_USE_USB_HOST == ENABLED || VSF_USE_USB_DEVICE == ENABLED
typedef struct vsf_board_t {
#if VSF_USE_USB_HOST == ENABLED
    vk_usbh_t usbh_dev;
#endif
#if VSF_USE_USB_DEVICE == ENABLED
    vk_dwcotg_dcd_t dwcotg_dcd;
#endif
} vsf_board_t;
#endif

/*============================ GLOBAL VARIABLES ==============================*/

#if VSF_USE_USB_HOST == ENABLED || VSF_USE_USB_DEVICE == ENABLED
extern vsf_board_t vsf_board;
#endif

#if VSF_USE_USB_DEVICE == ENABLED
extern const i_usb_dc_t VSF_USB_DC0;
#endif

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

extern void vsf_board_init(void);

#if VSF_USE_LINUX == ENABLED
extern void vsf_board_init_linux(void);
#endif

#endif      // __VSF_BOARD_H__
