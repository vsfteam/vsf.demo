#include "vsf.h"

#if VSF_USE_TCPIP == ENABLED

#include <pthread.h>

extern bool app_shell_is_orig_busybox(void);

static const char __user_httpd_root[] = VSF_STR(
<html>
  <head>
    <link rel="stylesheet" href="http://xtermjs.org/css/xterm.css" />
    <script src="http://xtermjs.org/js/xterm.js"></script>
  </head>
  <body>
    <h1 style="text-align: center;">WebTerminal</h1>
    <div style="width: 736px; height: 408px; margin: auto;">
      <div id="terminal"></div>
    </div>
    <script>
      const term = new Terminal();
      term.open(document.getElementById('terminal'));

      const socket = new WebSocket("ws://" + window.location.host + "/webterminal");
      let reg_match_rn = new RegExp('\r\n', 'g');
      let reg_match_n = new RegExp('\n', 'g');

      term.onData((data) => {
        socket.send(data);
      });
      socket.onmessage = (event) => {
        let str = event.data;
        str = str.replace(reg_match_rn, '\n');
        str = str.replace(reg_match_n, '\r\n');
        term.write(str);
      }
    </script>
  </body>
</html>
);

#define __VSF_LINUX_FS_CLASS_INHERIT__
#include <unistd.h>
#include <pty.h>
// for O_NOCTTY
#include <fcntl.h>

#include "shell/sys/linux/app/httpd/vsf_linux_httpd.h"

static int __user_httpd_terminal_on_open(vsf_linux_httpd_request_t *req);
static void __user_httpd_terminal_on_error(vsf_linux_httpd_request_t *req);
static void __user_httpd_terminal_on_close(vsf_linux_httpd_request_t *req);
static void __user_httpd_terminal_on_message(vsf_linux_httpd_request_t *req,
    const vsf_linux_httpd_urihandler_websocket_t *websocket_ctx, uint8_t *buf, uint32_t len);
static int __user_httpd_terminal_poll(vsf_linux_httpd_request_t *req, fd_set *rset, fd_set *wset, bool prepare);

static const vsf_linux_httpd_urihandler_t __user_httpd_urihandler[] = {
    {
        .match              = VSF_LINUX_HTTPD_URI_MATCH_URI,
        .uri                = "/",
        .type               = VSF_LINUX_HTTPD_URI_OP,
        .op                 = &vsf_linux_httpd_urihandler_buffer_op,
        .buffer             = {
            .ptr            = (uint8_t *)__user_httpd_root,
            .size           = sizeof(__user_httpd_root) - 1,
        },
    },
    {
        .match              = VSF_LINUX_HTTPD_URI_MATCH_URI,
        .uri                = "/webterminal",
        .type               = VSF_LINUX_HTTPD_URI_OP,
        .op                 = &vsf_linux_httpd_urihandler_websocket_op,
        .websocket          = {
            .on_open        = __user_httpd_terminal_on_open,
            .on_error       = __user_httpd_terminal_on_error,
            .on_close       = __user_httpd_terminal_on_close,
            .on_message     = __user_httpd_terminal_on_message,
        },
        .poll_fn            = __user_httpd_terminal_poll,
    },
};

typedef struct __user_httpd_terminal_ctx_t {
    int fd_master;
    pid_t pid_child;
} __user_httpd_terminal_ctx_t;
static __user_httpd_terminal_ctx_t __user_httpd_terminal_ctx = {
    .fd_master = -1,
    .pid_child = -1,
};

static int __user_httpd_terminal_on_open(vsf_linux_httpd_request_t *req)
{
    if (__user_httpd_terminal_ctx.fd_master >= 0) {
        req->target = &__user_httpd_terminal_ctx;
    } else {
        int master = -1;
        pid_t pid = forkpty(&master, NULL, NULL, NULL);

        switch (pid) {
        case -1:
            printf("fail to forkpty\n");
            return -1;
        case 0:
            if (!app_shell_is_orig_busybox()) {
                vsf_linux_fd_t *sfd = vsf_linux_fd_get(STDIN_FILENO);
                sfd->priv->flags &= ~O_NOCTTY;
            }
            execl("/bin/sh", "sh", NULL);
            break;
        default:
            __user_httpd_terminal_ctx.fd_master = master;
            __user_httpd_terminal_ctx.pid_child = pid;
            req->target = &__user_httpd_terminal_ctx;
            break;
        }
    }
    return 0;
}

static void __user_httpd_terminal_on_error(vsf_linux_httpd_request_t *req)
{
}

static void __user_httpd_terminal_on_close(vsf_linux_httpd_request_t *req)
{
    req->target = NULL;
}

static void __user_httpd_terminal_on_message(vsf_linux_httpd_request_t *req,
    const vsf_linux_httpd_urihandler_websocket_t *websocket_ctx, uint8_t *buf, uint32_t len)
{
    __user_httpd_terminal_ctx_t *ctx = req->target;
    if (ctx->fd_master >= 0) {
        write(ctx->fd_master, buf, len);
    }
}

static int __user_httpd_terminal_poll(vsf_linux_httpd_request_t *req,
    fd_set *rset, fd_set *wset, bool prepare)
{
    __user_httpd_terminal_ctx_t *ctx = req->target;
    if (NULL == ctx) {
        return 0;
    }
    if (ctx->fd_master < 0) {
        return -1;
    }

    if (prepare) {
        FD_SET(ctx->fd_master, rset);
        return ctx->fd_master;
    } else if (FD_ISSET(ctx->fd_master, rset)) {
        vsf_linux_fd_t *sfd = vsf_linux_fd_get(ctx->fd_master);
        vsf_linux_stream_priv_t *priv = (vsf_linux_stream_priv_t *)sfd->priv;
        uint8_t *ptr_src;
        int size_src = vsf_stream_get_rbuf(priv->stream_rx, &ptr_src);
        int realsize = vsf_linux_httpd_websocket_write(req, ptr_src, size_src, true);
        if (realsize > 0) {
            read(ctx->fd_master, ptr_src, size_src);
        }
        return 1;
    } else {
        return 0;
    }
}

// webterminal [PORT]
int webterminal_main(int argc, char **argv)
{
    static bool __httpd_webterminal_started = false;
    if (!__httpd_webterminal_started) {
        __httpd_webterminal_started = true;

        int port = argc == 2 ? atoi(argv[1]) : 80;

        vsf_linux_httpd_t __user_httpd_webterminal = {
            .port               = port,
            .backlog            = 8,

            .num_of_urihandler  = dimof(__user_httpd_urihandler),
            .urihandler         = (vsf_linux_httpd_urihandler_t *)__user_httpd_urihandler,
        };

        char __name[32], *name;
        if (!app_config_read("webterminal_name", __name, sizeof(__name))) {
            name = __name;
        } else {
            name = "webterminal";
        }
        const char *txt[] = {
            "path=/",
        };
        app_mdns_update_service(NULL, name, "_http", port, true, txt, dimof(txt));

        daemon(1, 1);
        vsf_linux_httpd_thread(&__user_httpd_webterminal);
        __httpd_webterminal_started = false;
        return 0;
    }

    printf("httpd aready started.\n");
    return -1;
}

#endif
