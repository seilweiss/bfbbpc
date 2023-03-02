#pragma once

#include "iAnim.h"

#include "xMath3.h"
#include "xMemMgr.h"
#include "xMorph.h"

struct xModelInstance;

struct xAnimAssetFile;
struct xAnimAssetState;
struct xAnimAssetTable;
struct xAnimFile;
struct xAnimMultiFileBase;
struct xAnimMultiFileEntry;
struct xAnimMultiFile;
struct xAnimState;
struct xAnimEffect;
struct xAnimActiveEffect;
struct xAnimTransition;
struct xAnimTransitionList;
struct xAnimTable;
struct xAnimSingle;
struct xAnimPlay;

typedef void(*xAnimStateCallback)(xAnimState*, xAnimSingle*, void*);
typedef U32(*xAnimEffectCallback)(U32, xAnimActiveEffect*, xAnimSingle*, void*);
typedef U32(*xAnimTransitionConditionalCallback)(xAnimTransition*, xAnimSingle*, void*);
typedef U32(*xAnimTransitionCallback)(xAnimTransition*, xAnimSingle*, void*);
typedef void(*xAnimBeforeEnterCallback)(xAnimPlay*, xAnimState*);
typedef void(*xAnimBeforeAnimMatricesCallback)(xAnimPlay*, xQuat*, xVec3*, S32);
typedef xAnimTable*(*xAnimTableConstructor)();

struct xAnimAssetFile
{
    U32 FileFlags;
    F32 Duration;
    F32 TimeOffset;
    U16 NumAnims[2];
    void** RawData;
    S32 Physics;
    S32 StartPose;
    S32 EndPose;
};

struct xAnimAssetState
{
    U32 StateID;
    U32 FileIndex;
    U32 EffectCount;
    U32 EffectOffset;
    F32 Speed;
    U32 SubStateID;
    U32 SubStateCount;
};

struct xAnimAssetEffect
{
    U32 StateID;
    F32 StartTime;
    F32 EndTime;
    U32 Flags;
    U32 EffectType;
    U32 UserDataSize;
};

struct xAnimAssetTable
{
    U32 Magic;
    U32 NumRaw;
    U32 NumFiles;
    U32 NumStates;
    U32 ConstructFunc;
};

struct xAnimFile
{
    xAnimFile* Next;
    const char* Name;
    U32 ID;
    U32 FileFlags;
    F32 Duration;
    F32 TimeOffset;
    U16 BoneCount;
    U8 NumAnims[2];
    void** RawData;
};

enum
{
    // Present in assert strings
    AnimFile_Reverse = 0x1000,
    AnimFile_BackAndForth = 0x2000,

    // Guessed based on context
    AnimFile_Bilinear = 0x4000,
    AnimFile_UseMorphSeq = 0x8000,
    AnimFile_0x10000 = 0x10000,
    AnimFile_0x20000 = 0x20000,
    AnimFile_0x40000 = 0x40000,
    AnimFile_0x80000 = 0x80000,
    AnimFile_0xF0000Mask = 0xF0000
};

#define xAnimFileGetRawDuration(afile) (((afile)->FileFlags & AnimFile_UseMorphSeq) ? xMorphSeqDuration((xMorphSeqFile*)(afile)->RawData[0]) : iAnimDuration((afile)->RawData[0]))

struct xAnimMultiFileBase
{
    U32 Count;
};

struct xAnimMultiFileEntry
{
    U32 ID;
    xAnimFile* File;
};

struct xAnimMultiFile : xAnimMultiFileBase
{
    xAnimMultiFileEntry Files[1];
};

struct xAnimState
{
    xAnimState* Next;
    const char* Name;
    U32 ID;
    U32 Flags;
    U32 UserFlags;
    F32 Speed;
    xAnimFile* Data;
    xAnimEffect* Effects;
    xAnimTransitionList* Default;
    xAnimTransitionList* List;
    F32* BoneBlend;
    F32* TimeSnap;
    F32 FadeRecip;
    U16* FadeOffset;
    void* CallbackData;
    xAnimMultiFile* MultiFile;
    xAnimBeforeEnterCallback BeforeEnter;
    xAnimStateCallback StateCallback;
    xAnimBeforeAnimMatricesCallback BeforeAnimMatrices;
};

enum
{
    // Guessed based on context
    AnimState_0xFMask = 0xF,
    AnimState_0x10 = 0x10,
    AnimState_0x20 = 0x20,
    AnimState_0x30 = 0x30,
    AnimState_0x40 = 0x40,
    AnimState_0x70Mask = 0x70,
    AnimState_0x100 = 0x100,
    AnimState_0x200 = 0x200,
    AnimState_0x400 = 0x400,
    AnimState_0x800 = 0x800,

    // Present in assert strings
    AnimState_TempUsed = 0x80000000
};

struct xAnimEffect
{
    xAnimEffect* Next;
    U32 Flags;
    F32 StartTime;
    F32 EndTime;
    xAnimEffectCallback Callback;
};

enum
{
    // Guessed based on context
    AnimEffect_0x1 = 0x1,
    AnimEffect_0x2 = 0x2,
    AnimEffect_0x4 = 0x4,
    AnimEffect_0x8 = 0x8,
    AnimEffect_0x10 = 0x10,
    AnimEffect_0x20 = 0x20
};

struct xAnimActiveEffect
{
    xAnimEffect* Effect;
    U32 Handle;
};

struct xAnimTransition
{
    xAnimTransition* Next;
    xAnimState* Dest;
    xAnimTransitionConditionalCallback Conditional;
    xAnimTransitionCallback Callback;
    U32 Flags;
    U32 UserFlags;
    F32 SrcTime;
    F32 DestTime;
    U16 Priority;
    U16 QueuePriority;
    F32 BlendRecip;
    U16* BlendOffset;
};

enum
{
    // Guessed based on context
    AnimTransition_0x2 = 0x2,
    AnimTransition_0x4 = 0x4,
    AnimTransition_0x8 = 0x8,
    AnimTransition_0x10 = 0x10,
    AnimTransition_0x20 = 0x20,
    AnimTransition_0x40 = 0x40,
    AnimTransition_0x80 = 0x80,
    AnimTransition_0x100 = 0x100,

    // Present in assert strings
    AnimTransition_TempUsed = 0x80000000
};

struct xAnimTransitionList
{
    xAnimTransitionList* Next;
    xAnimTransition* T;
};

struct xAnimTable
{
    xAnimTable* Next;
    const char* Name;
    xAnimTransition* TransitionList;
    xAnimState* StateList;
    U32 AnimIndex;
    U32 MorphIndex;
    U32 UserFlags;
};

struct xAnimSingle
{
    U32 SingleFlags;
    xAnimState* State;
    F32 Time;
    F32 CurrentSpeed;
    F32 BilinearLerp[2];
    xAnimEffect* Effect;
    U32 ActiveCount;
    F32 LastTime;
    xAnimActiveEffect* ActiveList;
    xAnimPlay* Play;
    xAnimTransition* Sync;
    xAnimTransition* Tran;
    xAnimSingle* Blend;
    F32 BlendFactor;
    U32 pad;
};

enum
{
    // Guessed based on context
    AnimSingle_0x1 = 0x1,
    AnimSingle_0x2 = 0x2,
    AnimSingle_0x4 = 0x4,
    AnimSingle_0x8000 = 0x8000
};

struct xAnimPlay
{
    xAnimPlay* Next;
    U16 NumSingle;
    U16 BoneCount;
    xAnimSingle* Single;
    void* Object;
    xAnimTable* Table;
    xMemPool* Pool;
    xModelInstance* ModelInst;
    xAnimBeforeAnimMatricesCallback BeforeAnimMatrices;
};

extern U32 gxAnimUseGrowAlloc;

#define xAnimMemAlloc(size) (gxAnimUseGrowAlloc ? xGROWALLOC((size)) : xMALLOC((size)))

void xAnimInit();
void xAnimTempTransitionInit(U32 count);
xAnimFile* xAnimFileNewBilinear(void** rawData, const char* name, U32 flags, xAnimFile** linkedList, U32 numX, U32 numY);
xAnimFile* xAnimFileNew(void* rawData, const char* name, U32 flags, xAnimFile** linkedList);
void xAnimFileSetTime(xAnimFile* data, F32 duration, F32 timeOffset);
void xAnimFileEval(xAnimFile* data, F32 time, F32* bilinear, U32 flags, xVec3* tran, xQuat* quat, F32*);
xAnimEffect* xAnimStateNewEffect(xAnimState* state, U32 flags, F32 startTime, F32 endTime, xAnimEffectCallback callback, U32 userDataSize);
xAnimTable* xAnimTableNew(const char* name, xAnimTable** linkedList, U32 userFlags);
void xAnimDefaultBeforeEnter(xAnimPlay*, xAnimState* state);
xAnimState* xAnimTableNewState(xAnimTable* table, const char* name, U32 flags, U32 userFlags, F32 speed, F32* boneBlend, F32* timeSnap, F32 fadeRecip, U16* fadeOffset, void* callbackData, xAnimBeforeEnterCallback beforeEnter, xAnimStateCallback stateCallback, xAnimBeforeAnimMatricesCallback beforeAnimMatrices);
void xAnimTableAddTransition(xAnimTable* table, xAnimTransition* tran, const char* source);
xAnimTransition* xAnimTableNewTransition(xAnimTable* table, const char* source, const char* dest, xAnimTransitionConditionalCallback conditional, xAnimTransitionCallback callback, U32 flags, U32 userFlags, F32 srcTime, F32 destTime, U16 priority, U16 queuePriority, F32 fBlendTime, U16* blendOffset);
void xAnimTableAddFile(xAnimTable* table, xAnimFile* file, const char* states);
xAnimState* xAnimTableAddFileID(xAnimTable* table, xAnimFile* file, U32 stateID, U32 subStateID, U32 subStateCount);
xAnimState* xAnimTableGetStateID(xAnimTable* table, U32 ID);
xAnimState* xAnimTableGetState(xAnimTable* table, const char* name);
void xAnimPlaySetState(xAnimSingle* single, xAnimState* state, F32 startTime);
void xAnimPlaySetup(xAnimPlay* play, void* object, xAnimTable* table, xModelInstance* modelInst);
void xAnimPlayChooseTransition(xAnimPlay* play);
void xAnimPlayStartTransition(xAnimPlay* play, xAnimTransition* transition);
void xAnimPlayUpdate(xAnimPlay* play, F32 timeDelta);
void xAnimPlayEval(xAnimPlay* play);
void xAnimPoolCB(xMemPool* pool, void* data);
void xAnimPoolInit(xMemPool* pool, U32 count, U32 singles, U32 blendFlags, U32 effectMax);
xAnimPlay* xAnimPoolAlloc(xMemPool* pool, void* object, xAnimTable* table, xModelInstance* modelInst);
void xAnimPoolFree(xAnimPlay* play);

inline F32 xAnimFileRawTime(xAnimFile* afile, F32 time)
{
    if ((afile->FileFlags & AnimFile_Reverse) ||
        ((afile->FileFlags & AnimFile_BackAndForth) &&
        time > 0.5f * afile->Duration)) {
        return afile->TimeOffset + afile->Duration - time;
    }
    return afile->TimeOffset + time;
}

#define xAnimTableNewStateDefault(table, name, flags, userFlags) xAnimTableNewState((table), (name), (flags), (userFlags), 1.0f, NULL, NULL, 0.0f, NULL, NULL, xAnimDefaultBeforeEnter, NULL, NULL)
#define xAnimTableNewTransitionDefault(table, source, dest, priority, blendRecip) xAnimTableNewTransition((table), (source), (dest), NULL, NULL, 0x10, 0, 0.0f, 0.0f, (priority), 0, (blendRecip), NULL)