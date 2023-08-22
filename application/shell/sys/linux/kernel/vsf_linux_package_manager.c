#define __VSF_ROMFS_CLASS_INHERIT__
#define __VSF_HW_FLASH_MAL_CLASS_INHERIT__

#include <unistd.h>

#if VSF_USE_MBEDTLS == ENABLED
#   include "component/3rd-party/mbedtls/extension/tls_session/mbedtls_tls_session.h"
#endif

#define REPO_HOST_NAME                      "gitee.com"
#define REPO_HOST_PORT                      "443"
#define REPO_PATH                           "/vsf-linux/MCULinux.repo/raw/main/"

#define __VPM_BUF_SIZE                      512
#define __VPM_HOST_PATH_SIZE                512

extern int app_config_read(const char *cfgname, char *cfgvalue, size_t valuelen);
extern int app_config_write(const char *cfgname, char *cfgvalue);
extern vk_cached_mal_t romfs_mal;

struct __vpm_t {
    vk_romfs_info_t *fsinfo;
    bool can_uninstall;
    bool can_install;
} static __vpm;

#if VSF_USE_MBEDTLS == ENABLED && defined(APP_MSCBOOT_CFG_FLASH)
static void __vpm_parse_host_path(char *buf, int bufsize, char **host, char **path)
{
    char *cur = buf;
    int curbufsize = bufsize, cursize;
    if (app_config_read("vpm-host", cur, curbufsize)) {
        cur = REPO_HOST_NAME;
        cursize = 0;
    } else {
        cursize = strlen(cur);
    }
    *host = cur;

    cur += cursize;
    curbufsize -= cursize;
    if (app_config_read("vpm-path", cur, curbufsize)) {
        cur = REPO_PATH;
    }
    *path = cur;
}
#endif

static int __vpm_install_package(char *package)
{
    vk_romfs_header_t *image = (vk_romfs_header_t *)__vpm.fsinfo->image;
    char *tmp;
    while ((image != NULL) && vsf_romfs_is_image_valid(image)) {
        tmp = strstr((const char *)image->name, package);
        if (tmp != NULL && tmp[strlen(package)] == ' ') {
            printf("%s already installed\n", package);
            return -1;
        }
        image = vsf_romfs_chain_get_next(__vpm.fsinfo, image, true);
    }
    if (NULL == image) {
        printf("not enough romfs space\n");
        return -1;
    }

#if VSF_USE_MBEDTLS == ENABLED && defined(APP_MSCBOOT_CFG_FLASH)
    vk_romfs_header_t header = { 0 };
    vsf_http_client_t *http = (vsf_http_client_t *)malloc(sizeof(vsf_http_client_t) +
                    sizeof(mbedtls_session_t) + __VPM_BUF_SIZE + __VPM_HOST_PATH_SIZE);
    if (NULL == http) {
        printf("failed to allocate http context\n");
        return -1;
    }
    mbedtls_session_t *session = (mbedtls_session_t *)&http[1];
    memset(session, 0, sizeof(*session));
    http->op = &vsf_mbedtls_http_op;
    http->param = session;
    vsf_http_client_init(http);
    vk_mal_init(&romfs_mal.use_as__vk_mal_t);

    uint8_t *buf = (uint8_t *)&session[1];
    char *host = (char *)buf + __VPM_BUF_SIZE, *path;
    __vpm_parse_host_path(host, __VPM_HOST_PATH_SIZE, &host, &path);

    uint64_t flash_addr = (uint64_t)image - (uint64_t)__vpm.fsinfo->image;
    int result = 0, rsize, remain;
    strcpy((char *)buf, path);
    strcat((char *)buf, VSF_BOARD_ARCH_STR "/romfs/");
    strcat((char *)buf, package);
    strcat((char *)buf, ".img");
    if (    (vsf_http_client_request(http, host, REPO_HOST_PORT, "GET", (char *)buf, NULL, 0) < 0)
        ||  (http->resp_status != 200)) {
        printf("failed to start http\n");
        result = -1;
        goto do_exit;
    }

    printf("installing %s:", package);
    remain = http->content_length;
    if (!remain) {
        remain = INT_MAX;
    }
    while (remain > 0) {
        rsize = vsf_min(__VPM_BUF_SIZE, remain);
        rsize = vsf_http_client_read(http, buf, rsize);
        if (rsize > 0) {
            if (0 == header.size) {
                header = *(vk_romfs_header_t *)buf;
                memset(buf, 0xFF, sizeof(header));
            }
            vk_mal_write(&romfs_mal.use_as__vk_mal_t, flash_addr, rsize, buf);
            flash_addr += rsize;
            printf("*");
        } else if (!rsize) {
            break;
        }
    }
    if (!http->content_length || !remain) {
        if (header.size != 0) {
            flash_addr = (uint64_t)image - (uint64_t)__vpm.fsinfo->image;
            vk_mal_write(&romfs_mal.use_as__vk_mal_t, flash_addr, sizeof(header), (uint8_t *)&header);
        }

        printf("success\n");
        result = 0;
    } else {
        printf("connection closed before all data received, remaining %d\n", remain);
        result = -1;
    }

do_exit:
    vsf_http_client_close(http);
    free(http);

    vk_mal_fini(&romfs_mal.use_as__vk_mal_t);
    return result;
#else
    printf("mbedtls is needed for the current command\n");
    return -1;
#endif
}

static int __vpm_install_packages(char *argv[])
{
    while (*argv != NULL) {
        __vpm_install_package(*argv++);
    }
    return 0;
}

static bool __vpm_is_to_uninstall(vk_romfs_header_t *image, char *argv[])
{
    while (*argv != NULL) {
        if ((strstr((const char *)image->name, *argv) != NULL) && (' ' == image->name[strlen(*argv)])) {
            return true;
        }
        argv++;
    }
    return false;
}

static int __vpm_uninstall_packages(char *argv[])
{
    vk_romfs_header_t *image = (vk_romfs_header_t *)__vpm.fsinfo->image;
    if (!vsf_romfs_is_image_valid(image)) {
        goto not_found;
    }

    while ( (image != NULL)
        &&  !__vpm_is_to_uninstall(image, argv)) {
        image = vsf_romfs_chain_get_next(__vpm.fsinfo, image, false);
    }
    if (NULL == image) {
    not_found:
        printf("not found\n");
        return 0;
    }

#if defined(APP_MSCBOOT_CFG_FLASH)
    vk_mal_init(&romfs_mal.use_as__vk_mal_t);

    uint64_t flash_addr = (uint64_t)image - (uint64_t)__vpm.fsinfo->image;
    uint32_t image_size;
    printf("uninstall %s\n", image->name);
    image = vsf_romfs_chain_get_next(__vpm.fsinfo, image, false);
    while (image != NULL) {
        if (!__vpm_is_to_uninstall(image, argv)) {
            image_size = be32_to_cpu(image->size);
            vk_mal_write(&romfs_mal.use_as__vk_mal_t, flash_addr, image_size, (uint8_t *)image);
            flash_addr += image_size;
        } else {
            printf("uninstall %s\n", image->name);
        }

        image = vsf_romfs_chain_get_next(__vpm.fsinfo, image, false);
    }

    vk_romfs_header_t header = { 0 };
    vk_mal_write(&romfs_mal.use_as__vk_mal_t, flash_addr, sizeof(header), (uint8_t *)&header);

    vk_mal_fini(&romfs_mal.use_as__vk_mal_t);
    return 0;
#else
    printf("flash operation is not enabled for the current command\n");
    return -1;
#endif
}

static int __vpm_list_local_packages(void)
{
    vk_romfs_header_t *image = (vk_romfs_header_t *)__vpm.fsinfo->image;
    while ( (image != NULL) && vsf_romfs_is_image_valid(image)
        &&  ((uint8_t *)image - (uint8_t *)__vpm.fsinfo->image < __vpm.fsinfo->image_size)) {
        printf("%s\n", image->name);
        image = vsf_romfs_chain_get_next(__vpm.fsinfo, image, false);
    }
    return 0;
}

static int __vpm_list_remote_packages(void)
{
#if VSF_USE_MBEDTLS == ENABLED
    vsf_http_client_t *http = (vsf_http_client_t *)malloc(sizeof(vsf_http_client_t) +
                    sizeof(mbedtls_session_t) + __VPM_BUF_SIZE + __VPM_HOST_PATH_SIZE);
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
    char *host = (char *)buf + __VPM_BUF_SIZE, *path;
    __vpm_parse_host_path(host, __VPM_HOST_PATH_SIZE, &host, &path);

    int result = 0, rsize, remain;
    strcpy((char *)buf, path);
    strcat((char *)buf, VSF_BOARD_ARCH_STR "/romfs/");
    strcat((char *)buf, "list.txt");
    if (    (vsf_http_client_request(http, host, REPO_HOST_PORT, "GET", (char *)buf, NULL, 0) < 0)
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
#else
    printf("mbedtls is needed for the current command\n");
    return -1;
#endif
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
  uninstall - uninstall packages\n\
  repo - set repo path\n");
        return result;
    }

    if (!strcmp(argv[1], "list-local")) {
        return __vpm_list_local_packages();
    } else if (!strcmp(argv[1], "list-remote")) {
        return __vpm_list_remote_packages();
    } else if (!strcmp(argv[1], "install")) {
        if (!__vpm.can_install) {
            printf("vpm: Can not install packages in current mode, Please reboot to LinuxBoot mode\n");
            return -1;
        }
        return __vpm_install_packages(&argv[2]);
    } else if (!strcmp(argv[1], "uninstall")) {
        if (!__vpm.can_uninstall) {
            printf("vpm: Can not uninstall packages in current mode, Please reboot to LinuxBoot mode\n");
            return -1;
        }
        return __vpm_uninstall_packages(&argv[2]);
    } else if (!strcmp(argv[1], "repo")) {
        if (2 == argc) {
        } else if (4 == argc) {
            app_config_write("vpm-host", argv[2]);
            app_config_write("vpm-path", argv[3]);
        } else {
            printf("Usage: %s repo HOST REPO_URL\n", argv[0]);
            printf("  eg: %s repo gitee.com /USER/REPO.repo/raw/main/\n", argv[0]);
            printf("  eg: %s repo raw.githubusercontent.com /USER/REPO/master/\n", argv[0]);
            return -1;
        }

        char buffer[256], *ptr;
        ptr = app_config_read("vpm-host", buffer, sizeof(buffer)) ? REPO_HOST_NAME : buffer;
        printf("host: %s\n", ptr);
        ptr = app_config_read("vpm-path", buffer, sizeof(buffer)) ? REPO_PATH : buffer;
        printf("path: %s\n", ptr);
        return 0;
    } else {
        printf("unsupported command: %s\n\n", argv[1]);
        result = -1;
        goto show_help;
    }
}

void vsf_linux_install_package_manager(vk_romfs_info_t *fsinfo, bool can_uninstall, bool can_install)
{
    __vpm.fsinfo = fsinfo;
    __vpm.can_uninstall = can_uninstall;
    __vpm.can_install = can_install;
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/vpm", __vpm_main);
}
