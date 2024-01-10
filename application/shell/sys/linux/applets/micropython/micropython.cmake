# CMakeLists head

cmake_policy(SET CMP0079 NEW)

file(GLOB MPY_CORE_FILES raw/py/*.c)
file(GLOB_RECURSE MPY_SOURCES
    raw/ports/unix/alloc.c
    raw/ports/unix/coverage.c
    raw/ports/unix/gccollect.c
    raw/ports/unix/input.c
    raw/ports/unix/main.c
    raw/ports/unix/modsocket.c
    raw/ports/unix/modtermios.c
    raw/ports/unix/mpthreadport.c
    raw/ports/unix/unix_mphal.c

    raw/extmod/machine_bitstream.c
    raw/extmod/machine_i2c.c
    raw/extmod/machine_mem.c
    raw/extmod/machine_pinbase.c
    raw/extmod/machine_pulse.c
    raw/extmod/machine_pwm.c
    raw/extmod/machine_signal.c
    raw/extmod/machine_spi.c
    raw/extmod/machine_timer.c
    raw/extmod/modasyncio.c
    raw/extmod/modbinascii.c
    raw/extmod/modcryptolib.c
    raw/extmod/moddeflate.c
    raw/extmod/modframebuf.c
    raw/extmod/modhashlib.c
    raw/extmod/modheapq.c
    raw/extmod/modjson.c
    raw/extmod/modnetwork.c
#    raw/extmod/modos.c
    raw/extmod/modplatform.c
    raw/extmod/modrandom.c
    raw/extmod/modre.c
    raw/extmod/modselect.c
    raw/extmod/modsocket.c
    raw/extmod/modssl_mbedtls.c
    raw/extmod/modtime.c
    raw/extmod/moductypes.c
    raw/extmod/modwebsocket.c
    raw/extmod/vfs.c
    raw/extmod/vfs_posix.c
    raw/extmod/vfs_posix_file.c
    raw/extmod/vfs_reader.c
    raw/extmod/virtpin.c

    raw/shared/runtime/gchelper_generic.c

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

    # vsf use micropython
    VSF_USE_MICROPYTHON=1
)
vsf_add_include_directories(
    port
    port/${mpy_variant}
    raw
    raw/ports/unix
    raw/ports/unix/variants/${mpy_variant}

    $ENV{VSF_PATH}/source/component/3rd-party/mbedtls/raw/include
)
vsf_add_sources(
    # vsf modules/port for micropython
    $ENV{VSF_PATH}/source/component/script/python/al/micropython/vsf_micropython.c
    $ENV{VSF_PATH}/source/component/script/python/module/os/vsf_python_module_os.c

    ${MPY_SOURCES}
)