#include "zCameraTweak.h"

#include "zCameraTweakAsset.h"

#include "xMath3.h"
#include "zCamera.h"
#include "zEvent.h"

struct zCamTweak
{
    U32 owner;
    F32 priority;
    F32 time;
    F32 pitch;
    F32 distMult;
};

struct zCamTweakLook
{
    F32 h;
    F32 dist;
    F32 pitch;
};

static S32 sCamTweakCount;
static zCamTweak sCamTweakList[8];
static F32 sCamTweakLerp;
static F32 sCamTweakTime;
static F32 sCamTweakPitch[2];
static F32 sCamTweakDistMult[2];
static F32 sCamTweakPitchCur;
static F32 sCamTweakDistMultCur;
static F32 sCamD;
static F32 sCamH;
static F32 sCamPitch;
static zCamTweakLook zcam_neartweak;
static zCamTweakLook zcam_fartweak;

static void zCameraTweak_LookPreCalc(zCamTweakLook* tweak, F32 d, F32 h, F32 pitch)
{
    F32 f1 = d * itan(pitch);
    tweak->h = h - f1;
    tweak->dist = xsqrt(xsqr(f1) + xsqr(d));
    tweak->pitch = pitch;
}

void zCameraTweakGlobal_Init()
{
    zCameraTweak_LookPreCalc(&zcam_neartweak, zcam_near_d, zcam_near_h, zcam_near_pitch);
    zCameraTweak_LookPreCalc(&zcam_fartweak, zcam_far_d, zcam_far_h, zcam_far_pitch);
    zCameraTweakGlobal_Reset();
}

void zCameraTweakGlobal_Add(U32 owner, F32 priority, F32 time, F32 pitch, F32 distMult) NONMATCH("https://decomp.me/scratch/Zb7MN")
{
    S32 i, j;
    
    for (i = 0; i < sCamTweakCount; i++) {
        if (owner == sCamTweakList[i].owner) return;
    }

    for (i = 0; i < sCamTweakCount; i++) {
        if (priority >= sCamTweakList[i].priority) {
            for (j = sCamTweakCount; j >= i + 1; j--) {
                sCamTweakList[j] = sCamTweakList[j-1];
            }
            break;
        }
    }

    sCamTweakList[i].owner = owner;
    sCamTweakList[i].priority = priority;
    sCamTweakList[i].time = xmax(time, 0.001f);
    sCamTweakList[i].pitch = xdeg2rad(pitch);
    sCamTweakList[i].distMult = xmax(distMult, 0.001f);
    
    sCamTweakCount++;

    if (i == 0) {
        sCamTweakPitch[1] = sCamTweakPitch[1] * sCamTweakLerp + sCamTweakPitch[0] * (1.0f - sCamTweakLerp);
        sCamTweakDistMult[1] = sCamTweakDistMult[1] * sCamTweakLerp + sCamTweakDistMult[0] * (1.0f - sCamTweakLerp);
        sCamTweakLerp = 1.0f;
        sCamTweakTime = sCamTweakList[0].time;
        sCamTweakPitch[0] = sCamTweakList[0].pitch;
        sCamTweakDistMult[0] = sCamTweakList[0].distMult;
    }
}

void zCameraTweakGlobal_Remove(U32 owner) NONMATCH("https://decomp.me/scratch/t2G0P")
{
    S32 i, j;

    for (i = 0; i < sCamTweakCount; i++) {
        if (owner == sCamTweakList[i].owner) {
            if (i == 0) {
                sCamTweakPitch[1] = sCamTweakPitch[1] * sCamTweakLerp + sCamTweakPitch[0] * (1.0f - sCamTweakLerp);
                sCamTweakDistMult[1] = sCamTweakDistMult[1] * sCamTweakLerp + sCamTweakDistMult[0] * (1.0f - sCamTweakLerp);
                sCamTweakLerp = 1.0f;
                sCamTweakTime = sCamTweakList[0].time;
        
                if (sCamTweakCount > 1) {
                    sCamTweakPitch[0] = sCamTweakList[1].pitch;
                    sCamTweakDistMult[0] = sCamTweakList[1].distMult;
                } else {
                    sCamTweakPitch[0] = 0.0f;
                    sCamTweakDistMult[0] = 1.0f;
                }
            }
        
            for (j = i; j < sCamTweakCount - 1; j++) {
                sCamTweakList[j] = sCamTweakList[j+1];
            }
        
            sCamTweakCount--;
            break;
        }
    }
}

void zCameraTweakGlobal_Reset() NONMATCH("https://decomp.me/scratch/41EJ1")
{
    sCamTweakCount = 0;
    sCamTweakPitch[0] = 0.0f;
    sCamTweakPitch[1] = 0.0f;
    sCamTweakDistMult[0] = 1.0f;
    sCamTweakDistMult[1] = 1.0f;
    sCamTweakTime = 0.1f;
    sCamTweakLerp = 0.0f;
    sCamTweakPitchCur = 0.0f;
    sCamTweakDistMultCur = 1.0f;
}

void zCameraTweakGlobal_Update(F32 dt) NONMATCH("https://decomp.me/scratch/icTAI")
{
    sCamTweakLerp -= dt / sCamTweakTime;
    if (sCamTweakLerp < 0.0f) sCamTweakLerp = 0.0f;
    
    sCamTweakPitchCur = sCamTweakPitch[1] * sCamTweakLerp + sCamTweakPitch[0] * (1.0f - sCamTweakLerp);
    sCamTweakDistMultCur = sCamTweakDistMult[1] * sCamTweakLerp + sCamTweakDistMult[0] * (1.0f - sCamTweakLerp);
    
    zCamTweakLook* tlook = &zcam_fartweak;
    if (zcam_near) tlook = &zcam_neartweak;

    sCamD = sCamTweakDistMultCur * tlook->dist * icos(tlook->pitch + sCamTweakPitchCur);
    sCamH = tlook->h + sCamTweakDistMultCur * tlook->dist * isin(tlook->pitch + sCamTweakPitchCur);
    sCamPitch = tlook->pitch + sCamTweakPitchCur;
}

F32 zCameraTweakGlobal_GetD()
{
    return sCamD;
}

F32 zCameraTweakGlobal_GetH()
{
    return sCamH;
}

F32 zCameraTweakGlobal_GetPitch()
{
    return sCamPitch;
}

void zCameraTweak_Init(xBase& data, xDynAsset& asset, size_t asset_size)
{
    zCameraTweak_Init((zCameraTweak*)&data, (CameraTweak_asset*)&asset);
}

void zCameraTweak_Init(zCameraTweak* tweak, CameraTweak_asset* asset)
{
    xBaseInit(tweak, asset);
    
    CameraTweak_asset* casset = asset;
    tweak->casset = casset;
    tweak->eventFunc = zCameraTweak_EventCB;

    if (tweak->linkCount) {
        tweak->link = (xLinkAsset*)(casset + 1);
    } else {
        tweak->link = NULL;
    }
}

void zCameraTweak_Save(zCameraTweak* tweak, xSerial* s)
{
    xBaseSave(tweak, s);
}

void zCameraTweak_Load(zCameraTweak* tweak, xSerial* s)
{
    xBaseLoad(tweak, s);
}

S32 zCameraTweak_EventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget)
{
    zCameraTweak* tweak = (zCameraTweak*)to;
    
    switch (toEvent) {
    case eEventEnable:
        xBaseEnable(tweak);
        break;
    case eEventDisable:
        xBaseDisable(tweak);
        break;
    case eEventRun:
        if (xBaseIsEnabled(tweak)) {
            zCameraTweakGlobal_Add(tweak->id,
                (F32)tweak->casset->priority,
                tweak->casset->time,
                tweak->casset->pitch_adjust,
                tweak->casset->dist_adjust);
        }
        break;
    case eEventStop:
        if (xBaseIsEnabled(tweak)) {
            zCameraTweakGlobal_Remove(tweak->id);
        }
        break;
    }

    return 1;
}