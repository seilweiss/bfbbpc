#pragma once

#include "iPad.h"
#include "xMath2.h"

typedef enum _tagRumbleType
{
    eRumble_Off,
    eRumble_Hi,
    eRumble_VeryLightHi,
    eRumble_VeryLight,
    eRumble_LightHi,
    eRumble_Light,
    eRumble_MediumHi,
    eRumble_Medium,
    eRumble_HeavyHi,
    eRumble_Heavy,
    eRumble_VeryHeavyHi,
    eRumble_VeryHeavy,
    eRumble_Total,
    eRumbleForceU32 = FORCEENUMSIZEINT
} xRumbleType;

typedef struct _tagxRumble
{
    xRumbleType type;
    F32 seconds;
    _tagxRumble* next;
    S16 active;
    U16 fxflags;
} xRumble;

typedef struct _tagPadAnalog
{
    S8 x;
    S8 y;
} xPadAnalog;

typedef enum _tagPadState
{
    ePad_Disabled,
    ePad_DisabledError,
    ePad_Enabled,
    ePad_Missing,
    ePad_Total
} xPadState;

typedef struct _tagxPad
{
    struct analog_data
    {
        xVec2 offset;
        xVec2 dir;
        F32 mag;
        F32 ang;
    };

    U8 value[22];
    U8 last_value[22];
    U32 on;
    U32 pressed;
    U32 released;
    xPadAnalog analog1;
    xPadAnalog analog2;
    xPadState state;
    U32 flags;
    xRumble rumble_head;
    S16 port;
    S16 slot;
    iPad context;
    F32 al2d_timer;
    F32 ar2d_timer;
    F32 d_timer;
    F32 up_tmr[22];
    F32 down_tmr[22];
    analog_data analog[2];
} xPad;

// NOTE: Probably wrong. These are from Ratatouille Proto
#define k_XPAD_START  ((U32)(1 << 0))  // 0x1
#define k_XPAD_SELECT ((U32)(1 << 1))  // 0x2
#define k_XPAD_UP     ((U32)(1 << 4))  // 0x10
#define k_XPAD_RIGHT  ((U32)(1 << 5))  // 0x20
#define k_XPAD_DOWN   ((U32)(1 << 6))  // 0x40
#define k_XPAD_LEFT   ((U32)(1 << 7))  // 0x80
#define k_XPAD_L1     ((U32)(1 << 8))  // 0x100
#define k_XPAD_L2     ((U32)(1 << 9))  // 0x200
#define k_XPAD_L3     ((U32)(1 << 10)) // 0x400
#define k_XPAD_R1     ((U32)(1 << 12)) // 0x1000
#define k_XPAD_R2     ((U32)(1 << 13)) // 0x2000
#define k_XPAD_R3     ((U32)(1 << 14)) // 0x4000
#define k_XPAD_A      ((U32)(1 << 16)) // 0x10000
#define k_XPAD_B      ((U32)(1 << 17)) // 0x20000
#define k_XPAD_C      ((U32)(1 << 18)) // 0x40000
#define k_XPAD_D      ((U32)(1 << 19)) // 0x80000
#define k_XPAD_E      ((U32)(1 << 20)) // 0x100000
#define k_XPAD_F      ((U32)(1 << 21)) // 0x200000

extern xPad* gDebugPad;
extern xPad* gPlayerPad;

S32 xPadInit();
xPad* xPadEnable(S32 idx);
void xPadRumbleEnable(S32 idx, S32 enable);
void xPadKill();