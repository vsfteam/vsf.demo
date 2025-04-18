#include <vsf.h>
#include <vsf_board.h>

#include <unistd.h>
#include <getopt.h>

#include <SDL.h>
#include <litehtml.h>

#include "./SDLContainer.h"

#if VSF_USE_MBEDTLS == ENABLED
#   include "component/3rd-party/mbedtls/extension/tls_session/mbedtls_tls_session.h"
#endif

class VSF_Container : public SDLContainer
{
public:
    VSF_Container(int width, int height)
        : SDLContainer(width ,height) {}
    ~VSF_Container(void) {}

    void import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) override {
        if (!request_target(text, url, baseurl)) {
            vsf_trace_error("%s: can not handle %s %s\n", __FUNCTION__, baseurl.c_str(), url.c_str());
        }
    }

    bool request_target(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) {
        return request_url(text, url, baseurl) || request_file(text, baseurl + '/' + url);
    }
    bool request_file(litehtml::string& text, const litehtml::string file) {
        FILE* fp = fopen(file.c_str(), "r");
        if (fp == NULL) {
            printf("Failed to open file: %s\n", file.c_str());
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
    bool request_url(litehtml::string& text, const litehtml::string url, litehtml::string baseurl) {
        if (url.find("https://", 0) == 0) {
            size_t pos = url.find('/', 8);
            std::string host = url.substr(8, std::string::npos == pos ? pos : pos - 8);
            std::string path = std::string::npos == pos ? "/" : url.substr(pos);

            vsf_trace_debug("%s: %s %s\n", __FUNCTION__, host.c_str(), path.c_str());
            vsf_http_client_t* http = (vsf_http_client_t*)malloc(sizeof(vsf_http_client_t) + sizeof(mbedtls_session_t));
            if (NULL == http) {
                printf("failed to allocate http context\n");
                return true;
            }

            mbedtls_session_t* session = (mbedtls_session_t*)&http[1];
            memset(session, 0, sizeof(*session));
            http->op = &vsf_mbedtls_http_op;
            http->param = session;
            vsf_http_client_init(http);

            vsf_http_client_req_t req = {
                .host = host.c_str(),
                .port = "443",
                .verb = "GET",
                .path = (char*)path.c_str(),
            };
            if (vsf_http_client_request(http, &req) < 0) {
                printf("failed to start http with response %d\n", http->resp_status);
                goto failure;
            }
            int rsize;
            char buffer[512 + 1];
            while ((rsize = vsf_http_client_read(http, (uint8_t*)buffer, sizeof(buffer) - 1)) > 0) {
                buffer[rsize] = '\0';
                text += buffer;
            }

        failure:
            free(http);
            http = NULL;
            return true;
        }
        return false;
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
            if (!container.request_url(html_content, target, "")) {
                printf("failed to request url %s\n", target.c_str());
                return -1;
            }
            break;
        case 'f':
        open_file:
            if (!container.request_file(html_content, target)) {
                printf("failed to open file %s\n", target.c_str());
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
                    printf("Failed to open directory: %s\n", target.c_str());
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
    SDL_RenderPresent(container.m_renderer);

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
