#ifndef __USER_UI_H__
#define __USER_UI_H__

#include <unistd.h>

// for vsf_tgui_tile_t
#include <vsf.h>

#ifndef VSF_TGUI_CFG_BORDER_SIZE
#   define VSF_TGUI_CFG_BORDER_SIZE         16
#endif

typedef struct vsf_tgui_frame_t vsf_tgui_frame_t;
typedef struct vsf_tgui_t vsf_tgui_t;
typedef void (*vsf_tgui_frame_init_t)(vsf_tgui_t *gui, vsf_tgui_frame_t *frame, void *panel, int panel_size, int is_first);
struct vsf_tgui_frame_t {
    vsf_tgui_frame_t *__next;
    char *__tile;
    vsf_tgui_frame_init_t __fn_init;
};

typedef struct vsf_ui_executor_ctx_t vsf_ui_executor_ctx_t;
struct vsf_ui_executor_ctx_t {
    vsf_dlist_node_t __node;

    char *cmd;
    char **args;
    int (*fn_select)(vsf_ui_executor_ctx_t *ctx, fd_set *rfds, fd_set *wfds);
    // if NULL == rfds, it means forked process exited
    void (*on_select)(vsf_ui_executor_ctx_t *ctx, fd_set *rfds, fd_set *wfds);
    void *param;

    bool __exited;
    int __pid;
    int __stdout_pipe[2], __stdin_pipe[2], __stderr_pipe[2];
};

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

extern int vsf_tgui_install_apps(vsf_tgui_app_t *apps, int app_num);

extern int vsf_tgui_executor_select_stdout(vsf_ui_executor_ctx_t *ctx, fd_set *rfds, fd_set *wfds);

extern vsf_tgui_frame_t * vsf_tgui_frame_new(int frame_size, vsf_tgui_frame_init_t fn_init);
extern void vsf_tgui_frame_init(vsf_tgui_frame_t *frame);
extern void vsf_tgui_frame_exit(void);

extern vsf_ui_executor_ctx_t __vsf_tgui_app_executor;
extern void vsf_tgui_execute_in_shell(vsf_ui_executor_ctx_t *ctx);

declare_tgui_panel(vsf_tgui_popup_panel_t)
typedef struct vsf_tgui_popup_frame_t vsf_tgui_popup_frame_t;
struct vsf_tgui_popup_frame_t {
    implement(vsf_tgui_frame_t)
    const char *pstrTitle;
    const char *pstrInformation;
    vsf_tgui_popup_panel_t *panel;
    void *param;
};

def_tgui_panel(vsf_tgui_popup_panel_t,
    tgui_contains(
        vsf_tgui_label_t tInformation;
        vsf_tgui_button_t tOK;
    )
)
vsf_tgui_popup_frame_t *frame;
end_def_tgui_panel(vsf_tgui_popup_panel_t)
extern vsf_tgui_popup_frame_t * vsf_tgui_popup(const char *pstrTitle, const char *pstrInformation);

#endif
