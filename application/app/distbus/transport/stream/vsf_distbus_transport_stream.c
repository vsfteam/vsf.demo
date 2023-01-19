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
 */

/*============================ INCLUDES ======================================*/

#include "./vsf_distbus_transport_stream.h"

#if VSF_DISTBUS_TRANSPORT_USE_STREAM == ENABLED

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

describe_mem_stream(vsf_distbus_transport_stream_rx, 1024)
describe_mem_stream(vsf_distbus_transport_stream_tx, 1024)

static vsf_distbus_transport_t __vsf_distbus_transport;

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

#endif      // VSF_DISTBUS_TRANSPORT_USE_STREAM
