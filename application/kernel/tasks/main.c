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
    const char *name;
    int counter;
    int interval_ms;
)

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static vsf_eda_t __eda_task;
static vsf_teda_t __teda_task;
static __thread_task_t __thread_task_a = {
    .counter = 0,
};
static __thread_task_t __thread_task_b = {
    .counter = 0,
};

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

implement_vsf_thread(__thread_task_t)
{
    vsf_trace_info("thread task started\n");
    while (true) {
        vsf_thread_delay_ms(vsf_pthis->interval_ms);
        vsf_trace_info("%s: thread task timer triggered %d\n", vsf_pthis->name, vsf_pthis->counter++);
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

    __thread_task_a.name = "thread_task_a";
    __thread_task_a.interval_ms = 1000;
    init_vsf_thread(__thread_task_t, &__thread_task_a, vsf_prio_0);

    __thread_task_b.name = "thread_task_b";
    __thread_task_b.interval_ms = 500;
    init_vsf_thread(__thread_task_t, &__thread_task_b, vsf_prio_0);
    return 0;
}
