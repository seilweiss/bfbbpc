#include "xpkrsvc.h"

#include "xMemMgr.h"

#include <string.h>
#include <stdio.h>

struct st_PACKER_ATOC_NODE
{
    U32 aid;
    U32 asstype;
    S32 d_off;
    S32 d_size;
    S32 d_pad;
    U32 d_chksum;
    S32 assalign;
    S32 infoflag;
    S32 loadflag;
    char* memloc;
    S32 x_size;
    S32 readcnt;
    S32 readrem;
    PKRAssetType* typeref;
    HIPLOADDATA* ownpkg;
    PKRReadData* ownpr;

    const char* Name() const { return "<unknown>"; }
};

#define PKR_LDASS_READY (1<<19) // 0x80000
#define PKR_LDASS_ISDUP (1<<20) // 0x100000
#define PKR_LDASS_ISNIL (1<<21) // 0x200000

struct st_PACKER_LTOC_NODE
{
    LAYER_TYPE laytyp;
    XORDEREDARRAY assref;
    S32 flg_ldstat;
    S32 danglecnt;
    U32 chksum;
    S32 laysize;
    char* laymem;
    char* laytru;
};

#define PKR_LDLAY_INPROG (1<<24) // 0x1000000
#define PKR_LDLAY_ISDONE (1<<25) // 0x2000000

struct st_PACKER_READ_DATA
{
    PKRAssetType* types;
    void* userdata;
    U32 opts;
    U32 pkgver;
    S32 cltver;
    S32 subver;
    S32 compatver;
    HIPLOADDATA* pkg;
    U32 base_sector;
    S32 lockid;
    char packfile[PKR_MAX_NAMLEN];
    S32 asscnt;
    S32 laycnt;
    XORDEREDARRAY asstoc;
    XORDEREDARRAY laytoc;
    PKRANode* pool_anode;
    S32 pool_nextaidx;
    XORDEREDARRAY typelist[PKR_MAX_TYPES + 1];
    time_t time_made;
    time_t time_mod;
};

enum en_PKR_LAYER_LOAD_DEST
{
    PKR_LDDEST_SKIP,
    PKR_LDDEST_KEEPSTATIC,
    PKR_LDDEST_KEEPMALLOC,
    PKR_LDDEST_RWHANDOFF,
    PKR_LDDEST_NOMORE,
    PKR_LDDEST_FORCE = FORCEENUMSIZEINT
};
typedef enum en_PKR_LAYER_LOAD_DEST PKR_LAYER_LOAD_DEST;

static PKRReadData* PKR_ReadInit(void* userdata, char* pkgfile, U32 opts, S32* cltver, PKRAssetType* typelist);
static void PKR_ReadDone(PKRReadData* pr);
static S32 PKR_LoadLayer(PKRReadData*, LAYER_TYPE);
static U32 PKR_GetAssetSize(PKRReadData* pr, U32 aid);
static void* PKR_LoadAsset(PKRReadData* pr, U32 aid, const char*, void*);
static void* PKR_AssetByType(PKRReadData* pr, U32 type, S32 idx, U32* size);
static S32 PKR_AssetCount(PKRReadData* pr, U32 type);
static S32 PKR_IsAssetReady(PKRReadData* pr, U32 aid);
static S32 PKR_SetActive(PKRReadData* pr, LAYER_TYPE layer);
static const char* PKR_AssetName(PKRReadData* pr, U32 aid);
static U32 PKR_GetBaseSector(PKRReadData* pr);
static S32 PKR_GetAssetInfo(PKRReadData* pr, U32 aid, PKRAssetTOCInfo* tocinfo);
static S32 PKR_GetAssetInfoByType(PKRReadData* pr, U32 type, S32 idx, PKRAssetTOCInfo* tocinfo);
static S32 PKR_PkgHasAsset(PKRReadData* pr, U32 aid);
static U32 PKR_getPackTimestamp(PKRReadData* pr);
static void PKR_Disconnect(PKRReadData* pr);

static PKRReadFuncs g_pkr_read_funcmap_original = {
    1,
    PKR_ReadInit,
    PKR_ReadDone,
    PKR_LoadLayer,
    PKR_GetAssetSize,
    PKR_LoadAsset,
    PKR_AssetByType,
    PKR_AssetCount,
    PKR_IsAssetReady,
    PKR_SetActive,
    PKR_AssetName,
    PKR_GetBaseSector,
    PKR_GetAssetInfo,
    PKR_GetAssetInfoByType,
    PKR_PkgHasAsset,
    PKR_getPackTimestamp,
    PKR_Disconnect
};

static PKRReadFuncs g_pkr_read_funcmap = g_pkr_read_funcmap_original;
static HIPLOADFUNCS* g_hiprf = NULL;
static PKRReadData g_readdatainst[PKR_MAX_INST] = {};
static U32 g_loadlock = 0;

S32 pkr_sector_size = 0;

static S32 g_packinit = 0;

static S32 g_memalloc_pair = 0;
static S32 g_memalloc_runtot = 0;
static S32 g_memalloc_runfree = 0;

static S32 PKR_parse_TOC(HIPLOADDATA* pkg, PKRReadData* pr);
static S32 PKR_LoadStep_Async();
static char* PKR_LayerMemReserve(PKRReadData* pr, PKRLNode* layer);
static void PKR_LayerMemRelease(PKRReadData*, PKRLNode* layer);
static PKR_LAYER_LOAD_DEST PKR_layerLoadDest(LAYER_TYPE laytyp);
static S32 PKR_layerTypeNeedsXForm(LAYER_TYPE laytyp);
static S32 PKR_findNextLayerToLoad(PKRReadData** work_on_pkg, PKRLNode** next_layer);
static void PKR_updateLayerAssets(PKRLNode* laynode);
static void PKR_xformLayerAssets(PKRLNode* laynode);
static void PKR_xform_asset(PKRANode* assnode, S32 dumpable_layer);
static void* PKR_FindAsset(PKRReadData* pr, U32 aid);
static S32 PKR_FRIEND_assetIsGameDup(U32 aid, const PKRReadData* skippr, S32 oursize, U32 ourtype, U32 chksum, char* our_fnam);
static S32 PKR_makepool_anode(PKRReadData* pr, S32 cnt);
static void PKR_kiilpool_anode(PKRReadData* pr);
static PKRANode* PKR_newassnode(PKRReadData* pr, U32 aid);
static PKRLNode* PKR_newlaynode(LAYER_TYPE laytyp, S32 refcnt);
static void PKR_oldlaynode(PKRLNode* laynode);
static S32 OrdComp_R_Asset(void* vkey, void* vitem);
static S32 OrdTest_R_AssetID(const void* vkey, void* vitem);
static S32 LOD_r_HIPA(HIPLOADDATA*, PKRReadData* pr);
static S32 LOD_r_PACK(HIPLOADDATA* pkg, PKRReadData* pr);
static S32 LOD_r_PVER(HIPLOADDATA* pkg, PKRReadData* pr);
static S32 LOD_r_PFLG(HIPLOADDATA* pkg, PKRReadData*);
static S32 LOD_r_PCNT(HIPLOADDATA* pkg, PKRReadData* pr);
static S32 LOD_r_PCRT(HIPLOADDATA* pkg, PKRReadData* pr);
static S32 LOD_r_PMOD(HIPLOADDATA* pkg, PKRReadData* pr);
static S32 ValidatePlatform(HIPLOADDATA*, PKRReadData*, S32, char* plat, char* vid, char* lang, char* title);
static S32 LOD_r_PLAT(HIPLOADDATA* pkg, PKRReadData* pr);
static S32 LOD_r_DICT(HIPLOADDATA* pkg, PKRReadData* pr);
static S32 LOD_r_ATOC(HIPLOADDATA* pkg, PKRReadData* pr);
static S32 LOD_r_AINF(HIPLOADDATA* pkg, PKRReadData*);
static S32 LOD_r_AHDR(HIPLOADDATA* pkg, PKRReadData* pr);
static S32 LOD_r_ADBG(HIPLOADDATA* pkg, PKRReadData* pr, PKRANode* assnode);
static S32 LOD_r_LTOC(HIPLOADDATA* pkg, PKRReadData* pr);
static S32 LOD_r_LINF(HIPLOADDATA* pkg, PKRReadData*);
static S32 LOD_r_LHDR(HIPLOADDATA* pkg, PKRReadData* pr);
static S32 LOD_r_LDBG(HIPLOADDATA* pkg, PKRReadData* pr, PKRLNode* laynode);
static S32 LOD_r_STRM(HIPLOADDATA* pkg, PKRReadData* pr);
static S32 LOD_r_DHDR(HIPLOADDATA* pkg, PKRReadData*);
static S32 LOD_r_DPAK(HIPLOADDATA*, PKRReadData*);
static void PKR_spew_verhist();
static PKRAssetType* PKR_type2typeref(U32 type, PKRAssetType* typelist);
static void PKR_bld_typecnt(PKRReadData* pr);
static S32 PKR_typeHdlr_idx(PKRReadData* pr, U32 type);
static void PKR_alloc_chkidx();
static void* PKR_getmem(U32 id, S32 amount, U32 aid, S32 align);
static void* PKR_getmem(U32 id, S32 amount, U32 aid, S32 align, S32 isTemp, char** memtru);
static void PKR_relmem(U32 id, S32 blksize, void* memptr, U32 aid, S32 isTemp);
static S32 PKR_push_memmark();
static S32 PKR_pop_memmark();

PKRReadFuncs* PKRGetReadFuncs(S32 apiver)
{
    switch (apiver)
    {
    case 1: return &g_pkr_read_funcmap;
    }
    return NULL;
}

S32 PKRStartup()
{
    if (!g_packinit++) {
        g_pkr_read_funcmap = g_pkr_read_funcmap_original;
        g_hiprf = get_HIPLFuncs();
        pkr_sector_size = 32;
    }
    
    return g_packinit;
}

S32 PKRShutdown()
{
    g_packinit--;
    return g_packinit;
}

S32 PKRLoadStep(S32 block)
{
    S32 more_todo = 1;
    more_todo = PKR_LoadStep_Async();
    return more_todo;
}

static PKRReadData* PKR_ReadInit(void* userdata, char* pkgfile, U32 opts, S32* cltver, PKRAssetType* typelist)
{
    PKRReadData* pr = NULL;
    S32 i = 0;
    S32 uselock = -1;
    S32 rc = 0;
    char* tocbuf_RAW = NULL;
    char* tocbuf_aligned = NULL;
    
    PKR_alloc_chkidx();

    tocbuf_aligned = (char*)PKR_getmem('PTOC', 0x8000, 'PTOC', 64, TRUE, &tocbuf_RAW);

    for (i = 0; i < PKR_MAX_INST; i++) {
        if (!(g_loadlock & (1 << i))) {
            g_loadlock |= 1 << i;
            pr = &g_readdatainst[i];
            uselock = i;
            break;
        }
    }

    if (pr) {
        memset(pr, 0, sizeof(PKRReadData));

        pr->lockid = uselock;
        pr->userdata = userdata;
        pr->opts = opts;
        pr->types = typelist;
        pr->cltver = -1;

        strncpy(pr->packfile, pkgfile, PKR_MAX_NAMLEN);

        if (!tocbuf_aligned) {
            pr->pkg = g_hiprf->create(pkgfile, NULL, 0);
        }
        else {
            pr->pkg = g_hiprf->create(pkgfile, tocbuf_aligned, 0x8000);
        }

        if (pr->pkg) {
            pr->base_sector = g_hiprf->basesector(pr->pkg);
            rc = PKR_parse_TOC(pr->pkg, pr);
            *cltver = pr->cltver;
            g_hiprf->setBypass(pr->pkg, TRUE, TRUE);
        }
        else {
            PKR_ReadDone(pr);
            pr = NULL;
            *cltver = -1;
        }
    }
    else {
        pr = NULL;
        *cltver = -1;
    }

    PKR_relmem('PTOC', 0x8000, tocbuf_RAW, 'PTOC', TRUE);
    tocbuf_RAW = NULL;

    return pr;
}

static void PKR_ReadDone(PKRReadData* pr)
{
    S32 i = 0;
    S32 j = 0;
    S32 lockid = -1;
    PKRANode* assnode = NULL;
    PKRLNode* laynode = NULL;
    XORDEREDARRAY* tmplist = NULL;

    if (!pr) return;

    for (j = pr->laytoc.cnt - 1; j >= 0; j--) {
        laynode = (PKRLNode*)pr->laytoc.list[j];
        for (i = laynode->assref.cnt - 1; i >= 0; i--) {
            assnode = (PKRANode*)laynode->assref.list[i];
            if (assnode->typeref)
                if (assnode->typeref->assetUnloaded)
                    if (!(assnode->loadflag & PKR_LDASS_ISDUP))
                        assnode->typeref->assetUnloaded(assnode->memloc, assnode->aid);
        }
    }
    
    for (i = 0; i < pr->laytoc.cnt; i++) {
        laynode = (PKRLNode*)pr->laytoc.list[i];
        if (!laynode->laymem) continue;
        PKR_LayerMemRelease(pr, laynode);
        laynode->laymem = NULL;
    }

    PKR_kiilpool_anode(pr);

    for (i = 0; i < pr->laytoc.cnt; i++) {
        laynode = (PKRLNode*)pr->laytoc.list[i];
        PKR_oldlaynode(laynode);
    }

    XOrdDone(&pr->asstoc, FALSE);
    XOrdDone(&pr->laytoc, FALSE);

    for (i = 0; i < 129; i++) {
        tmplist = &pr->typelist[i];
        if (tmplist->max)
            XOrdDone(tmplist, FALSE);
    }

    if (pr->pkg) {
        g_hiprf->destroy(pr->pkg);
        pr->pkg = NULL;
    }

    lockid = pr->lockid;
    memset(pr, 0, sizeof(PKRReadData));
    g_loadlock &= ~(1 << lockid);
    
}

static S32 PKR_SetActive(PKRReadData* pr, LAYER_TYPE layer)
{
    S32 result = 1;
    S32 rc = 0;
    S32 i = 0;
    S32 j = 0;
    PKRANode* assnode = NULL;
    PKRLNode* laynode = NULL;

    if (!pr) return 0;

    for (i = 0; i < pr->laytoc.cnt; i++) {
        laynode = (PKRLNode*)pr->laytoc.list[i];
        if (layer > PKR_LTYPE_DEFAULT && laynode->laytyp != layer) continue;

        for (j = 0; j < laynode->assref.cnt; j++) {
            assnode = (PKRANode*)laynode->assref.list[j];

            if (assnode->loadflag & 0x10000) continue;
            if (!(assnode->loadflag & PKR_LDASS_READY)) continue;

            if (!assnode->typeref) {
                continue;
            }

            if (assnode->typeref->assetLoaded) {
                rc = assnode->typeref->assetLoaded(pr->userdata,
                                                   assnode->aid,
                                                   assnode->memloc,
                                                   assnode->d_size);
                if (!rc) {
                    result = 0;
                }
                else assnode->loadflag |= 0x10000;
            }
        }
    }

    return result;
}

static S32 PKR_parse_TOC(HIPLOADDATA* pkg, PKRReadData* pr)
{
    S32 is_ok = FALSE;
    U32 cid = 0;
    S32 done = FALSE;

    cid = g_hiprf->enter(pkg);
    while (cid) {
        switch (cid) {
        case 'HIPA':
            is_ok = LOD_r_HIPA(pkg, pr);
            break;
        case 'PACK':
            LOD_r_PACK(pkg, pr);
            if (is_ok) {
                if (pr->asscnt > 0) {
                    XOrdInit(&pr->asstoc, pr->asscnt, FALSE);
                    PKR_makepool_anode(pr, pr->asscnt);
                }
                if (pr->laycnt > 0) XOrdInit(&pr->laytoc, pr->laycnt, FALSE);
            }
            break;
        case 'DICT':
            LOD_r_DICT(pkg, pr);
            PKR_bld_typecnt(pr);
            done = TRUE;
            break;
        case 'STRM':
            LOD_r_STRM(pkg, pr);
            break;
        }
        
        if (!is_ok) {
            break;
        }

        g_hiprf->exit(pkg);
        if (!done) cid = g_hiprf->enter(pkg);
        else break;
    }

    return is_ok;
}

static S32 PKR_LoadStep_Async()
{
    S32 moretodo = TRUE;
    S32 rc = 0;
    READ_ASYNC_STATUS readstat = HIP_RDSTAT_NONE;
    static PKRReadData* curpr = NULL;
    static PKRLNode* asynlay = NULL;

    if (!asynlay) {
        PKR_findNextLayerToLoad(&curpr, &asynlay);
        if (asynlay) {
            PKR_LAYER_LOAD_DEST loaddest = PKR_layerLoadDest(asynlay->laytyp);
            if (loaddest != PKR_LDDEST_SKIP && asynlay->laysize > 1 && asynlay->assref.cnt > 0) {
                asynlay->laymem = PKR_LayerMemReserve(curpr, asynlay);
                PKR_drv_guardLayer(asynlay);
                
                PKRANode* tmpass = (PKRANode*)asynlay->assref.list[0];
                g_hiprf->setSpot(tmpass->ownpkg, tmpass->d_off);

                rc = g_hiprf->readBytes(tmpass->ownpkg, asynlay->laymem, asynlay->laysize);
                if (rc) {
                    asynlay->flg_ldstat |= PKR_LDLAY_INPROG;
                }
                else {
                    PKR_LayerMemRelease(curpr, asynlay);
                    asynlay->flg_ldstat &= ~(PKR_LDLAY_INPROG | PKR_LDLAY_ISDONE);
                    curpr = NULL;
                    asynlay = NULL;
                }

                moretodo = TRUE;
            }
            else {
                asynlay->flg_ldstat |= PKR_LDLAY_ISDONE;
                asynlay = NULL;
                moretodo = TRUE;
            }
        }
        else {
            curpr = NULL;
            asynlay = NULL;
            moretodo = FALSE;
        }
    }
    else {
        readstat = g_hiprf->pollRead(curpr->pkg);
        if (readstat == HIP_RDSTAT_SUCCESS) {
            readstat = PKR_drv_guardVerify(asynlay);
        }

        if (readstat == HIP_RDSTAT_INPROG) {
            moretodo = TRUE;
        }
        else if (readstat == HIP_RDSTAT_SUCCESS) {
            PKR_updateLayerAssets(asynlay);
            if (PKR_layerTypeNeedsXForm(asynlay->laytyp)) {
                PKR_xformLayerAssets(asynlay);
            }

            PKR_LAYER_LOAD_DEST loaddest = PKR_layerLoadDest(asynlay->laytyp);
            if (loaddest == PKR_LDDEST_RWHANDOFF) {
                PKR_LayerMemRelease(curpr, asynlay);
            }

            asynlay->flg_ldstat |= PKR_LDLAY_ISDONE;
            asynlay = NULL;
            moretodo = TRUE;
        }
        else {
            PKR_LAYER_LOAD_DEST loaddest = PKR_layerLoadDest(asynlay->laytyp);
            if (asynlay->laymem && loaddest == PKR_LDDEST_RWHANDOFF) {
                PKR_LayerMemRelease(curpr, asynlay);
            }

            asynlay->flg_ldstat &= ~(PKR_LDLAY_INPROG | PKR_LDLAY_ISDONE);
            asynlay = NULL;
            curpr = NULL;
            moretodo = TRUE;
        }
    }

    return moretodo;
}

static char* PKR_LayerMemReserve(PKRReadData* pr, PKRLNode* layer)
{
    char* mem = NULL;

    if (layer->laymem) return layer->laymem;

    PKR_LAYER_LOAD_DEST loaddest = PKR_layerLoadDest(layer->laytyp);
    switch (loaddest) {
    case PKR_LDDEST_SKIP:
        break;
    case PKR_LDDEST_KEEPSTATIC:
        mem = (char*)PKR_getmem('LYR\0', layer->laysize, layer->laytyp + 0x8000, 64);
        break;
    case PKR_LDDEST_KEEPMALLOC:
        mem = (char*)PKR_getmem('LYR\0', layer->laysize, layer->laytyp + 0x8000, 64, TRUE, &layer->laytru);
        break;
    case PKR_LDDEST_RWHANDOFF:
        PKR_push_memmark();
        mem = (char*)PKR_getmem('LYR\0', layer->laysize, layer->laytyp + 0x8000, 64);
        break;
    }

    return mem;
}

static void PKR_LayerMemRelease(PKRReadData*, PKRLNode* layer)
{
    PKR_LAYER_LOAD_DEST loaddest = PKR_layerLoadDest(layer->laytyp);
    switch (loaddest) {
    case PKR_LDDEST_SKIP:
        break;
    case PKR_LDDEST_RWHANDOFF:
        PKR_relmem('LYR\0', layer->laysize, layer->laymem, layer->laytyp + 0x8000, FALSE);
        PKR_pop_memmark();
        layer->laymem = NULL;
        break;
    case PKR_LDDEST_KEEPSTATIC:
        PKR_relmem('LYR\0', layer->laysize, layer->laymem, layer->laytyp + 0x8000, FALSE);
        break;
    case PKR_LDDEST_KEEPMALLOC:
        PKR_relmem('LYR\0', layer->laysize, layer->laytru, layer->laytyp + 0x8000, TRUE);
        layer->laymem = NULL;
        layer->laytru = NULL;
        break;
    }
}

void PKR_drv_guardLayer(PKRLNode*)
{
}

READ_ASYNC_STATUS PKR_drv_guardVerify(PKRLNode*)
{
    READ_ASYNC_STATUS confirm = HIP_RDSTAT_SUCCESS;
    return confirm;
}

static PKR_LAYER_LOAD_DEST PKR_layerLoadDest(LAYER_TYPE laytyp)
{
    PKR_LAYER_LOAD_DEST load_place = PKR_LDDEST_KEEPSTATIC;

    switch (laytyp) {
    case PKR_LTYPE_SRAM:
    case PKR_LTYPE_CUTSCENE:
        load_place = PKR_LDDEST_SKIP;
        break;
    case PKR_LTYPE_TEXTURE:
    case PKR_LTYPE_BSP:
    case PKR_LTYPE_MODEL:
        load_place = PKR_LDDEST_RWHANDOFF;
        break;
    case PKR_LTYPE_ANIMATION:
    case PKR_LTYPE_JSPINFO:
        load_place = PKR_LDDEST_KEEPMALLOC;
        break;
    case PKR_LTYPE_DEFAULT:
    case PKR_LTYPE_VRAM:
    case PKR_LTYPE_SNDTOC:
    case PKR_LTYPE_CUTSCENETOC:
        load_place = PKR_LDDEST_KEEPSTATIC;
        break;
    default:
        load_place = PKR_LDDEST_KEEPSTATIC;
        break;
    }

    return load_place;
}

static S32 PKR_layerTypeNeedsXForm(LAYER_TYPE laytyp)
{
    S32 do_xform = FALSE;

    switch (laytyp) {
    case PKR_LTYPE_TEXTURE:
    case PKR_LTYPE_BSP:
    case PKR_LTYPE_MODEL:
        do_xform = TRUE;
        break;
    case PKR_LTYPE_DEFAULT:
    case PKR_LTYPE_ANIMATION:
    case PKR_LTYPE_VRAM:
    case PKR_LTYPE_SNDTOC:
    case PKR_LTYPE_CUTSCENETOC:
    case PKR_LTYPE_JSPINFO:
        do_xform = TRUE;
        break;
    case PKR_LTYPE_SRAM:
    case PKR_LTYPE_CUTSCENE:
        do_xform = FALSE;
        break;
    default:
        do_xform = FALSE;
        break;
    }

    return do_xform;
}

static S32 PKR_findNextLayerToLoad(PKRReadData** work_on_pkg, PKRLNode** next_layer)
{
    PKRReadData* tmppr = NULL;
    PKRLNode* tmplay = NULL;
    S32 i = 0;
    S32 j = 0;

    *next_layer = NULL;
    tmppr = *work_on_pkg;
    if (tmppr) {
        for (i = 0; i < tmppr->laytoc.cnt; i++) {
            tmplay = (PKRLNode*)tmppr->laytoc.list[i];
            if (tmplay->flg_ldstat & PKR_LDLAY_ISDONE) continue;
            *next_layer = tmplay;
            *work_on_pkg = tmppr;
            break;
        }
    }
    
    if (!*next_layer) {
        for (j = 0; j < PKR_MAX_INST; j++) {
            if (!(g_loadlock & (1 << j))) continue;

            tmppr = &g_readdatainst[j];
            if (tmppr == *work_on_pkg) continue;

            for (i = 0; i < tmppr->laytoc.cnt; i++) {
                tmplay = (PKRLNode*)tmppr->laytoc.list[i];
                if (tmplay->flg_ldstat & PKR_LDLAY_ISDONE) continue;
                *next_layer = tmplay;
                *work_on_pkg = tmppr;
                break;
            }
            
            if (*next_layer) break;
        }
    }

    return (*next_layer) ? TRUE : FALSE;
}

static void PKR_updateLayerAssets(PKRLNode* laynode)
{
    S32 i = 0;
    PKRANode* tmpass = NULL;
    for (i = 0; i < laynode->assref.cnt; i++) {
        tmpass = (PKRANode*)laynode->assref.list[i];
        if (tmpass->d_off > 0 && tmpass->d_size > 0) break;
        tmpass = NULL;
    }
    if (!tmpass) return;
    
    S32 lay_hip_pos = tmpass->d_off;
    for (i = 0; i < laynode->assref.cnt; i++) {
        tmpass = (PKRANode*)laynode->assref.list[i];
        if (tmpass->loadflag & PKR_LDASS_ISDUP) continue;
        if (tmpass->loadflag & PKR_LDASS_ISNIL) {
            tmpass->memloc = NULL;
        }
        else {
            S32 reloff = tmpass->d_off - lay_hip_pos;
            tmpass->memloc = laynode->laymem + reloff;
            tmpass->loadflag |= PKR_LDASS_READY;
        }
    }
}


static void PKR_xformLayerAssets(PKRLNode* laynode)
{
    S32 i = 0;
    S32 will_be_dumped = FALSE;
    PKRANode* tmpass = NULL;
    PKR_LAYER_LOAD_DEST loaddest = PKR_layerLoadDest(laynode->laytyp);
    if (loaddest == PKR_LDDEST_RWHANDOFF) will_be_dumped = TRUE;

    for (i = 0; i < laynode->assref.cnt; i++) {
        tmpass = (PKRANode*)laynode->assref.list[i];
        if (tmpass->loadflag & PKR_LDASS_ISDUP) continue;
        
        PKR_xform_asset(tmpass, will_be_dumped);
        if (will_be_dumped && tmpass->x_size < 1) {
            tmpass->memloc = NULL;
        }
    }
}

static void PKR_xform_asset(PKRANode* assnode, S32 dumpable_layer)
{
    char* xformloc = NULL;

    if (!(assnode->infoflag & 0x4)) {
        if (assnode->typeref && assnode->typeref->readXForm) {
        }
        else return;
    }

    const PKRAssetType* atype = assnode->typeref;
    if (!atype) {
        return;
    }

    if (!atype->readXForm) {
        return;
    }

    if (!assnode->d_size) {
        assnode->memloc = NULL;
    }

    xformloc = (char*)atype->readXForm(
        assnode->ownpr->userdata,
        assnode->aid,
        assnode->memloc,
        assnode->d_size,
        (U32*)&assnode->x_size);

    if (!dumpable_layer && assnode->memloc == xformloc && assnode->x_size) {
    }
    else if (assnode->d_size == 0 || assnode->x_size == 0) {
        assnode->memloc = NULL;
        assnode->loadflag |= PKR_LDASS_ISNIL;
    }
    else assnode->memloc = xformloc;
}


static void* PKR_FindAsset(PKRReadData* pr, U32 aid)
{
    S32 idx = -1;
    PKRANode* assnode = NULL;

    idx = XOrdLookup(&pr->asstoc, (void*)aid, OrdTest_R_AssetID);
    if (idx >= 0)
    {
        assnode = (PKRANode*)pr->asstoc.list[idx];
    }

    if (assnode)
    {
        if (!assnode->memloc) assnode->Name(); // decomp: probably a warn here
        return assnode->memloc;
    }

    return NULL;
}

static S32 PKR_LoadLayer(PKRReadData*, LAYER_TYPE)
{
    return 0;
}

static void* PKR_LoadAsset(PKRReadData* pr, U32 aid, const char*, void*)
{
    return PKR_FindAsset(pr, aid);
}

static U32 PKR_GetAssetSize(PKRReadData* pr, U32 aid)
{
    S32 idx = -1;
    const PKRANode* assnode = NULL;
    
    idx = XOrdLookup(&pr->asstoc, (void*)aid, OrdTest_R_AssetID);
    if (idx > -1) {
        assnode = (PKRANode*)pr->asstoc.list[idx];
    }
    if (assnode) {
        if (assnode->x_size > 0) return assnode->x_size;
        return assnode->d_size;
    }
    return 0;
}

static S32 PKR_AssetCount(PKRReadData* pr, U32 type)
{
    S32 cnt = 0;
    S32 idx = -1;
    XORDEREDARRAY* tmplist = NULL;

    if (!type) return pr->asstoc.cnt;

    idx = PKR_typeHdlr_idx(pr, type);
    if (idx >= 0) {
        tmplist = &pr->typelist[idx];
        cnt = tmplist->cnt;
    }

    return cnt;
}

static void* PKR_AssetByType(PKRReadData* pr, U32 type, S32 idx, U32* size)
{
    void* memloc = NULL;
    S32 typidx = 0;
    XORDEREDARRAY* typlist = NULL;
    PKRANode* assnode = NULL;
    
    if (size) *size = 0;
    if (idx < 0) idx = 0;

    typidx = PKR_typeHdlr_idx(pr, type);
    if (typidx < 0) return memloc;

    typlist = &pr->typelist[typidx];
    if (idx >= typlist->cnt) return memloc;

    assnode = (PKRANode*)typlist->list[idx];
    if (size) *size = assnode->d_size;

    memloc = assnode->memloc;
    return memloc;
}

static S32 PKR_IsAssetReady(PKRReadData* pr, U32 aid)
{
    S32 is_ok = FALSE;
    PKRANode* assnode = NULL;
    S32 idx = -1;
    
    idx = XOrdLookup(&pr->asstoc, (void*)aid, OrdTest_R_AssetID);
    if (idx < 0) {
    }
    else {
        assnode = (PKRANode*)pr->asstoc.list[idx];
        if (assnode->loadflag & PKR_LDASS_READY) is_ok = TRUE;
        else is_ok = FALSE;
    }

    return is_ok;
}

static U32 PKR_getPackTimestamp(PKRReadData* pr)
{
    return (U32)pr->time_made;
}

static void PKR_Disconnect(PKRReadData* pr)
{
    if (pr->pkg) {
        g_hiprf->destroy(pr->pkg);
        pr->pkg = NULL;
    }
}

U32 PKRAssetIDFromInst(void* asset_inst)
{
    PKRANode* assnode = (PKRANode*)asset_inst;
    return assnode->aid;
}

static const char* PKR_AssetName(PKRReadData* pr, U32 aid)
{
    const char* da_name = NULL;
    S32 idx = -1;
    PKRANode* assnode = NULL;

    if (!aid) return da_name;

    idx = XOrdLookup(&pr->asstoc, (void*)aid, OrdTest_R_AssetID);
    if (idx >= 0) {
        assnode = (PKRANode*)pr->asstoc.list[idx];
        da_name = assnode->Name();
    }

    return da_name;
}


static U32 PKR_GetBaseSector(PKRReadData* pr)
{
    return pr->base_sector;
}

static S32 PKR_GetAssetInfo(PKRReadData* pr, U32 aid, PKRAssetTOCInfo* tocinfo)
{
    S32 idx = -1;
    PKRANode* assnode = NULL;

    memset(tocinfo, 0, sizeof(PKRAssetTOCInfo));

    idx = XOrdLookup(&pr->asstoc, (void*)aid, OrdTest_R_AssetID);
    if (idx >= 0) {
        assnode = (PKRANode*)pr->asstoc.list[idx];
    
        tocinfo->aid = aid;
        tocinfo->typeref = assnode->typeref;
        tocinfo->sector = pr->base_sector + assnode->d_off / pkr_sector_size;
        tocinfo->plus_offset = assnode->d_off % pkr_sector_size;
        tocinfo->size = assnode->d_size;
        tocinfo->mempos = assnode->memloc;
    }
    
    return idx >= 0 ? TRUE : FALSE;
}

static S32 PKR_GetAssetInfoByType(PKRReadData* pr, U32 type, S32 idx, PKRAssetTOCInfo* tocinfo)
{
    PKRANode* assnode = NULL;
    S32 typidx = 0;
    XORDEREDARRAY* typlist = NULL;
    
    memset(tocinfo, 0, sizeof(PKRAssetTOCInfo));

    if (idx < 0) idx = 0;

    typidx = PKR_typeHdlr_idx(pr, type);
    if (typidx < 0) return FALSE;

    typlist = &pr->typelist[typidx];
    if (idx >= typlist->cnt) return FALSE;

    assnode = (PKRANode*)typlist->list[idx];
    
    tocinfo->aid = assnode->aid;
    tocinfo->typeref = assnode->typeref;
    tocinfo->sector = pr->base_sector + assnode->d_off / pkr_sector_size;
    tocinfo->plus_offset = assnode->d_off % pkr_sector_size;
    tocinfo->size = assnode->d_size;
    tocinfo->mempos = assnode->memloc;
    
    return TRUE;
}

static S32 PKR_PkgHasAsset(PKRReadData* pr, U32 aid)
{
    S32 rc = 0;
    S32 idx = -1;
    PKRANode* assnode = NULL;

    idx = XOrdLookup(&pr->asstoc, (void*)aid, OrdTest_R_AssetID);

    if (idx < 0) rc = 0;
    else {
        assnode = (PKRANode*)pr->asstoc.list[idx];

        rc = 1;

        if (assnode->loadflag & PKR_LDASS_ISDUP) rc = 0;
        else if (assnode->loadflag & PKR_LDASS_ISNIL) rc = 0;
    }

    return rc;
}

static S32 PKR_FRIEND_assetIsGameDup(U32 aid, const PKRReadData* skippr, S32 oursize, U32 ourtype, U32 chksum, char* our_fnam)
{
    S32 is_dup = FALSE;
    PKRReadData* tmp_pr = NULL;
    const PKRANode* tmp_ass = NULL;
    S32 i = 0;
    S32 idx = -1;
    
    S32 bonus = 0;
    S32 is_a_sound = FALSE;

    if (aid == 0x7AB6743A) return is_dup;
    if (aid == 0x98A3F56C) return is_dup;

    for (i = 0; i < PKR_MAX_INST; i++) {
        if (!(g_loadlock & (1 << i))) {
            continue;
        }
        tmp_pr = &g_readdatainst[i];
        if (tmp_pr == skippr) {
            continue;
        }

        idx = XOrdLookup(&tmp_pr->asstoc, (void*)aid, OrdTest_R_AssetID);
        if (idx < 0) {
            continue;
        }

        tmp_ass = (PKRANode*)tmp_pr->asstoc.list[idx];
        is_a_sound = FALSE;
        if (!(tmp_ass->loadflag & PKR_LDASS_READY)) {
            if (tmp_ass->asstype != 'SND ' && tmp_ass->asstype != 'SNDS') {
                continue;
            }

            is_a_sound = TRUE;
        }

        if (ourtype != 0 && ourtype != tmp_ass->asstype) {
            bonus++;
        }

        if (oursize >= 0 && oursize != tmp_ass->d_size) {
            bonus++;
        }

        if (chksum != 0 && chksum != tmp_ass->d_chksum) {
            bonus++;
        }
        
        is_dup = TRUE;
        break;
    }

    return is_dup;
}

static S32 PKR_makepool_anode(PKRReadData* pr, S32 cnt)
{
    PKRANode* asspool = NULL;
    S32 amount = 0;

    if (!cnt) return amount;

    amount = cnt * sizeof(PKRANode);
    asspool = (PKRANode*)PKR_getmem('ANOD', amount, 'FAKE', 64);

    if (asspool) {
        pr->pool_anode = asspool;
        pr->pool_nextaidx = 0;
    }

    return asspool ? amount : 0;
}

static void PKR_kiilpool_anode(PKRReadData* pr)
{
    S32 amount = 0;
    if (pr->asscnt) {
        amount = pr->asscnt * sizeof(PKRANode);
        PKR_relmem('ANOD', amount, pr->pool_anode, 'FAKE', FALSE);
        pr->pool_anode = NULL;
        pr->pool_nextaidx = 0;
    }
}

static PKRANode* PKR_newassnode(PKRReadData* pr, U32 aid)
{
    PKRANode* assnode = NULL;
    assnode = &pr->pool_anode[pr->pool_nextaidx];
    pr->pool_nextaidx++;
    memset(assnode, 0, sizeof(PKRANode));
    assnode->aid = aid;
    return assnode;
}

static PKRLNode* PKR_newlaynode(LAYER_TYPE laytyp, S32 refcnt)
{
    PKRLNode* laynode = NULL;
    laynode = (PKRLNode*)PKR_getmem('LNOD', sizeof(PKRLNode), laytyp + 0x8000, 64);
    memset(laynode, 0, sizeof(PKRLNode));
    laynode->laytyp = laytyp;
    XOrdInit(&laynode->assref, (refcnt > 1) ? refcnt : 2, FALSE);
    return laynode;
}

static void PKR_oldlaynode(PKRLNode* laynode)
{
    XOrdDone(&laynode->assref, FALSE);
    PKR_relmem('LNOD', sizeof(PKRLNode), laynode, laynode->laytyp + 0x8000, FALSE);
}

static S32 OrdComp_R_Asset(void* vkey, void* vitem)
{
    S32 rc = 0;
    PKRANode* key = (PKRANode*)vkey;
    PKRANode* item = (PKRANode*)vitem;
    
    if (key->aid < item->aid) rc = -1;
    else if (key->aid > item->aid) rc = 1;
    else rc = 0;
    
    return rc;
}

static S32 OrdTest_R_AssetID(const void* vkey, void* vitem)
{
    S32 rc = 0;
    U32 key = (U32)vkey;
    PKRANode* item = (PKRANode*)vitem;

    if (key < item->aid) rc = -1;
    else if (key > item->aid) rc = 1;
    else rc = 0;

    return rc;
}

static S32 LOD_r_HIPA(HIPLOADDATA*, PKRReadData* pr)
{
    S32 result = TRUE;
    pr->pkgver = PKR_PKG_VER;
    return result;
}

static S32 LOD_r_PACK(HIPLOADDATA* pkg, PKRReadData* pr)
{
    S32 result = TRUE;
    U32 cid = 0;
    cid = g_hiprf->enter(pkg);
    while (cid) {
        switch (cid) {
        case 'PVER':
            LOD_r_PVER(pkg, pr);
            break;
        case 'PFLG':
            LOD_r_PFLG(pkg, pr);
            break;
        case 'PCNT':
            LOD_r_PCNT(pkg, pr);
            break;
        case 'PCRT':
            LOD_r_PCRT(pkg, pr);
            break;
        case 'PMOD':
            LOD_r_PMOD(pkg, pr);
            break;
        case 'PLAT':
            LOD_r_PLAT(pkg, pr);
            break;
        }
        g_hiprf->exit(pkg);
        cid = g_hiprf->enter(pkg);
    }
    return result;
}

static S32 LOD_r_PVER(HIPLOADDATA* pkg, PKRReadData* pr)
{
    S32 result = TRUE;
    S32 ver = 0;
    S32 amt = 0;

    g_hiprf->readLongs(pkg, &ver, 1);
    pr->subver = ver;
    if (ver < PKR_SUB_VER) {
        PKR_spew_verhist();
    }

    ver = -1;
    amt = g_hiprf->readLongs(pkg, &ver, 1);
    pr->cltver = ver;
    
    ver = -1;
    amt = g_hiprf->readLongs(pkg, &ver, 1);
    if (amt != 1) {
        pr->compatver = PKR_COMPAT_VER;
    }
    else pr->compatver = ver;

    return result;
}

static S32 LOD_r_PFLG(HIPLOADDATA* pkg, PKRReadData*)
{
    S32 result = TRUE;
    S32 pkgflags = 0;
    g_hiprf->readLongs(pkg, &pkgflags, 1);
    return result;
}

static S32 LOD_r_PCNT(HIPLOADDATA* pkg, PKRReadData* pr)
{
    S32 result = TRUE;
    S32 ival = 0;

    g_hiprf->readLongs(pkg, &ival, 1);
    pr->asscnt = ival;

    g_hiprf->readLongs(pkg, &ival, 1);
    pr->laycnt = ival;

    g_hiprf->readLongs(pkg, &ival, 1);
    g_hiprf->readLongs(pkg, &ival, 1);
    g_hiprf->readLongs(pkg, &ival, 1);

    return result;
}

static S32 LOD_r_PCRT(HIPLOADDATA* pkg, PKRReadData* pr)
{
    S32 result = TRUE;
    S32 ival = 0;
    char tymbuf[256] = {};

    g_hiprf->readLongs(pkg, &ival, 1);
    pr->time_made = ival;

    if (pr->subver > PKR_COMPAT_VER) {
        g_hiprf->readString(pkg, tymbuf);
    }

    return result;
}

static S32 LOD_r_PMOD(HIPLOADDATA* pkg, PKRReadData* pr)
{
    S32 result = TRUE;
    S32 ival = 0;

    g_hiprf->readLongs(pkg, &ival, 1);
    pr->time_mod = ival;

    return result;
}

static S32 ValidatePlatform(HIPLOADDATA*, PKRReadData*, S32, char* plat, char* vid, char* lang, char* title)
{
    char fullname[128] = {};
    sprintf(fullname, "%s %s %s %s", plat, vid, lang, title);

    S32 rc;
    rc = !strcmp(plat, "GameCube") || !strcmp(plat, "Xbox") || !strcmp(plat, "PlayStation 2");
    if (!rc) return FALSE;

    rc = !strcmp(vid, "NTSC") || !strcmp(vid, "PAL");
    if (!rc) return FALSE;

    rc = !strcmp(lang, "US Common") || !strcmp(lang, "United Kingdom") || !strcmp(lang, "French") || !strcmp(lang, "German");
    if (!rc) return FALSE;

    rc = !strcmp(title, "Sponge Bob") || !strcmp(title, "Incredibles") || !strcmp(title, "Jimmy Newtron");
    if (!rc) return FALSE;

    rc = !strcmp(plat, "GameCube");
    if (!rc) return FALSE;
    
    rc = !strcmp(vid, "NTSC");
    if (!rc) return FALSE;
    
    rc = !strcmp(lang, "US Common");
    if (!rc) return FALSE;
    
    rc = !strcmp(title, "Sponge Bob");
    if (!rc) return FALSE;

    return TRUE;
}

static S32 LOD_r_PLAT(HIPLOADDATA* pkg, PKRReadData* pr)
{
    S32 result = TRUE;
    S32 plattag = 0;
    char platname[32] = {};
    char vidname[32] = {};
    char langname[32] = {};
    char titlename[32] = {};
    S32 n;
    S32 rc;

    n = g_hiprf->readLongs(pkg, &plattag, 1);
    n = g_hiprf->readString(pkg, platname);
    n = g_hiprf->readString(pkg, vidname);
    n = g_hiprf->readString(pkg, langname);
    n = g_hiprf->readString(pkg, titlename);
    if (!n) strcpy(titlename, "<Unknown>");

    rc = ValidatePlatform(pkg, pr, plattag, platname, vidname, langname, titlename);
    if (!rc) result = FALSE;

    return result;
}

static S32 LOD_r_DICT(HIPLOADDATA* pkg, PKRReadData* pr)
{
    S32 result = TRUE;
    U32 cid = 0;

    cid = g_hiprf->enter(pkg);
    while (cid) {
        switch (cid) {
        case 'ATOC':
            LOD_r_ATOC(pkg, pr);
            XOrdSort(&pr->asstoc, OrdComp_R_Asset);
            break;
        case 'LTOC':
            LOD_r_LTOC(pkg, pr);
            break;
        }

        g_hiprf->exit(pkg);
        cid = g_hiprf->enter(pkg);
    }

    return result;
}

static S32 LOD_r_ATOC(HIPLOADDATA* pkg, PKRReadData* pr)
{
    S32 result = TRUE;
    U32 cid = 0;

    cid = g_hiprf->enter(pkg);
    while (cid) {
        switch (cid) {
        case 'AINF':
            LOD_r_AINF(pkg, pr);
            break;
        case 'AHDR':
            LOD_r_AHDR(pkg, pr);
            break;
        }

        g_hiprf->exit(pkg);
        cid = g_hiprf->enter(pkg);
        
    }
    return result;
}

static S32 LOD_r_AINF(HIPLOADDATA* pkg, PKRReadData*)
{
    S32 result = TRUE;
    S32 ival = 0;

    g_hiprf->readLongs(pkg, &ival, 1);

    return result;
}

static S32 LOD_r_AHDR(HIPLOADDATA* pkg, PKRReadData* pr)
{
    S32 result = TRUE;
    U32 cid = 0;
    S32 ival = 0;
    S32 isdup = FALSE;
    PKRANode* assnode = NULL;

    g_hiprf->readLongs(pkg, &ival, 1);
    assnode = PKR_newassnode(pr, ival);
    assnode->ownpr = pr;
    assnode->ownpkg = pkg;
    XOrdAppend(&pr->asstoc, assnode);
    
    g_hiprf->readLongs(pkg, &ival, 1);
    assnode->asstype = ival;
    assnode->typeref = PKR_type2typeref(assnode->asstype, pr->types);

    g_hiprf->readLongs(pkg, &ival, 1);
    assnode->d_off = ival;

    g_hiprf->readLongs(pkg, &ival, 1);
    assnode->d_size = ival;
    assnode->readrem = ival;
    assnode->readcnt = 0;
    if (assnode->d_size < 1) assnode->loadflag |= PKR_LDASS_ISNIL;

    g_hiprf->readLongs(pkg, &ival, 1);
    assnode->d_pad = ival;

    g_hiprf->readLongs(pkg, &ival, 1);
    assnode->infoflag = ival;

    cid = g_hiprf->enter(pkg);
    while (cid) {
        switch (cid) {
        case 'ADBG':
            LOD_r_ADBG(pkg, pr, assnode);
            break;
        }

        g_hiprf->exit(pkg);
        cid = g_hiprf->enter(pkg);
    }

    isdup = PKR_FRIEND_assetIsGameDup(
        assnode->aid, pr, assnode->d_size,
        assnode->asstype, assnode->d_chksum, NULL);
    if (isdup) assnode->loadflag |= PKR_LDASS_ISDUP;

    return result;
}

static S32 LOD_r_ADBG(HIPLOADDATA* pkg, PKRReadData* pr, PKRANode* assnode)
{
    S32 result = TRUE;
    S32 ival = 0;
    char tmpbuf[256] = {};
    S32 amt = 0;

    g_hiprf->readLongs(pkg, &ival, 1);
    assnode->assalign = ival;

    g_hiprf->readString(pkg, tmpbuf);
    
    tmpbuf[0] = '\0';
    g_hiprf->readString(pkg, tmpbuf);
    
    if (pr->subver > PKR_COMPAT_VER) {
        amt = g_hiprf->readLongs(pkg, &ival, 1);
        assnode->d_chksum = ival;
    }
    
    return result;
}

static S32 LOD_r_LTOC(HIPLOADDATA* pkg, PKRReadData* pr)
{
    S32 result = TRUE;
    U32 cid = 0;

    cid = g_hiprf->enter(pkg);
    while (cid) {
        switch (cid) {
        case 'LINF':
            LOD_r_LINF(pkg, pr);
            break;
        case 'LHDR':
            LOD_r_LHDR(pkg, pr);
            break;
        }

        g_hiprf->exit(pkg);
        cid = g_hiprf->enter(pkg);
    }

    return result;
}

static S32 LOD_r_LINF(HIPLOADDATA* pkg, PKRReadData*)
{
    S32 result = TRUE;
    S32 ival = 0;

    g_hiprf->readLongs(pkg, &ival, 1);

    return result;
}

static S32 LOD_r_LHDR(HIPLOADDATA* pkg, PKRReadData* pr)
{
    S32 result = TRUE;
    U32 cid = 0;
    S32 ival = 0;
    LAYER_TYPE laytyp = PKR_LTYPE_NOMORE;
    S32 refcnt = 0;
    S32 idx = -1;
    S32 i = 0;
    PKRLNode* laynode = NULL;
    PKRANode* assnode = NULL;

    g_hiprf->readLongs(pkg, &ival, 1);
    laytyp = (LAYER_TYPE)ival;

    g_hiprf->readLongs(pkg, &refcnt, 1);

    laynode = PKR_newlaynode(laytyp, refcnt);
    XOrdAppend(&pr->laytoc, laynode);

    for (i = 0; i < refcnt; i++) {
        g_hiprf->readLongs(pkg, &ival, 1);
        
        idx = XOrdLookup(&pr->asstoc, (void*)ival, OrdTest_R_AssetID);
        assnode = (PKRANode*)pr->asstoc.list[idx];
        XOrdAppend(&laynode->assref, assnode);

        if (i != refcnt - 1) {
            laynode->laysize += assnode->d_size + assnode->d_pad;
        }
        else {
            laynode->laysize += assnode->d_size;
        }
    }

    if (laynode->laysize > 0) {
        laynode->laysize = (laynode->laysize + 0x7FF) & ~0x7FF;
    }
    
    cid = g_hiprf->enter(pkg);
    while (cid) {
        switch (cid) {
        case 'LDBG':
            LOD_r_LDBG(pkg, pr, laynode);
            break;
        }

        g_hiprf->exit(pkg);
        cid = g_hiprf->enter(pkg);
    }

    return result;
}

static S32 LOD_r_LDBG(HIPLOADDATA* pkg, PKRReadData* pr, PKRLNode* laynode)
{
    S32 result = TRUE;
    S32 ival = 0;
    S32 amt = 0;

    if (pr->subver > PKR_COMPAT_VER) {
        amt = g_hiprf->readLongs(pkg, &ival, 1);
        laynode->chksum = ival;
    }

    return result;
}

static S32 LOD_r_STRM(HIPLOADDATA* pkg, PKRReadData* pr)
{
    S32 result = TRUE;
    U32 cid = 0;

    cid = g_hiprf->enter(pkg);
    while (cid) {
        switch (cid) {
        case 'DHDR':
            LOD_r_DHDR(pkg, pr);
            break;
        case 'DPAK':
            LOD_r_DPAK(pkg, pr);
            break;
        }

        g_hiprf->exit(pkg);
        cid = g_hiprf->enter(pkg);
    }

    return result;
}

static S32 LOD_r_DHDR(HIPLOADDATA* pkg, PKRReadData*)
{
    S32 result = TRUE;
    S32 ival = 0;

    g_hiprf->readLongs(pkg, &ival, 1);

    return result;
}

static S32 LOD_r_DPAK(HIPLOADDATA*, PKRReadData*)
{
    S32 result = TRUE;
    return result;
}

static void PKR_spew_verhist()
{
}

PKRAssetType* PKR_type2typeref(U32 type, PKRAssetType* typelist)
{
    PKRAssetType* da_type = NULL;
    PKRAssetType* tmptype = NULL;
    S32 safety = 0;

    tmptype = typelist;
    if (tmptype) {
        while (tmptype->typetag) {
            if (tmptype->typetag == type) {
                da_type = tmptype;
                break;
            }

            tmptype++;
        }
    }

    return da_type;
}

static void PKR_bld_typecnt(PKRReadData* pr)
{
    PKRLNode* laynode = NULL;
    PKRANode* assnode = NULL;
    S32 i = 0;
    S32 j = 0;
    S32 idx = -1;
    S32 typcnt[PKR_MAX_TYPES+1] = {};
    XORDEREDARRAY* tmplist = NULL;
    U32 lasttype = 0;
    S32 lasttidx = 0;
    S32 hits = 0;
    S32 miss = 0;
    PKR_LAYER_LOAD_DEST loadDest = PKR_LDDEST_NOMORE;
    U32 memTag = 0;
    U32 assSize = 0;

    for (j = 0; j < pr->laytoc.cnt; j++) {
        laynode = (PKRLNode*)pr->laytoc.list[j];
        for (i = 0; i < laynode->assref.cnt; i++) {
            assnode = (PKRANode*)laynode->assref.list[i];
            if (assnode->loadflag & PKR_LDASS_ISDUP) {
                continue;
            }

            if (assnode->loadflag & PKR_LDASS_ISNIL) {
                continue;
            }
            
            if (lasttype && assnode->asstype == lasttype) {
                idx = lasttidx;
                hits++;
            }
            else {
                idx = PKR_typeHdlr_idx(pr, assnode->asstype);
                
                lasttidx = idx;
                lasttype = assnode->asstype;
                miss++;
            }

            if (idx < 0) {
                typcnt[PKR_MAX_TYPES]++;
            }
            else typcnt[idx]++;
        }
    }
    
    for (i = 0; i < PKR_MAX_TYPES+1; i++) {
        if (typcnt[i] >= 1) {
            tmplist = &pr->typelist[i];

            XOrdInit(tmplist,
                     typcnt[i] > 1 ? typcnt[i] : 2,
                     FALSE);
        }
    }
    for (j = 0; j < pr->laytoc.cnt; j++) {
        laynode = (PKRLNode*)pr->laytoc.list[j];

        for (i = 0; i < laynode->assref.cnt; i++) {
            assnode = (PKRANode*)laynode->assref.list[i];

            if (assnode->loadflag & PKR_LDASS_ISDUP) continue;
            if (assnode->loadflag & PKR_LDASS_ISNIL) continue;

            if (lasttype && assnode->asstype == lasttype) {
                idx = lasttidx;
                hits++;
            }
            else {
                idx = PKR_typeHdlr_idx(pr, assnode->asstype);

                lasttidx = idx;
                lasttype = assnode->asstype;
                miss++;
            }

            if (idx < 0) tmplist = &pr->typelist[PKR_MAX_TYPES];
            else {
                tmplist = &pr->typelist[idx];
            }

            XOrdAppend(tmplist, assnode);
        }
    }
}

static S32 PKR_typeHdlr_idx(PKRReadData* pr, U32 type)
{
    S32 da_idx = -1;
    S32 cnt = 0;
    const PKRAssetType* atype = NULL;

    atype = pr->types;
    while (atype->typetag) {
        if (atype->typetag == type) {
            da_idx = cnt;
            break;
        }
        cnt++;
        atype++;
    }

    return da_idx;
}

void PKR_alloc_chkidx()
{
}

void* PKR_getmem(U32 id, S32 amount, U32 aid, S32 align)
{
    return PKR_getmem(id, amount, aid, align, FALSE, NULL);
}

void* PKR_getmem(U32 id, S32 amount, U32 aid, S32 align, S32 isTemp, char** memtru)
{
    void* memptr = NULL;
    if (!amount) return memptr;
    
    if (isTemp) {
        memptr = xMemPushTemp(amount + align);
        if (memtru) *memtru = (char*)memptr;
        if (align) memptr = (void*)((((S32)memptr) + align - 1) & -align);
    }
    else memptr = xMALLOCALIGN(amount, align);

    if (memptr) memset(memptr, 0, amount);

    g_memalloc_pair++;
    g_memalloc_runtot += amount;
    if (g_memalloc_runtot < 0) g_memalloc_runtot = amount;
    
    return memptr;
}

void PKR_relmem(U32 id, S32 blksize, void* memptr, U32 aid, S32 isTemp)
{
    g_memalloc_pair--;
    g_memalloc_runfree += blksize;
    if (g_memalloc_runfree < 0) g_memalloc_runfree = blksize;

    if (memptr) {
        if (blksize > 0) {
            if (isTemp) xMemPopTemp(memptr);
        }
    }
}

static S32 PKR_push_memmark()
{
    S32 curlvl = 0;
    curlvl = xMemPushBase();
    return curlvl;
}

static S32 PKR_pop_memmark()
{
    S32 curlvl = 0;
    curlvl = xMemGetBase() - 1;
    curlvl = xMemPopBase(curlvl);
    return curlvl;
}