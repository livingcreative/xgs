/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    dx11/xGSframebuffer.cpp
        FrameBuffer object implementation class
*/

#include "xGSframebuffer.h"
#include "xGStexture.h"
#include "kcommon/c_util.h"


using namespace xGS;
using namespace c_util;


xGSFrameBufferImpl::xGSFrameBufferImpl(xGSImpl *owner)
{}

xGSFrameBufferImpl::~xGSFrameBufferImpl()
{}

void xGSFrameBufferImpl::AllocateImpl(int multisampled_attachments)
{
    // TODO: xGSFrameBufferImpl::allocate

    for (GSuint n = 0; n < p_colortargets; ++n) {
        if (p_colorformat[n] == GS_COLOR_DEFAULT) {
            attachTexture(p_colortextures[n], 0);
        }
    }

    if (p_depthstencilformat == GS_DEPTH_DEFAULT) {
        attachTexture(p_depthtexture, 0);
    }
}

void xGSFrameBufferImpl::bind()
{
    // TODO: xGSFrameBufferImpl::bind
}

void xGSFrameBufferImpl::unbind()
{
    // TODO: xGSFrameBufferImpl::unbind
}

void xGSFrameBufferImpl::ReleaseRendererResources()
{
    // TODO: xGSFrameBufferImpl::ReleaseRendererResources
}

void xGSFrameBufferImpl::attachTexture(const Attachment &texture, int attachment)
{
    // TODO: xGSFrameBufferImpl::attachTexture
}
