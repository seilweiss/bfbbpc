#pragma once

#include "xFile.h"

struct xCamera;

struct xCutsceneInfo
{
    U32 Magic;
    U32 AssetID;
    U32 NumData;
    U32 NumTime;
    U32 MaxModel;
    U32 MaxBufEven;
    U32 MaxBufOdd;
    U32 HeaderSize;
    U32 VisCount;
    U32 VisSize;
    U32 BreakCount;
    U32 pad;
    char SoundLeft[16];
    char SoundRight[16];
};

struct xCutsceneData
{
    U32 DataType;
    U32 AssetID;
    U32 ChunkSize;
    union
    {
        U32 FileOffset;
        void* DataPtr;
    };
};

struct xCutsceneBreak
{
    F32 Time;
    S32 Index;
};

struct xCutsceneTime
{
    F32 StartTime;
    F32 EndTime;
    U32 NumData;
    U32 ChunkIndex;
};

struct XCSNNosey
{
    void* userdata;
    S32 flg_nosey;
};

struct xCutscene
{
    xCutsceneInfo* Info;
    xCutsceneData* Data;
    U32* TimeChunkOffs;
    U32* Visibility;
    xCutsceneBreak* BreakList;
    xCutsceneTime* Play;
    xCutsceneTime* Stream;
    U32 Waiting;
    U32 BadReadPause;
    F32 BadReadSpeed;
    void* RawBuf;
    void* AlignBuf;
    F32 Time;
    F32 CamTime;
    U32 PlayIndex;
    U32 Ready;
    S32 DataLoading;
    U32 GotData;
    U32 ShutDownWait;
    F32 PlaybackSpeed;
    U32 Opened;
    xFile File;
    S32 AsyncID;
    void* MemBuf;
    void* MemCurr;
    U32 SndStarted;
    U32 SndNumChannel;
    U32 SndChannelReq[2];
    U32 SndAssetID[2];
    U32 SndHandle[2];
    XCSNNosey* cb_nosey;
};

struct xCutsceneZbuffer
{
    F32 start;
    F32 end;
    F32 nearPlane;
    F32 farPlane;
};

struct xCutsceneZbufferHack
{
    char* name;
    xCutsceneZbuffer times[4];
};

void xCutscene_SetCamera(xCutscene* csn, xCamera* cam);