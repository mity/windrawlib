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
#include "lock.h"
#include "backend-d2d.h"
#include "backend-gdix.h"


WD_HSTROKESTYLE 
wdCreateStrokeStyle(UINT dashStyle, UINT lineCap, UINT lineJoin)
{
    if(d2d_enabled()) {
        HRESULT hr;
        dummy_D2D1_STROKE_STYLE_PROPERTIES p;
        dummy_ID2D1StrokeStyle *s;

        p.startCap = lineCap;
        p.endCap = lineCap;
        p.dashCap = lineCap;
        p.lineJoin = lineJoin;
        p.miterLimit = 1.0f;
        p.dashStyle = dashStyle;
        p.dashOffset = 0.0f;

        wd_lock();
        hr = dummy_ID2D1Factory_CreateStrokeStyle(d2d_factory, &p, NULL, 0, &s);
        wd_unlock();
        if (FAILED(hr)) {
            WD_TRACE_HR("wdCreateStrokeStyle: "
                        "ID2D1Factory::CreateStrokeStyle() failed.");
            return NULL;
        }

        return (WD_HSTROKESTYLE)s;
    }
    else {
        gdix_strokestyle_t* s;

        s = gdix_strokestyle_alloc(0);
        if(s == NULL) {
            WD_TRACE("wdCreateStrokeStyle: "
                     "gdix_strokestyle_alloc() failed.");
            return NULL;
        }

        s->dashStyle = dashStyle;
        s->lineCap = lineCap;
        s->lineJoin = lineJoin;

        return (WD_HSTROKESTYLE)s;
    }
}

WD_HSTROKESTYLE 
wdCreateStrokeStyleCustom(const float* dashes, UINT dashesCount, UINT lineCap, UINT lineJoin)
{
    if(d2d_enabled()) {
        HRESULT hr;
        dummy_D2D1_STROKE_STYLE_PROPERTIES p;
        dummy_ID2D1StrokeStyle *s;

        p.startCap = lineCap;
        p.endCap = lineCap;
        p.dashCap = lineCap;
        p.lineJoin = lineJoin;
        p.miterLimit = 1.0f;
        p.dashStyle = dummy_D2D1_DASH_STYLE_CUSTOM;
        p.dashOffset = 0.0f;

        wd_lock();
        hr = dummy_ID2D1Factory_CreateStrokeStyle(d2d_factory, &p, dashes, dashesCount, &s);
        wd_unlock();
        if (FAILED(hr)) {
            WD_TRACE_HR("wdCreateStrokeStyle: "
                        "ID2D1Factory::CreateStrokeStyle() failed.");
            return NULL;
        }

        return (WD_HSTROKESTYLE)s;
    }
    else {
        gdix_strokestyle_t* s;

        s = gdix_strokestyle_alloc(dashesCount);
        if(s == NULL) {
            WD_TRACE("wdCreateStrokeStyle: "
                     "gdix_strokestyle_alloc() failed.");
            return NULL;
        }

        s->dashStyle = dummy_DashStyleCustom;
        s->lineCap = lineCap;
        s->lineJoin = lineJoin;
        memcpy(s->dashes, dashes, dashesCount*sizeof(float));
        s->dashesCount = dashesCount;

        return (WD_HSTROKESTYLE)s;
    }
}

void
wdDestroyStrokeStyle(WD_HSTROKESTYLE hStrokeStyle)
{
    if(d2d_enabled()) {
        dummy_ID2D1StrokeStyle_Release((dummy_ID2D1StrokeStyle*) hStrokeStyle);
    } else {
        gdix_strokestyle_t* s = (gdix_strokestyle_t*)hStrokeStyle;
        if (s->dashes)
            free(s->dashes);
        free((void*)hStrokeStyle);
    }
}
