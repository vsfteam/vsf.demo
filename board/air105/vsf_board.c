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

/*============================ INCLUDES ======================================*/

#define __VSF_SIMPLE_STREAM_CLASS_INHERIT__
#include "./vsf_board.h"

#if VSF_USE_LINUX == ENABLED
#   include <unistd.h>
#endif

#ifdef FALSE
#   undef FALSE
#endif
#ifdef TRUE
#   undef TRUE
#endif
#include "air105.h"
#include "io_map.h"
#include "platform_define.h"
#include "resource_map.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ LOCAL VARIABLES ===============================*/

#if VSF_USE_USB_DEVICE == ENABLED
static const vk_musb_fdrc_dcd_param_t __musb_fdrc_dcd_param = {
    .op                     = &VSF_USB_DC0_IP,
};
#endif

#if VSF_USE_USB_HOST == ENABLED
static const vk_musb_fdrc_hcd_param_t __musb_fdrc_hcd_param = {
    .op                     = &VSF_USB_HC0_IP,
    .priority               = vsf_arch_prio_0,
};
#endif

/*============================ GLOBAL VARIABLES ==============================*/

#if VSF_USE_USB_HOST == ENABLED || VSF_USE_USB_DEVICE == ENABLED
vsf_board_t vsf_board = {
#if VSF_USE_USB_DEVICE == ENABLED
    .musb_fdrc_dcd.param    = &__musb_fdrc_dcd_param,
#endif
#if VSF_USE_USB_HOST == ENABLED
    .host.drv               = &vk_musb_fdrc_hcd_drv,
    .host.param             = (void*)&__musb_fdrc_hcd_param,
#endif
};
#endif

/*============================ IMPLEMENTATION ================================*/

#if VSF_USE_USB_DEVICE == ENABLED
vsf_usb_dc_from_musbfdrc_ip(0, vsf_board.musb_fdrc_dcd, VSF_USB_DC0)
#endif

#if VSF_USE_USB_HOST == ENABLED

#   ifndef MUSB_FDRC_CFG_EP64_NUM
#       define MUSB_FDRC_CFG_EP64_NUM               2
#   endif
#   ifndef MUSB_FDRC_CFG_EP256_NUM
#       define MUSB_FDRC_CFG_EP256_NUM              3
#   endif

typedef struct musb_fdrc_ep_buffer_t {
    uint32_t ep64_mask, ep256_mask;
} musb_fdrc_ep_buffer_t;

static musb_fdrc_ep_buffer_t __musb_fdrc_ep_buffer = {
    .ep64_mask      = ~VSF_BITMASK(MUSB_FDRC_CFG_EP64_NUM),
    .ep256_mask     = ~VSF_BITMASK(MUSB_FDRC_CFG_EP256_NUM),
};

void vsf_musb_fdrc_hcd_free_fifo(vk_usbh_hcd_t *hcd, uint_fast16_t fifo)
{
    uint_fast16_t offset = (fifo & 0xFFF) << 3;
    int_fast8_t idx;

    if (offset < 128) {
        VSF_USB_ASSERT(false);
    }

    offset -= 128;
    if (offset < 64 * MUSB_FDRC_CFG_EP64_NUM) {
        idx = offset / 64;
        __musb_fdrc_ep_buffer.ep64_mask &= ~(1 << idx);
        return;
    }

    offset -= 64 * MUSB_FDRC_CFG_EP64_NUM;
    if (offset < 256 * MUSB_FDRC_CFG_EP256_NUM) {
        idx = offset / 256;
        __musb_fdrc_ep_buffer.ep256_mask &= ~(1 << idx);
        return;
    }

    VSF_USB_ASSERT(false);
}

uint_fast16_t vsf_musb_fdrc_hcd_alloc_fifo(vk_usbh_hcd_t *hcd, vk_usbh_pipe_t pipe)
{
    // skip the buffer for ep0
    uint_fast16_t offset = 128;
    int_fast8_t idx;

    if (pipe.size <= 64) {
        idx = vsf_ffz32(__musb_fdrc_ep_buffer.ep64_mask);
        if (idx >= 0) {
            __musb_fdrc_ep_buffer.ep64_mask |= 1 << idx;
            offset += 64 * idx;
            return offset >> 3;
        }
        goto try_256buffer;
    } else if (pipe.size <= 256) {
    try_256buffer:

        // skip the 64-byte buffer
        offset += 64 * MUSB_FDRC_CFG_EP64_NUM;

        idx = vsf_ffz32(__musb_fdrc_ep_buffer.ep256_mask);
        if (idx >= 0) {
            __musb_fdrc_ep_buffer.ep256_mask |= 1 << idx;
            offset += 256 * idx;
            return offset >> 3;
        }
    }
    VSF_USB_ASSERT(false);
    return 0;
}
#endif

/*----------------------------------------------------------------------------*
 * debug stream                                                               *
 *----------------------------------------------------------------------------*/

#define __VSF_SIMPLE_STREAM_CLASS_INHERIT__
#include "service/simple_stream/vsf_simple_stream.h"

#define VSF_DEBUG_STREAM_CFG_RX_BUF_SIZE        32
static uint8_t __vsf_debug_stream_rx_buff[VSF_DEBUG_STREAM_CFG_RX_BUF_SIZE];
vsf_mem_stream_t VSF_DEBUG_STREAM_RX = {
    .op         = &vsf_mem_stream_op,
    .buffer     = __vsf_debug_stream_rx_buff,
    .size       = sizeof(__vsf_debug_stream_rx_buff),
};

static void GPIO_Iomux(uint32_t Pin, uint32_t Function)
{
    uint8_t Port = (Pin >> 4);
    uint32_t Mask = ~(0x03 << ((Pin & 0x0000000f) * 2));
    Function = Function << ((Pin & 0x0000000f) * 2);
    GPIO->ALT[Port] = (GPIO->ALT[Port] & Mask) | Function;
}

static void __vsf_debug_stream_uart_evthandler( void *target_ptr,
                                                vsf_usart_t *usart_ptr,
                                                vsf_usart_irq_mask_t irq_mask)
{
    vsf_usart_t *usart = target_ptr;

    switch (irq_mask) {
    case VSF_USART_IRQ_MASK_RX:
        while (true) {
            uint_fast16_t data_size = vsf_usart_rxfifo_get_data_count(usart);
            if (!data_size) { break; }

            uint8_t *ptr;
            uint_fast32_t buf_size = vsf_stream_get_wbuf(&VSF_DEBUG_STREAM_RX.use_as__vsf_stream_t, &ptr);
            data_size = vsf_min(data_size, buf_size);

            vsf_usart_rxfifo_read(usart, ptr, data_size);
            vsf_stream_write(&VSF_DEBUG_STREAM_RX.use_as__vsf_stream_t, NULL, data_size);
        }
        break;
    }
}

static void __VSF_DEBUG_STREAM_TX_INIT(void)
{
    static bool is_inited = false;
    if (!is_inited) {
        is_inited = true;

        GPIO_Iomux(DBG_UART_TX_PIN, DBG_UART_TX_AF);
        GPIO_Iomux(DBG_UART_RX_PIN, DBG_UART_RX_AF);

        vsf_usart_init(&vsf_hw_usart0, &(vsf_usart_cfg_t){
            .mode           = VSF_USART_8_BIT_LENGTH | VSF_USART_NO_PARITY | VSF_USART_1_STOPBIT,
            .baudrate       = DBG_UART_BR,
            .isr            = {
                .handler_fn = __vsf_debug_stream_uart_evthandler,
                .target_ptr = &vsf_hw_usart0,
                .prio       = vsf_arch_prio_0,
            },
        });
        vsf_usart_irq_enable(&vsf_hw_usart0, VSF_USART_IRQ_MASK_RX);

        vsf_stream_connect_tx(&VSF_DEBUG_STREAM_RX.use_as__vsf_stream_t);
    }
}

static void __VSF_DEBUG_STREAM_TX_WRITE_BLOCKED(uint8_t *buf, uint_fast32_t size)
{
    uint_fast16_t written_size;
    while (size > 0) {
        written_size = vsf_usart_txfifo_write(&vsf_hw_usart0, buf, size);
        buf += written_size;
        size -= written_size;
    }
}

#define VSF_HAL_USE_DEBUG_STREAM                        ENABLED
#include "hal/driver/common/debug_stream/debug_stream_tx_blocked.inc"

void vsf_board_init_linux(void)
{
}

void vsf_board_init(void)
{
    // do not change order below
    VSF_STREAM_INIT(&VSF_DEBUG_STREAM_RX);
    VSF_STREAM_INIT(&VSF_DEBUG_STREAM_TX);
}
