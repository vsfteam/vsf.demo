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
    "res_cloud\n" /*width: 48, height: 48, RGBA*/ \
    "res_empty\n" /*width: 48, height: 48, RGBA*/ \
    "res_local\n" /*width: 48, height: 48, RGBA*/ \
    "corner12\n" /*width: 24, height: 24, L*/ \
    "corner16\n" /*width: 32, height: 32, L*/ \
    "None"

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
extern const vsf_tgui_tile_buf_root_t res_cloud_RGBA; //res_cloud:RGBA, width: 48, height: 48
extern const vsf_tgui_tile_buf_root_t res_empty_RGBA; //res_empty:RGBA, width: 48, height: 48
extern const vsf_tgui_tile_buf_root_t res_local_RGBA; //res_local:RGBA, width: 48, height: 48
extern const vsf_tgui_tile_buf_root_t corner12_L; //corner12:L, width: 24, height: 24
extern const vsf_tgui_tile_buf_root_t corner16_L; //corner16:L, width: 32, height: 32

extern const vsf_tgui_tile_t *tiles_list[5];

/*============================ PROTOTYPES ====================================*/


#endif

#endif
/* EOF */

