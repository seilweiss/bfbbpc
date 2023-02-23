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

typedef struct st_PACKER_READ_DATA PKRReadData;
typedef struct st_PACKER_READ_FUNCS PKRReadFuncs;
typedef struct st_PACKER_WRITE_DATA PKRWriteData;
typedef struct st_PACKER_ASSETTYPE PKRAssetType;
typedef struct st_PKR_ASSET_TOCINFO PKRAssetTOCInfo;
typedef struct st_PACKER_ATOC_NODE PKRANode;
typedef struct st_PACKER_LTOC_NODE PKRLNode;

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
typedef enum en_LAYER_TYPE LAYER_TYPE;

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
    PKRAssetType* typeref;
    U32 sector;
    U32 plus_offset;
    U32 size;
    void* mempos;
};

struct st_PACKER_READ_FUNCS
{
    U32 api_ver;
    PKRReadData*(*Init)(void*, char*, U32, S32*, PKRAssetType*);
    void(*Done)(PKRReadData*);
    S32(*LoadLayer)(PKRReadData*, LAYER_TYPE);
    U32(*GetAssetSize)(PKRReadData*, U32);
    void*(*LoadAsset)(PKRReadData*, U32, const char*, void*);
    void*(*AssetByType)(PKRReadData*, U32, S32, U32*);
    S32(*AssetCount)(PKRReadData*, U32);
    S32(*IsAssetReady)(PKRReadData*, U32);
    S32(*SetActive)(PKRReadData*, LAYER_TYPE);
    const char*(*AssetName)(PKRReadData*, U32);
    U32(*GetBaseSector)(PKRReadData*);
    S32(*GetAssetInfo)(PKRReadData*, U32, PKRAssetTOCInfo*);
    S32(*GetAssetInfoByType)(PKRReadData*, U32, S32, PKRAssetTOCInfo*);
    S32(*PkgHasAsset)(PKRReadData*, U32);
    U32(*PkgTimeStamp)(PKRReadData*);
    void(*PkgDisconnect)(PKRReadData*);
};

extern S32 pkr_sector_size;

PKRReadFuncs* PKRGetReadFuncs(S32 apiver);
S32 PKRStartup();
S32 PKRShutdown();
S32 PKRLoadStep(S32 block);
void PKR_drv_guardLayer(PKRLNode*);
READ_ASYNC_STATUS PKR_drv_guardVerify(PKRLNode*);
U32 PKRAssetIDFromInst(void* asset_inst);