/******************************************************************************
 *   Copyright(C)2009-2024 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*
 *  See the License for the specific language governing permissions and       *
 *  limitations under the License.                                           *
 *                                                                           *
 *****************************************************************************/

#ifndef __VSF_TEST_BOARD_PINS_H__
#define __VSF_TEST_BOARD_PINS_H__

/*============================ GPIO TEST PINS ================================*/
/* Self-loopback pin: RP2040 SIO allows simultaneous output + input on one pin */
#define VSF_TEST_PIN_SELF_LOOPBACK      25

/* Jumper loopback pair: GP6 → GP7 (external wire required) */
#define VSF_TEST_PIN_JUMPER_OUT         6
#define VSF_TEST_PIN_JUMPER_IN          7

/* LA-monitored pin: wired to DSLogic for frequency/period measurement */
#define VSF_TEST_PIN_LA_MONITORED       4

/* UART1 pins used by gpio_pinmux and gpio_io_check suites */
#define VSF_TEST_PIN_UART1_TX           8
#define VSF_TEST_PIN_UART1_RX           9

/* Multi-pin loopback pairs */
#define VSF_TEST_PIN_MULTI_A            24
#define VSF_TEST_PIN_MULTI_B            25
#define VSF_TEST_PIN_MULTI_C            16
#define VSF_TEST_PIN_MULTI_D            17

/* Alternative EXTI trigger pins (SPI0 MOSI→MISO jumper) */
#define VSF_TEST_PIN_EXTI_ALT_OUT       3
#define VSF_TEST_PIN_EXTI_ALT_IN        4

/* Concurrent priority test pair */
#define VSF_TEST_PIN_CONCURRENT_OUT     4
#define VSF_TEST_PIN_CONCURRENT_IN      5

/*============================ I2C TEST PINS =================================*/
/* I2C0: GP20=SDA, GP21=SCL  |  I2C1: GP18=SCL, GP19=SDA */
#define VSF_TEST_PIN_I2C0_SCL           21
#define VSF_TEST_PIN_I2C0_SDA           20
#define VSF_TEST_PIN_I2C1_SCL           18
#define VSF_TEST_PIN_I2C1_SDA           19

/*============================ PWM TEST PINS =================================*/
/* PWM slice 0 on GP0/GP1 */
#define VSF_TEST_PIN_PWM_GPIO_A         0
#define VSF_TEST_PIN_PWM_GPIO_B         1

#endif      // __VSF_TEST_BOARD_PINS_H__
