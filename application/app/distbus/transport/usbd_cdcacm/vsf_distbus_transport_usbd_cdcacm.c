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
 * distbus transport implemented by usbd cdcacm
 *
 * Dependency:
 * Board:
 *   VSF_USB_DC0
 */

/*============================ INCLUDES ======================================*/

#include "./vsf_distbus_transport_usbd_cdcacm.h"

#if VSF_DISTBUS_TRANSPORT_USE_USBD_CDCACM == ENABLED

// for usbd
#include "component/vsf_component.h"
#include "vsf_board.h"

/*============================ MACROS ========================================*/

// __APP_CFG_CDC_BULK_SIZE is for internal usage
#if VSF_USBD_CFG_SPEED == USB_DC_SPEED_HIGH
#   define __APP_CFG_CDC_BULK_SIZE          512
#else
#   define __APP_CFG_CDC_BULK_SIZE          64
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

describe_block_stream(vsf_distbus_transport_stream_rx, 8, __APP_CFG_CDC_BULK_SIZE)
describe_block_stream(vsf_distbus_transport_stream_tx, 8, __APP_CFG_CDC_BULK_SIZE)

describe_usbd(__user_usbd_cdc, APP_CFG_USBD_VID, APP_CFG_USBD_PID, VSF_USBD_CFG_SPEED)
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
                        USB_CONFIG_ATT_WAKEUP, 100
    )
        usbd_cdc_acm_desc_iad(__user_usbd_cdc,
                        // interface_start
                        0 * USB_CDC_ACM_IFS_NUM,
                        // function string index(start from 0)
                        0,
                        // interrupt in ep, bulk in ep, bulk out ep
                        1, 2, 2,
                        // bulk ep size
                        __APP_CFG_CDC_BULK_SIZE,
                        // interrupt ep interval
                        16
        )
    usbd_func_desc(__user_usbd_cdc)
        usbd_func_str_desc(__user_usbd_cdc, 0, u"VSF-CDC0")
    usbd_std_desc_table(__user_usbd_cdc)
        usbd_func_str_desc_table(__user_usbd_cdc, 0)
    usbd_func(__user_usbd_cdc)
        usbd_cdc_acm_func(__user_usbd_cdc,
                        // function index
                        0,
                        // interrupt in ep, bulk in ep, bulk out ep
                        1, 2, 2,
                        // stream_rx, stream_tx
                        &vsf_distbus_transport_stream_rx, &vsf_distbus_transport_stream_tx,
                        // default line coding
                        USB_CDC_ACM_LINECODE(115200, 8, USB_CDC_ACM_PARITY_NONE, USB_CDC_ACM_STOPBIT_1)
        )
    usbd_ifs(__user_usbd_cdc)
        usbd_cdc_acm_ifs(__user_usbd_cdc, 0)
end_describe_usbd(__user_usbd_cdc, VSF_USB_DC0)

struct vsf_distbus_transport_t {
    struct {
        void *param;
        void (*on_inited)(void *p);
    } callback_on_inited;
    struct {
        uint8_t *buffer;
        uint32_t size;

        struct {
            void *param;
            void (*on_sent)(void *p);
        } callback;
    } tx;
    struct {
        uint8_t *buffer;
        uint32_t size;

        struct {
            void *param;
            void (*on_recv)(void *p);
        } callback;
    } rx;
} static __vsf_distbus_transport;

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

static uint_fast32_t __vsf_distbus_transport_try_send(struct vsf_distbus_transport_t *transport)
{
    uint_fast32_t trans_size = 0, wsiz;
    uint8_t *wbuf;

    while (transport->tx.size > 0) {
        wsiz = VSF_STREAM_GET_WBUF(&vsf_distbus_transport_stream_tx, &wbuf);
        if (wsiz <= 0) {
            break;
        }

        wsiz = vsf_min(wsiz, transport->tx.size);
        memcpy(wbuf, transport->tx.buffer, wsiz);
        trans_size += wsiz;
        transport->tx.size -= wsiz;
        transport->tx.buffer += wsiz;
    }
    return trans_size;
}

static uint_fast32_t __vsf_distbus_transport_try_recv(struct vsf_distbus_transport_t *transport)
{
    uint_fast32_t trans_size = 0, rsiz;
    uint8_t *rbuf;

    while (transport->rx.size > 0) {
        rsiz = VSF_STREAM_GET_RBUF(&vsf_distbus_transport_stream_rx, &rbuf);
        if (rsiz <= 0) {
            break;
        }

        rsiz = vsf_min(rsiz, transport->rx.size);
        memcpy(transport->rx.buffer, rbuf, rsiz);
        trans_size += rsiz;
        transport->rx.size -= rsiz;
        transport->rx.buffer += rsiz;
    }
    return trans_size;
}

static void __vsf_distbus_transport_stream_evthandler(vsf_stream_t *stream, void *param, vsf_stream_evt_t evt)
{
    struct vsf_distbus_transport_t *transport = param;
    uint32_t remain_size;

    switch (evt) {
    case VSF_STREAM_ON_IN:
        remain_size = transport->rx.size;
        if (    (remain_size > 0)
            &&  (__vsf_distbus_transport_try_recv(transport) == remain_size)
            &&  (transport->rx.callback.on_recv != NULL)) {
            transport->rx.callback.on_recv(transport->rx.callback.param);
        }
        break;
    case VSF_STREAM_ON_OUT:
        remain_size = transport->tx.size;
        if (    (remain_size > 0)
            &&  (__vsf_distbus_transport_try_send(transport) == remain_size)
            &&  (transport->tx.callback.on_sent != NULL)) {
            transport->tx.callback.on_sent(transport->tx.callback.param);
        }
        break;
    }
}

bool vsf_distbus_transport_init(void *p, void (*on_inited)(void *p))
{
    VSF_STREAM_INIT(&vsf_distbus_transport_stream_rx);
    vsf_distbus_transport_stream_rx.rx.param = &__vsf_distbus_transport;
    vsf_distbus_transport_stream_rx.rx.evthandler = __vsf_distbus_transport_stream_evthandler;
    VSF_STREAM_CONNECT_RX(&vsf_distbus_transport_stream_rx);

    VSF_STREAM_INIT(&vsf_distbus_transport_stream_tx);
    vsf_distbus_transport_stream_tx.tx.param = &__vsf_distbus_transport;
    vsf_distbus_transport_stream_tx.tx.evthandler = __vsf_distbus_transport_stream_evthandler;
    VSF_STREAM_CONNECT_TX(&vsf_distbus_transport_stream_tx);

    __vsf_distbus_transport.tx.size = 0;
    __vsf_distbus_transport.rx.size = 0;
    __vsf_distbus_transport.callback_on_inited.param = p;
    __vsf_distbus_transport.callback_on_inited.on_inited = on_inited;

    vk_usbd_init(&__user_usbd_cdc);
    vk_usbd_connect(&__user_usbd_cdc);
    return VSF_ERR_NONE;
}

bool vsf_distbus_transport_send(uint8_t *buffer, uint_fast32_t size, void *p, void (*on_sent)(void *p))
{
    __vsf_distbus_transport.tx.buffer = buffer;
    __vsf_distbus_transport.tx.size = size;
    return __vsf_distbus_transport_try_send(&__vsf_distbus_transport) == size;
}

bool vsf_distbus_transport_recv(uint8_t *buffer, uint_fast32_t size, void *p, void (*on_recv)(void *p))
{
    __vsf_distbus_transport.rx.buffer = buffer;
    __vsf_distbus_transport.rx.size = size;
    return __vsf_distbus_transport_try_recv(&__vsf_distbus_transport) == size;
}

#endif      // VSF_DISTBUS_TRANSPORT_USE_USBD_CDCACM
