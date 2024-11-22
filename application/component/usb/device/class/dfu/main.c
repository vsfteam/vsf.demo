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
 * usbd_dfu demo
 *
 * Dependency:
 * Board:
 *   VSF_USB_DC0
 */

/*============================ INCLUDES ======================================*/

#define __VSF_USBD_CLASS_IMPLEMENT
#include "vsf.h"
#include "vsf_board.h"

/*============================ MACROS ========================================*/

#define APP_CFG_USBD_VID                    0xA7A8
#define APP_CFG_USBD_PID                    0x2348
#define APP_CFG_USBD_EP0_SIZE               64
#define APP_CFG_USBD_VENDOR_STR             u"vsf"
#define APP_CFG_USBD_PRODUCT_STR            u"vsf_dfu"
#define APP_CFG_USBD_SERIAL_STR             u"0000"
#define APP_CFG_USBD_WEBUSB_URL             "devanlai.github.io/webdfu/dfu-util/"

#define APP_CFG_USBD_SPEED                  USB_DC_SPEED_FULL

#define APP_CFG_USBD_DFU_TRANSFER_SIZE      256
#define APP_CFG_USBD_DFU_PROTOCOL           USB_DFU_PROTOCOL_DFU

#if VSF_USBD_CFG_AUTOSETUP != ENABLED
#   error VSF_USBD_CFG_AUTOSETUP is needed for this demo
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

static vsf_err_t __dfu_vendor_prepare(vk_usbd_dev_t *dev);

/*============================ LOCAL VARIABLES ===============================*/

describe_usbd(__user_usbd_dfu, APP_CFG_USBD_VID, APP_CFG_USBD_PID, APP_CFG_USBD_SPEED, 0x210, 0x0409, 0x0000)
    usbd_func(__user_usbd_dfu,
              usbd_dfu_func(  __user_usbd_dfu,
                        // function index
                        0,
                        // protocol
                        APP_CFG_USBD_DFU_PROTOCOL,
                        // dfu attributes
                        1,
                        // transfer size
                        APP_CFG_USBD_DFU_TRANSFER_SIZE
        )
        usbd_func_str_desc(__user_usbd_dfu, 0, u"VSF-DFU")
        usbd_str_desc(  __user_usbd_dfu, bos, u"MSFT100!")      // last !(0x21) is the vendor_code
        usbd_bos_desc(  __user_usbd_dfu,
                        // number of bos capability
                        1,
                        // capabilities size
                        USB_BOS_CAP_WEBUSB_LEN,
                        // capabilities: webusb(vendor_code, landing_page)
                        bos_desc_webusb(1, 1)
        )
        usbd_webusb_url_desc(__user_usbd_dfu, 1, WEBUSB_URL_PREFIX_HTTPS, sizeof(APP_CFG_USBD_WEBUSB_URL) - 1, APP_CFG_USBD_WEBUSB_URL)
        usbd_msos10_compatid_desc(__user_usbd_dfu, 1,
            __usbd_msos10_compatid_func_desc(0, 'W', 'I', 'N', 'U', 'S', 'B')
        )
    )

    usbd_common_desc(   __user_usbd_dfu,
                        // str_product, str_vendor, str_serial
                        APP_CFG_USBD_PRODUCT_STR, APP_CFG_USBD_VENDOR_STR, APP_CFG_USBD_SERIAL_STR,
                        // class, subclass, protocol
                        0, 0, 0,
                        // ep0_size
                        APP_CFG_USBD_EP0_SIZE,
                        // total function descriptor size
                        USB_DESC_DFU_LEN,
                        // total function interface number
                        USB_DFU_IFS_NUM,
                        // attribute, max_power
                        0, 200,
        usbd_dfu_desc(  __user_usbd_dfu,
                        // interface_start
                        0 * USB_DFU_IFS_NUM,
                        // function string index(start from 0)
                        0,
                        // dfu protocol
                        APP_CFG_USBD_DFU_PROTOCOL,
                        // dfu attributes
                        1,
                        // detach timeout
                        0,
                        // transfer size
                        APP_CFG_USBD_DFU_TRANSFER_SIZE
        )
    )

    usbd_std_desc_table(__user_usbd_dfu,
        usbd_func_str_desc_table(__user_usbd_dfu, 0)
        usbd_str_desc_table(__user_usbd_dfu, 0xEE, bos)
        usbd_str_desc_table(__user_usbd_dfu, 0xEE, bos, 0)
        usbd_bos_desc_table(__user_usbd_dfu)
    )

    usbd_ifs(__user_usbd_dfu,
        usbd_dfu_ifs(__user_usbd_dfu, 0)
    )
end_describe_usbd(__user_usbd_dfu, VSF_USB_DC0,
    .vendor.prepare     = __dfu_vendor_prepare,
)

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

static vsf_err_t __dfu_vendor_prepare(vk_usbd_dev_t *dev)
{
    vk_usbd_ctrl_handler_t *ctrl_handler = &dev->ctrl_handler;
    struct usb_ctrlrequest_t *request = &ctrl_handler->request;
    uint8_t *buffer = NULL;
    uint_fast32_t size = 0;
    vsf_err_t result = VSF_ERR_FAIL;

    switch (request->bRequest) {
    case 0x01:              // vendor_code of webusb
        switch (request->wIndex) {
        case 0x02:
            buffer = (uint8_t *)usbd_webusb_url_desc_buffer(__user_usbd_dfu, 1);
            size = usbd_webusb_url_desc_len(__user_usbd_dfu, 1);
            result = VSF_ERR_NONE;
            break;
        }
        break;
    case (uint8_t)'!':      // vendor_code of winusb
        switch (request->wIndex) {
        case USB_MSOS10_EXT_COMPATID_INDEX:
            buffer = (uint8_t *)usbd_msos10_compatid_desc_buffer(__user_usbd_dfu);
            size = usbd_msos10_compatid_desc_len(__user_usbd_dfu);
            result = VSF_ERR_NONE;
            break;
        case USB_MSOS10_EXT_PROPERTIES_INDEX:
            result = VSF_ERR_NONE;
            break;
        }
    }
    ctrl_handler->trans.buffer = buffer;
    ctrl_handler->trans.size = size;
    return result;
}

int VSF_USER_ENTRY(void)
{
    vsf_board_init();
#if VSF_USE_TRACE == ENABLED
    vsf_start_trace();
#endif

    vk_usbd_init(&__user_usbd_dfu);
    vk_usbd_connect(&__user_usbd_dfu);
    return 0;
}
