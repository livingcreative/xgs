/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    opengl/xGScontext.h
        context object interface
            in OpenGL implementation context object handles platform specific
            OpenGL context initialization
*/

#pragma once

#include "xGS/xGS.h"
#include "xGSutil.h"


namespace xGS
{

    // this is base class for context, it has common data for any
    // OpenGL windowing subsystem
    // Every specific subsystem implementation (WGL, GLX etc.) should
    // derive its own xGScontext class from this one
    class xGScontextBase
    {
    public:
        xGScontextBase() :
            p_pixelformatlist(),
            p_colorbitssupport(0),
            p_depthbitssupport(0),
            p_stencilbitssupport(0),
            p_multisamplesupport(0),
            p_multisamplemax(0),
            p_srgb(GS_FALSE)
        {
            ResetRTFormat();
        }

        /*

        OpenGL implementation expects to see these interface
        in xGScontext class

        These functions not declared as virtual here because there's no need
        for them to be virtual, they are "constant" for any specific implementation
        and won't be changed at runtime.

        C++ and its OOP is poor for defining this kind of must have interface for
        a class, so this comment here left as a hint

        GSerror Initialize();
        GSerror CreateRenderer(const GSrendererdescription &desc);
        GSbool DestroyRenderer();
        GSbool Display();
        GSsize RenderTargetSize() const;
        */

        GSuint ColorBitsSupport() const { return p_colorbitssupport; }
        GSuint DepthBitsSupport() const { return p_depthbitssupport; }
        GSuint StencilBitsSupport() const { return p_stencilbitssupport; }
        GSuint MultisampleSupport() const { return p_multisamplesupport; }
        GSbool sRGBSupport() const { return p_srgb; }

        const GSpixelformat& RenderTargetFormat() const { return p_rtformat; }

    protected:
        void ResetRTFormat()
        {
            memset(&p_rtformat, 0, sizeof(p_rtformat));
        }

    protected:
        // this should be initialized in descendants
        std::vector<GSpixelformat>  p_pixelformatlist;
        GSuint                      p_colorbitssupport;
        GSuint                      p_depthbitssupport;
        GSuint                      p_stencilbitssupport;
        GSuint                      p_multisamplesupport;
        GSint                       p_multisamplemax;
        GSbool                      p_srgb;
        GSpixelformat               p_rtformat;
    };

} // namespace xGS
