#include <vsf.h>
#include <btstack.h>

static vsf_eda_t *__btstack_scan_eda;
static void __btstack_scan_pkthandler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    uint8_t event = hci_event_packet_get_type(packet);
    bd_addr_t addr;

    switch (event) {
    case HCI_EVENT_EXTENDED_INQUIRY_RESPONSE: {
            uint8_t *eir_data;
            ad_context_t context;
            int event_type = hci_event_packet_get_type(packet);
            int num_reserved_fields = (event_type == HCI_EVENT_INQUIRY_RESULT) ? 2 : 1;
            if (event_type == HCI_EVENT_EXTENDED_INQUIRY_RESPONSE) {
                // EIR packets only contain a single inquiry response
                eir_data = &packet[3 + (6 + 1 + num_reserved_fields + 3 + 2 + 1)];
                for (ad_iterator_init(&context, EXTENDED_INQUIRY_RESPONSE_DATA_LEN, eir_data) ; ad_iterator_has_more(&context) ; ad_iterator_next(&context)){
                    uint8_t data_type    = ad_iterator_get_data_type(&context);
                    uint8_t data_size    = ad_iterator_get_data_len(&context);
                    const uint8_t * data = ad_iterator_get_data(&context);
                    vsf_trace_debug("eir: type %d size %d:" VSF_TRACE_CFG_LINEEND, data_type, data_size);
                    vsf_trace_buffer(VSF_TRACE_DEBUG, (void *)data, data_size);
                }
            }
        }
        break;
    case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
            vsf_trace_info("btstack started..." VSF_TRACE_CFG_LINEEND);

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
        vsf_trace_info("Inquiry scan done.." VSF_TRACE_CFG_LINEEND);
        VSF_ASSERT(__btstack_scan_eda != NULL);
        vsf_eda_post_evt(__btstack_scan_eda, VSF_EVT_USER);
        break;
    default:
        break;
    }
}

int btstack_scan_main(int argc, char **argv)
{
    hci_set_inquiry_mode(INQUIRY_MODE_RSSI_AND_EIR);
    hci_power_control(HCI_POWER_ON);

    btstack_packet_callback_registration_t __hci_event_callback_registration = {
        .callback       = &__btstack_scan_pkthandler,
    };
    hci_add_event_handler(&__hci_event_callback_registration);

    __btstack_scan_eda = vsf_eda_get_cur();
    VSF_ASSERT(__btstack_scan_eda != NULL);
    vsf_thread_wfe(VSF_EVT_USER);
    __btstack_scan_eda = NULL;

    hci_remove_event_handler(&__hci_event_callback_registration);
    hci_power_control(HCI_POWER_OFF);

    return 0;
}
