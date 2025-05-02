# CMakeLists head

cmake_policy(SET CMP0079 NEW)

if(NOT VSF_USE_MBEDTLS)
    message(FATAL_ERROR "VSF_USE_MBEDTLS MUST be set because micropython MICROPY_SSL_MBEDTLS depends on mbedtls")
endif()

set(MICROPYTHON_PATH ${MICROPYTHON_APPLET_PATH}/raw)
file(GLOB MPY_CORE_FILES ${MICROPYTHON_PATH}/py/*.c)
file(GLOB MPY_SOURCES
    ${MICROPYTHON_PATH}/ports/unix/alloc.c
    ${MICROPYTHON_PATH}/ports/unix/coverage.c
    ${MICROPYTHON_PATH}/ports/unix/gccollect.c
    ${MICROPYTHON_PATH}/ports/unix/input.c
    ${MICROPYTHON_PATH}/ports/unix/main.c
    ${MICROPYTHON_PATH}/ports/unix/modmachine.c
    ${MICROPYTHON_PATH}/ports/unix/modsocket.c
    ${MICROPYTHON_PATH}/ports/unix/modtermios.c
    ${MICROPYTHON_PATH}/ports/unix/mpthreadport.c
    ${MICROPYTHON_PATH}/ports/unix/unix_mphal.c

    ${MICROPYTHON_PATH}/extmod/machine_bitstream.c
    ${MICROPYTHON_PATH}/extmod/machine_i2c.c
    ${MICROPYTHON_PATH}/extmod/machine_mem.c
    ${MICROPYTHON_PATH}/extmod/machine_pinbase.c
    ${MICROPYTHON_PATH}/extmod/machine_pulse.c
    ${MICROPYTHON_PATH}/extmod/machine_pwm.c
    ${MICROPYTHON_PATH}/extmod/machine_signal.c
    ${MICROPYTHON_PATH}/extmod/machine_spi.c
    ${MICROPYTHON_PATH}/extmod/machine_timer.c
    ${MICROPYTHON_PATH}/extmod/modasyncio.c
    ${MICROPYTHON_PATH}/extmod/modbinascii.c
    ${MICROPYTHON_PATH}/extmod/modcryptolib.c
    ${MICROPYTHON_PATH}/extmod/moddeflate.c
    ${MICROPYTHON_PATH}/extmod/modframebuf.c
    ${MICROPYTHON_PATH}/extmod/modhashlib.c
    ${MICROPYTHON_PATH}/extmod/modheapq.c
    ${MICROPYTHON_PATH}/extmod/modjson.c
    ${MICROPYTHON_PATH}/extmod/modmachine.c
    ${MICROPYTHON_PATH}/extmod/modnetwork.c
    ${MICROPYTHON_PATH}/extmod/modos.c
    ${MICROPYTHON_PATH}/extmod/modplatform.c
    ${MICROPYTHON_PATH}/extmod/modrandom.c
    ${MICROPYTHON_PATH}/extmod/modre.c
    ${MICROPYTHON_PATH}/extmod/modselect.c
    ${MICROPYTHON_PATH}/extmod/modsocket.c
    ${MICROPYTHON_PATH}/extmod/modssl_mbedtls.c
    ${MICROPYTHON_PATH}/extmod/modtime.c
    ${MICROPYTHON_PATH}/extmod/moductypes.c
    ${MICROPYTHON_PATH}/extmod/modwebsocket.c
    ${MICROPYTHON_PATH}/extmod/vfs.c
    ${MICROPYTHON_PATH}/extmod/vfs_posix.c
    ${MICROPYTHON_PATH}/extmod/vfs_posix_file.c
    ${MICROPYTHON_PATH}/extmod/vfs_reader.c
    ${MICROPYTHON_PATH}/extmod/virtpin.c

    ${MICROPYTHON_PATH}/shared/runtime/gchelper_generic.c

    ${MPY_CORE_FILES}
)

vsf_add_compile_definitions(
    ${VSF_TARGET_DEFINITIONS}
    __unix__
    MICROPY_PY_TERMIOS=1
    MICROPY_PY_SOCKET=1
    MICROPY_PY_THREAD=1
    MICROPY_PY_THREAD_GIL=0
    MICROPY_PY_SSL=1
    MICROPY_PY_RANDOM=1
    MICROPY_SSL_MBEDTLS=1

    # alloca support
    alloca=__builtin_alloca

    # vsf use micropython
    VSF_USE_MICROPYTHON=1
)
vsf_add_include_directories(
    ${VSF_COMMON_INCLUDE_DIRECTORIES}
    ${MICROPYTHON_PATH}/port
    ${MICROPYTHON_PATH}/port/${mpy_variant}
    ${MICROPYTHON_PATH}
    ${MICROPYTHON_PATH}/ports/unix
    ${MICROPYTHON_PATH}/ports/unix/variants/${mpy_variant}

    $ENV{VSF_PATH}/source/component/3rd-party/mbedtls/raw/include
)
vsf_add_include_directories(
    $ENV{VSF_PATH}/source/component/script/python/al/micropython/port/${mpy_variant}
)
vsf_add_sources(
    # vsf modules/port for micropython
    $ENV{VSF_PATH}/source/component/script/python/al/vsf_python_al.c
    $ENV{VSF_PATH}/source/component/script/python/al/micropython/vsf_micropython.c
    # use extmod/modos.c instead for linux
#    $ENV{VSF_PATH}/source/component/script/python/module/os/vsf_python_module_os.c

    ${MPY_SOURCES}
)

if(MICROPYTHON_USE_PYGAME)
    if(NOT VSF_USE_FREETYPE)
        message(FATAL_ERROR "VSF_USE_FREETYPE MUST be set because pygame depends on freetype")
    endif()
    if(NOT VSF_LINUX_USE_ALSA)
        message(FATAL_ERROR "VSF_LINUX_USE_ALSA MUST be set because pygame depends on alsa")
    endif()

    set(PYGAME_PATH ${MICROPYTHON_APPLET_PATH}/module/mpygame)

    if(VSF_APPLET)
        vsf_add_include_directories(
            $ENV{VSF_PATH}/source/component/3rd-party/freetype/port
            $ENV{VSF_PATH}/source/component/3rd-party/freetype/port/vsf_config
            ${LINUX_APPLET_PATH}/alsa/port
            ${LINUX_APPLET_PATH}/alsa/port/include
            ${LINUX_APPLET_PATH}/alsa/port/include/alsa
        )
    endif()

    vsf_add_compile_definitions(
        # pygame uses freetype
        VSF_FREETYPE_USE_STDIO=ENABLED
        VSF_USE_UI=ENABLED
        VSF_USE_INPUT=ENABLED
    )
    if(PYGAME_RENDER_USE_LIBCG)
        file(GLOB PYGAME_RENDER_SOURCES
            ${PYGAME_PATH}/renderer/libcg/raw/src/cg.c
            ${PYGAME_PATH}/renderer/libcg/raw/src/xft.c
            ${PYGAME_PATH}/renderer/libcg/mpygame_renderer_libcg.c
        )
        vsf_add_compile_definitions(
            MPYGAME_RENDERER_CFG_LIBCG
        )
        vsf_add_include_directories(
            ${PYGAME_PATH}/renderer/libcg/raw/src
        )
    else()
        message(FATAL_ERROR "PYGAME_RENDER_USE_XXX MUST be defined to select a renderer")
    endif()

    file(GLOB PYGAME_SOURCES
        ${PYGAME_PATH}/mpygame.c
        ${PYGAME_PATH}/mpygame_color.c
        ${PYGAME_PATH}/mpygame_display.c
        ${PYGAME_PATH}/mpygame_draw.c
        ${PYGAME_PATH}/mpygame_event.c
        ${PYGAME_PATH}/mpygame_font.c
        ${PYGAME_PATH}/mpygame_freetype.c
        ${PYGAME_PATH}/mpygame_image.c
        ${PYGAME_PATH}/mpygame_mixer.c
        ${PYGAME_PATH}/mpygame_rect.c
        ${PYGAME_PATH}/mpygame_sprite.c
        ${PYGAME_PATH}/mpygame_surface.c
        ${PYGAME_PATH}/mpygame_time.c
    )
    vsf_add_include_directories(
        ${PYGAME_PATH}/stb
    )
    vsf_add_sources(
        ${PYGAME_SOURCES}
        ${PYGAME_RENDER_SOURCES}
    )
endif()