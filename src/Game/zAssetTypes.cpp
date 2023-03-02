#include "zAssetTypes.h"

#include "iModel.h"
#include "xstransvc.h"
#include "xJSP.h"
#include "xAnim.h"
#include "xHudModel.h"
#include "xString.h"
#include "zEnt.h"
#include "zEntPlayerAnimationTables.h"
#include "zNPCTypeCommon.h"
#include "zNPCTypeVillager.h"
#include "zNPCTypeRobot.h"
#include "zNPCTypeAmbient.h"
#include "zNPCTypeTiki.h"
#include "zNPCTypeDuplotron.h"
#include "zNPCTypeTest.h"
#include "zNPCTypeKingJelly.h"
#include "zNPCTypeDutchman.h"
#include "zNPCTypePrawn.h"
#include "zNPCTypeBossSandy.h"
#include "zNPCTypeBossPatrick.h"
#include "zNPCTypeBossSB1.h"
#include "zNPCTypeBossSB2.h"
#include "zNPCTypeBoss.h"
#include "zNPCTypeBossPlankton.h"
#include "zNPCHazard.h"
#include "zEntCruiseBubble.h"

#include <rwcore.h>
#include <rpworld.h>
#include <string.h>

static void* Model_Read(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize);
static void* Curve_Read(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize);
static void Model_Unload(void* userdata, U32 assetid);
static void* BSP_Read(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize);
static void BSP_Unload(void* userdata, U32 assetid);
static void* JSP_Read(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize);
static void JSP_Unload(void* userdata, U32 assetid);
static void* RWTX_Read(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize);
static void TextureRW3_Unload(void* userdata, U32 assetid);
static void* ATBL_Read(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize);
static void Anim_Unload(void* userdata, U32 assetid);
static void LightKit_Unload(void* userdata, U32 assetid);
static void MovePoint_Unload(void* userdata, U32 assetid);
static void* SndInfoRead(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize);

static PKRAssetType assetTypeHandlers[] WIP =
{
    { 'BSP ', 0, 0, BSP_Read, NULL, NULL, NULL, NULL, BSP_Unload, NULL },
    { 'JSP ', 0, 0, JSP_Read, NULL, NULL, NULL, NULL, JSP_Unload, NULL },
    { 'TXD ', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'MODL', 0, 0, Model_Read, NULL, NULL, NULL, NULL, Model_Unload, NULL },
    { 'ANIM', 0, 0, NULL, NULL, NULL, NULL, NULL, Anim_Unload, NULL },
    { 'RWTX', 0, 0, RWTX_Read, NULL, NULL, NULL, NULL, TextureRW3_Unload, NULL },
    { 'LKIT', 0, 0, NULL, NULL, NULL, NULL, NULL, LightKit_Unload, NULL },
    { 'CAM ', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'PLYR', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'NPC ', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'ITEM', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'PKUP', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'TRIG', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'SDF ', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'TEX ', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'TXT ', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'ENV ', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'ATBL', 0, 0, ATBL_Read, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'MINF', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'PICK', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'PLAT', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'PEND', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'MRKR', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'MVPT', 0, 0, NULL, NULL, NULL, NULL, NULL, MovePoint_Unload, NULL },
    { 'TIMR', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'CNTR', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'PORT', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'SND ', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'SNDS', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'GRUP', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'MPHT', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'SFX ', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'SNDI', 0, 0, SndInfoRead, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'HANG', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'SIMP', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'BUTN', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'SURF', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'DSTR', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'BOUL', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'MAPR', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'GUST', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'VOLU', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'UI  ', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'UIFT', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'TEXT', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'COND', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'DPAT', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'PRJT', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'LOBM', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'FOG ', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'LITE', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'PARE', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'PARS', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'CSN ', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'CTOC', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'CSNM', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'EGEN', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'ALST', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'RAW ', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'LODT', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'SHDW', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'DYNA', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'VIL ', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'VILP', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'COLL', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'PARP', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'PIPT', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'DSCO', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'JAW ', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'SHRP', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'FLY ', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'TRCK', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'CRV ', 0, 0, Curve_Read, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'ZLIN', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'DUPC', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'SLID', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 'CRDT', 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { 0 },
};

struct HackModelRadius
{
    U32 assetid;
    F32 radius;
};

static HackModelRadius hackRadiusTable[] = {
    { 0xFA77E6FA, 20.0f },
    { 0x5BD0EDAC, 1000.0f },
    { 0xED21A1C6, 50.0f }
};

static xJSPHeader* sTempJSP;
static xJSPHeader sDummyEmptyJSP;

static const char* jsp_shadow_hack_textures[] = {
    "beach_towel",
    "wood_board_Nails_singleV2",
    "wood_board_Nails_singleV3",
    "glass_broken",
    "ground_path_alpha"
};
static const char** jsp_shadow_hack_end_textures = jsp_shadow_hack_textures + ARRAY_LENGTH(jsp_shadow_hack_textures);

struct AnimTableList
{
    const char* name;
    xAnimTableConstructor constructor;
    U32 id;
};

#define ANIMTABLE(name) { #name, name, 0 }

AnimTableList animTable[] = {
    ANIMTABLE(ZNPC_AnimTable_Test),
    ANIMTABLE(ZNPC_AnimTable_Dutchman),
    ANIMTABLE(ZNPC_AnimTable_Duplotron),
    ANIMTABLE(ZNPC_AnimTable_Common),
    ANIMTABLE(ZNPC_AnimTable_BossPlankton),
    ANIMTABLE(ZNPC_AnimTable_BossSandy),
    ANIMTABLE(ZNPC_AnimTable_SleepyTime),
    ANIMTABLE(ZNPC_AnimTable_BossSandyHead),
    ANIMTABLE(ZNPC_AnimTable_Hammer),
    ANIMTABLE(ZNPC_AnimTable_TTSauce),
    ANIMTABLE(ZNPC_AnimTable_KingJelly),
    ANIMTABLE(ZNPC_AnimTable_Slick),
    ANIMTABLE(ZNPC_AnimTable_TarTar),
    ANIMTABLE(ZNPC_AnimTable_Villager),
    ANIMTABLE(ZNPC_AnimTable_BalloonBoy),
    ANIMTABLE(ZNPC_AnimTable_Fodder),
    ANIMTABLE(ZNPC_AnimTable_Prawn),
    ANIMTABLE(ZNPC_AnimTable_Neptune),
    ANIMTABLE(ZNPC_AnimTable_BossSB1),
    ANIMTABLE(ZNPC_AnimTable_BossSBobbyArm),
    ANIMTABLE(ZNPC_AnimTable_Monsoon),
    ANIMTABLE(ZNPC_AnimTable_ArfDog),
    ANIMTABLE(ZNPC_AnimTable_ArfArf),
    ANIMTABLE(ZNPC_AnimTable_BossSB2),
    ANIMTABLE(ZNPC_AnimTable_Tiki),
    ANIMTABLE(ZNPC_AnimTable_Tubelet),
    ANIMTABLE(ZNPC_AnimTable_Ambient),
    ANIMTABLE(ZNPC_AnimTable_GLove),
    ANIMTABLE(ZNPC_AnimTable_LassoGuide),
    ANIMTABLE(ZNPC_AnimTable_Chuck),
    ANIMTABLE(ZNPC_AnimTable_Jelly),
    ANIMTABLE(ZNPC_AnimTable_SuperFriend),
    ANIMTABLE(ZNPC_AnimTable_BossPatrick),
};

static xAnimTableConstructor tableFuncList[] = {
    zEntPlayer_AnimTable,
    ZNPC_AnimTable_Common,
    zPatrick_AnimTable,
    zSandy_AnimTable,
    ZNPC_AnimTable_Villager,
    zSpongeBobTongue_AnimTable,
    ZNPC_AnimTable_LassoGuide,
    ZNPC_AnimTable_Hammer,
    ZNPC_AnimTable_TarTar,
    ZNPC_AnimTable_GLove,
    ZNPC_AnimTable_Monsoon,
    ZNPC_AnimTable_SleepyTime,
    ZNPC_AnimTable_ArfDog,
    ZNPC_AnimTable_ArfArf,
    ZNPC_AnimTable_Chuck,
    ZNPC_AnimTable_Tubelet,
    ZNPC_AnimTable_Slick,
    ZNPC_AnimTable_Ambient,
    ZNPC_AnimTable_Tiki,
    ZNPC_AnimTable_Fodder,
    ZNPC_AnimTable_Duplotron,
    ZNPC_AnimTable_Jelly,
    ZNPC_AnimTable_Test,
    ZNPC_AnimTable_Neptune,
    ZNPC_AnimTable_KingJelly,
    ZNPC_AnimTable_Dutchman,
    ZNPC_AnimTable_Prawn,
    ZNPC_AnimTable_BossSandy,
    ZNPC_AnimTable_BossPatrick,
    ZNPC_AnimTable_BossSB1,
    ZNPC_AnimTable_BossSB2,
    ZNPC_AnimTable_BossSBobbyArm,
    ZNPC_AnimTable_BossPlankton,
    zEntPlayer_BoulderVehicleAnimTable,
    ZNPC_AnimTable_BossSandyHead,
    ZNPC_AnimTable_BalloonBoy,
    xEnt_AnimTable_AutoEventSmall,
    ZNPC_AnimTable_SlickShield,
    ZNPC_AnimTable_SuperFriend,
    ZNPC_AnimTable_ThunderCloud,
    XHUD_AnimTable_Idle,
    ZNPC_AnimTable_NightLight,
    ZNPC_AnimTable_HazardStd,
    ZNPC_AnimTable_FloatDevice,
    cruise_bubble::anim_table,
    ZNPC_AnimTable_BossSandyScoreboard,
    zEntPlayer_TreeDomeSBAnimTable,
    NULL
};

static U32 s_sbFootSoundA;
static U32 s_sbFootSoundB;
static U32 s_scFootSoundA;
static U32 s_scFootSoundB;
static U32 s_patFootSoundA;
static U32 s_patFootSoundB;

static U32 dummyEffectCB(U32, xAnimActiveEffect*, xAnimSingle*, void*);
static U32 soundEffectCB(U32 cbenum, xAnimActiveEffect* acteffect, xAnimSingle*, void* object);

static xAnimEffectCallback effectFuncList[] = {
    dummyEffectCB,
    soundEffectCB,
};

static void ATBL_Init();

void zAssetStartup()
{
    xSTStartup(assetTypeHandlers);
    ATBL_Init();
}

void zAssetShutdown()
{
    xSTShutdown();
}

static void* Model_Read(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize) NONMATCH("https://decomp.me/scratch/RHvJ7")
{
    RpAtomic* model = iModelFileNew(indata, insize);
    *outsize = sizeof(RpAtomic);

    for (U32 i = 0; i < ARRAY_LENGTH(hackRadiusTable); i++) {
        if (assetid == hackRadiusTable[i].assetid) {
            RpAtomic* tmpModel = model;
            while (tmpModel) {
                tmpModel->boundingSphere.radius = hackRadiusTable[i].radius;
                tmpModel->boundingSphere.center.x = 0.0f;
                tmpModel->boundingSphere.center.y = 0.0f;
                tmpModel->boundingSphere.center.z = 0.0f;
                tmpModel->interpolator.flags &= ~rpINTERPOLATORDIRTYSPHERE;
                tmpModel = iModelFile_RWMultiAtomic(tmpModel);
            }
            break;
        }
    }
    
    return model;
}

static void* Curve_Read(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize) WIP
{
    return NULL;
}

static void Model_Unload(void* userdata, U32 assetid)
{
    if (userdata) {
        iModelUnload((RpAtomic*)userdata);
    }
}

static void* BSP_Read(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize) WIP
{
    return NULL;
}

static void BSP_Unload(void* userdata, U32 assetid) WIP
{
}

inline bool jsp_shadow_hack_match(RpAtomic* atomic)
{
    RpGeometry* geom = RpAtomicGetGeometry(atomic);
    S32 num = RpGeometryGetNumMaterials(geom);
    const char** cur = jsp_shadow_hack_textures;
    const char** end = jsp_shadow_hack_end_textures;
    while (cur != end) {
        const char* name = *cur;
        for (S32 i = 0; i < num; i++) {
            RpMaterial* mat = RpGeometryGetMaterial(geom, i);
            RwTexture* tex = RpMaterialGetTexture(mat);
            if (tex) {
                const char* texname = RwTextureGetName(tex);
                if (texname && !stricmp(texname, name)) {
                    return 1;
                }
            }
        }
        cur++;
    }
    return 0;
}

struct jsp_shadow_hack_atomic_context
{
    xJSPHeader* jsp;
    S32 index;
    S32 last_material;
};

static RpAtomic* jsp_shadow_hack_atomic_cb(RpAtomic* atomic, void* data)
{
    jsp_shadow_hack_atomic_context& context = *(jsp_shadow_hack_atomic_context*)data;
    S32 index = context.index++;
    
    if (!jsp_shadow_hack_match(atomic)) {
        return atomic;
    }

    xClumpCollBSPTree* colltree = context.jsp->colltree;
    
    S32 material_index = context.jsp->jspNodeList[index].originalMatIndex;
    if (material_index == context.last_material) {
        return atomic;
    }
    context.last_material = material_index;
    
    xClumpCollBSPTriangle* tri = colltree->triangles;
    xClumpCollBSPTriangle* end_tri = tri + colltree->numTriangles;
    while (tri != end_tri) {
        if (tri->matIndex == material_index) {
            tri->flags |= 0x20;
        }
        tri++;
    }

    return atomic;
}

static void jsp_shadow_hack(xJSPHeader* jsp)
{
    if (!jsp || !jsp->clump || !jsp->colltree) return;

    jsp_shadow_hack_atomic_context context = { jsp, 0, -1 };
    RpClumpForAllAtomics(jsp->clump, jsp_shadow_hack_atomic_cb, &context);
}

static void* JSP_Read(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize)
{
    xJSPHeader* retjsp = &sDummyEmptyJSP;
    *outsize = sizeof(xJSPHeaderEx);

    xJSP_MultiStreamRead(indata, insize, &sTempJSP);

    if (sTempJSP->jspNodeList) {
        retjsp = sTempJSP;
        sTempJSP = NULL;
        *outsize = sizeof(xJSPHeader*); // is this correct?
    }

    jsp_shadow_hack(retjsp);

    return retjsp;
}

static void JSP_Unload(void* userdata, U32 assetid)
{
    if (userdata != &sDummyEmptyJSP) {
        xJSP_Destroy((xJSPHeader*)userdata);
    }
}

static RwTexture* TexCB(RwTexture* texture, void* data)
{
    RwTexture** texFound = (RwTexture**)data;
    if (!*texFound) *texFound = texture;
    return texture;
}

static void* RWTX_Read(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize) WIP
{
    RwTexDictionary* txd;
    RwMemory rwmem;
    RwStream* stream;
    RwTexture* tex;
    RwError error;

    tex = NULL;

    if (insize) {
        rwmem.start = (RwUInt8*)indata;
        rwmem.length = insize;

        stream = RwStreamOpen(rwSTREAMMEMORY, rwSTREAMREAD, &rwmem);

        if (stream) {
            if (!RwStreamFindChunk(stream, rwID_TEXDICTIONARY, NULL, NULL)) {
                RwErrorGet(&error);

                RwStreamFindChunk(stream, rwID_TEXDICTIONARY, NULL, NULL);
                RwStreamClose(stream, NULL);
            } else {
                txd = RwTexDictionaryStreamRead(stream);

                RwStreamClose(stream, NULL);

                if (txd) {
                    RwTexDictionaryForAllTextures(txd, TexCB, &tex);

                    if (!tex) {
                        RwTexDictionaryDestroy(txd);
                    } else {
                        RwTexDictionaryRemoveTexture(tex);
                        RwTexDictionaryDestroy(txd);

                        RwTextureAddRef(tex);
                        RwTextureSetFilterMode(tex, rwFILTERLINEARMIPLINEAR);

                        *outsize = sizeof(RwTexture);
                        return tex;
                    }
                }
            }
        }
    }

    *outsize = insize;
    return NULL;
}

static void TextureRW3_Unload(void* userdata, U32 assetid) WIP
{
    RwTexture* tex = (RwTexture*)userdata;
    if (tex) {
        tex->refCount = 1;
        RwTextureDestroy(tex);
    }
}

static void ATBL_Init()
{
    for (S32 i = 0; i < (S32)ARRAY_LENGTH(animTable); i++) {
        animTable[i].id = xStrHash(animTable[i].name);
    }
}

void FootstepHackSceneEnter()
{
    s_sbFootSoundA = xStrHash("SB_run1L");
    s_sbFootSoundB = xStrHash("SB_run1R");
    s_scFootSoundA = xStrHash("SC_run_kelpL");
    s_scFootSoundB = xStrHash("SC_run_kelpL");
    s_patFootSoundA = xStrHash("Pat_run_rock_dryL");
    s_patFootSoundB = xStrHash("Pat_run_rock_dryR");
}

static U32 dummyEffectCB(U32, xAnimActiveEffect*, xAnimSingle*, void*)
{
    return 0;
}

static U32 soundEffectCB(U32 cbenum, xAnimActiveEffect* acteffect, xAnimSingle*, void* object) WIP
{
    return 0;
}

static void* FindAssetCB(U32 ID, char*)
{
    U32 size;
    return xSTFindAsset(ID, &size);
}

static xAnimTable* Anim_ATBL_getTable(xAnimTableConstructor func);

static void* ATBL_Read(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize) NONMATCH("https://decomp.me/scratch/IEjPg")
{
    U32 i, j;
    U32 debugNum = 0;
    U32 tmpsize;
    xAnimTable* table;
    xAnimState* astate;
    xAnimTransition* atran;
    U8* zaBytes = (U8*)indata;
    xAnimAssetTable* zaTbl = (xAnimAssetTable*)indata;
    void** zaRaw = (void**)(zaTbl + 1);
    xAnimAssetFile* zaFile = (xAnimAssetFile*)(zaRaw + zaTbl->NumRaw);
    xAnimAssetState* zaState = (xAnimAssetState*)(zaFile + zaTbl->NumFiles);
    
    for (i = 0; i < zaTbl->NumRaw; i++) {
        zaRaw[i] = xSTFindAsset((U32)zaRaw[i], &tmpsize);
    }

    for (i = 0; i < zaTbl->NumRaw; i++) {
        if (!zaRaw[i]) {
            for (j = 0; j < zaTbl->NumRaw; j++) {
                if (zaRaw[j]) {
                    zaRaw[i] = zaRaw[j];
                    break;
                }
            }
        }
    }

    for (i = 0; i < zaTbl->NumRaw; i++) {
        if (*(U32*)zaRaw[i] == 'QSPM') {
            xMorphSeqSetup(zaRaw[i], FindAssetCB);
        }
    }

    for (i = 0; i < zaTbl->NumFiles; i++) {
        zaFile[i].RawData = (void**)(zaBytes + (U32)zaFile[i].RawData);
        for (S32 k = 0; k < zaFile[i].NumAnims[0] * zaFile[i].NumAnims[1]; k++) {
            zaFile[i].RawData[k] = zaRaw[(U32)zaFile[i].RawData[k]];
        }
    }

    xAnimFile** fList = (xAnimFile**)zaFile;
    for (i = 0; i < zaTbl->NumFiles; i++) {
        fList[i] = xAnimFileNewBilinear(
            zaFile[i].RawData, "", zaFile[i].FileFlags, NULL,
            zaFile[i].NumAnims[0], zaFile[i].NumAnims[1]);
        if (zaFile[i].TimeOffset >= 0.0f) {
            xAnimFileSetTime(fList[i], zaFile[i].Duration, zaFile[i].TimeOffset);
        }
    }

    xAnimTableConstructor constructor = NULL;
    if (zaTbl->ConstructFunc < ARRAY_LENGTH(tableFuncList)) {
        constructor = tableFuncList[zaTbl->ConstructFunc];
    } else {
        for (S32 i = 0; i < (S32)ARRAY_LENGTH(animTable); i++) {
            if (zaTbl->ConstructFunc == animTable[i].id) {
                constructor = animTable[i].constructor;
                break;
            }
        }
    }

    gxAnimUseGrowAlloc = 1;
    table = Anim_ATBL_getTable(constructor);

    for (i = 0; i < zaTbl->NumStates; i++) {
        astate = xAnimTableAddFileID(
            table, fList[zaState[i].FileIndex],
            zaState[i].StateID, zaState[i].SubStateID, zaState[i].SubStateCount);
        if (!astate) {
            char tmpstr[32];
            sprintf(tmpstr, "Debug%02d", debugNum++);
            astate = xAnimTableNewStateDefault(table, tmpstr, 0x20, 0x80000000);
            atran = xAnimTableNewTransitionDefault(table, tmpstr, NULL, 0, 0.2f);
            atran->Dest = table->StateList;
            xAnimTableAddFileID(table, fList[zaState[i].FileIndex], astate->ID, 0, 0);
        }
        astate->Speed = zaState[i].Speed;
    }

    xAnimFile* foundFile = NULL;
    astate = table->StateList;
    while (astate) {
        if (!foundFile && astate->Data) {
            foundFile = astate->Data;
        }
        astate = astate->Next;
    }

    astate = table->StateList;
    while (astate) {
        if (!astate->Data) {
            astate->Data = foundFile;
            astate->UserFlags |= 0x40000000;
        }
        astate = astate->Next;
    }

    for (i = 0; i < zaTbl->NumStates; i++) {
        if (zaState[i].EffectCount) {
            xAnimState* state = xAnimTableGetStateID(table, zaState[i].StateID);
            xAnimAssetEffect* zaEffect = (xAnimAssetEffect*)(zaBytes + zaState[i].EffectOffset);
            if (state) {
                for (j = 0; j < zaState[i].EffectCount; j++) {
                    xAnimEffect* effect = xAnimStateNewEffect(
                        state, zaEffect->Flags,
                        zaEffect->StartTime, zaEffect->EndTime,
                        effectFuncList[zaEffect->EffectType],
                        zaEffect->UserDataSize);
                    memcpy((void*)(effect + 1), (void*)(zaEffect + 1), zaEffect->UserDataSize);
                    zaEffect = (xAnimAssetEffect*)((U8*)zaEffect + zaEffect->UserDataSize + sizeof(xAnimAssetEffect));
                }
            }
        }
    }

    gxAnimUseGrowAlloc = 0;
    *outsize = sizeof(xAnimTable);
    
    return table;
}

static void Anim_Unload(void* userdata, U32 assetid)
{
}

static void LightKit_Unload(void* userdata, U32 assetid) WIP
{
}

static xAnimTable* Anim_ATBL_getTable(xAnimTableConstructor func)
{
    return func();
}

static void MovePoint_Unload(void* userdata, U32 assetid) WIP
{
}

static void* SndInfoRead(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize) WIP
{
    return NULL;
}