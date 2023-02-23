#pragma once

#include "iPad.h"
#include "xMath2.h"

enum _tagRumbleType
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
};
typedef enum _tagRumbleType xRumbleType;

typedef struct _tagxRumble xRumble;
struct _tagxRumble
{
    xRumbleType type;
    F32 seconds;
    xRumble* next;
    S16 active;
    U16 fxflags;
};

typedef struct _tagPadAnalog xPadAnalog;
struct _tagPadAnalog
{
    S8 x;
    S8 y;
};

enum _tagPadState
{
    ePad_Disabled,
    ePad_DisabledError,
    ePad_Enabled,
    ePad_Missing,
    ePad_Total
};
typedef enum _tagPadState xPadState;

typedef struct _tagxPad xPad;
struct _tagxPad
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
};

extern xPad* gDebugPad;
extern xPad* gPlayerPad;

S32 xPadInit();
xPad* xPadEnable(S32 idx);
void xPadRumbleEnable(S32 idx, S32 enable);
void xPadKill();