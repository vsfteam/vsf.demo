set(SDL2_PATH ${SDL2_APPLET_PATH})
file(GLOB_RECURSE SDL2_SOURCES
    ${SDL2_PATH}/port/src/audio/vsf/SDL_vsfaudio.c
    ${SDL2_PATH}/port/src/main/vsf/SDL_vsf_main.c
    ${SDL2_PATH}/port/src/video/vsf/SDL_vsfvideo.c
    ${SDL2_PATH}/port/sdl2_platform_vsf.c

    ${SDL2_PATH}/raw/src/atomic/SDL_atomic.c
    ${SDL2_PATH}/raw/src/atomic/SDL_spinlock.c

    ${SDL2_PATH}/raw/src/audio/dummy/SDL_dummyaudio.c
    ${SDL2_PATH}/raw/src/audio/SDL_audio.c
    ${SDL2_PATH}/raw/src/audio/SDL_audiocvt.c
    ${SDL2_PATH}/raw/src/audio/SDL_audiotypecvt.c
    ${SDL2_PATH}/raw/src/audio/SDL_mixer.c
    ${SDL2_PATH}/raw/src/audio/SDL_wave.c

    ${SDL2_PATH}/raw/src/cpuinfo/SDL_cpuinfo.c

    ${SDL2_PATH}/raw/src/events/imKStoUCS.c
    ${SDL2_PATH}/raw/src/events/SDL_clipboardevents.c
    ${SDL2_PATH}/raw/src/events/SDL_displayevents.c
    ${SDL2_PATH}/raw/src/events/SDL_dropevents.c
    ${SDL2_PATH}/raw/src/events/SDL_events.c
    ${SDL2_PATH}/raw/src/events/SDL_gesture.c
    ${SDL2_PATH}/raw/src/events/SDL_keyboard.c
    ${SDL2_PATH}/raw/src/events/SDL_mouse.c
    ${SDL2_PATH}/raw/src/events/SDL_quit.c
    ${SDL2_PATH}/raw/src/events/SDL_touch.c
    ${SDL2_PATH}/raw/src/events/SDL_windowevents.c

    ${SDL2_PATH}/raw/src/file/SDL_rwops.c

    ${SDL2_PATH}/raw/src/filesystem/unix/SDL_sysfilesystem.c

    ${SDL2_PATH}/raw/src/joystick/dummy/SDL_sysjoystick.c
    ${SDL2_PATH}/raw/src/joystick/SDL_gamecontroller.c
    ${SDL2_PATH}/raw/src/joystick/SDL_joystick.c

    ${SDL2_PATH}/raw/src/render/software/SDL_blendfillrect.c
    ${SDL2_PATH}/raw/src/render/software/SDL_blendline.c
    ${SDL2_PATH}/raw/src/render/software/SDL_blendpoint.c
    ${SDL2_PATH}/raw/src/render/software/SDL_drawline.c
    ${SDL2_PATH}/raw/src/render/software/SDL_drawpoint.c
    ${SDL2_PATH}/raw/src/render/software/SDL_render_sw.c
    ${SDL2_PATH}/raw/src/render/software/SDL_rotate.c
    ${SDL2_PATH}/raw/src/render/software/SDL_triangle.c
    ${SDL2_PATH}/raw/src/render/SDL_d3dmath.c
    ${SDL2_PATH}/raw/src/render/SDL_render.c
    ${SDL2_PATH}/raw/src/render/SDL_yuv_sw.c

    ${SDL2_PATH}/raw/src/stdlib/SDL_crc32.c
    ${SDL2_PATH}/raw/src/stdlib/SDL_getenv.c
    ${SDL2_PATH}/raw/src/stdlib/SDL_iconv.c
    ${SDL2_PATH}/raw/src/stdlib/SDL_malloc.c
    ${SDL2_PATH}/raw/src/stdlib/SDL_qsort.c
    ${SDL2_PATH}/raw/src/stdlib/SDL_stdlib.c
    ${SDL2_PATH}/raw/src/stdlib/SDL_string.c
    ${SDL2_PATH}/raw/src/stdlib/SDL_strtokr.c

    ${SDL2_PATH}/raw/src/thread/pthread/SDL_syscond.c
    ${SDL2_PATH}/raw/src/thread/pthread/SDL_sysmutex.c
    ${SDL2_PATH}/raw/src/thread/pthread/SDL_syssem.c
    ${SDL2_PATH}/raw/src/thread/pthread/SDL_systhread.c
    ${SDL2_PATH}/raw/src/thread/pthread/SDL_systls.c
    ${SDL2_PATH}/raw/src/thread/SDL_thread.c

    ${SDL2_PATH}/raw/src/timer/unix/SDL_systimer.c
    ${SDL2_PATH}/raw/src/timer/SDL_timer.c

    ${SDL2_PATH}/raw/src/video/dummy/SDL_nullevents.c
    ${SDL2_PATH}/raw/src/video/dummy/SDL_nullframebuffer.c
    ${SDL2_PATH}/raw/src/video/dummy/SDL_nullvideo.c
    ${SDL2_PATH}/raw/src/video/yuv2rgb/yuv_rgb.c
    ${SDL2_PATH}/raw/src/video/SDL_blit.c
    ${SDL2_PATH}/raw/src/video/SDL_blit_0.c
    ${SDL2_PATH}/raw/src/video/SDL_blit_1.c
    ${SDL2_PATH}/raw/src/video/SDL_blit_A.c
    ${SDL2_PATH}/raw/src/video/SDL_blit_auto.c
    ${SDL2_PATH}/raw/src/video/SDL_blit_copy.c
    ${SDL2_PATH}/raw/src/video/SDL_blit_N.c
    ${SDL2_PATH}/raw/src/video/SDL_blit_slow.c
    ${SDL2_PATH}/raw/src/video/SDL_bmp.c
    ${SDL2_PATH}/raw/src/video/SDL_clipboard.c
    ${SDL2_PATH}/raw/src/video/SDL_egl.c
    ${SDL2_PATH}/raw/src/video/SDL_fillrect.c
    ${SDL2_PATH}/raw/src/video/SDL_pixels.c
    ${SDL2_PATH}/raw/src/video/SDL_rect.c
    ${SDL2_PATH}/raw/src/video/SDL_RLEaccel.c
    ${SDL2_PATH}/raw/src/video/SDL_shape.c
    ${SDL2_PATH}/raw/src/video/SDL_stretch.c
    ${SDL2_PATH}/raw/src/video/SDL_surface.c
    ${SDL2_PATH}/raw/src/video/SDL_video.c
    ${SDL2_PATH}/raw/src/video/SDL_vulkan_utils.c
    ${SDL2_PATH}/raw/src/video/SDL_yuv.c

    ${SDL2_PATH}/raw/src/SDL.c
    ${SDL2_PATH}/raw/src/SDL_assert.c
    ${SDL2_PATH}/raw/src/SDL_dataqueue.c
    ${SDL2_PATH}/raw/src/SDL_error.c
    ${SDL2_PATH}/raw/src/SDL_hints.c
    ${SDL2_PATH}/raw/src/SDL_list.c
    ${SDL2_PATH}/raw/src/SDL_log.c
)
vsf_add_compile_definitions(
    ${VSF_TARGET_DEFINITIONS}
    VSF_USER_ENTRY=SDL_main
    SIGWINCH=28
)
vsf_add_include_directories(
    ${VSF_COMMON_INCLUDE_DIRECTORIES}
    ${SDL2_PATH}/include
    ${SDL2_PATH}/include/SDL2
)
vsf_add_sources(
    ${SDL2_SOURCES}
)
