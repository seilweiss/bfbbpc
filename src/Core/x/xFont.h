#pragma once

#include "xColor.h"
#include "xMath2.h"
#include "xString.h"

#include <rwcore.h>

static const basic_rect<F32> screen_bounds = { 0.0f, 0.0f, 1.0f, 1.0f };

inline F32 NSCREENX(F32 v) { return (1.0f/FB_XRES) * v; }
inline F32 NSCREENY(F32 v) { return (1.0f/FB_YRES) * v; }

class xfont
{
public:
    U32 id;
    F32 width;
    F32 height;
    F32 space;
    xColor color;
    basic_rect<F32> clip;

    static xfont create() NONMATCH("https://decomp.me/scratch/dZHiX")
    {
        return create(0, 0.0f, 0.0f, 0.0f, g_WHITE, screen_bounds);
    }

    static xfont create(U32 id, F32 width, F32 height, F32 space, xColor color, const basic_rect<F32>& clip)
    {
        xfont r;
        r.id = id;
        r.width = width;
        r.height = height;
        r.space = space;
        r.color = color;
        r.clip = clip;
        return r;
    }

    basic_rect<F32> bounds(char c) const;
    basic_rect<F32> bounds(const char* text) const;
    basic_rect<F32> bounds(const char* text, size_t text_size, F32 max_width, size_t& size) const;
    void start_render() const;
    void stop_render() const;
    void irender(const char* text, F32 x, F32 y) const;
    void irender(const char* text, size_t text_size, F32 x, F32 y) const;

    void render(const char* text, F32 x, F32 y) const
    {
        start_render();
        irender(text, x, y);
        stop_render();
    }

    static void init();
    static void set_render_state(RwRaster* raster);
    static void restore_render_state();
};

class xtextbox
{
public:
    struct callback;
    struct split_tag;
    struct tag_type;
    struct tag_entry;
    struct tag_entry_list;
    struct jot;
    struct jot_line;
    class layout;

    xfont font;
    basic_rect<F32> bounds;
    U32 flags;
    F32 line_space;
    F32 tab_stop;
    F32 left_indent;
    F32 right_indent;
    const callback* cb;
    void* context;

    static callback text_cb;

    static xtextbox create()
    {
        return create(xfont::create(), screen_bounds, 0, 0.0f, 0.0f, 0.0f, 0.0f);
    }

    static xtextbox create(const xfont& font, const basic_rect<F32>& bounds, U32 flags, F32 line_space, F32 tab_stop, F32 left_indent, F32 right_indent)
    {
        xtextbox r;
        r.font = font;
        r.bounds = bounds;
        r.flags = flags;
        r.line_space = line_space;
        r.tab_stop = tab_stop;
        r.left_indent = left_indent;
        r.right_indent = right_indent;
        r.texts_size = 0;
        r.text_hash = 0;
        r.cb = &text_cb;
        return r;
    }

    void set_text(const char* text);
    void set_text(const char* text, size_t text_size);
    void set_text(const char** texts, size_t size);
    void set_text(const char** texts, const size_t* text_sizes, size_t size);
    layout& temp_layout(bool cache) const;
    void render(layout& l, S32 begin_jot, S32 end_jot) const;
    F32 yextent(F32 max, S32& size, const layout& l, S32 begin_jot, S32 end_jot) const;

    void render(bool cache) const
    {
        render(temp_layout(cache), 0, -1);
    }

    F32 yextent(const layout& l, S32 begin_jot, S32 end_jot) const
    {
        S32 size;
        return yextent(HUGE, size, l, begin_jot, end_jot);
    }

    F32 yextent(bool cache) const
    {
        return yextent(temp_layout(cache), 0, -1);
    }

    F32 yextent(F32 max, S32& size, bool cache) const
    {
        return yextent(max, size, temp_layout(cache), 0, -1);
    }

    static void text_render(const jot& j, const xtextbox& tb, F32 x, F32 y);
    static tag_entry_list read_tag(const substr& s);
    static const tag_entry* find_entry(const tag_entry_list& el, const substr& name);
    static size_t read_list(const tag_entry& e, F32* v, size_t vsize);
    static size_t read_list(const tag_entry& e, S32* v, size_t vsize);
    static void clear_layout_cache();
    static void register_tags(const tag_type* t, size_t size);
    static tag_type* find_format_tag(const substr& s, S32& index);

    static tag_type* find_format_tag(const substr& s)
    {
        S32 index;
        return find_format_tag(s, index);
    }

    static void xtextbox::log_layout_stats(bool log) WIP
    {
    }

private:
    const char** texts;
    const size_t* text_sizes;
    size_t texts_size;
    substr text;
    U32 text_hash;
    
    friend class layout;
};

struct xtextbox::callback
{
    void(*render)(const jot&, const xtextbox&, F32, F32);
    void(*layout_update)(const jot&, xtextbox&, const xtextbox&);
    void(*render_update)(const jot&, xtextbox&, const xtextbox&);
};

struct xtextbox::split_tag
{
    substr tag;
    substr name;
    substr action;
    substr value;
};

struct xtextbox::tag_type
{
    substr name;
    void(*parse_tag)(jot&, const xtextbox&, const xtextbox&, const split_tag&);
    void(*reset_tag)(jot&, const xtextbox&, const xtextbox&, const split_tag&);
    void* context;
};

struct xtextbox::tag_entry
{
    substr name;
    char op;
    substr* args;
    size_t args_size;
};

struct xtextbox::tag_entry_list
{
    const tag_entry* entries;
    size_t size;
};

struct xtextbox::jot
{
    substr s;
    struct
    {
        bool invisible : 1; // 24
        bool ethereal : 1; // 25
        bool merge : 1; // 26
        bool word_break : 1; // 27
        bool word_end : 1; // 28
        bool line_break : 1; // 29
        bool stop : 1; // 30
        bool tab : 1; // 31
        bool insert : 1; // 24
        bool dynamic : 1; // 25
        bool page_break : 1; // 26
        bool stateful : 1; // 27
        U16 dummy : 4;
    } flag;
    U16 context_size;
    void* context;
    basic_rect<F32> bounds;
    basic_rect<F32> render_bounds;
    const callback* cb;
    const tag_type* tag;

    void reset_flags()
    {
        *(U16*)&flag = 0;
    }

    void intersect_flags(const jot& a)
    {
        *(U16*)&flag &= *(U16*)&a.flag;
    }
};

struct xtextbox::jot_line
{
    basic_rect<F32> bounds;
    F32 baseline;
    size_t first;
    size_t last;
    bool page_break;
};

class xtextbox::layout
{
public:
    void refresh(const xtextbox& tb, bool force);
    void refresh_end(const xtextbox& tb);
    void clear();
    void trim_line(jot_line& line);
    void erase_jots(size_t first, size_t last);
    void merge_line(jot_line& line);
    void bound_line(jot_line& line);
    bool fit_line();
    void next_line();
    void calc(const xtextbox& ctb, size_t start_text);
    void render(const xtextbox& ctb, S32 begin_jot, S32 end_jot);
    F32 yextent(F32 max, S32& size, S32 begin_jot, S32 end_jot) const;
    bool changed(const xtextbox& ctb);

    const jot* jots() const { return _jots; }
    size_t jots_size() const { return _jots_size; }

private:
    xtextbox tb;
    jot _jots[512];
    size_t _jots_size;
    jot_line _lines[128];
    size_t _lines_size;
    U8 context_buffer[1024];
    size_t context_buffer_size;
    U16 dynamics[64];
    size_t dynamics_size;

    friend class xtextbox;
};

void render_fill_rect(const basic_rect<F32>& bounds, xColor color);

#if defined(DEBUG) || defined(RELEASE)
void debug_mode_fontperf();
#endif