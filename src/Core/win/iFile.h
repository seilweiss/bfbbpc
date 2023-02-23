#pragma once

#include "types.h"

typedef struct tag_xFile xFile;

#define IFILE_NAMELEN_MAX 128

#define IFILE_OPENED 1

#define IFILE_READ (1<<0)
#define IFILE_WRITE (1<<1)
#define IFILE_ABSPATH (1<<2)

#define IFILE_SEEK_SET 0
#define IFILE_SEEK_CUR 1
#define IFILE_SEEK_END 2

typedef struct tag_iFile iFile;
struct tag_iFile
{
    U32 flags;
    char path[IFILE_NAMELEN_MAX];
    S32 fd;
};

enum IFILE_READSECTOR_STATUS
{
    IFILE_RDSTAT_NOOP,
    IFILE_RDSTAT_INPROG,
    IFILE_RDSTAT_DONE,
    IFILE_RDSTAT_FAIL,
    IFILE_RDSTAT_QUEUED,
    IFILE_RDSTAT_EXPIRED
};

typedef void(*iFileReadAsyncDoneCallback)(xFile*);

void iFileInit();
void iFileExit();
U32* iFileLoad(const char* name, U32* buffer, U32* size);
U32 iFileOpen(const char* name, S32 flags, xFile* file);
S32 iFileSeek(xFile* file, S32 offset, S32 whence);
U32 iFileRead(xFile* file, void* buf, U32 size);
S32 iFileReadAsync(xFile* file, void* buf, U32 asize, iFileReadAsyncDoneCallback dcb, S32 priority);
IFILE_READSECTOR_STATUS iFileReadAsyncStatus(S32 key, S32* bytes_read);
U32 iFileClose(xFile* file);
U32 iFileGetSize(xFile* file);
void iFileReadStop();
void iFileFullPath(const char* relname, char* fullname);
void iFileSetPath(const char* path);
U32 iFileFind(const char* name, xFile* file);
void iFileGetInfo(xFile* file, U32* starting_sector, U32* size_in_bytes);

inline void iFileAsyncService() {}