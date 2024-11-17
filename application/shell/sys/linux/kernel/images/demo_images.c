/*****************************************************************************
 *   Copyright(C)2009-2019 by VSF Team                                       *
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

#if VSF_USE_UI == ENABLED && VSF_USE_TINY_GUI == ENABLED

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
const vsf_tgui_tile_buf_root_t corner12_L  = {
    .use_as__vsf_tgui_tile_core_t = {
        .Attribute = {
            .u2RootTileType = 0,
            .u2ColorType = 2,
            .u3ColorSize = 3,
            .bIsRootTile = 1,
        },
#if VSF_TGUI_CFG_SUPPORT_NAME_STRING == ENABLED
        .name_ptr = "corner12_L",
#endif
    },
    .tSize = {
        .iWidth = 24,
        .iHeight = 24,
    },
    .ptBitmap = (vsf_tgui_disp_ram_uintptr_t)0x00,
};

const vsf_tgui_tile_buf_root_t corner16_L  = {
    .use_as__vsf_tgui_tile_core_t = {
        .Attribute = {
            .u2RootTileType = 0,
            .u2ColorType = 2,
            .u3ColorSize = 3,
            .bIsRootTile = 1,
        },
#if VSF_TGUI_CFG_SUPPORT_NAME_STRING == ENABLED
        .name_ptr = "corner16_L",
#endif
    },
    .tSize = {
        .iWidth = 32,
        .iHeight = 32,
    },
    .ptBitmap = (vsf_tgui_disp_ram_uintptr_t)0x240,
};

const vsf_tgui_tile_buf_root_t empty_L  = {
    .use_as__vsf_tgui_tile_core_t = {
        .Attribute = {
            .u2RootTileType = 0,
            .u2ColorType = 2,
            .u3ColorSize = 3,
            .bIsRootTile = 1,
        },
#if VSF_TGUI_CFG_SUPPORT_NAME_STRING == ENABLED
        .name_ptr = "empty_L",
#endif
    },
    .tSize = {
        .iWidth = 64,
        .iHeight = 64,
    },
    .ptBitmap = (vsf_tgui_disp_ram_uintptr_t)0x640,
};

const vsf_tgui_tile_t* tiles_list[3] = {
    (const vsf_tgui_tile_t *)&corner12_L, //corner12:L, width: 24, height: 24
    (const vsf_tgui_tile_t *)&corner16_L, //corner16:L, width: 32, height: 32
    (const vsf_tgui_tile_t *)&empty_L, //empty:L, width: 64, height: 64
};

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/



#endif


/* EOF */

