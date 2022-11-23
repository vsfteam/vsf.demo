/*
 * evue demo
 *
 * Dependency:
 * Board:
 *   vsf_board.display_dev(match LV_COLOR_DEPTH in lv_conf.h)
 *
 * Submodule(except PLOOC):
 *   evm(application/component/3rd-party/evm/raw)
 * 
 * Include Directories:
 *   vsf/source/shell/sys/linux/include
 *   vsf/source/shell/sys/linux/include/simple_libc
 *   application/component/3rd-party/evm
 *   application/component/3rd-party/evm/raw
 *   application/component/3rd-party/evm/raw/include
 *   application/component/3rd-party/evm/raw/bsp/evue-simulator/pikascript/pikascript-core
 *   application/component/3rd-party/evm/raw/bsp/evue-simulator/pikascript/pikascript-api
 *   application/component/3rd-party/evm/raw/bsp/evue-simulator/pikascript
 *   application/component/3rd-party/evm/raw/engines/pikascript
 *   application/component/3rd-party/evm/raw/components/lvgl
 *   application/component/3rd-party/evm/raw/components/lvgl/lvgl
 * 
 * Source Code:
 *   vsf/component/3rd-party/lvgl/port
 *   vsf/example/template/demo/linux_demo/mount_demo.c if mount is necessary
 *   application/component/3rd-party/evm
 *   application/component/3rd-party/evm/raw/bsp/evue-simulator/pikascript
 *   application/component/3rd-party/evm/raw/components/lvgl
 *   application/component/3rd-party/evm/raw/engines/pikascript
 *   application/component/3rd-party/evm/raw/modules/iot/common
 *   application/component/3rd-party/evm/raw/modules/iot/linux
 * 
 * Pre-defined symbols:
 *   LV_CONF_INCLUDE_SIMPLE
 *   LV_LVGL_H_INCLUDE_SIMPLE
 *   PIKASCRIPT=1
 *   PIKA_CONFIG_ENABLE=1
 */

#include <unistd.h>
#include <stdlib.h>
#include "vsf_board.h"

#include "PikaVM.h"
#include "lvgl/lvgl.h"
#include "component/3rd-party/lvgl/port/vsf_lvgl_port.h"
#include "evm_module.h"

char __platfrom_getChar(void){
    return (char)getchar();
}

char* getdir(const char* filepath) {
    int len = strlen(filepath);

    int i = len - 1;
    while(i >= 0) {
        char c = filepath[i];
        if (c == '/') {
            break;
        }
        i--;
    }

    if (i == -1 || (i == 1 && filepath[0] == '.')) {
        char* filepath = malloc(256);
        if(getcwd(filepath, 256) == NULL) {
            return NULL;
        }
        return filepath;
    }

    char* dirname = malloc(i + 1);
    memcpy(dirname, filepath, i);
    dirname[i] = '\0';

    return dirname;
}

char* getfilename(const char* filepath) {
    int len = strlen(filepath);

    int i = len - 1;
    while(i >= 0) {
        char c = filepath[i];
        if (c == '/') {
            break;
        }
        i--;
    }


    char* filename = NULL;
    if (i == -1) {
        filename = malloc(len + 1);
        memcpy(filename, filepath, len);
        filename[len] = '\0';
    } else {
        int flen = len - i;
        filename = (char *)malloc(flen + 1);
        memcpy(filename, filepath + i + 1, flen);
        filename[flen] = '\0';
    }

    return filename;
}

static NO_INIT vsf_eda_t *__lvgl_demo_evt_to_notify;
static void __lvgl_on_disp_drv_inited(lv_disp_drv_t *disp_drv)
{
    if (__lvgl_demo_evt_to_notify != NULL) {
        vsf_eda_post_evt(__lvgl_demo_evt_to_notify, VSF_EVT_USER);
    }
}

static vk_touchscreen_evt_t __lvgl_ts_evt;
static vk_keyboard_evt_t __lvgl_kb_evt;
static void __lvgl_on_evt(vk_input_notifier_t *notifier, vk_input_type_t type, vk_input_evt_t *evt)
{
    switch (type) {
    case VSF_INPUT_TYPE_TOUCHSCREEN: {
            vk_touchscreen_evt_t *ts_evt = (vk_touchscreen_evt_t *)evt;
            if (0 == vsf_input_touchscreen_get_id(ts_evt)) {
                __lvgl_ts_evt = *ts_evt;
            }
        }
        break;
    case VSF_INPUT_TYPE_KEYBOARD: {
            vk_keyboard_evt_t *kb_evt = (vk_keyboard_evt_t *)evt;
            __lvgl_kb_evt = *kb_evt;
        }
        break;
    }

    vsf_eda_trig_set_isr((vsf_trig_t *)notifier->param);
}

static bool __lvgl_touchscreen_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    vk_touchscreen_evt_t *ts_evt = &__lvgl_ts_evt;
    data->state = vsf_input_touchscreen_is_down(ts_evt) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;

    data->point.x = vsf_input_touchscreen_get_x(ts_evt);
    data->point.y = vsf_input_touchscreen_get_y(ts_evt);

    return false;
}

static bool __lvgl_keyboard_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static bool __last_state = LV_INDEV_STATE_REL;
    static bool __more_to_read = false;
    static uint16_t __last_key = 0;

    if (__more_to_read) {
        data->key = __last_key;
        data->state = __last_state;
        __more_to_read = false;
    } else {
        vk_keyboard_evt_t *kb_evt = &__lvgl_kb_evt;
        uint16_t key = vsf_input_keycode_to_lvgl_keycode(vsf_input_keyboard_get_keycode(kb_evt));
        bool state = vsf_input_keyboard_is_down(kb_evt) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;

        __more_to_read = (__last_state == LV_INDEV_STATE_PR) && (__last_key != key);
        data->key   = __more_to_read ? __last_key         : key;
        data->state = __more_to_read ? LV_INDEV_STATE_REL : state;

        __last_key = key;
        __last_state = state;
    }

    return __more_to_read;
}

int evue_main(int argc, char** argv)
{
    if(argc < 2) {
        printf("format: %s filename\n", argv[0]);
        return 0;
    }

    char *dirname = getdir(argv[1]);
    if (dirname == NULL) {
        return 0;
    }

    char *filename = getfilename(argv[1]);
    if (filename == NULL) {
        return 0;
    }

    chdir(dirname);

    /*Initialize the HAL (display, input devices, tick) for LVGL*/
    vk_disp_t *vsf_disp = vsf_board.display_dev;
    VSF_ASSERT(vsf_disp != NULL);
#if LV_COLOR_DEPTH == 32
    VSF_ASSERT(vsf_disp_get_pixel_format(vsf_disp) == VSF_DISP_COLOR_ARGB8888);
#elif LV_COLOR_DEPTH == 16
    VSF_ASSERT(vsf_disp_get_pixel_format(vsf_disp) == VSF_DISP_COLOR_RGB565);
#elif LV_COLOR_DEPTH == 8
    VSF_ASSERT(vsf_disp_get_pixel_format(vsf_disp) == VSF_DISP_COLOR_RGB332);
#else
    VSF_ASSERT(false);
#endif

    /*Initialize LVGL*/
    lv_init();
#if USE_LV_LOG
    lv_log_register_print(vsf_lvgl_printf);
#endif

    /*Create a display buffer*/
    lv_disp_draw_buf_t disp_buf;
    int bufsize =  vsf_disp_get_width(vsf_disp) * 64;
    lv_color_t *color_buf = malloc(2 * sizeof(lv_color_t) * bufsize);
    lv_color_t *buf1 = &color_buf[0];
    lv_color_t *buf2 = &color_buf[bufsize];
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, bufsize);

    /*Create a display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv); /*Basic initialization*/
    disp_drv.draw_buf = &disp_buf;
    disp_drv.hor_res = vsf_disp_get_width(vsf_disp);
    disp_drv.ver_res = vsf_disp_get_height(vsf_disp);
    disp_drv.antialiasing = 1;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);
    __lvgl_demo_evt_to_notify = vsf_eda_get_cur();
    vsf_lvgl_bind_disp(vsf_disp, disp->driver, __lvgl_on_disp_drv_inited);
    vsf_thread_wfe(VSF_EVT_USER);

    lv_group_t *g = lv_group_create();
    lv_group_set_default(g);

    /*Create a indev for mounse*/
    lv_indev_drv_t indev_drv_mouse;
    lv_indev_drv_init(&indev_drv_mouse);
    indev_drv_mouse.type = LV_INDEV_TYPE_POINTER;
    indev_drv_mouse.disp = disp;
    indev_drv_mouse.read_cb = __lvgl_touchscreen_read;
    lv_indev_drv_register(&indev_drv_mouse);

    /*Create a indev for keyboard*/
    lv_indev_drv_t indev_drv_keyboard;
    lv_indev_drv_init(&indev_drv_keyboard);
    indev_drv_keyboard.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv_keyboard.read_cb = __lvgl_keyboard_read;
    lv_indev_t* kb_indev = lv_indev_drv_register(&indev_drv_keyboard);
    lv_indev_set_group(kb_indev, g);

    vsf_trig_t trig;
    vsf_eda_trig_init(&trig, false, true);
    vk_input_notifier_t notifier = {
        .mask       = (1 << VSF_INPUT_TYPE_TOUCHSCREEN) | (1 << VSF_INPUT_TYPE_KEYBOARD),
        .on_evt     = __lvgl_on_evt,
        .param      = &trig,
    };
    vk_input_notifier_register(&notifier);

    if(argc > 1) {
        evm_main(filename);
    }
        
    while (1) {
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
        vsf_thread_trig_pend(&trig, vsf_systimer_ms_to_tick(20));
        lv_tick_inc(5);
        lv_timer_handler();
    }

    return 0;
}

int vsf_linux_create_fhs(void)
{
    // 0. devfs, busybox, etc
    vsf_linux_vfs_init();
    busybox_install();

    // 1. driver
    // 2. fs
    // 3. app
#if APP_USE_LINUX_MOUNT_DEMO == ENABLED
    extern int mount_main(int argc, char *argv[]);
    busybox_bind(VSF_LINUX_CFG_BIN_PATH "/mount", mount_main);
#endif
    busybox_bind(VSF_LINUX_CFG_BIN_PATH "/evue", evue_main);
    return 0;
}

int VSF_USER_ENTRY(void)
{
    vsf_board_init();
    vsf_start_trace();

    vsf_linux_init(&(vsf_linux_stdio_stream_t){
        .in     = (vsf_stream_t *)&VSF_DEBUG_STREAM_RX,
        .out    = (vsf_stream_t *)&VSF_DEBUG_STREAM_TX,
        .err    = (vsf_stream_t *)&VSF_DEBUG_STREAM_TX,
    });

    return 0;
}
