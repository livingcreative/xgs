/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    opengl/mac/xGScontextplatform.cpp
        xGScontext class implementation
*/

#include "xGScontextplatform.h"
#include "contextwrapper.h"


xGScontext::xGScontext() :
    p_context(nullptr)
{}

xGScontext::~xGScontext()
{
    CleanUp();
}

GSerror xGScontext::Initialize()
{
    return GS_OK;
}

GSerror xGScontext::CreateRenderer(const GSrendererdescription &desc)
{
    p_context = cw_create(reinterpret_cast<NSWidget*>(desc.widget));

    p_rtformat.pfColorBits = 32;
    p_rtformat.pfDepthBits = 24;
    p_rtformat.pfSRGB = true;

    return GS_OK;
}

GSbool xGScontext::DestroyRenderer()
{
    CleanUp();
    return GS_TRUE;
}

GSbool xGScontext::Display()
{
    cw_display(p_context);
    return GS_TRUE;
}

GSsize xGScontext::RenderTargetSize() const
{
    int width, height;
    cw_getsize(p_context, &width, &height);

    GSsize result = {
        width, height
    };

    return result;
}

void xGScontext::CleanUp()
{
    if (p_context) {
        cw_destroy(p_context);
        p_context = nullptr;
    }
}
