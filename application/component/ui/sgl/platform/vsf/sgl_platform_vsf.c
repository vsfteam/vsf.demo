#define __VSF_DISP_CLASS_INHERIT__
#include <vsf.h>
#include <sgl.h>

#if VSF_USE_UI != ENABLED
#   error Please enable VSF_USE_UI
#endif
#if VSF_KERNEL_CFG_SUPPORT_THREAD == ENABLED && VSF_KERNEL_USE_SIMPLE_SHELL != ENABLED
#   error Please enable VSF_KERNEL_USE_SIMPLE_SHELL in thread mode
#endif

#ifndef SGL_CONFIG_FRAME_BUFFER_XRES
#   error Please define SGL_CONFIG_FRAME_BUFFER_XRES
#endif

static bool __sgl_disp_ready = false;
extern void sgl_disp_area(int16_t x1, int16_t y1, int16_t x2, int16_t y2, const sgl_color_t *src);
static void __sgl_disp_on_ready(vk_disp_t *disp);
static sgl_color_t __sgl_framebuffer[SGL_CONFIG_FRAME_BUFFER_XRES * 20];
static sgl_device_panel_t __sgl_panel = {
    .framebuffer    = __sgl_framebuffer,
    .buffer_size    = sizeof(__sgl_framebuffer),
    .flush_area     = sgl_disp_area,
};
static vk_disp_t *__sgl_disp = NULL;

#if VSF_USE_TRACE == ENABLED
static int __sgl_stdout_put(char *str);
static sgl_device_stdout_t __sgl_stdout_dev = {
    .put = __sgl_stdout_put,
};
#endif

#if VSF_USE_INPUT == ENABLED
#   if VSF_INPUT_CFG_REGISTRATION_MECHANISM != ENABLED
#       error Please enable VSF_INPUT_CFG_REGISTRATION_MECHANISM
#   endif

typedef struct __vsf_input_record_t {
    vk_input_type_t type;
    vk_input_evt_t *evt;
} __vsf_input_record_t;

extern sgl_event_pos_t sgl_input_get(void *data);
static sgl_device_input_t __sgl_input = {
    .get            = sgl_input_get,
};
static void __vsf_input_on_evt(vk_input_notifier_t *notifier, vk_input_type_t type, vk_input_evt_t *evt);
vk_input_notifier_t __vsf_input_notifier = {
    .on_evt         = __vsf_input_on_evt,
    .mask           = (1 << VSF_INPUT_TYPE_KEYBOARD) | (1 << VSF_INPUT_TYPE_MOUSE) | (1 << VSF_INPUT_TYPE_TOUCHSCREEN),
};
#endif

static void __sgl_disp_on_ready(vk_disp_t *disp)
{
#if VSF_KERNEL_CFG_SUPPORT_THREAD == ENABLED
    vsf_eda_t *eda = (vsf_eda_t *)disp->ui_data;
    if (eda != NULL) {
        vsf_eda_post_evt(eda, VSF_EVT_USER);
    } else
#endif
    {
        __sgl_disp_ready = true;
    }
}

void sgl_disp_area(int16_t x1, int16_t y1, int16_t x2, int16_t y2, const sgl_color_t *src)
{
    vk_disp_t *disp = __sgl_disp;
    VSF_ASSERT(disp != NULL);
    VSF_ASSERT( (src != NULL) && (x2 >= x1) && (y2 >= y1)
            &&  (x1 < disp->param.width) && (x2 < disp->param.width)
            &&  (y1 < disp->param.height) && (y2 < disp->param.height));

#if VSF_KERNEL_CFG_SUPPORT_THREAD == ENABLED
    vsf_eda_t *eda = vsf_eda_get_cur();
    bool is_thread = NULL == eda ? false : vsf_eda_is_stack_owner(eda);

    if (is_thread) {
        disp->ui_data = eda;
    } else
#endif
    {
        __sgl_disp_ready = false;
        disp->ui_data = NULL;
    }

    disp->ui_on_ready = __sgl_disp_on_ready;
    vk_disp_refresh(disp, &(vk_disp_area_t){
        .pos.x      = x1,
        .pos.y      = y1,
        .size.x     = x2 - x1 + 1,
        .size.y     = y2 - y1 + 1,
    }, (void *)src);

#if VSF_KERNEL_CFG_SUPPORT_THREAD == ENABLED
    if (is_thread) {
        vsf_thread_wfe(VSF_EVT_USER);
    } else
#endif
    {
        while (!__sgl_disp_ready);
    }
}

#if VSF_USE_TRACE == ENABLED
static int __sgl_stdout_put(char *str)
{
    vsf_trace_debug("%s\n", str);
    return strlen(str) + 1;
}
#endif

#if VSF_USE_INPUT == ENABLED
void __vsf_input_on_evt(vk_input_notifier_t *notifier, vk_input_type_t type, vk_input_evt_t *evt)
{
    __vsf_input_record_t record = {
        .type       = type,
        .evt        = evt,
    };
    sgl_device_input_handle(&record);
}

sgl_event_pos_t sgl_input_get(void *data)
{
    __vsf_input_record_t *record = data;
    sgl_event_pos_t sdl_evt = {
        .type       = SGL_EVENT_NULL,
    };

    switch (record->type) {
    case VSF_INPUT_TYPE_MOUSE:
        if (vsf_input_mouse_evt_button_get(record->evt) == VSF_INPUT_MOUSE_BUTTON_LEFT) {
            sdl_evt.type = vsf_input_mouse_evt_button_is_down(record->evt) ? SGL_EVENT_PRESSED : SGL_EVENT_RELEASED;
            sdl_evt.x = vsf_input_mouse_evt_get_x(record->evt);
            sdl_evt.y = vsf_input_mouse_evt_get_y(record->evt);
        }
        break;
    case VSF_INPUT_TYPE_TOUCHSCREEN:
        if (vsf_input_touchscreen_get_id(record->evt) == 0) {
            sdl_evt.type = vsf_input_touchscreen_is_down(record->evt) ? SGL_EVENT_PRESSED : SGL_EVENT_RELEASED;
            sdl_evt.x = vsf_input_touchscreen_get_x(record->evt);
            sdl_evt.y = vsf_input_touchscreen_get_y(record->evt);
        }
        break;
    case VSF_INPUT_TYPE_KEYBOARD:
        // TODO: process keyboard event if sgl supports keyboard
        break;
    }
    return sdl_evt;
}
#endif

vsf_err_t sgl_platform_bind_vsf(vk_disp_t *disp)
{
    VSF_ASSERT(disp != NULL);
    // if assert below, change SGL_CONFIG_PANEL_PIXEL_DEPTH to fit the pixel bitsize of the vsf_board.display_dev
    VSF_ASSERT(vsf_disp_get_pixel_bitsize(disp) == SGL_CONFIG_PANEL_PIXEL_DEPTH);
    // if assert below, change SGL_CONFIG_FRAME_BUFFER_XRES to fit the width of the vsf_board.display_dev
    VSF_ASSERT(vsf_disp_get_width(disp) == SGL_CONFIG_FRAME_BUFFER_XRES);
    // if assert below, sgl_bind_vsf is called more than once, which should not happen
    VSF_ASSERT(NULL == __sgl_disp);

#if VSF_KERNEL_CFG_SUPPORT_THREAD == ENABLED
    vsf_eda_t *eda = vsf_eda_get_cur();
    bool is_thread = NULL == eda ? false : vsf_eda_is_stack_owner(eda);

    if (is_thread) {
        disp->ui_data = eda;
    } else
#endif
    {
        __sgl_disp_ready = false;
        disp->ui_data = NULL;
    }

    disp->ui_on_ready = __sgl_disp_on_ready;
    vk_disp_init(disp);

#if VSF_KERNEL_CFG_SUPPORT_THREAD == ENABLED
    if (is_thread) {
        vsf_thread_wfe(VSF_EVT_USER);
    } else
#endif
    {
        while (!__sgl_disp_ready);
    }
    __sgl_disp = disp;

    __sgl_panel.xres = disp->param.width;
    __sgl_panel.yres = disp->param.height;

#if VSF_USE_INPUT == ENABLED
    vk_input_notifier_register(&__vsf_input_notifier);
    if (sgl_device_register(&__sgl_panel, &__sgl_input) != 0) {
        vsf_trace_error("fail to register sgl_panel" VSF_TRACE_CFG_LINEEND);
        return VSF_ERR_FAIL;
    }
#else
    if (sgl_device_register(&__sgl_panel, NULL) != 0) {
        vsf_trace_error("fail to register sgl_panel" VSF_TRACE_CFG_LINEEND);
        return VSF_ERR_FAIL;
    }
#endif

#if VSF_USE_TRACE == ENABLED
    sgl_device_stdout_register(&__sgl_stdout_dev);
#endif
    return VSF_ERR_NONE;
}
