/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    dx11/xGSimpl.cpp
        xGS API object implementation class
*/

#include "xGSimpl.h"
#include "xGSgeometry.h"
#include "xGSgeometrybuffer.h"
#include "xGSdatabuffer.h"
#include "xGStexture.h"
#include "xGSframebuffer.h"
#include "xGSstate.h"
#include "xGSinput.h"
#include "xGSparameters.h"

#ifdef _DEBUG
    #ifdef WIN32
        #include <Windows.h>
    #endif
#endif


using namespace xGS;


xGSImpl::xGSImpl() :
    xGSImplBase()
{
    // TODO: xGSImpl::xGSImpl DX11

    p_systemstate = SYSTEM_READY;
}

#ifdef _DEBUG
void xGSImpl::debugTrackDXError(const char *text)
{
    // TODO: xGSImpl::debugTrackDXError
}
#endif

void xGSImpl::CreateRendererImpl(const GSrendererdescription &desc)
{
    // TODO: xGSImpl::CreateRenderer

    memset(p_timerqueries, 0, sizeof(p_timerqueries));
    p_timerindex = 0;
    p_opentimerqueries = 0;
    p_timerscount = 0;


#ifdef _DEBUG
    debugTrackDXError("xGSImpl::CreateRenderer");
#endif

    DefaultRTFormats();
}

void xGSImpl::DestroyRendererImpl()
{
    // TODO: xGSImpl::DestroyRendererImpl
}

void xGSImpl::CreateSamplersImpl(const GSsamplerdescription *samplers, GSuint count)
{
    // TODO: xGSImpl::CreateSamplersImpl

    p_error = GS_OK;
}

void xGSImpl::GetRenderTargetSizeImpl(GSsize &size)
{
    // TODO: xGSImpl::RenderTargetSize
    //size = p_rendertarget ? p_rendertarget->size() : ;
}

void xGSImpl::ClearImpl(GSbool color, GSbool depth, GSbool stencil, const GScolor &colorvalue, float depthvalue, GSdword stencilvalue)
{
    // TODO: xGSImpl::ClearImpl
}

void xGSImpl::DisplayImpl()
{
    // TODO: xGSImpl::DisplayImpl()
}

void xGSImpl::SetRenderTargetImpl()
{
    // TODO: xGSImpl::SetRenderTargetImpl
}

void xGSImpl::SetViewportImpl(const GSrect &viewport)
{
    // TODO: xGSImpl::SetViewportImpl
}

void xGSImpl::SetStencilReferenceImpl(GSuint ref)
{
    // TODO
    error(GSE_UNIMPLEMENTED);
}

void xGSImpl::SetBlendColorImpl(const GScolor &color)
{
    // TODO: xGSImpl::SetBlendColorImpl
}

void xGSImpl::SetUniformValueImpl(GSenum type, GSint location, const void *value)
{
    // TODO: xGSImpl::SetUniformValueImpl
}


struct SimpleDrawer
{
    void DrawArrays(GSenum mode, GSuint first, GSuint count) const
    {
        // TODO
    }

    void DrawElementsBaseVertex(GSenum mode, GSuint count, GSenum type, const void *indices, GSuint basevertex) const
    {
        // TODO
    }
};

struct InstancedDrawer
{
    InstancedDrawer(GSuint count) :
        p_count(count)
    {}

    void DrawArrays(GSenum mode, GSuint first, GSuint count) const
    {
        // TODO
    }

    void DrawElementsBaseVertex(GSenum mode, GSuint count, GSenum type, const void *indices, GSuint basevertex) const
    {
        // TODO
    }

private:
    GSuint p_count;
};

struct SimpleMultiDrawer
{
    void MultiDrawArrays(GSenum mode, int *first, int *count, GSuint drawcount) const
    {
        // TODO
    }

    void MultiDrawElementsBaseVertex(GSenum mode, int *count, GSenum type, void **indices, GSuint primcount, int *basevertex) const
    {
        // TODO
    }
};

struct InstancedMultiDrawer
{
    InstancedMultiDrawer(GSuint count) :
        p_count(count)
    {}

    void MultiDrawArrays(GSenum mode, int *first, int *count, GSuint drawcount) const
    {
        // TODO
    }

    void MultiDrawElementsBaseVertex(GSenum mode, int *count, GSenum type, void **indices, GSuint primcount, int *basevertex) const
    {
        // TODO
    }

private:
    GSuint p_count;
};


void xGSImpl::BeginCaptureImpl(GSenum mode)
{
    // TODO: xGSImpl::BeginCaptureImpl
}

void xGSImpl::EndCaptureImpl(GSuint *elementcount)
{
    // TODO: xGSImpl::EndCaptureImpl
}

void xGSImpl::DrawImmediatePrimitives(xGSGeometryBufferImpl *buffer)
{
    // TODO: xGSImpl::DrawImmediatePrimitives
    for (size_t n = 0; n < buffer->immediateCount(); ++n) {
        const xGSGeometryBufferImpl::Primitive &p = buffer->immediatePrimitive(n);

        if (p.indexcount == 0) {
            // TODO
        } else {
            // TODO
        }
    }
}

void xGSImpl::BuildMIPsImpl(xGSTextureImpl *texture)
{
    // TODO: xGSImpl::BuildMIPsImpl
}

void xGSImpl::CopyImageImpl(
    xGSTextureImpl *src, GSuint srclevel, GSuint srcx, GSuint srcy, GSuint srcz,
    xGSTextureImpl *dst, GSuint dstlevel, GSuint dstx, GSuint dsty, GSuint dstz,
    GSuint width, GSuint height, GSuint depth
)
{
    // TODO: xGSImpl::CopyImageImpl
}

void xGSImpl::CopyDataImpl(xGSObject *src, xGSObject *dst, GSuint readoffset, GSuint writeoffset, GSuint size, GSuint flags)
{
    // TODO: xGSImpl::CopyDataImpl

    p_error = GS_OK;
}

void xGSImpl::BufferCommitmentImpl(xGSObject *buffer, GSuint offset, GSuint size, GSbool commit, GSuint flags)
{
    // TODO: xGSImpl::BufferCommitmentImpl

    p_error = GS_OK;
}

void xGSImpl::GeometryBufferCommitmentImpl(xGSGeometryBufferImpl *buffer)
{
    // TODO: xGSImpl::GeometryBufferCommitmentImpl
}

void xGSImpl::GeometryBufferCommitmentGeometry(xGSGeometryImpl *geometry, GSuint vertexsize, GSuint indexsize, GSbool commit)
{
    // TODO: xGSImpl::GeometryBufferCommitmentGeometry
}

void xGSImpl::TextureCommitmentImpl(xGSTextureImpl *texture, GSuint level, GSuint x, GSuint y, GSuint z, GSuint width, GSuint height, GSuint depth, GSbool commit)
{
    // TODO: xGSImpl::TextureCommitmentImpl

    p_error = GS_OK;
}

void xGSImpl::BeginTimerQueryImpl()
{
    // TODO: xGSImpl::BeginTimerQueryImpl
}

void xGSImpl::EndTimerQueryImpl()
{
    // TODO: xGSImpl::EndTimerQueryImpl

    p_error = GS_OK;
}

void xGSImpl::TimestampQueryImpl()
{
    // TODO: xGSImpl::TimestampQueryImpl
}

void xGSImpl::GatherTimersImpl(GSuint flags, GSuint64 *values, GSuint count)
{
    // TODO: xGSImpl::GatherTimersImpl

    p_error = GS_OK;
}


GSbool xGSImpl::GetTextureFormatDescriptor(GSvalue format, TextureFormatDescriptor &descriptor)
{
    auto d = p_texturedescs.find(format);
    if (d == p_texturedescs.end()) {
        return GS_FALSE;
    }

    descriptor = d->second;

    return GS_TRUE;
}

//const GSpixelformat& xGSImpl::DefaultRenderTargetFormat()
//{
//    TODO
//}


void xGSImpl::AddTextureFormatDescriptor(GSvalue format)
{
    // TODO: xGSImpl::AddTextureFormatDescriptor
    p_texturedescs.insert(std::make_pair(
        format,
        TextureFormatDescriptor(0)
    ));
}

void xGSImpl::DefaultRTFormats()
{
    // TODO: xGSImpl::DefaultRTFormats

    // TODO: fill in current RT formats with default RT formats
    for (size_t n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        p_colorformats[n] = GS_NONE;
    }

    //const GSpixelformat &fmt = p_context->RenderTargetFormat();

    //p_colorformats[0] = ColorFormatFromPixelFormat(fmt);
    //p_depthstencilformat = DepthFormatFromPixelFormat(fmt);
}
