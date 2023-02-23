#include "iFile.h"

#include "xFile.h"
#include "xDebug.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void iFileInit()
{
    xASSERT(sizeof(iFile::fd) == sizeof(FILE*));
}

void iFileExit()
{
}

U32* iFileLoad(const char* name, U32* buffer, U32* size)
{
    char fullpath[IFILE_NAMELEN_MAX];
    iFileFullPath(name, fullpath);

    xFile file;
    iFileOpen(name, IFILE_ABSPATH, &file);

    U32 fsize = iFileGetSize(&file);
    if (!buffer) {
        buffer = (U32*)malloc((fsize + 31) & ~31);
    }

    iFileRead(&file, buffer, fsize);

    if (size) {
        *size = fsize;
    }

    iFileClose(&file);

    return buffer;
}

U32 iFileOpen(const char* name, S32 flags, xFile* file)
{
    iFile* ps = &file->ps;
    char openFlags[16];

    if (flags & IFILE_ABSPATH) {
        strcpy(ps->path, name);
    } else {
        iFileFullPath(name, ps->path);
    }

    if (flags & IFILE_WRITE) {
        strcpy(openFlags, "wb");
    } else if (flags & IFILE_READ) {
        strcpy(openFlags, "rb");
    } else {
        strcpy(openFlags, "rb");
    }

    ps->fd = (S32)fopen(ps->path, openFlags);
    if (!ps->fd) return 1;

    ps->flags = IFILE_OPENED;
    iFileSeek(file, 0, IFILE_SEEK_SET);

    return 0;
}

S32 iFileSeek(xFile* file, S32 offset, S32 whence)
{
    iFile* ps = &file->ps;
    int mode;

    switch (whence) {
    case IFILE_SEEK_SET:
        mode = SEEK_SET;
        break;
    case IFILE_SEEK_CUR:
        mode = SEEK_CUR;
        break;
    case IFILE_SEEK_END:
        mode = SEEK_END;
        break;
    default:
        xASSERTFAILMSG("bad seek mode");
        mode = SEEK_SET;
        break;
    }

    return (S32)fseek((FILE*)ps->fd, offset, mode);
}

U32 iFileRead(xFile* file, void* buf, U32 size)
{
    iFile* ps = &file->ps;
    U32 numItemsRead = (U32)fread(buf, 1, size, (FILE*)ps->fd);
    return numItemsRead;
}

S32 iFileReadAsync(xFile* file, void* buf, U32 asize, iFileReadAsyncDoneCallback dcb, S32 priority)
{
    iFile* ps = &file->ps;
    fread(buf, 1, asize, (FILE*)ps->fd);
    if (dcb) dcb(file);
    return 0;
}

IFILE_READSECTOR_STATUS iFileReadAsyncStatus(S32 key, S32* bytes_read)
{
    return IFILE_RDSTAT_DONE;
}

U32 iFileClose(xFile* file)
{
    iFile* ps = &file->ps;
    fclose((FILE*)ps->fd);
    ps->flags = 0;
    return 0;
}

U32 iFileGetSize(xFile* file)
{
    iFile* ps = &file->ps;
    long cur = ftell((FILE*)ps->fd);
    fseek((FILE*)ps->fd, 0, SEEK_END);
    long end = ftell((FILE*)ps->fd);
    fseek((FILE*)ps->fd, cur, SEEK_SET);
    return (U32)end;
}

void iFileReadStop()
{
}

void iFileFullPath(const char* relname, char* fullname)
{
    strcpy(fullname, relname);
}

void iFileSetPath(const char* path)
{
}

U32 iFileFind(const char* name, xFile* file)
{
    return iFileOpen(name, 0, file);
}

void iFileGetInfo(xFile* file, U32* starting_sector, U32* size_in_bytes)
{
    iFile* ps = &file->ps;

    if (starting_sector) {
        *starting_sector = 0;
    }

    if (size_in_bytes) {
        *size_in_bytes = iFileGetSize(file);
    }
}