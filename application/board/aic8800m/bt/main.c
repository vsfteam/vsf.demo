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
 * aic8800m btstack demo
 *
 * Dependency:
 * Board:
 *   board/aic8800m or board/aic8800m_uikit
 *
 * Submodule(except PLOOC):
 *   CMSIS(source/utilities/compiler/arm/3rd-party/CMSIS)
 *   AIC8800M_SDK_vsf(source/hal/driver/AIC/AIC8800/vendor)
 *   btstack(source/component/3rd-party/btstack/raw)
 * 
 * Include Directories necessary for btstack:
 *   vsf/source/component/3rd-party/btstack/raw/src
 *
 * Source Code:
 *   vsf/component/3rd-party/btstack
 */

/*============================ INCLUDES ======================================*/

#include "vsf.h"
#include "vsf_board.h"

#include "btstack.h"
#include "component/3rd-party/btstack/port/btstack_run_loop_vsf.h"

/*============================ MACROS ========================================*/

#if VSF_USE_TRACE != ENABLED
#   error this demo depends on VSF_USE_TRACE, please enable it!
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static btstack_packet_callback_registration_t __hci_event_callback_registration;

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

static void __btstack_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    uint8_t event = hci_event_packet_get_type(packet);
    bd_addr_t addr;

    switch (event) {
    case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
            vsf_trace_info("Starting inquiry scan.." VSF_TRACE_CFG_LINEEND);
            gap_inquiry_start(5);
        }
        break;
    case GAP_EVENT_INQUIRY_RESULT:
        gap_event_inquiry_result_get_bd_addr(packet, addr);

        vsf_trace_info("Device found: %s ",  bd_addr_to_str(addr));
        vsf_trace_info("with COD: 0x%06x, ", (unsigned int)gap_event_inquiry_result_get_class_of_device(packet));
        vsf_trace_info("pageScan %d, ",      gap_event_inquiry_result_get_page_scan_repetition_mode(packet));
        vsf_trace_info("clock offset 0x%04x",gap_event_inquiry_result_get_clock_offset(packet));

        if (gap_event_inquiry_result_get_rssi_available(packet)) {
            vsf_trace_info(", rssi %d dBm", (int8_t) gap_event_inquiry_result_get_rssi(packet));
        }
        if (gap_event_inquiry_result_get_name_available(packet)) {
            char name_buffer[240];
            int name_len = gap_event_inquiry_result_get_name_len(packet);
            memcpy(name_buffer, gap_event_inquiry_result_get_name(packet), name_len);
            name_buffer[name_len] = 0;
            vsf_trace_info(", name '%s'", name_buffer);
        }
        vsf_trace_info(VSF_TRACE_CFG_LINEEND);
        break;
    case GAP_EVENT_INQUIRY_COMPLETE:
        vsf_trace_info("Inquiry scan done." VSF_TRACE_CFG_LINEEND);
        break;
    default:
        break;
    }
}

int VSF_USER_ENTRY(void)
{
    vsf_board_init();
    vsf_start_trace();

    btstack_memory_init();
    btstack_run_loop_init(btstack_run_loop_vsf_get_instance());

    extern const hci_transport_t * hci_transport_aic8800_instance(void);
    hci_init(hci_transport_aic8800_instance(), NULL);

    hci_set_link_key_db(btstack_link_key_db_memory_instance());
    hci_set_inquiry_mode(INQUIRY_MODE_RSSI_AND_EIR);

    __hci_event_callback_registration.callback = &__btstack_packet_handler;
    hci_add_event_handler(&__hci_event_callback_registration);

    hci_power_control(HCI_POWER_ON);
    return 0;
}
