// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

#include "Geometry.h"
#include "CachedBitmap.h"
#include "GnashImage.h"
#include "Renderer.h"
#include "openvg/OpenVGRenderer.h"
#include "openvg/OpenVGBitmap.h"
#include "VG/openvg.h"

namespace gnash {

namespace renderer {

namespace openvg {

static const int NUM_STOPS = 10;

OpenVGBitmap::OpenVGBitmap(VGPaint paint)
    : _vgimage(VG_INVALID_HANDLE),
#ifdef BUILD_X11_DEVICE
      _pixel_format(VG_sARGB_8888),
      _stride(4),
#else
      _pixel_format(VG_sRGB_565),
      _stride(2),
#endif
      _vgpaint(paint)
{
    // GNASH_REPORT_FUNCTION;
}

OpenVGBitmap::OpenVGBitmap(CachedBitmap *bitmap, VGPaint vgpaint)
    : _vgimage(VG_INVALID_HANDLE),
#ifdef BUILD_X11_DEVICE
      _pixel_format(VG_sARGB_8888),
      _stride(4),
#else
      _pixel_format(VG_sRGB_565),
      _stride(2),
#endif
       _vgpaint(vgpaint)
{
    GNASH_REPORT_FUNCTION;

    // Store the reference so so it's available to createPatternBitmap()
    //    _image.reset(&im);

    // extract a reference to the image from the cached bitmap
    image::GnashImage &im = bitmap->image();

    // Create a VG image
    _vgimage = vgCreateImage(_pixel_format, im.width(), im.height(),
                             VG_IMAGE_QUALITY_FASTER);    
    if (_vgimage == VG_INVALID_HANDLE) {
        log_error("Failed to create VG image! %s",
                  Renderer_ovg::getErrorString(vgGetError()));
    }
    
    switch (im.type()) {
    case image::TYPE_RGB:
        log_debug("Image has RGB Pixel Format, Stride is %d",
                  im.stride());
        vgImageSubData(_vgimage, im.begin(), im.stride(), VG_sRGBA_8888,
                   0, 0, im.width(), im.height());
        break;
    case image::TYPE_RGBA:
        log_debug("Image has RGBA Pixel Format, Stride is %d",
                  im.stride());
        // Copy the image data into the VG image container
        vgImageSubData(_vgimage, im.begin(), im.stride(), VG_sRGBA_8888,
                   0, 0, im.width(), im.height());
        break;
    default:
        std::abort();
    }

    vgSetParameteri (_vgpaint, VG_PAINT_TYPE, VG_PAINT_TYPE_PATTERN);
    vgPaintPattern (_vgpaint, _vgimage);
}

// 
// VG_sRGB_565
// VG_sRGBA_5551
// VG_sRGBA_4444
// VG_A_8
// VG_A_4
OpenVGBitmap::OpenVGBitmap(image::GnashImage *image, VGPaint vgpaint)
    : _image(image),
      _vgimage(VG_INVALID_HANDLE),
#ifdef BUILD_X11_DEVICE
      _pixel_format(VG_sARGB_8888),
      _stride(4),
#else
      _pixel_format(VG_sRGB_565),
      _stride(2),
#endif
    _vgpaint(vgpaint)
{
    GNASH_REPORT_FUNCTION;
    
    switch (image->type()) {
    case image::TYPE_RGB:
        log_debug("Image has RGB Pixel Format, Stride is %d",
                  image->stride());
        vgImageSubData(_vgimage, image->begin(), image->stride(), VG_sRGBA_8888,
                   0, 0, image->width(), image->height());
        break;
    case image::TYPE_RGBA:
        log_debug("Image has RGBA Pixel Format, Stride is %d",
                  image->stride());
        vgImageSubData(_vgimage, image->begin(), image->stride(), VG_sRGBA_8888,
                   0, 0, image->width(), image->height());
        break;
    default:
        std::abort();
    }

    size_t width = _image->width();
    size_t height = _image->height();

    // Create a VG image, and copy the GnashImage data into it
    _vgimage = vgCreateImage(_pixel_format, width, height,
                             VG_IMAGE_QUALITY_FASTER);    
    
    vgImageSubData(_vgimage, image->begin(), image->stride(), _pixel_format,
                   0, 0, width, height);

    // vgSetParameteri(vgpaint, VG_PAINT_TYPE, VG_PAINT_TYPE_PATTERN);
    //    vgSetParameteri(vgpaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_REPEAT);
    vgPaintPattern(vgpaint, _vgimage);
    vgDrawImage(_vgimage);
    vgFlush();
    
#if 0
    _tex_size += width * height * 4;
    log_debug("Add Texture size:%d (%d x %d x %dbpp)", width * height * 4, 
              width, height, 4);
    log_debug("Current Texture size: %d", _tex_size);
#endif
} 

OpenVGBitmap::~OpenVGBitmap()
{
    // GNASH_REPORT_FUNCTION;
    
#if 0
    _tex_size -= _image->width() * _image->height() * 4;
    log_debug(_("Remove Texture size:%d (%d x %d x %dbpp)"),
              _image->width() * _image->height() * 4,
              _image->width(), _image->height(), 4);
    log_debug(_("Current Texture size: %d"), _tex_size);
#endif
    
    vgDestroyPaint(_vgpaint);
    vgDestroyImage(_vgimage);
}

image::GnashImage&
OpenVGBitmap::image()
{
    GNASH_REPORT_FUNCTION;
    if (_image) {
        return *_image;
    }
}    

/// OpenVG supports creating linear and gradient fills in hardware, so
/// we want to use that instead of the existing way of calculating the
/// gradient in software.

// A Radial Gradient Bitmap uses two points and a radius in the paint
// coordinate system. The gradient starts at x0,y0 as the center, and
// x1,y1 is the focal point that is forced to be in the circle.
OpenVGBitmap *
OpenVGBitmap::createRadialBitmap(float cx, float cy, float fx, float fy,
                                 float radial, const rgba &incolor,
                                 VGPaint paint)
{
    GNASH_REPORT_FUNCTION;

    VGfloat rgParams[] = { cx, cy, fx, fy, radial };
    
    // Paint Type 
    vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_RADIAL_GRADIENT);

    // Gradient Parameters
    vgSetParameterfv(paint, VG_PAINT_RADIAL_GRADIENT, 5, rgParams);

    VGfloat rampStop[] = {0.00f, 1.0f, 1.0f, 1.0f, 1.0f,
                          0.33f, 1.0f, 0.0f, 0.0f, 1.0f,
                          0.66f, 0.0f, 1.0f, 0.0f, 1.0f,
                          1.00f, 0.0f, 0.0f,  1.0f, 1.0f};
    vgSetParameterfv(paint, VG_PAINT_COLOR_RAMP_STOPS, 20, rampStop);    

    // Color Ramp is the same as for linear gradient
    return this;
}

// A Linear Gradient Bitmap uses two points, x0,y0 and x1,y1 in the paint
// coordinate system. The gradient starts at x0,y0 and goes to x1,y1. If
// x1 and y1 are outside the boundaries of the shape, then the gradient gets
// clipped at the boundary instead of x1,y1.
OpenVGBitmap *
OpenVGBitmap::createLinearBitmap(float x0, float y0, float x1, float y1,
                                 const rgba &incolor, const VGPaint paint)
{
    // GNASH_REPORT_FUNCTION;
    VGfloat color[] = {
        incolor.m_r / 255.0f,
        incolor.m_g / 255.0f,
        incolor.m_b / 255.0f,
        incolor.m_a / 255.0f
    };
    vgSetParameteri (paint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
    vgSetParameterfv (paint, VG_PAINT_COLOR, 4, color);

    vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);
    vgSetParameteri(paint, VG_PAINT_COLOR_RAMP_SPREAD_MODE,
                    VG_COLOR_RAMP_SPREAD_PAD);

    //    VGfloat linearGradient[4] = { x0, y0, 10000, 10000 };
    VGfloat linearGradient[4] = { x0, y0, x1, y1 };
    vgSetParameterfv(paint, VG_PAINT_LINEAR_GRADIENT, 4, linearGradient);

#if 0
    VGfloat stops[] = { 0.0, 0.33, 0.66, 1.0};
    vgSetParameterfv(paint, VG_PAINT_COLOR_RAMP_STOPS, 4, stops);
#else
    VGfloat rampStop[] = {0.00f, 1.0f, 1.0f, 1.0f, 1.0f,
                          0.33f, 1.0f, 0.0f, 0.0f, 1.0f,
                          0.66f, 0.0f, 1.0f, 0.0f, 1.0f,
                          1.00f, 0.0f, 0.0f,  1.0f, 1.0f};
    vgSetParameterfv(paint, VG_PAINT_COLOR_RAMP_STOPS, 20, rampStop);    
#endif
    
    return this;
}

    // Create and fill pattern image
OpenVGBitmap *
OpenVGBitmap::createPatternBitmap(CachedBitmap *bitmap, const gnash::SWFMatrix& matrix,
                                  bitmap_wrap_mode mode)
{
    GNASH_REPORT_FUNCTION;

    // extract a reference to the image from the cached bitmap
    image::GnashImage &im = bitmap->image();

    // Create a VG image
    _vgimage = vgCreateImage(_pixel_format, im.width(), im.height(),
                             VG_IMAGE_QUALITY_FASTER);    
    if (_vgimage == VG_INVALID_HANDLE) {
        log_error("Failed to create VG image! %s",
                  Renderer_ovg::getErrorString(vgGetError()));
    }
    
    switch (im.type()) {
    case image::TYPE_RGB:
        log_debug("Image has RGB Pixel Format, Stride is %d",
                  im.stride());
        vgImageSubData(_vgimage, im.begin(), im.stride(), VG_sRGBA_8888,
                   0, 0, im.width(), im.height());
        break;
    case image::TYPE_RGBA:
        log_debug("Image has RGBA Pixel Format, Stride is %d",
                  im.stride());
        // Copy the image data into the VG image container
        vgImageSubData(_vgimage, im.begin(), im.stride(), VG_sRGBA_8888,
                   0, 0, im.width(), im.height());
        break;
    default:
        log_error("");
        return 0;
    }

    vgSetParameteri (_vgpaint, VG_PAINT_TYPE, VG_PAINT_TYPE_PATTERN);
//    vgPaintPattern (_vgpaint, _vgimage);

    gnash::SWFMatrix mat;
    VGfloat     vmat[9];
    
    // Paint the cached VG image into the VG paint surface
    mat = matrix;
    mat.invert();
//    Renderer_ovg::printVGMatrix(mat);
    
    memset(vmat, 0, sizeof(vmat));
    vmat[0] = mat.sx  / 65536.0f;
    vmat[1] = mat.shx / 65536.0f;
    vmat[3] = mat.shy / 65536.0f;
    vmat[4] = mat.sy  / 65536.0f;
    vmat[6] = mat.tx;
    vmat[7] = mat.ty;
    
    Renderer_ovg::printVGMatrix(vmat);
    
    vgSeti (VG_MATRIX_MODE, VG_MATRIX_FILL_PAINT_TO_USER);
    vgLoadMatrix (vmat);
    vgSeti (VG_MATRIX_MODE, VG_MATRIX_STROKE_PAINT_TO_USER);
    vgLoadMatrix (vmat);
    vgSeti (VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    
    switch (mode) {
      case WRAP_FILL:
          vgSetParameteri (_vgpaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_FILL);
          break;
      case WRAP_PAD:
          vgSetParameteri (_vgpaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_PAD);
          break;
      case WRAP_REPEAT:
          vgSetParameteri (_vgpaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_REPEAT);
          break;
      case WRAP_REFLECT:
          vgSetParameteri (_vgpaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_REFLECT);
          break;
      default:
          log_error("No supported wrap mode specified!");
          break;
    }

    vgPaintPattern(_vgpaint, _vgimage);
//    vgDrawImage(_vgimage);
//    vgFlush();

    return this;
}

} // namespace gnash::renderer::openvg
} // namespace gnash::renderer
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End: