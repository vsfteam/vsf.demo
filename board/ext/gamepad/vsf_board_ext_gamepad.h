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

typedef struct gamepad_io_value_t {
    struct {
        union {
            struct {
                uint16_t l_up       : 1;
                uint16_t l_down     : 1;
                uint16_t l_left     : 1;
                uint16_t l_right    : 1;
                uint16_t r_up       : 1;
                uint16_t r_down     : 1;
                uint16_t r_left     : 1;
                uint16_t r_right    : 1;
                uint16_t l_bumper   : 1;
                uint16_t r_bumper   : 1;
                uint16_t l_stick    : 1;
                uint16_t r_stick    : 1;
                uint16_t l_menu     : 1;
                uint16_t m_menu     : 1;
                uint16_t r_menu     : 1;
                uint16_t special    : 1;
            };
            uint16_t button_value;
        };
        union {
            struct {
                uint16_t l_stick_x;
                uint16_t l_stick_y;
                uint16_t r_stick_x;
                uint16_t r_stick_y;
                uint16_t l_trigger;
                uint16_t r_trigger;
            };
            uint16_t adc_buffer[6];
        };
    } value;
    vsf_systimer_tick_t sample_tick;
} gamepad_io_value_t;

vsf_declare_class(gamepad_io_ctx_t)
typedef struct gamepad_io_cfg_t {
    uint8_t polling_ms;
    void (*on_changed)(gamepad_io_ctx_t *ctx);
} gamepad_io_cfg_t;

vsf_class(gamepad_io_ctx_t) {
    public_member(
        gamepad_io_value_t cur;
        gamepad_io_value_t prev;
        void (*on_changed)(gamepad_io_ctx_t *ctx);
    )
    private_member(
        vsf_callback_timer_t polling_timer;
        uint8_t polling_ms;
        bool is_busy;
        bool is_to_poll;
    )
};

/*============================ GLOBAL VARIABLES ==============================*/

extern vsf_board_ext_gamepad_t vsf_board_ext_gamepad;

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

extern void vsf_board_ext_gamepad_init(void);
extern void gamepad_io_start(gamepad_io_ctx_t *ctx, gamepad_io_cfg_t *cfg);

#ifdef __cplusplus
}
#endif

#endif      // __VSF_BOARD_EXT_GAMEPAD_H__
