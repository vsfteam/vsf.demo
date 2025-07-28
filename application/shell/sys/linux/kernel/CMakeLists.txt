vsf_add_include_directories(
    ./
)
vsf_add_sources(
    hwtest_main.c
    user_httpd.c
    user_mdns.c
    user_qrcode.c
    vsf_linux_package_manager.c
    vsf_main.c
)

if(VSF_USE_BTSTACK)
    vsf_add_include_directories(
        ./config/btstack
    )
endif()

if(VSF_USE_LWIP AND NOT VSF_LWIP_VENDOR)
    vsf_add_include_directories(
        ./config/lwip
    )
endif()