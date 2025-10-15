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
 * distbus hal gpio
 *
 * Dependency:
 * Board:
 */

/*============================ INCLUDES ======================================*/

#include "hal/vsf_hal_cfg.h"

#if VSF_HAL_USE_GPIO == ENABLED && VSF_USE_DISTBUS == ENABLED

#define __VSF_DISTBUS_CLASS_INHERIT__
#define __VSF_DISTBUS_HAL_GPIO_CLASS_IMPLEMENT
// for hal_distbus_gpio constants
#define __VSF_HAL_DISTBUS_GPIO_CLASS_INHERIT__
#include "./vsf_distbus_hal_gpio.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/

static bool __vsf_distbus_hal_gpio_service_msghandler(vsf_distbus_t *distbus,
                        vsf_distbus_service_t *service, vsf_distbus_msg_t *msg);

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static const vsf_distbus_service_info_t __vsf_distbus_hal_gpio_service_info = {
    .mtu                    = VSF_HAL_DISTBUS_CFG_MTU,
    .addr_range             = VSF_HAL_DISTBUS_GPIO_CMD_ADDR_RANGE,
    .handler                = __vsf_distbus_hal_gpio_service_msghandler,
};

/*============================ IMPLEMENTATION ================================*/

static vsf_gpio_mode_t __vsf_hal_distbus_gpio_mode(uint32_t mode)
{
    vsf_gpio_mode_t gpio_mode = 0;
    uint32_t tmp;

    tmp = mode & VSF_HAL_DISTBUS_ENUM(VSF_GPIO_MODE_MASK);
    switch (tmp) {
    case VSF_HAL_DISTBUS_ENUM(VSF_GPIO_INPUT):                      gpio_mode |= VSF_GPIO_INPUT;                    break;
    case VSF_HAL_DISTBUS_ENUM(VSF_GPIO_ANALOG):                     gpio_mode |= VSF_GPIO_ANALOG;                   break;
    case VSF_HAL_DISTBUS_ENUM(VSF_GPIO_OUTPUT_PUSH_PULL):           gpio_mode |= VSF_GPIO_OUTPUT_PUSH_PULL;         break;
    case VSF_HAL_DISTBUS_ENUM(VSF_GPIO_OUTPUT_OPEN_DRAIN):          gpio_mode |= VSF_GPIO_OUTPUT_OPEN_DRAIN;        break;
    case VSF_HAL_DISTBUS_ENUM(VSF_GPIO_EXTI):                       gpio_mode |= VSF_GPIO_EXTI;                     break;
    case VSF_HAL_DISTBUS_ENUM(VSF_GPIO_AF):                         gpio_mode |= VSF_GPIO_AF;                       break;
    }

    tmp = mode & VSF_HAL_DISTBUS_ENUM(VSF_GPIO_PULL_UP_DOWN_MASK);
    switch (tmp) {
    case VSF_HAL_DISTBUS_ENUM(VSF_GPIO_NO_PULL_UP_DOWN):            gpio_mode |= VSF_GPIO_NO_PULL_UP_DOWN;          break;
    case VSF_HAL_DISTBUS_ENUM(VSF_GPIO_PULL_UP):                    gpio_mode |= VSF_GPIO_PULL_UP;                  break;
    case VSF_HAL_DISTBUS_ENUM(VSF_GPIO_PULL_DOWN):                  gpio_mode |= VSF_GPIO_PULL_DOWN;                break;
    }

    tmp = mode & VSF_HAL_DISTBUS_ENUM(VSF_GPIO_EXTI_MODE_MASK);
    switch (tmp) {
    case VSF_HAL_DISTBUS_ENUM(VSF_GPIO_EXTI_MODE_NONE):             gpio_mode |= VSF_GPIO_EXTI_MODE_NONE;           break;
    case VSF_HAL_DISTBUS_ENUM(VSF_GPIO_EXTI_MODE_LOW_LEVEL):        gpio_mode |= VSF_GPIO_EXTI_MODE_LOW_LEVEL;      break;
    case VSF_HAL_DISTBUS_ENUM(VSF_GPIO_EXTI_MODE_HIGH_LEVEL):       gpio_mode |= VSF_GPIO_EXTI_MODE_HIGH_LEVEL;     break;
    case VSF_HAL_DISTBUS_ENUM(VSF_GPIO_EXTI_MODE_RISING):           gpio_mode |= VSF_GPIO_EXTI_MODE_RISING;         break;
    case VSF_HAL_DISTBUS_ENUM(VSF_GPIO_EXTI_MODE_FALLING):          gpio_mode |= VSF_GPIO_EXTI_MODE_FALLING;        break;
    case VSF_HAL_DISTBUS_ENUM(VSF_GPIO_EXTI_MODE_RISING_FALLING):   gpio_mode |= VSF_GPIO_EXTI_MODE_RISING_FALLING; break;
    }
    return gpio_mode;
}

static bool __vsf_distbus_hal_gpio_service_msghandler(vsf_distbus_t *distbus,
                        vsf_distbus_service_t *service, vsf_distbus_msg_t *msg)
{
    vsf_distbus_hal_gpio_t *gpio = vsf_container_of(service, vsf_distbus_hal_gpio_t, service);
    uint32_t datalen = msg->header.datalen;
    bool retain_msg = false;

    union {
        void *ptr;
        vsf_hal_distbus_gpio_port_config_pins_t *port_config_pins;
        vsf_hal_distbus_gpio_set_direction_t *set_direction;
        vsf_hal_distbus_gpio_write_t *write;
        vsf_hal_distbus_gpio_pin_mask_t *set;
        vsf_hal_distbus_gpio_pin_mask_t *clear;
        vsf_hal_distbus_gpio_pin_mask_t *toggle;
        vsf_hal_distbus_gpio_pin_mask_t *output_and_set;
        vsf_hal_distbus_gpio_pin_mask_t *output_and_clear;
    } u_arg;
    u_arg.ptr = (uint8_t *)&msg->header + sizeof(msg->header);

    switch (msg->header.addr) {
    case VSF_HAL_DISTBUS_GPIO_CMD_PORT_CONFIG_PINS:
        VSF_ASSERT(datalen == sizeof(*u_arg.port_config_pins));
        u_arg.port_config_pins->pin_mask = le32_to_cpu(u_arg.port_config_pins->pin_mask);
        {
            vsf_gpio_cfg_t cfg = {
                .mode                   = __vsf_hal_distbus_gpio_mode(le32_to_cpu(u_arg.port_config_pins->mode)),
                .alternate_function     = le16_to_cpu(u_arg.port_config_pins->alternate_function),
            };
            vsf_gpio_port_config_pins(gpio->target, u_arg.port_config_pins->pin_mask,
                                        &cfg);
        }
        break;
    case VSF_HAL_DISTBUS_GPIO_CMD_SET_DIRECTION:
        VSF_ASSERT(datalen == sizeof(*u_arg.set_direction));
        u_arg.set_direction->pin_mask = le32_to_cpu(u_arg.set_direction->pin_mask);
        u_arg.set_direction->direction_mask = le32_to_cpu(u_arg.set_direction->direction_mask);
        vsf_gpio_set_direction(gpio->target, u_arg.set_direction->pin_mask,
            u_arg.set_direction->direction_mask);
        break;
    case VSF_HAL_DISTBUS_GPIO_CMD_WRITE:
        VSF_ASSERT(datalen == sizeof(*u_arg.write));
        u_arg.write->pin_mask = le32_to_cpu(u_arg.write->pin_mask);
        u_arg.write->value = le32_to_cpu(u_arg.write->value);
        vsf_gpio_write(gpio->target, u_arg.write->pin_mask, u_arg.write->value);
        break;
    case VSF_HAL_DISTBUS_GPIO_CMD_SET:
        VSF_ASSERT(datalen == sizeof(*u_arg.set));
        u_arg.set->pin_mask = le32_to_cpu(u_arg.set->pin_mask);
        vsf_gpio_set(gpio->target, u_arg.set->pin_mask);
        break;
    case VSF_HAL_DISTBUS_GPIO_CMD_CLEAR:
        VSF_ASSERT(datalen == sizeof(*u_arg.clear));
        u_arg.clear->pin_mask = le32_to_cpu(u_arg.clear->pin_mask);
        vsf_gpio_clear(gpio->target, u_arg.clear->pin_mask);
        break;
    case VSF_HAL_DISTBUS_GPIO_CMD_TOGGLE:
        VSF_ASSERT(datalen == sizeof(*u_arg.toggle));
        u_arg.toggle->pin_mask = le32_to_cpu(u_arg.toggle->pin_mask);
        vsf_gpio_toggle(gpio->target, u_arg.toggle->pin_mask);
        break;
    case VSF_HAL_DISTBUS_GPIO_CMD_OUTPUT_AND_SET:
        VSF_ASSERT(datalen == sizeof(*u_arg.output_and_set));
        u_arg.output_and_set->pin_mask = le32_to_cpu(u_arg.output_and_set->pin_mask);
        vsf_gpio_output_and_set(gpio->target, u_arg.output_and_set->pin_mask);
        break;
    case VSF_HAL_DISTBUS_GPIO_CMD_OUTPUT_AND_CLEAR:
        VSF_ASSERT(datalen == sizeof(*u_arg.output_and_clear));
        u_arg.output_and_clear->pin_mask = le32_to_cpu(u_arg.output_and_clear->pin_mask);
        vsf_gpio_output_and_clear(gpio->target, u_arg.output_and_clear->pin_mask);
        break;
    default:
        VSF_HAL_ASSERT(false);
        break;
    }
    return retain_msg;
}

uint32_t vsf_distbus_hal_gpio_declare(vsf_distbus_hal_gpio_t *gpio, uint8_t *ptr, uint32_t size)
{
    if (size >= sizeof(vsf_hal_distbus_gpio_info_t)) {
        vsf_gpio_capability_t cap = vsf_gpio_capability(gpio->target);
        gpio->value = vsf_gpio_read(gpio->target);
        vsf_hal_distbus_gpio_info_t info = {
            .support_output_and_set     = cap.support_output_and_set,
            .support_output_and_clear   = cap.support_output_and_clear,
            .pin_count                  = cap.pin_count,
            .pin_mask                   = cpu_to_le32(cap.pin_mask),

            .direction                  = cpu_to_le32(vsf_gpio_get_direction(gpio->target, cap.pin_mask)),
            .value                      = cpu_to_le32(gpio->value),
        };
        memcpy(ptr, &info, sizeof(vsf_hal_distbus_gpio_info_t));
    }
    return sizeof(vsf_hal_distbus_gpio_info_t);
}

void vsf_distbus_hal_gpio_poll(vsf_distbus_hal_gpio_t *gpio)
{
    if (!gpio->is_to_poll) {
        return;
    }

    uint32_t value = vsf_gpio_read(gpio->target);
    if (value != gpio->value) {
        gpio->value = value;

        vsf_hal_distbus_gpio_sync_t *param;
        vsf_distbus_msg_t *msg = vsf_distbus_alloc_msg(gpio->distbus, sizeof(*param), (uint8_t **)&param);
        VSF_ASSERT(msg != NULL);

        param->value = cpu_to_le32(value);
        msg->header.addr = VSF_HAL_DISTBUS_GPIO_CMD_SYNC;
        vsf_distbus_send_msg(gpio->distbus, &gpio->service, msg);
    }
}

void vsf_distbus_hal_gpio_register(vsf_distbus_t *distbus, vsf_distbus_hal_gpio_t *gpio)
{
    gpio->is_to_poll = true;
    gpio->distbus = distbus;
    gpio->service.info = &__vsf_distbus_hal_gpio_service_info;
    vsf_distbus_register_service(distbus, &gpio->service);
}

#endif      // VSF_HAL_USE_GPIO && VSF_USE_DISTBUS
