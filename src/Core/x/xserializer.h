#pragma once

#include "types.h"

typedef struct st_SERIAL_CLIENTINFO SERIAL_CLIENTINFO;

struct xSerial
{
public:
    xSerial();
    ~xSerial();
    void operator delete(void* p);
    void setClient(U32 idtag);
    S32 Write(char* data, S32 elesize, S32 n);
    S32 Write_b1(S32 bits);
    S32 Write_b7(U32 bits);
    S32 Write(U8 data);
    S32 Write(S16 data);
    S32 Write(S32 data);
    S32 Write(U32 data);
    S32 Write(F32 data);
    S32 Read(char* buf, S32 elesize, S32 n);
    S32 Read_b1(S32* bits);
    S32 Read_b7(U32* bits);
    S32 Read(U8* buf);
    S32 Read(S16* buf);
    S32 Read(S32* buf);
    S32 Read(U32* buf);
    S32 Read(F32* buf);
    void wrbit(S32 is_on);
    S32 rdbit();
    void prepare(U32 idtag);

private:
    U32 idtag;
    S32 baseoff;
    SERIAL_CLIENTINFO* ctxtdata;
    S32 warned;
    S32 curele;
    S32 bitidx;
    S32 bittally;
};

typedef struct st_SERIAL_PERCID_SIZE SERIAL_PERCID_SIZE;
struct st_SERIAL_PERCID_SIZE
{
    U32 idtag;
    S32 needsize;
};

typedef S32(*xSerialTraverseCallback)(U32, xSerial*);

S32 xSerialStartup(S32 count, SERIAL_PERCID_SIZE* sizeinfo);
S32 xSerialShutdown();
void xSerialTraverse(xSerialTraverseCallback func);
void xSerialWipeMainBuffer();