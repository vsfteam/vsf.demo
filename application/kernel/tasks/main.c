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

#include "vsf.h"
#include "vsf_board.h"

/*============================ MACROS ========================================*/

#if VSF_USE_TRACE != ENABLED
#   error this demo depends on VSF_USE_TRACE, please enable it!
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

declare_vsf_thread(__thread_task_t)
define_vsf_thread(__thread_task_t, 1024,
    int counter;
)

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static vsf_eda_t __eda_task;
static vsf_teda_t __teda_task;
static __thread_task_t __thread_task;

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

implement_vsf_thread(__thread_task_t)
{
    vsf_trace_info("thread task started\n");
    vsf_pthis->counter = 0;
    while (true) {
        vsf_thread_delay_ms(1000);
        vsf_trace_info("%d: thread task timer triggered\n", vsf_pthis->counter++);
    }
}

static void __eda_evthandler(vsf_eda_t *eda, vsf_evt_t evt)
{
    switch (evt) {
    case VSF_EVT_INIT:
        vsf_trace_info("eda task started\n");
        break;
    }
}

static void __teda_evthandler(vsf_eda_t *eda, vsf_evt_t evt)
{
    switch (evt) {
    case VSF_EVT_INIT:
        vsf_trace_info("teda task started\n");
        vsf_teda_set_timer_ms(1000);
        break;
    case VSF_EVT_TIMER:
        vsf_trace_info("teda task timer triggered\n");
        break;
    }
}

int VSF_USER_ENTRY(void)
{
    vsf_board_init();
    vsf_start_trace();

    vsf_eda_start(&__eda_task, &(vsf_eda_cfg_t){
        .fn.evthandler          = __eda_evthandler,
        .priority               = vsf_prio_0,
    });
    vsf_teda_start(&__teda_task, &(vsf_eda_cfg_t){
        .fn.evthandler          = __teda_evthandler,
        .priority               = vsf_prio_0,
    });
    init_vsf_thread(__thread_task_t, &__thread_task, vsf_prio_0);
    return 0;
}
