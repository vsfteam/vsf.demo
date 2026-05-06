/*****************************************************************************
 *   Copyright(C)2009-2026 by VSF Team                                       *
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
 ****************************************************************************/

/*
 * ESP-IDF LCD panel API smoke test.
 *
 * Creates a panel wrapping the board's vsf_board.display_dev and
 * exercises the full esp_lcd_panel_* API surface.
 */

/*============================ INCLUDES ======================================*/

#include <vsf_board.h>

#include "esp_err.h"
#include "esp_lcd_types.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

#include <stdio.h>

/*============================ MACROS ========================================*/

#define TEST_PASS()  printf("  [PASS] %s\n", __func__)
#define TEST_FAIL()  printf("  [FAIL] %s\n", __func__)

/*============================ LOCAL FUNCTIONS ===============================*/

static void test_panel_null_args(void)
{
    esp_lcd_panel_handle_t panel = NULL;
    if (ESP_ERR_INVALID_ARG ==
        esp_lcd_new_panel_from_vsf_disp(NULL, NULL, NULL)) TEST_PASS();
    else TEST_FAIL();

    if (ESP_ERR_INVALID_ARG ==
        esp_lcd_new_panel_from_vsf_disp(vsf_board.display_dev, NULL, NULL)) TEST_PASS();
    else TEST_FAIL();

    if (ESP_ERR_INVALID_ARG ==
        esp_lcd_new_panel_from_vsf_disp(NULL, NULL, &panel)) TEST_PASS();
    else TEST_FAIL();
}

static void test_panel_lifecycle(void)
{
    esp_lcd_panel_dev_config_t panel_cfg = {
        .rgb_ele_order   = LCD_RGB_ELEMENT_ORDER_BGR,
        .data_endian     = LCD_RGB_DATA_ENDIAN_BIG,
        .bits_per_pixel  = 16,
        .reset_gpio_num  = -1,
    };
    esp_lcd_panel_handle_t panel = NULL;
    esp_err_t ret = esp_lcd_new_panel_from_vsf_disp(
                        vsf_board.display_dev, &panel_cfg, &panel);
    if (ret != ESP_OK || !panel) { TEST_FAIL(); return; }
    TEST_PASS();

    ret = esp_lcd_panel_reset(panel);
    if (ret != ESP_OK) { TEST_FAIL(); goto done; }
    TEST_PASS();

    ret = esp_lcd_panel_init(panel);
    if (ret != ESP_OK) { TEST_FAIL(); goto done; }
    TEST_PASS();

    ret = esp_lcd_panel_disp_on_off(panel, true);
    if (ret != ESP_OK) { TEST_FAIL(); goto done; }
    TEST_PASS();

    ret = esp_lcd_panel_disp_sleep(panel, true);
    if (ret != ESP_OK) { TEST_FAIL(); goto done; }
    TEST_PASS();

    ret = esp_lcd_panel_set_brightness(panel, 50);
    if (ret != ESP_OK) { TEST_FAIL(); goto done; }
    TEST_PASS();

    ret = esp_lcd_panel_mirror(panel, true, false);
    if (ret != ESP_OK) { TEST_FAIL(); goto done; }
    TEST_PASS();

    ret = esp_lcd_panel_swap_xy(panel, false);
    if (ret != ESP_OK) { TEST_FAIL(); goto done; }
    TEST_PASS();

    ret = esp_lcd_panel_invert_color(panel, true);
    if (ret != ESP_OK) { TEST_FAIL(); goto done; }
    TEST_PASS();

    // Draw a visible red rectangle using the board display's actual width.
    vk_disp_t *disp = vsf_board.display_dev;
    uint16_t draw_w = disp->param.width  < 240 ? disp->param.width  : 240;
    uint16_t draw_h = disp->param.height < 80  ? disp->param.height : 80;
    uint16_t pixel_line[240];
    for (uint16_t c = 0; c < draw_w; c++) pixel_line[c] = 0xF800;  // red
    for (uint16_t y = 0; y < draw_h; y++) {
        ret = esp_lcd_panel_draw_bitmap(panel, 0, (int)y,
                                        (int)draw_w - 1, (int)y, pixel_line);
        if (ret != ESP_OK) { TEST_FAIL(); goto done; }
    }
    TEST_PASS();

done:
    esp_lcd_panel_del(panel);
}

/*============================ PUBLIC =========================================*/

void vsf_espidf_lcd_test(void)
{
    printf("[esp_lcd] begin\n");

    test_panel_null_args();
    test_panel_lifecycle();

    printf("[esp_lcd] end\n");
}
