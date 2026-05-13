;---------------------------------
;
; Functions accessed by the debugger to perform a flash download.
; All public symbols and the function FlashBreak() are looked up and called by the debugger.
;
; Copyright (c) 2008 IAR Systems
;
; $Revision: 54488 $
;
;---------------------------------

; The upstream IAR framework2 sample only lists pre-ARMv8-M cores in the
; _CORTEX_ predicate (ARMv6-M / ARMv7-M / ARMv7E-M). BH1098 is ARMv8.1-M
; (core_star / Cortex-M85, IAR state=72), which falls through to the ARM
; (CODE32) branch and errors out with "does not support ARM/CODE32".
; This flashloader is Cortex-M only, so force THUMB mode unconditionally.
#define _CORTEX_  1

        PUBLIC FlashInitEntry
        PUBLIC FlashWriteEntry
        ; FlashEraseWriteEntry is intentionally NOT exported.
        ; IAR 9.60 C-SPY's combined erase-write path fails to populate
        ; theFlashParams.base_ptr/offset_into_block/count before each
        ; Fl2FlashEraseWriteEntry invocation (only block_size is written),
        ; resulting in 355 identical calls that all try to program
        ; base=0x02000000/count=0. Dropping the symbol forces C-SPY to
        ; fall back to the legacy two-stage FlashErase + FlashWrite
        ; protocol, which does populate the params properly.
        ; PUBLIC FlashEraseWriteEntry
        PUBLIC FlashChecksumEntry
        PUBLIC FlashSignoffEntry
        PUBLIC FlashBufferStart
        PUBLIC FlashBufferEnd

        EXTERN FlashBreak
        EXTERN Fl2FlashInitEntry
        EXTERN Fl2FlashWriteEntry
        EXTERN Fl2FlashEraseWriteEntry
        EXTERN Fl2FlashChecksumEntry
        EXTERN Fl2FlashSignoffEntry

        ; IAR CRT segment-init entry. Processes Region$$Table to copy .data
        ; initializers from their ROM image and zero-init .bss / zero-init
        ; regions. We MUST call this before the first C code runs, because
        ; FlashInitEntry does not go through __iar_program_start (the usual
        ; startup entry that chains __iar_data_init3 + main). Without this,
        ; all initialized globals in the flashloader (e.g. BH1098's
        ; g_jmp_tab ROM trampoline array and vsf_hw_xip_flash0 driver
        ; instance) hold garbage and the first call through them faults.
        ;
        ; Placing the call here (rather than in C) is the cleanest option:
        ; C-SPY guarantees FlashInitEntry is invoked exactly once per
        ; flashloader load, giving us a natural one-shot site. Moving the
        ; call into C would require a gate flag, and since
        ; __iar_data_init3 re-initializes every .data symbol the gate
        ; would have to live in a .noinit segment whose startup value is
        ; undefined -- a worse trade-off than this single assembly line.
        EXTERN __iar_data_init3

        ; IAR CRT FPU bring-up. BH1098 is a Cortex-M85 (ARMv8.1-M with
        ; FPU); CPACR CP10/CP11 come out of reset DISABLED, so any VFP
        ; instruction (which the compiler may emit for memcpy/memset-like
        ; loops, vsnprintf, or register save/restore on IRQ entry) faults
        ; silently under C-SPY RPC. __iar_program_start normally chains
        ; this for us; FlashInitEntry bypasses __iar_program_start, so we
        ; must call __iar_init_vfp explicitly before any C code runs.
        ; The IAR standard startup order is __iar_init_core / __iar_init_vfp
        ; before __iar_data_init3, so place the call here.
        EXTERN __iar_init_vfp

        SECTION CSTACK:DATA:NOROOT(3)


;---------------------------------
;
; FlashInitEntry()
; Debugger interface function
;
;---------------------------------
        SECTION .text:CODE:ROOT(2)
#if !_CORTEX_
        ARM
#else
        THUMB
#endif

FlashInitEntry:
#if !_CORTEX_
        ;; Set up the normal stack pointer.
        LDR     sp, =SFE(CSTACK)        ; End of CSTACK
#endif
        ;; Enable FPU first (writes CPACR; safe to run before data_init3
        ;; because __iar_init_vfp does not touch RAM-resident data).
        BL       __iar_init_vfp
        ;; Run IAR CRT data/bss initialization. Only needed on the Init
        ;; entry -- subsequent Write/Erase entries must NOT re-run this or
        ;; they would wipe state carried over from FlashInit (e.g. the
        ;; cached g_cap).
        BL       __iar_data_init3
        BL       Fl2FlashInitEntry
        BL       FlashBreak


;---------------------------------
;
; FlashWriteEntry()
; Debugger interface function
;
;---------------------------------
        SECTION .text:CODE:ROOT(2)
#if !_CORTEX_
        ARM
#else
        THUMB
#endif

FlashWriteEntry:
        BL       Fl2FlashWriteEntry
        BL       FlashBreak


;---------------------------------
;
; FlashEraseWriteEntry
; Debugger interface function
;
;---------------------------------
        SECTION .text:CODE:ROOT(2)
#if !_CORTEX_
        ARM
#else
        THUMB
#endif

FlashEraseWriteEntry:
        BL       Fl2FlashEraseWriteEntry
        BL       FlashBreak


;---------------------------------
;
; FlashChecksumEntry
; Debugger interface function
;
;---------------------------------
        SECTION .text:CODE:NOROOT(2)
#if !_CORTEX_
        ARM
#else
        THUMB
#endif

FlashChecksumEntry:
        BL       Fl2FlashChecksumEntry
        BL       FlashBreak


;---------------------------------
;
; FlashSignoffEntry
; Debugger interface function
;
;---------------------------------
        SECTION .text:CODE:NOROOT(2)
#if !_CORTEX_
        ARM
#else
        THUMB
#endif

FlashSignoffEntry:
        BL       Fl2FlashSignoffEntry
        BL       FlashBreak


;---------------------------------
;
; Flash buffer and Cortex stack
;
;---------------------------------
        SECTION LOWEND:DATA(8)
        DATA
FlashBufferStart:

        SECTION HIGHSTART:DATA
        DATA
FlashBufferEnd:


#if _CORTEX_
        PUBLIC __vector_table

        SECTION .intvec:CODE:ROOT(2)
        DATA

__vector_table:
        DC32    SFE(CSTACK)
        DC32    FlashInitEntry
        ; The following dummy entries are needed for NXP devices that requires a checksum for the range __vector_table-__vector_table+0x1B
        DC32    0
        DC32    0
        DC32    0
        DC32    0
        DC32    0
        DC32    0
#endif

        END
