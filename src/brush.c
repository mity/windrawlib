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
        c_ID2D1SolidColorBrush* b;
        c_D2D1_COLOR_F clr;
        HRESULT hr;

        d2d_init_color(&clr, color);
        hr = c_ID2D1RenderTarget_CreateSolidColorBrush(
                        c->target, &clr, NULL, &b);
        if(FAILED(hr)) {
            WD_TRACE_HR("wdCreateSolidBrush: "
                        "ID2D1RenderTarget::CreateSolidColorBrush() failed.");
            return NULL;
        }
        return (WD_HBRUSH) b;
    } else {
        c_GpSolidFill* b;
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
        c_ID2D1Brush_Release((c_ID2D1Brush*) hBrush);
    } else {
        gdix_vtable->fn_DeleteBrush((void*) hBrush);
    }
}

void
wdSetSolidBrushColor(WD_HBRUSH hBrush, WD_COLOR color)
{
    if(d2d_enabled()) {
        c_D2D1_COLOR_F clr;

        d2d_init_color(&clr, color);
        c_ID2D1SolidColorBrush_SetColor((c_ID2D1SolidColorBrush*) hBrush, &clr);
    } else {
        c_GpSolidFill* b = (c_GpSolidFill*) hBrush;

        gdix_vtable->fn_SetSolidFillColor(b, (c_ARGB) color);
    }
}

WD_HBRUSH
wdCreateLinearGradientBrushEx(WD_HCANVAS hCanvas, float x0, float y0, float x1, float y1,
    const WD_COLOR* colors, const float* offsets, UINT numStops)
{
    if(numStops < 2)
        return NULL;
    if(d2d_enabled()) {
        d2d_canvas_t* c = (d2d_canvas_t*) hCanvas;

        HRESULT hr;
        c_ID2D1GradientStopCollection* collection;
        c_ID2D1LinearGradientBrush* b;
        c_D2D1_GRADIENT_STOP* stops = (c_D2D1_GRADIENT_STOP*)malloc(numStops * sizeof(c_D2D1_GRADIENT_STOP));
        c_D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES gradientProperties;

        for (UINT i = 0; i < numStops; i++)
        {
            d2d_init_color(&stops[i].color, colors[i]);
            stops[i].position = offsets[i];
        }
        hr = c_ID2D1RenderTarget_CreateGradientStopCollection(c->target, stops, numStops, c_D2D1_GAMMA_2_2, c_D2D1_EXTEND_MODE_CLAMP, &collection);
        if(FAILED(hr)) {
            WD_TRACE_HR("wdCreateLinearGradientBrushEx: "
                        "ID2D1RenderTarget::CreateGradientStopCollection() failed.");
            free(stops);
            return NULL;
        }
        gradientProperties.startPoint.x = x0;
        gradientProperties.startPoint.y = y0;
        gradientProperties.endPoint.x = x1;
        gradientProperties.endPoint.y = y1;
        hr = c_ID2D1RenderTarget_CreateLinearGradientBrush(c->target, &gradientProperties, NULL, collection, &b);
        c_ID2D1GradientStopCollection_Release(collection);
		free(stops);
        if(FAILED(hr)) {
            WD_TRACE_HR("wdCreateLinearGradientBrushEx: "
                        "ID2D1RenderTarget::CreateLinearGradientBrush() failed.");
            return NULL;
        }
        return (WD_HBRUSH) b;
    } else {
        int status;
        WD_COLOR color0 = colors[0];
        WD_COLOR color1 = colors[numStops - 1];
        c_GpLineGradient* grad;
        c_GpPointF p0;
        c_GpPointF p1;
        p0.x = x0;
        p0.y = y0;
        p1.x = x1;
        p1.y = y1;
        status = gdix_vtable->fn_CreateLineBrush(&p0, &p1, color0, color1, c_WrapModeTile, &grad);
        if(status != 0) {
            WD_TRACE("wdCreateLinearGradientBrushEx: "
                     "GdipCreateLineBrush() failed. [%d]", status);
            return NULL;
        }
        status = gdix_vtable->fn_SetLinePresetBlend(grad, colors, offsets, numStops);
        if(status != 0) {
            WD_TRACE("wdCreateLinearGradientBrushEx: "
                     "GdipSetLinePresetBlend() failed. [%d]", status);
            return NULL;
        }
        return (WD_HBRUSH)grad;
    }
    return NULL;
}

WD_HBRUSH
wdCreateLinearGradientBrush(WD_HCANVAS hCanvas, float x0, float y0,
    WD_COLOR color0, float x1, float y1, WD_COLOR color1)
{
    WD_COLOR colors[] = { color0, color1 };
    float offsets[] = { 0.0f, 1.0f };
    return wdCreateLinearGradientBrushEx(hCanvas, x0, y0, x1, y1, colors, offsets, 2);
}

WD_HBRUSH
wdCreateRadialGradientBrushEx(WD_HCANVAS hCanvas, float cx, float cy, float r,
    float fx, float fy, const WD_COLOR* colors, const float* offsets, UINT numStops)
{
    if(numStops < 2)
        return NULL;
    if(d2d_enabled()) {
        d2d_canvas_t* c = (d2d_canvas_t*) hCanvas;

        HRESULT hr;
        c_ID2D1GradientStopCollection* collection;
        c_ID2D1RadialGradientBrush* b;
        c_D2D1_GRADIENT_STOP* stops = (c_D2D1_GRADIENT_STOP*)malloc(numStops * sizeof(c_D2D1_GRADIENT_STOP));
        c_D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES gradientProperties;

        for (UINT i = 0; i < numStops; i++)
        {
            d2d_init_color(&stops[i].color, colors[i]);
            stops[i].position = offsets[i];
        }
        hr = c_ID2D1RenderTarget_CreateGradientStopCollection(c->target, stops, numStops, c_D2D1_GAMMA_2_2, c_D2D1_EXTEND_MODE_CLAMP, &collection);
        if(FAILED(hr)) {
            WD_TRACE_HR("wdCreateRadialGradientBrushEx: "
                        "ID2D1RenderTarget::CreateGradientStopCollection() failed.");
            free(stops);
            return NULL;
        }
        gradientProperties.center.x = cx;
        gradientProperties.center.y = cy;
        gradientProperties.gradientOriginOffset.x = fx - cx;
        gradientProperties.gradientOriginOffset.y = fy - cy;
        gradientProperties.radiusX = r;
        gradientProperties.radiusY = r;
        hr = c_ID2D1RenderTarget_CreateRadialGradientBrush(c->target, &gradientProperties, NULL, collection, &b);
        c_ID2D1GradientStopCollection_Release(collection);
        free(stops);
        if(FAILED(hr)) {
            WD_TRACE_HR("wdCreateRadialGradientBrushEx: "
                        "ID2D1RenderTarget::CreateRadialGradientBrush() failed.");
            return NULL;
        }
        return (WD_HBRUSH) b;
    } else {
        // TODO: Colors outside of the ellipse can only get faked
        // with a second brush.
        WD_HPATH p;
        WD_RECT rect;
        rect.x0 = cx - r;
        rect.y0 = cy - r;
        rect.x1 = cx + r;
        rect.y1 = cy + r;
        p = wdCreateRoundedRectPath(hCanvas, &rect, r);

        int status;
        c_GpPathGradient* grad;
        status = gdix_vtable->fn_CreatePathGradientFromPath((void*)p, &grad);
        wdDestroyPath(p);
        if(status != 0) {
            WD_TRACE("wdCreateRadialGradientBrushEx: "
                     "GdipCreatePathGradientFromPath() failed. [%d]", status);
            return NULL;
        }
        WD_POINT focalPoint[1];
        focalPoint[0].x = fx;
        focalPoint[0].y = fy;
        gdix_vtable->fn_SetPathGradientCenterPoint(grad, (c_GpPointF*)focalPoint);

        float* reverseStops = (float*)malloc(numStops * sizeof(float));
        WD_COLOR* reverseColors = (WD_COLOR*)malloc(numStops * sizeof(WD_COLOR));
        for (UINT i = 0; i < numStops; i++) {
            reverseStops[i] = 1 - offsets[numStops - i - 1];
            reverseColors[i] = colors[numStops - i - 1];
        }

        status = gdix_vtable->fn_SetPathGradientPresetBlend(grad, reverseColors, reverseStops, numStops);
        free(reverseStops);
        free(reverseColors);
        if(status != 0) {
            WD_TRACE("wdCreateRadialGradientBrushEx: "
                     "GdipSetPathGradientPresetBlend() failed. [%d]", status);
            return NULL;
        }
        return (WD_HBRUSH) grad;
    }
    return NULL;
}

WD_HBRUSH
wdCreateRadialGradientBrush(WD_HCANVAS hCanvas, float cx, float cy, float r,
    WD_COLOR color0, WD_COLOR color1)
{
    WD_COLOR colors[] = { color0, color1 };
    float offsets[] = { 0.0f, 1.0f };
    return wdCreateRadialGradientBrushEx(hCanvas, cx, cy, r, cx, cy, colors, offsets, 2);
}
