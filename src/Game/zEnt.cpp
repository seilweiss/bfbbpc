#include "zEnt.h"

#include "xModelAsset.h"
#include "xString.h"
#include "xEvent.h"
#include "xstransvc.h"
#include "xSnd.h"
#include "xutil.h"
#include "zAnimList.h"
#include "zGlobals.h"
#include "zEvent.h"

#define XENT_ANIMTABLE_NUMANIMS 5

U32 g_hash_xentanim[XENT_ANIMTABLE_NUMANIMS] = {};

char* g_strz_xentanim[XENT_ANIMTABLE_NUMANIMS] = {
    "Idle01",
    "Anim02",
    "Anim03",
    "Anim04",
    "Anim05"
};

ShadowParams gShadowParams[5] = {
    { 'NTR0', 0.33f, 2.0f },
    { 'NTR3', 0.0f, 1.25f },
    { 'NTFA', -0.25f, 0.75f },
    { 'NTR5', 0.5f, 1.0f },
    { 'NTB0', 1.0f, 2.0f }
};

inline void checkpoint_collision_hack(zEnt* ent) NONMATCH("https://decomp.me/scratch/etmNs")
{
    static const U32 model_id = xStrHash("checkpoint_bind");
    if (ent->asset->modelInfoID != model_id) return;

    static const F32 xlower = -0.5f;
    static const F32 xupper = 0.5f;
    static const F32 ylower = 0.0f;
    static const F32 yupper = 5.0f;

    ent->bound.type = k_XBOUNDTYPE_BOX;

    xBound& bound = ent->bound;
    xVec3& lower = ent->bound.box.box.lower;
    xVec3& upper = ent->bound.box.box.upper;

    lower = upper = xEntGetFrame(ent)->pos;
    lower.x += xlower;
    lower.z += xlower;
    upper.x += xupper;
    upper.y += yupper;
    upper.z += xupper;

    xEntDefaultBoundUpdate(ent, &xEntGetFrame(ent)->pos);

    ent->miscflags |= 0x8;

    xEntAnimateCollision(*ent, false);
    xModelAnimCollStop(ent->collModel ? *ent->collModel : *ent->model);

    ent->moreFlags &= (U8)~(0x2 | k_MORE_FLAGS_ANIM_COLL);
    ent->asset->moreFlags &= (U8)~(0x2 | k_MORE_FLAGS_ANIM_COLL);
    ent->baseFlags &= (U16)~k_XBASE_RECEIVES_SHADOWS;
    ent->asset->baseFlags &= (U16)~k_XBASE_RECEIVES_SHADOWS;
}

void zEntInit(zEnt* ent, xEntAsset* asset, U32 type) NONMATCH("https://decomp.me/scratch/ThRy6")
{
    xEntInit(ent, asset);

    ent->update = (xEntUpdateCallback)zEntUpdate;

    if (type == 'PLYR') {
        ent->collType = k_XENT_COLLTYPE_PC;
        ent->collLev = 4;
        ent->bound.type = k_XBOUNDTYPE_SPHERE;
        zEntParseModelInfo(ent, asset->modelInfoID);
    } else if (type == 'VIL ') {
        ent->collType = k_XENT_COLLTYPE_NPC;
        ent->collLev = 4;
        ent->bound.type = k_XBOUNDTYPE_SPHERE;
        ent->moreFlags |= k_MORE_FLAGS_HITTABLE;
        zEntParseModelInfo(ent, asset->modelInfoID);
    } else if (type == 'ITEM') {
        ent->collType = k_XENT_COLLTYPE_STAT;
        ent->collLev = 4;
        ent->bound.type = k_XBOUNDTYPE_SPHERE;
        ent->eventFunc = NULL;
        zEntParseModelInfo(ent, asset->modelInfoID);
    } else if (type == 'PKUP') {
        ent->collType = 0;
        ent->bound.type = k_XBOUNDTYPE_NONE;
    } else if (type == 'PLAT') {
        ent->collType = k_XENT_COLLTYPE_DYN;
        if (asset->moreFlags & 0x2) {
            ent->collLev = 5;
        } else {
            ent->collLev = 4;
        }
        zEntParseModelInfo(ent, asset->modelInfoID);
        ent->bound.type = k_XBOUNDTYPE_OBB;
        ent->bound.mat = (xMat4x3*)ent->model->Mat;
    } else if (type == 'PEND') {
        ent->collType = k_XENT_COLLTYPE_DYN;
        if (asset->moreFlags & 0x2) {
            ent->collLev = 5;
        } else {
            ent->collLev = 4;
        }
        zEntParseModelInfo(ent, asset->modelInfoID);
        ent->bound.type = k_XBOUNDTYPE_OBB;
        ent->bound.mat = (xMat4x3*)ent->model->Mat;
    } else if (type == 'TRIG') {
        ent->collType = k_XENT_COLLTYPE_TRIG;
        ent->bound.type = k_XBOUNDTYPE_NONE;
    } else if (type == 'HANG') {
        ent->collType = k_XENT_COLLTYPE_DYN;
        ent->collLev = 4;
        ent->bound.type = k_XBOUNDTYPE_SPHERE;
        zEntParseModelInfo(ent, asset->modelInfoID);
    } else if (type == 'SIMP') {
        ent->collType = k_XENT_COLLTYPE_STAT;
        if (asset->moreFlags & 0x2) {
            ent->collLev = 5;
        } else {
            ent->collLev = 4;
        }
        ent->bound.type = k_XBOUNDTYPE_OBB;
        zEntParseModelInfo(ent, asset->modelInfoID);
        ent->bound.mat = (xMat4x3*)ent->model->Mat;
    } else if (type == 'UI  ') {
    } else if (type == 'BUTN') {
        ent->collType = k_XENT_COLLTYPE_DYN;
        if (asset->moreFlags & 0x2) {
            ent->collLev = 5;
        } else {
            ent->collLev = 4;
        }
        ent->bound.type = k_XBOUNDTYPE_OBB;
        ent->moreFlags |= k_MORE_FLAGS_HITTABLE;
        zEntParseModelInfo(ent, asset->modelInfoID);
        ent->bound.mat = (xMat4x3*)ent->model->Mat;
    } else if (type == 'DSTR') {
        ent->collType = k_XENT_COLLTYPE_DYN;
        if (asset->moreFlags & 0x2) {
            ent->collLev = 5;
        } else {
            ent->collLev = 4;
        }
        ent->bound.type = k_XBOUNDTYPE_OBB;
        ent->moreFlags |= k_MORE_FLAGS_HITTABLE;
        zEntParseModelInfo(ent, asset->modelInfoID);
        ent->bound.mat = (xMat4x3*)ent->model->Mat;
    } else if (type == 'EGEN') {
        ent->collType = k_XENT_COLLTYPE_DYN;
        if (asset->moreFlags & 0x2) {
            ent->collLev = 5;
        } else {
            ent->collLev = 4;
        }
        ent->bound.type = k_XBOUNDTYPE_OBB;
        zEntParseModelInfo(ent, asset->modelInfoID);
        ent->bound.mat = (xMat4x3*)ent->model->Mat;
    } else if (type == 'TBOX') {
        ent->collType = k_XENT_COLLTYPE_STAT;
        ent->collLev = 5;
        ent->bound.type = k_XBOUNDTYPE_OBB;
        zEntParseModelInfo(ent, asset->modelInfoID);
        ent->bound.mat = (xMat4x3*)ent->model->Mat;
    }

    if (asset->animListID != 0) {
        S32 num_used = zAnimListGetNumUsed(asset->animListID);
        if (num_used > 0) {
            ent->atbl = zAnimListGetTable(asset->animListID);
            xAnimPoolAlloc(&globals.sceneCur->mempool, ent, ent->atbl, ent->model);
            xAnimState* ast = xAnimTableGetState(ent->atbl, "idle");
            if (ast) {
                xAnimSingle* single = ent->model->Anim->Single;
                single->State = ast;
                single->Time = 0.0f;
                single->CurrentSpeed = 1.0f;
                xModelEval(ent->model);
            }
        } else {
            ent->atbl = NULL;
        }
    } else {
        ent->atbl = NULL;
    }

    xEntInitForType(ent);
}

void zEntSetup(zEnt* ent)
{
    xEntSetup(ent);
    checkpoint_collision_hack(ent);
}

void zEntSave(zEnt* ent, xSerial* s)
{
    xEntSave(ent, s);
}

void zEntLoad(zEnt* ent, xSerial* s)
{
    xEntLoad(ent, s);
}

void zEntReset(zEnt* ent) NONMATCH("https://decomp.me/scratch/UbEr5")
{
    xEntReset(ent);

    if (ent->asset->animListID != 0 && ent->atbl) {
        xAnimState* ast = xAnimTableGetState(ent->atbl, "idle");
        if (ast) {
            xAnimSingle* single = ent->model->Anim->Single;
            single->State = ast;
            single->Time = 0.0f;
            single->CurrentSpeed = 1.0f;
            xModelEval(ent->model);
        }
    }
    
    if (!(ent->miscflags & 0x1) &&
        ent->asset->modelInfoID != 0 &&
        ent->model &&
        ent->model->Anim &&
        ent->model->Anim->Table &&
        !strcmp("xEntAutoEventSimple", ent->model->Anim->Table->Name)) {
        xAnimPlaySetState(ent->model->Anim->Single, ent->model->Anim->Table->StateList, 0.0f);
        ent->miscflags |= 0x1;
    }

    checkpoint_collision_hack(ent);
}

void zEntUpdate(zEnt* ent, zScene* scene, F32 elapsedSec)
{
    xEntUpdate(ent, scene, elapsedSec);
}

void zEntEventAll(xBase* from, U32 fromEvent, U32 toEvent, F32* toParam)
{
    zScene* s = globals.sceneCur;
    for (U16 i = 0; i < s->num_ents; i++) {
        zEntEvent(from, fromEvent, s->ents[i], toEvent, toParam, NULL, 0);
    }
}

void zEntEventAllOfType(xBase* from, U32 fromEvent, U32 toEvent, F32* toParam, U32 type)
{
    zScene* s = globals.sceneCur;
    if (!s) return;

    for (U16 i = 0; i < s->num_ents; i++) {
        if (type == s->ents[i]->baseType) {
            zEntEvent(from, fromEvent, s->ents[i], toEvent, toParam, NULL, 0);
        }
    }
}

void zEntEventAllOfType(U32 toEvent, U32 type) WIP
{
    zEntEventAllOfType(NULL, 0, toEvent, NULL, type);
}

xModelInstance* zEntRecurseModelInfo(void* info, xEnt* ent)
{
    U32 i, bufsize;
    RpAtomic* imodel;
    xModelInstance* tempInst[64];
    xModelAssetInfo* zinfo = (xModelAssetInfo*)info;
    xModelAssetInst* zinst = (xModelAssetInst*)(zinfo + 1);
    xAnimTable* table;

    for (i = 0; i < zinfo->NumModelInst; i++) {
        imodel = (RpAtomic*)xSTFindAsset(zinst[i].ModelID, &bufsize);
        if (*(U32*)imodel == 'FNIM') {
            tempInst[i] = zEntRecurseModelInfo(imodel, ent);
            if (i != 0) {
                tempInst[i]->Flags |= zinst[i].Flags;
                tempInst[i]->BoneIndex = zinst[i].Bone;
                xModelInstanceAttach(tempInst[i], tempInst[zinst[i].Parent]);
            }
        } else if (i == 0) {
            tempInst[i] = xModelInstanceAlloc(imodel, ent, 0, 0, NULL);
            tempInst[i]->modelID = zinst[i].ModelID;
            while (imodel = iModelFile_RWMultiAtomic(imodel)) {
                xModelInstanceAttach(xModelInstanceAlloc(imodel, ent, 0x2000, 0, NULL), tempInst[i]);
            }
        } else {
            tempInst[i] = xModelInstanceAlloc(imodel, ent, zinst[i].Flags, zinst[i].Bone, NULL);
            xModelInstanceAttach(tempInst[i], tempInst[zinst[i].Parent]);
            while (imodel = iModelFile_RWMultiAtomic(imodel)) {
                xModelInstanceAttach(xModelInstanceAlloc(imodel, ent, 0x2000, 0, NULL), tempInst[i]);
            }
        }
    }

    if (zinfo->AnimTableID != 0) {
        table = (xAnimTable*)xSTFindAsset(zinfo->AnimTableID, &bufsize);
        tempInst[0]->Anim = xAnimPoolAlloc(&globals.sceneCur->mempool, ent, table, tempInst[0]);
    }

    return tempInst[0];
}

void zEntParseModelInfo(xEnt* ent, U32 assetID)
{
    U32 bufsize;
    void* info = xSTFindAsset(assetID, &bufsize);
    if (*(U32*)info == 'FNIM') {
        ent->model = zEntRecurseModelInfo(info, ent);
    } else {
        xEntLoadModel(ent, (RpAtomic*)info);
        ent->model->modelID = assetID;
    }
}

void zEntAnimEvent(zEnt* ent, U32 animEvent, const F32* animParam) NONMATCH("https://decomp.me/scratch/0N4dV")
{
    xAnimPlay* play = ent->model->Anim;
    if (!play) return;
    
    xAnimSingle* single = play->Single;
    if (!single) return;

    if (ent->miscflags & 0x1) {
        zEntAnimEvent_AutoAnim(ent, animEvent, animParam);
        return;
    }

    switch (animEvent) {
    case eEventAnimPlay:
    case eEventAnimPlayLoop:
    {
        if (!animParam) break;
        
        S32 anum = (S32)animParam[0] - 1;
        if (anum < 0) break;
        if (anum >= 10) break;

        if (!ent->atbl) break;

        char name[12];
        if (animEvent == eEventAnimPlayLoop) {
            sprintf(name, "loop%d", anum);
        } else {
            sprintf(name, "stop%d", anum);
        }

        xAnimState* ast = xAnimTableGetState(ent->atbl, name);
        if (!ast) break;

        xAnimPlaySetState(single, ast, 0.0f);
        single->CurrentSpeed = 1.0f;
        xAnimPlayUpdate(play, 0.0f);
        xAnimPlayEval(play);

        if (ent->asset->modelInfoID == 0x8D398D0C && animEvent == eEventAnimPlay) {
            xSndPlay3D(xStrHash("Check1"), 0.77f, 0.0f, 0x80, 0, (xVec3*)&ent->model->Mat->pos, 0.0f, SND_CAT_GAME, 0.0f);
        }

        break;
    }
    case eEventAnimStop:
    {
        if (!strcmp(single->State->Name, "idle")) break;

        char name[12];
        strcpy(name, single->State->Name);
        name[0] = 's';
        name[1] = 't';

        xAnimState* ast = xAnimTableGetState(ent->atbl, name);
        single->State = ast;

        break;
    }
    case eEventAnimPause:
    {
        single->CurrentSpeed = 0.0f;
        break;
    }
    case eEventAnimResume:
    {
        single->CurrentSpeed = 1.0f;
        break;
    }
    case eEventAnimTogglePause:
    {
        if (single->CurrentSpeed) {
            single->CurrentSpeed = 0.0f;
        } else {
            single->CurrentSpeed = 1.0f;
        }
        break;
    }
    case eEventAnimPlayRandom:
    {
        if (!animParam) break;

        S32 anum1 = (S32)animParam[0] - 1;
        S32 anum2 = (S32)animParam[1] - 1;

        if (anum1 < 0) break;
        if (anum1 > anum2) break;
        if (anum2 >= 10) break;
        
        if (!ent->atbl) break;

        S32 anum = anum1 + ((S32)xrand() % (anum2 - anum1 + 1));
        char name[12];
        sprintf(name, "stop%d", anum);

        xAnimState* ast = xAnimTableGetState(ent->atbl, name);
        if (!ast) break;

        xAnimPlaySetState(single, ast, 0.0f);
        single->CurrentSpeed = 1.0f;
        xAnimPlayUpdate(play, 0.0f);
        xAnimPlayEval(play);

        break;
    }
    case eEventAnimPlayMaybe:
    {
        if (!animParam) break;

        S32 anum = (S32)animParam[0] - 1;
        F32 prob = 0.01f * animParam[1];
        if (anum < 0) break;
        if (anum >= 10) break;
        
        if (xurand() < prob) {
            if (!ent->atbl) break;
    
            char name[12];
            sprintf(name, "stop%d", anum);
    
            xAnimState* ast = xAnimTableGetState(ent->atbl, name);
            if (!ast) break;
    
            single->CurrentSpeed = 1.0f;
            xAnimPlaySetState(single, ast, 0.0f);
            xAnimPlayUpdate(play, 0.0f);
            xAnimPlayEval(play);
        }

        break;
    }
    }
}

xAnimTable* xEnt_AnimTable_AutoEventSmall() NONMATCH("https://decomp.me/scratch/tOyfF")
{
    S32 i;
    char** names = g_strz_xentanim;
    xAnimTransition* deftran = NULL;
    xAnimTable* table;

    if (g_hash_xentanim[0] == 0) {
        for (i = 0; i < XENT_ANIMTABLE_NUMANIMS; i++) {
            g_hash_xentanim[i] = xStrHash(g_strz_xentanim[i]);
        }
    }

    table = xAnimTableNew("xEntAutoEventSimple", NULL, 0);
    for (i = 0; i < XENT_ANIMTABLE_NUMANIMS; i++) {
        if (i == 0) {
            xAnimTableNewStateDefault(table, names[i], 0x10, 0x1);
        } else {
            xAnimTableNewStateDefault(table, names[i], 0x20, 0x1);
        }
        if (deftran) {
            xAnimTableAddTransition(table, deftran, names[i]);
        } else if (i != 0) {
            deftran = xAnimTableNewTransitionDefault(table, names[i], names[0], 1, 0.25f);
        }
    }

    return table;
}

void zEntAnimEvent_AutoAnim(zEnt* ent, U32 animEvent, const F32* animParam) NONMATCH("https://decomp.me/scratch/GShT6")
{
    xAnimPlay* play = ent->model->Anim;
    xAnimSingle* single = ent->model->Anim->Single;

    switch (animEvent) {
    case eEventAnimPlay:
    case eEventAnimPlayLoop:
    {
        if (!animParam) break;

        S32 anum = (S32)animParam[0] - 1;
        if (anum < 0) break;
        if (anum >= 5) break;

        xAnimTable* tab = ent->model->Anim->Table;
        if (!tab) break;

        xAnimState* ast = xAnimTableGetStateID(tab, g_hash_xentanim[anum]);
        if (!ast) break;
        
        if (anum != 0) {
            if (animEvent == eEventAnimPlayLoop) {
                ast->Flags &= ~0x20;
                ast->Flags |= 0x10;
            } else {
                ast->Flags |= 0x20;
                ast->Flags &= ~0x10;
            }
        }

        xAnimPlaySetState(single, ast, 0.0f);
        single->CurrentSpeed = 1.0f;
        xAnimPlayUpdate(play, 0.0f);
        xAnimPlayEval(play);

        break;
    }
    case eEventAnimStop:
    {
        xAnimTable* tab = ent->model->Anim->Table;
        if (!tab) break;

        xAnimState* ast = xAnimTableGetStateID(tab, g_hash_xentanim[0]);
        if (!ast) break;

        xAnimPlaySetState(single, ast, 0.0f);
        single->CurrentSpeed = 0.0f;
        xAnimPlayUpdate(play, 0.0f);
        xAnimPlayEval(play);

        break;
    }
    case eEventAnimPause:
    {
        single->CurrentSpeed = 0.0f;
        break;
    }
    case eEventAnimResume:
    {
        single->CurrentSpeed = 1.0f;
        break;
    }
    case eEventAnimTogglePause:
    {
        if (single->CurrentSpeed > 0.0f) {
            single->CurrentSpeed = 0.0f;
        } else {
            single->CurrentSpeed = 1.0f;
        }
        break;
    }
    case eEventAnimPlayRandom:
    {
        if (!animParam) break;

        S32 anum1 = (S32)animParam[0] - 1;
        S32 anum2 = (S32)animParam[1] - 1;

        if (anum1 < 0) break;
        if (anum2 < 0) break;
        if (anum1 > anum2) break;
        if (anum2 >= XENT_ANIMTABLE_NUMANIMS) break;

        S32 anum = anum1 + ((S32)xrand() % (anum2 - anum1 + 1));
        
        xAnimTable* tab = ent->model->Anim->Table;
        if (!tab) break;

        xAnimState* ast = xAnimTableGetStateID(tab, g_hash_xentanim[anum]);
        if (!ast) break;

        if (anum != 0) {
            ast->Flags |= 0x20;
            ast->Flags &= ~0x10;
        }

        xAnimPlaySetState(single, ast, 0.0f);
        single->CurrentSpeed = 1.0f;
        xAnimPlayUpdate(play, 0.0f);
        xAnimPlayEval(play);
        
        break;
    }
    case eEventAnimPlayMaybe:
    {
        F32 prob = 0.01f * animParam[1];
        if (xUtil_yesno(prob)) {
            zEntAnimEvent_AutoAnim(ent, eEventAnimPlay, animParam);
        }
        break;
    }
    }
}

xModelAssetParam* zEntGetModelParams(U32 assetID, U32* size)
{
    U32 bufsize;
    U32 tempsize = 0;
    void* info = xSTFindAsset(assetID, &bufsize);

    if (*(U32*)info == 'FNIM') {
        xModelAssetInfo* minf = (xModelAssetInfo*)info;
        tempsize = bufsize - (minf->NumModelInst * sizeof(xModelAssetInst) + sizeof(xModelAssetInfo));
        *size = tempsize;
        if (*size) {
            return (xModelAssetParam*)((U8*)minf + minf->NumModelInst * sizeof(xModelAssetInst) + sizeof(xModelAssetInfo));
        }
    } else {
        *size = 0;
    }

    return NULL;
}

char* zParamGetString(xModelAssetParam* param, U32 size, char* tok, char* def)
{
    U32 testhash = xStrHash(tok);
    while (param && size) {
        if (param->HashID == testhash) {
            return (char*)param->String;
        }
        size -= param->WordLength * sizeof(U32) + sizeof(xModelAssetParam);
        param = (xModelAssetParam*)((U8*)param + param->WordLength * sizeof(U32) + sizeof(xModelAssetParam));
    }
    return def;
}

S32 zParamGetInt(xModelAssetParam* param, U32 size, const char* tok, S32 def)
{
    return zParamGetInt(param, size, (char*)tok, def);
}

S32 zParamGetInt(xModelAssetParam* param, U32 size, char* tok, S32 def)
{
    char* str = zParamGetString(param, size, tok, NULL);
    if (str) {
        return atoi(str);
    }
    return def;
}

F32 zParamGetFloat(xModelAssetParam* param, U32 size, const char* tok, F32 def)
{
    return zParamGetFloat(param, size, (char*)tok, def);
}

F32 zParamGetFloat(xModelAssetParam* param, U32 size, char* tok, F32 def)
{
    char* str = zParamGetString(param, size, tok, NULL);
    if (str) {
        return xatof(str);
    }
    return def;
}

S32 zParamGetFloatList(xModelAssetParam* param, U32 size, const char* tok, S32 count, F32* def, F32* result)
{
    return zParamGetFloatList(param, size, (char*)tok, count, def, result);
}

S32 zParamGetFloatList(xModelAssetParam* param, U32 size, char* tok, S32 count, F32* def, F32* result)
{
    char* str = zParamGetString(param, size, tok, NULL);
    S32 act = 0;
    if (def) {
        for (S32 i = 0; i < count; i++) {
            result[i] = def[i];
        }
    }
    if (str) {
        act = xStrParseFloatList(result, str, count);
    }
    return act;
}

S32 zParamGetVector(xModelAssetParam* param, U32 size, const char* tok, xVec3 def, xVec3* result)
{
    return zParamGetVector(param, size, (char*)tok, def, result);
}

S32 zParamGetVector(xModelAssetParam* param, U32 size, char* tok, xVec3 def, xVec3* result)
{
    char* str = zParamGetString(param, size, tok, NULL);
    S32 act = 0;
    F32 fltbuf[3] = {};

    xVec3Copy(result, &def);

    if (str) {
        act = xStrParseFloatList(fltbuf, str, 3);
        if (act > 0) result->x = fltbuf[0];
        if (act > 1) result->y = fltbuf[1];
        if (act > 2) result->z = fltbuf[2];
    }

    return act;
}

void zEntGetShadowParams(xEnt* ent, xVec3* center, F32* radius, xEntShadow::radius_enum rtype) WIP
{
}