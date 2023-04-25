#pragma once

#include "xhipio.h"
#include "xordarray.h"

#include <time.h>

#define PKR_LIB_VER 1 // actual name unknown
#define PKR_PKG_VER 'HIPA' // actual name unknown
#define PKR_SUB_VER 2 // actual name unknown
#define PKR_COMPAT_VER 1 // actual name unknown

#define PKR_MAX_INST 16 // actual name unknown
#define PKR_ASS_NAMLEN 32 // actual name unknown
#define PKR_MAX_NAMLEN 128
#define PKR_MAX_ARRAY 4096
#define PKR_MAX_TYPES 128
#define PKR_DUMBNUM 0x33333333

struct st_PACKER_READ_DATA;
struct st_PACKER_LTOC_NODE;

enum en_LAYER_TYPE
{
    PKR_LTYPE_ALL = -1,
    PKR_LTYPE_DEFAULT = 0,
    PKR_LTYPE_TEXTURE,
    PKR_LTYPE_BSP,
    PKR_LTYPE_MODEL,
    PKR_LTYPE_ANIMATION,
    PKR_LTYPE_VRAM,
    PKR_LTYPE_SRAM,
    PKR_LTYPE_SNDTOC,
    PKR_LTYPE_CUTSCENE,
    PKR_LTYPE_CUTSCENETOC,
    PKR_LTYPE_JSPINFO,
    PKR_LTYPE_NOMORE
};

struct st_PACKER_ASSETTYPE
{
    U32 typetag;
    U32 tflags;
    S32 typalign;
    void*(*readXForm)(void*, U32, void*, U32, U32*);
    void*(*writeXForm)(void*, U32, void*, void*, U32, U32*);
    S32(*assetLoaded)(void*, U32, void*, S32);
    void*(*makeData)(void*, U32, void*, S32*, S32*);
    void(*cleanup)(void*, U32, void*);
    void(*assetUnloaded)(void*, U32);
    void(*writePeek)(void*, U32, void*, char*);
};

struct st_PKR_ASSET_TOCINFO
{
    U32 aid;
    st_PACKER_ASSETTYPE* typeref;
    U32 sector;
    U32 plus_offset;
    U32 size;
    void* mempos;
};

struct st_PACKER_READ_FUNCS
{
    U32 api_ver;
    st_PACKER_READ_DATA*(*Init)(void*, char*, U32, S32*, st_PACKER_ASSETTYPE*);
    void(*Done)(st_PACKER_READ_DATA*);
    S32(*LoadLayer)(st_PACKER_READ_DATA*, en_LAYER_TYPE);
    U32(*GetAssetSize)(st_PACKER_READ_DATA*, U32);
    void*(*LoadAsset)(st_PACKER_READ_DATA*, U32, const char*, void*);
    void*(*AssetByType)(st_PACKER_READ_DATA*, U32, S32, U32*);
    S32(*AssetCount)(st_PACKER_READ_DATA*, U32);
    S32(*IsAssetReady)(st_PACKER_READ_DATA*, U32);
    S32(*SetActive)(st_PACKER_READ_DATA*, en_LAYER_TYPE);
    const char*(*AssetName)(st_PACKER_READ_DATA*, U32);
    U32(*GetBaseSector)(st_PACKER_READ_DATA*);
    S32(*GetAssetInfo)(st_PACKER_READ_DATA*, U32, st_PKR_ASSET_TOCINFO*);
    S32(*GetAssetInfoByType)(st_PACKER_READ_DATA*, U32, S32, st_PKR_ASSET_TOCINFO*);
    S32(*PkgHasAsset)(st_PACKER_READ_DATA*, U32);
    U32(*PkgTimeStamp)(st_PACKER_READ_DATA*);
    void(*PkgDisconnect)(st_PACKER_READ_DATA*);
};

extern S32 pkr_sector_size;

st_PACKER_READ_FUNCS* PKRGetReadFuncs(S32 apiver);
S32 PKRStartup();
S32 PKRShutdown();
S32 PKRLoadStep(S32 block);
void PKR_drv_guardLayer(st_PACKER_LTOC_NODE*);
en_READ_ASYNC_STATUS PKR_drv_guardVerify(st_PACKER_LTOC_NODE*);
U32 PKRAssetIDFromInst(void* asset_inst);