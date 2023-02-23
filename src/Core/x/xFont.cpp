#include "xFont.h"

#include "iTime.h"

#include "xstransvc.h"
#include "xModel.h"
#include "xModelBucket.h"
#include "xTextAsset.h"
#include "xTimer.h"

#include "zScene.h"

#include <string.h>

namespace {
struct font_asset
{
    static const U32 MAX_SET_SIZE = 127;

    U32 tex_id;
    U16 u;
    U16 v;
    U8 du;
    U8 dv;
    U8 line_size;
    U8 baseline;
    struct
    {
        S16 x;
        S16 y;
    } space;
    U32 flags;
    U8 char_set[MAX_SET_SIZE + 1];
    struct
    {
        U8 offset;
        U8 size;
    } char_pos[MAX_SET_SIZE];
};

struct font_data
{
    font_asset* asset;
    U32 index_max;
    U8 char_index[256];
    F32 iwidth;
    F32 iheight;
    basic_rect<F32> tex_bounds[font_asset::MAX_SET_SIZE];
    basic_rect<F32> bounds[font_asset::MAX_SET_SIZE];
    xVec2 dstfrac[font_asset::MAX_SET_SIZE];
    RwTexture* texture;
    RwRaster* raster;
};

const size_t MAX_FONT_TEXTURES = 3;

char* default_font_texture[MAX_FONT_TEXTURES] = {
    "font_sb",
    "font1_sb",
    "font_numbers"
};

const size_t MAX_FONTS = 4;

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4838 ) // conversion from 'char' to 'U8' requires a narrowing conversion
#endif

font_asset default_font_assets[MAX_FONTS] = {
    {
        1,
        0, 0, 18, 22,
        14, 0,
        0, 0,
        0x1,
        {
            '~', '{', '}', '#',
            'A', 'B', 'C', 'D',
            'E', 'F', 'G', 'H',
            'I', 'J', 'K', 'L',
            'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T',
            'U', 'V', 'W', 'X',
            'Y', 'Z', '0', '1',
            '2', '3', '4', '5',
            '6', '7', '8', '9',
            '?', '!', '.', ',',
            '-', ':', '_', '"',
            '\'','&', '(', ')',
            '<', '>', '/', '%',
            'ü', 'û', 'ù', 'â',
            'ä', 'à', 'ê', 'è',
            'é', 'î', 'ö', 'ô',
            'ç', 'ß', '+'
        },
        {}
    },
    {
        0,
        0, 0, 18, 22,
        14, 0,
        1, 0,
        0,
        {
            'A', 'B', 'C', 'D',
            'E', 'F', 'G', 'H',
            'I', 'J', 'K', 'L',
            'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T',
            'U', 'V', 'W', 'X',
            'Y', 'Z', 'a', 'b',
            'c', 'd', 'e', 'f',
            'g', 'h', 'i', 'j',
            'k', 'l', 'm', 'n',
            'o', 'p', 'q', 'r',
            's', 't', 'u', 'v',
            'w', 'x', 'y', 'z',
            '0', '1', '2', '3',
            '4', '5', '6', '7',
            '8', '9', '?', '!',
            '.', ',', ';', ':',
            '+', '-', '=', '/',
            '&', '(', ')', '%',
            '"', '\'','_', '<',
            '>', '*', '[', ']',
            'Ü', 'Û', 'Ù', 'Â',
            'Ä', 'À', 'Ê', 'È',
            'É', 'Î', 'Ö', 'Ô',
            'Ç', 'ß', 'ü', 'û',
            'ù', 'â', 'ä', 'à',
            'ê', 'è', 'é', 'î',
            'ö', 'ô', 'ç', '~',
            '©', '®', '™', '@',
            '|'
        },
        {}
    },
    {
        1,
        148, 232, 6, 8,
        18, 0,
        0, 0,
        0x9,
        {
            '0', '1', '2', '3',
            '4', '5', '6', '7',
            '8', '9', 'A', 'B',
            'C', 'D', 'E', 'F',
            'G', 'H', 'I', 'J',
            'K', 'L', 'M', 'N',
            'O', 'P', 'Q', 'R',
            'S', 'T', 'U', 'V',
            'W', 'X', 'Y', 'Z',
            ',', '.', '/', '*',
            '+', '-', '=', ':',
            ';', '%', '<', '>',
            '[', ']', '|', '(',
            ')', '_'
        },
        {}
    },
    {
        2,
        0, 0, 32, 32,
        4, 0,
        0, 0,
        0,
        {
            '1', '2', '3', '4',
            '5', '6', '7', '8',
            '9', '0', '/'
        },
        {}
    }
};

#ifdef _WIN32
#pragma warning( pop )
#endif

font_data active_fonts[MAX_FONTS];
size_t active_fonts_size;

const size_t vert_buffer_size = 120;
RwIm2DVertex vert_buffer[vert_buffer_size];
size_t vert_buffer_used;

F32 rcz;
F32 nsz;

basic_rect<S32> find_bounds(const xColor* bits, const basic_rect<S32>& r, S32 pitch)
{
    S32 diff = pitch - r.w;
    const xColor* endp = bits + pitch * r.h;
    const xColor* p = bits;
    S32 pmode = (p->r == p->g && p->g == p->b && p->r >= 240);
    S32 minx = r.x + r.w;
    S32 maxx = r.x - 1;
    S32 miny = r.y + r.h;
    S32 maxy = r.y - 1;
    S32 y = r.y;
    
    while (p != endp) {
        const xColor* endline = p + r.w;
        S32 x = r.x;
        
        while (p != endline) {
            if ((pmode && p->a != 0) || (!pmode && p->r != 0)) {
                minx = xmin(x, minx);
                maxx = xmax(x, maxx);
                miny = xmin(y, miny);
                maxy = xmax(y, maxy);
            }
            
            p++;
            x++;
        }
        
        p += diff;
        y++;
    }
    
    basic_rect<S32> b;
    b.x = minx;
    b.y = miny;
    b.w = maxx + 1 - minx;
    b.h = maxy + 1 - miny;
    
    return b;
}

bool reset_font_spacing(font_asset& a)
{
    RwTexture* tex = (RwTexture*)xSTFindAsset(a.tex_id, NULL);
    if (!tex) return false;
    
    basic_rect<S32> char_bounds;
    char_bounds.w = a.du;
    char_bounds.h = a.dv;
    
    U8 baseline_count[256];
    memset(baseline_count, 0, sizeof(baseline_count));

    a.baseline = 0;
    
    S32 width = RwRasterGetWidth(RwTextureGetRaster(tex));
    S32 height = RwRasterGetHeight(RwTextureGetRaster(tex));
    RwImage* image = RwImageCreate(width, height, 32);
    if (!image) return false;

    RwImageAllocatePixels(image);
    RwImageSetFromRaster(image, RwTextureGetRaster(tex));
    
    const xColor* bits = (const xColor*)RwImageGetPixels(image);
    for (S32 i = 0; a.char_set[i] != '\0'; i++) {
        if (a.flags & 0x4) {
            char_bounds.x = a.u + i / a.line_size * char_bounds.w;
            char_bounds.y = a.v + i % a.line_size * char_bounds.h;
        } else {
            char_bounds.x = a.u + i % a.line_size * char_bounds.w;
            char_bounds.y = a.v + i / a.line_size * char_bounds.h;
        }

        const xColor* p = bits + width * char_bounds.y + char_bounds.x;
        basic_rect<S32> r = find_bounds(p, char_bounds, width);
    
        if (a.flags & 0x8) {
            a.char_pos[i].offset = 0;
            a.char_pos[i].size = char_bounds.w;
        } else {
            a.char_pos[i].offset = r.x - char_bounds.x;
            a.char_pos[i].size = r.w;
        }

        S32 baseline = r.y - char_bounds.y + r.h + 1;

        if (++baseline_count[baseline] > baseline_count[a.baseline]) {
            a.baseline = baseline;
        }
    }

    RwImageDestroy(image);
    return true;
}

basic_rect<F32> get_tex_bounds(const font_data& fd, U8 i) NONMATCH("https://decomp.me/scratch/j5ds6")
{
    const font_asset& a = *fd.asset;
    basic_rect<F32> r;

    if (a.flags & 0x4) {
        r.x = (F32)(i / a.line_size);
        r.y = (F32)(i % a.line_size);
    } else {
        r.x = (F32)(i % a.line_size);
        r.y = (F32)(i / a.line_size);
    }

    r.x = a.char_pos[i].offset + (a.du * r.x + a.u);
    r.w = a.char_pos[i].size - 0.5f;
    r.y = a.dv * r.y + a.v;
    r.h = a.dv - 0.5f;
    r.scale(fd.iwidth, fd.iheight);

    return r;
};

basic_rect<F32> get_bounds(const font_data& fd, U8 i)
{
    const font_asset& a = *fd.asset;
    basic_rect<F32> r;

    r.x = 0.0f;
    r.y = (F32)-a.baseline / a.dv;
    r.w = (F32)(a.char_pos[i].size + a.space.x) / (a.du + a.space.x);
    r.h = 1.0f;

    return r;
}

bool init_font_data(font_data& fd) NONMATCH("https://decomp.me/scratch/8uMna")
{
    const font_asset& a = *fd.asset;
    fd.texture = (RwTexture*)xSTFindAsset(a.tex_id, NULL);
    if (!fd.texture) return false;

    RwTextureSetFilterMode(fd.texture, rwFILTERLINEAR);

    fd.raster = RwTextureGetRaster(fd.texture);
    
    S32 width = RwRasterGetWidth(fd.raster);
    S32 height = RwRasterGetHeight(fd.raster);
    fd.iwidth = 1.0f / width;
    fd.iheight = 1.0f / height;

    memset(fd.char_index, 255, sizeof(fd.char_index));
    fd.index_max = 0;

    while (a.char_set[fd.index_max] != '\0') {
        U8 i = fd.index_max;
        U8 c = a.char_set[i];
        
        fd.char_index[c] = i;

        if ((a.flags & 0x1) && c >= 'A' && c <= 'Z') {
            fd.char_index[c+32] = i;
        } else if ((a.flags & 0x2) && c >= 'a' && c <= 'z') {
            fd.char_index[c-32] = i;
        }

        fd.tex_bounds[i] = get_tex_bounds(fd, i);
        fd.bounds[i] = get_bounds(fd, i);
        fd.dstfrac[i].x = (F32)a.char_pos[i].size / (a.char_pos[i].size + a.space.x);
        fd.dstfrac[i].y = (F32)a.dv / (a.dv + a.space.y);
        fd.index_max++;
    }
    
    size_t tail_index = fd.index_max;

    if (fd.char_index[' '] == 255) {
        fd.char_index[' '] = (U8)tail_index;
        fd.tex_bounds[tail_index].assign(0.0f, 0.0f, 0.0f, 0.0f);
        fd.bounds[tail_index].assign(0.0f, (F32)-a.baseline / a.dv, (a.flags & 0x8) ? 1.0f : 0.5f, 1.0f);
        tail_index++;
    }

    if (fd.char_index['\n'] == 255) {
        fd.char_index['\n'] = (U8)tail_index;
        fd.tex_bounds[tail_index].assign(0.0f, 0.0f, 0.0f, 0.0f);
        fd.bounds[tail_index].assign(0.0f, (F32)-a.baseline / a.dv, 0.0f, 1.0f);
        tail_index++;
    }

    return true;
}

void start_tex_render(U32 font)
{
    RwCamera* cam = RwCameraGetCurrentCamera();
    rcz = 1.0f / RwCameraGetNearClipPlane(cam);
    nsz = RwIm2DGetNearScreenZ();
    
    const font_data& fd = active_fonts[font];
    xfont::set_render_state(fd.raster);
}

void tex_flush()
{
    if (vert_buffer_used) {
        RwIm2DRenderPrimitive(rwPRIMTYPETRILIST, vert_buffer, vert_buffer_used);
        vert_buffer_used = 0;
    }
}

void stop_tex_render()
{
    tex_flush();
    xfont::restore_render_state();
}

inline RwRaster* set_tex_raster(RwRaster* raster)
{
    RwRaster* oldraster;
    RwRenderStateGet(rwRENDERSTATETEXTURERASTER, (void*)&oldraster);

    if (raster == oldraster) return raster;

    tex_flush();
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)raster);

    return oldraster;
}

inline void set_vert(RwIm2DVertex& vert, F32 x, F32 y, F32 u, F32 v, xColor c)
{
    RwIm2DVertexSetScreenX(&vert, x);
    RwIm2DVertexSetScreenY(&vert, y);
    RwIm2DVertexSetScreenZ(&vert, nsz);
    RwIm2DVertexSetRecipCameraZ(&vert, rcz);
    RwIm2DVertexSetU(&vert, u, rcz);
    RwIm2DVertexSetV(&vert, v, rcz);
    RwIm2DVertexSetIntRGBA(&vert, c.r, c.g, c.b, c.a);
}

void tex_render(const basic_rect<F32>& src, const basic_rect<F32>& dst, const basic_rect<F32>& clip, xColor color)
{
    basic_rect<F32> r = dst;
    basic_rect<F32> rt = src;

    clip.clip(r, rt);
    if (r.empty()) return;

    if (vert_buffer_used == vert_buffer_size) {
        RwIm2DRenderPrimitive(rwPRIMTYPETRILIST, vert_buffer, vert_buffer_size);
        vert_buffer_used = 0;
    }

    RwIm2DVertex* vert = vert_buffer + vert_buffer_used;

    r.scale(FB_XRES, FB_YRES);

    set_vert(vert[0], r.x, r.y, rt.x, rt.y, color);
    set_vert(vert[1], r.x, r.y + r.h, rt.x, rt.y + rt.h, color);
    set_vert(vert[2], r.x + r.w, r.y, rt.x + rt.w, rt.y, color);
    vert[3] = vert[2];
    vert[4] = vert[1];
    set_vert(vert[5], r.x + r.w, r.y + r.h, rt.x + rt.w, rt.y + rt.h, color);

    vert_buffer_used += 6;
}

inline void char_render(U8 c, U32 font_index, const basic_rect<F32>& bounds, const basic_rect<F32>& clip, xColor color)
{
    const font_data& fd = active_fonts[font_index];
    U32 i = fd.char_index[c];
    if (i >= fd.index_max) return;
    
    basic_rect<F32> dest = fd.bounds[i];
    dest.scale(bounds.w, bounds.h);
    dest.x += bounds.x;
    dest.y += bounds.y;
    dest.w *= fd.dstfrac[i].x;
    dest.h *= fd.dstfrac[i].y;

    tex_render(fd.tex_bounds[i], dest, clip, color);
}

const size_t MAX_MODELS = 8;

struct model_pool
{
    RwMatrix mat[MAX_MODELS];
    xModelInstance model[MAX_MODELS];
};

struct model_cache_entry
{
    U32 id;
    U32 order;
    xModelInstance* model;
};

model_cache_entry model_cache[MAX_MODELS];
bool model_cache_inited;

void init_model_cache() NONMATCH("https://decomp.me/scratch/zzJJ2")
{
    model_cache_inited = true;

    void* data = xMALLOCALIGN(sizeof(model_pool), 16);
    
    model_pool& pool = *(model_pool*)data;
    memset(&pool, 0, sizeof(model_pool));

    for (size_t i = 0; i < MAX_MODELS; i++) {
        xModelInstance& model = pool.model[i];
        model_cache_entry& e = model_cache[i];

        e.id = 0;
        e.order = 0;
        e.model = &model;

        model.Mat = &pool.mat[i];
        model.Flags = 0x1;
        model.BoneCount = 1;
        model.shadowID = 0xDEADBEEF;
    }
}

xModelInstance* load_model(U32 id) NONMATCH("https://decomp.me/scratch/vpVKQ")
{
    static U32 next_order = 0;
    next_order++;
    
    size_t oldest = 0;
    for (size_t i = 0; i < MAX_MODELS; i++) {
        model_cache_entry& e = model_cache[i];
        
        if (e.id == id) {
            e.order = next_order;
            return e.model;
        }

        if (e.order < model_cache[oldest].order) {
            oldest = i;
        }
    }

    RpAtomic* mf = (RpAtomic*)xSTFindAsset(id, NULL);
    if (!mf) return NULL;

    model_cache_entry& e = model_cache[oldest];
    e.id = id;
    e.order = next_order;

    xModelInstance& model = *e.model;
    xMat4x3Identity((xMat4x3*)model.Mat);
    model.Data = mf;
    model.Bucket = xModelBucket_GetBuckets(model.Data);
    model.PipeFlags = model.Bucket ? model.Bucket[0]->PipeFlags : xModelGetPipeFlags(model.Data);

    return &model;
}
}

void xfont::init()
{
    active_fonts_size = 0;
    
    for (size_t i = 0; i < MAX_FONTS; i++) {
        font_asset& a = default_font_assets[i];
        if (a.tex_id < MAX_FONT_TEXTURES) {
            a.tex_id = xStrHash(default_font_texture[a.tex_id]);
        }

        RwTexture* tex = (RwTexture*)xSTFindAsset(a.tex_id, NULL);

        if (reset_font_spacing(a)) {
            font_data& fd = active_fonts[active_fonts_size];
            fd.asset = &a;
            if (init_font_data(fd)) {
                active_fonts_size++;
            }
        }
    }

    init_model_cache();
}

namespace {
struct
{
    RwBool fogenable;
    RwBool vertexalphaenable;
    RwBool zwriteenable;
    RwBool ztestenable;
    RwUInt32 srcblend;
    RwUInt32 destblend;
    RwUInt32 shademode;
    RwRaster* textureraster;
    RwTextureFilterMode filter;
} oldrs;
}

void xfont::set_render_state(RwRaster* raster)
{
    RwRenderStateGet(rwRENDERSTATEFOGENABLE, (void*)&oldrs.fogenable);
    RwRenderStateGet(rwRENDERSTATESRCBLEND, (void*)&oldrs.srcblend);
    RwRenderStateGet(rwRENDERSTATEDESTBLEND, (void*)&oldrs.destblend);
    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)&oldrs.vertexalphaenable);
    RwRenderStateGet(rwRENDERSTATETEXTURERASTER, (void*)&oldrs.textureraster);
    RwRenderStateGet(rwRENDERSTATESHADEMODE, (void*)&oldrs.shademode);
    RwRenderStateGet(rwRENDERSTATEZWRITEENABLE, (void*)&oldrs.zwriteenable);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)&oldrs.ztestenable); // BUG: should be Get, not Set
    RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, (void*)&oldrs.filter);

    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)raster);
    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEFLAT);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
}

void xfont::restore_render_state()
{
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)oldrs.fogenable);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)oldrs.srcblend);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)oldrs.destblend);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)oldrs.vertexalphaenable);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)oldrs.textureraster);
    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)oldrs.shademode);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)oldrs.zwriteenable);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)oldrs.ztestenable);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)oldrs.filter);
}

basic_rect<F32> xfont::bounds(char c) const
{
    const font_data& fd = active_fonts[id];
    U32 i = fd.char_index[c];
    if (i == 255) return basic_rect<F32>::m_Null;

    basic_rect<F32> r = fd.bounds[i];
    r.scale(width, height);

    return r;
}

basic_rect<F32> xfont::bounds(const char* text) const
{
    size_t size;
    return bounds(text, 0x40000000, HUGE, size);
}

basic_rect<F32> xfont::bounds(const char* text, size_t text_size, F32 max_width, size_t& size) const NONMATCH("https://decomp.me/scratch/WbEvQ")
{
    const font_data& fd = active_fonts[id];
    basic_rect<F32> r;
    r.x = 0.0f;
    r.w = 0.0f;
    r.y = fd.bounds[0].y * height;
    r.h = fd.bounds[0].h * height;

    if (!text || text_size == 0) {
        size = 0;
        return r;
    }

    const char* s = text;
    for (size_t i = 0; i < text_size && *s != '\0'; i++, s++) {
        U32 c = *s;
        U32 charIndex = fd.char_index[c];
        if (charIndex != 255) {
            F32 dx = fd.bounds[charIndex].w * width;
            if (r.w + dx > max_width) break;
            r.w += dx + space;
        }
    }

    if (r.w > 0.0f) {
        r.w -= space;
    }

    size = s - text;

    return r;
}

void xfont::start_render() const
{
    start_tex_render(id);
}

void xfont::stop_render() const
{
    stop_tex_render();
}

void xfont::irender(const char* text, F32 x, F32 y) const
{
    irender(text, 0x40000000, x, y);
}

void xfont::irender(const char* text, size_t text_size, F32 x, F32 y) const NONMATCH("https://decomp.me/scratch/RLMbg")
{
    if (!text) return;

    const font_data& fd = active_fonts[id];
    set_tex_raster(fd.raster);

    basic_rect<F32> bounds = {};
    bounds.x = x, bounds.y = y, bounds.w = width, bounds.h = height;

    const char* s = text;
    for (size_t i = 0; i < text_size && *s != '\0'; i++, s++) {
        U32 c = *s;
        char_render(c, id, bounds, clip, color);

        U32 charIndex = fd.char_index[c];
        if (charIndex != 255) {
            bounds.x += fd.bounds[charIndex].w * width + space;
        }
    }
}

namespace {
substr text_delims = { " \t\n{}=*+:;,", 11 };

size_t parse_split_tag(xtextbox::split_tag& ti) NONMATCH("https://decomp.me/scratch/GTTwc")
{
    ti.value.size = 0;
    ti.action.size = 0;
    ti.name.size = 0;
    
    substr s;
    s.text = ti.tag.text;
    s.size = ti.tag.size;
    s.text++;
    s.size--;

    ti.name.text = skip_ws(s);
    
    s.text = find_char(s, text_delims);
    if (!s.text) return 0;

    ti.name.size = s.text - ti.name.text;
    s.size -= ti.name.size;

    ti.action.text = skip_ws(s);
    if (s.size == 0) return 0;
    
    char c = s.text[0];
    if (c == '\0' || c == '{') return 0;

    s.text++;
    s.size--;

    if (c == '}') return ti.tag.size - s.size;

    ti.action.size = 1;
    
    ti.value.text = skip_ws(s);
    s.text = find_char(s, '}');
    s.size -= s.text - ti.value.text;
    if (!s.text) return 0;

    ti.value.size = s.text - ti.value.text;
    rskip_ws(ti.value);
    
    s.text++;
    s.size--;

    return ti.tag.size - s.size;
}

const char* parse_next_tag_jot(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const char* text, size_t text_size)
{
    xtextbox::split_tag ti = {};
    ti.tag.text = text;
    ti.tag.size = text_size;

    size_t size = parse_split_tag(ti);
    if (size == 0) return NULL;

    a.s.text = text;
    a.s.size = size;
    a.flag.invisible = a.flag.ethereal = true;

    if (icompare(ti.name, substr::create("~", 1)) == 0 ||
        icompare(ti.name, substr::create("reset", 5)) == 0) {
        a.tag = xtextbox::find_format_tag(ti.value);
        if (a.tag && a.tag->reset_tag) {
            a.tag->reset_tag(a, tb, ctb, ti);
        }
    } else {
        a.tag = xtextbox::find_format_tag(ti.name);
        if (a.tag && a.tag->parse_tag) {
            a.tag->parse_tag(a, tb, ctb, ti);
        }
    }

    return text + size;
}

const char* parse_next_text_jot(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const char* text, size_t text_size) NONMATCH("https://decomp.me/scratch/j7JLp")
{
    a.s.text = text;
    a.s.size = 1;
    a.flag.merge = true;
    
    char c = text[0];
    if (c == '\n') {
        a.flag.line_break = true;
    } else if (c == '\t') {
        a.flag.tab = true;
    } else if (c == '-') {
        a.flag.word_end = true;
    }

    if (is_ws(c)) {
        a.flag.invisible = a.flag.word_break = true;
    }

    a.bounds = tb.font.bounds(c);
    a.cb = &xtextbox::text_cb;
    a.context = NULL;
    a.context_size = 0;

    return a.s.text + a.s.size;
}

const char* parse_next_jot(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const char* text, size_t text_size)
{
    char c = text[0];
    const char* next;
    
    if (c != '{' || !(next = parse_next_tag_jot(a, tb, ctb, text, text_size))) {
        next = parse_next_text_jot(a, tb, ctb, text, text_size);
    }

    a.flag.merge = a.flag.merge && !(tb.flags & 0x80);
    return next;
}

struct tex_args
{
    RwRaster* raster;
    F32 rot;
    basic_rect<F32> src;
    basic_rect<F32> dst;
    xVec2 off;
    enum
    {
        SCALE_FONT,
        SCALE_SCREEN,
        SCALE_SIZE,
        SCALE_FONT_WIDTH,
        SCALE_FONT_HEIGHT,
        SCALE_SCREEN_WIDTH,
        SCALE_SCREEN_HEIGHT
    } scale;
};

void reset_tex_args(tex_args& ta) NONMATCH("https://decomp.me/scratch/TqRyD")
{
    ta.raster = NULL;
    ta.rot = 0.0f;
    ta.src = ta.dst = basic_rect<F32>::m_Unit;
    ta.off.x = ta.off.y = 1.0f;
    ta.scale = tex_args::SCALE_FONT;
}

void load_tex_args(tex_args& ta, const substr& s)
{
    xtextbox::tag_entry_list el = xtextbox::read_tag(s);
    if (el.size == 0) return;

    const xtextbox::tag_entry* e = &el.entries[0];
    if (e->op == ':' || (e->args_size == 1 && e->args[0].size != 0)) {
        const substr& name = e->args[0];
        U32 id;
        if (name.size == 10 && imemcmp(name.text, "0x", 2) == 0) {
            id = atox(substr::create(name.text+2, 8));
        } else {
            id = xStrHash(name.text, name.size);
        }

        RwTexture* texture = (RwTexture*)xSTFindAsset(id, NULL);
        if (texture &&
            texture->raster &&
            texture->raster->width > 0 &&
            texture->raster->height > 0 &&
            texture->raster->width <= 4096 &&
            texture->raster->height <= 4096) {
            RwTextureSetFilterMode(texture, rwFILTERLINEAR);
            ta.raster = RwTextureGetRaster(texture);
        }
    }

    el.entries++;
    el.size--;

    e = xtextbox::find_entry(el, substr::create("rot", 3));
    if (e && e->op == '=' && e->args_size == 1) {
        xtextbox::read_list(*e, &ta.rot, 1);
    }

    e = xtextbox::find_entry(el, substr::create("src", 3));
    if (e && e->op == '=' && e->args_size == 4) {
        xtextbox::read_list(*e, &ta.src.x, 4);
    }

    e = xtextbox::find_entry(el, substr::create("dst", 3));
    if (e && e->op == '=' && e->args_size == 4) {
        xtextbox::read_list(*e, &ta.dst.x, 4);
    }

    e = xtextbox::find_entry(el, substr::create("off", 3));
    if (e && e->op == '=' && e->args_size == 2) {
        xtextbox::read_list(*e, &ta.off.x, 2);
    }

    e = xtextbox::find_entry(el, substr::create("scale", 5));
    if (e && e->op == '=' && e->args_size == 1) {
        if (icompare(e->args[0], substr::create("font", 4)) == 0) {
            ta.scale = tex_args::SCALE_FONT;
        } else if (icompare(e->args[0], substr::create("screen", 6)) == 0) {
            ta.scale = tex_args::SCALE_SCREEN;
        } else if (icompare(e->args[0], substr::create("size", 4)) == 0) {
            ta.scale = tex_args::SCALE_SIZE;
        } else if (icompare(e->args[0], substr::create("font_width", 10)) == 0) {
            ta.scale = tex_args::SCALE_FONT_WIDTH;
        } else if (icompare(e->args[0], substr::create("font_height", 11)) == 0) {
            ta.scale = tex_args::SCALE_FONT_HEIGHT;
        } else if (icompare(e->args[0], substr::create("screen_width", 12)) == 0) {
            ta.scale = tex_args::SCALE_SCREEN_WIDTH;
        } else if (icompare(e->args[0], substr::create("screen_height", 13)) == 0) {
            ta.scale = tex_args::SCALE_SCREEN_HEIGHT;
        } else {
            ta.scale = tex_args::SCALE_FONT;
        }
    }
}

struct model_args
{
    xModelInstance* model;
    xVec3 rot;
    basic_rect<F32> dst;
    xVec2 off;
    enum
    {
        SCALE_FONT,
        SCALE_SCREEN,
        SCALE_SIZE
    } scale;
};

void reset_model_args(model_args& ma)
{
    ma.model = NULL;
    ma.rot = xVec3::m_Null;
    ma.dst = basic_rect<F32>::m_Unit;
    ma.off.x = ma.off.y = 1.0f;
    ma.scale = model_args::SCALE_FONT;
}

void load_model_args(model_args& ma, const substr& s)
{
    xtextbox::tag_entry_list el = xtextbox::read_tag(s);
    if (el.size == 0) return;

    const xtextbox::tag_entry* e = &el.entries[0];
    if (e->op == ':' || (e->args_size == 1 && e->args[0].size != 0)) {
        const substr& name = e->args[0];
        U32 id = xStrHash(name.text, name.size);
        ma.model = load_model(id);
    }

    el.entries++;
    el.size--;

    e = xtextbox::find_entry(el, substr::create("rot", 3));
    if (e && e->op == '=' && e->args_size == 3) {
        xtextbox::read_list(*e, &ma.rot.x, 3);
    }

    e = xtextbox::find_entry(el, substr::create("dst", 3));
    if (e && e->op == '=' && e->args_size == 4) {
        xtextbox::read_list(*e, &ma.dst.x, 4);
    }

    e = xtextbox::find_entry(el, substr::create("off", 3));
    if (e && e->op == '=' && e->args_size == 2) {
        xtextbox::read_list(*e, &ma.off.x, 2);
    }

    e = xtextbox::find_entry(el, substr::create("scale", 5));
    if (e && e->op == '=' && e->args_size == 1) {
        if (icompare(e->args[0], substr::create("screen", 6)) == 0) {
            ma.scale = model_args::SCALE_SCREEN;
        } else {
            ma.scale = model_args::SCALE_FONT;
        } // model_args::SCALE_SIZE unused?
    }
}

tex_args def_tex_args;
model_args def_model_args;

void start_layout(const xtextbox&)
{
    reset_tex_args(def_tex_args);
    reset_model_args(def_model_args);
}

void stop_layout(const xtextbox&)
{
}

void start_render(const xtextbox& tb)
{
    tb.font.start_render();
}

void stop_render(const xtextbox& tb)
{
    tb.font.stop_render();
}

struct tl_cache_entry
{
    U32 used;
    iTime last_used;
    xtextbox::layout tl;
};

const size_t MAX_CACHE = 1;
tl_cache_entry tl_cache[3];
}

xtextbox::callback xtextbox::text_cb = {
    text_render,
    NULL,
    NULL
};

void xtextbox::text_render(const jot& j, const xtextbox& tb, F32 x, F32 y)
{
    tb.font.irender(j.s.text, j.s.size, x, y);
}

void xtextbox::set_text(const char* text)
{
    set_text(text, 0x40000000);
}

void xtextbox::set_text(const char** texts, size_t size)
{
    set_text(texts, NULL, size);
}

void xtextbox::set_text(const char* text, size_t text_size)
{
    if (!text || text_size == 0) {
        texts_size = 0;
        text_hash = 0;
        return;
    }

    this->text.text = text;
    this->text.size = text_size;
    set_text(&this->text.text, &this->text.size, 1);
}

void xtextbox::set_text(const char** texts, const size_t* text_sizes, size_t size)
{
    texts_size = size;
    text_hash = 0;

    if (size == 0) return;

    this->texts = texts;
    this->text_sizes = text_sizes;

    if (!text_sizes) {
        for (size_t i = 0; i < size; i++) {
            text_hash = text_hash * 131 + xStrHash(texts[i]);
        }
    } else {
        for (size_t i = 0; i < size; i++) {
            text_hash = text_hash * 131 + xStrHash(texts[i], text_sizes[i]);
        }
    }
}

namespace tweaker {
namespace {
void log_cache(bool hit) {}
}
}

xtextbox::layout& xtextbox::temp_layout(bool cache) const NONMATCH("https://decomp.me/scratch/Ok3UW")
{
    iTime r29 = iTimeFromSec(1.0f);
    iTime cur_time = iTimeGet();
    bool refresh = false;
    size_t index = 0;

    if (cache) {
        if (tl_cache[0].tl.changed(*this)) {
            index = 1;
        }
    } else {
        index = 1;
    }

    tweaker::log_cache(index > 1);

    if (index >= 1) {
        refresh = true;
        index = 0;

        for (size_t i = 0; i < MAX_CACHE; i++) {
            if (r29 < cur_time - tl_cache[i].last_used) {
                index = i;
            } else {
                S32 used = tl_cache[i].used;
                if (tl_cache[i].tl.dynamics_size != 0) used -= 10;
                if (tl_cache[i].tl.jots_size() > 50) used += 10;
                if (used < 1000000000) index = i;
            }
        }
    }

    tl_cache_entry& e = tl_cache[index];
    if (refresh) {
        e.used = 0;
        e.tl.refresh(*this, true);
    } else {
        e.tl.tb = *this;
    }

    if (cache) {
        e.used++;
        e.last_used = cur_time;
    }

    return e.tl;
}

void xtextbox::render(layout& l, S32 begin_jot, S32 end_jot) const
{
    l.render(*this, begin_jot, end_jot);
}

F32 xtextbox::yextent(F32 max, S32& size, const layout& l, S32 begin_jot, S32 end_jot) const
{
    return l.yextent(max, size, begin_jot, end_jot);
}

xtextbox::tag_entry_list xtextbox::read_tag(const substr& s)
{
    size_t args_used = 0;
    size_t entries_used = 0;
    substr it = s;
    const char* d = find_char(it, '{');
    if (d) {
        it.size -= d + 1 - it.text;
        it.text = d + 1;
    }

    substr delims = { "=:+*;}", 6 };
    substr sub_delims = { ",;}", 3 };

    static substr arg_buffer[32];
    static tag_entry entry_buffer[16];

    while (it.size) {
        tag_entry& entry = entry_buffer[entries_used];
        entry.args_size = 0;
        entry.op = 0;
        entry.name.text = it.text;

        const char* d = find_char(it, delims);
        entry.name.size = !d ? it.size : d - it.text;

        trim_ws(entry.name);
        if (entry.name.size) entries_used++;

        if (!d || *d == '}') break;

        it.size -= d + 1 - it.text;
        it.text = d + 1;

        if (*d != ';') {
            entry.op = *d;
            entry.args = arg_buffer + args_used;

            while (it.size) {
                substr& arg = arg_buffer[args_used];
                arg.text = it.text;

                const char* d = find_char(it, sub_delims);
                if (!d) {
                    arg.size = it.size;
                    it.size = 0;
                } else {
                    arg.size = d - it.text;
                    it.size -= arg.size + 1;
                    it.text += arg.size + 1;
                }

                trim_ws(arg);
                if (arg.size) {
                    args_used++;
                    entry.args_size++;
                }

                if (!d || *d == '}') {
                    it.size = 0;
                    break;
                }

                if (*d == ';') break;
            }
        }
    }
    
    xtextbox::tag_entry_list ret = { entry_buffer, entries_used };
    return ret;
}

const xtextbox::tag_entry* xtextbox::find_entry(const tag_entry_list& el, const substr& name) NONMATCH("https://decomp.me/scratch/VCWdJ")
{
    for (size_t i = 0; i < el.size; i++) {
        const tag_entry& e = el.entries[i];
        if (icompare(name, e.name) == 0) {
            return &e;
        }
    }
    return NULL;
}

size_t xtextbox::read_list(const tag_entry& e, F32* v, size_t vsize) NONMATCH("https://decomp.me/scratch/skY56")
{
    size_t total = xmin(vsize, e.args_size);
    for (size_t i = 0; i < total; i++) {
        v[i] = xatof(e.args[i].text);
    }
    return total;
}

size_t xtextbox::read_list(const tag_entry& e, S32* v, size_t vsize) NONMATCH("https://decomp.me/scratch/SC5cl")
{
    size_t total = xmin(vsize, e.args_size);
    for (size_t i = 0; i < total; i++) {
        v[i] = atoi(e.args[i].text);
    }
    return total;
}

void xtextbox::clear_layout_cache()
{
    for (size_t index = 0; index < MAX_CACHE; index++) {
        tl_cache[index].tl.clear();
    }
}

void xtextbox::layout::refresh(const xtextbox& tb, bool force)
{
    if (force || changed(tb)) {
        this->tb = tb;
        calc(tb, 0);
    }
}

void xtextbox::layout::refresh_end(const xtextbox& tb)
{
    if (this->tb.texts_size > tb.texts_size) {
        size_t start_text = this->tb.texts_size;
        this->tb = tb;
        calc(tb, start_text);
    }
}

void xtextbox::layout::clear()
{
    _jots_size = _lines_size = context_buffer_size = dynamics_size = 0;
    tb = xtextbox::create();
}

void xtextbox::layout::trim_line(jot_line& line)
{
    for (S32 i = line.last - 1; i >= (S32)line.first; i--) {
        jot& a = _jots[i];
        if (!a.flag.ethereal) {
            if (a.flag.invisible) {
                erase_jots(i, i+1);
                line.last--;
            }
            break;
        }
    }

    for (size_t i = line.first; i < line.last; i++) {
        jot& a = _jots[i];
        if (!a.flag.ethereal) {
            if (a.flag.invisible) {
                erase_jots(i, i+1);
            }
            break;
        }
    }
}

void xtextbox::layout::erase_jots(size_t first, size_t last)
{
    if (last >= _jots_size) {
        _jots_size = first;
        return;
    }

    size_t offset = last - first;
    _jots_size -= offset;

    for (size_t i = first; i < _jots_size; i++) {
        jot& a = _jots[i];
        a = _jots[i+offset];
    }
}

void xtextbox::layout::merge_line(jot_line& line)
{
    size_t d = line.first;
    for (size_t i = line.first + 1; i != line.last; i++) {
        jot& a1 = _jots[d];
        jot& a2 = _jots[i];
        if (!a1.flag.ethereal &&
            !a2.flag.ethereal &&
            a1.flag.merge &&
            a2.flag.merge &&
            a1.cb == a2.cb) {
            a1.s.size = a2.s.text - a1.s.text + a2.s.size;
            a1.bounds |= a2.bounds;
            a1.intersect_flags(a2);
        } else {
            d++;
            if (d != i) {
                _jots[d] = a2;
            }
        }
    }
    d++;
    erase_jots(d, line.last);
    line.last = d;
}

void xtextbox::layout::bound_line(jot_line& line)
{
    line.bounds.w = line.bounds.h = line.baseline = 0.0f;

    for (size_t i = line.first; i != line.last; i++) {
        jot& a = _jots[i];
        if (!a.flag.ethereal && -a.bounds.y > line.baseline) {
            line.baseline = -a.bounds.y;
        }
    }

    for (size_t i = line.first; i != line.last; i++) {
        jot& a = _jots[i];
        if (!a.flag.ethereal) {
            a.bounds.x = line.bounds.w;
            line.bounds.w += a.bounds.w;
            F32 total_height = line.baseline + a.bounds.y + a.bounds.h;
            if (total_height > line.bounds.h) {
                line.bounds.h = total_height;
            }
        }
    }

    line.page_break = (line.last > line.first && _jots[line.last-1].flag.page_break);
}

bool xtextbox::layout::fit_line()
{
    jot_line& line = _lines[_lines_size];
    
    if (line.bounds.w > tb.bounds.w) {
        switch (tb.flags & 0x30) {
        case 0x10:
            if (line.last > line.first + 1) {
                line.last--;
            }
            break;
        case 0x20:
            return false;
        default:
        {
            for (S32 i = line.last - 1; i > (S32)line.first; i--) {
                jot& a = _jots[i];
                if (a.flag.word_break) {
                    line.last = i + 1;
                    break;
                } else if (_jots[i-1].flag.word_end) {
                    line.last = i;
                    break;
                }
            }

            if (line.last <= line.first) {
                line.last = line.first + 1;
            }

            trim_line(line);
            break;
        }
        }
    }

    merge_line(line);
    bound_line(line);
    
    return true;
}

void xtextbox::layout::next_line() NONMATCH("https://decomp.me/scratch/GptGF")
{
    jot_line& line = _lines[_lines_size++];
    jot_line& nline = _lines[_lines_size];
    
    nline.first = line.last;
    nline.last = _jots_size;
    nline.bounds.x = 0.0f;
    nline.bounds.y = line.bounds.y + line.bounds.h;

    bound_line(nline);
}

void xtextbox::layout::calc(const xtextbox& ctb, size_t start_text) NONMATCH("https://decomp.me/scratch/hzUXT")
{
    if (start_text == 0) {
        _jots_size = _lines_size = context_buffer_size = dynamics_size = 0;
    }

    if (tb.texts_size == 0) return;

    start_layout(ctb);

    jot_line& first_line = _lines[_lines_size];
    first_line.first = 0;
    first_line.bounds.x = first_line.bounds.w = 0.0f;
    first_line.bounds.y = 0.0f;
    first_line.baseline = 0.0f;

    struct
    {
        const char* s;
        const char* end;
    } text_stack[16];
    
    size_t text_stack_size = 0;
    size_t text_index = start_text;
    size_t text_size = !tb.text_sizes ? 0x40000000 : tb.text_sizes[text_index];
    const char* s = tb.texts[text_index];
    const char* end = s + text_size;
    const char* next;
    
    text_index++;

    while (true) {
        jot& a = _jots[_jots_size];
        jot_line& line = _lines[_lines_size];
        
        a.context = context_buffer + context_buffer_size;
        a.context_size = 0;
        a.reset_flags();
        a.cb = NULL;
        a.tag = NULL;
        
        if (s == end || *s == '\0') {
            if (text_stack_size != 0) {
                text_stack_size--;
                s = text_stack[text_stack_size].s;
                end = text_stack[text_stack_size].end;
            } else if (text_index < tb.texts_size) {
                text_size = !tb.text_sizes ? 0x40000000 : tb.text_sizes[text_index];
                s = tb.texts[text_index];
                end = s + text_size;
                text_index++;
            } else {
                break;
            }

            a.flag.invisible = a.flag.ethereal = true;
            a.s = substr::create(NULL, 0);
            _jots_size++;
        } else {
            next = parse_next_jot(a, tb, ctb, s, end - s);
            if (a.context == context_buffer + context_buffer_size) {
                context_buffer_size += (a.context_size + 3) & ~3;
            }
            _jots_size++;
            if (a.cb && a.cb->layout_update) {
                a.cb->layout_update(a, tb, ctb);
            }
            if (a.flag.stop) break;
            if (!a.flag.ethereal) {
                a.bounds.x += line.bounds.w;
                line.bounds.w += a.bounds.w;
                if (line.bounds.w >= tb.bounds.w) {
                    line.last = _jots_size;
                    if (!fit_line()) break;
                    next_line();
                }
            }
            if (a.flag.line_break || a.flag.page_break) {
                line.last = _jots_size;
                if (!fit_line()) break;
                next_line();
            }
            s = next;
            if (a.flag.insert) {
                text_stack[text_stack_size].s = s;
                text_stack[text_stack_size].end = end;
                s = (const char*)a.context;
                end = s + a.context_size;
                text_stack_size++;
            }
        }
    }

    jot_line& last_line = _lines[_lines_size];
    if (last_line.first < _jots_size) {
        last_line.last = _jots_size;
        if (fit_line()) {
            _lines_size++;
        }
    }

    for (size_t i = 0; i < _jots_size; i++) {
        if (_jots[i].flag.dynamic) {
            dynamics[dynamics_size++] = (U16)i;
        }
    }

    stop_layout(ctb);
}

void xtextbox::layout::render(const xtextbox& ctb, S32 begin_jot, S32 end_jot)
{
    if (begin_jot < 0) begin_jot = 0;
    if (end_jot < begin_jot) end_jot = _jots_size;
    if (begin_jot >= end_jot) return;

    tb = ctb;
    start_render(ctb);

    S32 begin_line = 0;
    while (true) {
        if (begin_line >= (S32)_lines_size) {
            stop_render(ctb);
            break;
        }
        
        if ((S32)_lines[begin_line].last <= begin_jot) {
            begin_line++;
        } else {
            for (S32 i = 0; i < begin_jot; i++) {
                jot& j = _jots[i];
                if (j.cb && j.cb->render_update) {
                    j.cb->render_update(j, tb, ctb);
                }
            }

            F32 top = _lines[begin_line].bounds.y;
            size_t li = begin_line - 1;
            S32 line_last = -1;
            F32 x, y;

            for (S32 i = begin_jot; i < end_jot; i++) {
                if (i >= line_last) {
                    li++;
                    
                    const jot_line& line = _lines[li];
                    line_last = line.last;
                    x = tb.bounds.x + line.bounds.x;
                    y = tb.bounds.y + line.bounds.y + line.baseline - top;

                    U32 xj = tb.flags & 0x3;
                    if (xj == 2) {
                        x += (tb.bounds.w - line.bounds.w) * 0.5f;
                    } else if (xj == 1) {
                        x += tb.bounds.w - line.bounds.w;
                    }

                    if (line.page_break && end_jot > line_last) {
                        end_jot = line_last;
                    }
                }

                jot& j = _jots[i];
                if (j.cb) {
                    if (j.cb->render_update) {
                        j.cb->render_update(j, tb, ctb);
                    }
                    if (!j.flag.ethereal && !j.flag.invisible && j.cb->render) {
                        j.cb->render(j, tb, x + j.bounds.x, y);
                    }
                }
            }

            stop_render(ctb);
            break;
        }
    }
}

F32 xtextbox::layout::yextent(F32 max, S32& size, S32 begin_jot, S32 end_jot) const NONMATCH("https://decomp.me/scratch/AQKtE")
{
    size = 0;
    if (begin_jot < 0) begin_jot = 0;
    if (end_jot < begin_jot) end_jot = _jots_size;
    if (begin_jot >= end_jot) return 0.0f;

    S32 begin_line = 0;
    while (true) {
        if (begin_line >= (S32)_lines_size) return 0.0f;
        
        if ((S32)_lines[begin_line].last <= begin_jot) {
            begin_line++;
        } else {
            F32 top = _lines[begin_line].bounds.y;
            max += top;

            S32 i = begin_line;
            while (true) {
                if (i == (S32)_lines_size) break;
                const jot_line& line = _lines[i];
                if (line.bounds.y + line.bounds.h > max) {
                    i--;
                    break;
                }
                if ((S32)line.last >= end_jot) break;
                if (line.page_break) break;
                i++;
            }
            if (i < begin_line) return 0.0f;

            const jot_line& line = _lines[i];
            
            size = ((S32)line.last >= end_jot ? end_jot : line.last) - begin_jot;
            return line.bounds.y + line.bounds.h - top;
        }
    }
}

bool xtextbox::layout::changed(const xtextbox& ctb)
{
    U32 flags1 = tb.flags & 0x70;
    U32 flags2 = ctb.flags & 0x70;
    
    if (tb.text_hash != ctb.text_hash ||
        tb.font.id != ctb.font.id ||
        tb.font.width != ctb.font.width ||
        tb.font.height != ctb.font.height ||
        tb.font.space != ctb.font.space ||
        tb.bounds.w != ctb.bounds.w ||
        flags1 != flags2 ||
        tb.line_space != ctb.line_space) {
        return true;
    }

    S32 i = dynamics_size;
    while (i > 0) {
        i--;
        
        jot& j = _jots[dynamics[i]];
        U32 oldval = xStrHash((const char*)j.context, j.context_size);
        parse_next_jot(j, tb, ctb, j.s.text, j.s.size);
        U32 val = xStrHash((const char*)j.context, j.context_size);

        if (val != oldval) return true;
    }

    return false;
}

namespace {
void update_tag_alpha(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb) NONMATCH("https://decomp.me/scratch/fB60s")
{
    tb.font.color.a = (U8)((F32&)j.context * 255.0f + 0.5f);
}

void update_tag_reset_alpha(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.font.color.a = ctb.font.color.a;
}

void parse_tag_alpha(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    F32& v = (F32&)a.context;

    if (ti.value.size == 0 || ti.action.size == 0) return;

    v = xatof(ti.value.text);

    switch (ti.action.text[0]) {
    case '=':
        break;
    case '+':
        v += (1 / 255.0f) * tb.font.color.a;
        break;
    case '*':
        v *= (1 / 255.0f) * tb.font.color.a;
        break;
    default:
        return;
    }

    if (v < 0.0f) v = 0.0f;
    else if (v > 1.0f) v = 1.0f;

    static xtextbox::callback cb = { NULL, update_tag_alpha, update_tag_alpha };
    a.cb = &cb;
}

void reset_tag_alpha(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    static xtextbox::callback cb = { NULL, update_tag_reset_alpha, update_tag_reset_alpha };
    a.cb = &cb;
}

void update_tag_red(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.font.color.r = (U8)((F32&)j.context * 255.0f + 0.5f);
}

void update_tag_reset_red(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.font.color.r = ctb.font.color.r;
}

void parse_tag_red(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    F32& v = (F32&)a.context;

    if (ti.value.size == 0 || ti.action.size == 0) return;

    v = xatof(ti.value.text);

    switch (ti.action.text[0]) {
    case '=':
        break;
    case '+':
        v += (1 / 255.0f) * tb.font.color.r;
        break;
    case '*':
        v *= (1 / 255.0f) * tb.font.color.r;
        break;
    default:
        return;
    }

    if (v < 0.0f) v = 0.0f;
    else if (v > 1.0f) v = 1.0f;

    static xtextbox::callback cb = { NULL, update_tag_red, update_tag_red };
    a.cb = &cb;
}

void reset_tag_red(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    static xtextbox::callback cb = { NULL, update_tag_reset_red, update_tag_reset_red };
    a.cb = &cb;
}

void update_tag_green(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.font.color.g = (U8)((F32&)j.context * 255.0f + 0.5f);
}

void update_tag_reset_green(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.font.color.g = ctb.font.color.g;
}

void parse_tag_green(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    F32& v = (F32&)a.context;

    if (ti.value.size == 0 || ti.action.size == 0) return;

    v = xatof(ti.value.text);

    switch (ti.action.text[0]) {
    case '=':
        break;
    case '+':
        v += (1 / 255.0f) * tb.font.color.g;
        break;
    case '*':
        v *= (1 / 255.0f) * tb.font.color.g;
        break;
    default:
        return;
    }

    if (v < 0.0f) v = 0.0f;
    else if (v > 1.0f) v = 1.0f;

    static xtextbox::callback cb = { NULL, update_tag_green, update_tag_green };
    a.cb = &cb;
}

void reset_tag_green(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    static xtextbox::callback cb = { NULL, update_tag_reset_green, update_tag_reset_green };
    a.cb = &cb;
}

void update_tag_blue(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.font.color.b = (U8)((F32&)j.context * 255.0f + 0.5f);
}

void update_tag_reset_blue(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.font.color.b = ctb.font.color.b;
}

void parse_tag_blue(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    F32& v = (F32&)a.context;

    if (ti.value.size == 0 || ti.action.size == 0) return;

    v = xatof(ti.value.text);

    switch (ti.action.text[0]) {
    case '=':
        break;
    case '+':
        v += (1 / 255.0f) * tb.font.color.b;
        break;
    case '*':
        v *= (1 / 255.0f) * tb.font.color.b;
        break;
    default:
        return;
    }

    if (v < 0.0f) v = 0.0f;
    else if (v > 1.0f) v = 1.0f;

    static xtextbox::callback cb = { NULL, update_tag_blue, update_tag_blue };
    a.cb = &cb;
}

void reset_tag_blue(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    static xtextbox::callback cb = { NULL, update_tag_reset_blue, update_tag_reset_blue };
    a.cb = &cb;
}

void update_tag_width(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.font.width = (F32&)j.context;
}

void update_tag_reset_width(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.font.width = ctb.font.width;
}

void parse_tag_width(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    F32& v = (F32&)a.context;

    if (ti.value.size == 0 || ti.action.size == 0) return;

    v = xatof(ti.value.text);

    switch (ti.action.text[0]) {
    case '=':
        break;
    case '+':
        v += tb.font.width;
        break;
    case '*':
        v *= tb.font.width;
        break;
    default:
        return;
    }

    if (v < 0.0f) v = 0.0f;
    else if (v > 1.0f) v = 1.0f;

    static xtextbox::callback cb = { NULL, update_tag_width, update_tag_width };
    a.cb = &cb;
}

void reset_tag_width(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    static xtextbox::callback cb = { NULL, update_tag_reset_width, update_tag_reset_width };
    a.cb = &cb;
}

void update_tag_height(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.font.height = (F32&)j.context;
}

void update_tag_reset_height(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.font.height = ctb.font.height;
}

void parse_tag_height(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    F32& v = (F32&)a.context;

    if (ti.value.size == 0 || ti.action.size == 0) return;

    v = xatof(ti.value.text);

    switch (ti.action.text[0]) {
    case '=':
        break;
    case '+':
        v += tb.font.height;
        break;
    case '*':
        v *= tb.font.height;
        break;
    default:
        return;
    }

    if (v < 0.0f) v = 0.0f;
    else if (v > 1.0f) v = 1.0f;

    static xtextbox::callback cb = { NULL, update_tag_height, update_tag_height };
    a.cb = &cb;
}

void reset_tag_height(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    static xtextbox::callback cb = { NULL, update_tag_reset_height, update_tag_reset_height };
    a.cb = &cb;
}

void update_tag_left_indent(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.left_indent = (F32&)j.context;
}

void update_tag_reset_left_indent(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.left_indent = ctb.font.height;
}

void parse_tag_left_indent(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    F32& v = (F32&)a.context;

    if (ti.value.size == 0 || ti.action.size == 0) return;

    v = xatof(ti.value.text);

    switch (ti.action.text[0]) {
    case '=':
        break;
    case '+':
        v += tb.left_indent;
        break;
    case '*':
        v *= tb.left_indent;
        break;
    default:
        return;
    }

    if (v < 0.0f) v = 0.0f;
    else if (v > 1.0f) v = 1.0f;

    static xtextbox::callback cb = { NULL, update_tag_left_indent, update_tag_left_indent };
    a.cb = &cb;
}

void reset_tag_left_indent(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    static xtextbox::callback cb = { NULL, update_tag_reset_left_indent, update_tag_reset_left_indent };
    a.cb = &cb;
}

void update_tag_right_indent(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.right_indent = (F32&)j.context;
}

void update_tag_reset_right_indent(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.right_indent = ctb.font.height;
}

void parse_tag_right_indent(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    F32& v = (F32&)a.context;

    if (ti.value.size == 0 || ti.action.size == 0) return;

    v = xatof(ti.value.text);

    switch (ti.action.text[0]) {
    case '=':
        break;
    case '+':
        v += tb.right_indent;
        break;
    case '*':
        v *= tb.right_indent;
        break;
    default:
        return;
    }

    if (v < 0.0f) v = 0.0f;
    else if (v > 1.0f) v = 1.0f;

    static xtextbox::callback cb = { NULL, update_tag_right_indent, update_tag_right_indent };
    a.cb = &cb;
}

void reset_tag_right_indent(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    static xtextbox::callback cb = { NULL, update_tag_reset_right_indent, update_tag_reset_right_indent };
    a.cb = &cb;
}

void update_tag_tab_stop(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.tab_stop = (F32&)j.context;
}

void update_tag_reset_tab_stop(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.tab_stop = ctb.font.height;
}

void parse_tag_tab_stop(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    F32& v = (F32&)a.context;

    if (ti.value.size == 0 || ti.action.size == 0) return;

    v = xatof(ti.value.text);

    switch (ti.action.text[0]) {
    case '=':
        break;
    case '+':
        v += tb.tab_stop;
        break;
    case '*':
        v *= tb.tab_stop;
        break;
    default:
        return;
    }

    if (v < 0.0f) v = 0.0f;
    else if (v > 1.0f) v = 1.0f;

    static xtextbox::callback cb = { NULL, update_tag_tab_stop, update_tag_tab_stop };
    a.cb = &cb;
}

void reset_tag_tab_stop(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    static xtextbox::callback cb = { NULL, update_tag_reset_tab_stop, update_tag_reset_tab_stop };
    a.cb = &cb;
}

void update_tag_xspace(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.font.space = (F32&)j.context;
}

void update_tag_reset_xspace(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.font.space = ctb.font.height;
}

void parse_tag_xspace(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    F32& v = (F32&)a.context;

    if (ti.value.size == 0 || ti.action.size == 0) return;

    v = xatof(ti.value.text);

    switch (ti.action.text[0]) {
    case '=':
        break;
    case '+':
        v += tb.font.space;
        break;
    case '*':
        v *= tb.font.space;
        break;
    default:
        return;
    }

    if (v < 0.0f) v = 0.0f;
    else if (v > 1.0f) v = 1.0f;

    static xtextbox::callback cb = { NULL, update_tag_xspace, update_tag_xspace };
    a.cb = &cb;
}

void reset_tag_xspace(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    static xtextbox::callback cb = { NULL, update_tag_reset_xspace, update_tag_reset_xspace };
    a.cb = &cb;
}

void update_tag_yspace(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.line_space = (F32&)j.context;
}

void update_tag_reset_yspace(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.line_space = ctb.font.height;
}

void parse_tag_yspace(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    F32& v = (F32&)a.context;

    if (ti.value.size == 0 || ti.action.size == 0) return;

    v = xatof(ti.value.text);

    switch (ti.action.text[0]) {
    case '=':
        break;
    case '+':
        v += tb.line_space;
        break;
    case '*':
        v *= tb.line_space;
        break;
    default:
        return;
    }

    if (v < 0.0f) v = 0.0f;
    else if (v > 1.0f) v = 1.0f;

    static xtextbox::callback cb = { NULL, update_tag_yspace, update_tag_yspace };
    a.cb = &cb;
}

void reset_tag_yspace(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    static xtextbox::callback cb = { NULL, update_tag_reset_yspace, update_tag_reset_yspace };
    a.cb = &cb;
}

void update_tag_reset_all(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb = ctb;
}

void reset_tag_all(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    static xtextbox::callback cb = { NULL, update_tag_reset_all, update_tag_reset_all };
    a.cb = &cb;
}

void update_tag_color(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.font.color = (xColor&)j.context;
}

void update_tag_reset_color(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.font.color = ctb.font.color;
}

void parse_tag_color(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    xColor& color = (xColor&)a.context;

    if (ti.value.size < 6 || ti.action.size == 0) return;

    U32 v = atox(ti.value);
    if (ti.value.size < 8) {
        v = (v & 0xFF000000) | (tb.font.color.a << 24);
    }

    switch (ti.action.text[0]) {
    case '=':
        color.a = (v & 0xFF000000) >> 24;
        color.r = (v & 0x00FF0000) >> 16;
        color.g = (v & 0x0000FF00) >> 8;
        color.b = (v & 0x000000FF) >> 0;
        break;
    case '+':
        color.a = xmin(255, ((v & 0xFF000000) >> 24) + tb.font.color.a);
        color.r = xmin(255, ((v & 0x00FF0000) >> 16) + tb.font.color.r);
        color.g = xmin(255, ((v & 0x0000FF00) >> 8) + tb.font.color.g);
        color.b = xmin(255, ((v & 0x000000FF) >> 0) + tb.font.color.b);
        break;
    case '*':
        color.a = ((v & 0xFF000000) >> 24) * tb.font.color.a / 255;
        color.r = ((v & 0x00FF0000) >> 16) * tb.font.color.r / 255;
        color.g = ((v & 0x0000FF00) >> 8) * tb.font.color.g / 255;
        color.b = ((v & 0x000000FF) >> 0) * tb.font.color.b / 255;
        break;
    default:
        return;
    }

    static xtextbox::callback cb = { NULL, update_tag_color, update_tag_color };
    a.cb = &cb;
}

void reset_tag_color(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    static xtextbox::callback cb = { NULL, update_tag_reset_color, update_tag_reset_color };
    a.cb = &cb;
}

void update_tag_font(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.font.id = (U32&)j.context;
}

void update_tag_reset_font(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.font.id = ctb.font.id;
}

void parse_tag_font(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    if (ti.action.size < 1 || ti.action.text[0] != '=' || ti.value.size == 0) return;

    U32& id = (U32&)a.context;
    id = atoi(ti.value.text);
    if (id < active_fonts_size) {
        static xtextbox::callback cb = { NULL, update_tag_font, update_tag_font };
        a.cb = &cb;
    }
}

void reset_tag_font(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    static xtextbox::callback cb = { NULL, update_tag_reset_font, update_tag_reset_font };
    a.cb = &cb;
}

void update_tag_wrap(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.flags = (tb.flags & ~0x30) | ((U32&)j.context & 0x30);
}

void update_tag_reset_wrap(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.flags = (tb.flags & ~0x30) | (ctb.flags & 0x30);
}

void parse_tag_wrap(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    if (ti.action.size < 1 || ti.action.text[0] != '=' || ti.value.size != 4) return;

    U32& flags = (U32&)a.context;
    if (imemcmp(ti.value.text, "word", 4) == 0) {
        flags = 0;
    } else if (imemcmp(ti.value.text, "char", 4) == 0) {
        flags = 0x10;
    } else if (imemcmp(ti.value.text, "none", 4) == 0) {
        flags = 0x20;
    } else {
        return;
    }

    static xtextbox::callback cb = { NULL, update_tag_wrap, update_tag_wrap };
    a.cb = &cb;
}

void reset_tag_wrap(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    static xtextbox::callback cb = { NULL, update_tag_reset_wrap, update_tag_reset_wrap };
    a.cb = &cb;
}

void update_tag_xjustify(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.flags = (tb.flags & ~0x3) | ((U32&)j.context & 0x3);
}

void update_tag_reset_xjustify(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.flags = (tb.flags & ~0x3) | (ctb.flags & 0x3);
}

void parse_tag_xjustify(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    if (ti.action.size < 1 || ti.action.text[0] != '=') return;

    U32& flags = (U32&)a.context;
    if (icompare(ti.value, substr::create("left", 4)) == 0) {
        flags = 0;
    } else if (icompare(ti.value, substr::create("center", 6)) == 0) {
        flags = 0x2;
    } else if (icompare(ti.value, substr::create("right", 5)) == 0) {
        flags = 0x1;
    } else {
        return;
    }

    static xtextbox::callback cb = { NULL, update_tag_xjustify, update_tag_xjustify };
    a.cb = &cb;
}

void reset_tag_xjustify(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    static xtextbox::callback cb = { NULL, update_tag_reset_xjustify, update_tag_reset_xjustify };
    a.cb = &cb;
}

void update_tag_yjustify(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.flags = (tb.flags & ~0xC) | ((U32&)j.context & 0xC);
}

void update_tag_reset_yjustify(const xtextbox::jot& j, xtextbox& tb, const xtextbox& ctb)
{
    tb.flags = (tb.flags & ~0xC) | (ctb.flags & 0xC);
}

void parse_tag_yjustify(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    if (ti.action.size < 1 || ti.action.text[0] != '=') return;

    U32& flags = (U32&)a.context;
    if (icompare(ti.value, substr::create("top", 4)) == 0) { // BUG: "top" is 3 chars, not 4
        flags = 0;
    } else if (icompare(ti.value, substr::create("center", 4)) == 0) { // BUG: "center" is 6 chars, not 4
        flags = 0x8;
    } else if (icompare(ti.value, substr::create("bottom", 4)) == 0) { // BUG: "bottom" is 6 chars, not 4
        flags = 0x4;
    } else {
        return;
    }

    static xtextbox::callback cb = { NULL, update_tag_yjustify, update_tag_yjustify };
    a.cb = &cb;
}

void reset_tag_yjustify(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    static xtextbox::callback cb = { NULL, update_tag_reset_yjustify, update_tag_reset_yjustify };
    a.cb = &cb;
}

void parse_tag_open_curly(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti) NONMATCH("https://decomp.me/scratch/tVpfD")
{
    a.s.text = ti.tag.text;
    a.s.size = 1;

    char c = a.s.text[0];

    a.reset_flags();
    a.bounds = tb.font.bounds(c);
    a.cb = tb.cb;
    a.context = NULL;
}

void parse_tag_newline(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    a.flag.line_break = true;
    a.flag.ethereal = false;
    a.bounds = tb.font.bounds('\n');
}

void parse_tag_tab(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    a.flag.tab = true;
}

void parse_tag_word_break(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    a.flag.word_break = true;
}

void parse_tag_page_break(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    a.flag.page_break = true;
}

struct model_tag_context
{
    xModelInstance* model;
    xVec3 rot;
    basic_rect<F32> dst;
    xSphere o;
};

void render_tag_model(const xtextbox::jot& j, const xtextbox& tb, F32 x, F32 y)
{
    const model_tag_context& mtc = *(const model_tag_context*)j.context;

    basic_rect<F32> dst = mtc.dst;
    dst.move(x, y);

    xVec3 from = { 0.0f, 0.0f, 1.0f };
    xVec3 to = { 0.0f, 0.0f, -0.001f };

    xMat4x3 frame;
    xMat3x3Euler(&frame, &mtc.rot);

    F32 ir = mtc.o.r <= 0.0f ? 1.0f : 0.5f / mtc.o.r;
    F32 scale = 1.001f * ir;

    frame.right *= scale;
    frame.up *= scale;
    frame.at *= scale;
    frame.pos = mtc.o.center;
    frame.flags = 0;

    xModelSetFrame(mtc.model, &frame);
    xModelSetMaterialAlpha(mtc.model, tb.font.color.a);

    tex_flush();

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEGOURAUD);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);

    xModelRender2D(*mtc.model, dst, from, to);

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEFLAT);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
}

void parse_tag_model(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    load_model_args(def_model_args, ti.tag);
    if (!def_model_args.model) return;

    model_tag_context& mtc = *(model_tag_context*)a.context;
    mtc.model = def_model_args.model;
    mtc.rot = def_model_args.rot;
    mtc.rot *= xdeg2rad(1.0f);
    mtc.dst = def_model_args.dst;

    const xSphere* o = xModelGetLocalSBound(mtc.model);
    mtc.o = *o;
    mtc.dst.y -= def_model_args.off.y;

    a.bounds.assign(0.0f, -def_model_args.off.y, def_model_args.off.x, def_model_args.off.y);

    switch (def_model_args.scale) {
    case model_args::SCALE_SCREEN:
        break;
    case model_args::SCALE_SIZE:
        mtc.dst.scale(mtc.o.r);
        a.bounds.scale(mtc.o.r);
        break;
    default:
        mtc.dst.scale(tb.font.width, tb.font.height);
        a.bounds.scale(tb.font.width, tb.font.height);
        break;
    }

    a.reset_flags();
    a.context_size = sizeof(model_tag_context);

    static xtextbox::callback cb = { render_tag_model, NULL, NULL };
    a.cb = &cb;
}

void reset_tag_model(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    reset_model_args(def_model_args);
}

struct tex_tag_context
{
    RwRaster* raster;
    F32 rot;
    basic_rect<F32> src;
    basic_rect<F32> dst;
};

void render_tag_tex(const xtextbox::jot& j, const xtextbox& tb, F32 x, F32 y)
{
    const tex_tag_context& ttc = *(const tex_tag_context*)j.context;

    set_tex_raster(ttc.raster);

    basic_rect<F32> dst = ttc.dst;
    dst.move(x, y);

    tex_render(ttc.src, dst, tb.font.clip, tb.font.color);
}

inline xVec2 get_texture_size(RwRaster& raster) NONMATCH("https://decomp.me/scratch/6WgKZ")
{
    xVec2 ret = {
        (F32)RwRasterGetWidth(&raster) / FB_XRES,
        (F32)RwRasterGetHeight(&raster) / FB_YRES
    };
    return ret;
}

void parse_tag_tex(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti) NONMATCH("https://decomp.me/scratch/5tB1p")
{
    load_tex_args(def_tex_args, ti.tag);
    if (!def_tex_args.raster) return;

    tex_tag_context& ttc = *(tex_tag_context*)a.context;
    ttc.raster = def_tex_args.raster;
    ttc.rot = def_tex_args.rot;
    ttc.src = def_tex_args.src;
    ttc.dst = def_tex_args.dst;
    ttc.dst.y -= def_tex_args.off.y;

    a.bounds.assign(0.0f, -def_tex_args.off.y, def_tex_args.off.x, def_tex_args.off.y);

    xVec2 scale;
    switch (def_tex_args.scale) {
    case tex_args::SCALE_SCREEN:
        scale.assign(1.0f, 1.0f);
        break;
    case tex_args::SCALE_SIZE:
        scale = get_texture_size(*ttc.raster);
        break;
    case tex_args::SCALE_FONT_WIDTH:
        scale = get_texture_size(*ttc.raster);
        scale.y *= tb.font.width / scale.x;
        scale.x = tb.font.width;
        break;
    case tex_args::SCALE_FONT_HEIGHT:
        scale = get_texture_size(*ttc.raster);
        scale.x *= tb.font.height / scale.y;
        scale.y = tb.font.height;
        break;
    case tex_args::SCALE_SCREEN_WIDTH:
        scale = get_texture_size(*ttc.raster);
        scale.y *= 1.0f / scale.x;
        scale.x = 1.0f;
        break;
    case tex_args::SCALE_SCREEN_HEIGHT:
        scale = get_texture_size(*ttc.raster);
        scale.x *= 1.0f / scale.y;
        scale.y = 1.0f;
        break;
    default:
        scale.assign(tb.font.width, tb.font.height);
        break;
    }

    ttc.dst.scale(scale.x, scale.y);

    a.bounds.scale(scale.x, scale.y);
    a.reset_flags();
    a.context_size = sizeof(tex_tag_context);

    static xtextbox::callback cb = { render_tag_tex, NULL, NULL };
    a.cb = &cb;
}

void reset_tag_tex(xtextbox::jot& a, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    reset_tex_args(def_tex_args);
}

void parse_tag_insert(xtextbox::jot& j, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    j.reset_flags();
    j.flag.invisible = j.flag.ethereal = true;

    if (ti.action.size != 1 || ti.action.text[0] != ':' || ti.value.size == 0) return;
    
    U32 id = xStrHash(ti.value.text, ti.value.size);
    if (id == 0) return;

    xTextAsset* ta = (xTextAsset*)xSTFindAsset(id, NULL);
    if (!ta) return;

    j.context = (void*)xTextGetString(ta);
    j.context_size = ta->len;
    j.flag.insert = true;
}

void parse_tag_insert_hash(xtextbox::jot& j, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    j.reset_flags();
    j.flag.invisible = j.flag.ethereal = true;

    if (ti.action.size != 1 || ti.action.text[0] != ':' || ti.value.size == 0) return;
    
    U32 id = atox(ti.value);
    if (id == 0) return;

    xTextAsset* ta = (xTextAsset*)xSTFindAsset(id, NULL);
    if (!ta) return;

    j.context = (void*)xTextGetString(ta);
    j.context_size = ta->len;
    j.flag.insert = true;
}

void parse_tag_pop(xtextbox::jot& j, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    if (ti.action.size != 1 || ti.action.text[0] != ':' || ti.value.size == 0) return;
}

void parse_tag_timer(xtextbox::jot& j, const xtextbox& tb, const xtextbox& ctb, const xtextbox::split_tag& ti)
{
    j.reset_flags();
    j.flag.invisible = j.flag.ethereal = true;

    if (ti.action.size != 1 || ti.action.text[0] != ':' || ti.value.size == 0) return;
    
    U32 id = xStrHash(ti.value.text, ti.value.size);
    if (id == 0) return;
    
    const xTimer* ta = (const xTimer*)zSceneFindObject(id);
    if (!ta) return;
    
    j.flag.insert = j.flag.dynamic = true;
    
    char buffer[64];
    U32 sec = (U32)ta->secondsLeft + 1;
    U32 mn = sec / 60;
    if (mn) {
        sprintf(buffer, "%d:%02d", mn, sec % 60);
    } else {
        sprintf(buffer, "%02d", sec);
    }
    
    char* text = (char*)j.context;
    sprintf(text, "%.*s", 15, buffer);
    j.context_size = 16;
}

namespace {

// If adding a new tag type, make sure this table is sorted and update format_tags_size!!!
xtextbox::tag_type format_tags_buffer[2][128] = {
    {
        { "", 0, parse_tag_open_curly, NULL, NULL },
        { "a", 1, parse_tag_alpha, reset_tag_alpha, NULL },
        { "all", 3, NULL, reset_tag_all, NULL },
        { "alpha", 5, parse_tag_alpha, reset_tag_alpha, NULL },
        { "b", 1, parse_tag_blue, reset_tag_blue, NULL },
        { "blue", 4, parse_tag_blue, reset_tag_blue, NULL },
        { "c", 1, parse_tag_color, reset_tag_color, NULL },
        { "color", 5, parse_tag_color, reset_tag_color, NULL },
        { "f", 1, parse_tag_font, reset_tag_font, NULL },
        { "font", 4, parse_tag_font, reset_tag_font, NULL },
        { "g", 1, parse_tag_green, reset_tag_green, NULL },
        { "green", 5, parse_tag_green, reset_tag_green, NULL },
        { "h", 1, parse_tag_height, reset_tag_height, NULL },
        { "height", 6, parse_tag_height, reset_tag_height, NULL },
        { "i", 1, parse_tag_insert, NULL, NULL },
        { "ih", 2, parse_tag_insert_hash, NULL, NULL },
        { "insert", 6, parse_tag_insert, NULL, NULL },
        { "left_indent", 11, parse_tag_left_indent, reset_tag_left_indent, NULL },
        { "li", 2, parse_tag_left_indent, reset_tag_left_indent, NULL },
        { "model", 5, parse_tag_model, reset_tag_model, NULL },
        { "n", 1, parse_tag_newline, NULL, NULL },
        { "newline", 7, parse_tag_newline, NULL, NULL },
        { "page_break", 10, parse_tag_page_break, NULL, NULL },
        { "pb", 2, parse_tag_page_break, NULL, NULL },
        { "pop", 3, parse_tag_pop, NULL, NULL },
        { "r", 1, parse_tag_red, reset_tag_red, NULL },
        { "red", 3, parse_tag_red, reset_tag_red, NULL },
        { "ri", 2, parse_tag_right_indent, reset_tag_right_indent, NULL },
        { "right_indent", 12, parse_tag_right_indent, reset_tag_right_indent, NULL },
        { "t", 1, parse_tag_tab, NULL, NULL },
        { "tab", 3, parse_tag_tab, NULL, NULL },
        { "tab_stop", 8, parse_tag_tab_stop, reset_tag_tab_stop, NULL },
        { "tex", 3, parse_tag_tex, reset_tag_tex, NULL },
        { "timer", 5, parse_tag_timer, NULL, NULL },
        { "ts", 2, parse_tag_tab_stop, reset_tag_tab_stop, NULL },
        { "w", 1, parse_tag_width, reset_tag_width, NULL },
        { "wb", 2, parse_tag_word_break, NULL, NULL },
        { "width", 5, parse_tag_width, reset_tag_width, NULL },
        { "word_break", 10, parse_tag_word_break, NULL, NULL },
        { "wrap", 4, parse_tag_wrap, reset_tag_wrap, NULL },
        { "xj", 2, parse_tag_xjustify, reset_tag_xjustify, NULL },
        { "xjustify", 8, parse_tag_xjustify, reset_tag_xjustify, NULL },
        { "xs", 2, parse_tag_xspace, reset_tag_xspace, NULL },
        { "xspace", 6, parse_tag_xspace, reset_tag_xspace, NULL },
        { "yj", 2, parse_tag_yjustify, reset_tag_yjustify, NULL },
        { "yjustify", 8, parse_tag_yjustify, reset_tag_yjustify, NULL },
        { "ys", 2, parse_tag_yspace, reset_tag_yspace, NULL },
        { "yspace", 6, parse_tag_yspace, reset_tag_yspace, NULL },
    }
};
xtextbox::tag_type* format_tags = format_tags_buffer[0];
size_t format_tags_size = 48;
}
}

void xtextbox::register_tags(const tag_type* t, size_t size) NONMATCH("https://decomp.me/scratch/mluWx")
{
    const tag_type* s1 = format_tags;
    const tag_type* s2 = t;
    const tag_type* end1 = s1 + format_tags_size;
    const tag_type* end2 = s2 + size;
    
    tag_type* d = (s1 == format_tags_buffer[0]) ? format_tags_buffer[0] + 128 : format_tags_buffer[0];
    format_tags = d;
    
    while (s1 < end1 && s2 < end2) {
        const tag_type& t1 = *s1;
        const tag_type& t2 = *s2;
        S32 c = icompare(t1.name, t2.name);
        if (c < 0) {
            *d = t1;
            s1++;
        } else if (c > 0) {
            *d = t2;
            s2++;
        } else {
            *d = t2;
            s1++;
            s2++;
        }
        d++;
    }

    while (s1 < end1) {
        *d = *s1;
        d++;
        s1++;
    }

    while (s2 < end2) {
        *d = *s2;
        d++;
        s2++;
    }

    format_tags_size = d - format_tags;
}

xtextbox::tag_type* xtextbox::find_format_tag(const substr& s, S32& index)
{
    S32 start = 0;
    S32 end = format_tags_size;
    while (start != end) {
        index = (start + end) / 2;
        
        tag_type& t = format_tags[index];
        S32 c = icompare(s, t.name);
        if (c < 0) {
            end = index;
        } else if (c > 0) {
            start = index + 1;
        } else {
            return &t;
        }
    }

    index = -1;
    return NULL;
}

namespace {
void set_rect_vert(RwIm2DVertex& vert, F32 x, F32 y, F32 z, xColor c, F32 rcz)
{
    RwIm2DVertexSetScreenX(&vert, x);
    RwIm2DVertexSetScreenY(&vert, y);
    RwIm2DVertexSetScreenZ(&vert, z);
    RwIm2DVertexSetRecipCameraZ(&vert, rcz);
    RwIm2DVertexSetIntRGBA(&vert, c.r, c.g, c.b, c.a);
}

void set_rect_verts(RwIm2DVertex* vert, F32 x, F32 y, F32 w, F32 h, xColor c, F32 rcz, F32 nsz)
{
    set_rect_vert(vert[0], x, y, nsz, c, rcz);
    set_rect_vert(vert[1], x, y+h, nsz, c, rcz);
    set_rect_vert(vert[2], x+w, y, nsz, c, rcz);
    set_rect_vert(vert[3], x+w, y+h, nsz, c, rcz);
}
}

void render_fill_rect(const basic_rect<F32>& bounds, xColor color) NONMATCH("https://decomp.me/scratch/xwBGf")
{
    if (bounds.empty()) return;
    
    F32 rcz, nsz;
    RwCamera* cam;
    RwIm2DVertex vert[4];

    cam = RwCameraGetCurrentCamera();
    rcz = 1.0f / RwCameraGetNearClipPlane(cam);
    nsz = RwIm2DGetNearScreenZ();

    xfont::set_render_state(NULL);
    
    basic_rect<F32> r = bounds;
    r.scale(FB_XRES, FB_YRES);

    set_rect_verts(vert, r.x, r.y, r.w, r.h, color, rcz, nsz);
    RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, vert, 4);

    xfont::restore_render_state();
}

#if defined(DEBUG) || defined(RELEASE)
void debug_mode_fontperf() WIP
{
}
#endif