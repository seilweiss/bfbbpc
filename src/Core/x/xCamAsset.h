#pragma once

#include "xBaseAsset.h"
#include "xVec3.h"

typedef enum _tagTransType
{
    eTransType_None,
    eTransType_Interp1,
    eTransType_Interp2,
    eTransType_Interp3,
    eTransType_Interp4,
    eTransType_Linear,
    eTransType_Interp1Rev,
    eTransType_Interp2Rev,
    eTransType_Interp3Rev,
    eTransType_Interp4Rev,
    eTransType_Total
} xTransType;

typedef struct _tagxCamFollowAsset
{
    F32 rotation;
    F32 distance;
    F32 height;
    F32 rubber_band;
    F32 start_speed;
    F32 end_speed;
} xCamFollowAsset;

typedef struct _tagxCamShoulderAsset
{
    F32 distance;
    F32 height;
    F32 realign_speed;
    F32 realign_delay;
} xCamShoulderAsset;

typedef struct _tagp2CamStaticAsset
{
    U32 unused;
} xCamStaticAsset;

typedef struct _tagxCamPathAsset
{
    U32 assetID;
    F32 time_end;
    F32 time_delay;
} xCamPathAsset;

typedef struct _tagp2CamStaticFollowAsset
{
    F32 rubber_band;
} xCamStaticFollowAsset;

struct xCamAsset : xBaseAsset
{
    xVec3 pos;
    xVec3 at;
    xVec3 up;
    xVec3 right;
    xVec3 view_offset;
    S16 offset_start_frames;
    S16 offset_end_frames;
    F32 fov;
    F32 trans_time;
    xTransType trans_type;
    U32 flags;
    F32 fade_up;
    F32 fade_down;
    union
    {
        xCamFollowAsset cam_follow;
        xCamShoulderAsset cam_shoulder;
        xCamStaticAsset cam_static;
        xCamPathAsset cam_path;
        xCamStaticFollowAsset cam_staticFollow;
    };
    U32 valid_flags;
    U32 markerid[2];
    U8 cam_type;
    U8 pad[3];
};