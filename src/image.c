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
#include "backend-wic.h"
#include "backend-gdix.h"
#include "lock.h"
#include "memstream.h"


WD_HIMAGE
wdCreateImageFromHBITMAP(HBITMAP hBmp)
{
    if(d2d_enabled()) {
        IWICBitmap* bitmap;
        IWICBitmapSource* converted_bitmap;
        HRESULT hr;

        if(wic_factory == NULL) {
            WD_TRACE("wdCreateImageFromHBITMAP: Image API disabled.");
            return NULL;
        }

        hr = IWICImagingFactory_CreateBitmapFromHBITMAP(wic_factory, hBmp,
                                                        NULL, WICBitmapUsePremultipliedAlpha, &bitmap);
        if(FAILED(hr)) {
            WD_TRACE_HR("wdCreateImageFromHBITMAP: "
                        "IWICImagingFactory::CreateBitmapFromHBITMAP() failed.");
            return NULL;
        }

        converted_bitmap = wic_convert_bitmap((IWICBitmapSource*) bitmap);
        if(converted_bitmap == NULL)
            WD_TRACE("wdCreateImageFromHBITMAP: wic_convert_bitmap() failed.");

        IWICBitmap_Release(bitmap);

        return (WD_HIMAGE) converted_bitmap;
    } else {
        dummy_GpBitmap* b;
        int status;

        /* OLD status = gdix_vtable->fn_CreateBitmapFromHBITMAP(hBmp, NULL, &b); - does not support alpha */
        status = gdix_create_bitmap(hBmp, &b);
        if (status != 0) {
            WD_TRACE("wdCreateImageFromHBITMAP: "
                     "GdipCreateBitmapFromHBITMAP() failed. [%d]", status);
            return NULL;
        }

        return (WD_HIMAGE) b;
    }
}

WD_HIMAGE
wdLoadImageFromFile(const WCHAR* pszPath)
{
    if(d2d_enabled()) {
        IWICBitmapDecoder* decoder;
        IWICBitmapFrameDecode* bitmap;
        IWICBitmapSource* converted_bitmap = NULL;
        HRESULT hr;

        if(wic_factory == NULL) {
            WD_TRACE("wdLoadImageFromFile: Image API disabled.");
            return NULL;
        }

        hr = IWICImagingFactory_CreateDecoderFromFilename(wic_factory, pszPath,
                NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
        if(FAILED(hr)) {
            WD_TRACE_HR("wdLoadImageFromFile: "
                        "IWICImagingFactory::CreateDecoderFromFilename() failed.");
            goto err_CreateDecoderFromFilename;
        }

        hr = IWICBitmapDecoder_GetFrame(decoder, 0, &bitmap);
        if(FAILED(hr)) {
            WD_TRACE_HR("wdLoadImageFromFile: "
                        "IWICBitmapDecoder::GetFrame() failed.");
            goto err_GetFrame;
        }

        converted_bitmap = wic_convert_bitmap((IWICBitmapSource*) bitmap);
        if(converted_bitmap == NULL)
            WD_TRACE("wdLoadImageFromFile: wic_convert_bitmap() failed.");

        IWICBitmapFrameDecode_Release(bitmap);
err_GetFrame:
        IWICBitmapDecoder_Release(decoder);
err_CreateDecoderFromFilename:
        return (WD_HIMAGE) converted_bitmap;
    } else {
        dummy_GpImage* img;
        int status;

        status = gdix_vtable->fn_LoadImageFromFile(pszPath, &img);
        if(status != 0) {
            WD_TRACE("wdLoadImageFromFile: "
                     "GdipLoadImageFromFile() failed. [%d]", status);
            return NULL;
        }

        return (WD_HIMAGE) img;
    }
}

WD_HIMAGE
wdLoadImageFromIStream(IStream* pStream)
{
    if(d2d_enabled()) {
        IWICBitmapDecoder* decoder;
        IWICBitmapFrameDecode* bitmap;
        IWICBitmapSource* converted_bitmap = NULL;
        HRESULT hr;

        if(wic_factory == NULL) {
            WD_TRACE("wdLoadImageFromIStream: Image API disabled.");
            return NULL;
        }

        hr = IWICImagingFactory_CreateDecoderFromStream(wic_factory, pStream,
                NULL, WICDecodeMetadataCacheOnLoad, &decoder);
        if(FAILED(hr)) {
            WD_TRACE_HR("wdLoadImageFromIStream: "
                        "IWICImagingFactory::CreateDecoderFromFilename() failed.");
            goto err_CreateDecoderFromFilename;
        }

        hr = IWICBitmapDecoder_GetFrame(decoder, 0, &bitmap);
        if(FAILED(hr)) {
            WD_TRACE_HR("wdLoadImageFromIStream: "
                        "IWICBitmapDecoder::GetFrame() failed.");
            goto err_GetFrame;
        }

        converted_bitmap = wic_convert_bitmap((IWICBitmapSource*) bitmap);
        if(converted_bitmap == NULL)
            WD_TRACE("wdLoadImageFromIStream: wic_convert_bitmap() failed.");

        IWICBitmapFrameDecode_Release(bitmap);
err_GetFrame:
        IWICBitmapDecoder_Release(decoder);
err_CreateDecoderFromFilename:
        return (WD_HIMAGE) converted_bitmap;
    } else {
        dummy_GpImage* img;
        int status;

        status = gdix_vtable->fn_LoadImageFromStream(pStream, &img);
        if(status != 0) {
            WD_TRACE("wdLoadImageFromIStream: "
                     "GdipLoadImageFromFile() failed. [%d]", status);
            return NULL;
        }

        return (WD_HIMAGE) img;
    }
}

WD_HIMAGE
wdLoadImageFromResource(HINSTANCE hInstance, const WCHAR* pszResType,
                        const WCHAR* pszResName)
{
    IStream* stream;
    WD_HIMAGE img;
    HRESULT hr;

    hr = memstream_create_from_resource(hInstance, pszResType, pszResName, &stream);
    if(FAILED(hr)) {
        WD_TRACE_HR("wdLoadImageFromResource: "
                 "memstream_create_from_resource() failed.");
        return NULL;
    }

    img = wdLoadImageFromIStream(stream);
    if(img == NULL)
        WD_TRACE("wdLoadImageFromResource: wdLoadImageFromIStream() failed.");

    IStream_Release(stream);
    return img;
}

void
wdDestroyImage(WD_HIMAGE hImage)
{
    if(d2d_enabled()) {
        IWICBitmapSource_Release((IWICBitmapSource*) hImage);
    } else {
        gdix_vtable->fn_DisposeImage((dummy_GpImage*) hImage);
    }
}

void
wdGetImageSize(WD_HIMAGE hImage, UINT* puWidth, UINT* puHeight)
{
    if(d2d_enabled()) {
        UINT w, h;

        IWICBitmapSource_GetSize((IWICBitmapSource*) hImage, &w, &h);
        if(puWidth != NULL)
            *puWidth = w;
        if(puHeight != NULL)
            *puHeight = h;
    } else {
        if(puWidth != NULL)
            gdix_vtable->fn_GetImageWidth((dummy_GpImage*) hImage, puWidth);
        if(puHeight != NULL)
            gdix_vtable->fn_GetImageHeight((dummy_GpImage*) hImage, puHeight);
    }
}

static void 
wdBufferRGB2Bitmap(BYTE* Scan0, INT dstStride, INT srcStride, UINT channels, UINT width, UINT height, const BYTE *rgb)
{
  UINT i, j;

  for (j = 0; j < height; j++)
  {
    UINT line_offset = j * srcStride;
    const BYTE* rgb_line = rgb + line_offset;
    BYTE* line_data = Scan0 + j * dstStride;

    for (i = 0; i < width; i++)
    {
      int offset = i * 3;
      int offset_data = i * channels;
      line_data[offset_data + 0] = rgb_line[offset + 2];  /* Blue */
      line_data[offset_data + 1] = rgb_line[offset + 1];  /* Green */
      line_data[offset_data + 2] = rgb_line[offset + 0];  /* Red */

      if (channels == 4)
        line_data[offset_data + 3] = 255;
    }
  }
}

static void 
wdBufferRGBA2Bitmap(BYTE* Scan0, INT dstStride, INT srcStride, BOOL PreAlpha, UINT width, UINT height, const BYTE* rgba)
{
  UINT i, j;

  for (j = 0; j < height; j++)
  {
    UINT line_offset = j * srcStride;
    const BYTE* rgba_line = rgba + line_offset;
    BYTE* line_data = Scan0 + j * dstStride;

    for (i = 0; i < width; i++)
    {
      int offset = i * 4;

      if (PreAlpha)
      {
        line_data[offset + 0] = (rgba_line[offset + 2] * rgba_line[offset + 3]) / 255;  /* Blue */
        line_data[offset + 1] = (rgba_line[offset + 1] * rgba_line[offset + 3]) / 255;  /* Green */
        line_data[offset + 2] = (rgba_line[offset + 0] * rgba_line[offset + 3]) / 255;  /* Red */
      }
      else
      {
        line_data[offset + 0] = rgba_line[offset + 2];  /* Blue */
        line_data[offset + 1] = rgba_line[offset + 1];  /* Green */
        line_data[offset + 2] = rgba_line[offset + 0];  /* Red */
      }

      line_data[offset + 3] = rgba_line[offset + 3];  /* Alpha */
    }
  }
}

static void
wdBufferBGRA2Bitmap(BYTE* Scan0, INT dstStride, INT srcStride, UINT width, UINT height, const BYTE* bgra)
{
  UINT i, j;

  for (j = 0; j < height; j++)
  {
    UINT line_offset = (height - 1 - j) * srcStride;  /* source is bottom-up */
    const BYTE* bgra_line = bgra + line_offset;
    BYTE* line_data = Scan0 + j * dstStride;

    for (i = 0; i < width; i++)
    {
      int offset = i * 4;
      line_data[offset + 0] = bgra_line[offset + 0];  /* Blue */
      line_data[offset + 1] = bgra_line[offset + 1];  /* Green */
      line_data[offset + 2] = bgra_line[offset + 2];  /* Red */
      line_data[offset + 3] = bgra_line[offset + 3];  /* Alpha */
    }
  }
}

static void 
wdBufferMap2Bitmap(BYTE* Scan0, INT dstStride, INT srcStride, UINT channels, UINT width, UINT height, const BYTE* map, const COLORREF* palette, UINT uPaletteSize)
{
  UINT i, j;

  for (j = 0; j < height; j++)
  {
    UINT line_offset = j * srcStride;
    const BYTE* map_line = map + line_offset;
    BYTE* line_data = Scan0 + j * dstStride;

    for (i = 0; i < width; i++)
    {
      int map_index = map_line[i];
      COLORREF color = palette[map_index];

      int offset_data = i * channels;
      line_data[offset_data + 0] = GetBValue(color);
      line_data[offset_data + 1] = GetGValue(color);
      line_data[offset_data + 2] = GetRValue(color);

      if (channels == 4)
        line_data[offset_data + 3] = 255;
    }
  }

  (void)uPaletteSize; /* unused */
}

WD_HIMAGE wdCreateImageFromBuffer(UINT uWidth, UINT uHeight, UINT srcStride, const BYTE* pBuffer,
                                  int pixelFormat, const COLORREF* cPalette, UINT uPaletteSize)
{
    if (d2d_enabled()) {
        IWICBitmap* bitmap = NULL;
        HRESULT hr;
        WICRect rect = { 0, 0, uWidth, uHeight};
        IWICBitmapLock *bitmap_lock = NULL;
        UINT cbBufferSize = 0;
        UINT dstStride = 0;
        BYTE *Scan0 = NULL;

        if(wic_factory == NULL) {
            WD_TRACE("wdCreateImageFromBuffer: Image API disabled.");
            return NULL;
        }

        hr = IWICImagingFactory_CreateBitmap(wic_factory, uWidth, uHeight, &wic_pixel_format, WICBitmapCacheOnDemand, &bitmap);   /* GUID_WICPixelFormat32bppPBGRA - pre-multiplied alpha, BGRA order */
        if(FAILED(hr)) {
            WD_TRACE_HR("wdCreateImageFromBuffer: "
                        "IWICImagingFactory::CreateBitmap() failed.");
            return NULL;
        }

        hr = IWICBitmap_Lock(bitmap, &rect, WICBitmapLockWrite, &bitmap_lock);
        if (FAILED(hr)) {
            WD_TRACE_HR("wdCreateImageFromBuffer: "
                        "IWICBitmap::Lock() failed.");
            IWICBitmap_Release(bitmap);
            return NULL;
        }

        IWICBitmapLock_GetStride(bitmap_lock, &dstStride);
        IWICBitmapLock_GetDataPointer(bitmap_lock, &cbBufferSize, &Scan0);

        if (pixelFormat == WD_PIXELFORMAT_PALETTE) {
            if (srcStride == 0) srcStride = uWidth * 1;
            wdBufferMap2Bitmap(Scan0, dstStride, srcStride, 4, uWidth, uHeight, pBuffer, cPalette, uPaletteSize);
        }
        else
        {
          if (pixelFormat == WD_PIXELFORMAT_R8G8B8) {
              if (srcStride == 0) srcStride = uWidth * 3;
              wdBufferRGB2Bitmap(Scan0, dstStride, srcStride, 4, uWidth, uHeight, pBuffer);
          }
          else if (pixelFormat == WD_PIXELFORMAT_R8G8B8A8) {
              if (srcStride == 0) srcStride = uWidth * 4;
              wdBufferRGBA2Bitmap(Scan0, dstStride, srcStride, TRUE, uWidth, uHeight, pBuffer);
          }
          else {
              if (srcStride == 0) srcStride = uWidth * 4;
              wdBufferBGRA2Bitmap(Scan0, dstStride, srcStride, uWidth, uHeight, pBuffer);
          }
        }


        IWICBitmapLock_Release(bitmap_lock);
        return (WD_HIMAGE)bitmap;
    }
    else {
        dummy_GpPixelFormat format;
        int status;
        dummy_GpBitmap *bitmap = NULL;
        dummy_GpBitmapData bitmapData;
        dummy_GpRectI rect = { 0, 0, uWidth, uHeight };

        if (pixelFormat == WD_PIXELFORMAT_R8G8B8 || pixelFormat == WD_PIXELFORMAT_PALETTE)
            format = dummy_PixelFormat24bppRGB;
        else if (pixelFormat == WD_PIXELFORMAT_R8G8B8A8)
            format = dummy_PixelFormat32bppARGB;
        else
            format = dummy_PixelFormat32bppPARGB;

        status = gdix_vtable->fn_CreateBitmapFromScan0(uWidth, uHeight, 0, format, NULL, &bitmap);
        if(status != 0) {
            WD_TRACE("wdCreateImageFromBuffer: "
                     "GdipCreateBitmapFromScan0() failed. [%d]", status);
            return NULL;
        }

        gdix_vtable->fn_BitmapLockBits(bitmap, &rect, dummy_ImageLockModeWrite, format, &bitmapData);

        if (pixelFormat == WD_PIXELFORMAT_PALETTE) {
            if (srcStride == 0) srcStride = uWidth * 1;
            wdBufferMap2Bitmap((BYTE*)bitmapData.Scan0, bitmapData.Stride, srcStride, 3, uWidth, uHeight, pBuffer, cPalette, uPaletteSize);
        }
        else
        {
            if (pixelFormat == WD_PIXELFORMAT_R8G8B8) {
                if (srcStride == 0) srcStride = uWidth * 3;
                wdBufferRGB2Bitmap((BYTE*)bitmapData.Scan0, bitmapData.Stride, srcStride, 3, uWidth, uHeight, pBuffer);
            }
            else if (pixelFormat == WD_PIXELFORMAT_R8G8B8A8) {
                if (srcStride == 0) srcStride = uWidth * 4;
                wdBufferRGBA2Bitmap((BYTE*)bitmapData.Scan0, bitmapData.Stride, srcStride, FALSE, uWidth, uHeight, pBuffer);
            }
            else {
                if (srcStride == 0) srcStride = uWidth * 4;
                wdBufferBGRA2Bitmap((BYTE*)bitmapData.Scan0, bitmapData.Stride, srcStride, uWidth, uHeight, pBuffer);
            }
        }

        gdix_vtable->fn_BitmapUnlockBits(bitmap, &bitmapData);

        return (WD_HIMAGE)bitmap;
    }
}
