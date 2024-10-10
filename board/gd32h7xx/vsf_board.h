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

#define VSF_USE_BOARD           (VSF_HAL_USE_I2C == ENABLED)                    \
                            ||  (VSF_USE_USB_HOST == ENABLED)                   \
                            ||  (VSF_USE_USB_DEVICE == ENABLED)                 \
                            ||  (VSF_HAL_USE_SDIO == ENABLED)                   \
                            ||  (VSF_USE_UI == ENABLED)

#if VSF_HAL_USE_SDIO == ENABLED
#   define VSF_BOARD_SDMMC_DETECTED()       1
#endif

/*============================ TYPES =========================================*/

#if VSF_USE_BOARD == ENABLED
typedef struct vsf_board_t {
#if VSF_HAL_USE_I2C == ENABLED
    vsf_i2c_t *i2c;
#endif
#if VSF_USE_USB_HOST == ENABLED
    vk_usbh_t usbh_dev;
#endif
#if VSF_USE_USB_DEVICE == ENABLED
    vk_dwcotg_dcd_t dwcotg_dcd;
#endif
#if VSF_HAL_USE_SDIO == ENABLED
    vsf_sdio_t *sdio;
    uint8_t sdio_bus_width;
    uint32_t sdio_voltage;
#endif
#if VSF_USE_UI == ENABLED && VSF_DISP_USE_FB == ENABLED
    vsf_hw_fb_t hw_fb;
    vk_disp_fb_t display_fb_layer0;
    vk_disp_t *display_dev;
    vsf_gpio_t *bl_port;
    vsf_gpio_t *rst_port;
    uint8_t bl_pin, rst_pin;
#   if      defined(VSF_BOARD_RGBLCD_LAYER1_WIDTH)                              \
        &&  defined(VSF_BOARD_RGBLCD_LAYER1_HEIGHT)                             \
        &&  defined(VSF_BOARD_RGBLCD_LAYER1_COLOR)                              \
        &&  defined(VSF_BOARD_RGBLCD_LAYER1_SRAM_BUFFER_T)
    vk_disp_fb_t display_fb_layer1;
#   endif
#endif
#if VSF_USE_AUDIO == ENABLED
    vk_audio_dev_t *audio_dev;
#   if VSF_AUDIO_USE_DUMMY == ENABLED
    vk_audio_dummy_dev_t audio_dummy;
#   endif
#endif
} vsf_board_t;
#endif

/*============================ GLOBAL VARIABLES ==============================*/

#if VSF_USE_BOARD == ENABLED
extern vsf_board_t vsf_board;
#endif

#if VSF_USE_USB_DEVICE == ENABLED
extern const i_usb_dc_t VSF_USB_DC0;
#endif

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

extern void vsf_board_init(void);

#if VSF_USE_LINUX == ENABLED
extern void vsf_board_init_linux(void);
#endif

#endif      // __VSF_BOARD_H__
