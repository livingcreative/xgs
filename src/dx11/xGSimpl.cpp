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

#include <Windows.h>
#include <d3d11.h>


using namespace xGS;


xGSImpl::xGSImpl() :
    xGSImplBase(),
    p_swapchain(nullptr),
    p_device(nullptr),
    p_context(nullptr),
    p_defaultrt(nullptr)
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

    DXGI_SWAP_CHAIN_DESC swapchaindesc = {};
    swapchaindesc.SampleDesc.Count = 1;
    swapchaindesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchaindesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchaindesc.BufferCount = 1;
    swapchaindesc.OutputWindow = HWND(desc.widget);
    swapchaindesc.Windowed = true;

    HRESULT result = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, 0, nullptr, 0, D3D11_SDK_VERSION,
        &swapchaindesc, &p_swapchain, &p_device, nullptr, &p_context
    );
    if (result != S_OK) {
        error(GSE_SUBSYSTEMFAILED);
        return;
    }

    ID3D11Texture2D *backbuffertex = nullptr;
    p_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffertex));
    if (backbuffertex == nullptr) {
        DestroyRendererImpl();
        error(GSE_SUBSYSTEMFAILED);
        return;
    }
    p_device->CreateRenderTargetView(backbuffertex, nullptr, &p_defaultrt);
    backbuffertex->Release();

    if (p_defaultrt == nullptr) {
        DestroyRendererImpl();
        error(GSE_SUBSYSTEMFAILED);
        return;
    }

    // TODO: depth/stencil
    p_context->OMSetRenderTargets(1, &p_defaultrt, nullptr);

    memset(p_timerqueries, 0, sizeof(p_timerqueries));
    p_timerindex = 0;
    p_opentimerqueries = 0;
    p_timerscount = 0;


#ifdef _DEBUG
    debugTrackDXError("xGSImpl::CreateRenderer");
#endif

    DefaultRTFormats();

    p_error = GS_OK;
}

void xGSImpl::DestroyRendererImpl()
{
    // TODO: xGSImpl::DestroyRendererImpl
    ::Release(p_defaultrt);
    ::Release(p_swapchain);
    ::Release(p_device);
    ::Release(p_context);
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
    if (p_rendertarget) {
        size = p_rendertarget->size();
    } else {
        DXGI_SWAP_CHAIN_DESC desc;
        p_swapchain->GetDesc(&desc);
        size.width = desc.BufferDesc.Width;
        size.height = desc.BufferDesc.Height;
    }
}

void xGSImpl::ClearImpl(GSbool color, GSbool depth, GSbool stencil, const GScolor &colorvalue, float depthvalue, GSdword stencilvalue)
{
    // TODO: non default RT's, depthstencil
    ID3D11RenderTargetView *rt = p_defaultrt;
    ID3D11DepthStencilView *ds = nullptr;

    if (color) {
        p_context->ClearRenderTargetView(p_defaultrt, &colorvalue.r);
    }

    if (ds && (depth || stencil)) {
        UINT flags = 0;

        if (depth) {
            flags |= D3D11_CLEAR_DEPTH;
        }

        if (stencil) {
            flags |= D3D11_CLEAR_STENCIL;
        }

        p_context->ClearDepthStencilView(ds, flags, depthvalue, stencilvalue);
    }
}

void xGSImpl::DisplayImpl()
{
    p_swapchain->Present(0, 0);
}

void xGSImpl::SetRenderTargetImpl()
{
    // TODO: xGSImpl::SetRenderTargetImpl
}

void xGSImpl::SetViewportImpl(const GSrect &viewport)
{
    D3D11_VIEWPORT d3dviewport = {
        FLOAT(viewport.left), FLOAT(viewport.top),
        FLOAT(viewport.width), FLOAT(viewport.height),
        0, 1
    };

    p_context->RSSetViewports(1, &d3dviewport);
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
