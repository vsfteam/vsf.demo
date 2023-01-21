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

#ifndef __VSF_DISTBUS_HAL_H__
#define __VSF_DISTBUS_HAL_H__

#include "hal/vsf_hal.h"
// for distbus
#include "service/vsf_service.h"

#include "./io/vsf_distbus_hal_io.h"
#include "./gpio/vsf_distbus_hal_gpio.h"
#include "./i2c/vsf_distbus_hal_i2c.h"
#include "./spi/vsf_distbus_hal_spi.h"
#include "./usart/vsf_distbus_hal_usart.h"
#include "./mmc/vsf_distbus_hal_mmc.h"
#include "./adc/vsf_distbus_hal_adc.h"
#include "./dac/vsf_distbus_hal_dac.h"
#include "./pwm/vsf_distbus_hal_pwm.h"
#include "./i2s/vsf_distbus_hal_i2s.h"
#include "./usbd/vsf_distbus_hal_usbd.h"
#include "./usbh/vsf_distbus_hal_usbh.h"

#if     defined(__VSF_DISTBUS_HAL_CLASS_IMPLEMENT)
#   define __VSF_CLASS_IMPLEMENT__
#elif   defined(__VSF_DISTBUS_HAL_CLASS_INHERIT__)
#   define __VSF_CLASS_INHERIT__
#endif

#include "utilities/ooc_class.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ INCLUDES ======================================*/
/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef struct __vsf_distbus_hal_enum_t {
#define VSF_DISTBUS_HAL_DEFINE(__TYPE)                                          \
        struct {                                                                \
            uint8_t                 dev_num;                                    \
            VSF_MCONNECT(vsf_distbus_hal_, __TYPE, _t) *dev;                    \
        } __TYPE;

#define __VSF_DISTBUS_HAL_ENUM      VSF_DISTBUS_HAL_DEFINE
#include "vsf_distbus_hal_enum.inc"
} __vsf_distbus_hal_enum_t;

vsf_class(vsf_distbus_hal_t) {
    public_member(
        implement(__vsf_distbus_hal_enum_t)
    )
    private_member(
        vsf_distbus_service_t   service;
        vsf_distbus_t           *distbus;
    )
};

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

extern void vsf_distbus_hal_register(vsf_distbus_t *distbus, vsf_distbus_hal_t *distbus_hal);
extern void vsf_distbus_hal_start(vsf_distbus_hal_t *distbus_hal);

#ifdef __cplusplus
}
#endif

#endif      // __VSF_DISTBUS_HAL_H__
