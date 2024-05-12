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

#include "vsf_board.h"
#include "./vsf_board_ext_gamepad.h"

#if VSF_BOARD_USE_EXT_GAMEPAD == ENABLED

/*============================ MACROS ========================================*/

#if     !defined(VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT)                            \
    ||  !defined(VSF_BOARD_EXT_GAMEPAD_EXTIO74_OUT_PIN)                         \
    ||  !defined(VSF_BOARD_EXT_GAMEPAD_EXTIO74_IN_PIN)                          \
    ||  !defined(VSF_BOARD_EXT_GAMEPAD_EXTIO74_PL_PIN)                          \
    ||  !defined(VSF_BOARD_EXT_GAMEPAD_EXTIO74_CLK_PIN)                         \
    ||  !defined(VSF_BOARD_EXT_GAMEPAD_EXTIO74_OE_PIN)
#   error current board does not support gamepad extension
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef struct gamepad_io_value_t {
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
} gamepad_io_value_t;

typedef struct gamepad_io_ctx_t {
    gamepad_io_value_t cur;
    gamepad_io_value_t prev;
    vsf_callback_timer_t polling_timer;
    uint8_t polling_ms;
    bool is_busy;
    bool is_to_poll;
} gamepad_io_ctx_t;

/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ GLOBAL VARIABLES ==============================*/

static void __74hc_load_control(void *param, uint_fast8_t bit);
static void __74hc_clock_control(void *param, uint_fast8_t bit);
static uint_fast8_t __74hc_serial_input(void *param);
#define __74hc_latch_control            __74hc_load_control
static void __74hc_oe_control(void *param, uint_fast8_t bit);
static void __74hc_clock_control(void *param, uint_fast8_t bit);
static void __74hc_serial_output(void *param, uint_fast8_t bit);

#if VSF_HAL_USE_74HC165_GPIO == ENABLED
describe_74hc165_gpio(__gpio_74hc165, 2, NULL,
    NULL,
    __74hc_load_control,
    __74hc_clock_control,
    __74hc_serial_input
)
#endif

#if VSF_HAL_USE_74HC595_GPIO == ENABLED
describe_74hc595_gpio(__gpio_74hc595, 1, NULL,
    __74hc_oe_control,
    __74hc_latch_control,
    __74hc_clock_control,
    __74hc_serial_output
)

#   if VSF_HAL_USE_TIMER_GPIO_PWM == ENABLED
static void __pwm_led0_gpio_control(uint_fast8_t value)
{
    vsf_gpio_write(&__gpio_74hc595, 1 << VSF_BOARD_EXT_GAMEPAD_GPIO_OUT_LED0_PIN, value ? 1 << VSF_BOARD_EXT_GAMEPAD_GPIO_OUT_LED0_PIN : 0);
}
describe_timer_gpio_single_pwm(__pwm_led0, &vsf_hw_timer0, __pwm_led0_gpio_control)
#   endif
#endif

#if VSF_HAL_USE_ADS7830_ADC == ENABLED
describe_ads7830_adc(__adc_joystick, NULL, 0x48)
#endif

vsf_board_ext_gamepad_t vsf_board_ext_gamepad = {
#if VSF_HAL_USE_74HC165_GPIO == ENABLED
    .gpio_input                 = (vsf_gpio_t *)&__gpio_74hc165,
#endif
#if VSF_HAL_USE_74HC595_GPIO == ENABLED
    .gpio_output                = (vsf_gpio_t *)&__gpio_74hc595,
#   if VSF_HAL_USE_TIMER_GPIO_PWM == ENABLED
    .pwm_led[0]                 = (vsf_pwm_t *)&__pwm_led0,
#   endif
#endif

#if VSF_HAL_USE_ADC == ENABLED && VSF_HAL_USE_ADS7830_ADC == ENABLED
    .adc                        = (vsf_adc_t *)&__adc_joystick,
#endif
};

/*============================ LOCAL VARIABLES ===============================*/

#if VSF_USE_INPUT == ENABLED
static const vk_input_item_info_t __vsf_board_ext_gamepad_item_info[GAMEPAD_ID_NUM] = {
    VSF_GAMEPAD_DEF_ITEM_INFO(  L_UP,           0,  1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  L_DOWN,         1,  1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  L_LEFT,         2,  1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  L_RIGHT,        3,  1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  R_UP,           4,  1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  R_DOWN,         5,  1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  R_LEFT,         6,  1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  R_RIGHT,        7,  1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  LB,             8,  1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  RB,             9,  1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  LS,             10, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  RS,             11, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  MENU_LEFT,      12, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  MENU_RIGHT,     14, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  MENU_MAIN,      13, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  SPECIAL,        15, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  LX,             16, 16, false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  LY,             32, 16, false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  RX,             48, 16, false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  RY,             64, 16, false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  LT,             80, 16, false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  RT,             96, 16, false),
};
#endif

static gamepad_io_ctx_t __gamepad_io_ctx;

/*============================ IMPLEMENTATION ================================*/

static void __74hc_load_control(void *param, uint_fast8_t bit)
{
    vsf_gpio_pin_mask_t mask = 1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_PL_PIN;
    vsf_gpio_write(&VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT, mask, bit ? mask : 0);
}

static void __74hc_clock_control(void *param, uint_fast8_t bit)
{
    vsf_gpio_pin_mask_t mask = 1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_CLK_PIN;
    vsf_gpio_write(&VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT, mask, bit ? mask : 0);
}

static uint_fast8_t __74hc_serial_input(void *param)
{
    vsf_gpio_pin_mask_t mask = 1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_IN_PIN;
    return vsf_gpio_read(&VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT) & mask ? 1 : 0;
}

static void __74hc_oe_control(void *param, uint_fast8_t bit)
{
    vsf_gpio_pin_mask_t mask = 1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_OE_PIN;
    vsf_gpio_write(&VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT, mask, bit ? mask : 0);
}

static void __74hc_serial_output(void *param, uint_fast8_t bit)
{
    vsf_gpio_pin_mask_t mask = 1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_OUT_PIN;
    vsf_gpio_write(&VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT, mask, bit ? mask : 0);
}

void vsf_board_ext_gamepad_init(void)
{

#if VSF_HAL_USE_74HC165_GPIO == ENABLED || VSF_HAL_USE_74HC595_GPIO == ENABLED
    vsf_gpio_set_output(&VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT,
                            (1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_PL_PIN)
                        |   (1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_CLK_PIN));
#   if VSF_HAL_USE_74HC165_GPIO == ENABLED
    vsf_gpio_set_input(&VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT,
                            (1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_IN_PIN));
    vsf_74hc165_gpio_init(&__gpio_74hc165);
    vsf_gpio_set_input(vsf_board_ext_gamepad.gpio_input, 0xFFFF);
#   endif

#   if VSF_HAL_USE_74HC595_GPIO == ENABLED
    vsf_gpio_set_output(&VSF_BOARD_EXT_GAMEPAD_EXTIO74_PORT,
                            (1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_OE_PIN)
                        |   (1 << VSF_BOARD_EXT_GAMEPAD_EXTIO74_OUT_PIN));
    vsf_74hc595_gpio_init(&__gpio_74hc595);
    vsf_gpio_clear(vsf_board_ext_gamepad.gpio_output, 0xFF);
    vsf_gpio_set_output(vsf_board_ext_gamepad.gpio_output, 0xFF);
#   endif
#endif

#if VSF_HAL_USE_I2C == ENABLED
#   if VSF_HAL_USE_ADS7830_ADC == ENABLED
    __adc_joystick.i2c = vsf_board.i2c_adc;
#   endif
#endif
}

static void __gamepad_io_on_polling_timer(vsf_callback_timer_t *timer);
static void __gamepad_io_adc_irqhandler(void *target_ptr, vsf_adc_t *adc_ptr, vsf_adc_irq_mask_t irq_mask);

static void __gamepad_io_adc_irqhandler(void *target_ptr, vsf_adc_t *adc_ptr, vsf_adc_irq_mask_t irq_mask)
{
    gamepad_io_ctx_t *ctx = target_ptr;

    vsf_gpio_pin_mask_t button_value = vsf_gpio_read(vsf_board_ext_gamepad.gpio_input);
#if VSF_BOARD_EXT_GAMEPAD_GPIO_IN_INVERSE == ENABLED
    ctx->cur.l_up       = !(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LU));
    ctx->cur.l_down     = !(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LD));
    ctx->cur.l_left     = !(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LL));
    ctx->cur.l_right    = !(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LR));
    ctx->cur.r_up       = !(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RU));
    ctx->cur.r_down     = !(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RD));
    ctx->cur.r_left     = !(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RL));
    ctx->cur.r_right    = !(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RR));
    ctx->cur.l_bumper   = !(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LB));
    ctx->cur.r_bumper   = !(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RB));
    ctx->cur.l_stick    = !(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LS));
    ctx->cur.r_stick    = !(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RS));
    ctx->cur.l_menu     = !(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_ML));
    ctx->cur.m_menu     = !(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_HOME));
    ctx->cur.r_menu     = !(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_MR));
    ctx->cur.special    = !(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_SPECIAL));
#else
    ctx->cur.l_up       = !!(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LU));
    ctx->cur.l_down     = !!(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LD));
    ctx->cur.l_left     = !!(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LL));
    ctx->cur.l_right    = !!(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LR));
    ctx->cur.r_up       = !!(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RU));
    ctx->cur.r_down     = !!(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RD));
    ctx->cur.r_left     = !!(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RL));
    ctx->cur.r_right    = !!(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RR));
    ctx->cur.l_bumper   = !!(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LB));
    ctx->cur.r_bumper   = !!(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RB));
    ctx->cur.l_stick    = !!(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LS));
    ctx->cur.r_stick    = !!(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RS));
    ctx->cur.l_menu     = !!(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_ML));
    ctx->cur.m_menu     = !!(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_HOME));
    ctx->cur.r_menu     = !!(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_MR));
    ctx->cur.special    = !!(button_value & (1 << VSF_BOARD_EXT_GAMEPAD_GPIO_IN_SPECIAL));
#endif

    // convert adc results if adc result is 8-bit
    vsf_adc_capability_t adc_cap = vsf_adc_capability(adc_ptr);
    if (adc_cap.max_data_bits <= 8) {
        uint16_t *buf16 = &ctx->cur.adc_buffer[dimof(ctx->cur.adc_buffer) - 1];
        uint8_t *buf8 = &((uint8_t *)ctx->cur.adc_buffer)[dimof(ctx->cur.adc_buffer) - 1];
        for (int i = 0; i < dimof(ctx->cur.adc_buffer); i++) {
            *buf16-- = (*buf8--) << (16 - adc_cap.max_data_bits);
        }
    }

#if VSF_USE_INPUT == ENABLED
    vk_gamepad_input_parse(ctx, (vk_input_item_info_t *)__vsf_board_ext_gamepad_item_info,
            (uint8_t *)&ctx->prev, (uint8_t *)&ctx->cur);
#endif
    ctx->prev = ctx->cur;

    bool is_to_poll;
    vsf_protect_t orig = vsf_protect_int();
    is_to_poll = ctx->is_busy = ctx->is_to_poll;
    ctx->is_to_poll = false;
    vsf_unprotect_int(orig);

    if (is_to_poll) {
        vsf_adc_channel_request(vsf_board_ext_gamepad.adc, ctx->cur.adc_buffer, dimof(ctx->cur.adc_buffer));
    }
}

static void __gamepad_io_on_polling_timer(vsf_callback_timer_t *timer)
{
    gamepad_io_ctx_t *ctx = container_of(timer, gamepad_io_ctx_t, polling_timer);
    bool is_to_poll;

    vsf_protect_t orig = vsf_protect_int();
    is_to_poll = !ctx->is_busy;
    if (is_to_poll) {
        ctx->is_busy = true;
    } else {
        ctx->is_to_poll = true;
    }
    vsf_unprotect_int(orig);

    if (is_to_poll) {
        vsf_adc_channel_request(vsf_board_ext_gamepad.adc, ctx->cur.adc_buffer, dimof(ctx->cur.adc_buffer));
    }
    vsf_callback_timer_add_ms(timer, ctx->polling_ms);
}

void gamepad_io_start(uint8_t polling_ms)
{
    VSF_ASSERT(polling_ms > 0);

    gamepad_io_ctx_t *ctx = &__gamepad_io_ctx;
    ctx->is_to_poll = false;
    ctx->is_busy = false;
    ctx->polling_ms = polling_ms;
    memset(&ctx->prev, 0, sizeof(ctx->prev));
    memset(&ctx->cur, 0, sizeof(ctx->cur));

    vsf_adc_init(vsf_board_ext_gamepad.adc, &(vsf_adc_cfg_t){
        .mode           = VSF_ADC_SCAN_CONV_SEQUENCE_MODE,
        .isr            = {
            .handler_fn = __gamepad_io_adc_irqhandler,
            .target_ptr = ctx,
            .prio       = VSF_ARCH_PRIO_0,
        },
    });
    vsf_adc_enable(vsf_board_ext_gamepad.adc);
    vsf_adc_irq_enable(vsf_board_ext_gamepad.adc, VSF_ADC_IRQ_MASK_CPL);
    vsf_adc_channel_cfg_t adc_channels[dimof(ctx->cur.adc_buffer)] = {
        [0]             = {
            .channel    = VSF_BOARD_EXT_GAMEPAD_ADC_CHANNEL_LX,
        },
        [1]             = {
            .channel    = VSF_BOARD_EXT_GAMEPAD_ADC_CHANNEL_LY,
        },
        [2]             = {
            .channel    = VSF_BOARD_EXT_GAMEPAD_ADC_CHANNEL_RX,
        },
        [3]             = {
            .channel    = VSF_BOARD_EXT_GAMEPAD_ADC_CHANNEL_RY,
        },
        [4]             = {
            .channel    = VSF_BOARD_EXT_GAMEPAD_ADC_CHANNEL_LT,
        },
        [5]             = {
            .channel    = VSF_BOARD_EXT_GAMEPAD_ADC_CHANNEL_RT,
        },
    };
    vsf_adc_channel_config(vsf_board_ext_gamepad.adc, adc_channels, dimof(adc_channels));

#if VSF_YSE_INPUT == ENABLED
    vsf_input_on_new_dev(VSF_INPUT_TYPE_GAMEPAD, ctx);
#endif

    vsf_callback_timer_init(&ctx->polling_timer);
    ctx->polling_timer.on_timer = __gamepad_io_on_polling_timer;
    __gamepad_io_on_polling_timer(&ctx->polling_timer);
}

#endif      // VSF_BOARD_USE_EXT_GAMEPAD
