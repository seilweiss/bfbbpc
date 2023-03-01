#include "zNPCTypeCommon.h"

void zNPCCommon::Init(xEntAsset* entass) WIP
{
}

void zNPCCommon::Setup() WIP
{
}

void zNPCCommon::Reset() WIP
{
}

void zNPCCommon::Process(xScene* xscn, F32 dt) WIP
{
}

void zNPCCommon::BUpdate(xVec3* pos) WIP
{
}

void zNPCCommon::NewTime(xScene* xscn, F32 dt) WIP
{
}

void zNPCCommon::Move(xScene* xscn, F32 dt, xEntFrame* frm) WIP
{
}

S32 zNPCCommon::SysEvent(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget, S32* handled) WIP
{
    return 1;
}

void zNPCCommon::CollideReview() WIP
{
}

void zNPCCommon::Destroy() WIP
{
}

S32 zNPCCommon::NPCMessage(NPCMsg* mail) WIP
{
    return 1;
}

void zNPCCommon::ParseINI() WIP
{
}

void zNPCCommon::ParseLinks() WIP
{
}

void zNPCCommon::ParseProps() WIP
{
}

void zNPCCommon::SelfDestroy() WIP
{
}

void zNPCCommon::Damage(NPC_DAMAGE_TYPE damtype, xBase* who, const xVec3* vec_hit) WIP
{
}

S32 zNPCCommon::Respawn(const xVec3* pos, zMovePoint* mvptFirst, zMovePoint* mvptSpawnRef) WIP
{
    return 1;
}

void zNPCCommon::DuploNotice(SM_NOTICES note, void* data) WIP
{
}

void zNPCCommon::LassoNotify(LASSO_EVENT event) WIP
{
}

void zNPCCommon::GetParm(npcparm pid, void* val) WIP
{
}

S32 zNPCCommon::GetParmDefault(en_npcparm pid, void* val) WIP
{
    return 1;
}

S32 zNPCCommon::LassoSetup() WIP
{
    return 1;
}