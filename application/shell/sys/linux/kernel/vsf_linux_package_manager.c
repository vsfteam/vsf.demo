#define __VSF_ROMFS_CLASS_INHERIT__
#define __VSF_FLASH_MAL_CLASS_INHERIT__

#include <vsf.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#if VSF_USE_MBEDTLS == ENABLED
#   include "component/3rd-party/mbedtls/extension/tls_session/mbedtls_tls_session.h"
#endif

#define REPO_HOST_NAME                      "gitee.com"
#define REPO_HOST_PORT                      "443"
#define REPO_PATH                           "/vsf-linux/MCULinux.repo/raw/main/"

#define __VPM_BUF_SIZE                      512
#define __VPM_HOST_PATH_SIZE                512

#if     VSF_HAL_USE_FLASH == ENABLED && defined(APP_MSCBOOT_CFG_FLASH)          \
    &&  defined(APP_MSCBOOT_CFG_ROMFS_SIZE) && defined(APP_MSCBOOT_CFG_ROMFS_ADDR)
#   define VPM_ROMFS_IS_FLASH_MAL
extern vk_cached_mal_t romfs_mal;
#endif

struct __vpm_t {
#ifdef VPM_ROMFS_IS_FLASH_MAL
    vk_romfs_info_t *fsinfo;
#else
    const char *package_path;
#endif
    bool can_uninstall;
    bool can_install;
} static __vpm;

#if VSF_USE_MBEDTLS == ENABLED
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

VSF_CAL_WEAK(vpm_on_installed)
void vpm_on_installed(void)
{
}

static int __vpm_install_package(char *package)
{
#ifdef VPM_ROMFS_IS_FLASH_MAL
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
#endif

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

    int result = 0, rsize, remain;
    uint32_t checksum = 0, totalsize = 0;
    uint8_t *buf = (uint8_t *)&session[1];
    char *host = (char *)buf + __VPM_BUF_SIZE, *path;
    __vpm_parse_host_path(host, __VPM_HOST_PATH_SIZE, &host, &path);

#   ifdef VPM_ROMFS_IS_FLASH_MAL
    vk_romfs_header_t header = { 0 };
    vk_mal_init(&romfs_mal.use_as__vk_mal_t);

    uint64_t flash_addr = (uint64_t)(uintptr_t)image - (uint64_t)(uintptr_t)__vpm.fsinfo->image;
    strcpy((char *)buf, path);
    strcat((char *)buf, VSF_BOARD_ARCH_STR "/" VSF_BOARD_ARCH_APP_FORMAT "/");
    strcat((char *)buf, package);
    strcat((char *)buf, ".img");
#   else
    int fd_root = open(__vpm.package_path, 0);
    if (fd_root < 0) {
        printf("failed to open %s to download package\n", __vpm.package_path);
        free(http);
        return -1;
    }
    char package_name[strlen(package) + 5];
    char install_cmdline[8 + strlen(__vpm.package_path) + strlen(package_name) + 2];
    strcpy(package_name, package);
    strcat(package_name, ".deb");
    int fd_package = openat(fd_root, package_name, O_RDWR | O_CREAT | O_TRUNC);
    close(fd_root);
    if (fd_package < 0) {
        printf("failed to create %s\n", package_name);
        free(http);
        return -1;
    }

    strcpy((char *)buf, path);
    strcat((char *)buf, VSF_BOARD_ARCH_STR "/" VSF_BOARD_ARCH_APP_FORMAT "/");
    strcat((char *)buf, package_name);
#   endif

    int pos;
    result = vsf_http_client_request(http, &(vsf_http_client_req_t){
        .host       = host,
        .port       = REPO_HOST_PORT,
        .verb       = "GET",
        .path       = (char *)buf,
    });
    if ((result < 0) || (http->resp_status != 200)) {
        printf("failed to start http with response %d\n", http->resp_status);
    failure:
        result = -1;
        goto do_exit;
    }

    remain = http->content_length;
    if (!remain) {
        remain = INT_MAX;
        printf("Warning: Content-Length not found in Http response header, so it may fail even if success. \
Uninstall and install %s again if fail to run\n", package);
    }

#   ifdef VPM_ROMFS_IS_FLASH_MAL
    printf("installing %s:", package);
    pos = 12;
#   else
    printf("downloading %s:", package);
    pos = 13;
#   endif
    pos += strlen(package);
    while (remain > 0) {
        rsize = vsf_min(__VPM_BUF_SIZE, remain);
        rsize = vsf_http_client_read(http, buf, rsize);
        if (rsize > 0) {
            remain -= rsize;
            totalsize += rsize;
            for (int i = 0; i < rsize; i++) {
                checksum += buf[i];
            }
#   ifdef VPM_ROMFS_IS_FLASH_MAL
            if (0 == header.size) {
                header = *(vk_romfs_header_t *)buf;
                memset(buf, 0xFF, sizeof(header));
            }
            if (flash_addr >= romfs_mal.host_mal->size) {
            not_enough_flash:
                printf("not enough flash to install %s\n", package);
                goto failure;
            }
            vk_mal_write(&romfs_mal.use_as__vk_mal_t, flash_addr, rsize, buf);
            flash_addr += rsize;
#   else
            write(fd_package, buf, rsize);
#   endif
            printf("*");
            if (++pos >= 80) {
                pos = 0;
                printf("\n");
            }
        } else if (!rsize) {
            break;
        }
    }
#   ifdef VPM_ROMFS_IS_FLASH_MAL
    if (totalsize >= be32_to_cpu(header.size)) {
        if (header.size != 0) {
            vk_romfs_header_t header_zero = { 0 };
            flash_addr = (flash_addr + __vpm.fsinfo->alignment - 1) & ~(__vpm.fsinfo->alignment - 1);
            if (flash_addr >= romfs_mal.host_mal->size) {
                goto not_enough_flash;
            }
            vk_mal_write(&romfs_mal.use_as__vk_mal_t, flash_addr, sizeof(header_zero), (uint8_t *)&header_zero);

            flash_addr = (uint64_t)(uintptr_t)image - (uint64_t)(uintptr_t)__vpm.fsinfo->image;
            if (flash_addr >= romfs_mal.host_mal->size) {
                goto not_enough_flash;
            }
            vk_mal_write(&romfs_mal.use_as__vk_mal_t, flash_addr, sizeof(header), (uint8_t *)&header);
        }

        printf("\nsuccess with checksum %08X for %d bytes\n", checksum, totalsize);
        result = 0;
    } else {
        printf("\nconnection closed before all data received, remaining %d\n", remain);
        result = -1;
    }
#   else
    printf("\nsuccess with checksum %08X for %d bytes\nInstalling %s with dpkg\n", checksum, totalsize, package);
    strcpy(install_cmdline, "dpkg -i ");
    strcat(install_cmdline, __vpm.package_path);
    strcat(install_cmdline, "/");
    strcat(install_cmdline, package_name);
    system(install_cmdline);
    result = 0;
#   endif

do_exit:
    vsf_http_client_close(http);
    free(http);

#   ifdef VPM_ROMFS_IS_FLASH_MAL
    vk_mal_fini(&romfs_mal.use_as__vk_mal_t);
#   else
    if (fd_package >= 0) {
        close(fd_package);
    }
#   endif
    return result;
#else
    printf("mbedtls is needed for the current command\n");
    return -1;
#endif
}

static int __vpm_install_packages(char *argv[])
{
    int num_installed = 0;
    while (*argv != NULL) {
        if (!__vpm_install_package(*argv++)) {
            num_installed++;
        }
    }
    return num_installed;
}

#ifdef VPM_ROMFS_IS_FLASH_MAL
static int __vpm_install_local_package(char *path)
{
    uint8_t *buffer = NULL;
    FILE *f = NULL;
    long fsize, remain;
    size_t cursize;
    vk_romfs_header_t *image;
    int result = -1;
    uint64_t flash_addr;
    char *package, *version;
    vk_romfs_header_t header;
    uint32_t checksum = 0, totalsize = 0;

    f = fopen(path, "rb");
    if (NULL == f) {
        printf("failed to open %s\n", path);
        return -1;
    }
    fseek(f, 0, SEEK_END);
    fsize = remain = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (fsize < sizeof(*image)) {
        printf("invalid romfs image %s\n", path);
        goto do_exit_close_file;
    }

    buffer = malloc(4096);
    if (NULL == buffer) {
        printf("failed to allocate buffer\n");
        goto do_exit_close_file;
    }

    cursize = vsf_min(4096, remain);
    if (cursize != fread(buffer, 1, cursize, f)) {
        printf("failed to read %s\n", path);
        goto do_exit_free_buffer;
    }
    remain -= cursize;
    totalsize += cursize;
    for (int i = 0; i < cursize; i++) {
        checksum += buffer[i];
    }

    image = (vk_romfs_header_t *)buffer;
    if (!vsf_romfs_is_image_valid(image)) {
        printf("invalid package %s\n", path);
        goto do_exit_free_buffer;
    }

    package = strdup((const char *)image->name);
    if (NULL == package) {
        printf("fail to allocate name for package %s\n", image->name);
        goto do_exit_free_buffer;
    }
    version = strchr(package, ' ');
    if (version != NULL) {
        *version = '\0';
    }

    header = *(vk_romfs_header_t *)buffer;
    memset(buffer, 0xFF, sizeof(header));

    image = (vk_romfs_header_t *)__vpm.fsinfo->image;
    char *tmp;
    while ((image != NULL) && vsf_romfs_is_image_valid(image)) {
        tmp = strstr((const char *)image->name, package);
        if (tmp != NULL && tmp[strlen(package)] == ' ') {
            printf("%s already installed\n", package);
            goto do_exit_free_package;
        }
        image = vsf_romfs_chain_get_next(__vpm.fsinfo, image, true);
    }
    if (NULL == image) {
        printf("not enough romfs space\n");
        goto do_exit_free_package;
    }

    printf("installing %s:", package);
    vk_mal_init(&romfs_mal.use_as__vk_mal_t);
    flash_addr = (uintptr_t)image - (uintptr_t)__vpm.fsinfo->image;
    vk_mal_write(&romfs_mal.use_as__vk_mal_t, flash_addr, cursize, buffer);
    flash_addr += cursize;
    while (remain > 0) {
        cursize = vsf_min(4096, remain);
        if (cursize != fread(buffer, 1, cursize, f)) {
            printf("failed to read %s\n", path);
            goto do_exit_mal_fini;
        }
        remain -= cursize;
        totalsize += cursize;
        for (int i = 0; i < cursize; i++) {
            checksum += buffer[i];
        }

        vk_mal_write(&romfs_mal.use_as__vk_mal_t, flash_addr, cursize, buffer);
        flash_addr += cursize;
        printf("*");
    }

    if (header.size != 0) {
        vk_romfs_header_t header_zero = { 0 };
        flash_addr = (flash_addr + __vpm.fsinfo->alignment - 1) & ~(__vpm.fsinfo->alignment - 1);
        vk_mal_write(&romfs_mal.use_as__vk_mal_t, flash_addr, sizeof(header_zero), (uint8_t *)&header_zero);

        flash_addr = (uintptr_t)image - (uintptr_t)__vpm.fsinfo->image;
        vk_mal_write(&romfs_mal.use_as__vk_mal_t, flash_addr, sizeof(header), (uint8_t *)&header);
    }

    printf("\nsuccess with checksum %08X for %d bytes\n", checksum, totalsize);
    result = 0;

do_exit_mal_fini:
    vk_mal_fini(&romfs_mal.use_as__vk_mal_t);
do_exit_free_package:
    free(package);
do_exit_free_buffer:
    free(buffer);
do_exit_close_file:
    fclose(f);

    return result;
}

static int __vpm_install_local_packages(char *argv[])
{
    int num_installed = 0;
    while (*argv != NULL) {
        if (!__vpm_install_local_package(*argv++)) {
            num_installed++;
        }
    }
    return num_installed;
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

    vk_mal_init(&romfs_mal.use_as__vk_mal_t);

    uint64_t flash_addr = (uintptr_t)image - (uintptr_t)__vpm.fsinfo->image;
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
    flash_addr = (flash_addr + __vpm.fsinfo->alignment - 1) & ~(__vpm.fsinfo->alignment - 1);
    vk_mal_write(&romfs_mal.use_as__vk_mal_t, flash_addr, sizeof(header), (uint8_t *)&header);

    vk_mal_fini(&romfs_mal.use_as__vk_mal_t);
    return 0;
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
#else
static int __vpm_list_local_packages(void)
{
    DIR *dirp = opendir(__vpm.package_path);
    if (dirp != NULL) {
        struct dirent *entry;
        char *ext;
        while ((entry = readdir(dirp)) != NULL) {
            if (    (entry->d_type == DT_REG)
                &&  (ext = strrchr(entry->d_name, '.'))
                &&  !strcmp(ext, ".deb")) {

                *ext = '\0';
                printf("%s\n", entry->d_name);
                *ext = '.';
            }
        }
    }
    return 0;
}
#endif

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
    strcat((char *)buf, VSF_BOARD_ARCH_STR "/" VSF_BOARD_ARCH_APP_FORMAT "/");
    strcat((char *)buf, "list.txt");

    result = vsf_http_client_request(http, &(vsf_http_client_req_t){
        .host       = host,
        .port       = REPO_HOST_PORT,
        .verb       = "GET",
        .path       = (char *)buf,
    });
    if ((result < 0) || (http->resp_status != 200)) {
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
  install-local - install packages in local storage\n\
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
        if (__vpm_install_packages(&argv[2]) > 0) {
            vpm_on_installed();
            return 0;
        }
        return -1;
#ifdef VPM_ROMFS_IS_FLASH_MAL
    } else if (!strcmp(argv[1], "install-local")) {
        if (!__vpm.can_install) {
            printf("vpm: Can not install packages in current mode, Please reboot to LinuxBoot mode\n");
            return -1;
        }
        if (__vpm_install_local_packages(&argv[2]) > 0) {
            vpm_on_installed();
            return 0;
        }
        return -1;
    } else if (!strcmp(argv[1], "uninstall")) {
        if (!__vpm.can_uninstall) {
            printf("vpm: Can not uninstall packages in current mode, Please reboot to LinuxBoot mode\n");
            return -1;
        }
        return __vpm_uninstall_packages(&argv[2]);
#endif
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

void vsf_linux_install_package_manager(
#ifdef VPM_ROMFS_IS_FLASH_MAL
    vk_romfs_info_t *fsinfo,
#else
    const char *package_path,
#endif
    bool can_uninstall, bool can_install)
{
#ifdef VPM_ROMFS_IS_FLASH_MAL
    __vpm.fsinfo = fsinfo;
#else
    __vpm.package_path = package_path;
#endif
    __vpm.can_uninstall = can_uninstall;
    __vpm.can_install = can_install;
    vsf_linux_fs_bind_executable(VSF_LINUX_CFG_BIN_PATH "/vpm", __vpm_main);
}
