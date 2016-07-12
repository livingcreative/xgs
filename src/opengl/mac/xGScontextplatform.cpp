/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    mac/xGScontextplatform.cpp
        xGScontextOSX class implementation
*/

#include "xGScontextplatform.h"


xGScontextOSX::xGScontextOSX() :
    p_context(nullptr)
{}

xGScontextOSX::~xGScontextOSX()
{
    CleanUp();
}

GSerror xGScontextOSX::Initialize()
{
    return GS_OK;
}

GSerror xGScontextOSX::CreateRenderer(const GSrendererdescription &desc)
{
    // TODO: implement pixel format

    p_context = cw_create(reinterpret_cast<NSWidget*>(desc.widget));

    p_rtformat.pfColorBits = 32;
    p_rtformat.pfDepthBits = 24;
    p_rtformat.pfSRGB = true;

    return GS_OK;
}

GSbool xGScontextOSX::DestroyRenderer()
{
    CleanUp();
    return GS_TRUE;
}

GSbool xGScontextOSX::Display()
{
    cw_display(p_context);
    return GS_TRUE;
}

GSsize xGScontextOSX::RenderTargetSize() const
{
    int width, height;
    cw_getsize(p_context, &width, &height);

    GSsize result = {
        width, height
    };

    return result;
}

void xGScontextOSX::CleanUp()
{
    if (p_context) {
        cw_destroy(p_context);
        p_context = nullptr;
    }
}
