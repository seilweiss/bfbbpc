#pragma once

#include "xpkrsvc.h"

#define XST_OPTS_HIP (1<<0)
#define XST_OPTS_HOP (2<<0)
#define XST_OPTS_MASKHH (3<<0)

typedef S32(*xSTProgressMonitorCallback)(void*, F32);

S32 xSTStartup(PKRAssetType* handlers);
S32 xSTShutdown();
S32 xSTPreLoadScene(U32 sid, void* userdata, S32 flg_hiphop);
S32 xSTQueueSceneAssets(U32 sid, S32 flg_hiphop);
void xSTUnLoadScene(U32 sid, S32 flg_hiphop);
F32 xSTLoadStep(U32);
void xSTDisconnect(U32 sid, S32 flg_hiphop);
S32 xSTSwitchScene(U32 sid, void* userdata, xSTProgressMonitorCallback progmon);
const char* xSTAssetName(U32 aid);
const char* xSTAssetName(void* raw_HIP_asset);
void* xSTFindAsset(U32 aid, U32* size);
S32 xSTAssetCountByType(U32 type);
void* xSTFindAssetByType(U32 type, S32 idx, U32* size);
S32 xSTGetAssetInfo(U32 aid, PKRAssetTOCInfo* tocainfo);
S32 xSTGetAssetInfoByType(U32 type, S32 idx, PKRAssetTOCInfo* ainfo);
S32 xSTGetAssetInfoInHxP(U32, PKRAssetTOCInfo* ainfo, U32);
const char* xST_xAssetID_HIPFullPath(U32 aid);
const char* xST_xAssetID_HIPFullPath(U32 aid, U32* sceneID);