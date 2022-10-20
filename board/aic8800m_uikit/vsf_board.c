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

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ LOCAL VARIABLES ===============================*/

#if VSF_USE_USB_HOST == ENABLED
static const vk_dwcotg_hcd_param_t __dwcotg_hcd_param = {
    .op                         = &VSF_USB_HC0_IP,
    .priority                   = APP_CFG_USBH_ARCH_PRIO,
};
#endif

/*============================ GLOBAL VARIABLES ==============================*/

vsf_board_t vsf_board = {
    .usart                      = (vsf_usart_t *)&vsf_hw_usart1,
    .spi                        = (vsf_spi_t *)&vsf_hw_spi0,
    .i2c                        = (vsf_i2c_t *)&vsf_hw_i2c0,

    .mmc                        = (vsf_mmc_t *)&vsf_hw_mmc0,
    .mmc_bus_width              = 4,
    .mmc_voltage                = SD_OCR_VDD_32_33 | SD_OCR_VDD_33_34,

#if VSF_USE_UI == ENABLED
    .display_dev                = &vsf_board.disp_wingdi.use_as__vk_disp_t,
    .disp_spi_mipi              = {
        .param                  = {
            .height             = 320,
            .width              = 240,
            .drv                = &vk_disp_drv_mipi_lcd,
            .color              = VSF_DISP_COLOR_RGB565,
        },
        .spi                    = (vsf_spi_t *)&vsf_hw_spi0,
        .reset                  = {
            .gpio               = (vsf_gpio_t *)&vsf_hw_gpio1,
            .pin_mask           = 1 << 7,
        },
        .dcx                    = {
            .gpio               = (vsf_gpio_t *)&vsf_hw_gpio0,
            .pin_mask           = 1 << 4,
        },
        .clock_hz               = 60ul * 1000ul * 1000ul,
        .init_seq               = (const uint8_t [])VSF_DISP_MIPI_LCD_ST7789V_BASE,
        .init_seq_len           = sizeof((const uint8_t [])VSF_DISP_MIPI_LCD_ST7789V_BASE),
    },
#endif
#if VSF_USE_AUDIO == ENABLED
    .audio_dev                  = &vsf_board.audio_winsound.use_as__vk_audio_dev_t,
#endif
#if VSF_USE_USB_HOST == ENABLED
    .usbh_dev                   = {
        .drv                    = &vk_dwcotg_hcd_drv,
        .param                  = (void *)&__dwcotg_hcd_param,
    },
#endif
};

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

#if !defined(VSF_HAL_USE_DEBUG_STREAM) || VSF_HAL_USE_DEBUG_STREAM == DISABLED
static void __VSF_DEBUG_STREAM_TX_INIT(void)
{
    vsf_usart_t *debug_usart = (vsf_usart_t *)&vsf_hw_usart1;
    vsf_err_t err;

    err = vsf_usart_init(debug_usart, &(vsf_usart_cfg_t){
        .mode               = USART_8_BIT_LENGTH | USART_1_STOPBIT | USART_NO_PARITY
                            | USART_TX_ENABLE | USART_RX_ENABLE,
        .baudrate           = 921600,
    });
    if (err != VSF_ERR_NONE) {
        VSF_ASSERT(false);
        return;
    }

    while (fsm_rt_cpl != vsf_usart_enable(debug_usart));
}

static void __VSF_DEBUG_STREAM_TX_WRITE_BLOCKED(uint8_t *buf, uint_fast32_t size)
{
    vsf_usart_t *debug_usart = (vsf_usart_t *)&vsf_hw_usart1;

    for (uint_fast32_t i = 0; i < size; i++) {
        vsf_usart_txfifo_write(debug_usart, buf++, 1);
    }
}

// Because debug stream for AIC8800M is not used,
//  VSF_HAL_USE_DEBUG_STREAM is not defined in header files.
// But debug_stream_tx_blocked.inc will need VSF_HAL_USE_DEBUG_STREAM,
//  so define VSF_HAL_USE_DEBUG_STREAM here.
#undef VSF_HAL_USE_DEBUG_STREAM
#define VSF_HAL_USE_DEBUG_STREAM        ENABLED
#include "hal/driver/common/debug_stream/debug_stream_tx_blocked.inc"
#endif

void vsf_board_init(void)
{
    static const vsf_io_cfg_t cfgs[] = {
//        {VSF_PA0,   0x09,   0},
//        {VSF_PA3,   0x09,   0},

        // PA10 - PA15 as SDIOM
        {VSF_PA10,  0x06,   0},
        {VSF_PA11,  0x06,   0},
        {VSF_PA12,  0x06,   0},
        {VSF_PA13,  0x06,   0},
        {VSF_PA14,  0x06,   0},
        {VSF_PA15,  0x06,   0},

//        {VSF_PA1,   0x06,   0},
        {VSF_PA2,   0x06,   0},
        {VSF_PA7,   0x08,   0},
//        {VSF_PA13,  0x08,   0},

        {VSF_PA7,   0x08,   0},
//        {VSF_PA8,   0x08,   0},
//        {VSF_PA9,   0x08,   0},
        {VSF_PB0,   0x01,   0},
        {VSF_PB1,   0x01,   0},
        {VSF_PB5,   0x00,   0},
        {VSF_PB4,   0x00,   0},
        {VSF_PB13,  0x00,   0},

        // PB2 PB3 as UART1
        {VSF_PB2,   0x01,   0},
        {VSF_PB3,   0x01,   0},
    };
    vsf_io_config((vsf_io_cfg_t *)cfgs, dimof(cfgs));

    VSF_STREAM_INIT(&VSF_DEBUG_STREAM_TX);
}
