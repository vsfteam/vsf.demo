/*****************************************************************************
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
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

#ifndef __VSF_TINY_GUI_IMAGE_DATA_H__
#define __VSF_TINY_GUI_IMAGE_DATA_H__

/*============================ INCLUDES ======================================*/
#include "vsf.h"

#if VSF_USE_UI == ENABLED && VSF_USE_TINY_GUI == ENABLED
/*============================ MACROS ========================================*/

#define TILES_TEXT_LIST \
    "corner12\n" /*width: 24, height: 24, L*/ \
    "corner16\n" /*width: 32, height: 32, L*/ \
    "empty\n" /*width: 64, height: 64, L*/ \
    "None"

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
extern const vsf_tgui_tile_buf_root_t corner12_L; //corner12:L, width: 24, height: 24
extern const vsf_tgui_tile_buf_root_t corner16_L; //corner16:L, width: 32, height: 32
extern const vsf_tgui_tile_buf_root_t empty_L; //empty:L, width: 64, height: 64

extern const vsf_tgui_tile_t *tiles_list[3];

/*============================ PROTOTYPES ====================================*/


#endif

#endif
/* EOF */

