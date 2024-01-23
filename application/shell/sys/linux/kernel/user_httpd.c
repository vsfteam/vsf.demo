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

#include <unistd.h>
#include <pty.h>

#define __VSF_LINUX_HTTPD_CLASS_INHERIT__
#include "shell/sys/linux/app/httpd/vsf_linux_httpd.h"

static int __user_httpd_terminal_on_open(vsf_linux_httpd_request_t *req);
static void __user_httpd_terminal_on_error(vsf_linux_httpd_request_t *req);
static void __user_httpd_terminal_on_close(vsf_linux_httpd_request_t *req);
static void __user_httpd_terminal_on_message(vsf_linux_httpd_request_t *req, uint8_t *buf, uint32_t len);

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
    },
};

static int __user_httpd_terminal_on_open(vsf_linux_httpd_request_t *req)
{
    vsf_linux_httpd_session_t *session = container_of(req, vsf_linux_httpd_session_t, request);
    int master = -1;
    int pid = forkpty(&master, NULL, NULL, NULL);

    switch (pid) {
    case -1:
        printf("fail to forkpty\n");
        return -1;
    case 0:
        execl("/bin/sh", "sh", NULL);
        break;
    default:
        if (session->fd_stream_out >= 0) {
            close(session->fd_stream_out);
        }
        session->fd_stream_out = master;
        break;
    }
    return 0;
}

static void __user_httpd_terminal_on_error(vsf_linux_httpd_request_t *req)
{
}

static void __user_httpd_terminal_on_close(vsf_linux_httpd_request_t *req)
{
}

static void __user_httpd_terminal_on_message(vsf_linux_httpd_request_t *req, uint8_t *buf, uint32_t len)
{
    vsf_linux_httpd_session_t *session = container_of(req, vsf_linux_httpd_session_t, request);
    write(session->fd_stream_out, buf, len);
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
