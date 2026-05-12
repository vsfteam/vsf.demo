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
        PUBLIC FlashEraseWriteEntry
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
