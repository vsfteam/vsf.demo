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
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

int VSF_USER_ENTRY(void)
{
    enum {
        STATE_OPEN_ROOT,
        STATE_MOUNT,
        STATE_OPEN_CHILD,
        STATE_CLOSE_CHILD,
        STATE_CLOSE_ROOT,
    };
    vsf_err_t err;
    static vk_file_t *root, *child;

    switch (vsf_eda_get_cur_evt()) {
    case VSF_EVT_INIT:
        vsf_board_init();
        vsf_start_trace();

        vk_fs_init();
        vsf_eda_set_user_value(STATE_OPEN_ROOT);
        vk_file_open(NULL, "/", &root);
        break;
    case VSF_EVT_RETURN:
        err = (vsf_err_t)vsf_eda_get_return_value();

        switch (vsf_eda_get_user_value()) {
        case STATE_OPEN_ROOT:
            if (err != VSF_ERR_NONE) {
                vsf_trace_error("fail to open \"/\".\n");
                break;
            }

            vsf_eda_set_user_value(STATE_MOUNT);
            vk_fs_mount(root, vsf_board.fsop, vsf_board.fsinfo);
            break;
        case STATE_MOUNT:
            if (err != VSF_ERR_NONE) {
                vsf_trace_error("fail to mount.\n");
                break;
            }

        open_next_child:
            vsf_eda_set_user_value(STATE_OPEN_CHILD);
            vk_file_open(root, NULL, &child);
            break;
        case STATE_OPEN_CHILD:
            if (err != VSF_ERR_NONE) {
                vsf_trace_info("\n");

                vsf_eda_set_user_value(STATE_CLOSE_ROOT);
                vk_file_close(root);
                break;
            }

            vsf_trace_info("%s ", child->name);

            vsf_eda_set_user_value(STATE_CLOSE_CHILD);
            vk_file_close(child);
            break;
        case STATE_CLOSE_CHILD:
            goto open_next_child;
        case STATE_CLOSE_ROOT:
            break;
        }
        break;
    }
    return 0;
}
