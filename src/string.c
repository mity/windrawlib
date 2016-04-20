/*
 * WinDrawLib
 * Copyright (c) 2015-2016 Martin Mitas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "misc.h"
#include "backend-d2d.h"
#include "backend-dwrite.h"
#include "backend-gdix.h"
#include "lock.h"


void
wdDrawString(WD_HCANVAS hCanvas, WD_HFONT hFont, const WD_RECT* pRect,
             const WCHAR* pszText, int iTextLength, WD_HBRUSH hBrush,
             DWORD dwFlags)
{
    if(d2d_enabled()) {
        D2D1_POINT_2F origin = { pRect->x0, pRect->y0 };
        d2d_canvas_t* c = (d2d_canvas_t*) hCanvas;
        ID2D1Brush* b = (ID2D1Brush*) hBrush;
        dummy_IDWriteTextFormat* tf = (dummy_IDWriteTextFormat*) hFont;
        dummy_IDWriteTextLayout* layout;

        layout = dwrite_create_text_layout(tf, pRect, pszText, iTextLength, dwFlags);
        if(layout == NULL) {
            WD_TRACE("wdDrawString: dwrite_create_text_layout() failed.");
            return;
        }

        ID2D1RenderTarget_DrawTextLayout(c->target, origin,
                (IDWriteTextLayout*) layout, b,
                (dwFlags & WD_STR_NOCLIP) ? 0 : D2D1_DRAW_TEXT_OPTIONS_CLIP);

        dummy_IDWriteTextLayout_Release(layout);
    } else {
        gdix_canvas_t* c = (gdix_canvas_t*) hCanvas;
        dummy_GpRectF r;
        dummy_GpFont* f = (dummy_GpFont*) hFont;
        dummy_GpBrush* b = (dummy_GpBrush*) hBrush;

        r.x = pRect->x0;
        r.y = pRect->y0;
        r.w = pRect->x1 - pRect->x0;
        r.h = pRect->y1 - pRect->y0;

        gdix_canvas_apply_string_flags(c, dwFlags);
        gdix_vtable->fn_DrawString(c->graphics, pszText, iTextLength,
                f, &r, c->string_format, b);
    }
}

void
wdMeasureString(WD_HCANVAS hCanvas, WD_HFONT hFont, const WD_RECT* pRect,
                const WCHAR* pszText, int iTextLength, WD_RECT* pResult,
                DWORD dwFlags)
{
    if(d2d_enabled()) {
        dummy_IDWriteTextFormat* tf = (dummy_IDWriteTextFormat*) hFont;
        dummy_IDWriteTextLayout* layout;
        dummy_DWRITE_TEXT_METRICS tm;

        layout = dwrite_create_text_layout(tf, pRect, pszText, iTextLength, dwFlags);
        if(layout == NULL) {
            WD_TRACE("wdMeasureString: dwrite_create_text_layout() failed.");
            return;
        }

        dummy_IDWriteTextLayout_GetMetrics(layout, &tm);

        pResult->x0 = pRect->x0 + tm.left;
        pResult->y0 = pRect->y0 + tm.top;
        pResult->x1 = pResult->x0 + tm.width;
        pResult->y1 = pResult->y0 + tm.height;

        dummy_IDWriteTextLayout_Release(layout);
    } else {
        gdix_canvas_t* c = (gdix_canvas_t*) hCanvas;
        dummy_GpRectF r;
        dummy_GpFont* f = (dummy_GpFont*) hFont;
        dummy_GpRectF br;

        r.x = pRect->x0;
        r.y = pRect->y0;
        r.w = pRect->x1 - pRect->x0;
        r.h = pRect->y1 - pRect->y0;

        gdix_canvas_apply_string_flags(c, dwFlags);
        gdix_vtable->fn_MeasureString(c->graphics, pszText, iTextLength, f, &r,
                                c->string_format, &br, NULL, NULL);

        pResult->x0 = br.x;
        pResult->y0 = br.y;
        pResult->x1 = br.x + br.w;
        pResult->y1 = br.y + br.h;
    }
}

float
wdStringWidth(WD_HCANVAS hCanvas, WD_HFONT hFont, const WCHAR* pszText)
{
    const WD_RECT rcClip = { 0, 0, FLT_MAX, FLT_MAX };
    WD_RECT rcResult;

    wdMeasureString(hCanvas, hFont, &rcClip, pszText, wcslen(pszText),
                &rcResult, WD_STR_LEFTALIGN | WD_STR_NOWRAP);
    return rcResult.x1;
}
