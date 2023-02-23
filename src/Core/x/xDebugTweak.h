#pragma once

#include "xString.h"
#include "xVec3.h"

struct tweak_callback;

struct tweak_info
{
    substr name;
    void* value;
    const tweak_callback* cb;
    void* context;
    U8 type;
    U8 value_size;
    U16 flags;
    union
    {
        struct
        {
            S32 value_def;
            S32 value_min;
            S32 value_max;
        } int_context;
        struct
        {
            U32 value_def;
            U32 value_min;
            U32 value_max;
        } uint_context;
        struct
        {
            F32 value_def;
            F32 value_min;
            F32 value_max;
        } float_context;
        struct
        {
            U8 value_def;
        } bool_context;
        struct
        {
            U32 value_def;
            U32 labels_size;
            const char** labels;
            void* values;
        } select_context;
        struct
        {
            U32 value_def;
            U32 mask;
        } flag_context;
        struct
        {
            U8 pad[16];
        } all_context;
    };
};

struct tweak_callback
{
    void(*on_change)(tweak_info&);
    void(*on_select)(tweak_info&);
    void(*on_unselect)(tweak_info&);
    void(*on_start_edit)(tweak_info&);
    void(*on_stop_edit)(tweak_info&);
    void(*on_expand)(tweak_info&);
    void(*on_collapse)(tweak_info&);
    void(*on_update)(tweak_info&);
    void(*convert_mem_to_tweak)(tweak_info&, void*);
    void(*convert_tweak_to_mem)(tweak_info&, void*);
};

#if defined(DEBUG) || defined(RELEASE)
void debug_mode_tweak();
#endif

inline void xDebugRemoveTweak(const char* name)
{
}

inline void xDebugAddSelectTweak(const char* name, U32* v, const char** labels, const U32* values,
                                 U32 labels_size, const tweak_callback* cb, void* context,
                                 U32 flags)
{
}

inline void xDebugAddFlagTweak(const char* name, U32* v, U32 mask, const tweak_callback* cb,
                               void* context, U32 flags)
{
}

inline void xDebugAddTweak(const char* name, S16* v, S16 vmin, S16 vmax, const tweak_callback* cb,
                           void* context, U32 flags)
{
}

inline void xDebugAddTweak(const char* name, U32* v, U32 vmin, U32 vmax, const tweak_callback* cb,
                           void* context, U32 flags)
{
}

inline void xDebugAddTweak(const char* name, U8* v, U8 vmin, U8 vmax, const tweak_callback* cb,
                           void* context, U32 flags)
{
}

inline void xDebugAddTweak(const char* name, F32* v, F32 vmin, F32 vmax, const tweak_callback* cb,
                           void* context, U32 flags)
{
}

inline void xDebugAddTweak(const char* name, xVec3* v, const tweak_callback* cb,
                           void* context, U32 flags)
{
}

inline void xDebugAddTweak(const char* name, const char* message, const tweak_callback* cb,
                           void* context, U32 flags)
{
}