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
 *
 *   Source:
 *
 *   Include Directories:
 *
 */

/*============================ INCLUDES ======================================*/

#include "./vsf_board.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/

#if VSF_USE_USB_HOST == ENABLED
static const vk_winusb_hcd_param_t __winusb_hcd_param = {
    .priority                   = APP_CFG_USBH_ARCH_PRIO,
};
#endif

/*============================ GLOBAL VARIABLES ==============================*/

#if VSF_USE_FBCON == ENABLED
// fbcon uses a dedicated fifo stream as its data source.
static uint8_t __fbcon_stream_tx_buf[4096];
static vsf_fifo_stream_t __fbcon_stream_tx = {
    .op     = &vsf_fifo_stream_op,
    .buffer = __fbcon_stream_tx_buf,
    .size   = sizeof(__fbcon_stream_tx_buf),
};

// Adapter stream: tee writes to both hardware console and fbcon.
static void __adapter_tx_init(vsf_stream_t *stream)
{
    vsf_stream_connect_rx(stream);
}

static uint_fast32_t __adapter_tx_write(vsf_stream_t *stream,
            uint8_t *buf, uint_fast32_t size)
{
    // Forward to hardware console (VSF_DEBUG_STREAM_TX from driver.c)
    vsf_stream_write((vsf_stream_t *)&VSF_DEBUG_STREAM_TX, buf, size);
    // Forward to fbcon fifo stream
    vsf_stream_write(&__fbcon_stream_tx.use_as__vsf_stream_t, buf, size);
    return size;
}

static uint_fast32_t __adapter_tx_get_data_length(vsf_stream_t *stream)
{
    return 0;
}

static uint_fast32_t __adapter_tx_get_avail_length(vsf_stream_t *stream)
{
    return 0xFFFFFFFF;
}

static const vsf_stream_op_t __vsf_board_debug_stream_adapter_tx_op = {
    .init               = __adapter_tx_init,
    .fini               = __adapter_tx_init,
    .write              = __adapter_tx_write,
    .get_data_length    = __adapter_tx_get_data_length,
    .get_avail_length   = __adapter_tx_get_avail_length,
};

vsf_fifo_stream_t __vsf_board_debug_stream_adapter_tx = {
    .op     = &__vsf_board_debug_stream_adapter_tx_op,
};
#endif

vsf_board_t vsf_board = {
    .usart                      = NULL,

#if VSF_USE_UI == ENABLED
    .display_dev                = &vsf_board.disp_wingdi.use_as__vk_disp_t,
    .disp_wingdi                = {
        .param                  = {
            .height             = VSF_BOARD_DISP_HEIGHT,
            .width              = VSF_BOARD_DISP_WIDTH,
            .drv                = &vk_disp_drv_wingdi,
            .color              = VSF_BOARD_DISP_COLOR,
        },
    },
#endif
#if VSF_USE_AUDIO == ENABLED
#   if VSF_AUDIO_USE_WINSOUND == ENABLED
    .audio_dev                  = &vsf_board.audio_winsound.use_as__vk_audio_dev_t,
    .audio_winsound             = {
        .drv                    = &vk_winsound_drv,
        .hw_prio                = APP_CFG_WINSOUND_ARCH_PRIO,
    },
#   endif
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
/*============================ IMPLEMENTATION ================================*/

void vsf_board_init(void)
{
#if VSF_USE_FBCON == ENABLED
    vsf_board.fbcon.disp = vsf_board.display_dev;
    vsf_board.fbcon.stream = &__fbcon_stream_tx.use_as__vsf_stream_t;
    vsf_fbcon_init(&vsf_board.fbcon);
#endif
#if VSF_HAL_USE_USART == ENABLED
    vsf_hostos_usart_device_t usart_devices[8];
    uint8_t usart_devnum = vsf_hostos_usart_scan_devices((vsf_hostos_usart_device_t*)&usart_devices, dimof(usart_devices));

    if (usart_devnum > 0) {
        vsf_board.usart = usart_devices[0].instance;
    }
#endif
}
