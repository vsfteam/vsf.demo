#define __VSF_ROMFS_CLASS_INHERIT__

#include <unistd.h>

#define APP_MSCBOOT_CFG_ROMFS_FLASH_ADDR    (APP_MSCBOOT_CFG_FLASH_ADDR + APP_MSCBOOT_CFG_ROMFS_ADDR)

#define REPO_HOST_NAME                      "gitee.com"
#define REPO_HOST_PORT                      "443"
#define REPO_PATH                           "/vsf-linux/MCULinux.repo/raw/main/" VSF_BOARD_ARCH_STR "/romfs/"

#define __VPM_BUF_SIZE                      512

static int __vpm_install_package(char *package)
{
    vk_romfs_header_t *image = (vk_romfs_header_t *)APP_MSCBOOT_CFG_ROMFS_FLASH_ADDR, header = { 0 };
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

    vsf_http_client_t *http = (vsf_http_client_t *)malloc(sizeof(vsf_http_client_t) + sizeof(mbedtls_session_t) + __VPM_BUF_SIZE + APP_MSCBOOT_CFG_ERASE_BLOCK_SIZE);
    if (NULL == http) {
        printf("failed to allocate http context\n");
        return -1;
    }
    mbedtls_session_t *session = (mbedtls_session_t *)&http[1];
    memset(session, 0, sizeof(*session));
    http->op = &vsf_mbedtls_http_op;
    http->param = session;
    vsf_http_client_init(http);

    uint8_t *buf = (uint8_t *)&session[1], *cache = buf + __VPM_BUF_SIZE;
    uint8_t *curbuf, *curptr_flash = NULL, *curptr_cache;
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

    printf("installing %s:", package);
    remain = http->content_length;
    if (!remain) {
        remain = INT_MAX;
    }
    while (remain > 0) {
        rsize = vsf_min(__VPM_BUF_SIZE, remain);
        rsize = vsf_http_client_read(http, buf, rsize);
        if (rsize > 0) {
            curbuf = buf;
            if (NULL == curptr_flash) {
                header = *(vk_romfs_header_t *)buf;
                if (!vsf_romfs_is_image_valid(&header) || (rsize <= sizeof(header))) {
                    printf("invalid romfs image\n");
                    goto do_exit;
                }

                if ((uintptr_t)image == APP_MSCBOOT_CFG_ROMFS_FLASH_ADDR) {
                    curptr_flash = (uint8_t *)image;
                    remain = be32_to_cpu(header.size);
                    header.size = 0;
                } else {
                    rsize -= sizeof(header);
                    curbuf = buf + sizeof(header);
                    curptr_flash = (uint8_t *)&image[1];
                    remain = be32_to_cpu(header.size) - sizeof(header);
                }

                uint8_t *curptr_flash_aligned = (uint8_t *)((uintptr_t)curptr_flash & ~(APP_MSCBOOT_CFG_ERASE_ALIGN - 1));
                memcpy(cache, curptr_flash_aligned, APP_MSCBOOT_CFG_ERASE_BLOCK_SIZE);
                curptr_cache = &cache[curptr_flash - curptr_flash_aligned];
            }

            // write curbuf:rsize to cached curptr_flash
            while (rsize > 0) {
                result = curptr_cache - cache;
                if (result < APP_MSCBOOT_CFG_ERASE_BLOCK_SIZE) {
                    result = APP_MSCBOOT_CFG_ERASE_BLOCK_SIZE - result;
                    result = vsf_min(result, rsize);
                    memcpy(curptr_cache, curbuf, result);
                    curptr_flash += result;
                    curptr_cache += result;
                    curbuf += result;
                    rsize -= result;
                }
                result = curptr_cache - cache;
                if (result >= APP_MSCBOOT_CFG_ERASE_BLOCK_SIZE) {
                    curptr_flash -= APP_MSCBOOT_CFG_FLASH_ADDR + APP_MSCBOOT_CFG_ERASE_BLOCK_SIZE;
                    vsf_flash_erase(&APP_MSCBOOT_CFG_FLASH, (uintptr_t)curptr_flash, APP_MSCBOOT_CFG_ERASE_BLOCK_SIZE);
                    vsf_flash_write(&APP_MSCBOOT_CFG_FLASH, (uintptr_t)curptr_flash, cache, APP_MSCBOOT_CFG_ERASE_BLOCK_SIZE);
                    curptr_flash += APP_MSCBOOT_CFG_FLASH_ADDR + APP_MSCBOOT_CFG_ERASE_BLOCK_SIZE;
                    curptr_cache = cache;
                    printf("*");
                }
            }
        } else if (!rsize) {
            break;
        }
        remain -= rsize;
    }
    if (!http->content_length || !remain) {
        curptr_flash = (uint8_t *)((uintptr_t)curptr_flash & ~(APP_MSCBOOT_CFG_ERASE_ALIGN - 1));
        curptr_flash -= APP_MSCBOOT_CFG_FLASH_ADDR;
        vsf_flash_erase(&APP_MSCBOOT_CFG_FLASH, (uintptr_t)curptr_flash, APP_MSCBOOT_CFG_ERASE_BLOCK_SIZE);
        if (curptr_cache != cache) {
            *(uint32_t *)curptr_cache = 0xFFFFFFFF;
            vsf_flash_write(&APP_MSCBOOT_CFG_FLASH, (uintptr_t)curptr_flash, cache, APP_MSCBOOT_CFG_ERASE_BLOCK_SIZE);
        }

        if (header.size != 0) {
            curptr_flash = (uint8_t *)image;
            curptr_flash = (uint8_t *)((uintptr_t)curptr_flash & ~(APP_MSCBOOT_CFG_ERASE_ALIGN - 1));
            memcpy(cache, curptr_flash, APP_MSCBOOT_CFG_ERASE_BLOCK_SIZE);
            memcpy(&cache[(uint8_t *)image - curptr_flash], &header, sizeof(header));
            vsf_flash_erase(&APP_MSCBOOT_CFG_FLASH, (uintptr_t)curptr_flash, APP_MSCBOOT_CFG_ERASE_BLOCK_SIZE);
            vsf_flash_write(&APP_MSCBOOT_CFG_FLASH, (uintptr_t)curptr_flash, cache, APP_MSCBOOT_CFG_ERASE_BLOCK_SIZE);
        }

        printf("success\r\n");
        result = 0;
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
  remove - remove packages\n");
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
