#define __VSF_ROMFS_CLASS_INHERIT__

#include <unistd.h>

#define REPO_HOST_NAME                  "gitee.com"
#define REPO_HOST_PORT                  "80"

#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/certs.h>

typedef struct mbedtls_session_t {
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_net_context server_fd;
    mbedtls_x509_crt cacert;

    uint8_t buffer[1024];
    uint8_t *cur_buffer;
    uint16_t cur_size;
} mbedtls_session_t;

static void __mbedtls_debug(void *ctx, int level, const char *file, int line, const char *str)
{
    (void)level;
    fprintf((FILE *)ctx, "%s:%04d: %s", file, line, str);
}

void mbedtls_session_cleanup(mbedtls_session_t *session)
{
    mbedtls_net_free(&session->server_fd);
    mbedtls_x509_crt_free(&session->cacert);
    mbedtls_ssl_free(&session->ssl);
    mbedtls_ssl_config_free(&session->conf);
    mbedtls_ctr_drbg_free(&session->ctr_drbg);
    mbedtls_entropy_free(&session->entropy);
}

int mbedtls_session_write(mbedtls_session_t *session, uint8_t *buf, uint16_t len)
{
    int ret, result = 0;
    printf("  > Write to server:");
    while (len > 0) {
        ret = mbedtls_ssl_write(&session->ssl, buf, len);
        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
            continue;
        }
        if (ret < 0) {
            printf(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
            return -1;
        }
        buf += ret;
        len -= ret;
        result += ret;
    }
    printf(" %d bytes written\n\n", result);
    return result;
}

int mbedtls_session_read(mbedtls_session_t *session, uint8_t *buf, uint16_t len)
{
    int ret, result = 0;
    printf("  < Read from server:");
    while (len > 0) {
        ret = mbedtls_ssl_read(&session->ssl, buf, len);
        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
            continue;
        }
        if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY || ret == 0) {
            break;
        }
        if (ret < 0) {
            printf("failed\n  ! mbedtls_ssl_read returned %d\n\n", ret);
            return ret;
        }
        buf += ret;
        len -= ret;
        result += ret;
    }
    printf(" %d bytes read\n\n", result);
    return result;
}

void mbedtls_session_close(mbedtls_session_t *session)
{
    mbedtls_ssl_close_notify(&session->ssl);
    mbedtls_session_cleanup(session);
}

int mbedtls_session_start(mbedtls_session_t *session,
        const unsigned char *cert, size_t cert_len,
        const char *host, const char *port)
{
    int ret;

    mbedtls_ctr_drbg_init(&session->ctr_drbg);
    mbedtls_entropy_init(&session->entropy);
    printf("\n  . Seeding the random number generator...");
    ret = mbedtls_ctr_drbg_seed(&session->ctr_drbg, mbedtls_entropy_func, &session->entropy, NULL, 0);
    if (ret != 0) {
        printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
        goto free_entropy_and_fail;
    }
    printf(" ok\n");

    printf("  . Loading the CA root certificate ...");
    mbedtls_x509_crt_init(&session->cacert);
    ret = mbedtls_x509_crt_parse(&session->cacert, cert, cert_len);
    if (ret < 0) {
        printf(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", (unsigned int)-ret);
        goto free_cert_and_fail;
    }
    printf(" ok (%d skipped)\n", ret);

    printf("  . Connecting to tcp/%s/%s...", host, port);
    mbedtls_net_init(&session->server_fd);
    if ((ret = mbedtls_net_connect(&session->server_fd, host, port, MBEDTLS_NET_PROTO_TCP)) != 0) {
        printf(" failed\n  ! mbedtls_net_connect returned %d\n\n", ret);
        goto free_server_fd_and_fail;
    }
    printf(" ok\n");

    printf("  . Setting up the SSL/TLS structure...");
    mbedtls_ssl_config_init(&session->conf);
    if ((ret = mbedtls_ssl_config_defaults(&session->conf,
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        printf(" failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret);
        goto free_conf_and_fail;
    }
    printf(" ok\n");

    mbedtls_ssl_conf_authmode(&session->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&session->conf, &session->cacert, NULL);
    mbedtls_ssl_conf_rng(&session->conf, mbedtls_ctr_drbg_random, &session->ctr_drbg);
    mbedtls_ssl_conf_dbg(&session->conf, __mbedtls_debug, stdout);

    mbedtls_ssl_init(&session->ssl);
    ret = mbedtls_ssl_setup(&session->ssl, &session->conf);
    if (ret != 0) {
        printf(" failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret);
        goto free_ssl_and_fail;
    }
    ret = mbedtls_ssl_set_hostname(&session->ssl, host);
    if (ret != 0) {
        printf(" failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret);
        goto free_ssl_and_fail;
    }
    mbedtls_ssl_set_bio(&session->ssl, &session->server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    printf("  . Performing the SSL/TLS handshake...");
    while ((ret = mbedtls_ssl_handshake(&session->ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            printf(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", (unsigned int)-ret);
            goto free_ssl_and_fail;
        }
    }
    printf(" ok\n");

    printf("  . Verifying peer X.509 certificate...");
    {
        uint32_t flags = mbedtls_ssl_get_verify_result(&session->ssl);
        if (flags != 0) {
            char vrfy_buf[512];

            printf(" failed\n");
            mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);
            printf("%s\n", vrfy_buf);
        } else {
            printf(" ok\n");
        }
    }

    return 0;

free_ssl_and_fail:
    mbedtls_ssl_free(&session->ssl);
free_conf_and_fail:
    mbedtls_ssl_config_free(&session->conf);
free_server_fd_and_fail:
    mbedtls_net_free(&session->server_fd);
free_cert_and_fail:
    mbedtls_x509_crt_free(&session->cacert);
free_entropy_and_fail:
    mbedtls_ctr_drbg_free(&session->ctr_drbg);
    mbedtls_entropy_free(&session->entropy);
    return -1;
}

WEAK(strnchr)
char * strnchr(const char *s, size_t n, int c)
{
    do {
        if (*s == c) {
            return (char*)s;
        }
    } while (--n && *s++);
    return NULL;
}

typedef struct mbedtls_https_ctx_t {
    mbedtls_session_t session;
    int resp_status;
    int content_length;
} mbedtls_https_ctx_t;

void mbedtls_https_close(mbedtls_https_ctx_t *https)
{
    mbedtls_session_close(&https->session);
}

int mbedtls_https_start(mbedtls_https_ctx_t *https, const char *verb, char *path, const char *fmt)
{
    mbedtls_session_t *session = &https->session;

    int result = mbedtls_session_start(session,
                    (const unsigned char *)mbedtls_test_cas_pem, mbedtls_test_cas_pem_len,
                    REPO_HOST_NAME, REPO_HOST_PORT);
    if (result != 0) {
        return result;
    }

    result = sprintf((char *)session->buffer, "%s %s HTTP/1.1\r\nUser-Agent: %s\r\nAccept: */*\r\n\r\n", verb, path, "vsf");
    result = mbedtls_session_write(session, session->buffer, result);
    if (result < 0) {
        return result;
    }

read_more:
    if (session->cur_size >= sizeof(session->buffer)) {
    failed:
        mbedtls_https_close(https);
        return -1;
    }
    result = mbedtls_session_read(session, session->buffer + session->cur_size, sizeof(session->buffer) - session->cur_size);
    if (result < 0) {
        return result;
    }
    session->cur_size = result + session->cur_size;
    session->cur_buffer = session->buffer;

    char *tmp, *line;
    while (session->cur_size > 0) {
        tmp = strnchr((const char *)session->cur_buffer, session->cur_size, '\n');
        if (NULL == tmp) {
            memcpy(session->buffer, session->cur_buffer, session->cur_size);
            goto read_more;
        }
        line = (char *)session->cur_buffer;
        *tmp++ = '\0';
        session->cur_size -= tmp - line;
        session->cur_buffer = (uint8_t *)tmp;

        if (strstr(line, "HTTP/1.") != NULL) {
            line += sizeof("HTTP/1.x ") - 1;
            https->resp_status = atoi(line);
            if (https->resp_status != 200) {
                goto failed;
            }
            continue;
        }
        if (strstr(line, "Content-Length:")) {
            line += sizeof("Content-Length:") - 1;
            while (*line && isspace(*line)) { line++; }

            https->content_length = atoi(line);
            continue;
        }
        if (*line == '\0' || *line == '\r') {
            break;
        }
    }
    return 0;
}

int mbedtls_https_read(mbedtls_https_ctx_t *https, uint8_t *buf, uint16_t len)
{
    mbedtls_session_t *session = &https->session;
    int result = 0;
again:
    if (session->cur_size > 0) {
        uint16_t cur_size = vsf_min(len, session->cur_size);
        memcpy(buf, session->cur_buffer, cur_size);
        len -= cur_size;
        buf += cur_size;
        session->cur_size -= cur_size;
        session->cur_buffer += cur_size;
        result += cur_size;
    }
    if (len > 0) {
        int rxlen = mbedtls_session_read(session, session->buffer, sizeof(session->buffer));
        if (rxlen < 0) {
            return result;
        }
        session->cur_size = rxlen;
        session->cur_buffer = session->buffer;
        goto again;
    }
    return result;
}

static int __vpm_install_packages(char *argv[])
{
    return 0;
}

static int __vpm_remove_packages(char *argv[])
{
    printf("not supported yet\n");
    return -1;
}

static int __vpm_upgrade_packages(void)
{
    printf("not supported yet\n");
    return -1;
}

static int __vpm_list_local_packages(void)
{
#define APP_MSCBOOT_CFG_ROMFS_FLASH_ADDR        (APP_MSCBOOT_CFG_FLASH_ADDR + APP_MSCBOOT_CFG_ROMFS_ADDR)
    vk_romfs_header_t *image = (vk_romfs_header_t *)APP_MSCBOOT_CFG_ROMFS_FLASH_ADDR;
    while ( (image != NULL) && vsf_romfs_is_image_valid(image)
        &&  ((uintptr_t)image - APP_MSCBOOT_CFG_ROMFS_FLASH_ADDR < APP_MSCBOOT_CFG_ROMFS_SIZE)) {
        printf("%s\n", image->name);
        image = vsf_romfs_chain_get_next(image);
    }
    return 0;
}

static int __vpm_list_remote_packages(void)
{
    return 0;
}

int __vpm_main(int argc, char *argv[])
{
    int result = 0;
    if (argc < 2) {
    show_help:
        printf("vpm(VSF.Linux Package Manager)\n");
        printf("Usage: %s command\n\n", argv[0]);
        printf("\
vpm is commandline package manager for vsf.linux\n\n\
commands:\n\
  list-local - list local installed pakcages\n\
  list-remote - list remote available packages\n\
  install - install packages\n\
  remove - remove packages\n\
  upgrade - upgrade installed packages\n");
        return result;
    }

    if (!strcmp(argv[1], "list-local")) {
        return __vpm_list_local_packages();
    } else if (!strcmp(argv[1], "list-remote")) {
        return __vpm_list_remote_packages();
    } else if (!strcmp(argv[1], "install")) {
        return __vpm_install_packages(&argv[2]);
    } else if (!strcmp(argv[1], "remove")) {
        return __vpm_remove_packages(&argv[2]);
    } else if (!strcmp(argv[1], "remove")) {
        return __vpm_upgrade_packages();
    } else {
        printf("unsupported command: %s\n\n", argv[1]);
        result = -1;
        goto show_help;
    }
}

void vsf_linux_install_package_manager(void)
{
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/vpm", __vpm_main);
}
