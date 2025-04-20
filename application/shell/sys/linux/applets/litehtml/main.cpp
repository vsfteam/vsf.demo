#include <vsf.h>
#include <vsf_board.h>

#include <unistd.h>
#include <getopt.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <SDL.h>
#include <litehtml.h>

#include "./SDLContainer.h"

#if VSF_USE_MBEDTLS == ENABLED
#   include "component/3rd-party/mbedtls/extension/tls_session/mbedtls_tls_session.h"
#endif

static int __vsf_http_session_connect(void *param, const char *host, const char *port);
static void __vsf_http_session_close(void *param);
static int __vsf_http_session_write(void *param, uint8_t *buf, uint16_t len);
static int __vsf_http_session_read(void *param, uint8_t *buf, uint16_t len);

const vsf_http_op_t vsf_http_op = {
    .fn_connect     = (int (*)(void *, const char *, const char *))__vsf_http_session_connect,
    .fn_close       = (void (*)(void *))__vsf_http_session_close,
    .fn_write       = (int (*)(void *, uint8_t *, uint16_t))__vsf_http_session_write,
    .fn_read        = (int (*)(void *, uint8_t *, uint16_t))__vsf_http_session_read,
};

static int __vsf_http_session_connect(void *param, const char *host, const char *port)
{
    int* fd = (int *)param, result = -1;
    struct addrinfo hints, *addr_list, *cur;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(host, port, &hints, &addr_list) != 0) {
        return result;
    }

    for (cur = addr_list; cur != NULL; cur = cur->ai_next) {
        *fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (*fd < 0) {
            continue;
        }

        if (connect(*fd, cur->ai_addr, cur->ai_addrlen) == 0) {
            result = 0;
            break;
        }

        close(*fd);
    }

    freeaddrinfo(addr_list);
    return result;
}

static void __vsf_http_session_close(void *param)
{
    int* fd = (int *)param;
    if (*fd >= 0) {
        close(*fd);
        *fd = -1;
    }
}

static int __vsf_http_session_write(void *param, uint8_t *buf, uint16_t len)
{
    int* fd = (int *)param;
    return send(*fd, buf, len, 0);
}

static int __vsf_http_session_read(void *param, uint8_t *buf, uint16_t len)
{
    int* fd = (int *)param;
    return recv(*fd, buf, len, 0);
}

class VSF_Container : public SDLContainer
{
public:
    VSF_Container(int width, int height)
        : SDLContainer(width ,height) {}
    ~VSF_Container(void) {}

    typedef enum {
        HOST_FS,
        HOST_HTTP,
    } host_type;

    void import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) override {
        litehtml::string urlout;
        if (!request_target(text, url, baseurl, urlout)) {
            vsf_trace_error("%s: can not handle %s %s\n", __FUNCTION__, baseurl.c_str(), url.c_str());
        } else {
            baseurl = urlout;
        }
    }
    void set_base_url(const char* base_url) override {
        m_baseurl = base_url;
    }

    bool request_target(litehtml::string& text, const litehtml::string url, litehtml::string baseurl, litehtml::string& urlout) {
        if (m_hosttype == HOST_HTTP) {
            return request_url(text, url, baseurl, urlout);
        } else if (m_hosttype == HOST_FS) {
            return request_file(text, url);
        } else {
            return false;
        }
    }
    bool request_target(litehtml::string& text, const litehtml::string url, litehtml::string baseurl) override {
        litehtml::string urlout;
        return request_target(text, url, baseurl, urlout);
    }
    void set_host_type(host_type type) {
        m_hosttype = type;
    }

private:
    host_type m_hosttype;
    std::string m_baseurl;
    std::string m_host;
    bool m_is_https;

    bool request_file(litehtml::string& text, const litehtml::string file) {
        FILE* fp = fopen(file.c_str(), "r");
        if (fp == NULL) {
            vsf_trace_error("Failed to open file: %s\n", file.c_str());
            return false;
        }
        fseek(fp, 0, SEEK_END);
        size_t size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        text.resize(size + 1);
        fread((void*)text.c_str(), 1, size, fp);
        fclose(fp);
        return true;
    }
    void make_url(const litehtml::string url, litehtml::string& basepath, litehtml::string& out) {
        if ((url.find("https://", 0) == 0) || (url.find("http://", 0) == 0)) {
            out = url;
        } else if (url.find("//", 0) == 0) {
            if (m_is_https) {
                out = "https:" + url;
            } else {
                out = "http:" + url;
            }
        } else if (url.find("/", 0) == 0) {
            if (m_is_https) {
                out = "https://" + m_host + url;
            } else {
                out = "http://" + m_host + url;
            }
        } else {
            if (basepath.empty()) {
                basepath = m_baseurl;
            }
            if (basepath.back() != '/') {
                size_t pos = basepath.rfind('/');
                if (pos != std::string::npos) {
                    basepath = basepath.substr(0, pos + 1);
                }
            }
            out = basepath + url;
        }
    }
    bool request_url(litehtml::string& text, const litehtml::string url, litehtml::string baseurl, litehtml::string& urlout) {
        litehtml::string urlmut = url;
        litehtml::string port;
        size_t port_pos = urlmut.rfind(':');
        if ((port_pos != std::string::npos) && (urlmut[port_pos + 1] != '/')) {
            port = urlmut.substr(port_pos + 1);
            urlmut = urlmut.substr(0, port_pos);
        }

        make_url(urlmut, baseurl, urlout);

        vsf_http_client_t http;
        mbedtls_session_t session = { 0 };
        int fd = -1;

        size_t pos;
        std::string host, path;
        if (urlout.find("https://", 0) == 0) {
            m_is_https = true;
            pos = urlout.find('/', 8);
            host = urlout.substr(8, std::string::npos == pos ? pos : pos - 8);
            path = std::string::npos == pos ? "/" : urlout.substr(pos);

            http.op = &vsf_mbedtls_http_op;
            http.param = &session;
            if (port.empty()) {
                port = "443";
            }
        } else {
            m_is_https = false;
            pos = urlout.find('/', 7);
            host = urlout.substr(7, std::string::npos == pos ? pos : pos - 7);
            path = std::string::npos == pos ? "/" : urlout.substr(pos);

            http.op = &vsf_http_op;
            http.param = &fd;
            if (port.empty()) {
                port = "80";
            }
        }

        vsf_trace_debug("%s: %s %s\n", __FUNCTION__, host.c_str(), path.c_str());
        vsf_http_client_req_t req = {
            .host = host.c_str(),
            .port = port.c_str(),
            .verb = "GET",
            .path = (char*)path.c_str(),
        };
        vsf_http_client_init(&http);
        if (vsf_http_client_request(&http, &req) < 0) {
            vsf_trace_error("failed to start http with response %d\n", http.resp_status);
            return false;
        }

        if (    ((302 == http.resp_status) || (301 == http.resp_status))
            &&  (http.redirect_path != NULL)) {
            vsf_http_client_close(&http);
            urlmut = http.redirect_path;
            return request_url(text, urlmut, baseurl, urlout);
        }

        int rsize, cur_pos;
        char buffer[512 + 1], *cur_ptr;
        while ((rsize = vsf_http_client_read(&http, (uint8_t*)buffer, sizeof(buffer) - 1)) > 0) {
            cur_pos = text.length();
            text.resize(text.length() + rsize);
            cur_ptr = (char *)text.c_str() + cur_pos;
            memcpy(cur_ptr, buffer, rsize);
        }
        vsf_http_client_close(&http);

        if (m_host.empty()) {
            m_host = host;
        }

        return true;
    }
};

static int __litehtml_main(int argc, char **argv)
{
    VSF_Container container(VSF_BOARD_DISP_WIDTH, VSF_BOARD_DISP_HEIGHT);
    const litehtml::position pos(0, 0, VSF_BOARD_DISP_WIDTH, VSF_BOARD_DISP_HEIGHT);
    litehtml::document::ptr doc;

    std::string html_content, html_path, target;
    int height, ch;

    while ((ch = getopt(argc, argv, "u:f:d:t:h")) >= 0) {
        target = optarg;
        switch (ch) {
        case 'u':
            container.set_host_type(VSF_Container::HOST_HTTP);
            if (!container.request_target(html_content, target, "")) {
                vsf_trace_error("failed to request url %s\n", target.c_str());
                return -1;
            }
            break;
        case 'f':
        open_file:
            container.set_host_type(VSF_Container::HOST_FS);
            if (!container.request_target(html_content, target, "")) {
                vsf_trace_error("failed to open file %s\n", target.c_str());
                return -1;
            }
            break;
        case 'd':
            html_path = target;
            if (html_path.back() != '/') {
                html_path += '/';
            }
            html_path += "index.htm";
            if (access(html_path.c_str(), R_OK) != 0) {
                html_path += "l";
                if (access(html_path.c_str(), R_OK) != 0) {
                    vsf_trace_error("Failed to open directory: %s\n", target.c_str());
                    return -1;
                }
            }
            target = html_path;
            goto open_file;
        case 't':
            html_content = target;
            break;
        case 'h':
        help:
            printf("Usage: %s [-u url] [-f file] [-d directory] [-t html_content] [-h]\n", argv[0]);
            return 0;
        }
    }
    if (html_content.empty()) {
        goto help;
    }

    doc = litehtml::document::createFromString(html_content, &container);
    height = doc->render(VSF_BOARD_DISP_WIDTH);
    doc->draw(0, 0, 0, &pos);
    SDL_RenderPresent(container.get_renderer());

    SDL_Event evt;
    while (1) {
        SDL_WaitEvent(&evt);
        printf("evt: %d\n", evt.type);
    }

    return 0;
}

extern "C" {
    int main(int argc, char **argv)
    {
        return __litehtml_main(argc, argv);
    }
}
