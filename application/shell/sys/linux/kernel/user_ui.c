#include <unistd.h>
#include <vsf_board.h>

#if VSF_USE_UI == ENABLED

#if VSF_TGUI_CFG_FONT_USE_FREETYPE == ENABLED
#   include <ft2build.h>
#   include FT_FREETYPE_H
#endif

#ifndef VSF_TGUI_CFG_EVTQ_MAX
#   define VSF_TGUI_CFG_EVTQ_MAX            32
#endif

declare_tgui_panel(tgui_root_panel_t)
def_tgui_panel(tgui_root_panel_t,
    tgui_contains(
        vsf_tgui_label_t     tInformation;
        vsf_tgui_button_t    tOK;
    )
)
end_def_tgui_panel(tgui_root_panel_t)

describ_tgui_panel(tgui_root_panel_t, root_panel_descriptor,
    tgui_region(0, 0, VSF_BOARD_DISP_WIDTH, VSF_BOARD_DISP_HEIGHT),
    tgui_text(tTitle, "  Info", false, VSF_TGUI_ALIGN_CENTER),
    tgui_padding(16, 16, 16, 16),

    tgui_label(tInformation, tgui_null_parent(tgui_root_panel_t), tInformation, tOK,
        tgui_text(tLabel, "This is a popup message. \nPlease click the OK button to close", false, VSF_TGUI_ALIGN_MID_LEFT),
        tgui_region(0, 56, 240 - 32, 80),
        tgui_sv_tile_show_corner(false),
        tgui_sv_font_color(VSF_TGUI_COLOR_BLACK),
    ),

    tgui_button(tOK, tgui_null_parent(tgui_root_panel_t), tInformation, tOK,
        tgui_location(170, 140),
        tgui_text(tLabel, "OK", true),
    ),
);

typedef struct {
    vsf_tgui_t instance;
    vsf_tgui_evt_t evtq_buffer[VSF_TGUI_CFG_EVTQ_MAX];
#if VSF_TGUI_CFG_REFRESH_SCHEME == VSF_TGUI_REFRESH_SCHEME_BREADTH_FIRST_TRAVERSAL
    uint16_t bfs_buffer[VSF_TGUI_CFG_EVTQ_MAX];
#endif
    vk_input_notifier_t input_notifier;
#if     VSF_TGUI_CFG_DISP_COLOR == VSF_TGUI_COLOR_ARGB_8888
    uint32_t pfb[VSF_BOARD_DISP_WIDTH * 10];
#elif   VSF_TGUI_CFG_DISP_COLOR == VSF_TGUI_COLOR_RGB_565 || VSF_TGUI_CFG_DISP_COLOR == VSF_TGUI_COLOR_BGR_565
    uint16_t pfb[VSF_BOARD_DISP_WIDTH * 10];
#else
#   error TODO: add support for the specifed display color format
#endif

    union {
        tgui_root_panel_t root_panel;
    };
} vsf_tgui_demo_t;
static VSF_CAL_NO_INIT vsf_tgui_demo_t __tgui_demo;

#if VSF_TGUI_CFG_FONT_USE_FREETYPE == ENABLED

#include "./__font.h"

static FT_FILE __ft_root_dir[] = {
    {
        .attr           = VSF_FILE_ATTR_READ,
        .name           = "DejaVuSerif.ttf",
        .size           = sizeof(__font),
        .f.buff         = (uint8_t *)__font,
    },
};

FT_FILE ft_root = {
    .attr               = VSF_FILE_ATTR_DIRECTORY,
    .name               = "/",
    .d.child            = (vk_memfs_file_t *)__ft_root_dir,
    .d.child_num        = dimof(__ft_root_dir),
    .d.child_size       = sizeof(FT_FILE),
};
#endif

int ui_main(int argc, char **argv)
{
    static const vsf_tgui_cfg_t __cfg = {
        .evt_queue = {
            .obj_ptr = __tgui_demo.evtq_buffer,
            .s32_size = sizeof(__tgui_demo.evtq_buffer),
        },
#if VSF_TGUI_CFG_REFRESH_SCHEME == VSF_TGUI_REFRESH_SCHEME_BREADTH_FIRST_TRAVERSAL
        .bfs_queue = {
            .obj_ptr = __tgui_demo.bfs_buffer,
            .s32_size = sizeof(__tgui_demo.bfs_buffer),
        },
#endif
    };

    if (vk_tgui_init(&__tgui_demo.instance, &__cfg) != VSF_ERR_NONE) {
        printf("fail to initialize tgui\r\n");
        return -1;
    }

    tgui_initalize_top_container(root_panel_descriptor, &__tgui_demo.root_panel);
    vk_tgui_set_root_container(&__tgui_demo.instance, (vsf_tgui_root_container_t *)&__tgui_demo.root_panel, true);

    vsf_tgui_fonts_init((vsf_tgui_font_t *)vsf_tgui_font_get(0), vsf_tgui_font_number(), "/");
    vsf_tgui_sv_bind_disp(&__tgui_demo.instance, vsf_board.display_dev, &__tgui_demo.pfb, dimof(__tgui_demo.pfb));

    __tgui_demo.input_notifier.mask =
                    (1 << VSF_INPUT_TYPE_TOUCHSCREEN)
                |   (1 << VSF_INPUT_TYPE_KEYBOARD)
#if VSF_TGUI_CFG_SUPPORT_MOUSE_LIKE_EVENTS == ENABLED
                |   (1 << VSF_INPUT_TYPE_MOUSE)
#endif
        ;
    vsf_tgui_input_init(&__tgui_demo.instance, &__tgui_demo.input_notifier);
    return 0;
}
#endif
