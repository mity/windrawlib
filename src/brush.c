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
#include "backend-gdix.h"


WD_HBRUSH
wdCreateSolidBrush(WD_HCANVAS hCanvas, WD_COLOR color)
{
    if(d2d_enabled()) {
        d2d_canvas_t* c = (d2d_canvas_t*) hCanvas;
        dummy_ID2D1SolidColorBrush* b;
        dummy_D2D1_COLOR_F clr;
        HRESULT hr;

        d2d_init_color(&clr, color);
        hr = dummy_ID2D1RenderTarget_CreateSolidColorBrush(
                        c->target, &clr, NULL, &b);
        if(FAILED(hr)) {
            WD_TRACE_HR("wdCreateSolidBrush: "
                        "ID2D1RenderTarget::CreateSolidColorBrush() failed.");
            return NULL;
        }
        return (WD_HBRUSH) b;
    } else {
        dummy_GpSolidFill* b;
        int status;

        status = gdix_vtable->fn_CreateSolidFill(color, &b);
        if(status != 0) {
            WD_TRACE("wdCreateSolidBrush: "
                     "GdipCreateSolidFill() failed. [%d]", status);
            return NULL;
        }
        return (WD_HBRUSH) b;
    }
}

void
wdDestroyBrush(WD_HBRUSH hBrush)
{
    if(d2d_enabled()) {
        dummy_ID2D1Brush_Release((dummy_ID2D1Brush*) hBrush);
    } else {
        gdix_vtable->fn_DeleteBrush((void*) hBrush);
    }
}

void
wdSetSolidBrushColor(WD_HBRUSH hBrush, WD_COLOR color)
{
    if(d2d_enabled()) {
        dummy_D2D1_COLOR_F clr;

        d2d_init_color(&clr, color);
        dummy_ID2D1SolidColorBrush_SetColor((dummy_ID2D1SolidColorBrush*) hBrush, &clr);
    } else {
        dummy_GpSolidFill* b = (dummy_GpSolidFill*) hBrush;

        gdix_vtable->fn_SetSolidFillColor(b, (dummy_ARGB) color);
    }
}

WD_HBRUSH
wdCreateLinearGradientBrush(WD_HCANVAS hCanvas, const WD_POINT* p0, const WD_POINT* p1, const WD_COLOR* colors, const float* offsets, UINT stopCount)
{
    if(stopCount < 2)
        return NULL;
    if(d2d_enabled()) {
        d2d_canvas_t* c = (d2d_canvas_t*) hCanvas;

        HRESULT hr;
        dummy_ID2D1GradientStopCollection* collection;
        dummy_ID2D1LinearGradientBrush* b;
        // TODO: allocate memory dynamically
        dummy_D2D1_GRADIENT_STOP stops[100];
        dummy_D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES gradientProperties;

        for (UINT i = 0; i < stopCount; i++)
        {
            d2d_init_color(&stops[i].color, colors[i]);
            stops[i].position = offsets[i];
        }
        hr = dummy_ID2D1RenderTarget_CreateGradientStopCollection(c->target, stops, stopCount, dummy_D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP, &collection);
        if(FAILED(hr)) {
            WD_TRACE_HR("wdCreateLinearGradientBrush: "
                        "ID2D1RenderTarget::CreateGradientStopCollection() failed.");
            return NULL;
        }
        gradientProperties.startPoint.x = p0->x;
        gradientProperties.startPoint.y = p0->y;
        gradientProperties.endPoint.x = p1->x;
        gradientProperties.endPoint.y = p1->y;
        hr = dummy_ID2D1RenderTarget_CreateLinearGradientBrush(c->target, &gradientProperties, NULL, collection, &b);
        dummy_ID2D1GradientStopCollection_Release(collection);
        if(FAILED(hr)) {
            WD_TRACE_HR("wdCreateLinearGradientBrush: "
                        "ID2D1RenderTarget::CreateLinearGradientBrush() failed.");
            return NULL;
        }
        return (WD_HBRUSH) b;
    } else {
        int status;
        WD_COLOR color0 = colors[0];
        WD_COLOR color1 = colors[stopCount - 1];
        dummy_GpLineGradient* grad;
        status = gdix_vtable->fn_CreateLineBrush((const dummy_GpPointF*)p0, (const dummy_GpPointF*)p1, color0, color1, dummy_WrapModeTile, &grad);
        if(status != 0) {
            WD_TRACE("wdCreateLinearGradientBrush: "
                     "GdipCreateLineBrush() failed. [%d]", status);
            return NULL;
        }
        status = gdix_vtable->fn_SetLinePresetBlend(grad, colors, offsets, stopCount);
        if(status != 0) {
            WD_TRACE("wdCreateLinearGradientBrush: "
                     "GdipSetLinePresetBlend() failed. [%d]", status);
            return NULL;
        }
        return (WD_HBRUSH)grad;
    }
    return NULL;
}
