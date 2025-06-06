#include "../raw/src/SDL_internal.h"

#ifdef __VSF__

#if (VSF_USE_APPLET == ENABLED || (VSF_USE_LINUX == ENABLED && VSF_LINUX_USE_APPLET == ENABLED)) && VSF_SDL2_USE_VPLT == ENABLED

#   include <SDL.h>

#   define __SDL2_APIS                                                                                      \
        SDL_Init, SDL_InitSubSystem, SDL_QuitSubSystem, SDL_WasInit, SDL_Quit,  /* SDL.h */                 \
        SDL_AtomicTryLock, SDL_AtomicLock, SDL_AtomicUnlock, SDL_AtomicCAS,     /* SDL_atomic.h */          \
        SDL_AtomicSet, SDL_AtomicGet, SDL_AtomicAdd, SDL_AtomicCASPtr,                                      \
        SDL_AtomicSetPtr, SDL_AtomicGetPtr,                                                                 \
        SDL_malloc, SDL_calloc, SDL_realloc, SDL_free, SDL_GetMemoryFunctions,  /* SDL_stdinc.h */          \
        SDL_SetMemoryFunctions, SDL_GetNumAllocations, SDL_getenv, SDL_setenv,                              \
        SDL_qsort, SDL_abs, SDL_isalpha, SDL_isalnum, SDL_isblank, SDL_iscntrl,                             \
        SDL_isdigit, SDL_isxdigit, SDL_ispunct, SDL_isspace, SDL_isupper,                                   \
        SDL_islower, SDL_isprint, SDL_isgraph, SDL_toupper, SDL_tolower,                                    \
        SDL_crc32, SDL_memset, SDL_memcpy, SDL_memmove, SDL_memcmp, SDL_wcslen,                             \
        SDL_wcslcpy, SDL_wcslcat, SDL_wcsdup, SDL_wcsstr, SDL_wcscmp,                                       \
        SDL_wcsncmp, SDL_wcscasecmp, SDL_wcsncasecmp, SDL_strlen, SDL_strlcpy,                              \
        SDL_utf8strlcpy, SDL_strlcat, SDL_strdup, SDL_strrev, SDL_strupr,                                   \
        SDL_strlwr, SDL_strchr, SDL_strrchr, SDL_strstr, SDL_strtokr,                                       \
        SDL_utf8strlen, SDL_itoa, SDL_uitoa, SDL_ltoa, SDL_ultoa, SDL_lltoa,                                \
        SDL_ulltoa, SDL_atoi, SDL_atof, SDL_strtol, SDL_strtoul, SDL_strtoll,                               \
        SDL_strtoull, SDL_strtod, SDL_strcmp, SDL_strncmp, SDL_strcasecmp,                                  \
        SDL_strncasecmp, SDL_sscanf, SDL_vsscanf, SDL_snprintf, SDL_vsnprintf,                              \
        SDL_asprintf, SDL_vasprintf, SDL_acos, SDL_acosf, SDL_asin, SDL_asinf,                              \
        SDL_atan, SDL_atanf, SDL_atan2, SDL_atan2f, SDL_ceil, SDL_ceilf,                                    \
        SDL_copysign, SDL_copysignf, SDL_cos, SDL_cosf, SDL_exp, SDL_expf,                                  \
        SDL_fabs, SDL_fabsf, SDL_floor, SDL_floorf, SDL_trunc, SDL_truncf,                                  \
        SDL_fmod, SDL_fmodf, SDL_log, SDL_logf, SDL_log10, SDL_log10f, SDL_pow,                             \
        SDL_powf, SDL_round, SDL_roundf, SDL_lround, SDL_lroundf, SDL_scalbn,                               \
        SDL_scalbnf, SDL_sin, SDL_sinf, SDL_sqrt, SDL_sqrtf, SDL_tan, SDL_tanf,                             \
        SDL_iconv_open, SDL_iconv_close, SDL_iconv, SDL_iconv_string,                                       \
        SDL_SetHintWithPriority, SDL_SetHint, SDL_GetHint, SDL_GetHintBoolean,  /* SDL_hints.h */           \
        SDL_AddHintCallback, SDL_DelHintCallback, SDL_ClearHints,                                           \
        SDL_ShowMessageBox, SDL_ShowSimpleMessageBox,                           /* SDL_messagebox.h */      \
        SDL_GetNumAudioDrivers, SDL_GetAudioDriver, SDL_AudioInit,              /* SDL_audio.h */           \
        SDL_AudioQuit, SDL_GetCurrentAudioDriver, SDL_OpenAudio,                                            \
        SDL_GetNumAudioDevices, SDL_GetAudioDeviceName, SDL_GetAudioDeviceSpec,                             \
        SDL_OpenAudioDevice, SDL_GetAudioStatus, SDL_GetAudioDeviceStatus,                                  \
        SDL_PauseAudio, SDL_PauseAudioDevice, SDL_LoadWAV_RW, SDL_FreeWAV,                                  \
        SDL_BuildAudioCVT, SDL_ConvertAudio, SDL_NewAudioStream,                                            \
        SDL_AudioStreamPut, SDL_AudioStreamGet, SDL_AudioStreamAvailable,                                   \
        SDL_AudioStreamFlush, SDL_AudioStreamClear, SDL_FreeAudioStream,                                    \
        SDL_MixAudio, SDL_MixAudioFormat, SDL_QueueAudio, SDL_DequeueAudio,                                 \
        SDL_GetQueuedAudioSize, SDL_ClearQueuedAudio, SDL_LockAudio,                                        \
        SDL_LockAudioDevice, SDL_UnlockAudio, SDL_UnlockAudioDevice,                                        \
        SDL_CloseAudio, SDL_CloseAudioDevice,                                                               \
        SDL_SetClipboardText, SDL_GetClipboardText, SDL_HasClipboardText,       /* SDL_clipboard.h */       \
        SDL_GetCPUCount, SDL_GetCPUCacheLineSize, SDL_HasRDTSC, SDL_HasAltiVec, /* SDL_cpuinfo.h */         \
        SDL_HasMMX, SDL_Has3DNow, SDL_HasSSE, SDL_HasSSE2, SDL_HasSSE3,                                     \
        SDL_HasSSE41, SDL_HasSSE42, SDL_HasAVX, SDL_HasAVX2, SDL_HasAVX512F,                                \
        SDL_HasARMSIMD, SDL_HasNEON, SDL_GetSystemRAM, SDL_SIMDGetAlignment,                                \
        SDL_SIMDAlloc, SDL_SIMDRealloc, SDL_SIMDFree,                                                       \
        SDL_SetError, SDL_GetError, SDL_GetErrorMsg, SDL_ClearError, SDL_Error, /* SDL_error.h */           \
        SDL_PumpEvents, SDL_PeepEvents, SDL_HasEvent, SDL_HasEvents,            /* SDL_events.h */          \
        SDL_FlushEvent, SDL_FlushEvents, SDL_PollEvent, SDL_WaitEvent,                                      \
        SDL_WaitEventTimeout, SDL_PushEvent, SDL_SetEventFilter,                                            \
        SDL_GetEventFilter, SDL_AddEventWatch, SDL_DelEventWatch,                                           \
        SDL_FilterEvents, SDL_EventState, SDL_RegisterEvents,                                               \
        SDL_GetBasePath, SDL_GetPrefPath,                                       /* SDL_filesystem.h */      \
        SDL_GetKeyboardFocus, SDL_GetKeyboardState, SDL_GetModState,            /* SDL_keyboard.h */        \
        SDL_SetModState, SDL_GetKeyFromScancode, SDL_GetScancodeFromKey,                                    \
        SDL_GetScancodeName, SDL_GetScancodeFromName, SDL_GetKeyName,                                       \
        SDL_GetKeyFromName, SDL_StartTextInput, SDL_IsTextInputActive,                                      \
        SDL_StopTextInput, SDL_ClearComposition, SDL_IsTextInputShown,                                      \
        SDL_SetTextInputRect, SDL_HasScreenKeyboardSupport,                                                 \
        SDL_IsScreenKeyboardShown,                                                                          \
        SDL_GameControllerAddMappingsFromRW, SDL_GameControllerAddMapping,      /* SDL_gamecontroller.h */  \
        SDL_GameControllerNumMappings, SDL_GameControllerMappingForIndex,                                   \
        SDL_GameControllerMappingForGUID, SDL_GameControllerMapping,                                        \
        SDL_IsGameController, SDL_GameControllerNameForIndex,                                               \
        SDL_GameControllerTypeForIndex, SDL_GameControllerMappingForDeviceIndex,                            \
        SDL_GameControllerOpen, SDL_GameControllerFromInstanceID,                                           \
        SDL_GameControllerFromPlayerIndex, SDL_GameControllerName,                                          \
        SDL_GameControllerGetType, SDL_GameControllerGetPlayerIndex,                                        \
        SDL_GameControllerSetPlayerIndex, SDL_GameControllerGetVendor,                                      \
        SDL_GameControllerGetProduct, SDL_GameControllerGetProductVersion,                                  \
        SDL_GameControllerGetSerial, SDL_GameControllerGetAttached,                                         \
        SDL_GameControllerGetJoystick, SDL_GameControllerEventState,                                        \
        SDL_GameControllerUpdate, SDL_GameControllerGetAxisFromString,                                      \
        SDL_GameControllerGetStringForAxis, SDL_GameControllerGetBindForAxis,                               \
        SDL_GameControllerHasAxis, SDL_GameControllerGetAxis,                                               \
        SDL_GameControllerGetButtonFromString,                                                              \
        SDL_GameControllerGetStringForButton,                                                               \
        SDL_GameControllerGetBindForButton, SDL_GameControllerHasButton,                                    \
        SDL_GameControllerGetButton, SDL_GameControllerGetNumTouchpads,                                     \
        SDL_GameControllerGetNumTouchpadFingers,                                                            \
        SDL_GameControllerGetTouchpadFinger, SDL_GameControllerHasSensor,                                   \
        SDL_GameControllerSetSensorEnabled, SDL_GameControllerIsSensorEnabled,                              \
        SDL_GameControllerGetSensorDataRate, SDL_GameControllerGetSensorData,                               \
        SDL_GameControllerRumble, SDL_GameControllerRumbleTriggers,                                         \
        SDL_GameControllerHasLED, SDL_GameControllerHasRumble,                                              \
        SDL_GameControllerHasRumbleTriggers, SDL_GameControllerSetLED,                                      \
        SDL_GameControllerSendEffect, SDL_GameControllerClose,                                              \
        SDL_GameControllerGetAppleSFSymbolsNameForButton,                                                   \
        SDL_GameControllerGetAppleSFSymbolsNameForAxis,                                                     \
        SDL_LogSetAllPriority, SDL_LogSetPriority, SDL_LogGetPriority,          /* SDL_log.h */             \
        SDL_LogResetPriorities, SDL_Log, SDL_LogVerbose, SDL_LogDebug,                                      \
        SDL_LogInfo, SDL_LogWarn, SDL_LogError, SDL_LogCritical, SDL_LogMessage,                            \
        SDL_LogMessageV, SDL_LogGetOutputFunction, SDL_LogSetOutputFunction,                                \
        SDL_CreateMutex, SDL_LockMutex, SDL_TryLockMutex, SDL_UnlockMutex,      /* SDL_mutex.h */           \
        SDL_DestroyMutex, SDL_CreateSemaphore, SDL_DestroySemaphore,                                        \
        SDL_SemWait, SDL_SemTryWait, SDL_SemWaitTimeout, SDL_SemPost,                                       \
        SDL_SemValue, SDL_CreateCond, SDL_DestroyCond, SDL_CondSignal,                                      \
        SDL_CondBroadcast, SDL_CondWait, SDL_CondWaitTimeout,                                               \
        SDL_GetNumRenderDrivers, SDL_GetRenderDriverInfo,                       /* SDL_render.h */          \
        SDL_CreateWindowAndRenderer, SDL_CreateRenderer,                                                    \
        SDL_CreateSoftwareRenderer, SDL_GetRenderer, SDL_RenderGetWindow,                                   \
        SDL_GetRendererInfo, SDL_GetRendererOutputSize, SDL_CreateTexture,                                  \
        SDL_CreateTextureFromSurface, SDL_QueryTexture, SDL_SetTextureColorMod,                             \
        SDL_GetTextureColorMod, SDL_SetTextureAlphaMod, SDL_GetTextureAlphaMod,                             \
        SDL_SetTextureBlendMode, SDL_GetTextureBlendMode,                                                   \
        SDL_SetTextureScaleMode, SDL_GetTextureScaleMode,                                                   \
        SDL_SetTextureUserData, SDL_GetTextureUserData, SDL_UpdateTexture,                                  \
        SDL_UpdateYUVTexture, SDL_UpdateNVTexture, SDL_LockTexture,                                         \
        SDL_LockTextureToSurface, SDL_UnlockTexture, SDL_RenderTargetSupported,                             \
        SDL_SetRenderTarget, SDL_GetRenderTarget, SDL_RenderSetLogicalSize,                                 \
        SDL_RenderGetLogicalSize, SDL_RenderSetIntegerScale,                                                \
        SDL_RenderGetIntegerScale, SDL_RenderSetViewport, SDL_RenderGetViewport,                            \
        SDL_RenderSetClipRect, SDL_RenderGetClipRect, SDL_RenderIsClipEnabled,                              \
        SDL_RenderSetScale, SDL_RenderGetScale, SDL_RenderWindowToLogical,                                  \
        SDL_RenderLogicalToWindow, SDL_SetRenderDrawColor,                                                  \
        SDL_GetRenderDrawColor, SDL_SetRenderDrawBlendMode,                                                 \
        SDL_GetRenderDrawBlendMode, SDL_RenderClear, SDL_RenderDrawPoint,                                   \
        SDL_RenderDrawPoints, SDL_RenderDrawLine, SDL_RenderDrawLines,                                      \
        SDL_RenderDrawRect, SDL_RenderDrawRects, SDL_RenderFillRect,                                        \
        SDL_RenderFillRects, SDL_RenderCopy, SDL_RenderCopyEx,                                              \
        SDL_RenderDrawPointF, SDL_RenderDrawPointsF, SDL_RenderDrawLineF,                                   \
        SDL_RenderDrawLinesF, SDL_RenderDrawRectF, SDL_RenderDrawRectsF,                                    \
        SDL_RenderFillRectF, SDL_RenderFillRectsF, SDL_RenderCopyF,                                         \
        SDL_RenderCopyExF, SDL_RenderGeometry, SDL_RenderGeometryRaw,                                       \
        SDL_RenderReadPixels, SDL_RenderPresent, SDL_DestroyTexture,                                        \
        SDL_DestroyRenderer, SDL_RenderFlush, SDL_RenderSetVSync,                                           \
        SDL_RWFromFile, SDL_RWFromFP, SDL_RWFromMem, SDL_RWFromConstMem,        /* SDL_rwops.h */           \
        SDL_AllocRW, SDL_FreeRW, SDL_RWsize, SDL_RWseek, SDL_RWtell,                                        \
        SDL_RWread, SDL_RWwrite, SDL_RWclose, SDL_LoadFile_RW, SDL_LoadFile,                                \
        SDL_ReadU8, SDL_ReadLE16, SDL_ReadBE16, SDL_ReadLE32, SDL_ReadBE32,                                 \
        SDL_ReadLE64, SDL_ReadBE64, SDL_WriteU8, SDL_WriteLE16, SDL_WriteBE16,                              \
        SDL_WriteLE32, SDL_WriteBE32, SDL_WriteLE64, SDL_WriteBE64,                                         \
        SDL_CreateThread, SDL_CreateThreadWithStackSize, SDL_GetThreadName,     /* SDL_thread.h */          \
        SDL_ThreadID, SDL_GetThreadID, SDL_SetThreadPriority, SDL_WaitThread,                               \
        SDL_DetachThread, SDL_TLSCreate, SDL_TLSGet, SDL_TLSSet, SDL_TLSCleanup,                            \
        SDL_GetTicks, SDL_GetTicks64, SDL_GetPerformanceCounter,                /* SDL_timer.h */           \
        SDL_GetPerformanceFrequency, SDL_Delay, SDL_AddTimer, SDL_RemoveTimer,                              \
        SDL_GetVersion, SDL_GetRevision, SDL_GetRevisionNumber,                 /* SDL_version.h */         \
        SDL_GetNumVideoDrivers, SDL_GetVideoDriver, SDL_VideoInit,              /* SDL_video.h */           \
        SDL_VideoQuit, SDL_GetCurrentVideoDriver, SDL_GetNumVideoDisplays,                                  \
        SDL_GetDisplayName, SDL_GetDisplayBounds, SDL_GetDisplayUsableBounds,                               \
        SDL_GetDisplayDPI, SDL_GetDisplayOrientation, SDL_GetNumDisplayModes,                               \
        SDL_GetDisplayMode, SDL_GetDesktopDisplayMode,                                                      \
        SDL_GetCurrentDisplayMode, SDL_GetClosestDisplayMode,                                               \
        SDL_GetWindowDisplayIndex, SDL_SetWindowDisplayMode,                                                \
        SDL_GetWindowDisplayMode, SDL_GetWindowICCProfile,                                                  \
        SDL_GetWindowPixelFormat, SDL_CreateWindow, SDL_CreateWindowFrom,                                   \
        SDL_GetWindowID, SDL_GetWindowFromID, SDL_GetWindowFlags,                                           \
        SDL_SetWindowTitle, SDL_GetWindowTitle, SDL_SetWindowIcon,                                          \
        SDL_SetWindowData, SDL_GetWindowData, SDL_SetWindowPosition,                                        \
        SDL_GetWindowPosition, SDL_SetWindowSize, SDL_GetWindowSize,                                        \
        SDL_GetWindowBordersSize, SDL_SetWindowMinimumSize,                                                 \
        SDL_GetWindowMinimumSize, SDL_SetWindowMaximumSize,                                                 \
        SDL_GetWindowMaximumSize, SDL_SetWindowBordered, SDL_SetWindowResizable,                            \
        SDL_SetWindowAlwaysOnTop, SDL_ShowWindow, SDL_HideWindow,                                           \
        SDL_RaiseWindow, SDL_MaximizeWindow, SDL_MinimizeWindow,                                            \
        SDL_RestoreWindow, SDL_SetWindowFullscreen, SDL_GetWindowSurface,                                   \
        SDL_UpdateWindowSurface, SDL_UpdateWindowSurfaceRects,                                              \
        SDL_SetWindowGrab, SDL_SetWindowKeyboardGrab, SDL_SetWindowMouseGrab,                               \
        SDL_GetWindowGrab, SDL_GetWindowKeyboardGrab, SDL_GetWindowMouseGrab,                               \
        SDL_GetGrabbedWindow, SDL_SetWindowMouseRect, SDL_GetWindowMouseRect,                               \
        SDL_SetWindowBrightness, SDL_GetWindowBrightness, SDL_SetWindowOpacity,                             \
        SDL_GetWindowOpacity, SDL_SetWindowModalFor, SDL_SetWindowInputFocus,                               \
        SDL_SetWindowGammaRamp, SDL_GetWindowGammaRamp, SDL_SetWindowHitTest,                               \
        SDL_FlashWindow, SDL_DestroyWindow, SDL_IsScreenSaverEnabled,                                       \
        SDL_EnableScreenSaver, SDL_DisableScreenSaver,                                                      \
        SDL_CreateRGBSurface, SDL_CreateRGBSurfaceWithFormat,                   /* SDL_surface.h */         \
        SDL_CreateRGBSurfaceFrom, SDL_CreateRGBSurfaceWithFormatFrom,                                       \
        SDL_FreeSurface, SDL_SetSurfacePalette, SDL_LockSurface,                                            \
        SDL_UnlockSurface, SDL_LoadBMP_RW, SDL_SaveBMP_RW, SDL_SetSurfaceRLE,                               \
        SDL_HasSurfaceRLE, SDL_SetColorKey, SDL_HasColorKey, SDL_GetColorKey,                               \
        SDL_SetSurfaceColorMod, SDL_GetSurfaceColorMod, SDL_SetSurfaceAlphaMod,                             \
        SDL_GetSurfaceAlphaMod, SDL_SetSurfaceBlendMode,                                                    \
        SDL_GetSurfaceBlendMode, SDL_SetClipRect, SDL_GetClipRect,                                          \
        SDL_DuplicateSurface, SDL_ConvertSurface, SDL_ConvertSurfaceFormat,                                 \
        SDL_ConvertPixels, SDL_PremultiplyAlpha, SDL_FillRect, SDL_FillRects,                               \
        SDL_UpperBlit, SDL_LowerBlit, SDL_SoftStretch, SDL_SoftStretchLinear,                               \
        SDL_UpperBlitScaled, SDL_LowerBlitScaled, SDL_SetYUVConversionMode,                                 \
        SDL_GetYUVConversionMode, SDL_GetYUVConversionModeForResolution,                                    \
        SDL_GetPixelFormatName, SDL_PixelFormatEnumToMasks,                     /* SDL_pixels.h */          \
        SDL_MasksToPixelFormatEnum, SDL_AllocFormat, SDL_FreeFormat,                                        \
        SDL_AllocPalette, SDL_SetPixelFormatPalette, SDL_SetPaletteColors,                                  \
        SDL_FreePalette, SDL_MapRGB, SDL_MapRGBA, SDL_GetRGB, SDL_GetRGBA,                                  \
        SDL_CalculateGammaRamp

typedef struct vsf_sdl2_vplt_t {
    vsf_vplt_info_t info;

    // self
    VSF_APPLET_VPLT_ENTRY_MOD_DEF(sdl2);

#   define sdl2_define_api_entry(__name)        VSF_APPLET_VPLT_ENTRY_FUNC_DEF(__name);

    VSF_MFOREACH(sdl2_define_api_entry,
        __SDL2_APIS
    )
} vsf_sdl2_vplt_t;

static __VSF_VPLT_DECORATOR__ vsf_sdl2_vplt_t __vsf_sdl2_vplt = {
    VSF_APPLET_VPLT_INFO(vsf_sdl2_vplt_t, 0, 0, true),
    VSF_APPLET_VPLT_ENTRY_MOD(sdl2, __vsf_sdl2_vplt),

#   define sdl2_api_entry(__name)               VSF_APPLET_VPLT_ENTRY_FUNC(__name),
    VSF_MFOREACH(sdl2_api_entry,
        __SDL2_APIS
    )
};
#endif

VSF_CAL_NO_INIT SDL_VSF_platform SDL_platform;

void vsf_sdl2_init(vsf_sdl2_cfg_t *cfg)
{
    SDL_platform.cfg = *cfg;
    SDL_platform.is_disp_inited = false;
    SDL_platform.is_audio_inited = false;

#if (VSF_USE_APPLET == ENABLED || (VSF_USE_LINUX == ENABLED && VSF_LINUX_USE_APPLET == ENABLED)) && VSF_SDL2_USE_VPLT == ENABLED
    vsf_vplt_load_dyn((vsf_vplt_info_t *)&__vsf_sdl2_vplt.info);
#endif
}

#endif