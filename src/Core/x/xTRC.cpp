#include "xTRC.h"

#include "xFont.h"

void xTRCInit() WIP
{
}

void RenderText(const char* text, bool enabled)
{
    static xtextbox tb = xtextbox::create(
        xfont::create(1, NSCREENX(19.0f), NSCREENY(22.0f), 0.0f, xColorFromRGBA(255, 230, 0, 255), screen_bounds),
        screen_bounds, 0x2, 0.0f, 0.0f, 0.0f, 0.0f
    );
    
    tb.set_text(enabled ? text : "");
    tb.bounds = screen_bounds;
    tb.bounds.contract(0.1f);
    tb.bounds.h = tb.yextent(true);
    tb.bounds.y = 0.5f - tb.bounds.h * 0.5f;
    tb.render(true);

    if (!enabled) {
        render_fill_rect(tb.font.clip, xColorFromRGBA(0, 0, 0, 150));
    }
}