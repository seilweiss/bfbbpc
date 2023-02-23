#include "xserializer.h"

#include "xutil.h"
#include "xordarray.h"
#include "xMemMgr.h"

#include <string.h>

struct st_SERIAL_CLIENTINFO
{
    U32 idtag;
    S32* membuf;
    S32 trueoff;
    S32 actsize;
};

typedef struct st_XSERIAL_DATA_PRIV XSERIAL_DATA_PRIV;
struct st_XSERIAL_DATA_PRIV
{
    S32 flg_info;
    S32* bitbuf;
    S32 buf_bytcnt;
    SERIAL_CLIENTINFO* cltbuf;
    SERIAL_CLIENTINFO* cltnext;
    XORDEREDARRAY cltlist;
};

static S32 g_serinit;
static XSERIAL_DATA_PRIV g_xserdata = {};
static S32 g_tbl_onbit[32] = {};
static S32 g_tbl_clear[32] = {};

static void xSER_init_tables();
static void xSER_init_buffers(S32 count, SERIAL_PERCID_SIZE* sizeinfo);
static S32 xSER_ord_compare(void* e1, void* e2);
static S32 xSER_ord_test(const void* key, void* elt);
static SERIAL_CLIENTINFO* XSER_get_client(U32 idtag);

S32 xSerialStartup(S32 count, SERIAL_PERCID_SIZE* sizeinfo)
{
    if (!g_serinit++) {
        memset(&g_xserdata, 0, sizeof(g_xserdata));
        xSER_init_tables();
        xSER_init_buffers(count, sizeinfo);
    }
    return g_serinit;
}

S32 xSerialShutdown()
{
    g_serinit--;
    return g_serinit;
}

void xSerialTraverse(xSerialTraverseCallback func)
{
    S32 i = 0;
    XSERIAL_DATA_PRIV* xsd = &g_xserdata;
    SERIAL_CLIENTINFO* clt = NULL;
    xSerial xser;
    S32 rc = 0;

    for (i = 0; i < xsd->cltlist.cnt; i++) {
        clt = (SERIAL_CLIENTINFO*)xsd->cltlist.list[i];
        xser.setClient(clt->idtag);
        rc = func(clt->idtag, &xser);
        if (!rc) break;
    }
}

xSerial::xSerial()
{
}

xSerial::~xSerial()
{
    if (this->ctxtdata) {
        xUtil_idtag2string(this->idtag);
    }
}

void xSerial::operator delete(void* p)
{
    // BUG: nothing here - mem leak?
}

void xSerial::setClient(U32 idtag)
{
    this->prepare(idtag);
}

S32 xSerial::Write(char* data, S32 elesize, S32 n) NONMATCH("https://decomp.me/scratch/bDy9A")
{
    if (n == 0) return 0;

    S32 nbit;
    S32 numbytes;
    
    if (n > 0) {
        numbytes = n * elesize;
        nbit = numbytes * 8;
    } else {
        nbit = -n;
        numbytes = (nbit + 7) / 8;
        numbytes = (numbytes + 3) / 4;
    }

    if (n < 0) {
        S32 bidx = 0;
        S32* iptr = (S32*)data;
        for (S32 i = 0; i < nbit; i++) {
            S32 bitval = *iptr & g_tbl_onbit[bidx];
            this->wrbit(bitval);
            bidx++;
            if (bidx == 32) {
                bidx = 0;
                iptr++;
                numbytes -= 4;
            }
        }
    } else {
        S32 bidx = 0;
        char* cptr = (char*)data;
        for (S32 i = 0; i < nbit; i++) {
            S32 bitval = *cptr & g_tbl_onbit[bidx];
            this->wrbit(bitval);
            bidx++;
            if (bidx == 8) {
                bidx = 0;
                cptr++;
                numbytes--;
            }
        }
    }

    return nbit;
}

S32 xSerial::Write_b1(S32 bits)
{
    return this->Write((char*)&bits, sizeof(bits), -1);
}

S32 xSerial::Write_b7(U32 bits)
{
    return this->Write((char*)&bits, sizeof(bits), -7);
}

S32 xSerial::Write(U8 data)
{
    return this->Write((char*)&data, sizeof(data), 1);
}

S32 xSerial::Write(S16 data)
{
    return this->Write((char*)&data, sizeof(data), 1);
}

S32 xSerial::Write(S32 data)
{
    return this->Write((char*)&data, sizeof(data), 1);
}

S32 xSerial::Write(U32 data)
{
    return this->Write((char*)&data, sizeof(data), 1);
}

S32 xSerial::Write(F32 data)
{
    return this->Write((char*)&data, sizeof(data), 1);
}

S32 xSerial::Read(char* buf, S32 elesize, S32 n)
{
    S32 nbit = 0;
    S32 numbytes = 0;
    S32* iptr = NULL;
    S32 bidx = 0;
    S32 i = 0;

    if (n > 0) {
        numbytes = n * elesize;
        nbit = numbytes * 8;
    } else {
        nbit = -n;
        numbytes = (nbit + 7) / 8;
        numbytes = (numbytes + 3) / 4;
    }

    if (n < 0) {
        iptr = (S32*)buf;
        bidx = 0;
        for (i = 0; i < nbit; i++) {
            S32 bitval = this->rdbit();
            if (bitval) {
                *iptr |= g_tbl_onbit[bidx];
            } else {
                *iptr &= g_tbl_clear[bidx];
            }
            bidx++;
            if (bidx == 32) {
                bidx = 0;
                iptr++;
                numbytes -= 4;
            }
        }
    } else {
        char* cptr = (char*)buf;
        bidx = 0;
        for (i = 0; i < nbit; i++) {
            S32 bitval = this->rdbit();
            if (bitval) {
                *cptr |= (char)g_tbl_onbit[bidx];
            } else {
                *cptr &= (char)g_tbl_clear[bidx];
            }
            bidx++;
            if (bidx == 8) {
                bidx = 0;
                cptr++;
                numbytes--;
            }
        }
    }

    return nbit;
}

S32 xSerial::Read_b1(S32* bits)
{
    return this->Read((char*)bits, sizeof(*bits), -1);
}

S32 xSerial::Read_b7(U32* bits)
{
    return this->Read((char*)bits, sizeof(*bits), -7);
}

S32 xSerial::Read(U8* buf)
{
    return this->Read((char*)buf, sizeof(*buf), 1);
}

S32 xSerial::Read(S16* buf)
{
    return this->Read((char*)buf, sizeof(*buf), 1);
}

S32 xSerial::Read(S32* buf)
{
    return this->Read((char*)buf, sizeof(*buf), 1);
}

S32 xSerial::Read(U32* buf)
{
    return this->Read((char*)buf, sizeof(*buf), 1);
}

S32 xSerial::Read(F32* buf)
{
    return this->Read((char*)buf, sizeof(*buf), 1);
}

void xSerial::wrbit(S32 is_on)
{
    SERIAL_CLIENTINFO* clt = (SERIAL_CLIENTINFO*)this->ctxtdata;
    
    if (this->bittally + 1 > clt->actsize * 8) {
        this->warned = 1;
        return;
    }

    clt->membuf[this->curele] &= g_tbl_clear[this->bitidx];
    if (is_on) {
        clt->membuf[this->curele] |= g_tbl_onbit[this->bitidx];
    }

    this->bitidx++;
    if (this->bitidx == 32) {
        this->curele++;
        this->bitidx = 0;
    }
    this->bittally++;
}

S32 xSerial::rdbit()
{
    SERIAL_CLIENTINFO* clt = (SERIAL_CLIENTINFO*)this->ctxtdata;
    S32 is_on = 0;

    if (this->bittally + 1 > clt->actsize * 8) {
        this->warned = 1;
        return is_on;
    }

    is_on = (clt->membuf[this->curele] & g_tbl_onbit[this->bitidx]) ? 1 : 0;

    this->bitidx++;
    if (this->bitidx == 32) {
        this->curele++;
        this->bitidx = 0;
    }
    this->bittally++;

    return is_on;
}

void xSerial::prepare(U32 idtag)
{
    SERIAL_CLIENTINFO* clt = NULL;
    clt = XSER_get_client(idtag);
    this->idtag = clt->idtag;
    this->baseoff = clt->trueoff;
    this->ctxtdata = clt;
    this->warned = 0;
    this->curele = 0;
    this->bitidx = 0;
    this->bittally = 0;
}

void xSerialWipeMainBuffer()
{
    XSERIAL_DATA_PRIV* xser = &g_xserdata;
    memset(xser->bitbuf, 0, xser->buf_bytcnt);
}

static void xSER_init_tables()
{
    for (S32 i = 0; i < 32; i++) {
        g_tbl_onbit[i] = (1<<i);
        g_tbl_clear[i] = ~(1<<i);
    }
}

static void xSER_init_buffers(S32 count, SERIAL_PERCID_SIZE* sizeinfo) NONMATCH("https://decomp.me/scratch/uex0C")
{
    XSERIAL_DATA_PRIV* xsd = &g_xserdata;
    S32 i = 0;
    S32 tally = 0;
    S32 sicnt = 0;
    SERIAL_PERCID_SIZE* sitmp = NULL;
    SERIAL_CLIENTINFO* tmp_clt = NULL;

    XOrdInit(&g_xserdata.cltlist, count, 0);
    
    xsd->cltbuf = (SERIAL_CLIENTINFO*)xMALLOC(count * sizeof(SERIAL_CLIENTINFO));
    memset(xsd->cltbuf, 0, count * sizeof(SERIAL_CLIENTINFO));
    xsd->cltnext = xsd->cltbuf;

    sitmp = sizeinfo;
    while (sitmp->idtag != 0) {
        tally += (sitmp->needsize + 3) & ~3;
        sicnt++;
        sitmp++;
    }
    tally += (count - sicnt) * 400;

    xsd->bitbuf = (S32*)xMALLOC(tally);
    memset(xsd->bitbuf, 0, tally);

    xsd->buf_bytcnt = tally;

    sitmp = sizeinfo;
    tally = 0;
    tmp_clt = xsd->cltnext;
    while (sitmp->idtag != 0) {
        tmp_clt->idtag = sitmp->idtag;
        tmp_clt->trueoff = tally;
        tmp_clt->actsize = (sitmp->needsize + 3) & ~3;
        tmp_clt->membuf = xsd->bitbuf + tally / 4;
        XOrdAppend(&xsd->cltlist, tmp_clt);
        tally += tmp_clt->actsize;
        sitmp++;
        tmp_clt++;
    }
    XOrdSort(&xsd->cltlist, xSER_ord_compare);
    xsd->cltnext = tmp_clt;

    for (i = sicnt; i < count; i++) {
        tmp_clt->idtag = 0;
        tmp_clt->trueoff = tally;
        tmp_clt->actsize = 400;
        tmp_clt->membuf = xsd->bitbuf + tally / 4;
        tally += tmp_clt->actsize;
        tmp_clt++;
    }
}

static S32 xSER_ord_compare(void* e1, void* e2)
{
    S32 rc = 0;
    SERIAL_CLIENTINFO* clt_a = (SERIAL_CLIENTINFO*)e1;
    SERIAL_CLIENTINFO* clt_b = (SERIAL_CLIENTINFO*)e2;
    if (clt_a->idtag < clt_b->idtag) rc = -1;
    else if (clt_a->idtag > clt_b->idtag) rc = 1;
    else rc = 0;
    return rc;
}

static S32 xSER_ord_test(const void* key, void* elt)
{
    S32 rc = 0;
    U32 idtag = (U32)key;
    SERIAL_CLIENTINFO* clt = (SERIAL_CLIENTINFO*)elt;
    if (idtag < clt->idtag) rc = -1;
    else if (idtag > clt->idtag) rc = 1;
    else rc = 0;
    return rc;
}

static SERIAL_CLIENTINFO* XSER_get_client(U32 idtag)
{
    XSERIAL_DATA_PRIV* xsd = &g_xserdata;
    SERIAL_CLIENTINFO* clt = NULL;
    S32 idx = -1;

    idx = XOrdLookup(&xsd->cltlist, (void*)idtag, xSER_ord_test);
    if (idx < 0) {
        clt = xsd->cltnext;
        xsd->cltnext++;
        clt->idtag = idtag;
        XOrdInsert(&xsd->cltlist, clt, xSER_ord_compare);
    } else {
        clt = (SERIAL_CLIENTINFO*)xsd->cltlist.list[idx];
    }

    return clt;
}