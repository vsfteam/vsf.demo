#include "flash_loader.h"
#include "flash_loader_extra.h"

#ifndef MAX_ARGS
#define MAX_ARGS 7
#endif

// Maximum combined size of arguments, including a trailing null for each argument.
#ifndef MAX_ARG_SIZE
#define MAX_ARG_SIZE 64
#endif

// Functions in this file, called from the assembly wrapper
void Fl2FlashInitEntry(void);
void Fl2FlashWriteEntry(void);
void Fl2FlashEraseWriteEntry(void);
void Fl2FlashChecksumEntry(void);
void Fl2FlashSignoffEntry(void);
void FlashBreak(void);

#if CODE_ADDR_AS_VOID_PTR
extern uint32_t FlashChecksum(void const *begin, uint32_t count);
#else
extern uint32_t FlashChecksum(uint32_t begin, uint32_t count);
#endif
extern uint32_t FlashSignoff();

uint16_t Crc16_helper(uint8_t const *p, uint32_t len, uint16_t sum);

// Flashloader framework version advertised to C-SPY. The debugger looks up
// this symbol to select the parameter-passing ABI: 200 = framework2, which
// passes parameters through theFlashParams (populated by C-SPY before every
// RPC). Without this symbol C-SPY falls back to the legacy ABI (parameters
// in registers), and the asm wrappers here only read theFlashParams, so
// theFlashParams.count stays at whatever FlashInit wrote last -- which is
// exactly the "FlashWrite always sees cnt=0" symptom observed without it.
__root const uint16_t frameworkVersion = 200;

__root __no_init FlashParamsHolder theFlashParams;

__no_init int __argc;
__no_init char __argvbuf[MAX_ARG_SIZE];
#pragma required=__argvbuf
__no_init const char* __argv[MAX_ARGS];

#if CODE_ADDR_AS_VOID_PTR
#define CODE_REF void *
#else
#define CODE_REF uint32_t
#endif

void Fl2FlashInitEntry()
{
#if USE_ARGC_ARGV
  theFlashParams.count = FlashInit((CODE_REF)theFlashParams.base_ptr,
                                   theFlashParams.block_size,       // Image size
                                   theFlashParams.offset_into_block,// link adr
                                   theFlashParams.count,            // flags
                                   __argc,
                                   __argv);
#else
  theFlashParams.count = FlashInit((CODE_REF)theFlashParams.base_ptr,
                                   theFlashParams.block_size,       // Image size
                                   theFlashParams.offset_into_block,// link adr
                                   theFlashParams.count);           // flags
#endif
}

// The normal flash write function ----------------------------------------------
void Fl2FlashWriteEntry()
{
  theFlashParams.count = FlashWrite((CODE_REF)theFlashParams.base_ptr,
                                    theFlashParams.offset_into_block,
                                    theFlashParams.count,
                                    theFlashParams.buffer);
}

// The erase-first flash write function -----------------------------------------
//
// IAR 9.60 C-SPY oddity (empirically confirmed via two probes):
//   probe 1 (count = 0xDEADBEEF) failed       -> EraseWrite RPC IS invoked
//   probe 2 (standard impl, trusts base_ptr / offset / block_size from
//            theFlashParams every call)      -> ALSO failed
// Conclusion: under the current .flash config (page=4096, FLAG_ERASE_ONLY
// in FlashInit) C-SPY does not refresh base_ptr / offset_into_block /
// count for the EraseWrite RPC -- only block_size and the buffer contents.
// We therefore maintain our own block cursor: capture base_ptr at the very
// first call (Init-time value) and advance by block_size every subsequent
// call, treating each invocation as "erase + write one whole block worth
// of data starting from that running address". This matches what C-SPY
// actually does (it refills the buffer for each block).
// Also: the size argument to FlashWrite must be block_size (not count),
// because count is the byte-count input for the standalone Write RPC and
// is not refreshed for EraseWrite.
static uint32_t g_ew_cursor    = 0;  // running offset from Init base_ptr
static uint32_t g_ew_init_base = 0;  // captured from first call
static uint32_t g_ew_calls     = 0;  // used to detect the very first call

void Fl2FlashEraseWriteEntry()
{
  if (g_ew_calls == 0) {
    g_ew_init_base = theFlashParams.base_ptr;
    g_ew_cursor    = 0;
  }

  uint32_t tmp = theFlashParams.block_size;
  uint32_t cur_base = g_ew_init_base + g_ew_cursor;

  g_ew_calls++;

  if (tmp == 0)
  {
    FlashEraseData *p = (FlashEraseData*)theFlashParams.buffer;
    for (uint32_t i = 0; i < theFlashParams.count; ++i)
    {
      tmp = FlashErase((CODE_REF)p->start, p->length);
      if (tmp != 0) break;
      ++p;
    }
  }
  else
  {
    tmp = FlashErase((CODE_REF)cur_base, theFlashParams.block_size);
    if (tmp == 0)
    {
      tmp = FlashWrite((CODE_REF)cur_base, 0,
                       theFlashParams.block_size,
                       theFlashParams.buffer);
    }
    if (tmp == 0) {
      g_ew_cursor += theFlashParams.block_size;
    }
  }
  theFlashParams.count = tmp;
}


void Fl2FlashChecksumEntry()
{
  theFlashParams.count = FlashChecksum((CODE_REF)theFlashParams.base_ptr,
                                       theFlashParams.count);
}

void Fl2FlashSignoffEntry()
{
  theFlashParams.count = FlashSignoff();
}


uint16_t Crc16(uint8_t const *p, uint32_t len)
{
  uint8_t zero[2] = { 0, 0 };
  uint16_t sum = Crc16_helper(p, len, 0);
  return Crc16_helper(zero, 2, sum);
}

uint16_t Crc16_helper(uint8_t const *p, uint32_t len, uint16_t sum)
{
  while (len--)
  {
    int i;
    uint8_t byte = *p++;

    for (i = 0; i < 8; ++i)
    {
      uint32_t osum = sum;
      sum <<= 1;
      if (byte & 0x80)
        sum |= 1 ;
      if (osum & 0x8000)
        sum ^= 0x1021;
      byte <<= 1;
    }
  }
  return sum;
}

#pragma optimize=no_inline
__root void FlashBreak()
{
  while(1);
}
