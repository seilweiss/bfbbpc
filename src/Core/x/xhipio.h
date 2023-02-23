#pragma once

#include "xbinio.h"

typedef struct st_HIPLOADDATA HIPLOADDATA;

enum en_READ_ASYNC_STATUS
{
    HIP_RDSTAT_NONE = -1,
    HIP_RDSTAT_INPROG,
    HIP_RDSTAT_SUCCESS,
    HIP_RDSTAT_FAILED,
    HIP_RDSTAT_NOBYPASS,
    HIP_RDSTAT_NOASYNC
};
typedef enum en_READ_ASYNC_STATUS READ_ASYNC_STATUS;

typedef struct st_HIPLOADFUNCS HIPLOADFUNCS;
struct st_HIPLOADFUNCS
{
    HIPLOADDATA*(*create)(const char*, char*, S32);
    void(*destroy)(HIPLOADDATA*);
    U32(*basesector)(HIPLOADDATA*);
    U32(*enter)(HIPLOADDATA*);
    void(*exit)(HIPLOADDATA*);
    S32(*readBytes)(HIPLOADDATA*, char*, S32);
    S32(*readShorts)(HIPLOADDATA*, S16*, S32);
    S32(*readLongs)(HIPLOADDATA*, S32*, S32);
    S32(*readFloats)(HIPLOADDATA*, F32*, S32);
    S32(*readString)(HIPLOADDATA*, char*);
    S32(*setBypass)(HIPLOADDATA*, S32, S32);
    void(*setSpot)(HIPLOADDATA*, S32);
    READ_ASYNC_STATUS(*pollRead)(HIPLOADDATA*);
};

HIPLOADFUNCS* get_HIPLFuncs();