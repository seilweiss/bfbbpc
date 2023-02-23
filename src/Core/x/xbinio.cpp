#include "xbinio.h"

#include <string.h>

typedef struct st_BINIO_XTRADATA BINIO_XTRADATA;
struct st_BINIO_XTRADATA
{
    char* dbl_buf;
    S32 dblbuf_size;
    S32 dbl_beg;
    S32 dbl_end;
    S32 dbl_amt;
    U32 fpos;
    char* asyn_data;
    S32 asyn_amt;
    S32 asyn_elesize;
    S32 asyn_ismot;
    BIO_ASYNC_ERRCODES asyn_status;
    U32 pad[3];
    S32 gcaskey;
};

static U32 g_loadlock = 0xFFFFFF00;
static FILELOADINFO g_loadinst[8] = {};
static xFile g_xfload[8] = {};
static BINIO_XTRADATA g_xtraload[8] = {};
static BINIO_XTRADATA* g_async_context = NULL;

static void LoadDestroy(FILELOADINFO* fli);
static S32 SkipBytes(FILELOADINFO* fli, S32 fwd);
static S32 ReadSeek(FILELOADINFO* fli, S32 pos);
static void SetBuffer(FILELOADINFO* fli, char* dblbuffer, S32 bufsize);
static void DiscardBuffer(FILELOADINFO* fli);
static S32 ReadRaw(FILELOADINFO* fli, void* data, S32 size, S32 count);
static S32 ReadBytes(FILELOADINFO* fli, char* data, S32 count);
static S32 ReadMShorts(FILELOADINFO* fli, S16* data, S32 count);
static S32 ReadMLongs(FILELOADINFO* fli, S32* data, S32 count);
static S32 ReadMFloats(FILELOADINFO* fli, F32* data, S32 count);
static S32 ReadMDoubles(FILELOADINFO* fli, F64* data, S32 count);
static S32 ReadIShorts(FILELOADINFO* fli, S16* data, S32 count);
static S32 ReadILongs(FILELOADINFO* fli, S32* data, S32 count);
static S32 ReadIFloats(FILELOADINFO* fli, F32* data, S32 count);
static S32 ReadIDoubles(FILELOADINFO* fli, F64* data, S32 count);
static S32 AsyncMRead(FILELOADINFO* fli, S32 offset, char* data, S32 size, S32 n);
static S32 AsyncIRead(FILELOADINFO* fli, S32 offset, char* data, S32 size, S32 n);
static en_BIO_ASYNC_ERRCODES AsyncReadStatus(FILELOADINFO* fli);

static void Swap2(char* d, S32 n);
static void Swap4(char* d, S32 n);
static void Swap8(char* d, S32 n);

#ifdef LITTLE_ENDIAN
#define SwapM2(d, n) Swap2((d), (n))
#define SwapM4(d, n) Swap4((d), (n))
#define SwapM8(d, n) Swap8((d), (n))
#define SwapI2(d, n)
#define SwapI4(d, n)
#define SwapI8(d, n)
#else
#define SwapM2(d, n)
#define SwapM4(d, n)
#define SwapM8(d, n)
#define SwapI2(d, n) Swap2((d), (n))
#define SwapI4(d, n) Swap4((d), (n))
#define SwapI8(d, n) Swap8((d), (n))
#endif

static xFile* BFD_open(const char* filename, const char* mode, U32 lockid, S32, void* xtradata);
static void BFD_close(xFile* bffp, void* xtradata);
static S32 BFD_read(void* data, S32 elesize, S32 elecnt, xFile* bffp, void* xtradata);
static S32 BFD_seek(xFile* bffp, S32 offset, S32 refpos, void* xtradata);
static S32 BFD_getLength(xFile* bffp, void*);
static U32 BFD_startSector(const char* filename);
static void BFD_cb_GCP2_readasync(xFile* file);
static S32 BFD_AsyncRead(FILELOADINFO* fli, S32 pos, void* data, S32 size, S32 n, S32);
static en_BIO_ASYNC_ERRCODES BFD_AsyncReadStatus(FILELOADINFO* fli);

FILELOADINFO* xBinioLoadCreate(const char* filename)
{
    FILELOADINFO* fli = NULL;
    xFile* tmp_fp = NULL;
    S32 i = 0;
    S32 uselock = -1;

    for (i = 0; i < 8; i++) {
        if (!(g_loadlock & (1 << i))) {
            g_loadlock |= 1 << i;
            fli = &g_loadinst[i];
            uselock = i;
            break;
        }
    }

    if (fli) {
        memset(fli, 0, sizeof(FILELOADINFO));
        fli->lockid = (uselock < 0) ? 0 : uselock;
        fli->xtradata = &g_xtraload[uselock];
        fli->destroy = LoadDestroy;
        fli->readBytes = ReadBytes;
        fli->readMShorts = ReadMShorts;
        fli->readMLongs = ReadMLongs;
        fli->readMFloats = ReadMFloats;
        fli->readMDoubles = ReadMDoubles;
        fli->readIShorts = ReadIShorts;
        fli->readILongs = ReadILongs;
        fli->readIFloats = ReadIFloats;
        fli->readIDoubles = ReadIDoubles;
        fli->asyncIRead = AsyncIRead;
        fli->asyncMRead = AsyncMRead;
        fli->asyncReadStatus = AsyncReadStatus;
        fli->skipBytes = SkipBytes;
        fli->seekSpot = ReadSeek;
        fli->setDoubleBuf = SetBuffer;
        fli->discardDblBuf = DiscardBuffer;
        fli->error = FIOERR_NONE;
        fli->remain = 0;
        fli->basesector = 0;
        fli->filesize = 0;
        fli->position = 0;
        fli->basesector = 0;

        tmp_fp = BFD_open(filename, "rb", uselock, 0, fli->xtradata);
        if (tmp_fp) {
            fli->basesector = BFD_startSector(filename);
            fli->privdata = tmp_fp;

            fli->filesize = BFD_getLength(tmp_fp, fli->xtradata);
            fli->remain = fli->filesize;
        } else {
            LoadDestroy(fli);
            fli = NULL;
        }
    }

    return fli;
}

static void LoadDestroy(FILELOADINFO* fli)
{
    xFile* fp = (xFile*)fli->privdata;
    U32 lockid = 0;

    if (fp) BFD_close(fp, fli->xtradata);

    lockid = fli->lockid;
    memset(fli, 0, sizeof(FILELOADINFO));
    g_loadlock &= ~(1 << lockid);
}

static S32 SkipBytes(FILELOADINFO* fli, S32 fwd)
{
    xFile* file = (xFile*)fli->privdata;
    S32 rc = 0;

    if (fli->error != FIOERR_NONE) return 0;

    if (fwd == 0) return 1;
    if (fwd < 0) fwd = 0;

    rc = BFD_seek(file, fli->position + fwd, 0, fli->xtradata);
    if (rc) fli->error = FIOERR_SEEKFAIL;
    else {
        fli->remain -= fwd;
        fli->position += fwd;
    }

    if (fli->error != FIOERR_NONE) return 0;
    return 1;
}

static S32 ReadSeek(FILELOADINFO* fli, S32 pos)
{
    xFile* file = (xFile*)fli->privdata;
    S32 rc = 0;

    if (fli->error != FIOERR_NONE) return 0;

    pos = (pos < fli->filesize) ? pos : fli->filesize;

    rc = BFD_seek(file, pos, 0, fli->xtradata);
    if (rc) fli->error = FIOERR_SEEKFAIL;
    else {
        fli->position = pos;
        fli->remain = fli->filesize - pos;
    }

    if (fli->error != FIOERR_NONE) return 0;
    return 1;
}

static void SetBuffer(FILELOADINFO* fli, char* dblbuffer, S32 bufsize)
{
    BINIO_XTRADATA* xtra = (BINIO_XTRADATA*)fli->xtradata;
    if (xtra) {
        xtra->dbl_buf = dblbuffer; xtra->dblbuf_size = bufsize; xtra->dbl_beg = 0; xtra->dbl_end = 0; xtra->dbl_amt = 0;
    }
}

static void DiscardBuffer(FILELOADINFO* fli)
{
    SetBuffer(fli, NULL, 0);
}

static S32 ReadRaw(FILELOADINFO* fli, void* data, S32 size, S32 count)
{
    xFile* file = (xFile*)fli->privdata;
    S32 amt = 0;
    S32 n = 0;

    if (fli->error != FIOERR_NONE) return amt;

    if ((size * count) > fli->remain) amt = fli->remain / size;
    else amt = count;

    if (amt) {
        n = BFD_read(data, size, amt, file, fli->xtradata);
        if (n != amt) fli->error = FIOERR_READFAIL;

        fli->remain -= size * amt;
        fli->position += size * amt;
    }

    return amt;
}

static S32 ReadBytes(FILELOADINFO* fli, char* data, S32 count)
{
    S32 act = 0;
    act = ReadRaw(fli, data, sizeof(char), count);
    return act;
}

static S32 ReadMShorts(FILELOADINFO* fli, S16* data, S32 count)
{
    S32 act = 0;
    act = ReadRaw(fli, data, sizeof(S16), count);
    SwapM2((char*)data, act);
    return act;
}

static S32 ReadMLongs(FILELOADINFO* fli, S32* data, S32 count)
{
    S32 act = 0;
    act = ReadRaw(fli, data, sizeof(S32), count);
    SwapM4((char*)data, act);
    return act;
}

static S32 ReadMFloats(FILELOADINFO* fli, F32* data, S32 count)
{
    S32 act = 0;
    act = ReadRaw(fli, data, sizeof(F32), count);
    SwapM4((char*)data, act);
    return act;
}

static S32 ReadMDoubles(FILELOADINFO* fli, F64* data, S32 count)
{
    S32 act = 0;
    act = ReadRaw(fli, data, sizeof(F64), count);
    SwapM8((char*)data, act);
    return act;
}

static S32 ReadIShorts(FILELOADINFO* fli, S16* data, S32 count)
{
    S32 act = 0;
    act = ReadRaw(fli, data, sizeof(S16), count);
    SwapI2((char*)data, act);
    return act;
}

static S32 ReadILongs(FILELOADINFO* fli, S32* data, S32 count)
{
    S32 act = 0;
    act = ReadRaw(fli, data, sizeof(S32), count);
    SwapI4((char*)data, act);
    return act;
}

static S32 ReadIFloats(FILELOADINFO* fli, F32* data, S32 count)
{
    S32 act = 0;
    act = ReadRaw(fli, data, sizeof(F32), count);
    SwapI4((char*)data, act);
    return act;
}

static S32 ReadIDoubles(FILELOADINFO* fli, F64* data, S32 count)
{
    S32 act = 0;
    act = ReadRaw(fli, data, sizeof(F64), count);
    SwapI8((char*)data, act);
    return act;
}

static S32 AsyncMRead(FILELOADINFO* fli, S32 offset, char* data, S32 size, S32 n)
{
    return BFD_AsyncRead(fli, offset, data, size, n, 1);
}

static S32 AsyncIRead(FILELOADINFO* fli, S32 offset, char* data, S32 size, S32 n)
{
    return BFD_AsyncRead(fli, offset, data, size, n, 0);
}

static en_BIO_ASYNC_ERRCODES AsyncReadStatus(FILELOADINFO* fli)
{
    return BFD_AsyncReadStatus(fli);
}

static void Swap2(char* d, S32 n)
{
    char t;

    while (n--) {
        t = d[0];
        d[0] = d[1];
        d[1] = t;
        
        d += 2;
    }
}

static void Swap4(char* d, S32 n)
{
    char t = 0;

    while (n--) {
        t = d[0];
        d[0] = d[3];
        d[3] = t;

        t = d[1];
        d[1] = d[2];
        d[2] = t;

        d += 4;
    }
}

static void Swap8(char* d, S32 n)
{
    char t = 0;

    while (n--) {
        t = d[0];
        d[0] = d[7];
        d[7] = t;

        t = d[1];
        d[1] = d[6];
        d[6] = t;

        t = d[2];
        d[2] = d[5];
        d[5] = t;

        t = d[3];
        d[3] = d[4];
        d[4] = t;

        d += 8;
    }
}

static xFile* BFD_open(const char* filename, const char* mode, U32 lockid, S32, void* xtradata)
{
    xFile* bffp = NULL;
    U32 orc = 0;
    BINIO_XTRADATA* xtra = (BINIO_XTRADATA*)xtradata;
    S32 xfflg = 0x1;

    if (strcmp(mode, "rb") == 0) {
        bffp = &g_xfload[lockid];
        xfflg = XFILE_READ;
    }
    else if (strcmp(mode, "wb") == 0) {
        bffp = NULL;
        xfflg = XFILE_WRITE;
    } 
    else {
        bffp = NULL;
    }

    if (bffp) {
        strncpy(bffp->relname, filename, XFILE_RELNAME_MAX-1);
        bffp->relname[XFILE_RELNAME_MAX-1] = '\0';

        orc = iFileOpen(filename, xfflg, bffp);

        if (orc) {
            bffp = NULL;
        } 
        else {
            xtra->fpos = 0;
            xtra->dbl_buf = NULL;
            xtra->dblbuf_size = 0;
            xtra->dbl_beg = 0;
            xtra->dbl_end = 0;
            xtra->dbl_amt = 0;
        }
    }

    return bffp;
}

static void BFD_close(xFile* bffp, void* xtradata)
{
    BINIO_XTRADATA* xtra = (BINIO_XTRADATA*)xtradata;

    iFileClose(bffp);

    xtra->fpos = 0;
    xtra->dbl_buf = NULL;
    xtra->dblbuf_size = 0;
    xtra->dbl_beg = 0;
    xtra->dbl_end = 0;
    xtra->dbl_amt = 0;
}

static S32 BFD_read(void* data, S32 elesize, S32 elecnt, xFile* bffp, void* xtradata)
{
    BINIO_XTRADATA* xtra = (BINIO_XTRADATA*)xtradata;
    char* dest = (char*)data;
    S32 readbeg = 0;
    S32 refill = 0;
    S32 remain = 0;
    S32 actual = 0;
    U32 holdpos = 0;
    S32 toread = 0;
    char* cltoff = NULL;
    char* sectoff = NULL;
    U32 safety = 0;
    
    U32 numBytes = elesize * elecnt;
    if (numBytes==0) return actual;

    if (!xtra->dbl_buf || xtra->dblbuf_size < 1 || (S32)numBytes > xtra->dblbuf_size) {
        iFileSeek(bffp, xtra->fpos, IFILE_SEEK_SET);
        actual = iFileRead(bffp, dest, numBytes);
        xtra->fpos += actual;
    }
    else {
        remain = numBytes;
        
        do {
            if (xtra->fpos < (U32)xtra->dbl_beg) {
                xtra->dbl_beg = xtra->fpos - (xtra->fpos % xtra->dblbuf_size);
                xtra->dbl_end = xtra->dbl_beg + xtra->dblbuf_size;
                xtra->dbl_amt = 0;
                refill = 1;
            }
            else if (xtra->fpos >= (U32)xtra->dbl_end) {
                xtra->dbl_beg = xtra->fpos - (xtra->fpos % xtra->dblbuf_size);
                xtra->dbl_end = xtra->dbl_beg + xtra->dblbuf_size;
                xtra->dbl_amt = 0;
                refill = 1;
            }
            
            if (refill) {
                holdpos = xtra->fpos;
                iFileSeek(bffp, xtra->dbl_beg, IFILE_SEEK_SET);
                xtra->fpos = holdpos;
                xtra->dbl_amt = iFileRead(bffp, xtra->dbl_buf, xtra->dblbuf_size);
            }

            readbeg = xtra->fpos - xtra->dbl_beg;
            
            if (xtra->dbl_amt > readbeg) {
                toread = (remain < xtra->dbl_amt - readbeg) ? remain : xtra->dbl_amt - readbeg;
                cltoff = dest + actual;
                sectoff = xtra->dbl_buf + readbeg;
                memcpy(cltoff, sectoff, toread);

                actual += toread;
                xtra->fpos += toread;
                remain -= toread;
            }
            
            if (actual==numBytes) break;
            if (xtra->dbl_amt < 1) break;
        } while (safety++ <= 60000);
    }

    return actual / elesize;
}

static S32 BFD_seek(xFile* bffp, S32 offset, S32 refpos, void* xtradata)
{
    BINIO_XTRADATA* xtra = (BINIO_XTRADATA*)xtradata;
    S32 rc = 0;

    rc = iFileSeek(bffp, offset, refpos);
    rc = (rc == -1) ? 1 : 0;

    xtra->fpos = offset;

    return rc;
}

static S32 BFD_getLength(xFile* bffp, void*)
{
    S32 len = 0;
    len = iFileGetSize(bffp);
    return len;
}

static U32 BFD_startSector(const char* filename)
{
    xFile xf = {};
    U32 sect_start = 0;
    U32 fsize = 0;
    U32 rc = iFileFind(filename, &xf);
    if (rc==0) {
        iFileGetInfo(&xf, &sect_start, &fsize);
    }
    iFileClose(&xf);
    return sect_start;
}

static void BFD_cb_GCP2_readasync(xFile* file) {}

static S32 BFD_AsyncRead(FILELOADINFO* fli, S32 pos, void* data, S32 size, S32 n, S32)
{
    xFile* file = (xFile*)fli->privdata;
    BINIO_XTRADATA* xtra = (BINIO_XTRADATA*)fli->xtradata;
    S32 result = 0;
    U32 rdsec = 0;
    S32 rc = 0;

    if (fli->error != FIOERR_NONE) return 0;
    if (g_async_context) return 0;

    g_async_context = xtra;

    xtra->asyn_status = BINIO_ASYNC_INPROG;
    xtra->asyn_data = (char*)data;
    xtra->asyn_elesize = size;
    xtra->asyn_amt = n;
    xtra->asyn_ismot = 1;

    rc = iFileSeek(file, pos, IFILE_SEEK_SET);

    xFileSetUserData(file, fli);
    rc = iFileReadAsync(file, data, size * n, BFD_cb_GCP2_readasync, 0);
    if (rc<0) result = 0;
    else {
        xtra->gcaskey = rc;
        result = 1;
    }

    if (result==0) {
        g_async_context = NULL;
        xtra->asyn_status = BINIO_ASYNC_NOOP;
    }

    return (rdsec==0) ? 1 : 0;
}

static en_BIO_ASYNC_ERRCODES BFD_AsyncReadStatus(FILELOADINFO* fli)
{
    en_BIO_ASYNC_ERRCODES status = BINIO_ASYNC_NOOP;
    BINIO_XTRADATA* xtra = (BINIO_XTRADATA*)fli->xtradata;
    XFILE_READSECTOR_STATUS xrdstat = XFILE_RDSTAT_NOOP;

    if (g_async_context==NULL) return status;
    if (xtra != g_async_context) {
        return BINIO_ASYNC_INPROG;
    }

    if (xtra->asyn_status == BINIO_ASYNC_INPROG) {
        S32 amtsofar = 0;
        xrdstat = xFileReadAsyncStatus(xtra->gcaskey, &amtsofar);

        switch (xrdstat)
        {
        case XFILE_RDSTAT_INPROG:
        case XFILE_RDSTAT_QUEUED:
            xtra->asyn_status = BINIO_ASYNC_INPROG;
            break;
        case XFILE_RDSTAT_DONE:
            xtra->asyn_status = BINIO_ASYNC_DONE;
            break;
        case XFILE_RDSTAT_NOOP:
        case XFILE_RDSTAT_FAIL:
        case XFILE_RDSTAT_EXPIRED:
        default:
            xtra->asyn_status = BINIO_ASYNC_FAIL;
            break;
        }
    }

    status = xtra->asyn_status;
    if (status == BINIO_ASYNC_FAIL || status == BINIO_ASYNC_DONE) {
        xtra->asyn_status = BINIO_ASYNC_NOOP;
        g_async_context = NULL;
    }

    return status;
}