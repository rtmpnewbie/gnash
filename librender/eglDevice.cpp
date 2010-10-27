//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <iostream>
#include <cerrno>
#include <exception>

#include "log.h"
#include "RunResources.h"
#include "Renderer.h"
#include "Renderer_ovg.h"
#include "GnashException.h"
#ifdef HAVE_GTK2
#include "gdk/gdkx.h"
#include "X11/Xlib.h"
#include "X11/Xutil.h"
#endif

#ifdef HAVE_EGL_EGL_H
# include <EGL/egl.h>
#else
# error "This file needs EGL, which is part of OpenGL-ES"
#endif

//#include <GL/gl.h>

#include "eglDevice.h"

namespace gnash {

namespace renderer {
    
static const EGLint attrib32_list[] = {
    EGL_RED_SIZE,       8,
    EGL_GREEN_SIZE,     8,
    EGL_BLUE_SIZE,      8,
    EGL_ALPHA_SIZE,     0,
#ifdef RENDERER_GLES1    
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
#endif
#ifdef RENDERER_GLES2
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
#endif
#ifdef RENDERER_OPENVG
    EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
    EGL_DEPTH_SIZE,     24,
    EGL_STENCIL_SIZE,   8,
#endif
    EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
    EGL_NONE
};

static EGLint const attrib16_list[] = {
    EGL_RED_SIZE,       5,
    EGL_GREEN_SIZE,     6,
    EGL_BLUE_SIZE,      5,
    EGL_ALPHA_SIZE,     0,
#ifdef RENDERER_GLES1
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
#endif
#ifdef RENDERER_GLES2
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
#endif
#ifdef RENDERER_OPENVG
    EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
    EGL_DEPTH_SIZE,     0,
    EGL_STENCIL_SIZE,   0,
#endif
    EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
    EGL_SAMPLE_BUFFERS, 0,
    EGL_NONE
};

const EGLint window_attrib_list[] = {
    // Back buffering is used for window and pbuffer surfaces. Windows
    // require eglSwapBuffers() to become visible, and pbuffers don't.   
    // EGL_SINGLE_BUFFER is by pixmap surfaces. With OpenVG, windows
    // can also be single buffered. eglCopyBuffers() can be used to copy
    // both back and single buffered surfaces to a pixmap.
    EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
    EGL_COLORSPACE,    EGL_COLORSPACE_sRGB,
    EGL_NONE
};

// From the EGL 1.4 spec:
//
// EGL defines several types of drawing surfaces collectively referred
// to as EGLSurfaces. These include windows, used for onscreen
// rendering; pbuffers, used for offscreen rendering; and pixmaps,
// used for offscreen rendering into buffers that may be accessed
// through native APIs. EGL windows and pixmaps are tied to native
// window system windows and pixmaps.
//
// depth, multisample, and stencil buffers are currently used only by
// OpenGL-ES.

// EGL and OpenGL ES supports two rendering models: back buffered and
// single buffered. Back buffered rendering is used by window and
// pbuffer surfaces. Memory for the color buffer used during rendering
// is allocated and owned by EGL. When the client is finished drawing
// a frame, the back buffer may be copied to a visible window using
// eglSwapBuffers. Pbuffer surfaces have a back buffer but no
// associated window, so the back buffer need not be copied.
//
// Single buffered rendering is used by pixmap surfaces. Memory for
// the color buffer is specified at surface creation time in the form
// of a native pixmap, and client APIs are required to use that memory
// during rendering. When the client is finished drawing a frame, the
// native pixmap contains the final image. Pixmap surfaces typically
// do not support multisampling, since the native pixmap used as the
// color buffer is unlikely to provide space to store multisample
// information. Some client APIs , such as OpenGL and OpenVG , also
// support single buffered rendering to window surfaces. This behavior
// can be selected when creating the window surface, as defined in
// section 3.5.1. When mixing use of client APIs which do not support
// single buffered rendering into windows, like OpenGL ES , with
// client APIs which do support it, back color buffers and visible
// window contents must be kept consistent when binding window
// surfaces to contexts for each API type. Both back and single
// buffered surfaces may also be copied to a specified native pixmap
// using eglCopyBuffers.

// Native rendering will always be supported by pixmap surfaces (to
// the extent that native rendering APIs can draw to native
// pixmaps). Pixmap surfaces are typically used when mixing native and
// client API rendering is desirable, since there is no need to move
// data between the back buffer visible to the client APIs and the
// native pixmap visible to native rendering APIs. However, pixmap
// surfaces may, for the same reason, have restricted capabilities and
// performance relative to window and pbuffer surfaces.

// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();

EGLDevice::EGLDevice()
    : _eglConfig(0),
      _eglContext(EGL_NO_CONTEXT),
      _eglSurface(EGL_NO_SURFACE),
      _eglDisplay(EGL_NO_DISPLAY),
      _eglNumOfConfigs(0),
      _max_num_config(1),
      _bpp(32),
      _width(0),
      _height(0)
{
    GNASH_REPORT_FUNCTION;
}

EGLDevice::~EGLDevice()
{
    GNASH_REPORT_FUNCTION;

    if (_eglDisplay != EGL_NO_DISPLAY) {  
        eglMakeCurrent(_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        
        if (_eglContext != EGL_NO_CONTEXT)
            eglDestroyContext(_eglDisplay, _eglContext);
        
        if (_eglSurface != EGL_NO_SURFACE)
            eglDestroySurface(_eglDisplay, _eglSurface);
        
        // if (_eglwin_native)
        //     free(_eglwin_native);
        
        eglTerminate(_eglDisplay);
    }
    
#ifdef HAVE_GTK2
    gdk_exit (0);
#endif
}

bool
EGLDevice::init(EGLDevice::rtype_t rtype)
{
    GNASH_REPORT_FUNCTION;

    // // FIXME: for now, always run verbose till this supports command line args
//    dbglogfile.setVerbosity();

#ifdef HAVE_GTK2
    // As gdk_init() wants the command line arguments, we have to create
    // fake ones, as we don't care about the X11 options at this point.
    int argc = 0;
    char **argv = 0;
    gdk_init(&argc, &argv);
#endif

    EGLint major, minor;
    // see egl_config.c for a list of supported configs, this looks for
    // a 5650 (rgba) config, supporting OpenGL ES and windowed surfaces

    // step 1 - get an EGL display

    // This can be called multiple times, and always returns the same display
    _eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_NO_DISPLAY == _eglDisplay) {
        log_error( "eglGetDisplay() failed (error 0x%x)", eglGetError() );
        return false;
    }
    
    // This can be called multiple times safely
    if (EGL_FALSE == eglInitialize(_eglDisplay, &major, &minor)) {
        log_error( "eglInitialize() failed (error %s)",
                   getErrorString(eglGetError()));
        return false;
    }
    log_debug("EGL_CLIENT_APIS = %s", eglQueryString(_eglDisplay, EGL_CLIENT_APIS));
    log_debug("EGL_EXTENSIONS = %s",  eglQueryString(_eglDisplay, EGL_EXTENSIONS));
    log_debug("EGL_VERSION = %s, EGL_VENDOR = %s",
              eglQueryString(_eglDisplay, EGL_VERSION),
              eglQueryString(_eglDisplay, EGL_VENDOR));

    // step2 - bind to the wanted client API
    switch (rtype) {
      case OPENVG:
          log_debug("Initializing EGL for OpenVG");
          if(EGL_FALSE == eglBindAPI(EGL_OPENVG_API)) {
              log_error("eglBindAPI() failed to retrive the number of configs (error %s)",
                        getErrorString(eglGetError()));
              return false;
          }
          break;
      case OPENGLES1:
          log_debug("Initializing EGL for OpenGLES1");
          if(EGL_FALSE == eglBindAPI(EGL_OPENGL_ES_API)) {
              log_error("eglBindAPI() failed to retrive the number of configs (error %s)",
                        getErrorString(eglGetError()));
              return false;
          }
          break;
      case OPENGLES2:
          log_debug("Initializing EGL for OpenGLES2");
          if(EGL_FALSE == eglBindAPI(EGL_OPENGL_ES_API)) {
              log_error("eglBindAPI() failed to retrive the number of configs (error %s)",
                        getErrorString(eglGetError()));
              return false;
          }
          break;
      default:
          log_error("No EGL device type specified!");
          return false;
    }    

//    queryEGLConfig(_eglDisplay);
    
    // step3 - find a suitable config
    if (_bpp == 32) {
        if (EGL_FALSE == eglChooseConfig(_eglDisplay, attrib32_list, &_eglConfig,
                                          1, &_eglNumOfConfigs)) {
            log_error("eglChooseConfig() failed (error %s)", 
                       getErrorString(eglGetError()));
            return false;
        }
    } else if (_bpp == 16) {
        if (EGL_FALSE == eglChooseConfig(_eglDisplay, attrib16_list, &_eglConfig,
                                         1, &_eglNumOfConfigs)) {
            log_error("eglChooseConfig() failed (error %s)",
                       getErrorString(eglGetError()));
            return false;
        }
    } else {
        log_error("No supported bpp value!");
    }

    if (0 == _eglNumOfConfigs) {
        log_error("eglChooseConfig() was unable to find a suitable config");
        return false;
    }

    EGLint vid;
    if (!eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_NATIVE_VISUAL_ID, &vid)) {
        log_error("eglGetConfigAttrib() failed (error %s)",
                  getErrorString(eglGetError()));
        return false;
    }

#ifdef HAVE_GTK2
    Display *X11Display = XOpenDisplay(0);
    XVisualInfo *visInfo, visTemplate;
    int num_visuals;
    // The X window visual must match the EGL config
   visTemplate.visualid = vid;
   visInfo = XGetVisualInfo(X11Display, VisualIDMask, &visTemplate, &num_visuals);
   if (!visInfo) {
       log_error("couldn't get X visual");
       return false;
   }
   XFree(visInfo);
#endif
    
    // printEGLConfig(_eglConfig);
    
   if (!checkEGLConfig(_eglConfig)) {
       log_error("EGL configuration doesn't match!");
//       return false;
   } else {
       //printEGLConfig(_eglConfig);
   }

#if HAVE_GTK2
   _nativeWindow = gdk_x11_get_default_root_xwindow();
#endif

    // step4 - create a window surface
    switch (rtype) {
      case OPENVG:
          log_debug("Initializing EGL Surface for OpenVG");
          if (_nativeWindow) {
              _eglSurface = eglCreateWindowSurface(_eglDisplay, _eglConfig,
                                                   _nativeWindow, 0); // was window_attrib_list
          } else {
              log_error("No native window!");
              return false;
          }
          break;
      case OPENGLES1:
      case OPENGLES2:
          log_debug("Initializing EGL Surface for OpenGLES");
          _eglSurface = eglCreateWindowSurface(_eglDisplay, &_eglConfig, _nativeWindow, NULL);
          break;
      default:
          log_error("No EGL device type specified!");
          return false;
    }
    
    
    if (EGL_NO_SURFACE == _eglSurface) {
        log_error("eglCreateWindowSurface failed (error %s)", 
                  getErrorString(eglGetError()));
        return false;
    } else {
        //printEGLSurface(_eglSurface);
    }

    // step5 - create a context
    _eglContext = eglCreateContext(_eglDisplay, _eglConfig, EGL_NO_CONTEXT, NULL);
    if (EGL_NO_CONTEXT == _eglContext) {
        log_error("eglCreateContext failed (error %s)",
                   getErrorString(eglGetError()));
        return false;
    } else {
        printEGLContext(_eglContext);
    }
    
    // step6 - make the context and surface current
    if (EGL_FALSE == eglMakeCurrent(_eglDisplay, _eglSurface, _eglSurface, _eglContext)) {
        log_error("eglMakeCurrent failed (error %s)",
                  getErrorString(eglGetError()));
        return false;
    }       // begin user code

#if 0
#if 0
    eglSwapInterval(_eglDisplay, 0);
#else
    eglSwapBuffers(_eglDisplay, _eglSurface);
#endif

//    log_debug("Gnash EGL Frame width %d height %d bpp %d \n", _width, _height, _bpp);
#endif
    
    return true;
}

const char *
EGLDevice::getErrorString(int error)
{
    switch (error) {
    case EGL_SUCCESS:
        return "EGL_SUCCESS";
    case EGL_NOT_INITIALIZED:
        return "EGL_NOT_INITIALIZED";
    case EGL_BAD_ACCESS:
        return "EGL_BAD_ACCESS";
    case EGL_BAD_ALLOC:
        return "EGL_BAD_ALLOC";
    case EGL_BAD_ATTRIBUTE:
        return "EGL_BAD_ATTRIBUTE";
    case EGL_BAD_CONFIG:
        return "EGL_BAD_CONFIG";
    case EGL_BAD_CONTEXT:
        return "EGL_BAD_CONTEXT";
    case EGL_BAD_CURRENT_SURFACE:
        return "EGL_BAD_CURRENT_SURFACE";
    case EGL_BAD_DISPLAY:
        return "EGL_BAD_DISPLAY";
    case EGL_BAD_MATCH:
        return "EGL_BAD_MATCH";
    case EGL_BAD_NATIVE_PIXMAP:
        return "EGL_BAD_NATIVE_PIXMAP";
    case EGL_BAD_NATIVE_WINDOW:
        return "EGL_BAD_NATIVE_WINDOW";
    case EGL_BAD_PARAMETER:
        return "EGL_BAD_PARAMETER";
    case EGL_BAD_SURFACE:
        return "EGL_BAD_SURFACE";
    case EGL_CONTEXT_LOST:
        return "EGL_CONTEXT_LOST";
    default:
        return "unknown error code";
    }
}

bool
EGLDevice::checkEGLConfig(EGLConfig config)
{
    // GNASH_REPORT_FUNCTION;
    
    // Use this to explicitly check that the EGL config has the expected color depths
    EGLint value;
    if (_bpp == 32) {            
        eglGetConfigAttrib(_eglDisplay, config, EGL_RED_SIZE, &value);
        if (8 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_GREEN_SIZE, &value);
        if (8 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_BLUE_SIZE, &value);
        if (8 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_ALPHA_SIZE, &value);
        if (8 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_SAMPLES, &value);
        if (0 != value) {
            return false;
        }
    } else if (_bpp == 16) {
        eglGetConfigAttrib(_eglDisplay, config, EGL_RED_SIZE, &value);
        if ( 5 != value ) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_GREEN_SIZE, &value);
        if (6 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_BLUE_SIZE, &value);
        if (5 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_ALPHA_SIZE, &value);
        if (0 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_SAMPLES, &value);
#ifdef  RENDERER_GLES            
        if (4 != value) {
            return false;
        }
#endif
#ifdef  RENDERER_OPENVG
        if (0 != value) {
            return false;
            }
#endif
    } else {
        return false;
    }

    return true;
}

/// Query the system for all supported configs
int
EGLDevice::queryEGLConfig(EGLDisplay display)
{
     GNASH_REPORT_FUNCTION;
     EGLConfig *configs = 0;
     EGLint max_num_config = 0;

     // Get the number of supported configurations
     if ( EGL_FALSE == eglGetConfigs(display, 0, 0, &max_num_config) ) {
         log_error("eglGetConfigs() failed to retrive the number of configs (error %s)",
                   getErrorString(eglGetError()));
         return 0;
     }
     if(max_num_config <= 0) {
         printf( "No EGLconfigs found\n" );
         return 0;
     }
     log_debug("Max number of EGL Configs is %d", max_num_config);     
     
     configs = new EGLConfig[max_num_config];
     if (0 == configs) {
         log_error( "Out of memory\n" );
         return 0;
     }

     if ( EGL_FALSE == eglGetConfigs(display, configs, max_num_config, &max_num_config)) {
         log_error("eglGetConfigs() failed to retrive the configs (error %s)",
                   getErrorString(eglGetError()));
         return 0;
     }
#if 0
     for (int i=0; i<max_num_config; i++ ) {
         log_debug("Config[%d] is:", i);
         printEGLConfig(configs[i]);
     }
#endif
     
     return max_num_config;
}

void
EGLDevice::printEGLConfig(EGLConfig config)
{
    EGLint red, blue, green;
    EGLint value;
    eglGetConfigAttrib(_eglDisplay, config, EGL_RED_SIZE, &red);
    eglGetConfigAttrib(_eglDisplay, config, EGL_GREEN_SIZE, &green);
    eglGetConfigAttrib(_eglDisplay, config, EGL_BLUE_SIZE, &blue);
    std::cerr << "\tConfig has RED = " << red << ", GREEN = " << green
              << ", BLUE = " << blue  << std::endl;
    
    eglGetConfigAttrib(_eglDisplay, config, EGL_ALPHA_SIZE, &value);
    std::cerr << "\tEGL_ALPHA_SIZE is " << value  << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_STENCIL_SIZE, &value);
    std::cerr << "\tEGL_STENCIL_SIZE is " << value  << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_SAMPLES, &value);
    std::cerr << "\tEGL_SAMPLES is " << value  << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_DEPTH_SIZE, &value);
    std::cerr << "\tEGL_DEPTH_SIZE is " << value  << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_MAX_SWAP_INTERVAL, &value);
    std::cerr << "\tEGL_MAX_SWAP_INTERVAL is " << value << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_MIN_SWAP_INTERVAL, &value);
    std::cerr << "\tEGL_MIN_SWAP_INTERVAL is " << value << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_NATIVE_RENDERABLE, &value);
    std::string val = (value)? "true" : "false";
    std::cerr << "\tEGL_NATIVE_RENDERABLE is " << val << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_SAMPLE_BUFFERS, &value);
    std::cerr << "\tEGL_SAMPLE_BUFFERS is " << value << std::endl;
    eglGetConfigAttrib(_eglDisplay, config, EGL_RENDERABLE_TYPE, &value);
    if (value > 0) {
        std::string str;
        if (value & EGL_OPENGL_ES2_BIT) {
            str += " OpenGL-ES 2.0";
        }
        if (value & EGL_OPENGL_ES_BIT) {
            str += " OpenGL-ES 1.1";
        }
        if (value & EGL_OPENVG_BIT) {
            str += " OpenVG";
        }
        if (value & EGL_OPENGL_BIT) {
            str += " OpenGL";
        }
        std::cerr <<"\tEGL_RENDERABLE_TYPE = " << str << std::endl;
    } else {
        std::cerr <<"\tEGL_RENDERABLE_TYPE (default)" << std::endl;
    }
    eglGetConfigAttrib(_eglDisplay, config, EGL_SURFACE_TYPE, &value);
    if (value > 0) {
        std::string str;
        if (value & EGL_WINDOW_BIT) {
            str += " Window";
        }
        if (value & EGL_PIXMAP_BIT) {
            str += " Pixmap";
        }
        if (value & EGL_PBUFFER_BIT) {
            str += " Pbuffer";
        }
        std::cerr <<"\tEGL_SURFACE_TYPE = " << str  << std::endl;
    } else {
          std::cerr <<"\tEGL_SURFACE_TYPE (default)" << std::endl;
    }
}

void
EGLDevice::printEGLContext(EGLContext context)
{
    EGLint value;
    eglQueryContext(_eglDisplay, context, EGL_CONFIG_ID, &value);
    log_debug("Context EGL_CONFIG_ID is %d", value);
    eglQueryContext(_eglDisplay, context, EGL_CONTEXT_CLIENT_TYPE, &value);
    log_debug("\tEGL_CONTEXT_CLIENT_TYPE is %d", (value == EGL_OPENVG_API)
              ? "EGL_OPENVG_API" : "EGL_OPENGL_ES_API");
    // eglQueryContext(_eglDisplay, context, EGL_CONTEXT_CLIENT_VERSION, &value);
    // log_debug("EGL_CONTEXT_CLIENT_VERSION is %d", value);
    eglQueryContext(_eglDisplay, context, EGL_RENDER_BUFFER, &value);
    log_debug("\tEGL_RENDER_BUFFER is %s", (value == EGL_BACK_BUFFER)
              ? "EGL_BACK_BUFFER" : "EGL_SINGLE_BUFFER");
}

void
EGLDevice::printEGLSurface(EGLSurface surface)
{
    EGLint value;
    eglQuerySurface(_eglDisplay, surface, EGL_CONFIG_ID, &value);
    log_debug("Surface EGL_CONFIG_ID is %d", value);
    eglQuerySurface(_eglDisplay, surface, EGL_HEIGHT, &value);
    log_debug("\tEGL_HEIGHT is %d", value);
    eglQuerySurface(_eglDisplay, surface, EGL_WIDTH, &value);
    log_debug("\tEGL_WIDTH is %d", value);
    eglQuerySurface(_eglDisplay, surface, EGL_RENDER_BUFFER, &value);
    log_debug("\tEGL_RENDER_BUFFER is %s", (value == EGL_BACK_BUFFER)
              ? "EGL_BACK_BUFFER" : "EGL_SINGLE_BUFFER");
    eglQuerySurface(_eglDisplay, surface, EGL_VERTICAL_RESOLUTION, &value);
    log_debug("\tEGL_VERTICAL_RESOLUTION is %d", value);
    eglQuerySurface(_eglDisplay, surface, EGL_HORIZONTAL_RESOLUTION, &value);
    log_debug("\tEGL_HORIZONTAL_RESOLUTION is %d", value);
    eglQuerySurface(_eglDisplay, surface, EGL_SWAP_BEHAVIOR, &value);
    log_debug("\tEGL_SWAP_BEHAVIOR is %d", (value == EGL_BUFFER_DESTROYED)
              ? "EGL_BUFFER_DESTROYED" : "EGL_BUFFER_PRESERVED");
    eglQuerySurface(_eglDisplay, surface, EGL_MULTISAMPLE_RESOLVE, &value);
    log_debug("\tEGL_MULTISAMPLE_RESOLVE is %d", (value == EGL_MULTISAMPLE_RESOLVE_BOX)
              ? "EGL_MULTISAMPLE_RESOLVE_BOX" : "EGL_MULTISAMPLE_RESOLVE_DEFAULT");
}

} // namespace renderer
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End: