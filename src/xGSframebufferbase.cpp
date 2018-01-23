/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    opengl/xGSframebuffer.cpp
        FrameBuffer object implementation class
*/

#include "xGSframebufferbase.h"
#include "xGStexture.h"


using namespace xGS;
using namespace c_util;


xGSFrameBufferBase::xGSFrameBufferBase() :
    p_width(0),
    p_height(0),
    p_colortargets(1),
    p_srgb(false),
    p_activecolortargets(0),
    p_depthstencilformat(GS_DEPTH_NONE),
    p_multisample(GS_MULTISAMPLE_NONE)
{
    for (int n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        p_colorformat[n] = GS_COLOR_DEFAULT;
    }
}

GSsize xGSFrameBufferBase::size() const
{
    GSsize result;
    result.width = p_width;
    result.height = p_height;
    return result;
}

static inline GSenum RTFormat(GSenum format, xGSTextureImpl *attachment)
{
    if (format != GS_DEFAULT) {
        return format;
    } else {
        return attachment ? attachment->format() : GS_NONE;
    }
}

void xGSFrameBufferBase::getformats(GSenum *colorformats, GSenum &depthstencilformat) const
{
    for (size_t n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        colorformats[n] = RTFormat(p_colorformat[n], p_colortextures[n].p_texture);
    }
    depthstencilformat = RTFormat(p_depthstencilformat, p_depthtexture.p_texture);
}


xGSFrameBufferBase::Attachment::~Attachment()
{
    if (p_texture) {
        p_texture->Release();
    }
}

void xGSFrameBufferBase::Attachment::attach(xGSTextureImpl *texture, GSuint level, GSuint slice)
{
    if (p_texture) {
        p_texture->Release();
    }

    p_texture = texture;
    p_level = level;
    p_slice = slice;

    if (p_texture) {
        p_texture->AddRef();
    }
}
