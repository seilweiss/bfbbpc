#pragma once

#include "xEnt.h"
#include "zNPCCarry.h"
#include "zNPCDamage.h"

class zNPCCommon;
struct zMovePoint;

enum en_NPC_MSG_ID
{
    NPC_MID_NONE,
    NPC_MID_SYSEVENT,
    NPC_MID_RESPAWN,
    NPC_MID_DAMAGE,
    NPC_MID_EXPLOSION,
    NPC_MID_BUNNYHOP,
    NPC_MID_DTRON_NPCEMIT,
    NPC_MID_DTRON_NPCAVAIL,
    NPC_MID_DTRON_ISDEAD,
    NPC_MID_CHAT_REQUEST,
    NPC_MID_CHAT_DECLINE,
    NPC_MID_CHAT_ACCEPT,
    NPC_MID_CHAT_DONE,
    NPC_MID_CHAT_ABORT,
    NPC_MID_TALKSTART,
    NPC_MID_TALKON,
    NPC_MID_TALKOFF,
    NPC_MID_SAWPLYR,
    NPC_MID_NEEDMEDIC,
    NPC_MID_UNDERATTACK,
    NPC_MID_NPCDIED,
    NPC_MID_PLYRSPATULA,
    NPC_MID_BECOMESCARED,
    NPC_MID_NOLONGERSCARED,
    NPC_MID_CELEBRATE,
    NPC_MID_STUN,
    NPC_MID_SCRIPTBEGIN,
    NPC_MID_SCRIPTEND,
    NPC_MID_SCRIPTHALT,
    NPC_MID_DEV_ANIMSPIN,
    NPC_MID_DEV_ANIMCYCLE,
    NPC_MID_DEV_HEROMODE,
    NPC_MID_DEV_DONE,
    NPC_MID_NOMORE,
    NPC_MID_FORCE = FORCEENUMSIZEINT
};

enum en_NPC_MSG_DATA
{
    NPC_MDAT_BLANK,
    NPC_MDAT_SYSEVENT,
    NPC_MDAT_BLAST,
    NPC_MDAT_CHAT,
    NPC_MDAT_SPAWN,
    NPC_MDAT_TARGET,
    NPC_MDAT_DAMAGE,
    NPC_MDAT_STUN,
    NPC_MDAT_SCRIPT,
    NPC_MDAT_MOUNT,
    NPC_MDAT_AREANOTIFY,
    NPC_MDAT_NOMORE,
    NPC_MDAT_FORCE = FORCEENUMSIZEINT
};

struct NPCSysEvent
{
    S32 doLinkEvents;
    S32 handled;
    xBase* from;
    xBase* to;
    U32 toEvent;
    F32 toParam[4];
    xBase* toParamWidget;
};

struct NPCBlastInfo
{
    xVec3 pos_blast;
    F32 rad_blast;
    F32 spd_expand;
};

struct NPCChatInfo
{
    xVec3 pos_chat;
    F32 tym_chat;
};

struct NPCSpawnInfo
{
    xVec3 pos_spawn;
    zMovePoint* nav_firstMovepoint;
    zMovePoint* nav_spawnReference;
    S32 spawnSuccess;
};

struct NPCTargetInfo
{
    xBase* bas_tgt;
    xVec3 pos_tgt;
};

struct NPCDamageInfo
{
    en_NPC_DAMAGE_TYPE dmg_type;
    xBase* dmg_from;
    xVec3 vec_dmghit;
};

struct NPCStunInfo
{
    F32 tym_stuntime;
    en_NPC_CARRY_STATE carrystate;
    S32 allowStun;
};

struct NPCScriptInfo
{
    U32 aid_playanim;
};

struct NPCMountInfo
{
    xEnt* ent_toMount;
    xCollis* col_forMount;
};

struct NPCAreaInfo
{
    zNPCCommon* npc_origin;
    xVec3 pos_origin;
};

struct NPCMsg
{
    en_NPC_MSG_ID msgid;
    U32 sendto;
    U32 from;
    en_NPC_MSG_DATA infotype;
    union
    {
        NPCSysEvent sysevent;
        NPCBlastInfo blastarea;
        NPCChatInfo chatter;
        NPCSpawnInfo spawning;
        NPCTargetInfo target;
        NPCDamageInfo dmgdata;
        NPCStunInfo stundata;
        NPCScriptInfo scriptdata;
        NPCMountInfo mountdata;
        NPCAreaInfo areadata;
    };
    void* attached;
    NPCMsg* next;
    F32 tmr_delay;
};