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
 * basic_audio_player demo
 * Play a 48K 16bit 2channel audio to audio device defined in vsf_board.
 *
 * Dependency:
 * Board:
 *   vsf_board.audio_dev
 */

/*============================ INCLUDES ======================================*/

#define __VSF_SIMPLE_STREAM_CLASS_INHERIT__
#include "vsf.h"
#include "vsf_board.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

// 2ms buffer size for ticktock audio stream, 1ms interrupt interval
describe_mem_stream(__audio_capture_stream, 2 * 96)

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

static void __audio_capture_stream_evthandler(vsf_stream_t *stream, void *param, vsf_stream_evt_t evt)
{
    if (VSF_STREAM_ON_IN == evt) {
        vsf_stream_read(stream, NULL, vsf_stream_get_data_size(stream));
    }
}

int VSF_USER_ENTRY(void)
{
    vsf_board_init();

#if VSF_USE_TRACE == ENABLED
    vsf_start_trace();
#endif

    vsf_stream_init(&__audio_capture_stream.use_as__vsf_stream_t);
    __audio_capture_stream.rx.evthandler = __audio_capture_stream_evthandler;
    vsf_stream_connect_rx(&__audio_capture_stream.use_as__vsf_stream_t);

    vk_audio_init(vsf_board.audio_dev);

// if playback is enabled, playback stream index is 0, and capture stream index is 1
// if playback is not enabled, capture stream index is 0
#if VSF_AUDIO_USE_PLAYBACK == ENABLED
#   define AUDIO_CAPTURE_STREAM_INDEX           1
#else
#   define AUDIO_CAPTURE_STREAM_INDEX           0
#endif
    vk_audio_start(vsf_board.audio_dev, AUDIO_CAPTURE_STREAM_INDEX,
            &__audio_capture_stream.use_as__vsf_stream_t, &(vk_audio_format_t){
        .datatype.value     = VSF_AUDIO_DATA_TYPE_LES16,
        .sample_rate        = 480,
        .channel_num        = 1,
    });
    return 0;
}
