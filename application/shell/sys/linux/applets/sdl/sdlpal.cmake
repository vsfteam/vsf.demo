set(SDLPAL_PATH ${SDL2_APPLET_PATH}/sdlpal)
file(GLOB_RECURSE SDLPAL_SOURCES
    ${SDLPAL_PATH}/adplug/binfile.cpp
    ${SDLPAL_PATH}/adplug/binio.cpp
    ${SDLPAL_PATH}/adplug/dosbox_opls.cpp
    ${SDLPAL_PATH}/adplug/emuopls.cpp
    ${SDLPAL_PATH}/adplug/fprovide.cpp
    ${SDLPAL_PATH}/adplug/mame_opls.cpp
    ${SDLPAL_PATH}/adplug/nuked_opl.c
    ${SDLPAL_PATH}/adplug/player.cpp
    ${SDLPAL_PATH}/adplug/rix.cpp
    ${SDLPAL_PATH}/adplug/surroundopl.cpp

    ${SDLPAL_PATH}/timidity/common.c
    ${SDLPAL_PATH}/timidity/instrum.c
    ${SDLPAL_PATH}/timidity/mix.c
    ${SDLPAL_PATH}/timidity/output.c
    ${SDLPAL_PATH}/timidity/playmidi.c
    ${SDLPAL_PATH}/timidity/readmidi.c
    ${SDLPAL_PATH}/timidity/resample.c
    ${SDLPAL_PATH}/timidity/tables.c
    ${SDLPAL_PATH}/timidity/timidity.c

    ${SDLPAL_PATH}/audio.c
    ${SDLPAL_PATH}/aviplay.c
    ${SDLPAL_PATH}/battle.c
    ${SDLPAL_PATH}/ending.c
    ${SDLPAL_PATH}/fight.c
    ${SDLPAL_PATH}/font.c
    ${SDLPAL_PATH}/game.c
    ${SDLPAL_PATH}/global.c
    ${SDLPAL_PATH}/glslpl.c
    ${SDLPAL_PATH}/input.c
    ${SDLPAL_PATH}/itemmenu.c
    ${SDLPAL_PATH}/magicmenu.c
    ${SDLPAL_PATH}/main.c
    ${SDLPAL_PATH}/map.c
    ${SDLPAL_PATH}/midi.c
    ${SDLPAL_PATH}/midi_timidity.c
    ${SDLPAL_PATH}/midi_tsf.c
    ${SDLPAL_PATH}/mini_glloader.c
    ${SDLPAL_PATH}/mp3play.c
    ${SDLPAL_PATH}/oggplay.c
    ${SDLPAL_PATH}/opusplay.c
    ${SDLPAL_PATH}/overlay.c
    ${SDLPAL_PATH}/palcfg.c
    ${SDLPAL_PATH}/palcommon.c
    ${SDLPAL_PATH}/palette.c
    ${SDLPAL_PATH}/play.c
    ${SDLPAL_PATH}/res.c
    ${SDLPAL_PATH}/resampler.c
    ${SDLPAL_PATH}/rixplay.cpp
    ${SDLPAL_PATH}/rngplay.c
    ${SDLPAL_PATH}/scene.c
    ${SDLPAL_PATH}/script.c
    ${SDLPAL_PATH}/sound.c
    ${SDLPAL_PATH}/text.c
    ${SDLPAL_PATH}/ui.c
    ${SDLPAL_PATH}/uibattle.c
    ${SDLPAL_PATH}/uigame.c
    ${SDLPAL_PATH}/util.c
    ${SDLPAL_PATH}/video.c
    ${SDLPAL_PATH}/video_glsl.c
    ${SDLPAL_PATH}/yj1.c
)
vsf_add_compile_definitions(
    ${VSF_TARGET_DEFINITIONS}
    __STDC_WANT_SECURE_LIB__=0
    VSF_USE_UI=ENABLED
    VSF_USE_INPUT=ENABLED
)
vsf_add_include_directories(
    ${VSF_COMMON_INCLUDE_DIRECTORIES}
    ${SDLPAL_PATH}
    ${SDLPAL_PATH}/timidity
    ${SDLPAL_PATH}/vsf
    ${SDL2_APPLET_PATH}/include
    ${SDL2_APPLET_PATH}/include/SDL2
)
vsf_add_sources(
    ${SDLPAL_SOURCES}
)
# sdlpal uses c++ and need libstdc++, but libstdc++ is not compiled with required XIP options
#vsf_add_libraries(
#    stdc++
#)