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

/*
 * eda display demo
 * Refresh display defined in vsf_board.
 * 
 * Dependency:
 * Board:
 *   vsf_board.display_dev
 */

/*============================ INCLUDES ======================================*/

#define __VSF_DISP_CLASS_INHERIT__
#include "vsf.h"
#include "vsf_board.h"

/*============================ MACROS ========================================*/

#if VSF_USE_TRACE != ENABLED
#   error this demo depends on VSF_USE_TRACE, please enable it!
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static vsf_teda_t __teda_task;
static void *__framebuffer;
static int __fps;

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

static void __disp_on_ready(vk_disp_t *disp)
{
    vsf_eda_post_evt((vsf_eda_t *)disp->ui_data, VSF_EVT_USER);
}

static void __disp_demo_update_buffer(uint16_t *buf, uint32_t size)
{
    static const uint16_t __colors[] = {0x1F << 11, 0x3F << 6, 0x1F};
    static int __color_index = 0;

    for (int i = 0; i < size; i++) {
        buf[i] = __colors[__color_index];
    }
    __color_index = (__color_index + 1) % dimof(__colors);
}

static void __teda_task_evthandler(vsf_eda_t *eda, vsf_evt_t evt)
{
    switch (evt) {
    case VSF_EVT_INIT:
        if (NULL == vsf_board.display_dev) {
            vsf_trace_error("no display found.\n");
            break;
        }

        __framebuffer = vsf_heap_malloc(vsf_disp_get_frame_size(vsf_board.display_dev));
        if (NULL == __framebuffer) {
            vsf_trace_error("not enough heap for framebuffer.\n");
            break;
        }

        __fps = 0;
        vsf_board.display_dev->ui_data = vsf_eda_get_cur();
        vsf_board.display_dev->ui_on_ready = __disp_on_ready;
        // if vk_disp_init succeed, ui_on_ready will be called after initializaion finished.
        if (VSF_ERR_NONE != vk_disp_init(vsf_board.display_dev)) {
            vsf_trace_error("fail to initialize display.\n");
            break;
        }

        vsf_teda_set_timer_ms(1000);
        break;
    case VSF_EVT_TIMER:
        vsf_trace_info("fps: %d\n", __fps);
        __fps = 0;
        vsf_teda_set_timer_ms(1000);
        break;
    case VSF_EVT_USER:
        __disp_demo_update_buffer((uint16_t *)__framebuffer,
            vsf_board.display_dev->param.width * vsf_board.display_dev->param.height);
        // if area is NULL, then refresh full screen
        vk_disp_refresh(vsf_board.display_dev, NULL, __framebuffer);
        __fps++;
        break;
    }
}

int VSF_USER_ENTRY(void)
{
    vsf_board_init();
    vsf_start_trace();

    vsf_teda_start(&__teda_task, &(vsf_eda_cfg_t){
        .fn.evthandler      = __teda_task_evthandler,
        .priority           = vsf_prio_0,
    });
    return 0;
}
