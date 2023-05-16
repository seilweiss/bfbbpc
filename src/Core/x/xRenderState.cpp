#include "xRenderState.h"

static const S32 sBlendTable[] = {
    rwBLENDZERO,
    rwBLENDONE,
    rwBLENDSRCCOLOR,
    rwBLENDINVSRCCOLOR,
    rwBLENDSRCALPHA,
    rwBLENDINVSRCALPHA,
    rwBLENDDESTALPHA,
    rwBLENDINVDESTALPHA,
    rwBLENDDESTCOLOR,
    rwBLENDINVDESTCOLOR,
    rwBLENDSRCALPHASAT
};

void xRenderStateSetTexture(RwTexture* texture)
{
    if (texture) {
        RwRaster* raster = RwTextureGetRaster(texture);
        if (raster) {
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)raster);
        }
    } else {
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)NULL);
    }
}

void xRenderStateSetSrcBlendMode(S32 xmode)
{
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)sBlendTable[xmode]);
}

void xRenderStateSetDstBlendMode(S32 xmode)
{
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)sBlendTable[xmode]);
}