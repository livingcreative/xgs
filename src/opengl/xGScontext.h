/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    xGScontext.h
        context object interface
            in OpenGL implementation context object handles platform specific
            OpenGL context initialization
*/

#pragma once

#include "xGS/xGS.h"
#include "xGSutil.h"


namespace xGS
{

    class xGScontext
    {
    public:
        xGScontext() :
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

        virtual ~xGScontext() {}

        virtual GSerror Initialize() = 0;
        virtual GSerror CreateRenderer(const GSrendererdescription &desc) = 0;
        virtual GSbool DestroyRenderer() = 0;

        virtual GSbool Display() = 0;

        GSuint ColorBitsSupport() const { return p_colorbitssupport; }
        GSuint DepthBitsSupport() const { return p_depthbitssupport; }
        GSuint StencilBitsSupport() const { return p_stencilbitssupport; }
        GSuint MultisampleSupport() const { return p_multisamplesupport; }
        GSbool sRGBSupport() const { return p_srgb; }

        const GSpixelformat& RenderTargetFormat() const { return p_rtformat; }
        virtual GSsize RenderTargetSize() const = 0;

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


    class xGScontextCreator
    {
    public:
        virtual xGScontext* create() = 0;
    };

} // namespace xGS
