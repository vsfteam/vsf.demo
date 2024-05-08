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

#ifndef __VSF_BOARD_EXT_GAMEPAD_CFG_H__
#define __VSF_BOARD_EXT_GAMEPAD_CFG_H__

/*============================ INCLUDES ======================================*/

#include <vsf_board_cfg.h>

/*============================ MACROS ========================================*/

/*----------------------------------------------------------------------------*
 * Hal Driver Configurations                                                  *
 *----------------------------------------------------------------------------*/

#define VSF_HAL_USE_74HC165_GPIO                        ENABLED
#define VSF_HAL_USE_74HC595_GPIO                        ENABLED
#define VSF_HAL_USE_TIMER_GPIO_PWM                      ENABLED
#define VSF_HAL_USE_ADS7830_ADC                         ENABLED

#define VSF_BOARD_EXT_GAMEPAD_GPIO_IN_HOME              0
#define VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LS                1
#define VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LD                2
#define VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LR                3
#define VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LB                4
#define VSF_BOARD_EXT_GAMEPAD_GPIO_IN_ML                5
#define VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LU                6
#define VSF_BOARD_EXT_GAMEPAD_GPIO_IN_LL                7
#define VSF_BOARD_EXT_GAMEPAD_GPIO_IN_SPECIAL           8
#define VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RS                9
#define VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RD                10
#define VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RR                11
#define VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RB                12
#define VSF_BOARD_EXT_GAMEPAD_GPIO_IN_MR                13
#define VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RU                14
#define VSF_BOARD_EXT_GAMEPAD_GPIO_IN_RL                15
#define VSF_BOARD_EXT_GAMEPAD_GPIO_IN_INVERSE           ENABLED

#define VSF_BOARD_EXT_GAMEPAD_GPIO_OUT_RUMBLE_TL        0
#define VSF_BOARD_EXT_GAMEPAD_GPIO_OUT_RUMBLE_TR        1
#define VSF_BOARD_EXT_GAMEPAD_GPIO_OUT_RUMBLE_ML        2
#define VSF_BOARD_EXT_GAMEPAD_GPIO_OUT_RUMBLE_MR        3
#define VSF_BOARD_EXT_GAMEPAD_GPIO_OUT_LED0_PIN         4
#define VSF_BOARD_EXT_GAMEPAD_GPIO_OUT_LED1_PIN         5
#define VSF_BOARD_EXT_GAMEPAD_GPIO_OUT_LED2_PIN         6
#define VSF_BOARD_EXT_GAMEPAD_GPIO_OUT_LED3_PIN         7

#define VSF_BOARD_EXT_GAMEPAD_ADC_CHANNEL_LX            0
#define VSF_BOARD_EXT_GAMEPAD_ADC_CHANNEL_LY            1
#define VSF_BOARD_EXT_GAMEPAD_ADC_CHANNEL_RX            2
#define VSF_BOARD_EXT_GAMEPAD_ADC_CHANNEL_RY            3
#define VSF_BOARD_EXT_GAMEPAD_ADC_CHANNEL_LT            4
#define VSF_BOARD_EXT_GAMEPAD_ADC_CHANNEL_RT            5

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#endif      // __VSF_BOARD_EXT_GAMEPAD_CFG_H__
