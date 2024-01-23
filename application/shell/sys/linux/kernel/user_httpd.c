#include "vsf.h"

#if VSF_USE_TCPIP == ENABLED

static const char __user_httpd_root[] = VSF_STR(
<html>
  <head>
    <link rel="stylesheet" href="http://xtermjs.org/css/xterm.css" />
    <script src="http://xtermjs.org/js/xterm.js"></script>
  </head>
  <body>
    <div style="width: 736px; height: 408px;">
      <div id="terminal"></div>
    </div>
    <script>
      const term = new Terminal();
      term.open(document.getElementById('terminal'));

      const socket = new WebSocket("ws://" + window.location.host + "/webterminal");

      term.onData((data) => {
        socket.send(data);
      });
      socket.onmessage = (event) => {
        term.write(event.data);
      }
    </script>
  </body>
</html>
);

#define __VSF_LINUX_FS_CLASS_INHERIT__
#include <unistd.h>
#include <pty.h>

#define __VSF_LINUX_HTTPD_CLASS_INHERIT__
#include "shell/sys/linux/app/httpd/vsf_linux_httpd.h"

static int __user_httpd_terminal_on_open(vsf_linux_httpd_request_t *req);
static void __user_httpd_terminal_on_error(vsf_linux_httpd_request_t *req);
static void __user_httpd_terminal_on_close(vsf_linux_httpd_request_t *req);
static void __user_httpd_terminal_on_message(vsf_linux_httpd_request_t *req, uint8_t *buf, uint32_t len);
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

static int __user_httpd_terminal_on_open(vsf_linux_httpd_request_t *req)
{
    int master = -1;
    pid_t pid = forkpty(&master, NULL, NULL, NULL);

    switch (pid) {
    case -1:
        printf("fail to forkpty\n");
        return -1;
    case 0:
        execl("/bin/sh", "sh", NULL);
        break;
    default: {
            __user_httpd_terminal_ctx_t *ctx = malloc(sizeof(*ctx));
            if (NULL == ctx) {
                VSF_LINUX_ASSERT(false);
                return -1;
            }
            ctx->fd_master = master;
            ctx->pid_child = pid;
            req->target = (char *)ctx;
        }
        break;
    }
    return 0;
}

static void __user_httpd_terminal_on_error(vsf_linux_httpd_request_t *req)
{
}

static void __user_httpd_terminal_on_close(vsf_linux_httpd_request_t *req)
{
    free(req->target);
    req->target = NULL;
}

static void __user_httpd_terminal_on_message(vsf_linux_httpd_request_t *req, uint8_t *buf, uint32_t len)
{
    __user_httpd_terminal_ctx_t *ctx = req->target;
    if (ctx->fd_master >= 0) {
        write(ctx->fd_master, buf, len);
    }
}

static int __user_httpd_terminal_poll(vsf_linux_httpd_request_t *req, fd_set *rset, fd_set *wset, bool prepare)
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
    } else {
        int result = 0;
        vsf_linux_fd_t *sfd = vsf_linux_fd_get(ctx->fd_master);
        vsf_linux_stream_priv_t *priv = (vsf_linux_stream_priv_t *)sfd->priv;
        uint8_t *ptr_src;
        uint_fast32_t size_src, size_dst;
        vsf_stream_t *stream;
        uint8_t header[2];

        if (FD_ISSET(ctx->fd_master, rset)) {
            result++;

            stream = req->stream_out;
            if (stream != NULL) {
                size_src = vsf_stream_get_rbuf(priv->stream_rx, &ptr_src);
                // max frame size for websocket with 2-byte header
                size_src = vsf_min(size_src, 125);

                size_dst = vsf_stream_get_free_size(stream);
                // reserve 2-byte websocket header in dest
                size_src = vsf_min(size_src, size_dst - 2);

                header[0] = 0x81;
                header[1] = size_src;
                vsf_stream_write(stream, header, 2);
                vsf_stream_write(stream, ptr_src, size_src);
                read(ctx->fd_master, ptr_src, size_src);
            }
        }
        return result;
    }
}

int usr_httpd_start(void)
{
    static bool __httpd_started = false;
    if (!__httpd_started) {
        __httpd_started = true;

        static vsf_linux_httpd_t __user_httpd = {
            .port               = 80,
            .backlog            = 1,

            .num_of_urihandler  = dimof(__user_httpd_urihandler),
            .urihandler         = (vsf_linux_httpd_urihandler_t *)__user_httpd_urihandler,
        };
        vsf_linux_httpd_start(&__user_httpd);
        return 0;
    }

    printf("httpd aready started.\n");
    return -1;
}

#endif
