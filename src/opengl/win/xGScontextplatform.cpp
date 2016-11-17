/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    opengl/win/xGScontextplatform.cpp
        xGScontext class implementation
*/

#include "xGScontextplatform.h"
#include "xGSdefaultcontext.h"
#include "GL/wglew.h"


using namespace xGS;


xGScontext::xGScontext() :
    xGScontextBase(),
    p_pixelformatWGL(false),
    p_multisample(false),
    p_context(0)
{
#ifdef _DEBUG
    OutputDebugStringA("context WGL created\n");
#endif
}

xGScontext::~xGScontext()
{
#ifdef _DEBUG
    OutputDebugStringA("context WGL destroyed\n");
#endif
}

GSerror xGScontext::Initialize()
{
    // default context
    xGSdefaultcontext dc;
    if (!dc.CreateDefaultContext()) {
        return GSE_STARTUP_SUBSYSTEMFAILED;
    }

    if (glewInit() != GLEW_OK) {
        return GSE_STARTUP_INITFAILED;
    }

    // pfd search
    p_pixelformatWGL = WGLEW_ARB_pixel_format != 0;
    p_multisample = WGLEW_ARB_multisample != 0;
    p_srgb = WGLEW_ARB_framebuffer_sRGB != 0 || WGLEW_EXT_framebuffer_sRGB != 0;

    // WGL pfd search
    if (!EnumWGLPixelFormats(dc)) {
        return GSE_STARTUP_SUBSYSTEMFAILED;
    }

    // search standard pfds
    if (!EnumDefaultPixelFormats(dc)) {
        return GSE_STARTUP_SUBSYSTEMFAILED;
    }

    dc.DestroyDefaultContext();

    return GS_OK;
}

static inline int SetAttribute(int &attr, int *pfa, GLenum attrname, GLenum attrvalue)
{
    pfa[attr++] = attrname;
    pfa[attr] = attrvalue;
    return attr++;
}

static int color_bits(GSenum format)
{
    switch (format) {
        case GS_COLOR_DEFAULT:        return 32;
        case GS_COLOR_RGBX:           return 32;
        case GS_COLOR_RGBX_HALFFLOAT: return 64;
        case GS_COLOR_RGBX_FLOAT:     return 128;
        case GS_COLOR_RGBA:           return 32;
        case GS_COLOR_RGBA_HALFFLOAT: return 64;
        case GS_COLOR_RGBA_FLOAT:     return 128;
        case GS_COLOR_S_RGBX:         return 32;
        case GS_COLOR_S_RGBA:         return 32;
    }

    return 0;
}

static int alpha_bits(GSenum format)
{
    switch (format) {
        case GS_COLOR_RGBA:           return 8;
        case GS_COLOR_RGBA_HALFFLOAT: return 16;
        case GS_COLOR_RGBA_FLOAT:     return 32;
        case GS_COLOR_S_RGBA:         return 8;
    }

    return 0;
}

static int depth_bits(GSenum format)
{
    switch (format) {
        case GS_DEPTH_DEFAULT:  return 32;
        case GS_DEPTH_16:       return 16;
        case GS_DEPTH_24:       return 24;
        case GS_DEPTH_32:       return 32;
        case GS_DEPTH_32_FLOAT: return 32;
    }

    return 0;
}

static int stencil_bits(GSenum format)
{
    switch (format) {
        case GS_STENCIL_DEFAULT:    return 0;
        case GS_STENCIL_8:          return 8;
        case GS_DEPTHSTENCIL_D24S8: return 8;
    }

    return 0;
}

GSerror xGScontext::CreateRenderer(const GSrendererdescription &desc)
{
    if (p_pixelformatWGL) {
        xGSdefaultcontext dc;
        if (!dc.CreateDefaultContext()) {
            return GSE_SUBSYSTEMFAILED;
        }

        if (glewInit() != GLEW_OK) {
            return GSE_SUBSYSTEMFAILED;
        }

        int attr = 0;
        int pfa[64];

        SetAttribute(attr, pfa, WGL_DRAW_TO_WINDOW_ARB, 1);
        SetAttribute(attr, pfa, WGL_SUPPORT_OPENGL_ARB, 1);
        SetAttribute(attr, pfa, WGL_DOUBLE_BUFFER_ARB,  1);
        SetAttribute(attr, pfa, WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB);
        SetAttribute(attr, pfa, WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB);

        int alphabitsrequested = alpha_bits(desc.colorformat);
        int stencilbitsrequested = clamp(
            stencil_bits(desc.stencilformat),
            0,
            maxBitsSupport(p_stencilbitssupport)
        );

        SetAttribute(
            attr, pfa, WGL_COLOR_BITS_ARB,
            clamp(color_bits(desc.colorformat), 0, maxBitsSupport(p_colorbitssupport))
        );
        SetAttribute(attr, pfa, WGL_ALPHA_BITS_ARB, alphabitsrequested);

        int value_depthbits = SetAttribute(
            attr, pfa, WGL_DEPTH_BITS_ARB,
            clamp(depth_bits(desc.depthformat), 0, maxBitsSupport(p_depthbitssupport))
        );

        int value_stencilbits =
            SetAttribute(attr, pfa, WGL_STENCIL_BITS_ARB, stencilbitsrequested);

        int value_multisample = -1;
        if (desc.multisample && p_multisample) {
            SetAttribute(attr, pfa, WGL_SAMPLE_BUFFERS_ARB, desc.multisample ? 1 : 0);
            value_multisample = SetAttribute(
                attr, pfa, WGL_SAMPLES_ARB,
                desc.multisample == GS_DEFAULT ?
                    p_multisamplemax :
                    clamp(desc.multisample, 1, p_multisamplemax)
            );
        }

        bool sRGBrequested =
            desc.colorformat == GS_COLOR_S_RGBA ||
            desc.colorformat == GS_COLOR_S_RGBX;

        if (sRGBrequested && p_srgb) {
            SetAttribute(attr, pfa, WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, 1);
        }

        SetAttribute(attr, pfa, 0, 0);

        GSuint n = 0;
        int depthbits = pfa[value_depthbits];
        while (n == 0) {
            pfa[value_depthbits] = depthbits;
            while (n == 0) {
                wglChoosePixelFormatARB(dc.getDevice(), pfa, nullptr, 1, &p_devicepf, &n);
                if (n == 0 && pfa[value_depthbits] > 16) {
                    pfa[value_depthbits] -= 8; // reduce Z-buffer bit depth
                } else {
                    break;
                }
            }

            // if pf still couldn't be found, try to turn off
            // other features...
            if (n == 0) {
                if (value_multisample != -1 && pfa[value_multisample] > 1) {
                    --pfa[value_multisample];
                } else if (pfa[value_stencilbits] > 0) {
                    pfa[value_stencilbits] = 0;
                } else {
                    break;
                }
            }
        }

        if (n == 0) {
            return GSE_INCOMPATIBLE;
        }

        attr = 0;
        pfa[attr++] = WGL_ACCELERATION_ARB;
        pfa[attr++] = WGL_COLOR_BITS_ARB;
        if (alphabitsrequested) {
            pfa[attr++] = WGL_ALPHA_BITS_ARB;
        }
        pfa[attr++] = WGL_DEPTH_BITS_ARB;
        if (stencilbitsrequested) {
            pfa[attr++] = WGL_STENCIL_BITS_ARB;
        }
        if (p_multisample) {
            pfa[attr++] = WGL_SAMPLES_ARB;
        }
        if (p_srgb) {
            pfa[attr++] = WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB;
        }
        wglGetPixelFormatAttribivARB(dc.getDevice(), p_devicepf, 0, attr, pfa, pfa);
        if (pfa[0] != WGL_FULL_ACCELERATION_ARB) {
            return GSE_INCOMPATIBLE;
        }

        dc.DestroyDefaultContext();

        p_defaultwidget = xGSdefaultcontext::CreateDefaultWidget();
        if (p_defaultwidget == 0) {
            return GSE_WINDOWSYSTEMFAILED;
        }

        p_defaultdevice = GetDC(p_defaultwidget);
        if (p_defaultdevice == 0) {
            CleanUp();
            return GSE_WINDOWSYSTEMFAILED;
        }

        DescribePixelFormat(p_defaultdevice, p_devicepf, sizeof p_devicepfd, &p_devicepfd);

        attr = 1;
        p_rtformat.pfColorBits = pfa[attr++];
        p_rtformat.pfAlphaBits = alphabitsrequested ? pfa[attr++] : 0;
        p_rtformat.pfDepthBits = pfa[attr++];
        p_rtformat.pfStencilBits = stencilbitsrequested ? pfa[attr++] : 0;
        p_rtformat.pfMultisample = p_multisample ? pfa[attr++] : 0;
        p_rtformat.pfSRGB = p_srgb && sRGBrequested ? pfa[attr++] != 0 : GS_FALSE;
    } else {

        // TODO: non WGL ext. implementation

    }

    // widget set-up
    if (desc.widget) {
        p_renderwidget = HWND(desc.widget);

        p_renderdevice = GetDC(p_renderwidget);
        if (p_renderdevice == 0) {
            CleanUp();
            return GSE_WINDOWSYSTEMFAILED;
        }

        int n = GetPixelFormat(p_renderdevice);
        if ((n != 0) && (n != p_devicepf)) {
            CleanUp();
            return GSE_WINDOWSYSTEMFAILED;
        }

        if ((n == 0) && !SetPixelFormat(p_renderdevice, p_devicepf, &p_devicepfd)) {
            CleanUp();
            return GSE_WINDOWSYSTEMFAILED;
        }

        // assert p_context == 0
        p_context = CreateContext(p_renderdevice);
        if (p_context == 0) {
            CleanUp();
            return GSE_SUBSYSTEMFAILED;
        }

        if (!wglMakeCurrent(p_renderdevice, p_context)) {
            CleanUp();
            return GSE_SUBSYSTEMFAILED;
        }
    } else {
        if (!SetPixelFormat(p_defaultdevice, p_devicepf, &p_devicepfd)) {
            CleanUp();
            return GSE_WINDOWSYSTEMFAILED;
        }

        p_context = CreateContext(p_defaultdevice);
        if (p_context == 0) {
            CleanUp();
            return GSE_SUBSYSTEMFAILED;
        }

        if (!wglMakeCurrent(p_defaultdevice, p_context)) {
            CleanUp();
            return GSE_SUBSYSTEMFAILED;
        }
    }

    //if (glewInit() != GLEW_OK) {
    //    return GSE_SUBSYSTEMFAILED;
    //}

    return GS_OK;
}

GSbool xGScontext::DestroyRenderer()
{
    CleanUp();
    ResetRTFormat();
    return GS_TRUE;
}

GSbool xGScontext::Display()
{
    //if (wglSwapIntervalEXT)
    //    wglSwapIntervalEXT(0);
    return SwapBuffers(p_renderdevice) == TRUE;
}

GSsize xGScontext::RenderTargetSize() const
{
    RECT rc;
    rc.right = 0;
    rc.bottom = 0;
    GetClientRect(p_renderwidget, &rc);

    GSsize size;
    size.width = rc.right;
    size.height = rc.bottom;

    return size;
}

enum AttributeIndex
{
    ATTR_DRAW_TO_WINDOW,
    ATTR_SUPPORT_OPENGL,
    ATTR_DOUBLE_BUFFER,
    ATTR_PIXEL_TYPE,
    ATTR_COLOR_BITS,
    ATTR_ALPHA_BITS,
    ATTR_DEPTH_BITS,
    ATTR_STENCIL_BITS,
    ATTR_ACCELERATION,
    ATTR_OPT0,
    ATTR_OPT1,
    ATTR_COUNT
};

GSbool xGScontext::EnumWGLPixelFormats(xGSdefaultcontext &dc)
{
    if (p_pixelformatWGL) {
        GLint attr = WGL_NUMBER_PIXEL_FORMATS_ARB;
        if (wglGetPixelFormatAttribivARB(dc.getDevice(), 0, 0, 1, &attr, &attr)) {
            p_pixelformatlist.reserve(attr);

            // fill in attribute list
            int ir[ATTR_COUNT];
            int na = ATTR_OPT0;
            int ia[ATTR_COUNT] = {
                WGL_DRAW_TO_WINDOW_ARB, WGL_SUPPORT_OPENGL_ARB,
                WGL_DOUBLE_BUFFER_ARB, WGL_PIXEL_TYPE_ARB,
                WGL_COLOR_BITS_ARB, WGL_ALPHA_BITS_ARB,
                WGL_DEPTH_BITS_ARB, WGL_STENCIL_BITS_ARB,
                WGL_ACCELERATION_ARB
            };

            int attr_multisample = -1;
            if (p_multisample) {
                attr_multisample = na;
                ia[na++] = WGL_SAMPLES_ARB;
            }

            int attr_srgb = -1;
            if (p_srgb) {
                attr_srgb = na;
                ia[na++] = WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB;
            }

            // retreive attribute values
            for (int n = 1; n < attr; ++n)
                if (wglGetPixelFormatAttribivARB(dc.getDevice(), n, 0, na, ia, ir) &&
                    (ir[ATTR_DRAW_TO_WINDOW] != 0) && (ir[ATTR_SUPPORT_OPENGL] != 0) &&
                    (ir[ATTR_DOUBLE_BUFFER] != 0) && (ir[ATTR_PIXEL_TYPE] == WGL_TYPE_RGBA_ARB) &&
                    (ir[ATTR_ACCELERATION] == WGL_FULL_ACCELERATION_ARB))
                {
                    GSpixelformat pf;
                    pf.pfColorBits = ir[ATTR_COLOR_BITS];
                    pf.pfAlphaBits = ir[ATTR_ALPHA_BITS];
                    pf.pfDepthBits = ir[ATTR_DEPTH_BITS];
                    pf.pfStencilBits = ir[ATTR_STENCIL_BITS];

                    setBitsSupport(p_colorbitssupport, ir[ATTR_COLOR_BITS]);
                    setBitsSupport(p_depthbitssupport, ir[ATTR_DEPTH_BITS]);
                    setBitsSupport(p_stencilbitssupport, ir[ATTR_STENCIL_BITS]);

                    if (!p_multisample) {
                        pf.pfMultisample = GS_MULTISAMPLE_NONE;
                    } else {
                        pf.pfMultisample = ir[attr_multisample];
                        switch (ir[attr_multisample]) {
                            case 2: p_multisamplesupport |= s2; break;
                            case 4: p_multisamplesupport |= s4; break;
                            case 8: p_multisamplesupport |= s8; break;
                            case 16: p_multisamplesupport |= s16; break;
                        }
                        if (pf.pfMultisample > p_multisamplemax) {
                            p_multisamplemax = pf.pfMultisample;
                        }
                    }

                    if (!p_srgb) {
                        pf.pfSRGB = false;
                    } else {
                        pf.pfSRGB = ir[attr_srgb] != 0;
                    }

                    p_pixelformatlist.push_back(pf);
                }
        } else {
            return false;
        }
    }

    return true;
}

GSbool xGScontext::EnumDefaultPixelFormats(xGSdefaultcontext &dc)
{
    if (p_pixelformatlist.size() == 0) {
        int n = 1;
        PIXELFORMATDESCRIPTOR pfd;
        while (DescribePixelFormat(dc.getDevice(), n, sizeof pfd, &pfd)) {
            if (((pfd.dwFlags & (PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER)) ==
                 (PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER)) &&
                 (pfd.iPixelType == PFD_TYPE_RGBA) && (pfd.iLayerType == PFD_MAIN_PLANE))
            {
                GSpixelformat pf;
                pf.pfColorBits = pfd.cColorBits;
                pf.pfAlphaBits = pfd.cAlphaBits;
                pf.pfDepthBits = pfd.cDepthBits;
                pf.pfStencilBits = pfd.cStencilBits;
                pf.pfMultisample = 0;
                pf.pfSRGB = false;

                setBitsSupport(p_colorbitssupport, pfd.cColorBits);
                setBitsSupport(p_depthbitssupport, pfd.cDepthBits);
                setBitsSupport(p_stencilbitssupport, pfd.cStencilBits);

                p_pixelformatlist.push_back(pf);
            }
            ++n;
        }

        if (p_pixelformatlist.size() == 0) {
            return false;
        }
    }

    return true;
}

HGLRC xGScontext::CreateContext(HDC device)
{
    if (wglCreateContextAttribsARB) {
        struct Version
        {
            int major;
            int minor;
        };

        const Version versions[] = {
            4, 0,
            3, 3,
            0
        };

        HGLRC context = 0;
        int ver = 0;
        do
        {
            GLint attribs[9] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, versions[ver].major,
                WGL_CONTEXT_MINOR_VERSION_ARB, versions[ver].minor,
                WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
#ifdef _DEBUG
                | WGL_CONTEXT_DEBUG_BIT_ARB
#endif
                ,
                0
            };

            if (!WGLEW_ARB_create_context_profile) {
                attribs[4] = 0;
            }

            context = wglCreateContextAttribsARB(device, 0, attribs);
        } while (context == 0 && versions[++ver].major != 0);

        return context;
    } else {
        return wglCreateContext(device);
    }
}

void xGScontext::CleanUp()
{
    if (p_context) {
        wglMakeCurrent(0, 0);
        wglDeleteContext(p_context);
        p_context = 0;
    }

    p_devicepf = 0;

    if (p_renderdevice) {
        ReleaseDC(p_renderwidget, p_renderdevice);
        p_renderdevice = 0;
    }

    if (p_renderwidget) {
        // external widget, can't be deleted here
        p_renderwidget = 0;
    }

    if (p_defaultdevice) {
        ReleaseDC(p_defaultwidget, p_defaultdevice);
        p_defaultdevice = 0;
    }

    if (p_defaultwidget) {
        DestroyWindow(p_defaultwidget);
        p_defaultwidget = 0;
    }
}

// default context implementation
// think about option for "unity build" and consider it here
#include "xGSdefaultcontext.cpp"
