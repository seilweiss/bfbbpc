#include "xDebug.h"

#include "iCamera.h"
#include "iSystem.h"
#include "xDebugTweak.h"
#include "xFont.h"
#include "xFile.h"
#include "xTimestamp.h"

#include <stdio.h>
#include <stdarg.h>

#include <rwcore.h>

#if defined(DEBUG) || defined(RELEASE)
static bool gInRenderLoop;
static S32 g_STFU;
static xDebugMsgNotifyCallback g_msgnotify;
static FILE* g_userfp;
static FILE* g_errfp;
static S32 g_dbinit;
static iTime g_lasttym;
static char g_lprbuf[2048] = {};
static en_VERBOSE_MSGLEVEL g_msglvl = DBML_DBG;
static S32 g_prepend_msglevel = 1;
static char* g_dbml_strz[] = {
    NULL,       // DBML_NONE
    NULL,       // DBML_RELDISP
    NULL,       // DBML_DISP
    NULL,       // DBML_USER
    "ERROR:",   // DBML_ERR
    NULL,       // DBML_TIME
    "WARNING:", // DBML_WARN
    "INVALID:", // DBML_VALID
    NULL,       // DBML_INFO
    NULL,       // DBML_DBG
    NULL,       // DBML_TEST
    NULL,       // DBML_VDBG
    "(Screen)", // DBML_SPEW
};
xDebugModeCallback dm_callbacks[] = {
    NULL,
    debug_mode_page,
    debug_mode_tweak,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    debug_mode_fontperf,
};
static char* dm_names[] = {
    "",
    "** Page Selector **",
    "* Tweak Parameters *",
    "dm_profiler",
    "dm_collision",
    "dm_components",
    "dm_ent",
    "dm_shadow",
    "dm_snd",
    "dm_graph",
    "dm_par",
    "dm_render",
    "dm_mem",
    "dm_memtracker",
    "dm_vram",
    "dm_pad",
    "dm_lensflare",
    "dm_model",
    "dm_movie",
    "dm_fontperf",
};
static char xStatsFieldName[][32] = {
    "cpu",
    "render",
    "world_render",
    "model_render",
    "ent_update",
    "shadow",
    "shadow_track",
    "shadow_texrender",
    "shadow_collideworld",
    "shadow_renderworld",
    "shadow_receivesetup",
    "shadow_frustumtest",
    "shadow_receive",
    "collision",
    "anim",
    "particle",
    "sound",
    "font",
    "Villain_Update",
    "Villain_Collide",
    "Villain_DBG",
    "Villain_Input",
    "Villain_Anim",
    "Projectile_Update",
    "Projectile_Collide",
    "Projectile_Effects",
    "Projectile_Solver",
    "player_update",
    "Particle_Render",
    "Render_UI",
    "scene_update",
    "scene_render",
    "x1",
    "x2",
    "x3",
};
U32 gFrameCount;
U32 gSceneFrameCount;
static S32 sDebugMode;
S32 last_debug_mode;
U32 xprintf_display = 1;
U32 xprintf_display_modes = 2;

bool xDebugIDontWantYourXprintsOnMyScreen() WIP
{
    return xDebugModeGet() == 8 || xDebugModeGet() == 19;
}


struct DebugModeData
{
    xDebugModeCallback callback;
    const char* name;
    S32 virtualID;
};

DebugModeData sDebugModeData[68];
S32 sDebugModeLimit;
S32 sDebugModeNextVirtualID;

static S32 mapVirtualIDToRealID(S32 virtualID)
{
    for (S32 i = 0; i < sDebugModeLimit; i++)
        if (virtualID == sDebugModeData[i].virtualID)
            return i;
    return -1;
}

#ifdef DEBUG
void xDebugValidateFailed()
{


    
    static S32 break_enabled = 0;
    
    if (break_enabled)
        iDebugBreak();
    
}
#endif













void xSBCreate(xSB* sb, U32 size)
{
    xASSERT(sb != 0);

    
    xASSERT(sb->buf == 0);
    sb->buf = (char*)RwMalloc(size);
    sb->buf[0] = '\0';
    sb->max = size;
    sb->cur = sb->disp = sb->buf;
}

char* xSBAppend(xSB* sb, const char* str)
{
    xASSERT(sb != 0);

    if (str == 0)
        return 0;
    
    char* pRetVal = sb->cur;
    
    U32 size = strlen(str);
    if (sb->cur - sb->buf + size < sb->max - 2) {
        strcpy(sb->cur, str);
        sb->cur += size;
    }




    return pRetVal;
}

void xSBRender(xSB* sb)
{
    if (!RwCameraGetCurrentCamera())
        return;

    xtextbox::log_layout_stats(false);



    static xtextbox tb = xtextbox::create(
        xfont::create(2, NSCREENX(12.0f), NSCREENY(12.0f), NSCREENX(2.0f), g_GRAY80, screen_bounds),
        screen_bounds, 0, NSCREENY(2.0f), 0.0f, 0.0f, 0.0f
    );

    if (xDebugModeGet() == 8) {
        tb.font.width = NSCREENX(7.0f);
        tb.font.height = NSCREENY(7.0f);
    }
    else {





        tb.font.width = NSCREENX(12.0f);
        tb.font.height = NSCREENY(12.0f);
    }


    tb.bounds = screen_bounds;



    
    tb.bounds.contract(NSCREENX(28.0f), NSCREENY(80.0f), NSCREENX(28.0f), NSCREENY(0.0f));
    
    
    S32 lineCount = 1;
    for (S32 i = 0; sb->disp[i] != '\0'; i++) {
        if (sb->disp[i] == '\n') {
            lineCount++;
            if (lineCount >= 32) {
                
                sb->disp[i] = '\0';
                break;
            }
        }
    }

    tb.set_text(sb->disp);
    tb.render(false);
    xtextbox::log_layout_stats(false);
}

void xSBReset(xSB* sb)
{
    xASSERT(sb != 0);

    sb->cur = sb->buf;
}

void xSBDestroy(xSB* sb)
{
    xASSERT(sb != 0);
    RwFree(sb->buf);
    sb->buf = 0;
}







static xSB screenSB;
static xSB relMemInfoSB;

void DBStartup(en_VERBOSE_MSGLEVEL setlvl, char* logfile, char* errfile, xDebugMsgNotifyCallback msgnotify)
{
    if (!g_dbinit++) {
        g_msglvl = setlvl;
        g_msgnotify = msgnotify;

        if (logfile) g_userfp = fopen(logfile, "w");
        if (errfile) g_errfp = fopen(errfile, "w");








        g_lasttym = iTimeGet();



        xSBCreate(&screenSB, 0x3000);

        xSBCreate(&relMemInfoSB, 0x400);
    }







    DBprintf(DBML_VDBG, "xDebug: DBMsg services started ...\n");

}


void DBShutdown()
{
    if (!--g_dbinit) {
        if (g_userfp) fclose(g_userfp);
        g_userfp = NULL;

        if (g_errfp) fclose(g_errfp);
        g_errfp = NULL;

        g_msgnotify = NULL;


        xSBDestroy(&screenSB);

        xSBDestroy(&relMemInfoSB);
    }


    
}








S32 DBChkMsgLvl(en_VERBOSE_MSGLEVEL msglvl)
{





    
    if (msglvl <= DBML_NONE) {
        fprintf(stdout, "xDebug:WARN: Message uses DBML_NONE --> use DBML_USER\n");

        iDebugBreak();
        return 0;
    }


    if (msglvl > g_msglvl) {
        return 0;
    }

    if (g_msglvl == DBML_TIME && msglvl != DBML_TIME) {
        return 0;
    }

    return 1;
}


void DBprintf(en_VERBOSE_MSGLEVEL msglvl, const char* fmt, ...)
{
    va_list marker;
    char* skipptr;
    static iTime elap = 0;
    
    g_lprbuf[0] = '\0';

    
    if (g_STFU || !DBChkMsgLvl(msglvl))
        return;

    if (msglvl == DBML_TIME) {
        
        iTime tym = iTimeGet();


        if (g_lasttym > tym) elap = tym + (0xFFFFFFFF - g_lasttym);
        else elap = tym - g_lasttym;
        g_lasttym = tym;



        sprintf(g_lprbuf, "{Game: %fs xTime: %.5f Elap: %.5f} ",
                iTimeGetGame(), iTimeDiffSec(tym), iTimeDiffSec(elap));


        skipptr = g_lprbuf + strlen(g_lprbuf);
    }
    else if (g_prepend_msglevel) {
        if (g_dbml_strz[msglvl]) strcpy(g_lprbuf, g_dbml_strz[msglvl]);
        else g_lprbuf[0] = '\0';
        skipptr = g_lprbuf + strlen(g_lprbuf);
    }
    else skipptr = g_lprbuf;
    
    
    va_start(marker, fmt);
    vsprintf(skipptr, fmt, marker);
    va_end(marker);































    if (msglvl == DBML_DISP) {

        xSBAppend(&screenSB, g_lprbuf);

        return;
    }

    if (msglvl == DBML_RELDISP) {

#ifdef RELEASE
        xSBAppend(&relMemInfoSB, g_lprbuf);
#endif
        return;

    }




    fprintf(stdout, g_lprbuf);
    fflush(stdout);


    if (g_msgnotify) g_msgnotify(msglvl, g_lprbuf);


    if (g_errfp) {
        fprintf(g_errfp, g_lprbuf);
        fflush(g_errfp);
    }


    if (g_userfp && msglvl == DBML_USER) {
        fprintf(g_userfp, g_lprbuf);
        fflush(g_userfp);
    }








    iDebugIDEOutput(g_lprbuf);











    
}





void xDebugVSync()
{
    iDebugVSync();
}
#endif



void xprintf(const char* fmt, ...)
{
#if defined(DEBUG) || defined(RELEASE)
    va_list marker;
    char buf[512];
    char buf2[512];
    buf[0] = '\0';
    
    va_start(marker, fmt);
    vsprintf(buf, fmt, marker);
    va_end(marker);





    
    char* s = buf;
    char* d = buf2;
    while (*s != '\0') {

        if (*s == '%')
            *d++ = *s;
        *d++ = *s++;
    }
    *d = '\0';

    DBprintf(DBML_DISP, buf2);
#endif
}






#if defined(DEBUG) || defined(RELEASE)
static const char* _assert2func;
static const char* _assert2file;
static U32 _assert2line;
static const char* _assert2expr;

void xDebug_assert2_info(const char* func, const char* file, U32 line, const char* expr)
{
    _assert2func = func;
    _assert2file = file;
    _assert2line = line;
    _assert2expr = expr;
}



#if 0
void xDebugReportAssertion(const char* sfile, S32 line, const char* func, const char* expr, const char* message)
{
    xFile file;
    char workingBuffer[512];
    U32 result;
    const char* platform;
    char* it;


    
    static bool first = true;

    if (first) {

        result = iFileOpen("Assert.log", IFILE_HOST_IO | IFILE_WRITE | IFILE_0x4, &file);
        if (!result) {

            sprintf(workingBuffer, "%s\tPLATFORM\tREGION\tFILE\tLINE\tFUNC\tMESSAGE\r\n", zStealthDebugGetAssertionHeader());
            iFileWrite(&file, (void*)workingBuffer, strlen(workingBuffer));
            first = false;
        }
    }
    else result = iFileOpen("Assert.log", IFILE_HOST_IO | IFILE_APPEND | IFILE_READ, &file);
    
    if (result) {

        iprintf("cannot open assert.log!\n");
        return;
    }
    iFileSeek(&file, 0, IFILE_SEEK_END);




    platform = "GCN";





    sprintf(workingBuffer, "%s\t%s\t%s\t%s\t%d\t%s\t",
            zStealthDebugGetAssertionInfo(), platform, xRegionCode(xSTGetLocalizationEnum()), sfile, line, func);

    iFileWrite(&file, (void*)workingBuffer, strlen(workingBuffer));
    iFileWrite(&file, (void*)expr, strlen(expr));
    iFileWrite(&file, (void*)"\xFF", 1);


    strcpy(workingBuffer, message);
    for (it = workingBuffer; *it != '\0'; it++)
        if (*it == '\n')
            *it = '\xFF';
    iFileWrite(&file, (void*)workingBuffer, strlen(workingBuffer));
    iFileWrite(&file, (void*)"\r\n", 2);
    iFileClose(&file);
}
#endif

void xDebug_assert2(const char* fmt, ...)
{
    va_list argp;
    char buf1[512];
    char buf2[512];
    char pretty_buf[640];


    sprintf(buf1, "%s(%d) : ASSERTION FAILED (%s) in %s\n", _assert2file, _assert2line, _assert2expr, _assert2func);
    
    sprintf(pretty_buf, "{c=ff00aa00}%s{~:c}({c=ff00aa00}%d{~:c}) : ASSERTION FAILED ({c=ffaaaa00}%s{~:c}) in {c=ff00aa00}%s{~:c}\n",
            _assert2file, _assert2line, _assert2expr, _assert2func);



    va_start(argp, fmt);
    vsprintf(buf2, fmt, argp);
    va_end(argp);
#if 0
    xDebugReportAssertion(_assert2file, _assert2line, _assert2func, _assert2expr, buf2);


    gzAsyncRenderDisable = 1;


    xDebugAssertScreen(buf1, buf2, pretty_buf);






    gzAsyncRenderDisable = 0;
#endif
}

U32 xDebugBoing() WIP
{
    return 1;
}

S32 xDebugModeGet() WIP
{
    return 0;
}
#endif

S32 xDebugModeAdd(const char* debugModeName, xDebugModeCallback func) WIP
{
    return -1;
}

void xDebugInit() WIP
{
}

void xDebugUpdate() WIP
{
}

void xDebugExit() WIP
{
}

void xDebugTimestampScreen()
{
#if defined(DEBUG) || defined(RELEASE)
    RwCamera* cam = iCameraCreate(FB_XRES, FB_YRES, 0);
    RwRGBA bg = { 0, 0, 0, 0 };
    
    xtextbox tb = xtextbox::create(xfont::create(0, NSCREENX(21.0f), NSCREENY(22.0f), 0.0f, g_WHITE, screen_bounds), screen_bounds, 0, 0.0f, 0.0f, 0.0f, 0.0f);
    xColorInit(&tb.font.color, 0xC0, 0xC0, 0xC0, 0xFF);
    tb.flags |= 0x2;
    tb.bounds.assign(0.0f, 0.4125f, 1.0f, 0.25f);
    tb.bounds.contract(0.01f);
    
    tb.set_text(timestamp);
    tb.bounds.h = tb.yextent(true);
    tb.bounds.y = 0.5f - 0.5f * tb.bounds.h;
    tb.font.clip = tb.bounds;
    tb.font.clip.expand(0.01f);
    
    for (U32 i = 0; i < VBLANKS_PER_SEC * 3; i++) {

        RwCameraClear(cam, &bg, rwCAMERACLEARIMAGE | rwCAMERACLEARZ);
        RwCameraBeginUpdate(cam);
        
        xColor c = { 0x40, 0x40, 0x40, 0xFF };
        render_fill_rect(tb.font.clip, c);
        tb.render(false);

        RwCameraEndUpdate(cam);

        if (i < 2)
            RwCameraClear(cam, &bg, rwCAMERACLEARIMAGE);

        RwCameraShowRaster(cam, NULL, rwRASTERFLIPWAITVSYNC);

#if 1
        iSystemPollEvents();
        if (iSystemShouldQuit()) break;

        iVSync();
#endif
    }

    iCameraDestroy(cam);
#endif
}

#if defined(DEBUG) || defined(RELEASE)
void xDebugStackTrace() WIP
{
}

void debug_mode_page() WIP
{
}
#endif