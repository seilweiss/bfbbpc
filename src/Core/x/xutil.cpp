#include "xutil.h"

#include <ctype.h>

static S32 g_xutilinit = 0;
static S32 g_crc_needinit = 1;
static U32 g_crc32_table[256] = {};

S32 xUtilStartup()
{
    if (!g_xutilinit++) {
        xUtil_crc_init();
    }
    return g_xutilinit;
}

S32 xUtilShutdown() NONMATCH("https://decomp.me/scratch/IvmXZ")
{
    g_xutilinit--;
    return g_xutilinit;
}

char* xUtil_idtag2string(U32 srctag, S32 bufidx)
{
    static char buf[6][10] = {};
    
    U32 tag = srctag;
    char* strptr = NULL;
    char* uc = (char*)&tag;
    S32 is_mot = 0;

    if (bufidx < 0 || bufidx >= 7) {
        strptr = buf[0];
    } else {
        strptr = buf[bufidx];
    }

    // detect endian
    S32 l = 1;
    char* c = (char*)&l;
    is_mot = c[3];

    if (is_mot) {
        char t = uc[0];
        uc[0] = uc[3];
        uc[3] = t;
        t = uc[1];
        uc[1] = uc[2];
        uc[2] = t;
    }

    switch (bufidx) {
    case 4:
    case 5:
        strptr[0] = isprint(uc[0]) ? uc[0] : '?';
        strptr[1] = isprint(uc[1]) ? uc[1] : '?';
        strptr[2] = isprint(uc[2]) ? uc[2] : '?';
        strptr[3] = isprint(uc[3]) ? uc[3] : '?';
        break;
    case 6:
    default:
        strptr[0] = isprint(uc[3]) ? uc[3] : '?';
        strptr[1] = isprint(uc[2]) ? uc[2] : '?';
        strptr[2] = isprint(uc[1]) ? uc[1] : '?';
        strptr[3] = isprint(uc[0]) ? uc[0] : '?';
        break;
    }
    strptr[4] = '\0';
    if (bufidx == 6) {
        strptr[4] = '/';
        strptr[5] = isprint(uc[0]) ? uc[0] : '?';
        strptr[6] = isprint(uc[1]) ? uc[1] : '?';
        strptr[7] = isprint(uc[2]) ? uc[2] : '?';
        strptr[8] = isprint(uc[3]) ? uc[3] : '?';
        strptr[9] = '\0';
    }

    return strptr;
}

U32 xUtil_crc_init()
{
    S32 i = 0;
    S32 j = 0;
    U32 crc_accum = 0;

    if (g_crc_needinit) {
        for (i = 0; i < 256; i++) {
            crc_accum = i;
            crc_accum <<= 24;
            for (j = 0; j < 8; j++) {
                if (crc_accum & 0x80000000) {
                    crc_accum = (crc_accum << 1) ^ 0x4C11DB7;
                } else {
                    crc_accum <<= 1;
                }
            }
            g_crc32_table[i] = crc_accum;
        }
        g_crc_needinit = 0;
    }

    return -1;
}

U32 xUtil_crc_update(U32 crc_accum, char* data, S32 datasize) NONMATCH("https://decomp.me/scratch/qcpGv")
{
    S32 i = 0;
    S32 j = 0;

    if (g_crc_needinit) xUtil_crc_init();

    for (j = 0; j < datasize; data++, j++) {
        i = (crc_accum >> 24) ^ *data;
        i &= 0xFF;
        crc_accum = (crc_accum << 8) ^ g_crc32_table[i];
    }

    return crc_accum;
}

S32 xUtil_yesno(F32 wt_yes)
{
    if (wt_yes == 0.0f) return 0;
    if (wt_yes == 1.0f) return 1;
    if (xurand() <= wt_yes) return 1;
    return 0;
}

void xUtil_wtadjust(F32* wts, S32 cnt, F32 arbref) NONMATCH("https://decomp.me/scratch/OLuDi")
{
    S32 i = 0;
    F32 sum = 0.0f;
    F32 fac = 0.0f;

    for (i = 0; i < cnt; i++) {
        if (wts[i] < 0.0f) wts[i] = -wts[i];
        sum += wts[i];
    }

    fac = arbref / sum;

    for (i = 0; i < cnt; i++) {
        wts[i] *= fac;
    }
}