#pragma once

#include "xbinio.h"

struct st_HIPLOADDATA;

enum en_READ_ASYNC_STATUS
{
    HIP_RDSTAT_NONE = -1,
    HIP_RDSTAT_INPROG,
    HIP_RDSTAT_SUCCESS,
    HIP_RDSTAT_FAILED,
    HIP_RDSTAT_NOBYPASS,
    HIP_RDSTAT_NOASYNC
};

struct st_HIPLOADFUNCS
{
    st_HIPLOADDATA*(*create)(const char*, char*, S32);
    void(*destroy)(st_HIPLOADDATA*);
    U32(*basesector)(st_HIPLOADDATA*);
    U32(*enter)(st_HIPLOADDATA*);
    void(*exit)(st_HIPLOADDATA*);
    S32(*readBytes)(st_HIPLOADDATA*, char*, S32);
    S32(*readShorts)(st_HIPLOADDATA*, S16*, S32);
    S32(*readLongs)(st_HIPLOADDATA*, S32*, S32);
    S32(*readFloats)(st_HIPLOADDATA*, F32*, S32);
    S32(*readString)(st_HIPLOADDATA*, char*);
    S32(*setBypass)(st_HIPLOADDATA*, S32, S32);
    void(*setSpot)(st_HIPLOADDATA*, S32);
    en_READ_ASYNC_STATUS(*pollRead)(st_HIPLOADDATA*);
};

st_HIPLOADFUNCS* get_HIPLFuncs();