#include <vsf.h>

#if VSF_USE_TCPIP == ENABLED && VSF_USE_JSON == ENABLED
#include "component/3rd-party/mbedtls/extension/tls_session/mbedtls_tls_session.h"

int __deepseek_chat_main(int argc, char *argv[])
{
    vsf_http_client_t *http = (vsf_http_client_t *)malloc(sizeof(vsf_http_client_t) +
                    sizeof(mbedtls_session_t) + 1024);
    mbedtls_session_t *session = (mbedtls_session_t *)&http[1];
    char *json_body = (char *)&session[1], *prompt, *response;
    int response_size;

    char header[256], *env_key = getenv("DEEPSEEK_API_KEY");
    if (NULL == env_key) {
        printf("Please set DEEPSEEK_API_KEY environment\n");
        return -1;
    }
    snprintf(header, sizeof(header), "Authorization: Bearer %s\r\nContent-Type: application/json\r\n", env_key);
    strcpy(json_body, "{\"model\":\"deepseek-chat\",\"messages\":[{\"role\":\"user\",\"content\":\"");
    prompt = &json_body[strlen(json_body)];
    response = prompt;
    response_size = &json_body[1024] - response;

    while (1) {
        printf("\nprompt: ");
        fgets(prompt, 1024, stdin);
        prompt[strcspn(prompt, "\n")] = '\0';
        if (!strcmp(prompt, "exit")) {
            break;
        }
        strcat(prompt, "\"}]}");

        memset(session, 0, sizeof(*session));
        http->op = &vsf_mbedtls_http_op;
        http->param = session;
        vsf_http_client_init(http);

        int result = vsf_http_client_request(http, &(vsf_http_client_req_t){
            .host       = "api.deepseek.com",
            .port       = "443",
            .verb       = "POST",
            .path       = "/chat/completions",
            .header     = header,
            .txdata     = (uint8_t *)json_body,
            .txdata_len = strlen(json_body),
        });

        if (result) {
            printf("fail to send request to deepseek\n");
            break;
        }

        printf("\nresponse: ");
        int rsize = vsf_http_client_read(http, (uint8_t *)response, response_size);
        if (rsize > 0) {
            response[rsize - 1] = '\0';

            char *content = vsf_json_get(response, "choices/0/message/content");
            if (content != NULL) {
                vsf_json_get_string(content, response, response_size);
                printf("%s", response);
            }
        }
        printf("\n");
    }
    return 0;
}
#endif
