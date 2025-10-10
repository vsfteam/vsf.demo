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
#if VSF_USE_BTSTACK == ENABLED
#   include "btstack.h"
#endif

/*============================ MACROS ========================================*/
/*============================ TYPES =========================================*/

typedef struct vsf_board_t {
#if VSF_HW_USART_COUNT > 0
    vsf_usart_t *usart;
#endif
#if VSF_HW_I2C_COUNT > 0
    vsf_i2c_t *i2c;
#endif
#if VSF_HW_SDIO_COUNT > 0
    vsf_sdio_t *sdio;
#endif
#if VSF_USE_UI == ENABLED
    vk_disp_t *display_dev;
    vk_disp_wingdi_t disp_wingdi;
#endif
#if VSF_USE_AUDIO == ENABLED
    vk_audio_dev_t *audio_dev;
#   if VSF_AUDIO_USE_WINSOUND == ENABLED
    vk_winsound_dev_t audio_winsound;
#   endif
#endif
#if VSF_USE_USB_HOST == ENABLED
    vk_usbh_t usbh_dev;
#endif
#if VSF_USE_FS == ENABLED
    const vk_fs_op_t *fsop;
    void * fsinfo;
    vk_winfs_info_t winfs_info;
#endif
#if VSF_USE_BTSTACK == ENABLED
    struct {
        const btstack_run_loop_t *run_loop_instance;
        const hci_transport_t *hci_transport_instance;
        const btstack_chipset_t *chipset_instance;
        void *hci_transport_config;
    } btstack;
#endif
} vsf_board_t;

/*============================ GLOBAL VARIABLES ==============================*/

extern vsf_board_t vsf_board;

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

extern void vsf_board_init(void);

#endif      // __VSF_BOARD_H__
