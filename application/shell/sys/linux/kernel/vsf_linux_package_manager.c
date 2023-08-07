#include <unistd.h>

int __vpm_install_packages(char *argv[])
{
    return 0;
}

int __vpm_remove_packages(char *argv[])
{
    return 0;
}

int __vpm_list_local_packages(void)
{
    return 0;
}

int __vpm_list_remote_packages(void)
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
  list-remote - list remote availabed packages\n\
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
