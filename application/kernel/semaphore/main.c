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
    vsf_sem_t *sem;
)

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static vsf_sem_t __sem;

static vsf_eda_t __eda_task;
static vsf_teda_t __teda_task;
static __thread_task_t __thread_task = {
    .sem = &__sem,
};

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

implement_vsf_thread(__thread_task_t)
{
    vsf_sync_reason_t reason;
    while (true) {
//        reason = vsf_thread_sem_pend(vsf_pthis->sem, -1);
        reason = vsf_thread_sem_pend(vsf_pthis->sem, vsf_systimer_ms_to_tick(500));
        switch (reason) {
        case VSF_SYNC_GET:
            vsf_trace_info("thread got semaphore\n");
            break;
        case VSF_SYNC_TIMEOUT:
            vsf_trace_info("thread timeout to get semaphore\n");
            break;
        }
    }
}

static void __eda_evthandler(vsf_eda_t *eda, vsf_evt_t evt)
{
    switch (evt) {
    case VSF_EVT_INIT:
    wait_sem_again:
        if (vsf_eda_sem_pend(&__sem) != VSF_ERR_NONE) {
            // can not get semaphore now, break and wait VSF_EVT_SYNC
            break;
        }
        // got semaphore, all through
    case VSF_EVT_SYNC:
        vsf_trace_info("eda task got semaphore\n");
        goto wait_sem_again;
    }
}

static void __teda_evthandler(vsf_eda_t *eda, vsf_evt_t evt)
{
    switch (evt) {
    case VSF_EVT_TIMER:
        vsf_eda_sem_post(&__sem);
        // fall through to set next timer
    case VSF_EVT_INIT:
        vsf_teda_set_timer_ms(1000);
        break;
    }
}

int VSF_USER_ENTRY(void)
{
    vsf_board_init();
    vsf_start_trace();

    vsf_eda_sem_init(&__sem, 2);

    init_vsf_thread(__thread_task_t, &__thread_task, vsf_prio_0);
    vsf_eda_start(&__eda_task, &(vsf_eda_cfg_t){
        .fn.evthandler          = __eda_evthandler,
        .priority               = vsf_prio_0,
    });
    vsf_teda_start(&__teda_task, &(vsf_eda_cfg_t){
        .fn.evthandler          = __teda_evthandler,
        .priority               = vsf_prio_0,
    });

    return 0;
}
