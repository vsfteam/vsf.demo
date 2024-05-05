#define __VSF_DISP_CLASS_INHERIT__
#include <vsf.h>
#include <sgl.h>

#if VSF_USE_UI != ENABLED
#   error Please enable VSF_USE_UI
#endif
#if VSF_KERNEL_CFG_SUPPORT_THREAD == ENABLED && VSF_KERNEL_USE_SIMPLE_SHELL != ENABLED
#   error Please enable VSF_KERNEL_USE_SIMPLE_SHELL in thread mode
#endif

#define SGL_INPUT_QUEUE_LEN                     8

static bool __sgl_disp_ready = false;
extern void sgl_disp_area(int16_t x1, int16_t y1, int16_t x2, int16_t y2, const sgl_color_t *src);
static void __sgl_disp_on_ready(vk_disp_t *disp);
static sgl_device_panel_t __sgl_panel = {
    .flush_area     = sgl_disp_area,
};
static vk_disp_t *__sgl_disp = NULL;

#if VSF_USE_INPUT == ENABLED
#   if VSF_INPUT_CFG_REGISTRATION_MECHANISM != ENABLED
#       error Please enable VSF_INPUT_CFG_REGISTRATION_MECHANISM
#   endif
#   if VSF_USE_FIFO != ENABLED
#       error Please enable VSF_USE_FIFO
#   endif

typedef struct __vsf_input_record_t {
    vk_input_type_t type;
    vk_input_evt_t evt;
} __vsf_input_record_t;

dcl_vsf_fifo(__vsf_input_record_t)
def_vsf_fifo(__vsf_input_queue, __vsf_input_record_t, SGL_INPUT_QUEUE_LEN)
static VSF_CAL_NO_INIT vsf_fifo(__vsf_input_queue) __vsf_input_queue;

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

void vsf_disp_bind_sgl(vk_disp_t *disp)
{
    VSF_ASSERT(disp != NULL);
    VSF_ASSERT(vsf_disp_get_pixel_bitsize(disp) == SGL_CONFIG_PANEL_PIXEL_DEPTH);
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
    vsf_fifo_init((vsf_fifo_t *)&__vsf_input_queue, SGL_INPUT_QUEUE_LEN);
    vk_input_notifier_register(&__vsf_input_notifier);
    sgl_device_register(&__sgl_panel, &__sgl_input);
#else
    sgl_device_register(&__sgl_panel, NULL);
#endif
}

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
        .size.x     = x2 - x1,
        .size.y     = y2 - y1,
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

#if VSF_USE_INPUT == ENABLED
void __vsf_input_on_evt(vk_input_notifier_t *notifier, vk_input_type_t type, vk_input_evt_t *evt)
{
    __vsf_input_record_t record = {
        .type       = type,
        .evt        = *evt,
    };
    vsf_fifo_push((vsf_fifo_t *)&__vsf_input_queue, (uintptr_t)&record, sizeof(__vsf_input_record_t));
}

sgl_event_pos_t sgl_input_get(void *data)
{
    __vsf_input_record_t record;
    sgl_event_pos_t sdl_evt = {
        .type       = SGL_EVENT_NULL,
    };

    while (vsf_fifo_pop((vsf_fifo_t *)&__vsf_input_queue, (uintptr_t)&record, sizeof(__vsf_input_record_t))) {
        switch (record.type) {
        case VSF_INPUT_TYPE_MOUSE:
            if (vsf_input_mouse_evt_button_get(&record.evt) == VSF_INPUT_MOUSE_BUTTON_LEFT) {
                sdl_evt.type = vsf_input_mouse_evt_button_is_down(&record.evt) ? SGL_EVENT_PRESSED : SGL_EVENT_RELEASED;
                sdl_evt.x = vsf_input_mouse_evt_get_x(&record.evt);
                sdl_evt.y = vsf_input_mouse_evt_get_y(&record.evt);
            }
            break;
        case VSF_INPUT_TYPE_TOUCHSCREEN:
            if (vsf_input_touchscreen_get_id(&record.evt) == 0) {
                sdl_evt.type = vsf_input_touchscreen_is_down(&record.evt) ? SGL_EVENT_PRESSED : SGL_EVENT_RELEASED;
                sdl_evt.x = vsf_input_touchscreen_get_x(&record.evt);
                sdl_evt.y = vsf_input_touchscreen_get_y(&record.evt);
            }
            break;
        case VSF_INPUT_TYPE_KEYBOARD:
            // TODO: process keyboard event if sgl supports keyboard
            break;
        }
        if (sdl_evt.type != SGL_EVENT_NULL) {
            break;
        }
    }
    return sdl_evt;
}
#endif
