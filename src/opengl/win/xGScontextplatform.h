/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    opengl/win/xGScontextplatform.h
        xGScontextWGL - Windows OpenGL context implementation
*/

#pragma once

#include "xGScontext.h"
#include "xGSdefaultcontext.h"


namespace xGS
{

    class xGScontextWGL : public xGScontext
    {
    public:
        xGScontextWGL();
        ~xGScontextWGL() override;

        GSerror Initialize() override;
        GSerror CreateRenderer(const GSrendererdescription &desc) override;
        GSbool DestroyRenderer() override;

        GSbool Display() override;

        GSsize RenderTargetSize() const override;

    private:
        void setBitsSupport(GSuint &bitflags, int bits)
        {
            switch (bits) {
                case 2: bitflags |= b2; break;
                case 4: bitflags |= b4; break;
                case 8: bitflags |= b8; break;
                case 16: bitflags |= b16; break;
                case 24: bitflags |= b24; break;
                case 32: bitflags |= b32; break;
                case 64: bitflags |= b64; break;
                case 128: bitflags |= b128; break;
            }
        }

        GSbool EnumWGLPixelFormats(xGSdefaultcontext &dc);
        GSbool EnumDefaultPixelFormats(xGSdefaultcontext &dc);
        HGLRC CreateContext(HDC device);

        void CleanUp();

    private:
        GSbool                p_pixelformatWGL;
        GSbool                p_multisample;

        GSint                 p_devicepf;
        PIXELFORMATDESCRIPTOR p_devicepfd;
        HWND                  p_defaultwidget;
        HDC                   p_defaultdevice;
        HWND                  p_renderwidget;
        HDC                   p_renderdevice;
        HGLRC                 p_context;
    };


    class xGScontextCreatorPlatform : public xGScontextCreator
    {
    public:
        xGScontext* create() override
        {
            return new xGScontextWGL();
        }
    };

} // namespace xGS
