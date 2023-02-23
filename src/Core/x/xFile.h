#pragma once

#include "iFile.h"

#define XFILE_READ IFILE_READ
#define XFILE_WRITE IFILE_WRITE

#define XFILE_RELNAME_MAX 32

typedef struct tag_xFile xFile;
struct tag_xFile
{
    char relname[XFILE_RELNAME_MAX];
    iFile ps;
    void* user_data;
};

enum XFILE_READSECTOR_STATUS
{
    XFILE_RDSTAT_NOOP,
    XFILE_RDSTAT_INPROG,
    XFILE_RDSTAT_DONE,
    XFILE_RDSTAT_FAIL,
    XFILE_RDSTAT_QUEUED,
    XFILE_RDSTAT_EXPIRED
};

inline void xFileSetUserData(xFile* xf, void* p)
{
    xf->user_data = p;
}

inline XFILE_READSECTOR_STATUS xFileReadAsyncStatus(S32 key, S32* amtSoFar)
{
    return (XFILE_READSECTOR_STATUS)iFileReadAsyncStatus(key, amtSoFar);
}