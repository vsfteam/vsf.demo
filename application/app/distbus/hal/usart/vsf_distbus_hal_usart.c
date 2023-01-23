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
 * distbus hal usart
 *
 * Dependency:
 * Board:
 */

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"

#if VSF_HAL_USE_USART == ENABLED && VSF_USE_DISTBUS == ENABLED

#define __VSF_DISTBUS_CLASS_INHERIT__
#define __VSF_DISTBUS_HAL_USART_CLASS_IMPLEMENT
// for hal_distbus_usart constants
#define __VSF_HAL_DISTBUS_USART_CLASS_INHERIT__
#include "./vsf_distbus_hal_usart.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/

static bool __vsf_distbus_hal_usart_service_msghandler(vsf_distbus_t *distbus,
                        vsf_distbus_service_t *service, vsf_distbus_msg_t *msg);

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static const vsf_distbus_service_info_t __vsf_distbus_hal_usart_service_info = {
    .mtu                    = VSF_HAL_DISTBUS_CFG_MTU,
    .addr_range             = VSF_HAL_DISTBUS_USART_CMD_ADDR_RANGE,
    .handler                = __vsf_distbus_hal_usart_service_msghandler,
};

/*============================ IMPLEMENTATION ================================*/

static void __vsf_distbus_hal_usart_isrhandler(void *target, vsf_usart_t *generic_usart, vsf_usart_irq_mask_t irq_mask)
{
    vsf_distbus_hal_usart_t *usart = target;
    vsf_hal_distbus_usart_isr_t *param;
    vsf_distbus_msg_t *msg;

    if (irq_mask & VSF_USART_IRQ_MASK_RX) {
        uint8_t *buffer;
        uint32_t size;

        // TODO: allocate rxfifo_size of target usart
        msg = vsf_distbus_alloc_msg(usart->distbus, 64, &buffer);
        VSF_ASSERT(msg != NULL);

        size = vsf_usart_rxfifo_read(usart->target, buffer, 64);
        msg->header.addr = VSF_HAL_DISTBUS_USART_CMD_RX;
        msg->header.datalen = size;
        vsf_distbus_send_msg(usart->distbus, &usart->service, msg);
    }

    msg = vsf_distbus_alloc_msg(usart->distbus, sizeof(*param), (uint8_t **)&param);
    VSF_ASSERT(msg != NULL);

    param->irq_mask = cpu_to_le32(irq_mask);
    msg->header.addr = VSF_HAL_DISTBUS_USART_CMD_ISR;
    vsf_distbus_send_msg(usart->distbus, &usart->service, msg);
}

static bool __vsf_distbus_hal_usart_service_msghandler(vsf_distbus_t *distbus,
                        vsf_distbus_service_t *service, vsf_distbus_msg_t *msg)
{
    vsf_distbus_hal_usart_t *usart = container_of(service, vsf_distbus_hal_usart_t, service);
    uint32_t datalen = msg->header.datalen;
    bool retain_msg = false;

    union {
        void *ptr;
        vsf_hal_distbus_usart_init_t *init;
        vsf_hal_distbus_usart_isr_t *irq;
    } u_arg;
    u_arg.ptr = (uint8_t *)&msg->header + sizeof(msg->header);

    switch (msg->header.addr) {
    case VSF_HAL_DISTBUS_USART_CMD_INIT:
        VSF_ASSERT(datalen == sizeof(*u_arg.init));
        u_arg.init->mode = le32_to_cpu(u_arg.init->mode);
        vsf_usart_init(usart->target, &(vsf_usart_cfg_t){
            .mode           = vsf_hal_distbus_usart_mode_to_generic_usart_mode(u_arg.init->mode),
            .baudrate       = le32_to_cpu(u_arg.init->baudrate),
            .rx_timeout     = le32_to_cpu(u_arg.init->rx_timeout),
            .isr            = {
                .handler_fn = __vsf_distbus_hal_usart_isrhandler,
                .target_ptr = usart,
                .prio       = vsf_arch_prio_0,
            },
        });
        vsf_usart_irq_enable(usart->target, VSF_USART_IRQ_MASK_RX);
        break;
    case VSF_HAL_DISTBUS_USART_CMD_ENABLE:
        while (vsf_usart_enable(usart->target) != fsm_rt_cpl);
        break;
    case VSF_HAL_DISTBUS_USART_CMD_DISABLE:
        while (vsf_usart_disable(usart->target) != fsm_rt_cpl);
        break;
    case VSF_HAL_DISTBUS_USART_CMD_IRQ_ENABLE:
        VSF_ASSERT(datalen == sizeof(*u_arg.irq));
        u_arg.irq->irq_mask = le32_to_cpu(u_arg.irq->irq_mask);
        usart->irq_mask |= u_arg.irq->irq_mask;
        vsf_usart_irq_enable(usart->target, u_arg.irq->irq_mask);
        break;
    case VSF_HAL_DISTBUS_USART_CMD_IRQ_DISABLE:
        VSF_ASSERT(datalen == sizeof(*u_arg.irq));
        u_arg.irq->irq_mask = le32_to_cpu(u_arg.irq->irq_mask) & ~VSF_USART_IRQ_MASK_RX;
        usart->irq_mask &= ~u_arg.irq->irq_mask;
        vsf_usart_irq_disable(usart->target, u_arg.irq->irq_mask);
        break;
    case VSF_HAL_DISTBUS_USART_CMD_TX: {
            uint32_t written_size = vsf_usart_txfifo_write(usart->target, u_arg.ptr, datalen);

            vsf_hal_distbus_usart_txed_cnt_t *param;
            msg = vsf_distbus_alloc_msg(usart->distbus, sizeof(*param), (uint8_t **)&param);
            VSF_ASSERT(msg != NULL);

            param->count = cpu_to_le32(written_size);
            msg->header.addr = VSF_HAL_DISTBUS_USART_CMD_TXED_COUNT;
            vsf_distbus_send_msg(usart->distbus, &usart->service, msg);
        }
        break;
    default:
        VSF_HAL_ASSERT(false);
        break;
    }
    return retain_msg;
}

uint32_t vsf_distbus_hal_usart_declare(vsf_distbus_hal_usart_t *usart, uint8_t *ptr, uint32_t size)
{
    return 0;
}

void vsf_distbus_hal_usart_poll(vsf_distbus_hal_usart_t *usart)
{
}

void vsf_distbus_hal_usart_register(vsf_distbus_t *distbus, vsf_distbus_hal_usart_t *usart)
{
    usart->irq_mask = 0;
    usart->distbus = distbus;
    usart->service.info = &__vsf_distbus_hal_usart_service_info;
    vsf_distbus_register_service(distbus, &usart->service);
}

#endif      // VSF_HAL_USE_USART && VSF_USE_DISTBUS
