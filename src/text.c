/*
 * WinDrawLib
 * Copyright (c) 2019 Martin Mitas
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


WD_HTEXT
wdCreateText(WD_HFONT hFont, const WD_RECT* pRect,
            const WCHAR* pszText, int iTextLength, DWORD dwFlags)
{
    const WD_RECT dummyRect = { 0.0f, 0.0f, 0.0f, 0.0f };

    if(pRect->x0 > pRect->x1  ||  pRect->y0 > pRect->y1)
        pRect = &dummyRect;

    if(d2d_enabled()) {
        dwrite_font_t* font = (dwrite_font_t*) hFont;
        dummy_IDWriteTextLayout* layout;

        layout = dwrite_create_text_layout(font->tf, pRect, pszText, iTextLength, dwFlags);

        return (WD_HTEXT) layout;
    } else {
        WD_TRACE("wdCreateText: Not currently implemented for GDI+ back-end.");
        return NULL;
    }
}

void
wdDestroyText(WD_HTEXT hText)
{
    if(d2d_enabled()) {
        dummy_IDWriteTextLayout* layout = (dummy_IDWriteTextLayout*) hText;
        dummy_IDWriteTextLayout_Release(layout);
    }
}

float
wdMinimalTextWidth(WD_HTEXT hText)
{
    if(d2d_enabled()) {
        dummy_IDWriteTextLayout* layout = (dummy_IDWriteTextLayout*) hText;
        float min_width;

        dummy_IDWriteTextLayout_DetermineMinWidth(layout, &min_width);
        return min_width;
    } else {
        return 0;
    }
}

void
wdSetTextMaxWidth(WD_HTEXT hText, float fWidth)
{
    if(d2d_enabled()) {
        dummy_IDWriteTextLayout* layout = (dummy_IDWriteTextLayout*) hText;
        dummy_IDWriteTextLayout_SetMaxWidth(layout, fWidth);
    }
}

void
wdSetTextMaxHeight(WD_HTEXT hText, float fHeight)
{
    if(d2d_enabled()) {
        dummy_IDWriteTextLayout* layout = (dummy_IDWriteTextLayout*) hText;
        dummy_IDWriteTextLayout_SetMaxHeight(layout, fHeight);
    }
}

BOOL
wdSetTextFontFamily(WD_HTEXT hText, UINT uPos, UINT uLen, const WCHAR* pszFamilyName)
{
    if(d2d_enabled()) {
        dummy_IDWriteTextLayout* layout = (dummy_IDWriteTextLayout*) hText;
        dummy_DWRITE_TEXT_RANGE range = { uPos, uLen };
        HRESULT hr;

        hr = dummy_IDWriteTextLayout_SetFontFamilyName(layout, pszFamilyName, range);
        if(FAILED(hr)) {
            WD_TRACE_HR("wdSetTextFontFamily: IDWriteTextLayout::SetFontFamilyName() failed.");
            return FALSE;
        }

        return TRUE;
    } else {
        return FALSE;
    }
}

void
wdSetTextFontSize(WD_HTEXT hText, UINT uPos, UINT uLen, float fSize)
{
    if(d2d_enabled()) {
        dummy_IDWriteTextLayout* layout = (dummy_IDWriteTextLayout*) hText;
        dummy_DWRITE_TEXT_RANGE range = { uPos, uLen };

        dummy_IDWriteTextLayout_SetFontSize(layout, fSize, range);
    }
}

void
wdSetTextFontStyle(WD_HTEXT hText, UINT uPos, UINT uLen, int iStyle)
{
    if(d2d_enabled()) {
        dummy_IDWriteTextLayout* layout = (dummy_IDWriteTextLayout*) hText;
        dummy_DWRITE_TEXT_RANGE range = { uPos, uLen };
        dummy_DWRITE_FONT_STYLE style;

        switch(iStyle) {
            case WD_TEXTSTYLE_NORMAL:   style = dummy_DWRITE_FONT_STYLE_NORMAL; break;
            case WD_TEXTSTYLE_OBLIQUE:  style = dummy_DWRITE_FONT_STYLE_OBLIQUE; break;
            case WD_TEXTSTYLE_ITALIC:   /* Pass through. */
            default:                    style = dummy_DWRITE_FONT_STYLE_ITALIC; break;
        }

        dummy_IDWriteTextLayout_SetFontStyle(layout, style, range);
    }
}

void
wdSetTextFontWeight(WD_HTEXT hText, UINT uPos, UINT uLen, LONG lfWeight)
{
    if(d2d_enabled()) {
        dummy_IDWriteTextLayout* layout = (dummy_IDWriteTextLayout*) hText;
        dummy_DWRITE_TEXT_RANGE range = { uPos, uLen };

        dummy_IDWriteTextLayout_SetFontWeight(layout, lfWeight, range);
    }
}

void
wdSetTextStrikethrough(WD_HTEXT hText, UINT uPos, UINT uLen, BOOL bStrikethrough)
{
    if(d2d_enabled()) {
        dummy_IDWriteTextLayout* layout = (dummy_IDWriteTextLayout*) hText;
        dummy_DWRITE_TEXT_RANGE range = { uPos, uLen };

        dummy_IDWriteTextLayout_SetStrikethrough(layout, bStrikethrough, range);
    }
}

void
wdSetTextUnderline(WD_HTEXT hText, UINT uPos, UINT uLen, BOOL bUnderline)
{
    if(d2d_enabled()) {
        dummy_IDWriteTextLayout* layout = (dummy_IDWriteTextLayout*) hText;
        dummy_DWRITE_TEXT_RANGE range = { uPos, uLen };

        dummy_IDWriteTextLayout_SetUnderline(layout, bUnderline, range);
    }
}

void
wdTextMetrics(WD_HTEXT hText, WD_TEXTMETRICS* pMetrics)
{
    if(d2d_enabled()) {
        dummy_IDWriteTextLayout* layout = (dummy_IDWriteTextLayout*) hText;

        /* We have designed WD_TEXTMETRICS to be binary compatible with
         * DWRITE_TEXT_METRICS. */
        dummy_IDWriteTextLayout_GetMetrics(layout, (dummy_DWRITE_TEXT_METRICS*) pMetrics);
    }
}

void
wdDrawText(WD_HCANVAS hCanvas, WD_HTEXT hText, WD_HBRUSH hBrush,
            float x, float y, DWORD dwFlags)
{
    if(d2d_enabled()) {
        dummy_D2D1_POINT_2F origin = { x, y };
        d2d_canvas_t* c = (d2d_canvas_t*) hCanvas;
        dummy_ID2D1Brush* b = (dummy_ID2D1Brush*) hBrush;
        dummy_IDWriteTextLayout* layout = (dummy_IDWriteTextLayout*) hText;
        dummy_D2D1_MATRIX_3X2_F old_matrix;

        if(c->flags & D2D_CANVASFLAG_RTL) {
            d2d_disable_rtl_transform(c, &old_matrix);
            origin.x = (float)c->width - dummy_IDWriteTextLayout_GetMaxWidth(layout);

            dummy_IDWriteTextLayout_SetReadingDirection(layout,
                    dummy_DWRITE_READING_DIRECTION_RIGHT_TO_LEFT);
        }

        dummy_ID2D1RenderTarget_DrawTextLayout(c->target, origin, layout, b,
                (dwFlags & WD_STR_NOCLIP) ? 0 : dummy_D2D1_DRAW_TEXT_OPTIONS_CLIP);

        if(c->flags & D2D_CANVASFLAG_RTL) {
            dummy_ID2D1RenderTarget_SetTransform(c->target, &old_matrix);
        }
    }
}
