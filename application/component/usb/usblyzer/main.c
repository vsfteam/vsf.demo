#include <vsf.h>
#include <vsf_board.h>

#include "component/usb/utils/usbmitm/vsf_usbmitm.h"

#define usblyzer_trace_evt              vsf_trace_debug
#define usblyzer_trace_err              vsf_trace_error
#define usblyzer_trace_setup(__req)     do {                                    \
        vsf_trace_info("SETUP:");                                               \
        vsf_trace_buffer(VSF_TRACE_INFO, __req, 8, VSF_TRACE_DF_U8_16_N);       \
    } while (0)
#define usblyzer_trace_data(__trans)      do {                                  \
        vsf_trace_info("%s%d:" VSF_TRACE_CFG_LINEEND,                           \
            ((__trans)->ep & USB_DIR_MASK) == USB_DIR_IN ? "IN" : "OUT",        \
            (__trans)->ep & 0x0F                                                \
        );                                                                      \
        vsf_trace_buffer(VSF_TRACE_INFO, (__trans)->buffer, (__trans)->size);   \
    } while (0)

static void __usblyzer_callback(vsf_usb_mitm_t *mitm, vsf_usb_mitm_evt_t evt, void *param)
{
    switch (evt) {
    case USB_ON_ATTACH:
        usblyzer_trace_evt("attach" VSF_TRACE_CFG_LINEEND);
        break;
    case USB_ON_DETACH:
        usblyzer_trace_evt("detach" VSF_TRACE_CFG_LINEEND);
        break;
    case USB_ON_RESET:
        usblyzer_trace_evt("reset" VSF_TRACE_CFG_LINEEND);
        break;
    case USB_ON_SETUP:
        usblyzer_trace_setup((struct usb_ctrlrequest_t *)param);
        break;
    case USB_ON_ERROR:
        usblyzer_trace_err("error" VSF_TRACE_CFG_LINEEND);
        break;
    case USB_ON_SUSPEND:
        usblyzer_trace_evt("suspend" VSF_TRACE_CFG_LINEEND);
        break;
    case USB_ON_RESUME:
        usblyzer_trace_evt("resume" VSF_TRACE_CFG_LINEEND);
        break;
    case USB_ON_SOF:
        //usblyzer_trace_evt("sof" VSF_TRACE_CFG_LINEEND);
        break;
    case USB_ON_IN:
    case USB_ON_OUT:
        usblyzer_trace_data((vk_usbd_trans_t *)param);
        break;
    case USB_ON_NAK:
        break;
    case USB_ON_STATUS:
        break;
    case USB_ON_UNDERFLOW:
        usblyzer_trace_err("underflow" VSF_TRACE_CFG_LINEEND);
        break;
    case USB_ON_OVERFLOW:
        usblyzer_trace_err("overflow" VSF_TRACE_CFG_LINEEND);
        break;
    }
}

int VSF_USER_ENTRY(void)
{
    vsf_board_init();

    {
        static vsf_usb_mitm_t __usbmitm = {
            .usb_dev.drv    = &VSF_USB_DC0,
            .callback       = __usblyzer_callback,
        };
        __usbmitm.usb_host.drv = vsf_board.usbh_dev.drv;
        __usbmitm.usb_host.param = vsf_board.usbh_dev.param;
        vsf_usb_mitm_start(&__usbmitm);
    }

    return 0;
}
