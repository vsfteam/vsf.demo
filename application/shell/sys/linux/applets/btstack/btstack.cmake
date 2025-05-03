cmake_policy(SET CMP0079 NEW)

if(NOT VSF_BTSTACK_PATH)
    set(VSF_BTSTACK_PATH ${VSF_SRC_PATH}/component/3rd-party/btstack/raw)
endif()

vsf_add_sources(
    ./btstack_main.c
    ./btstack_scan_main.c

    ${VSF_SRC_PATH}/component/3rd-party/btstack/port/hci_dump_vsf.c

    ${VSF_BTSTACK_PATH}/src/classic/btstack_link_key_db_memory.c

    ${VSF_BTSTACK_PATH}/src/classic/hid_device.c
    ${VSF_BTSTACK_PATH}/src/classic/rfcomm.c
    ${VSF_BTSTACK_PATH}/src/classic/sdp_server.c
    ${VSF_BTSTACK_PATH}/src/classic/sdp_util.c
    ${VSF_BTSTACK_PATH}/src/classic/spp_server.c

    ${VSF_BTSTACK_PATH}/src/btstack_hid_parser.c
    ${VSF_BTSTACK_PATH}/src/ad_parser.c
    ${VSF_BTSTACK_PATH}/src/btstack_linked_list.c
    ${VSF_BTSTACK_PATH}/src/btstack_memory.c
    ${VSF_BTSTACK_PATH}/src/btstack_memory_pool.c
    ${VSF_BTSTACK_PATH}/src/btstack_run_loop.c
    ${VSF_BTSTACK_PATH}/src/btstack_util.c
    ${VSF_BTSTACK_PATH}/src/hci.c
    ${VSF_BTSTACK_PATH}/src/hci_cmd.c
    ${VSF_BTSTACK_PATH}/src/l2cap.c
    ${VSF_BTSTACK_PATH}/src/l2cap_signaling.c
)

vsf_add_compile_definitions(
    ${VSF_TARGET_DEFINITIONS}
    VSF_USE_BTSTACK=ENABLED
)
vsf_add_include_directories(
    ${VSF_COMMON_INCLUDE_DIRECTORIES}
    ${VSF_BTSTACK_PATH}/src
    ./
)
