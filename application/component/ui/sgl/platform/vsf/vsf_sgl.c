#define __VSF_DISP_CLASS_INHERIT__
#include <vsf.h>
#include <sgl.h>

#if VSF_USE_UI != ENABLED
#   error Please enable VSF_USE_UI
#endif

static bool __sgl_disp_ready = false;
extern void sgl_disp_area(int16_t x1, int16_t y1, int16_t x2, int16_t y2, const sgl_color_t *src);
static sgl_device_panel_t __sgl_panel = {
    .flush_area     = sgl_disp_area,
};
static vk_disp_t *__sgl_disp = NULL;

#if VSF_USE_INPUT == ENABLED
#   if VSF_INPUT_CFG_REGISTRATION_MECHANISM != ENABLED
#       error Please enable VSF_INPUT_CFG_REGISTRATION_MECHANISM
#   endif

extern sgl_event_pos_t sgl_input_get(void *data);
static sgl_device_input_t __sgl_input = {
    .get            = sgl_input_get,
};
static void __vsf_input_on_evt(vk_input_notifier_t *notifier, vk_input_type_t type, vk_input_evt_t *evt);
vk_input_notifier_t __vsf_input_notifier = {
    .on_evt         = __vsf_input_on_evt,
    .mask           = (1 << VSF_INPUT_TYPE_KEYBOARD) | (1 << VSF_INPUT_TYPE_MOUSE),
};
#endif

void __sgl_disp_on_ready(vk_disp_t *disp)
{
    vsf_eda_t *eda = (vsf_eda_t *)disp->ui_data;
    if (eda != NULL) {
        vsf_eda_post_evt(eda, VSF_EVT_USER);
    } else {
        __sgl_disp_ready = true;
    }
}

void vsf_disp_bind_sgl(vk_disp_t *disp)
{
    VSF_ASSERT(vsf_disp_get_pixel_bitsize(disp) == SGL_CONFIG_PANEL_PIXEL_DEPTH);
    VSF_ASSERT(NULL == __sgl_disp);

    vsf_eda_t *eda = vsf_eda_get_cur();
    bool is_thread = NULL == eda ? false : vsf_eda_is_stack_owner(eda);

    if (is_thread) {
        disp->ui_data = eda;
    } else {
        __sgl_disp_ready = false;
        disp->ui_data = NULL;
    }

    disp->ui_on_ready = __sgl_disp_on_ready;
    vk_disp_init(disp);

    if (is_thread) {
        vsf_thread_wfe(VSF_EVT_USER);
    } else {
        while (!__sgl_disp_ready);
    }
    __sgl_disp = disp;

    __sgl_panel.xres = disp->param.width;
    __sgl_panel.yres = disp->param.height;

#if VSF_USE_INPUT == ENABLED
    vk_input_notifier_register(&__vsf_input_notifier);
    sgl_device_register(&__sgl_panel, &__sgl_input);
#else
    sgl_device_register(&__sgl_panel, NULL);
#endif
}

void sgl_disp_area(int16_t x1, int16_t y1, int16_t x2, int16_t y2, const sgl_color_t *src)
{
    vk_disp_t *disp = __sgl_disp;
    VSF_ASSERT(disp != NULL);

    vsf_eda_t *eda = vsf_eda_get_cur();
    bool is_thread = NULL == eda ? false : vsf_eda_is_stack_owner(eda);

    if (is_thread) {
        disp->ui_data = eda;
    } else {
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

    if (is_thread) {
        vsf_thread_wfe(VSF_EVT_USER);
    } else {
        while (!__sgl_disp_ready);
    }
}

#if VSF_USE_INPUT == ENABLED
#   if VSF_INPUT_CFG_REGISTRATION_MECHANISM == ENABLED
void __vsf_input_on_evt(vk_input_notifier_t *notifier, vk_input_type_t type, vk_input_evt_t *evt)
{
}
#   endif

sgl_event_pos_t sgl_input_get(void *data)
{
}
#endif
