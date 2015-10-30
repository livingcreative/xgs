/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    xGSdefaultcontext.h
        WGL default context helper class
*/

#pragma once

#include "xGS/xGS.h"
#include <windows.h>


namespace xGS {

    // default context data
    class xGSdefaultcontext
    {
    public:
        xGSdefaultcontext();
        ~xGSdefaultcontext();

        GSbool CreateDefaultContext(GSbool doublebuffer = true, GSint colorbits = 0, GSint depthbits = 0, GSint stencilbits = 0);
        void DestroyDefaultContext();

        HDC getDevice() const { return dcDevice; }

        static HWND CreateDefaultWidget();
        static void DestroyDefaultWidget(HWND &widget);

    private:
        HWND  dcWidget;    // widget (window) for context
        HDC   dcDevice;    // device (DC) dor context
        HGLRC dcGLContext; // OpenGL context
    };

} // namespace xGS
