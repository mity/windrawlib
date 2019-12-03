/*
 * WinDrawLib
 * Copyright (c) 2015-2019 Martin Mitas
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

#ifndef WD_BACKEND_GDIX_H
#define WD_BACKEND_GDIX_H

#include "misc.h"
#include <c-gdiplus.h>


typedef struct gdix_strokestyle_tag gdix_strokestyle_t;
struct gdix_strokestyle_tag {
  c_GpLineCap lineCap;
  c_GpLineJoin lineJoin;
  c_GpDashStyle dashStyle;
  UINT dashesCount;
  float dashes[1];
};

typedef struct gdix_canvas_tag gdix_canvas_t;
struct gdix_canvas_tag {
    HDC dc;
    c_GpGraphics* graphics;
    c_GpPen* pen;
    c_GpStringFormat* string_format;
    int dc_layout;
    UINT width  : 31;
    UINT rtl    :  1;

    HDC real_dc;        /* non-NULL if double buffering is enabled. */
    HBITMAP orig_bmp;
    int x;
    int y;
    int cx;
    int cy;
};


typedef struct gdix_vtable_tag gdix_vtable_t;
struct gdix_vtable_tag {
    /* Graphics functions */
    int (WINAPI* fn_CreateFromHDC)(HDC, c_GpGraphics**);
    int (WINAPI* fn_DeleteGraphics)(c_GpGraphics*);
    int (WINAPI* fn_GraphicsClear)(c_GpGraphics*, c_ARGB);
    int (WINAPI* fn_GetDC)(c_GpGraphics*, HDC*);
    int (WINAPI* fn_ReleaseDC)(c_GpGraphics*, HDC);
    int (WINAPI* fn_ResetClip)(c_GpGraphics*);
    int (WINAPI* fn_ResetWorldTransform)(c_GpGraphics*);
    int (WINAPI* fn_RotateWorldTransform)(c_GpGraphics*, float, c_GpMatrixOrder);
    int (WINAPI* fn_ScaleWorldTransform)(c_GpGraphics*, float, float, c_GpMatrixOrder);
    int (WINAPI* fn_SetClipPath)(c_GpGraphics*, c_GpPath*, c_GpCombineMode);
    int (WINAPI* fn_SetClipRect)(c_GpGraphics*, float, float, float, float, c_GpCombineMode);
    int (WINAPI* fn_SetPageUnit)(c_GpGraphics*, c_GpUnit);
    int (WINAPI* fn_SetPixelOffsetMode)(c_GpGraphics*, c_GpPixelOffsetMode);
    int (WINAPI* fn_SetSmoothingMode)(c_GpGraphics*, c_GpSmoothingMode);
    int (WINAPI* fn_TranslateWorldTransform)(c_GpGraphics*, float, float, c_GpMatrixOrder);
    int (WINAPI* fn_MultiplyWorldTransform)(c_GpGraphics*, c_GpMatrix*, c_GpMatrixOrder);
    int (WINAPI* fn_CreateMatrix2)(float, float, float, float, float, float, c_GpMatrix**);
    int (WINAPI* fn_DeleteMatrix)(c_GpMatrix*);

    /* Brush functions */
    int (WINAPI* fn_CreateSolidFill)(c_ARGB, c_GpSolidFill**);
    int (WINAPI* fn_DeleteBrush)(c_GpBrush*);
    int (WINAPI* fn_SetSolidFillColor)(c_GpSolidFill*, c_ARGB);
    int (WINAPI* fn_CreateLineBrush)(const c_GpPointF*, const c_GpPointF*, c_ARGB, c_ARGB, c_GpWrapMode, c_GpLineGradient**);
    int (WINAPI* fn_CreatePathGradientFromPath)(const c_GpPath*, c_GpPathGradient**);
    int (WINAPI* fn_SetLinePresetBlend)(c_GpLineGradient*, const c_ARGB*, const float*, INT);
    int (WINAPI* fn_SetPathGradientPresetBlend)(c_GpPathGradient*, const c_ARGB*, const float*, INT);
    int (WINAPI* fn_SetPathGradientCenterPoint)(c_GpPathGradient*, const c_GpPointF*);

    /* Pen functions */
    int (WINAPI* fn_CreatePen1)(c_ARGB, float, c_GpUnit, c_GpPen**);
    int (WINAPI* fn_DeletePen)(c_GpPen*);
    int (WINAPI* fn_SetPenBrushFill)(c_GpPen*, c_GpBrush*);
    int (WINAPI* fn_SetPenWidth)(c_GpPen*, float);
    int (WINAPI* fn_SetPenStartCap)(c_GpPen*, c_GpLineCap);
    int (WINAPI* fn_SetPenEndCap)(c_GpPen*, c_GpLineCap);
    int (WINAPI* fn_SetPenLineJoin)(c_GpPen*, c_GpLineJoin);
    int (WINAPI* fn_SetPenMiterLimit)(c_GpPen*, float);
    int (WINAPI* fn_SetPenDashStyle)(c_GpPen*, c_GpDashStyle);
    int (WINAPI* fn_SetPenDashArray)(c_GpPen*, const float*, INT);

    /* Path functions */
    int (WINAPI* fn_CreatePath)(c_GpFillMode, c_GpPath**);
    int (WINAPI* fn_DeletePath)(c_GpPath*);
    int (WINAPI* fn_ClosePathFigure)(c_GpPath*);
    int (WINAPI* fn_StartPathFigure)(c_GpPath*);
    int (WINAPI* fn_GetPathLastPoint)(c_GpPath*, c_GpPointF*);
    int (WINAPI* fn_AddPathArc)(c_GpPath*, float, float, float, float, float, float);
    int (WINAPI* fn_AddPathBezier)(c_GpPath*, float, float, float, float, float, float, float, float);
    int (WINAPI* fn_AddPathLine)(c_GpPath*, float, float, float, float);

    /* Font functions */
    int (WINAPI* fn_CreateFontFromLogfontW)(HDC, const LOGFONTW*, c_GpFont**);
    int (WINAPI* fn_DeleteFont)(c_GpFont*);
    int (WINAPI* fn_DeleteFontFamily)(c_GpFont*);
    int (WINAPI* fn_GetCellAscent)(const c_GpFont*, int, UINT16*);
    int (WINAPI* fn_GetCellDescent)(const c_GpFont*, int, UINT16*);
    int (WINAPI* fn_GetEmHeight)(const c_GpFont*, int, UINT16*);
    int (WINAPI* fn_GetFamily)(c_GpFont*, void**);
    int (WINAPI* fn_GetFontSize)(c_GpFont*, float*);
    int (WINAPI* fn_GetFontStyle)(c_GpFont*, int*);
    int (WINAPI* fn_GetLineSpacing)(const c_GpFont*, int, UINT16*);

    /* Image & bitmap functions */
    int (WINAPI* fn_LoadImageFromFile)(const WCHAR*, c_GpImage**);
    int (WINAPI* fn_LoadImageFromStream)(IStream*, c_GpImage**);
    int (WINAPI* fn_CreateBitmapFromHBITMAP)(HBITMAP, HPALETTE, c_GpBitmap**);
    int (WINAPI* fn_CreateBitmapFromHICON)(HICON, c_GpBitmap**);
    int (WINAPI* fn_DisposeImage)(c_GpImage*);
    int (WINAPI* fn_GetImageWidth)(c_GpImage*, UINT*);
    int (WINAPI* fn_GetImageHeight)(c_GpImage*, UINT*);
    int (WINAPI* fn_CreateBitmapFromScan0)(UINT, UINT, INT, c_GpPixelFormat, BYTE*, c_GpBitmap**);
    int (WINAPI* fn_BitmapLockBits)(c_GpBitmap*, const c_GpRectI*, UINT, c_GpPixelFormat, c_GpBitmapData*);
    int (WINAPI* fn_BitmapUnlockBits)(c_GpBitmap*, c_GpBitmapData*);
    int (WINAPI* fn_CreateBitmapFromGdiDib)(const BITMAPINFO*, void*, c_GpBitmap**);

    /* Cached bitmap functions */
    int (WINAPI* fn_CreateCachedBitmap)(c_GpBitmap*, c_GpGraphics*, c_GpCachedBitmap**);
    int (WINAPI* fn_DeleteCachedBitmap)(c_GpCachedBitmap*);
    int (WINAPI* fn_DrawCachedBitmap)(c_GpGraphics*, c_GpCachedBitmap*, INT, INT);

    /* String format functions */
    int (WINAPI* fn_CreateStringFormat)(int, LANGID, c_GpStringFormat**);
    int (WINAPI* fn_DeleteStringFormat)(c_GpStringFormat*);
    int (WINAPI* fn_SetStringFormatAlign)(c_GpStringFormat*, c_GpStringAlignment);
    int (WINAPI* fn_SetStringFormatLineAlign)(c_GpStringFormat*, c_GpStringAlignment);
    int (WINAPI* fn_SetStringFormatFlags)(c_GpStringFormat*, int);
    int (WINAPI* fn_SetStringFormatTrimming)(c_GpStringFormat*, c_GpStringTrimming);

    /* Draw/fill functions */
    int (WINAPI* fn_DrawArc)(c_GpGraphics*, c_GpPen*, float, float, float, float, float, float);
    int (WINAPI* fn_DrawImageRectRect)(c_GpGraphics*, c_GpImage*, float, float, float, float, float, float, float, float, c_GpUnit, const void*, void*, void*);
    int (WINAPI* fn_DrawEllipse)(c_GpGraphics*, c_GpPen*, float, float, float, float);
    int (WINAPI* fn_DrawLine)(c_GpGraphics*, c_GpPen*, float, float, float, float);
    int (WINAPI* fn_DrawBezier)(c_GpGraphics*, c_GpPen*, float, float, float, float, float, float, float, float);
    int (WINAPI* fn_DrawPath)(c_GpGraphics*, c_GpPen*, c_GpPath*);
    int (WINAPI* fn_DrawPie)(c_GpGraphics*, c_GpPen*, float, float, float, float, float, float);
    int (WINAPI* fn_DrawRectangle)(c_GpGraphics*, void*, float, float, float, float);
    int (WINAPI* fn_DrawString)(c_GpGraphics*, const WCHAR*, int, const c_GpFont*, const c_GpRectF*, const c_GpStringFormat*, const c_GpBrush*);
    int (WINAPI* fn_FillEllipse)(c_GpGraphics*, c_GpBrush*, float, float, float, float);
    int (WINAPI* fn_FillPath)(c_GpGraphics*, c_GpBrush*, c_GpPath*);
    int (WINAPI* fn_FillPie)(c_GpGraphics*, c_GpBrush*, float, float, float, float, float, float);
    int (WINAPI* fn_FillRectangle)(c_GpGraphics*, void*, float, float, float, float);
    int (WINAPI* fn_MeasureString)(c_GpGraphics*, const WCHAR*, int, const c_GpFont*, const c_GpRectF*, const c_GpStringFormat*, c_GpRectF*, int*, int*);
};


extern gdix_vtable_t* gdix_vtable;

static inline BOOL
gdix_enabled(void)
{
    return (gdix_vtable != NULL);
}


int gdix_init(void);
void gdix_fini(void);


/* Helpers */
gdix_canvas_t* gdix_canvas_alloc(HDC dc, const RECT* doublebuffer_rect, UINT width, BOOL rtl);
void gdix_canvas_free(gdix_canvas_t* c);
void gdix_rtl_transform(gdix_canvas_t* c);
void gdix_reset_transform(gdix_canvas_t* c);
void gdix_delete_matrix(c_GpMatrix* m);
void gdix_canvas_apply_string_flags(gdix_canvas_t* c, DWORD flags);
void gdix_setpen(c_GpPen* pen, c_GpBrush* brush, float width, gdix_strokestyle_t* style);
c_GpBitmap* gdix_bitmap_from_HBITMAP_with_alpha(HBITMAP hBmp, BOOL has_premultiplied_alpha);


#endif  /* WD_BACKEND_GDIX_H */
