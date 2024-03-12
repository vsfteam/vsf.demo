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
 * usbd_cdc demo
 *
 * Dependency:
 * Board:
 *   VSF_USB_DC0
 */

/*============================ INCLUDES ======================================*/

#include "vsf.h"
#include "vsf_board.h"

/*============================ MACROS ========================================*/

// __APP_CFG_CDC_BULK_SIZE is for internal usage
#if VSF_USBD_CFG_SPEED == USB_DC_SPEED_HIGH
#   define __APP_CFG_CDC_BULK_SIZE          512
#else
#   define __APP_CFG_CDC_BULK_SIZE          64
#endif

#if VSF_USBD_CFG_AUTOSETUP != ENABLED
#   error VSF_USBD_CFG_AUTOSETUP is needed for this demo
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

describe_block_stream(__user_usbd_cdc_stream0, 2, __APP_CFG_CDC_BULK_SIZE)

describe_usbd(__user_usbd_cdc, APP_CFG_USBD_VID, APP_CFG_USBD_PID, VSF_USBD_CFG_SPEED)
    usbd_func(__user_usbd_cdc,
        usbd_cdc_acm_func(__user_usbd_cdc,
                        // function index
                        0,
                        // function string
                        u"VSF-CDC0",
                        // interface_start
                        0 * USB_CDC_ACM_IFS_NUM,
                        // function string index(start from 0)
                        0,
                        // interrupt in ep, bulk in ep, bulk out ep
                        1, 2, 2,
                        // bulk ep size
                        __APP_CFG_CDC_BULK_SIZE,
                        // interrupt ep interval
                        16,
                        // stream_rx, stream_tx
                        &__user_usbd_cdc_stream0, &__user_usbd_cdc_stream0,
                        // default line coding
                        USB_CDC_ACM_LINECODE(115200, 8, USB_CDC_ACM_PARITY_NONE, USB_CDC_ACM_STOPBIT_1)
        )
    )

    usbd_common_desc_iad(__user_usbd_cdc,
                        // str_product, str_vendor, str_serial
                        u"VSF-USBD-Simplest", u"SimonQian", u"1.0.0",
                        // ep0_size
                        64,
                        // total function descriptor size
                        USB_DESC_CDC_ACM_IAD_LEN,
                        // total function interface number
                        USB_CDC_ACM_IFS_NUM,
                        // attribute, max_power
                        USB_CONFIG_ATT_WAKEUP, 100,
        usbd_cdc_acm_desc_iad(__user_usbd_cdc, 0)
    )

    usbd_std_desc_table(__user_usbd_cdc,
        usbd_func_str_desc_table(__user_usbd_cdc, 0)
    )

    usbd_ifs(__user_usbd_cdc,
        usbd_cdc_acm_ifs(__user_usbd_cdc, 0)
    )
end_describe_usbd(__user_usbd_cdc, VSF_USB_DC0)

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

int VSF_USER_ENTRY(void)
{
    vsf_board_init();

    VSF_STREAM_INIT(&__user_usbd_cdc_stream0);
    vk_usbd_init(&__user_usbd_cdc);
    vk_usbd_connect(&__user_usbd_cdc);
    return 0;
}
