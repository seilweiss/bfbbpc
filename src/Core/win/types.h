#pragma once

#include <stdint.h>

#define GAME_NAME "SpongeBob SquarePants: Battle for Bikini Bottom"

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#define LITTLE_ENDIAN

#define VBLANKS_PER_SEC 60
#define SECS_PER_VBLANK (1.0f/VBLANKS_PER_SEC)
#define FB_XRES 640
#define FB_YRES 480
#define FB_DEPTH 24

typedef int8_t S8;
typedef int16_t S16;
typedef int32_t S32;
typedef int64_t S64;
typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef float F32;
typedef double F64;

#ifndef NULL
#define NULL 0L
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE !FALSE
#endif

#define FORCEENUMSIZEINT ((S32)((~((U32)0))>>1))

#define MACROSTART do {
#define MACROEND } while (0)

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof(a[0]))

// Decomp-related annotations
// (these don't do anything to the code, but they can be Ctrl+F'd easily)
#define WIP // Function is not fully implemented/decomped yet
#define NONMATCH(decompme) // Function is fully decomped, but not matching (must have decomp.me link!)

#define ENABLE_WIP_CODE 0 // Use to disable part of a function, usually code that depends on unimplemented functions