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
 * If VSF_HAL_DISTBUS is enabled:
 *   Configuration:
 *     VSF_OS_CFG_MAIN_MODE MUST be VSF_OS_CFG_MAIN_MODE_THREAD(default).
 *     VSF_BOARD_CFG_DISTBUS_USART can be defined as the usart used as transport of distbus.
 *
 *   Source:
 *     application/app/distbus/transport/*
 *
 *   Include Directories:
 *     application/app/distbus
 *
 */

/*============================ INCLUDES ======================================*/

#include "./vsf_board.h"

/*============================ MACROS ========================================*/

#if __APP_USE_DISTBUS
#   if VSF_OS_CFG_MAIN_MODE != VSF_OS_CFG_MAIN_MODE_THREAD
#   error VSF_OS_CFG_MAIN_MODE MUST be VSF_OS_CFG_MAIN_MODE_THREAD
#   endif

#   ifndef APP_DISTBUS_CFG_POOL_NUM
#       define APP_DISTBUS_CFG_POOL_NUM         32
#   endif
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

#if __APP_USE_DISTBUS
typedef struct __user_distbus_msg_t {
    implement(vsf_distbus_msg_t)
    uint8_t buffer[VSF_HAL_DISTBUS_CFG_MTU + offset_of(vsf_distbus_msg_t, header)];
} __user_distbus_msg_t;

dcl_vsf_pool(__user_distbus_msg_pool)
def_vsf_pool(__user_distbus_msg_pool, __user_distbus_msg_t)

typedef struct __user_distbus_t {
    vsf_distbus_t                       distbus;
    vsf_usart_stream_t                  usart_stream;
    vsf_distbus_transport_t             transport;
    vsf_hal_distbus_t                   hal;
    vsf_pool(__user_distbus_msg_pool)   msg_pool;

    vsf_eda_t                           *eda_pending;
} __user_distbus_t;

#endif

/*============================ PROTOTYPES ====================================*/

#if __APP_USE_DISTBUS
static void __user_distbus_on_error(vsf_distbus_t *distbus);
static void * __user_distbus_alloc_msg(uint_fast32_t size);
static void __user_distbus_free_msg(void *msg);
static void __user_hal_distbus_on_remote_connected(vsf_hal_distbus_t *hal_distbus);
#endif

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

#if __APP_USE_DISTBUS
describe_mem_stream(vsf_distbus_transport_stream_rx, 1024)
describe_mem_stream(vsf_distbus_transport_stream_tx, 1024)

static __user_distbus_t __user_distbus = {
    .distbus                    = {
        .op                     = {
            .mem                = {
                .alloc_msg      = __user_distbus_alloc_msg,
                .free_msg       = __user_distbus_free_msg,
            },
            .bus                = {
                .transport      = &__user_distbus.transport,
                .init           = vsf_distbus_transport_init,
                .send           = vsf_distbus_transport_send,
                .recv           = vsf_distbus_transport_recv,
            },
            .on_error           = __user_distbus_on_error,
        },
    },
    .hal                        = {
        .on_remote_connected    = __user_hal_distbus_on_remote_connected,
    },
#if VSF_DISTBUS_TRANSPORT_USE_STREAM == ENABLED
    .transport                  = {
        .stream_rx              = &vsf_distbus_transport_stream_rx.use_as__vsf_stream_t,
        .stream_tx              = &vsf_distbus_transport_stream_tx.use_as__vsf_stream_t,
    },
    .usart_stream               = {
        .stream_rx              = &vsf_distbus_transport_stream_rx.use_as__vsf_stream_t,
        .stream_tx              = &vsf_distbus_transport_stream_tx.use_as__vsf_stream_t,
    },
#endif
};
#endif

/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

#if __APP_USE_DISTBUS
imp_vsf_pool(__user_distbus_msg_pool, __user_distbus_msg_t)

static void __user_distbus_on_error(vsf_distbus_t *distbus)
{
    VSF_ASSERT(false);
}

static void * __user_distbus_alloc_msg(uint_fast32_t size)
{
    VSF_ASSERT(size <= sizeof(((__user_distbus_msg_t *)NULL))->buffer);
    return VSF_POOL_ALLOC(__user_distbus_msg_pool, &__user_distbus.msg_pool);
}

static void __user_distbus_free_msg(void *msg)
{
    VSF_POOL_FREE(__user_distbus_msg_pool, &__user_distbus.msg_pool, msg);
}

void vsf_hal_distbus_on_new(vsf_hal_distbus_t *hal_distbus, vsf_hal_distbus_type_t type, uint8_t num, void *devs)
{
    union {
        void *ptr;
#define VSF_BOARD_HAL_DISTBUS_DEFINE_DEVS(__TYPE)                               \
        VSF_MCONNECT(vsf_hal_distbus_, __TYPE, _t) *__TYPE;

#define __VSF_HAL_DISTBUS_ENUM      VSF_BOARD_HAL_DISTBUS_DEFINE_DEVS
#include "hal/driver/vsf/distbus/vsf_hal_distbus_enum.inc"
    } u_devs;
    u_devs.ptr = devs;

    switch (type) {
#define VSF_BOARD_HAL_DISTBUS_ENUM(__TYPE)                                      \
    case VSF_MCONNECT(VSF_HAL_DISTBUS_, __TYPE):                                \
        if (!vsf_board.chip.__TYPE.dev_num) {                                   \
            vsf_board.chip.__TYPE.dev_num = vsf_min(num, dimof(vsf_board.chip.__TYPE.dev));\
            for (uint8_t i = 0; i < vsf_board.chip.__TYPE.dev_num; i++) {       \
                vsf_board.chip.__TYPE.dev[i] = (VSF_MCONNECT(vsf_, __TYPE, _t) *)&u_devs.__TYPE[i];\
                __vsf_arch_trace(0, "[hal_distbus] new " VSF_STR(__TYPE) "%d %p" VSF_TRACE_CFG_LINEEND, i, vsf_board.chip.__TYPE.dev[i]);\
            }                                                                   \
        }                                                                       \
        break;

#define __VSF_HAL_DISTBUS_ENUM      VSF_BOARD_HAL_DISTBUS_ENUM
#include "hal/driver/vsf/distbus/vsf_hal_distbus_enum.inc"
    }

    switch (type) {
    case VSF_HAL_DISTBUS_USART:
        vsf_board.usart = vsf_board.chip.usart.dev[0];
        break;
    }

    __user_distbus_t *user_distbus = container_of(hal_distbus, __user_distbus_t, hal);
    if (user_distbus->eda_pending != NULL) {
        vsf_eda_t *eda_pending = user_distbus->eda_pending;
        user_distbus->eda_pending = NULL;
        vsf_eda_post_evt(eda_pending, VSF_EVT_USER);
    }
}

static void __user_hal_distbus_on_remote_connected(vsf_hal_distbus_t *hal_distbus)
{
    vsf_hal_distbus_start(hal_distbus);
}
#endif

void vsf_board_init(void)
{
    uint8_t usart_devnum;
    vsf_usart_win_device_t usart_devices[8];

    vsf_hw_usart_scan_devices();
    while (vsf_hw_usart_is_scanning(&usart_devnum));
    if (usart_devnum > 0) {
        vsf_hw_usart_get_devices((vsf_usart_win_device_t *)&usart_devices, dimof(usart_devices));

#ifdef VSF_BOARD_CFG_DISTBUS_USART
        for (uint8_t i = 0; i < usart_devnum; i++) {
            if (VSF_BOARD_CFG_DISTBUS_USART == usart_devices[i].port) {
                vsf_board.distbus_usart = usart_devices[i].instance;
                break;
            }
        }
#else
        vsf_board.usart = usart_devices[0].instance;
#endif
    }
#ifdef VSF_BOARD_CFG_DISTBUS_USART
    // note that even if assert here, assert message will not be diaplayed, because trace is not started here
    VSF_ASSERT(vsf_board.distbus_usart != NULL);
#endif

#if __APP_USE_DISTBUS
    VSF_POOL_INIT(__user_distbus_msg_pool, &__user_distbus.msg_pool, APP_DISTBUS_CFG_POOL_NUM);
    VSF_STREAM_INIT(&vsf_distbus_transport_stream_rx);
    VSF_STREAM_INIT(&vsf_distbus_transport_stream_tx);

    __user_distbus.usart_stream.usart = vsf_board.distbus_usart;
    vsf_usart_stream_init(&__user_distbus.usart_stream, &(vsf_usart_cfg_t) {
        .mode               = VSF_USART_8_BIT_LENGTH | VSF_USART_NO_PARITY | VSF_USART_1_STOPBIT | VSF_USART_TX_ENABLE | VSF_USART_RX_ENABLE,
        .baudrate           = 921600,
    });

    vsf_distbus_init(&__user_distbus.distbus);
    vsf_hal_distbus_register(&__user_distbus.distbus, &__user_distbus.hal);
    __user_distbus.eda_pending = vsf_eda_get_cur();
    vsf_distbus_start(&__user_distbus.distbus);

    vsf_thread_wfe(VSF_EVT_USER);
#endif
}
