#pragma once

#include "xFile.h"

enum en_FIOERRCODES
{
    FIOERR_NONE,
    FIOERR_READFAIL,
    FIOERR_WRITEFAIL,
    FIOERR_SEEKFAIL,
    FIOERR_USERABORT
};
typedef enum en_FIOERRCODES FIOERRCODES;

enum en_BIO_ASYNC_ERRCODES
{
    BINIO_ASYNC_FAIL = -1,
    BINIO_ASYNC_NOOP,
    BINIO_ASYNC_INPROG,
    BINIO_ASYNC_DONE,
    BINIO_ASYNC_FORCEENUMSIZEINT = FORCEENUMSIZEINT
};
typedef enum en_BIO_ASYNC_ERRCODES BIO_ASYNC_ERRCODES;

typedef struct st_FILELOADINFO FILELOADINFO;
struct st_FILELOADINFO
{
    void(*destroy)(FILELOADINFO*);
    S32(*readBytes)(FILELOADINFO*, char*, S32);
    S32(*readMShorts)(FILELOADINFO*, S16*, S32);
    S32(*readMLongs)(FILELOADINFO*, S32*, S32);
    S32(*readMFloats)(FILELOADINFO*, F32*, S32);
    S32(*readMDoubles)(FILELOADINFO*, F64*, S32);
    S32(*readIShorts)(FILELOADINFO*, S16*, S32);
    S32(*readILongs)(FILELOADINFO*, S32*, S32);
    S32(*readIFloats)(FILELOADINFO*, F32*, S32);
    S32(*readIDoubles)(FILELOADINFO*, F64*, S32);
    S32(*skipBytes)(FILELOADINFO*, S32);
    S32(*seekSpot)(FILELOADINFO*, S32);
    void(*setDoubleBuf)(FILELOADINFO*, char*, S32);
    void(*discardDblBuf)(FILELOADINFO*);
    S32(*asyncIRead)(FILELOADINFO*, S32, char*, S32, S32);
    S32(*asyncMRead)(FILELOADINFO*, S32, char*, S32, S32);
    BIO_ASYNC_ERRCODES(*asyncReadStatus)(FILELOADINFO*);
    U32 lockid;
    FIOERRCODES error;
    U32 basesector;
    void* privdata;
    void* xtradata;
    void* asyndata;
    S32 filesize;
    S32 remain;
    S32 position;
};

FILELOADINFO* xBinioLoadCreate(const char* filename);