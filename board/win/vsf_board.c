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

/*
 * board support for windows
 * 
 * Dependency:
 * If VSF_HAL_DISTBUS is enabled,
 *   VSF_OS_CFG_MAIN_MODE MUST be VSF_OS_CFG_MAIN_MODE_THREAD(default).
 *
 */

/*============================ INCLUDES ======================================*/

#include "./vsf_board.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ LOCAL VARIABLES ===============================*/

#if VSF_USE_USB_HOST == ENABLED
static const vk_winusb_hcd_param_t __winusb_hcd_param = {
    .priority                   = APP_CFG_USBH_ARCH_PRIO,
};
#endif

/*============================ GLOBAL VARIABLES ==============================*/

vsf_board_t vsf_board = {
    .usart                      = NULL,

#if VSF_USE_UI == ENABLED
    .display_dev                = &vsf_board.disp_wingdi.use_as__vk_disp_t,
    .disp_wingdi                = {
        .param                  = {
            .height             = APP_DISP_WINGDI_HEIGHT,
            .width              = APP_DISP_WINGDI_WIDTH,
            .drv                = &vk_disp_drv_wingdi,
            .color              = APP_DISP_WINGDI_COLOR,
        },
    },
#endif
#if VSF_USE_AUDIO == ENABLED
    .audio_dev                  = &vsf_board.audio_winsound.use_as__vk_audio_dev_t,
    .audio_winsound             = {
        .drv                    = &vk_winsound_drv,
        .hw_prio                = APP_CFG_WINSOUND_ARCH_PRIO,
    },
#endif
#if VSF_USE_USB_HOST == ENABLED
    .usbh_dev                   = {
        .drv                    = &vk_winusb_hcd_drv,
        .param                  = (void *)&__winusb_hcd_param,
    },
#endif
#if VSF_USE_FS == ENABLED
    .fsop                       = &vk_winfs_op,
    .fsinfo                     = (void *)&vsf_board.winfs_info,
    .winfs_info                 = {
        .root.name              = ".",
    },
#endif
};

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

void vsf_board_init(void)
{
    uint8_t usart_devnum;
    vsf_usart_win_device_t usart_devices[8];

    vsf_hw_usart_scan_devices();
    while (vsf_hw_usart_is_scanning(&usart_devnum));
    if (usart_devnum > 0) {
        vsf_hw_usart_get_devices((vsf_usart_win_device_t *)&usart_devices, dimof(usart_devices));
#ifdef VSF_BOARD_CFG_USART
        for (uint8_t i = 0; i < usart_devnum; i++) {
            if (VSF_BOARD_CFG_USART == usart_devices[i].port) {
                vsf_board.usart = usart_devices[i].instance;
                break;
            }
        }
#else
        
        vsf_board.usart = usart_devices[0].instance;
#endif
    }
#ifdef VSF_BOARD_CFG_USART
    // note that even if assert here, assert message will not be diaplayed, because trace is not started here
    VSF_ASSERT(vsf_board.usart != NULL);
#endif
}
