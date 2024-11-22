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

#ifndef __VSF_BOARD_H__
#define __VSF_BOARD_H__

/*============================ INCLUDES ======================================*/

#include "vsf.h"

/*============================ MACROS ========================================*/

#if VSF_HAL_USE_SDIO == ENABLED
#   define VSF_BOARD_SDMMC_DETECTED()       1
#endif

/*============================ TYPES =========================================*/

typedef struct vsf_board_t {
#if VSF_HAL_USE_USART == ENABLED
    vsf_usart_t *usart;
#endif
#if VSF_HAL_USE_SPI == ENABLED
    vsf_spi_t *spi;
#endif
#if VSF_HAL_USE_I2C == ENABLED
    vsf_i2c_t *i2c;
#endif
#if VSF_HAL_USE_SDIO == ENABLED
    vsf_sdio_t *sdio;
    uint8_t sdio_bus_width;
    uint32_t sdio_voltage;
#endif
#if VSF_HAL_USE_I2S == ENABLED
    vsf_i2s_t *i2s;
#endif

#if VSF_USE_UI == ENABLED
    vk_disp_t *display_dev;
    vk_disp_mipi_spi_lcd_t disp_spi_mipi;
#endif
#if VSF_USE_AUDIO == ENABLED
    vk_audio_dev_t *audio_dev;
    vk_aic1000a_dev_t aic1000a;
#endif
#if VSF_USE_USB_HOST == ENABLED
    vk_usbh_t usbh_dev;
#endif
#if VSF_USE_USB_DEVICE == ENABLED
    vk_dwcotg_dcd_t dwcotg_dcd;
#endif
} vsf_board_t;

/*============================ GLOBAL VARIABLES ==============================*/

extern vsf_board_t vsf_board;
#if VSF_USE_USB_DEVICE == ENABLED
extern const i_usb_dc_t VSF_USB_DC0;
#endif

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

extern void vsf_board_init(void);

#endif      // __VSF_BOARD_H__
