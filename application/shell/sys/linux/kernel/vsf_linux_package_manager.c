#define __VSF_ROMFS_CLASS_INHERIT__

#include <unistd.h>

#define APP_MSCBOOT_CFG_ROMFS_FLASH_ADDR    (APP_MSCBOOT_CFG_FLASH_ADDR + APP_MSCBOOT_CFG_ROMFS_ADDR)

#define REPO_HOST_NAME                      "gitee.com"
#define REPO_HOST_PORT                      "443"
#define REPO_PATH                           "/vsf-linux/MCULinux.repo/raw/main/" VSF_BOARD_ARCH_STR "/romfs/"

#define __VPM_BUF_SIZE                      512

static int __vpm_install_package(char *package)
{
    vk_romfs_header_t *image = (vk_romfs_header_t *)APP_MSCBOOT_CFG_ROMFS_FLASH_ADDR;
    char *tmp;
    while ( (image != NULL) && vsf_romfs_is_image_valid(image)
        &&  ((uintptr_t)image - APP_MSCBOOT_CFG_ROMFS_FLASH_ADDR < APP_MSCBOOT_CFG_ROMFS_SIZE)) {
        tmp = strstr((const char *)image->name, package);
        if (tmp != NULL && tmp[strlen(package)] == ' ') {
            printf("%s already installed\n", package);
            return -1;
        }
        image = vsf_romfs_chain_get_next(image);
    }

    vsf_http_client_t *http = (vsf_http_client_t *)malloc(sizeof(vsf_http_client_t) + sizeof(mbedtls_session_t) + __VPM_BUF_SIZE);
    if (NULL == http) {
        printf("failed to allocate http context\n");
        return -1;
    }
    mbedtls_session_t *session = (mbedtls_session_t *)&http[1];
    memset(session, 0, sizeof(*session));
    http->op = &vsf_mbedtls_http_op;
    http->param = session;
    vsf_http_client_init(http);

    uint8_t *buf = (uint8_t *)&session[1];
    int result = 0, rsize, remain;
    strcpy((char *)buf, REPO_PATH);
    strcat((char *)buf, package);
    strcat((char *)buf, ".img");
    if (    (vsf_http_client_request(http, REPO_HOST_NAME, REPO_HOST_PORT, "GET", (char *)buf, NULL, 0) < 0)
        ||  (http->resp_status != 200)) {
        printf("failed to start http\n");
        result = -1;
        goto do_exit;
    }

    remain = http->content_length;
    if (!remain) {
        remain = INT_MAX;
    }
    while (remain > 0) {
        rsize = vsf_min(__VPM_BUF_SIZE, remain);
        rsize = vsf_http_client_read(http, buf, rsize);
        if (rsize > 0) {
//            write(STDOUT_FILENO, buf, rsize);
        } else if (!rsize) {
            break;
        }
        remain -= rsize;
    }
    if (!http->content_length || !remain) {
        // success
    } else {
        printf("connection closed before all data received, remaining %d\n", remain);
        result = -1;
    }

do_exit:
    vsf_http_client_close(http);
    free(http);
    return result;
}

static int __vpm_install_packages(char *argv[])
{
    while (*argv != NULL) {
        __vpm_install_package(*argv++);
    }
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
    vsf_http_client_t *http = (vsf_http_client_t *)malloc(sizeof(vsf_http_client_t) + sizeof(mbedtls_session_t) + __VPM_BUF_SIZE);
    if (NULL == http) {
        printf("failed to allocate http context\n");
        return -1;
    }
    mbedtls_session_t *session = (mbedtls_session_t *)&http[1];
    memset(session, 0, sizeof(*session));
    http->op = &vsf_mbedtls_http_op;
    http->param = session;
    vsf_http_client_init(http);

    uint8_t *buf = (uint8_t *)&session[1];
    int result = 0, rsize, remain;
    strcpy((char *)buf, REPO_PATH);
    strcat((char *)buf, "list.txt");
    if (    (vsf_http_client_request(http, REPO_HOST_NAME, REPO_HOST_PORT, "GET", (char *)buf, NULL, 0) < 0)
        ||  (http->resp_status != 200)) {
        printf("failed to start http\n");
        result = -1;
        goto do_exit;
    }

    remain = http->content_length;
    if (!remain) {
        remain = INT_MAX;
    }
    while (remain > 0) {
        rsize = vsf_min(__VPM_BUF_SIZE, remain);
        rsize = vsf_http_client_read(http, buf, rsize);
        if (rsize > 0) {
            write(STDOUT_FILENO, buf, rsize);
        } else if (!rsize) {
            break;
        }
        remain -= rsize;
    }

do_exit:
    vsf_http_client_close(http);
    free(http);
    return result;
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
