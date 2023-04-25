#pragma once

#include "xBase.h"
#include "xDynAsset.h"

struct ztextbox;
struct zNPCCommon;

struct ztalkbox : xBase
{
    struct asset_type : xDynAsset
    {
        U32 dialog_box;
        U32 prompt_box;
        U32 quit_box;
        bool trap : 8;
        bool pause : 8;
        bool allow_quit : 8;
        bool trigger_pads : 8;
        bool page : 8;
        bool show : 8;
        bool hide : 8;
        bool audio_effect : 8;
        U32 teleport;
        struct
        {
            struct
            {
                bool time : 8;
                bool prompt : 8;
                bool sound : 8;
                bool event : 8;
            } type;
            F32 delay;
            S32 which_event;
        } auto_wait;
        struct
        {
            U32 skip;
            U32 noskip;
            U32 quit;
            U32 noquit;
            U32 yesno;
        } prompt;
    };

    struct
    {
        bool visible : 1;
    } flag;
    const asset_type* asset;
    ztextbox* dialog_box;
    ztextbox* prompt_box;
    ztextbox* quit_box;
    struct
    {
        const char* skip;
        const char* noskip;
        const char* quit;
        const char* noquit;
        const char* yesno;
    } prompt;
    zNPCCommon* npc;

    static ztalkbox* get_active();
};