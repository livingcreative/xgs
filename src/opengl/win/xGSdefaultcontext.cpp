/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    opengl/win/xGSdefaultcontext.cpp
        WGL default context helper class implementation
*/

#include "xGSdefaultcontext.h"


using namespace xGS;


xGSdefaultcontext::xGSdefaultcontext() :
    dcWidget(0),
    dcDevice(0),
    dcGLContext(0)
{}

xGSdefaultcontext::~xGSdefaultcontext()
{
    DestroyDefaultContext();
}

GSbool xGSdefaultcontext::CreateDefaultContext(GSbool doublebuffer, GSint colorbits, GSint depthbits, GSint stencilbits)
{
    // wdget
    dcWidget = CreateDefaultWidget();
    if (!dcWidget) {
        return false;
    }

    // get widget DC
    dcDevice = GetDC(dcWidget);
    if (!dcDevice) {
        DestroyDefaultContext();
        return false;
    }

    // fill in PFD struct
    PIXELFORMATDESCRIPTOR pf;
    memset(&pf, 0, sizeof pf);
    pf.nSize = sizeof pf;
    pf.nVersion = 1;
    pf.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    if (doublebuffer) {
        pf.dwFlags = pf.dwFlags | PFD_DOUBLEBUFFER;
    }
    pf.iPixelType = PFD_TYPE_RGBA;
    pf.cColorBits = colorbits;
    pf.cDepthBits = depthbits;
    pf.cStencilBits = stencilbits;
    pf.iLayerType = PFD_MAIN_PLANE;

    // choose format
    GSint n = ChoosePixelFormat(dcDevice, &pf);
    if (!n) {
        DestroyDefaultContext();
        return false;
    }

    // set pixel format for DC
    if (!SetPixelFormat(dcDevice, n, nullptr)) {
        DestroyDefaultContext();
        return false;
    }

    // create default GL context
    dcGLContext = wglCreateContext(dcDevice);
    if (!dcGLContext) {
        DestroyDefaultContext();
        return false;
    }

    // set context current
    if (!wglMakeCurrent(dcDevice, dcGLContext)) {
        DestroyDefaultContext();
        return false;
    }

    return true;
}

void xGSdefaultcontext::DestroyDefaultContext()
{
    if (dcGLContext) {
        wglMakeCurrent(0, 0);
        wglDeleteContext(dcGLContext);
        dcGLContext = 0;
    }

    if (dcDevice) {
        ReleaseDC(dcWidget, dcDevice);
        dcDevice = 0;
    }

    DestroyDefaultWidget(dcWidget);
}

HWND xGSdefaultcontext::CreateDefaultWidget()
{
    // create invisible pop-up "STATIC" window
    return CreateWindowExA(
        0, "STATIC", "xGS render", WS_POPUP,
        0, 0, 0, 0, 0, 0, GetModuleHandle(NULL), NULL
    );
}

void xGSdefaultcontext::DestroyDefaultWidget(HWND &widget)
{
    if (widget) {
        DestroyWindow(widget);
        widget = 0;
    }
}
