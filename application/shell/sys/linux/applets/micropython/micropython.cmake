# CMakeLists head

cmake_policy(SET CMP0079 NEW)

file(GLOB MPY_CORE_FILES ${MICROPYTHON_PATH}/raw/py/*.c)
file(GLOB MPY_SOURCES
    ${MICROPYTHON_PATH}/raw/ports/unix/alloc.c
    ${MICROPYTHON_PATH}/raw/ports/unix/coverage.c
    ${MICROPYTHON_PATH}/raw/ports/unix/gccollect.c
    ${MICROPYTHON_PATH}/raw/ports/unix/input.c
    ${MICROPYTHON_PATH}/raw/ports/unix/main.c
    ${MICROPYTHON_PATH}/raw/ports/unix/modsocket.c
    ${MICROPYTHON_PATH}/raw/ports/unix/modtermios.c
    ${MICROPYTHON_PATH}/raw/ports/unix/mpthreadport.c
    ${MICROPYTHON_PATH}/raw/ports/unix/unix_mphal.c

    ${MICROPYTHON_PATH}/raw/extmod/machine_bitstream.c
    ${MICROPYTHON_PATH}/raw/extmod/machine_i2c.c
    ${MICROPYTHON_PATH}/raw/extmod/machine_mem.c
    ${MICROPYTHON_PATH}/raw/extmod/machine_pinbase.c
    ${MICROPYTHON_PATH}/raw/extmod/machine_pulse.c
    ${MICROPYTHON_PATH}/raw/extmod/machine_pwm.c
    ${MICROPYTHON_PATH}/raw/extmod/machine_signal.c
    ${MICROPYTHON_PATH}/raw/extmod/machine_spi.c
    ${MICROPYTHON_PATH}/raw/extmod/machine_timer.c
    ${MICROPYTHON_PATH}/raw/extmod/modasyncio.c
    ${MICROPYTHON_PATH}/raw/extmod/modbinascii.c
    ${MICROPYTHON_PATH}/raw/extmod/modcryptolib.c
    ${MICROPYTHON_PATH}/raw/extmod/moddeflate.c
    ${MICROPYTHON_PATH}/raw/extmod/modframebuf.c
    ${MICROPYTHON_PATH}/raw/extmod/modhashlib.c
    ${MICROPYTHON_PATH}/raw/extmod/modheapq.c
    ${MICROPYTHON_PATH}/raw/extmod/modjson.c
    ${MICROPYTHON_PATH}/raw/extmod/modmachine.c
    ${MICROPYTHON_PATH}/raw/extmod/modnetwork.c
#    ${MICROPYTHON_PATH}/raw/extmod/modos.c
    ${MICROPYTHON_PATH}/raw/extmod/modplatform.c
    ${MICROPYTHON_PATH}/raw/extmod/modrandom.c
    ${MICROPYTHON_PATH}/raw/extmod/modre.c
    ${MICROPYTHON_PATH}/raw/extmod/modselect.c
    ${MICROPYTHON_PATH}/raw/extmod/modsocket.c
    ${MICROPYTHON_PATH}/raw/extmod/modssl_mbedtls.c
    ${MICROPYTHON_PATH}/raw/extmod/modtime.c
    ${MICROPYTHON_PATH}/raw/extmod/moductypes.c
    ${MICROPYTHON_PATH}/raw/extmod/modwebsocket.c
    ${MICROPYTHON_PATH}/raw/extmod/vfs.c
    ${MICROPYTHON_PATH}/raw/extmod/vfs_posix.c
    ${MICROPYTHON_PATH}/raw/extmod/vfs_posix_file.c
    ${MICROPYTHON_PATH}/raw/extmod/vfs_reader.c
    ${MICROPYTHON_PATH}/raw/extmod/virtpin.c

    ${MICROPYTHON_PATH}/raw/shared/runtime/gchelper_generic.c

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
    MICROPY_SSL_MBEDTLS=1

    # alloca support
    alloca=__builtin_alloca

    # vsf use micropython
    VSF_USE_MICROPYTHON=1
)
vsf_add_include_directories(
    ${MICROPYTHON_PATH}/port
    ${MICROPYTHON_PATH}/port/${mpy_variant}
    ${MICROPYTHON_PATH}/raw
    ${MICROPYTHON_PATH}/raw/ports/unix
    ${MICROPYTHON_PATH}/raw/ports/unix/variants/${mpy_variant}

    $ENV{VSF_PATH}/source/component/3rd-party/mbedtls/raw/include
)
vsf_add_sources(
    # vsf modules/port for micropython
    $ENV{VSF_PATH}/source/component/script/python/al/micropython/vsf_micropython.c
    $ENV{VSF_PATH}/source/component/script/python/module/os/vsf_python_module_os.c

    ${MPY_SOURCES}
)