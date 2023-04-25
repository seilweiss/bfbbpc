#include "xstransvc.h"

#include "iTRC.h"
#include "iSystem.h"

#include "xString.h"
#include "xutil.h"

#include <string.h>
#include <stdio.h>

#define STRAN_MAX_SCENES 16

struct st_STRAN_SCENE
{
    U32 scnid;
    S32 lockid;
    st_PACKER_READ_DATA* spkg;
    S32 isHOP;
    void* userdata;
    char fnam[256];
};

struct st_STRAN_DATA
{
    st_STRAN_SCENE hipscn[STRAN_MAX_SCENES];
    U32 loadlock;
};

static S32 g_straninit = 0;
static st_STRAN_DATA g_xstdata = {};
static st_PACKER_READ_FUNCS* g_pkrf = NULL;
static st_PACKER_ASSETTYPE* g_typeHandlers = NULL;

static S32 XST_PreLoadScene(st_STRAN_SCENE* sdata, const char* sfile);
static char* XST_translate_sid(U32 sid, char* exten);
static char* XST_translate_sid_path(U32 sid, char* exten);
static void XST_reset_raw();
static st_STRAN_SCENE* XST_lock_next();
static void XST_unlock(st_STRAN_SCENE* sdata);
static void XST_unlock_all();
static st_STRAN_SCENE* XST_get_rawinst(S32 idx);
static S32 XST_cnt_locked();
static st_STRAN_SCENE* XST_nth_locked(S32 nth);
static st_STRAN_SCENE* XST_find_bySID(U32 sid, S32 findTheHOP);

S32 xSTStartup(st_PACKER_ASSETTYPE* handlers)
{
    if (!g_straninit++) {
        g_typeHandlers = handlers;
        XST_reset_raw();
        PKRStartup();
        g_pkrf = PKRGetReadFuncs(1);
    }
    return g_straninit;
}

S32 xSTShutdown()
{
    if (!--g_straninit) {
        g_typeHandlers = NULL;
        XST_unlock_all();
        PKRShutdown();
    }
    return g_straninit;
}

S32 xSTPreLoadScene(U32 sid, void* userdata, S32 flg_hiphop)
{
    S32 result = 1;
    st_STRAN_SCENE* sdata = NULL;
    const char* sfile = NULL;
    S32 cltver = 0;

    if ((flg_hiphop & XST_OPTS_MASKHH) == XST_OPTS_HOP) {
        sdata = XST_lock_next();
        sdata->scnid = sid;
        sdata->userdata = userdata;
        sdata->isHOP = 1;

        sfile = XST_translate_sid_path(sid, ".HOP");
        if (sfile) {
            strcpy(sdata->fnam, sfile);
            cltver = XST_PreLoadScene(sdata, sfile);
        }
        if (cltver == 0) {
            sfile = XST_translate_sid(sid, ".HOP");
            if (sfile) {
                strcpy(sdata->fnam, sfile);
                cltver = XST_PreLoadScene(sdata, sfile);
            }
        }
        if (cltver == 0) {
            XST_unlock(sdata);
            result = 0;
        } else {
            result = cltver;
        }
    } else {
        do {
            sdata = XST_lock_next();
            sdata->scnid = sid;
            sdata->userdata = userdata;
            sdata->isHOP = 0;

            if (sid != IDTAG('B','O','O','T') &&
                sid != IDTAG('F','O','N','T')) {
                sfile = XST_translate_sid_path(sid, ".HIP");
                if (sfile) {
                    strcpy(sdata->fnam, sfile);
                    cltver = XST_PreLoadScene(sdata, sfile);
                }
            }
            if (cltver == 0) {
                sfile = XST_translate_sid(sid, ".HIP");
                if (sfile) {
                    strcpy(sdata->fnam, sfile);
                    cltver = XST_PreLoadScene(sdata, sfile);
                }
            }
            if (cltver == 0) {
                XST_unlock(sdata);
                result = 0;
            } else {
                result = cltver;
            }

            iSystemPollEvents();
        } while (cltver == 0 && !iSystemShouldQuit());
    }

    return result;
}

S32 xSTQueueSceneAssets(U32 sid, S32 flg_hiphop)
{
    S32 result = 1;
    st_STRAN_SCENE* sdata = NULL;
    S32 doTheHOP = ((flg_hiphop & XST_OPTS_MASKHH) == XST_OPTS_HOP) ? 1 : 0;

    sdata = XST_find_bySID(sid, doTheHOP);
    if (!sdata) {
        result = 0;
    } else if (sdata->spkg) {
        g_pkrf->LoadLayer(sdata->spkg, PKR_LTYPE_ALL);
    }

    return result;
}

void xSTUnLoadScene(U32 sid, S32 flg_hiphop)
{
    st_STRAN_SCENE* sdata = NULL;
    S32 cnt = 0;
    S32 i = 0;

    if (sid == 0) {
        cnt = XST_cnt_locked();
        for (i = 0; i < cnt; i++) {
            sdata = XST_nth_locked(i);
            if (sdata->spkg) {
                g_pkrf->Done(sdata->spkg);
            }
            sdata->spkg = NULL;
        }
        XST_unlock_all();
    } else {
        S32 doTheHOP = ((flg_hiphop & XST_OPTS_MASKHH) == XST_OPTS_HOP) ? 1 : 0;
        sdata = XST_find_bySID(sid, doTheHOP);
        if (sdata) {
            if (sdata->spkg) {
                g_pkrf->Done(sdata->spkg);
            }
            sdata->spkg = NULL;
        }
        XST_unlock(sdata);
    }
}

F32 xSTLoadStep(U32)
{
    F32 pct;
    S32 rc = PKRLoadStep(0);
    if (rc) {
        pct = 0.0f;
    } else {
        pct = 1.00001f;
    }

    iTRCDisk::CheckDVDAndResetState();
    iFileAsyncService();

    return pct;
}

void xSTDisconnect(U32 sid, S32 flg_hiphop)
{
    st_STRAN_SCENE* sdata = NULL;
    S32 doTheHOP = ((flg_hiphop & XST_OPTS_MASKHH) == XST_OPTS_HOP) ? 1 : 0;

    sdata = XST_find_bySID(sid, doTheHOP);
    if (sdata) {
        g_pkrf->PkgDisconnect(sdata->spkg);
    }
}

S32 xSTSwitchScene(U32 sid, void* userdata, xSTProgressMonitorCallback progmon)
{
    st_STRAN_SCENE* sdata = NULL;
    S32 rc = 0;
    S32 i = 0;

    for (i = 1; i >= 0; i--) {
        sdata = XST_find_bySID(sid, i);
        if (sdata) {
            if (progmon) {
                progmon(userdata, 0.0f);
            }
            rc = g_pkrf->SetActive(sdata->spkg, PKR_LTYPE_ALL);
            if (progmon) {
                progmon(userdata, 1.0f);
            }
        }
    }
    return rc;
}

const char* xSTAssetName(U32 aid)
{
    const char* aname = NULL;
    S32 scncnt = 0;
    S32 i = 0;
    st_STRAN_SCENE* sdata = NULL;

    scncnt = XST_cnt_locked();
    for (i = 0; i < scncnt; i++) {
        sdata = XST_nth_locked(i);
        if (sdata->spkg) {
            aname = g_pkrf->AssetName(sdata->spkg, aid);
            if (aname) break;
        }
    }
    return aname;
}

const char* xSTAssetName(void* raw_HIP_asset)
{
    const char* aname = NULL;
    S32 scncnt = XST_cnt_locked();

    for (S32 i = 0; i < scncnt; i++) {
        st_STRAN_SCENE* sdata = XST_nth_locked(i);
        U32 aid = PKRAssetIDFromInst(raw_HIP_asset);
        aname = g_pkrf->AssetName(sdata->spkg, aid);
        if (aname) break;
    }

    return aname;
}

void* xSTFindAsset(U32 aid, U32* size)
{
    void* memloc = NULL;
    if (aid == 0) return NULL;

    S32 ready;
    S32 scncnt = XST_cnt_locked();
    for (S32 i = 0; i < scncnt; i++) {
        st_STRAN_SCENE* sdata = XST_nth_locked(i);
        S32 rc = g_pkrf->PkgHasAsset(sdata->spkg, aid);
        if (!rc) continue;

        memloc = g_pkrf->LoadAsset(sdata->spkg, aid, NULL, NULL);
        ready = g_pkrf->IsAssetReady(sdata->spkg, aid);
        if (!ready) {
            memloc = NULL;
            if (size) *size = 0;
        } else {
            if (size) *size = g_pkrf->GetAssetSize(sdata->spkg, aid);
        }

        break;
    }

    return memloc;
}

S32 xSTAssetCountByType(U32 type)
{
    S32 sum = 0;
    S32 cnt = 0;
    st_STRAN_SCENE* sdata = NULL;
    S32 scncnt = 0;
    S32 i = 0;

    scncnt = XST_cnt_locked();
    for (i = 0; i < scncnt; i++) {
        sdata = XST_nth_locked(i);
        cnt = g_pkrf->AssetCount(sdata->spkg, type);
        sum += cnt;
    }

    return sum;
}

void* xSTFindAssetByType(U32 type, S32 idx, U32* size)
{
    st_STRAN_SCENE* sdata = NULL;
    void* memptr = NULL;
    S32 scncnt = 0;
    S32 i = 0;
    S32 sum = 0;
    S32 cnt = 0;

    scncnt = XST_cnt_locked();
    for (i = 0; i < scncnt; i++) {
        sdata = XST_nth_locked(i);
        cnt = g_pkrf->AssetCount(sdata->spkg, type);
        if (idx >= sum && idx < sum + cnt) {
            memptr = g_pkrf->AssetByType(sdata->spkg, type, idx - sum, size);
            break;
        }
        sum += cnt;
    }

    return memptr;
}

S32 xSTGetAssetInfo(U32 aid, st_PKR_ASSET_TOCINFO* tocainfo)
{
    S32 found = 0;
    S32 scncnt = XST_cnt_locked();

    for (S32 i = 0; i < scncnt; i++) {
        st_STRAN_SCENE* sdata = XST_nth_locked(i);
        S32 rc = g_pkrf->PkgHasAsset(sdata->spkg, aid);
        if (!rc) continue;

        g_pkrf->GetBaseSector(sdata->spkg);
        rc = g_pkrf->GetAssetInfo(sdata->spkg, aid, tocainfo);
        if (rc) {
            found = 1;
            break;
        }
    }

    return found;
}

S32 xSTGetAssetInfoByType(U32 type, S32 idx, st_PKR_ASSET_TOCINFO* ainfo)
{
    S32 found = 0;
    st_PKR_ASSET_TOCINFO tocinfo = {};
    S32 rc = 0;
    S32 scncnt = 0;
    S32 i = 0;
    st_STRAN_SCENE* sdata = NULL;
    S32 sum = 0;
    S32 cnt = 0;
    
    memset(ainfo, 0, sizeof(st_PKR_ASSET_TOCINFO));

    scncnt = XST_cnt_locked();
    for (i = 0; i < scncnt; i++) {
        sdata = XST_nth_locked(i);
        cnt = g_pkrf->AssetCount(sdata->spkg, type);

        if (idx >= sum && idx < sum + cnt) {
            g_pkrf->GetBaseSector(sdata->spkg);
            rc = g_pkrf->GetAssetInfoByType(sdata->spkg, type, idx - sum, &tocinfo);
            if (rc) {
                found = 1;
                ainfo->aid = tocinfo.aid;
                ainfo->sector = tocinfo.sector;
                ainfo->plus_offset = tocinfo.plus_offset;
                ainfo->size = tocinfo.size;
                ainfo->mempos = tocinfo.mempos;
                break;
            }
        }

        sum += cnt;
    }

    return found;
}

S32 xSTGetAssetInfoInHxP(U32 aid, st_PKR_ASSET_TOCINFO* ainfo, U32 scnhash)
{
    S32 found = 0;
    S32 scncnt = XST_cnt_locked();

    for (S32 i = 0; i < scncnt; i++) {
        st_STRAN_SCENE* sdata = XST_nth_locked(i);
        U32 hash = xStrHash(sdata->fnam);
        if (scnhash != hash) continue;

        S32 rc = g_pkrf->PkgHasAsset(sdata->spkg, aid);
        if (!rc) continue;

        g_pkrf->GetBaseSector(sdata->spkg);
        rc = g_pkrf->GetAssetInfo(sdata->spkg, aid, ainfo);
        if (rc) {
            found = 1;
            break;
        }
    }

    return found;
}

const char* xST_xAssetID_HIPFullPath(U32 aid)
{
    return xST_xAssetID_HIPFullPath(aid, NULL);
}

const char* xST_xAssetID_HIPFullPath(U32 aid, U32* sceneID)
{
    const char* da_hipname = NULL;
    st_STRAN_SCENE* sdata = NULL;
    S32 rc = 0;
    S32 scncnt = 0;
    S32 i = 0;

    scncnt = XST_cnt_locked();
    for (i = 0; i < scncnt; i++) {
        sdata = XST_nth_locked(i);
        rc = g_pkrf->PkgHasAsset(sdata->spkg, aid);
        if (!rc) continue;

        da_hipname = sdata->fnam;
        if (sceneID != 0) {
            *sceneID = sdata->scnid;
        }

        break;
    }

    return da_hipname;
}

static S32 XST_PreLoadScene(st_STRAN_SCENE* sdata, const char* sfile) NONMATCH("https://decomp.me/scratch/iRJmZ")
{
    S32 cltver = 0;
    sdata->spkg = g_pkrf->Init(sdata->userdata, (char*)sfile, 0x2E, &cltver, g_typeHandlers);
    return sdata->spkg ? cltver : 0;
}

static char* XST_translate_sid(U32 sid, char* exten)
{
    static char fname[64] = {};
    sprintf(fname, "%s%s", xUtil_idtag2string(sid), exten);
    return fname;
}

static char* XST_translate_sid_path(U32 sid, char* exten)
{
    static char fname[64] = {};
    
    char path_delimiter[2] = "/";
    sprintf(fname, "%c%c%s%s%s",
            xUtil_idtag2string(sid)[0],
            xUtil_idtag2string(sid)[1],
            path_delimiter,
            xUtil_idtag2string(sid),
            exten);

    return fname;
}

static void XST_reset_raw()
{
    memset(&g_xstdata, 0, sizeof(g_xstdata));
}

static st_STRAN_SCENE* XST_lock_next()
{
    st_STRAN_SCENE* sdata = NULL;
    S32 i = 0;
    S32 uselock = -1;

    for (i = 0; i < STRAN_MAX_SCENES; i++) {
        if (!(g_xstdata.loadlock & (1<<i))) {
            g_xstdata.loadlock |= (1<<i);
            
            sdata = &g_xstdata.hipscn[i];
            memset(sdata, 0, sizeof(st_STRAN_SCENE));

            uselock = i;
            break;
        }
    }

    if (sdata) {
        sdata->lockid = uselock;
    }

    return sdata;
}

static void XST_unlock(st_STRAN_SCENE* sdata) NONMATCH("https://decomp.me/scratch/nn8wn")
{
    if (!sdata) return;

    if (g_xstdata.loadlock & (1<<sdata->lockid)) {
        g_xstdata.loadlock &= ~(1<<sdata->lockid);
        memset(sdata, 0, sizeof(st_STRAN_SCENE));
    }
}

static void XST_unlock_all()
{
    st_STRAN_SCENE* sdata = NULL;
    S32 i = 0;
    
    if (g_xstdata.loadlock == 0) return;

    for (i = 0; i < STRAN_MAX_SCENES; i++) {
        if (g_xstdata.loadlock & (1<<i)) {
            sdata = XST_get_rawinst(i);
            XST_unlock(sdata);
        }
    }
}

static st_STRAN_SCENE* XST_get_rawinst(S32 idx)
{
    return &g_xstdata.hipscn[idx];
}

static S32 XST_cnt_locked()
{
    S32 cnt = 0;
    S32 i = 0;

    for (i = 0; i < STRAN_MAX_SCENES; i++) {
        if (g_xstdata.loadlock & (1<<i)) {
            cnt++;
        }
    }

    return cnt;
}

static st_STRAN_SCENE* XST_nth_locked(S32 nth)
{
    st_STRAN_SCENE* sdata = NULL;
    S32 cnt = 0;
    S32 i = 0;

    for (i = 0; i < STRAN_MAX_SCENES; i++) {
        if (g_xstdata.loadlock & (1<<i)) {
            if (cnt == nth) {
                sdata = &g_xstdata.hipscn[i];
                break;
            }
            cnt++;
        }
    }

    return sdata;
}

static st_STRAN_SCENE* XST_find_bySID(U32 sid, S32 findTheHOP)
{
    st_STRAN_SCENE* da_sdata = NULL;
    S32 i = 0;
    st_STRAN_SCENE* tmp_sdata = NULL;

    for (i = 0; i < STRAN_MAX_SCENES; i++) {
        if (g_xstdata.loadlock & (1<<i)) {
            tmp_sdata = &g_xstdata.hipscn[i];
            if (tmp_sdata->scnid == sid &&
                (findTheHOP || !tmp_sdata->isHOP) && (!findTheHOP || tmp_sdata->isHOP)) {
                da_sdata = tmp_sdata;
                break;
            }
        }
    }

    return da_sdata;
}