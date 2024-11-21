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
    int (*fn_select)(vsf_ui_executor_ctx_t *ctx, fd_set *rfds, fd_set *wfds);
    // if NULL == rfds, it means on_exit
    void (*on_select)(vsf_ui_executor_ctx_t *ctx, fd_set *rfds, fd_set *wfds);
    void *param;

    bool __exited;
    int __pid;
    int __stdout_pipe[2], __stdin_pipe[2], __stderr_pipe[2];
};

static VSF_CAL_NO_INIT vsf_dlist_t __vsf_ui_executor_queue_pending;
static VSF_CAL_NO_INIT vsf_dlist_t __vsf_ui_executor_queue;
static VSF_CAL_NO_INIT vsf_linux_fd_priv_t *__vsf_ui_new_eventfd_priv;
static VSF_CAL_NO_INIT vsf_linux_fd_priv_t *__vsf_ui_exit_eventfd_priv;

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

    VSF_ASSERT(__vsf_ui_new_eventfd_priv != NULL);
    eventfd_inc_isr(__vsf_ui_new_eventfd_priv);
}

void __vsf_ui_executor_atexit(void)
{
    pid_t pid = getpid();
    __vsf_dlist_foreach_unsafe(vsf_ui_executor_ctx_t, __node, &__vsf_ui_executor_queue) {
        if (_->__pid == pid) {
            _->__exited = true;
            eventfd_inc_isr(__vsf_ui_exit_eventfd_priv);
        }
    }
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

static vsf_tgui_frame_t *__vsf_tgui_root_frame = NULL;

vsf_tgui_frame_t * vsf_tgui_frame_new(int frame_size, vsf_tgui_frame_init_t fn_init)
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
    frame->__next = __vsf_tgui_root_frame;
    __vsf_tgui_root_frame = frame;

    return frame;
}

void vsf_tgui_frame_init(vsf_tgui_frame_t *frame)
{
    frame->__fn_init(&__tgui_demo.instance, frame, __vsf_tgui_panel_buffer, sizeof(__vsf_tgui_panel_buffer), true);
}

void vsf_tgui_frame_exit(void)
{
    vsf_tgui_frame_t *frame = __vsf_tgui_root_frame;
    VSF_ASSERT(frame != NULL);

    __vsf_tgui_root_frame = frame->__next;
    vsf_heap_free(frame);

    if (__vsf_tgui_root_frame != NULL) {
        frame = __vsf_tgui_root_frame;
        frame->__fn_init(&__tgui_demo.instance, frame, __vsf_tgui_panel_buffer, sizeof(__vsf_tgui_panel_buffer), false);
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
        VSF_ASSERT(__vsf_tgui_root_frame != NULL);
        return (unsigned char *)&__vsf_tgui_root_frame->__tile[tile_ptr->tBufRoot.ptBitmap];
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

static vsf_ui_executor_ctx_t __vsf_tgui_app_executor;

// popup information frame

declare_tgui_panel(vsf_tgui_popup_panel_t)
typedef struct vsf_tgui_popup_frame_t {
    implement(vsf_tgui_frame_t)
    const char *pstrTitle;
    const char *pstrInformation;
    vsf_tgui_popup_panel_t *panel;
    void *param;
} vsf_tgui_popup_frame_t;

def_tgui_panel(vsf_tgui_popup_panel_t,
    tgui_contains(
        vsf_tgui_label_t tInformation;
        vsf_tgui_button_t tOK;
    )
)
vsf_tgui_popup_frame_t *frame;
end_def_tgui_panel(vsf_tgui_popup_panel_t)

static fsm_rt_t __vsf_tgui_popup_frame_on_depose(vsf_tgui_control_t *control_ptr, vsf_msgt_msg_t *msg)
{
    vsf_tgui_frame_exit();
    return (fsm_rt_t)VSF_TGUI_MSG_RT_DONE;
}

static fsm_rt_t __vsf_tgui_popup_frame_on_ok_click(vsf_tgui_control_t *control_ptr, vsf_msgt_msg_t *msg)
{
    vsf_tgui_popup_panel_t *popup_panel = (vsf_tgui_popup_panel_t *)vk_tgui_control_get_top(control_ptr);

    vk_tgui_close_root_container(popup_panel->gui_ptr);
    return (fsm_rt_t)VSF_TGUI_MSG_RT_DONE;
}

describ_tgui_panel(vsf_tgui_popup_panel_t, popup_panel_descriptor,
    tgui_region(0, 0, VSF_BOARD_DISP_WIDTH, VSF_BOARD_DISP_HEIGHT),
    tgui_padding(VSF_TGUI_CFG_BORDER_SIZE, VSF_TGUI_CFG_BORDER_SIZE, VSF_TGUI_CFG_BORDER_SIZE, VSF_TGUI_CFG_BORDER_SIZE),

    tgui_msgmap(
        tgui_msg_handler(VSF_TGUI_EVT_ON_DEPOSE, __vsf_tgui_popup_frame_on_depose),
    ),

    tgui_label(tInformation, tgui_null_parent(vsf_tgui_popup_panel_t), tInformation, tOK,
        tgui_region(0, 3 * VSF_TGUI_CFG_BORDER_SIZE,
            VSF_BOARD_DISP_WIDTH - 2 * VSF_TGUI_CFG_BORDER_SIZE,
            VSF_BOARD_DISP_HEIGHT - 9 * VSF_TGUI_CFG_BORDER_SIZE),
        tgui_sv_tile_show_corner(false),
        tgui_sv_font_color(VSF_TGUI_COLOR_BLACK),
    ),

    tgui_button(tOK, tgui_null_parent(vsf_tgui_popup_panel_t), tInformation, tOK,
        .bIsEnabled = false,
        tgui_region((VSF_BOARD_DISP_WIDTH - 2 * VSF_TGUI_CFG_BORDER_SIZE) / 4,
            VSF_BOARD_DISP_HEIGHT - 5 * VSF_TGUI_CFG_BORDER_SIZE,
            (VSF_BOARD_DISP_WIDTH - 2 * VSF_TGUI_CFG_BORDER_SIZE) / 2,
            2 * VSF_TGUI_CFG_BORDER_SIZE
        ),
        tgui_text(tLabel, "OK", true),
        tgui_msgmap(
            tgui_msg_handler(VSF_TGUI_EVT_POINTER_CLICK, __vsf_tgui_popup_frame_on_ok_click),
        ),
    ),
);

static void __vsf_tgui_popup_frame_init(vsf_tgui_t *gui, vsf_tgui_frame_t *frame, void *panel, int panel_size, int is_first)
{
    vsf_tgui_popup_frame_t *popup_frame = (vsf_tgui_popup_frame_t *)frame;
    vsf_tgui_popup_panel_t *popup_panel = (vsf_tgui_popup_panel_t *)panel;
    VSF_ASSERT(panel_size >= sizeof(vsf_tgui_popup_panel_t));

    tgui_initalize_top_container(popup_panel_descriptor, popup_panel);
    popup_frame->panel = popup_panel;
    popup_frame->__tile = (char *)__tiles_data;
    popup_panel->frame = popup_frame;
    vsf_tgui_text_set(&popup_panel->tTitle, &(const vsf_tgui_string_t){
        .pstrText = popup_frame->pstrTitle,
#if VSF_TGUI_CFG_SAFE_STRING_MODE == ENABLED
        .s16_size = strlen(popup_frame->pstrTitle),
#endif
    });
    vsf_tgui_text_set(&popup_panel->tInformation.tLabel, &(const vsf_tgui_string_t){
        .pstrText = popup_frame->pstrInformation,
#if VSF_TGUI_CFG_SAFE_STRING_MODE == ENABLED
        .s16_size = strlen(popup_frame->pstrInformation),
#endif
    });
    vk_tgui_set_root_container(gui, (vsf_tgui_root_container_t *)popup_panel, true);
}

vsf_tgui_popup_frame_t * vsf_tgui_popup(const char *pstrTitle, const char *pstrInformation)
{
    vsf_tgui_popup_frame_t *frame = (vsf_tgui_popup_frame_t *)vsf_tgui_frame_new(
        sizeof(vsf_tgui_popup_frame_t), __vsf_tgui_popup_frame_init);
    frame->pstrTitle = pstrTitle;
    frame->pstrInformation = pstrInformation;
    vsf_tgui_frame_init(&frame->use_as__vsf_tgui_frame_t);
    return frame;
}

// local_app & remote_app frame

declare_tgui_panel(tgui_applist_panel_t)
typedef struct vsf_tgui_applist_frame_t {
    implement(vsf_tgui_frame_t)
    tgui_applist_panel_t *panel;

    vsf_ui_executor_ctx_t executor;
    char *args[4];
    char *info;
    int info_size;

    char appname_to_execute[64];
    char appname[64];
    bool is_remote;
    bool is_operation;
} vsf_tgui_applist_frame_t;

def_tgui_panel(tgui_applist_panel_t,
    tgui_contains(
        use_tgui_list(tAppList,
            tgui_contains(
                vsf_tgui_button_t tExitBtn;
            )
        )
    )
)
vsf_tgui_applist_frame_t *frame;
end_def_tgui_panel(tgui_applist_panel_t)

static int __vsf_tgui_applist_frame_opbtn_executor_select(vsf_ui_executor_ctx_t *ctx, fd_set *rfds, fd_set *wfds)
{
    FD_SET(ctx->__stdout_pipe[0], rfds);
    return ctx->__stdout_pipe[0];
}

static void __vsf_tgui_applist_frame_opbtn_executor_on_select(vsf_ui_executor_ctx_t *ctx, fd_set *rfds, fd_set *wfds)
{
    vsf_tgui_popup_frame_t *popup_frame = (vsf_tgui_popup_frame_t *)ctx->param;
    vsf_tgui_applist_frame_t *applist_frame = (vsf_tgui_applist_frame_t *)popup_frame->param;
    vsf_tgui_popup_panel_t *popup_panel = popup_frame->panel;

    if (NULL == rfds) {
        applist_frame->is_operation = false;
        popup_panel->tOK.bIsEnabled = true;
        vk_tgui_refresh_ex(popup_panel->gui_ptr, &popup_panel->use_as__vsf_tgui_control_t, NULL);
    } else if (FD_ISSET(ctx->__stdout_pipe[0], rfds)) {
        bool is_to_refresh = false;
        vsf_linux_fd_t *sfd = vsf_linux_fd_get(ctx->__stdout_pipe[0]);
        vsf_linux_stream_priv_t *stream_priv = (vsf_linux_stream_priv_t *)sfd->priv;
        vsf_stream_t *stream = stream_priv->stream_rx;
        uint_fast32_t rbuff_size;

        while (true) {
            rbuff_size = vsf_stream_get_data_size(stream);
            if (rbuff_size <= 0) {
                break;
            }

            applist_frame->info = vsf_heap_realloc(applist_frame->info, applist_frame->info_size + rbuff_size + 1);
            read(ctx->__stdout_pipe[0], &applist_frame->info[applist_frame->info_size], rbuff_size);
            applist_frame->info_size += rbuff_size;
            applist_frame->info[applist_frame->info_size] = '\0';

            vsf_tgui_label_set_text_static(&popup_panel->tInformation, applist_frame->info);
            is_to_refresh = true;
        }

        if (is_to_refresh) {
            vk_tgui_refresh_ex(popup_panel->gui_ptr, &popup_panel->use_as__vsf_tgui_control_t, NULL);
        }
    }
}

static fsm_rt_t __vsf_tgui_applist_frame_on_depose(vsf_tgui_control_t *control_ptr, vsf_msgt_msg_t *msg)
{
    tgui_applist_panel_t *applist_panel = (tgui_applist_panel_t *)vk_tgui_control_get_top(control_ptr);
    vsf_tgui_applist_frame_t *applist_frame = applist_panel->frame;
    vsf_tgui_list_t *list = (vsf_tgui_list_t *)control_ptr;
    vsf_tgui_control_t *control = vsf_tgui_list_get_child(list), *control_next;

    // the last control which Offset.next is 0 is the static tExitBtn
    while (control->Offset.next != 0) {
        control_next = (vsf_tgui_control_t *)((intptr_t)control + control->Offset.next);
        if (control->id == VSF_TGUI_COMPONENT_ID_LABEL) {
            vsf_tgui_label_t *label = (vsf_tgui_label_t *)control;
            if (label->tLabel.tString.pstrText != NULL) {
                vsf_heap_free((void *)label->tLabel.tString.pstrText);
            }
        }
        vsf_tgui_control_remove(&list->use_as__vsf_tgui_container_t, control);
        vsf_tgui_control_destroy(control);
        control = control_next;
    }

    if (applist_frame->is_operation) {
        vsf_tgui_popup_frame_t *popup_frame;
        if (applist_frame->is_remote) {
            popup_frame = vsf_tgui_popup("Installing", "");
        } else {
            popup_frame = vsf_tgui_popup("uninstalling", "");
        }
        popup_frame->param = applist_frame;

        applist_frame->executor.param = popup_frame;
        applist_frame->args[0] = "vpm";
        if (applist_frame->is_remote) {
            applist_frame->args[1] = "install";
        } else {
            applist_frame->args[1] = "uninstall";
        }
        applist_frame->args[2] = applist_frame->appname_to_execute;
        applist_frame->executor.cmd = applist_frame->args[0];
        applist_frame->executor.args = applist_frame->args;
        applist_frame->executor.on_select = __vsf_tgui_applist_frame_opbtn_executor_on_select;
        applist_frame->executor.fn_select = __vsf_tgui_applist_frame_opbtn_executor_select;
        vsf_tgui_execute_in_shell(&applist_frame->executor);
    } else {
        vsf_tgui_frame_exit();
    }
    return (fsm_rt_t)VSF_TGUI_MSG_RT_DONE;
}

static fsm_rt_t __vsf_tgui_applist_frame_on_exit_click(vsf_tgui_control_t *control_ptr, vsf_msgt_msg_t *msg)
{
    const vsf_tgui_root_container_t *ptTopContainer = vk_tgui_control_get_top(control_ptr);
    vsf_tgui_t *gui_ptr = ptTopContainer->gui_ptr;

    vk_tgui_close_root_container(gui_ptr);
    return (fsm_rt_t)VSF_TGUI_MSG_RT_DONE;
}

static fsm_rt_t __vsf_tgui_applist_frame_on_opbtn_click(vsf_tgui_control_t *control_ptr, vsf_msgt_msg_t *msg)
{
    tgui_applist_panel_t *applist_panel = (tgui_applist_panel_t *)vk_tgui_control_get_top(control_ptr);
    vsf_tgui_applist_frame_t *applist_frame = applist_panel->frame;
    vsf_msgt_container_t *container = control_ptr->parent_ptr;
    vsf_tgui_label_t *label = (vsf_tgui_label_t *)container->node_ptr;

    strcpy(applist_frame->appname_to_execute, label->tLabel.tString.pstrText);
    char *end = strchr(applist_frame->appname_to_execute, ' ');
    if (end != NULL) {
        *end = '\0';
    }
    applist_frame->is_operation = true;
    vk_tgui_close_root_container(applist_panel->gui_ptr);
    return (fsm_rt_t)VSF_TGUI_MSG_RT_DONE;
}

describ_tgui_panel(tgui_applist_panel_t, applist_panel_descriptor,
    tgui_region(0, 0, VSF_BOARD_DISP_WIDTH, VSF_BOARD_DISP_HEIGHT),
    tgui_padding(VSF_TGUI_CFG_BORDER_SIZE, VSF_TGUI_CFG_BORDER_SIZE, VSF_TGUI_CFG_BORDER_SIZE, VSF_TGUI_CFG_BORDER_SIZE),

    tgui_list(tAppList, tgui_null_parent(tgui_applist_panel_t), tAppList, tAppList,
        tgui_region(0, VSF_TGUI_CFG_BORDER_SIZE * 3,
            VSF_BOARD_DISP_WIDTH - 2 * VSF_TGUI_CFG_BORDER_SIZE,
            VSF_BOARD_DISP_HEIGHT - 5 * VSF_TGUI_CFG_BORDER_SIZE),
        tgui_msgmap(
            tgui_msg_handler(VSF_TGUI_EVT_ON_DEPOSE, __vsf_tgui_applist_frame_on_depose),
        ),

        tgui_list_items(
            tgui_container_type(VSF_TGUI_CONTAINER_TYPE_LINE_STREAM_VERTICAL),

            tgui_button(tExitBtn, &(tgui_null_parent(tgui_applist_panel_t)->tAppList.list), tExitBtn, tExitBtn,
                .bIsEnabled = false,
                tgui_size(VSF_BOARD_DISP_WIDTH - 2 * VSF_TGUI_CFG_BORDER_SIZE, 2 * VSF_TGUI_CFG_BORDER_SIZE),
                tgui_text(tLabel, "Exit", false),
                tgui_margin(0, 2, 0, 2),
                tgui_msgmap(
                    tgui_msg_handler(VSF_TGUI_EVT_POINTER_CLICK, __vsf_tgui_applist_frame_on_exit_click),
                ),
            ),
        )
    ),
);

static int __vsf_tgui_applist_frame_executor_select(vsf_ui_executor_ctx_t *ctx, fd_set *rfds, fd_set *wfds)
{
    FD_SET(ctx->__stdout_pipe[0], rfds);
    return ctx->__stdout_pipe[0];
}

static void __vsf_tgui_applist_frame_executor_on_select(vsf_ui_executor_ctx_t *ctx, fd_set *rfds, fd_set *wfds)
{
    static const vsf_tgui_user_evt_handler __vsf_tgui_applist_frame_opbtn_handler[] = {
        tgui_msg_handler(VSF_TGUI_EVT_POINTER_CLICK, __vsf_tgui_applist_frame_on_opbtn_click)
    };
    vsf_tgui_applist_frame_t *applist_frame = (vsf_tgui_applist_frame_t *)ctx->param;
    tgui_applist_panel_t *applist_panel = applist_frame->panel;

    if (NULL == rfds) {
        applist_panel->tAppList.list.tExitBtn.bIsEnabled = true;
        vk_tgui_refresh_ex(applist_panel->gui_ptr, &applist_panel->tAppList.list.tExitBtn.use_as__vsf_tgui_control_t, NULL);
    } else if (FD_ISSET(ctx->__stdout_pipe[0], rfds)) {
        bool is_to_refresh = false;
        size_t len = strlen(applist_frame->appname);
        char *end = applist_frame->appname + len;

        len = read(ctx->__stdout_pipe[0], end, sizeof(applist_frame->appname) - len - 1);
        end[len] = '\0';

        while (true) {
            end = strchr(applist_frame->appname, '\n');
            if (NULL == end) { break; }
            *end++ = '\0';

            vsf_tgui_label_t *label = vsf_tgui_label_new("tAppName");
            vsf_tgui_label_set_text_static(label, vsf_heap_strdup(applist_frame->appname));
            vsf_tui_control_set_size(label, (VSF_BOARD_DISP_WIDTH - 2 * VSF_TGUI_CFG_BORDER_SIZE) * 2 / 3,
                2 * VSF_TGUI_CFG_BORDER_SIZE);

            vsf_tgui_button_t *btn = vsf_tgui_button_new("tOpBtn");
            if (applist_frame->is_remote) {
                vsf_tgui_button_set_text_static(btn, "Install");
            } else {
                vsf_tgui_button_set_text_static(btn, "Remove");
            }
            vsf_tui_control_set_size(btn, (VSF_BOARD_DISP_WIDTH - 2 * VSF_TGUI_CFG_BORDER_SIZE) / 3,
                2 * VSF_TGUI_CFG_BORDER_SIZE);
            vsf_control_set_msghandler(&btn->use_as__vsf_tgui_control_t,
                __vsf_tgui_applist_frame_opbtn_handler,
                dimof(__vsf_tgui_applist_frame_opbtn_handler));

            vsf_tgui_container_t *container = vsf_tgui_container_new("tAppContainer");
            vsf_tgui_container_set_type(container, VSF_TGUI_CONTAINER_TYPE_LINE_STREAM_HORIZONTAL);
            vsf_tgui_control_set_margin(container, 0, 2, 0, 2);

            vsf_tgui_control_add(container, NULL, &label->use_as__vsf_tgui_control_t);
            vsf_tgui_control_add(container, &label->use_as__vsf_tgui_control_t, &btn->use_as__vsf_tgui_control_t);
            vsf_tgui_control_add(&applist_panel->tAppList.use_as__vsf_tgui_container_t, NULL, &container->use_as__vsf_tgui_control_t);
            vsf_tgui_control_sync(&applist_panel->tAppList.use_as__vsf_tgui_container_t, &container->use_as__vsf_tgui_control_t);

            is_to_refresh = true;
            strcpy(applist_frame->appname, end);
        }
        if (is_to_refresh) {
            vk_tgui_refresh_ex(applist_panel->gui_ptr, &applist_panel->tAppList.use_as__vsf_tgui_control_t, NULL);
        }
    }
}

static void __vsf_tgui_local_applist_frame_init(vsf_tgui_t *gui, vsf_tgui_frame_t *frame, void *panel, int panel_size, int is_first)
{
    static const char *__title = "Local Applications";

    vsf_tgui_applist_frame_t *applist_frame = (vsf_tgui_applist_frame_t *)frame;
    tgui_applist_panel_t *applist_panel = (tgui_applist_panel_t *)panel;
    VSF_ASSERT(panel_size >= sizeof(tgui_applist_panel_t));

    if (applist_frame->info != NULL) {
        vsf_heap_free(applist_frame->info);
    }

    tgui_initalize_top_container(applist_panel_descriptor, applist_panel);
    applist_panel->frame = applist_frame;
    applist_frame->panel = applist_panel;
    applist_frame->__tile = (char *)__tiles_data;
    applist_frame->is_remote = false;
    vsf_tgui_text_set(&applist_panel->tTitle, &(const vsf_tgui_string_t){
        .pstrText = __title,
#if VSF_TGUI_CFG_SAFE_STRING_MODE == ENABLED
        .s16_size = strlen(__title),
#endif
    });
    vk_tgui_set_root_container(gui, (vsf_tgui_root_container_t *)applist_panel, true);

    if (!is_first) {
        vsf_tgui_execute_in_shell(&__vsf_tgui_app_executor);
    }
}

static void __vsf_tgui_remote_applist_frame_init(vsf_tgui_t *gui, vsf_tgui_frame_t *frame, void *panel, int panel_size, int is_first)
{
    static const char *__title = "Remote Applications";

    vsf_tgui_applist_frame_t *applist_frame = (vsf_tgui_applist_frame_t *)frame;
    tgui_applist_panel_t *applist_panel = (tgui_applist_panel_t *)panel;
    VSF_ASSERT(panel_size >= sizeof(tgui_applist_panel_t));

    tgui_initalize_top_container(applist_panel_descriptor, applist_panel);
    applist_panel->frame = applist_frame;
    applist_frame->panel = applist_panel;
    applist_frame->__tile = (char *)__tiles_data;
    applist_frame->is_remote = true;
    vsf_tgui_text_set(&applist_panel->tTitle, &(const vsf_tgui_string_t){
        .pstrText = __title,
#if VSF_TGUI_CFG_SAFE_STRING_MODE == ENABLED
        .s16_size = strlen(__title),
#endif
    });
    vk_tgui_set_root_container(gui, (vsf_tgui_root_container_t *)applist_panel, true);

    if (!is_first) {
        vsf_tgui_execute_in_shell(&__vsf_tgui_app_executor);
    }
}

// root frame: application loader

typedef struct vsf_tgui_app_t {
    char *name;
    char *cmd;
    char **args;
    vsf_tgui_tile_t *icon;

    int frame_size;
    vsf_tgui_frame_init_t frame_init;
    int (*fn_select)(vsf_ui_executor_ctx_t *ctx, fd_set *rfds, fd_set *wfds);
    void (*on_select)(vsf_ui_executor_ctx_t *ctx, fd_set *rfds, fd_set *wfds);
} vsf_tgui_app_t;

typedef struct vsf_tgui_root_frame_t {
    implement(vsf_tgui_frame_t)
    vsf_tgui_app_t app[9];
    int cur_appidx;
} vsf_tgui_root_frame_t;

def_tgui_panel(tgui_root_panel_t,
    tgui_contains(
        use_tgui_panel(tAppPanel,
            tgui_contains(
                vsf_tgui_button_t tAppBtn[9];
            )
        )
    )
)
vsf_tgui_root_frame_t *frame;
end_def_tgui_panel(tgui_root_panel_t)

static fsm_rt_t __vsf_tgui_root_frame_on_depose(vsf_tgui_control_t *control_ptr, vsf_msgt_msg_t *msg)
{
    tgui_root_panel_t *root_panel = (tgui_root_panel_t *)vsf_tgui_get_cur_panel();
    vsf_tgui_root_frame_t *root_frame = root_panel->frame;
    VSF_ASSERT(root_frame->cur_appidx < dimof(root_frame->app));
    vsf_tgui_app_t *app = &root_frame->app[root_frame->cur_appidx];

    if ((app->frame_init != NULL) && (app->frame_size > 0)) {
        vsf_tgui_frame_t *frame = vsf_tgui_frame_new(app->frame_size, app->frame_init);
        vsf_tgui_frame_init(frame);
        __vsf_tgui_app_executor.param = frame;
    } else {
        __vsf_tgui_app_executor.param = NULL;
    }

    __vsf_tgui_app_executor.cmd = app->cmd;
    __vsf_tgui_app_executor.args = app->args;
    __vsf_tgui_app_executor.on_select = app->on_select;
    __vsf_tgui_app_executor.fn_select = app->fn_select;
    vsf_tgui_execute_in_shell(&__vsf_tgui_app_executor);
    return (fsm_rt_t)VSF_TGUI_MSG_RT_DONE;
}

static fsm_rt_t __vsf_tgui_root_frame_on_appbtn_click(vsf_tgui_control_t *control_ptr, vsf_msgt_msg_t *msg)
{
    tgui_root_panel_t *root_panel = (tgui_root_panel_t *)vsf_tgui_get_cur_panel();
    vsf_tgui_root_frame_t *root_frame = root_panel->frame;
    const vsf_tgui_root_container_t *ptTopContainer = vk_tgui_control_get_top(control_ptr);
    vsf_tgui_t *gui_ptr = ptTopContainer->gui_ptr;

    root_frame->cur_appidx = (vsf_tgui_button_t *)control_ptr - root_panel->tAppPanel.tAppBtn;

    vk_tgui_close_root_container(gui_ptr);
    return (fsm_rt_t)VSF_TGUI_MSG_RT_DONE;
}
static const vsf_tgui_user_evt_handler __vsf_tgui_root_frame_appbtn_handler[] = {
    tgui_msg_handler(VSF_TGUI_EVT_POINTER_CLICK, __vsf_tgui_root_frame_on_appbtn_click)
};

describ_tgui_panel(tgui_root_panel_t, root_panel_descriptor,
    tgui_region(0, 0, VSF_BOARD_DISP_WIDTH, VSF_BOARD_DISP_HEIGHT),
    tgui_text(tTitle, "Application", false),
    tgui_padding(VSF_TGUI_CFG_BORDER_SIZE, VSF_TGUI_CFG_BORDER_SIZE, VSF_TGUI_CFG_BORDER_SIZE, VSF_TGUI_CFG_BORDER_SIZE),
    tgui_msgmap(
        tgui_msg_handler(VSF_TGUI_EVT_ON_DEPOSE, __vsf_tgui_root_frame_on_depose),
    ),

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

static void __vsf_tgui_root_frame_init(vsf_tgui_t *gui, vsf_tgui_frame_t *frame, void *panel, int panel_size, int is_first)
{
    vsf_tgui_root_frame_t *root_frame = (vsf_tgui_root_frame_t *)frame;
    tgui_root_panel_t *root_panel = (tgui_root_panel_t *)panel;
    VSF_ASSERT(panel_size >= sizeof(tgui_root_panel_t));

    tgui_initalize_top_container(root_panel_descriptor, root_panel);
    root_frame->__tile = (char *)__tiles_data;
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
    vsf_tgui_root_frame_t *root_frame = (vsf_tgui_root_frame_t *)vsf_tgui_frame_new(
        sizeof(vsf_tgui_root_frame_t), __vsf_tgui_root_frame_init);
    // TODO: load applications
    root_frame->app[0].name = "Local\nApps";
    root_frame->app[0].cmd = "vpm";
    static const char *__vpm_list_local_args[] = {
        "vpm", "list-local", NULL
    };
    root_frame->app[0].args = (char **)__vpm_list_local_args;
    root_frame->app[0].icon = (vsf_tgui_tile_t *)&res_local_RGBA;
    root_frame->app[0].frame_size = sizeof(vsf_tgui_applist_frame_t);
    root_frame->app[0].frame_init = __vsf_tgui_local_applist_frame_init;
    root_frame->app[0].on_select = __vsf_tgui_applist_frame_executor_on_select;
    root_frame->app[0].fn_select = __vsf_tgui_applist_frame_executor_select;
    root_frame->app[1].name = "Remote\nApps";
    root_frame->app[1].cmd = "vpm";
    static const char *__vpm_list_remmote_args[] = {
        "vpm", "list-remote", NULL
    };
    root_frame->app[1].args = (char **)__vpm_list_remmote_args;
    root_frame->app[1].icon = (vsf_tgui_tile_t *)&res_cloud_RGBA;
    root_frame->app[1].frame_size = sizeof(vsf_tgui_applist_frame_t);
    root_frame->app[1].frame_init = __vsf_tgui_remote_applist_frame_init;
    root_frame->app[1].on_select = __vsf_tgui_applist_frame_executor_on_select;
    root_frame->app[1].fn_select = __vsf_tgui_applist_frame_executor_select;
    vsf_tgui_frame_init(&root_frame->use_as__vsf_tgui_frame_t);

    int executor_exit_notifier = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    int executor_new_notifier = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    VSF_ASSERT((executor_new_notifier >= 0) && (executor_exit_notifier >= 0));
    eventfd_t eventfd_value;
    vsf_ui_executor_ctx_t *executor;
    fd_set rfds, wfds;
    pid_t pid;
    int fd_num;

    __vsf_ui_new_eventfd_priv = vsf_linux_fd_get(executor_new_notifier)->priv;
    __vsf_ui_exit_eventfd_priv = vsf_linux_fd_get(executor_exit_notifier)->priv;
    vsf_dlist_init(&__vsf_ui_executor_queue);
    vsf_dlist_init(&__vsf_ui_executor_queue_pending);
    while (true) {
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_SET(executor_new_notifier, &rfds);
        FD_SET(executor_exit_notifier, &rfds);
        fd_num = vsf_max(executor_exit_notifier, executor_new_notifier);
        __vsf_dlist_foreach_unsafe(vsf_ui_executor_ctx_t, __node, &__vsf_ui_executor_queue) {
            if (_->fn_select!= NULL) {
                int fd = _->fn_select(_, &rfds, &wfds);
                if (fd > fd_num) {
                    fd_num = fd;
                }
            }
        }
        fd_num = select(fd_num + 1, &rfds, &wfds, NULL, NULL);
        if (fd_num <= 0) { continue; }

        if (FD_ISSET(executor_new_notifier, &rfds)) {
            eventfd_read(executor_new_notifier, &eventfd_value);
            vsf_protect_t orig = vsf_protect_int();
                vsf_dlist_remove_head(vsf_ui_executor_ctx_t, __node, &__vsf_ui_executor_queue_pending, executor);
            vsf_unprotect_int(orig);

            VSF_ASSERT(executor!= NULL);
            vsf_dlist_add_to_tail(vsf_ui_executor_ctx_t, __node, &__vsf_ui_executor_queue, executor);

            executor->__exited = false;
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
                close(executor->__stdin_pipe[0]);
                close(executor->__stdout_pipe[1]);
                close(executor->__stderr_pipe[1]);
                close(executor->__stdin_pipe[1]);
                close(executor->__stdout_pipe[0]);
                close(executor->__stderr_pipe[0]);

                atexit(__vsf_ui_executor_atexit);

                execvp(executor->cmd, executor->args);
                VSF_ASSERT(false);
            }
        }
        if (FD_ISSET(executor_exit_notifier, &rfds)) {
            eventfd_read(executor_exit_notifier, &eventfd_value);
        }

        __vsf_dlist_foreach_next_unsafe(vsf_ui_executor_ctx_t, __node, &__vsf_ui_executor_queue) {
            if (_->__exited) {
                close(_->__stdin_pipe[1]);
                close(_->__stdout_pipe[0]);
                close(_->__stderr_pipe[0]);
                if (_->on_select != NULL) {
                    _->on_select(_, NULL, NULL);
                }
                vsf_dlist_remove(vsf_ui_executor_ctx_t, __node, &__vsf_ui_executor_queue, _);
            } else if (    FD_ISSET(_->__stdin_pipe[1], &wfds)
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
