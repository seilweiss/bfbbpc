#include "iAnimSKB.h"

#include <rtquat.h>
#include <rtslerp.h>

void iAnimEvalSKB(iAnimSKBHeader* data, F32 time, U32 flags, xVec3* tran, xQuat* quat) NONMATCH("https://decomp.me/scratch/dd8Lo")
{
    U32 i, tidx;
    U32 bcount = data->BoneCount;
    U32 tcount = data->TimeCount;
    iAnimSKBKey* keys = (iAnimSKBKey*)(data + 1);
    F32* times = (F32*)(keys + data->KeyCount);
    U16* offsets = (U16*)(times + tcount);

    if (time < 0.0f) time = 0.0f;
    if (time > times[tcount-1]) time = times[tcount-1];

    tidx = (tcount-1) & 3;
    while (times[tidx] < time) tidx += 4;
    while (tidx != 0 && time <= times[tidx]) tidx--;
    
    offsets += tidx * bcount;
    if (flags & 0x1) bcount = 1;
    if (flags & 0x2) {
        bcount--;
        offsets++;
    }

    if (tcount == 1) {
        F32 scalex = data->Scale[0];
        F32 scaley = data->Scale[1];
        F32 scalez = data->Scale[2];
        for (i = 0; i < bcount; i++) {
            iAnimSKBKey* k = &keys[i*2];
            quat->v.x = (1/32767.0f) * k->Quat[0];
            quat->v.y = (1/32767.0f) * k->Quat[1];
            quat->v.z = (1/32767.0f) * k->Quat[2];
            quat->s = (1/32767.0f) * k->Quat[3];
            tran->x = scalex * k->Tran[0];
            tran->y = scaley * k->Tran[1];
            tran->z = scalez * k->Tran[2];
            quat++;
            tran++;
        }
    } else {
        F32 scalex = data->Scale[0];
        F32 scaley = data->Scale[1];
        F32 scalez = data->Scale[2];
        for (i = 0; i < bcount; i++) {
            F32 time1, time2, lerp;
            iAnimSKBKey* k = &keys[(*offsets)++];
            time1 = times[k->TimeIndex];
            time2 = times[(k+1)->TimeIndex];
            lerp = (time - time1) / (time2 - time1);

            RtQuat q1, q2;
            q1.imag.x = (1/32767.0f) * k->Quat[0];
            q1.imag.y = (1/32767.0f) * k->Quat[1];
            q1.imag.z = (1/32767.0f) * k->Quat[2];
            q1.real = (1/32767.0f) * k->Quat[3];
            q2.imag.x = (1/32767.0f) * (k+1)->Quat[0];
            q2.imag.y = (1/32767.0f) * (k+1)->Quat[1];
            q2.imag.z = (1/32767.0f) * (k+1)->Quat[2];
            q2.real = (1/32767.0f) * (k+1)->Quat[3];
            
            RtQuatSlerpCache qcache;
            RtQuatSetupSlerpCache(&q1, &q2, &qcache);
            RtQuatSlerp((RtQuat*)quat, &q1, &q2, lerp, &qcache);

            xVec3 t1, t2;
            t1.x = scalex * k->Tran[0];
            t1.y = scaley * k->Tran[1];
            t1.z = scalez * k->Tran[2];
            t2.x = scalex * (k+1)->Tran[0];
            t2.y = scaley * (k+1)->Tran[1];
            t2.z = scalez * (k+1)->Tran[2];

            tran->x = xlerp(t1.x, t2.x, lerp);
            tran->y = xlerp(t1.y, t2.y, lerp);
            tran->z = xlerp(t1.z, t2.z, lerp);

            quat++;
            tran++;
        }
    }
}

F32 iAnimDurationSKB(iAnimSKBHeader* data)
{
    return ((F32*)((iAnimSKBKey*)(data + 1) + data->KeyCount))[data->TimeCount-1];
}

// https://decomp.me/scratch/6pV2n
void _iAnimSKBAdjustTranslate(iAnimSKBHeader* data, U32 bone, F32* starttran, F32* endtran) WIP
{
}

// https://decomp.me/scratch/Gh6j4
S32 _iAnimSKBExtractTranslate(iAnimSKBHeader* data, U32 bone, xVec3* tranArray, S32 tranCount) WIP
{
    return 0;
}