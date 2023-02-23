#include "zAssetTypes.h"

#include "xstransvc.h"

#include <rwcore.h>
#include <rpworld.h>

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

static void* Model_Read(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize) WIP
{
    return NULL;
}

static void* Curve_Read(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize) WIP
{
    return NULL;
}

static void Model_Unload(void* userdata, U32 assetid) WIP
{
}

static void* BSP_Read(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize) WIP
{
    return NULL;
}

static void BSP_Unload(void* userdata, U32 assetid) WIP
{
}

static void* JSP_Read(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize) WIP
{
    return NULL;
}

static void JSP_Unload(void* userdata, U32 assetid) WIP
{
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

static void ATBL_Init() WIP
{
}

static void* ATBL_Read(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize) WIP
{
    return NULL;
}

static void Anim_Unload(void* userdata, U32 assetid) WIP
{
}

static void LightKit_Unload(void* userdata, U32 assetid) WIP
{
}

static void MovePoint_Unload(void* userdata, U32 assetid) WIP
{
}

static void* SndInfoRead(void* userdata, U32 assetid, void* indata, U32 insize, U32* outsize) WIP
{
    return NULL;
}