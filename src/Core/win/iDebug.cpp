#if defined(DEBUG) || defined(RELEASE)
#include "iDebug.h"

#include "iSystem.h"
#include "xDebug.h"

#include <rwcore.h>
#include <rtcharse.h>

static RtCharset* mCharset;
static RwRGBA mForegroundColor = { 200, 200, 200, 255 };
static RwRGBA mBackgroundColor = { 32, 32, 32, 0 };

void iDebugInit()
{
    if (!RtCharsetOpen())
    {
        xASSERTFAIL();
    }

    mCharset = RtCharsetCreate(&mForegroundColor, &mBackgroundColor);
    if (!mCharset)
    {
        DBprintf(DBML_WARN, "xDebugInit, Cannot create raster charset.");
    }
}

void iDebugUpdate()
{
}

void iDebugWriteProfile()
{
}

void iDebugVSync()
{
    iVSync();
}

void iDebugStackTrace()
{
}

void iDebugIDEOutput(char* buf)
{
    OutputDebugString(buf);
}
#endif