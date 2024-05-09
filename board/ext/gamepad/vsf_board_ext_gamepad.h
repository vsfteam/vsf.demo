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


//! \note User Level Board Configuration

#ifndef __VSF_BOARD_EXT_GAMEPAD_H__
#define __VSF_BOARD_EXT_GAMEPAD_H__

/*============================ INCLUDES ======================================*/

#include "./vsf_board_ext_gamepad_cfg.h"

#include <vsf.h>

#if     defined(__GAMEPAD_IO_CLASS_IMPLEMENT)
#   define __VSF_CLASS_IMPLEMENT__
#elif   defined(__GAMEPAD_IO_CLASS_INHERIT__)
#   define __VSF_CLASS_INHERIT__
#endif

#include "utilities/ooc_class.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
/*============================ TYPES =========================================*/

typedef struct vsf_board_ext_gamepad_t {
    vsf_gpio_t *gpio_input;
    vsf_gpio_t *gpio_output;
    vsf_pwm_t *pwm_tl, *pwm_tr, *pwm_ml, *pwm_mr, *pwm_led[4];
    vsf_i2c_t *i2c_adc;
    vsf_adc_t *adc;
} vsf_board_ext_gamepad_t;

/*============================ GLOBAL VARIABLES ==============================*/

extern vsf_board_ext_gamepad_t vsf_board_ext_gamepad;

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

extern void vsf_board_ext_gamepad_init(void);
extern void gamepad_io_start(uint8_t polling_ms);

#ifdef __cplusplus
}
#endif

#endif      // __VSF_BOARD_EXT_GAMEPAD_H__
