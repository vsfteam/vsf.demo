#define __VSF_LINUX_FS_CLASS_INHERIT__
#include <unistd.h>
#include <sys/eventfd.h>

#include <vsf_board.h>

#if VSF_USE_UI == ENABLED

// executor

typedef struct vsf_ui_executor_ctx_t vsf_ui_executor_ctx_t;
struct vsf_ui_executor_ctx_t {
    vsf_dlist_node_t __node;

    char *cmd;
    char **args;
    void (*on_select)(vsf_ui_executor_ctx_t *ctx, fd_set *rfds, fd_set *wfds);

    int __pid;
    int __stdout_pipe[2], __stdin_pipe[2], __stderr_pipe[2];
};

static vsf_dlist_t __vsf_ui_executor_queue_pending;
static vsf_linux_fd_priv_t *__vsf_ui_eventfd_priv = NULL;

extern void eventfd_inc_isr(vsf_linux_fd_priv_t *eventfd_priv);

void vsf_tgui_execute_in_shell(vsf_ui_executor_ctx_t *ctx)
{
    ctx->__pid = -1;
    ctx->__stdout_pipe[0] = -1;
    ctx->__stdout_pipe[1] = -1;
    ctx->__stdin_pipe[0] = -1;
    ctx->__stdin_pipe[1] = -1;
    ctx->__stderr_pipe[0] = -1;
    ctx->__stderr_pipe[1] = -1;

    vsf_protect_t orig = vsf_protect_int();
        vsf_dlist_add_to_tail(vsf_ui_executor_ctx_t, __node, &__vsf_ui_executor_queue_pending, ctx);
    vsf_unprotect_int(orig);

    VSF_ASSERT(__vsf_ui_eventfd_priv != NULL);
    eventfd_inc_isr(__vsf_ui_eventfd_priv);
}

// UI instance

#ifndef VSF_TGUI_CFG_EVTQ_MAX
#   define VSF_TGUI_CFG_EVTQ_MAX            32
#endif

typedef struct {
    vsf_tgui_t instance;
    vsf_tgui_evt_t evtq_buffer[VSF_TGUI_CFG_EVTQ_MAX];
#if VSF_TGUI_CFG_REFRESH_SCHEME == VSF_TGUI_REFRESH_SCHEME_BREADTH_FIRST_TRAVERSAL
    uint16_t bfs_buffer[VSF_TGUI_CFG_EVTQ_MAX];
#endif
    vk_input_notifier_t input_notifier;
#if     VSF_TGUI_CFG_DISP_COLOR == VSF_TGUI_COLOR_ARGB_8888
    uint32_t pfb[VSF_BOARD_DISP_WIDTH * 30];
#elif   VSF_TGUI_CFG_DISP_COLOR == VSF_TGUI_COLOR_RGB_565 || VSF_TGUI_CFG_DISP_COLOR == VSF_TGUI_COLOR_BGR_565
    uint16_t pfb[VSF_BOARD_DISP_WIDTH * 30];
#else
#   error TODO: add support for the specifed display color format
#endif
} vsf_tgui_demo_t;
static VSF_CAL_NO_INIT vsf_tgui_demo_t __tgui_demo;

// UI frame

static uint32_t __vsf_tgui_panel_buffer[4096 / 4];

static vsf_tgui_frame_t *__vsf_tgui_frame_root = NULL;

vsf_tgui_frame_t * vsf_tgui_frame_new(int frame_size, vsf_tgui_frame_init_t fn_init, char *tile)
{
    VSF_ASSERT(frame_size >= sizeof(vsf_tgui_frame_t));
    VSF_ASSERT(fn_init != NULL);

    vsf_tgui_frame_t *frame = vsf_heap_malloc(frame_size);
    if (NULL == frame) {
        VSF_ASSERT(false);
        return NULL;
    }

    memset(frame, 0, frame_size);

    frame->__fn_init = fn_init;
    frame->__tile = tile;
    frame->__next = __vsf_tgui_frame_root;
    __vsf_tgui_frame_root = frame;

    return frame;
}

void vsf_tgui_frame_init(vsf_tgui_frame_t *frame)
{
    frame->__fn_init(&__tgui_demo.instance, frame, __vsf_tgui_panel_buffer, sizeof(__vsf_tgui_panel_buffer));
}

void vsf_tgui_frame_exit(void)
{
    vsf_tgui_frame_t *frame = __vsf_tgui_frame_root;
    VSF_ASSERT(frame != NULL);

    vk_tgui_close_root_container(&__tgui_demo.instance);

    __vsf_tgui_frame_root = frame->__next;
    vsf_heap_free(frame);

    if (__vsf_tgui_frame_root != NULL) {
        frame = __vsf_tgui_frame_root;
        frame->__fn_init(&__tgui_demo.instance, frame, __vsf_tgui_panel_buffer, sizeof(__vsf_tgui_panel_buffer));
    }
}

vsf_tgui_panel_t * vsf_tgui_get_cur_panel(void)
{
    return (vsf_tgui_panel_t *)__vsf_tgui_panel_buffer;
}

// UI Image(tile) resources

#include "./images/demo_images.h"
#include "./images/demo_images_data.h"

unsigned char * vsf_tgui_tile_get_pixelmap(const vsf_tgui_tile_t *tile_ptr)
{
    VSF_TGUI_ASSERT(tile_ptr != NULL);

    uint_fast8_t tile_type = tile_ptr->_.tCore.Attribute.u2RootTileType;
    switch (tile_type) {
    case 0:  // buf_offset tile
        VSF_ASSERT(__vsf_tgui_frame_root != NULL);
        return (unsigned char *)&__vsf_tgui_frame_root->__tile[tile_ptr->tBufRoot.ptBitmap];
    case 2:  // buf_addr tile
        return (unsigned char *)tile_ptr->tBufRoot.ptBitmap;
    default:
        VSF_TGUI_ASSERT(0);
        return NULL;
    }
}

// UI description

#ifndef VSF_TGUI_CFG_BORDER_SIZE
#   define VSF_TGUI_CFG_BORDER_SIZE         16
#endif

typedef struct vsf_tgui_frame_root_t {
    implement(vsf_tgui_frame_t)
    struct {
        char *name;
        char *cmd;
        char **args;
        vsf_tgui_tile_t *icon;
    } app[9];
} vsf_tgui_frame_root_t;

def_tgui_panel(tgui_root_panel_t,
    tgui_contains(
        use_tgui_panel(tAppPanel,
            tgui_contains(
                vsf_tgui_button_t tAppBtn[9];
            )
        )
    )
)
vsf_ui_executor_ctx_t executor;
vsf_tgui_frame_root_t *frame;
end_def_tgui_panel(tgui_root_panel_t)

static void __vsf_tgui_frame_root_on_select(vsf_ui_executor_ctx_t *ctx, fd_set *rfds, fd_set *wfds)
{
}

static fsm_rt_t __vsf_tgui_root_frame_on_appbtn_click(vsf_tgui_control_t *control_ptr, vsf_msgt_msg_t *msg)
{
    tgui_root_panel_t *root_panel = (tgui_root_panel_t *)vsf_tgui_get_cur_panel();
    int btn_idx = (vsf_tgui_button_t *)control_ptr - root_panel->tAppPanel.tAppBtn;
    vsf_tgui_frame_root_t *root_frame = root_panel->frame;

    root_panel->executor.cmd = root_frame->app[btn_idx].cmd;
    root_panel->executor.args = root_frame->app[btn_idx].args;
    root_panel->executor.on_select = __vsf_tgui_frame_root_on_select;
    vsf_tgui_execute_in_shell(&root_panel->executor);
    return (fsm_rt_t)VSF_TGUI_MSG_RT_DONE;
}
static const vsf_tgui_user_evt_handler __vsf_tgui_root_frame_appbtn_handler[] = {
    tgui_msg_handler(VSF_TGUI_EVT_POINTER_CLICK, __vsf_tgui_root_frame_on_appbtn_click)
};

describ_tgui_panel(tgui_root_panel_t, root_panel_descriptor,
    tgui_region(0, 0, VSF_BOARD_DISP_WIDTH, VSF_BOARD_DISP_HEIGHT),
    tgui_text(tTitle, "Application", false),
    tgui_padding(VSF_TGUI_CFG_BORDER_SIZE, VSF_TGUI_CFG_BORDER_SIZE, VSF_TGUI_CFG_BORDER_SIZE, VSF_TGUI_CFG_BORDER_SIZE),

#define __app_button(__idx, __pre_idx, __next_idx, ...)                         \
    tgui_button(tAppBtn[__idx], &(tgui_null_parent(tgui_root_panel_t)->tAppPanel), tAppBtn[__pre_idx], tAppBtn[__next_idx],\
        tgui_size(64, 76),                                                      \
        tgui_margin(2, 2, 2, 2),                                                \
        tgui_sv_tile_show_corner(false),                                        \
        tgui_sv_background_color(VSF_TGUI_COLOR_TRANSPARENT),                   \
        tgui_sv_font(VSF_TGUI_FONT_DEJAVUSERIF_S12),                            \
        tgui_background((vsf_tgui_tile_t*)&res_empty_RGBA, VSF_TGUI_ALIGN_TOP), \
        tgui_text(tLabel, "Slot" #__idx "\nEmpty", false, VSF_TGUI_ALIGN_BOTTOM),\
        .tMSGMap = {                                                            \
            .ptItems = __vsf_tgui_root_frame_appbtn_handler,                    \
            .chCount = dimof(__vsf_tgui_root_frame_appbtn_handler),             \
        },                                                                      \
        __VA_ARGS__                                                             \
    )

    tgui_panel(tAppPanel, tgui_null_parent(tgui_root_panel_t), tAppPanel, tAppPanel,
        tgui_region(0, VSF_TGUI_CFG_BORDER_SIZE * 3,
            VSF_BOARD_DISP_WIDTH - 2 * VSF_TGUI_CFG_BORDER_SIZE, 0),
        tgui_sv_tile_show_corner(false),
        tgui_container_type(VSF_TGUI_CONTAINER_TYPE_STREAM_HORIZONTAL),
        tgui_sv_background_color(VSF_TGUI_COLOR_TRANSPARENT),

        tgui_contains(
            __app_button(0, 0, 1),
            __app_button(1, 0, 2),
            __app_button(2, 1, 3),
            __app_button(3, 2, 4),
            __app_button(4, 3, 5),
            __app_button(5, 4, 6),
            __app_button(6, 5, 7),
            __app_button(7, 6, 8),
            __app_button(8, 7, 8),
        )
    ),
);

static void __vsf_tgui_frame_root_init(vsf_tgui_t *gui, vsf_tgui_frame_t *frame, void *panel, int panel_size)
{
    vsf_tgui_frame_root_t *root_frame = (vsf_tgui_frame_root_t *)frame;
    tgui_root_panel_t *root_panel = (tgui_root_panel_t *)panel;
    VSF_ASSERT(panel_size >= sizeof(tgui_root_panel_t));

    tgui_initalize_top_container(root_panel_descriptor, root_panel);
    root_panel->frame = root_frame;
    for (int i = 0; i < dimof(root_frame->app); i++) {
        if (root_frame->app[i].name != NULL) {
            vsf_tgui_text_set(&root_panel->tAppPanel.tAppBtn[i].tLabel, &(const vsf_tgui_string_t){
                .pstrText = root_frame->app[i].name,
#if VSF_TGUI_CFG_SAFE_STRING_MODE == ENABLED
                .s16_size = strlen(root_frame->app[i].name),
#endif
            });
            root_panel->tAppPanel.tAppBtn[i].tBackground.ptTile = root_frame->app[i].icon;
        }
    }
    vk_tgui_set_root_container(gui, (vsf_tgui_root_container_t *)panel, true);
}

// UI corner

static const vsf_tgui_tile_t __controls_container_corner_tiles[CORNOR_TILE_NUM] = {
    [CORNOR_TILE_IN_TOP_LEFT] = {
        .tChild = {
            .tSize = {.iWidth = 12, .iHeight = 12, },
            .parent_ptr = (vsf_tgui_tile_core_t*)&corner12_L,
            .tLocation = {.iX = 0, .iY = 0},
        },
    },
    [CORNOR_TILE_IN_TOP_RIGHT] = {
        .tChild = {
            .tSize = {.iWidth = 12, .iHeight = 12, },
            .parent_ptr = (vsf_tgui_tile_core_t*)&corner12_L,
            .tLocation = {.iX = 12, .iY = 0},
        },
    },
    [CORNOR_TILE_IN_BOTTOM_LEFT] = {
        .tChild = {
            .tSize = {.iWidth = 12, .iHeight = 12, },
            .parent_ptr = (vsf_tgui_tile_core_t*)&corner12_L,
            .tLocation = {.iX = 0, .iY = 12},
        },
    },
    [CORNOR_TILE_IN_BOTTOM_RIGHT] = {
        .tChild = {
            .tSize = {.iWidth = 12, .iHeight = 12, },
            .parent_ptr = (vsf_tgui_tile_core_t*)&corner12_L,
            .tLocation = {.iX = 12, .iY = 12},
        },
    },
};

static const vsf_tgui_tile_t __controls_label_corner_tiles[CORNOR_TILE_NUM] = {
    [CORNOR_TILE_IN_TOP_LEFT] = {
        .tChild = {
            .tSize = {.iWidth = 16, .iHeight = 16, },
            .parent_ptr = (vsf_tgui_tile_core_t*)&corner16_L,
            .tLocation = {.iX = 0, .iY = 0},
        },
    },
    [CORNOR_TILE_IN_TOP_RIGHT] = {
        .tChild = {
            .tSize = {.iWidth = 16, .iHeight = 16, },
            .parent_ptr = (vsf_tgui_tile_core_t*)&corner16_L,
            .tLocation = {.iX = 16, .iY = 0},
        },
    },
    [CORNOR_TILE_IN_BOTTOM_LEFT] = {
        .tChild = {
            .tSize = {.iWidth = 16, .iHeight = 16, },
            .parent_ptr = (vsf_tgui_tile_core_t*)&corner16_L,
            .tLocation = {.iX = 0, .iY = 16},
        },
    },
    [CORNOR_TILE_IN_BOTTOM_RIGHT] = {
        .tChild = {
            .tSize = {.iWidth = 16, .iHeight = 16, },
            .parent_ptr = (vsf_tgui_tile_core_t*)&corner16_L,
            .tLocation = {.iX = 16, .iY = 16},
        },
    },
};

const vsf_tgui_tile_t * vsf_tgui_control_v_get_corner_tile(vsf_tgui_control_t *control_ptr, vsf_tgui_sv_cornor_tile_mode_t mode)
{
    if (control_ptr->id == VSF_TGUI_COMPONENT_ID_BUTTON || control_ptr->id == VSF_TGUI_COMPONENT_ID_LABEL) {
        if (mode < dimof(__controls_label_corner_tiles)) {
            return &__controls_label_corner_tiles[mode];
        }
    } else /*if (control_ptr->id == VSF_TGUI_COMPONENT_ID_CONTAINER || control_ptr->id == VSF_TGUI_COMPONENT_ID_PANEL)*/ {
        if (mode < dimof(__controls_container_corner_tiles)) {
            return &__controls_container_corner_tiles[mode];
        }
    }

    return NULL;
}

// UI font

#if VSF_TGUI_CFG_FONT_USE_FREETYPE == ENABLED

#if VSF_TGUI_CFG_FONT_USE_FREETYPE == ENABLED
#   include <ft2build.h>
#   include FT_FREETYPE_H
#endif

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

// UI main

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

    // start root panel frame
    vsf_tgui_frame_root_t *root_frame = (vsf_tgui_frame_root_t *)vsf_tgui_frame_new(
        sizeof(vsf_tgui_frame_root_t), __vsf_tgui_frame_root_init, (char *)__tiles_data);
    // TODO: load applications
    root_frame->app[0].name = "Local\nApps";
    root_frame->app[0].cmd = "vpm";
    static const char *__vpm_list_local_args[] = {
        "vpm", "list-local", NULL
    };
    root_frame->app[0].args = (char **)__vpm_list_local_args;
    root_frame->app[0].icon = (vsf_tgui_tile_t *)&res_local_RGBA;
    root_frame->app[1].name = "Remote\nApps";
    root_frame->app[1].cmd = "vpm";
    static const char *__vpm_list_remmote_args[] = {
        "vpm", "list-remote", NULL
    };
    root_frame->app[1].args = (char **)__vpm_list_remmote_args;
    root_frame->app[1].icon = (vsf_tgui_tile_t *)&res_cloud_RGBA;
    vsf_tgui_frame_init(&root_frame->use_as__vsf_tgui_frame_t);

    int executor_notifier = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    VSF_ASSERT(executor_notifier >= 0);
    eventfd_t eventfd_value;
    vsf_dlist_t executor_queue;
    vsf_ui_executor_ctx_t *executor;
    fd_set rfds, wfds;
    pid_t pid;
    int fd_num;

    __vsf_ui_eventfd_priv = vsf_linux_fd_get(executor_notifier)->priv;
    vsf_slist_init(&executor_queue);
    while (true) {
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_SET(executor_notifier, &rfds);
        fd_num = executor_notifier;
        __vsf_slist_foreach_unsafe(vsf_ui_executor_ctx_t, __node, &executor_queue) {
            FD_SET(_->__stdin_pipe[1], &wfds);
            if (_->__stdin_pipe[1] > fd_num) {
                fd_num = _->__stdin_pipe[1];
            }
            FD_SET(_->__stdout_pipe[0], &rfds);
            if (_->__stdout_pipe[0] > fd_num) {
                fd_num = _->__stdout_pipe[0];
            }
            FD_SET(_->__stderr_pipe[0], &rfds);
            if (_->__stderr_pipe[0] > fd_num) {
                fd_num = _->__stderr_pipe[0];
            }
        }
        fd_num = select(fd_num + 1, &rfds, &wfds, NULL, NULL);
        if (fd_num <= 0) { continue; }

        if (FD_ISSET(executor_notifier, &rfds)) {
            eventfd_read(executor_notifier, &eventfd_value);
            vsf_protect_t orig = vsf_protect_int();
                vsf_dlist_remove_head(vsf_ui_executor_ctx_t, __node, &__vsf_ui_executor_queue_pending, executor);
            vsf_unprotect_int(orig);

            VSF_ASSERT(executor!= NULL);
            vsf_dlist_add_to_tail(vsf_ui_executor_ctx_t, __node, &executor_queue, executor);

            if (    (pipe(executor->__stdout_pipe) != 0)
                ||  (pipe(executor->__stdin_pipe) != 0)
                ||  (pipe(executor->__stderr_pipe) != 0)) {
                printf("fail to create stdio pipes\r\n");
                VSF_ASSERT(false);
                continue;
            }

            pid = vfork();
            if (pid < 0) {
                printf("fail to fork\r\n");
                continue;
            }else if (pid > 0) {
                executor->__pid = pid;
            } else if (pid == 0) {
                dup2(executor->__stdin_pipe[0], STDIN_FILENO);
                dup2(executor->__stdout_pipe[1], STDOUT_FILENO);
                dup2(executor->__stderr_pipe[1], STDERR_FILENO);

                execvp(executor->cmd, executor->args);
                VSF_ASSERT(false);
            }
        }

        __vsf_slist_foreach_unsafe(vsf_ui_executor_ctx_t, __node, &executor_queue) {
            if (    FD_ISSET(_->__stdin_pipe[1], &wfds)
                ||  FD_ISSET(_->__stdout_pipe[0], &rfds)
                ||  FD_ISSET(_->__stderr_pipe[0], &rfds)) {

                if (_->on_select != NULL) {
                    _->on_select(_, &rfds, &wfds);
                }
            }
        }
    }
    return 0;
}
#endif
