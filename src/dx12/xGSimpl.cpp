/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    dx12/xGSimpl.cpp
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
#include <d3d12.h>


using namespace xGS;


xGSImpl::xGSImpl() :
    xGSImplBase(),
    p_swapchain(nullptr),
    p_device(nullptr),
    p_fence(nullptr),
    p_event(0),
    p_currentfence(0),
    p_commandqueue(nullptr),
    p_commandlist(nullptr),
    p_cmdallocator(nullptr),
    p_intcmdallocator(nullptr),
    p_intcommandlist(nullptr),
    p_sampler(nullptr),
    p_rtv(nullptr),
    p_dsv(nullptr),
    p_cbvsrv(nullptr),
    p_widget(0),
    p_checkresize(false),
    p_defaultrtwidth(0),
    p_defaultrtheight(0),
    p_depthstencil(nullptr)
{
    // TODO: xGSImpl::xGSImpl DX12

    p_buffers[0] = nullptr;
    p_buffers[1] = nullptr;

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

    // TODO: select adapter
    // TODO: use desc options

    p_widget = desc.widget;

#if _DEBUG
    ID3D12Debug *debugController = nullptr;
    if (D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)) == S_OK) {
        debugController->EnableDebugLayer();
        debugController->Release();
    }
#endif

    HRESULT result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&p_device));
    if (result != S_OK) {
        error(GSE_SUBSYSTEMFAILED);
        return;
    }

    p_currentfence = 0;
    result = p_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&p_fence));
    if (result != S_OK) {
        DestroyRendererImpl();
        error(GSE_SUBSYSTEMFAILED);
        return;
    }

    p_event = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

    p_samplersize = p_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    p_rtvsize = p_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    p_dsvsize = p_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    p_cbvsrvsize = p_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    result = p_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&p_commandqueue));
    if (result != S_OK) {
        DestroyRendererImpl();
        error(GSE_SUBSYSTEMFAILED);
        return;
    }

    result = p_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&p_cmdallocator));
    if (result != S_OK) {
        DestroyRendererImpl();
        error(GSE_SUBSYSTEMFAILED);
        return;
    }

    result = p_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, p_cmdallocator, nullptr, IID_PPV_ARGS(&p_commandlist));
    if (result != S_OK) {
        DestroyRendererImpl();
        error(GSE_SUBSYSTEMFAILED);
        return;
    }
    // NOTE: list left in opened state!

    result = p_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&p_intcmdallocator));
    if (result != S_OK) {
        DestroyRendererImpl();
        error(GSE_SUBSYSTEMFAILED);
        return;
    }

    result = p_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, p_intcmdallocator, nullptr, IID_PPV_ARGS(&p_intcommandlist));
    if (result != S_OK) {
        DestroyRendererImpl();
        error(GSE_SUBSYSTEMFAILED);
        return;
    }
    p_intcommandlist->Close();
    // NOTE: internal list left in closed state!

    IDXGIFactory1 *factory = nullptr;
    result = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    if (result != S_OK) {
        DestroyRendererImpl();
        error(GSE_SUBSYSTEMFAILED);
        return;
    }

    DXGI_SWAP_CHAIN_DESC swapchaindesc = {};
    swapchaindesc.SampleDesc.Count = 1;
    swapchaindesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchaindesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchaindesc.BufferCount = 2;
    swapchaindesc.OutputWindow = HWND(desc.widget);
    swapchaindesc.Windowed = true;
    swapchaindesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    IDXGISwapChain *swapchain = nullptr;
    result = factory->CreateSwapChain(p_commandqueue, &swapchaindesc, &swapchain);

    if (swapchain) {
        swapchain->QueryInterface(IID_PPV_ARGS(&p_swapchain));
        swapchain->Release();
    }

    factory->Release();

    if (result != S_OK) {
        DestroyRendererImpl();
        error(GSE_SUBSYSTEMFAILED);
        return;
    }

    D3D12_DESCRIPTOR_HEAP_DESC heapdesc = {};
    heapdesc.NumDescriptors = 256;
    heapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    heapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    result = p_device->CreateDescriptorHeap(&heapdesc, IID_PPV_ARGS(&p_rtv));
    if (result != S_OK) {
        DestroyRendererImpl();
        error(GSE_SUBSYSTEMFAILED);
        return;
    }

    heapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    result = p_device->CreateDescriptorHeap(&heapdesc, IID_PPV_ARGS(&p_dsv));
    if (result != S_OK) {
        DestroyRendererImpl();
        error(GSE_SUBSYSTEMFAILED);
        return;
    }

    heapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    result = p_device->CreateDescriptorHeap(&heapdesc, IID_PPV_ARGS(&p_cbvsrv));
    if (result != S_OK) {
        DestroyRendererImpl();
        error(GSE_SUBSYSTEMFAILED);
        return;
    }

    p_rtvstart = p_rtv->GetCPUDescriptorHandleForHeapStart();
    p_dsvstart = p_dsv->GetCPUDescriptorHandleForHeapStart();
    p_cbvstart = p_cbvsrv->GetCPUDescriptorHandleForHeapStart();

    // NOTE: fullscreen mode support
    RECT rc;
    GetClientRect(HWND(p_widget), &rc);
    p_defaultrtwidth = rc.right;
    p_defaultrtheight = rc.bottom;

    // default RT
    RecreateDefaultRT();
    RecreateDefaultRTDepthStencil();
    // TODO: check errors

    p_commandlist->OMSetRenderTargets(
        1, &RenderTargetDescriptor(), true,
        &DepthStencilDescriptor()
    );

    // RT creation commands will put transitions to command list, close list and
    // execute it
    p_commandlist->Close();
    p_commandqueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&p_commandlist));

    p_commandlist->Reset(p_cmdallocator, nullptr);

    //p_context->OMSetRenderTargets(1, &p_defaultrt, p_defaultrtds);

    AddTextureFormatDescriptor(GS_COLOR_RGBX, 4, DXGI_FORMAT_B8G8R8A8_UNORM);
    AddTextureFormatDescriptor(GS_COLOR_S_RGBX, 4, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
    AddTextureFormatDescriptor(GS_COLOR_RGBA, 4, DXGI_FORMAT_B8G8R8A8_UNORM);
    AddTextureFormatDescriptor(GS_COLOR_S_RGBA, 4, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);

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

    // TODO: wait for device to finish processing before deleting any resources
    WaitFence();

    CloseHandle(p_event);
    p_event = 0;

    ::Release(p_commandlist);
    ::Release(p_cmdallocator);
    ::Release(p_intcommandlist);
    ::Release(p_intcmdallocator);

    ::Release(p_sampler);
    ::Release(p_rtv);
    ::Release(p_dsv);
    ::Release(p_cbvsrv);

    ::Release(p_buffers[0]);
    ::Release(p_buffers[1]);
    ::Release(p_depthstencil);

    ::Release(p_swapchain);
    ::Release(p_commandqueue);
    ::Release(p_fence);
    ::Release(p_device);
}

void xGSImpl::CreateSamplersImpl(const GSsamplerdescription *samplers, GSuint count)
{
    ::Release(p_sampler);

    D3D12_DESCRIPTOR_HEAP_DESC heapdesc = {};
    heapdesc.NumDescriptors = count;
    heapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    heapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT result = p_device->CreateDescriptorHeap(&heapdesc, IID_PPV_ARGS(&p_sampler));
    if (result != S_OK) {
        error(GSE_OUTOFRESOURCES);
        return;
    }

    p_samplerstart = p_sampler->GetCPUDescriptorHandleForHeapStart();
    auto sampler = p_samplerstart;

    p_samplerlist.resize(count);
    for (size_t n = 0; n < count; ++n) {
        Sampler &s = p_samplerlist[n];

        s.refcount = 0;
        s.sampler = sampler;

        // TODO: check values

        D3D12_SAMPLER_DESC desc = {};

        switch (samplers->filter) {
            case GS_FILTER_NEAREST:
                desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
                break;

            case GS_FILTER_LINEAR:
                // TODO: linear filter without mips?
                desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                break;

            case GS_FILTER_TRILINEAR:
                desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                break;
        }

        desc.AddressU = dx12_texture_wrap(samplers->wrapu);
        desc.AddressV = dx12_texture_wrap(samplers->wrapv);
        desc.AddressW = dx12_texture_wrap(samplers->wrapw);

        desc.MinLOD = float(samplers->minlod);
        desc.MaxLOD = float(samplers->maxlod);
        desc.MipLODBias = samplers->bias;

        desc.MaxAnisotropy = 1; // ?

        desc.ComparisonFunc =
            samplers->depthcompare ?
            dx12_compare_func(samplers->depthcompare) :
            D3D12_COMPARISON_FUNC_ALWAYS;

        p_device->CreateSampler(&desc, sampler);
        sampler.ptr += p_samplersize;

        ++samplers;
    }

    p_error = GS_OK;
}

void xGSImpl::GetRenderTargetSizeImpl(GSsize &size)
{
    if (p_rendertarget) {
        size = p_rendertarget->size();
    } else {
        CheckDefaultRTResize();
        size.width = p_defaultrtwidth;
        size.height = p_defaultrtheight;
    }
}

void xGSImpl::ClearImpl(GSbool color, GSbool depth, GSbool stencil, const GScolor &colorvalue, float depthvalue, GSdword stencilvalue)
{
    CheckDefaultRTResize();

    // TODO: non default RT's, depthstencil
    auto rt = RenderTargetDescriptor();
    auto ds = DepthStencilDescriptor();

    if (color) {
        p_commandlist->ClearRenderTargetView(rt, &colorvalue.r, 0, nullptr);
    }

    if (ds.ptr && (depth || stencil)) {
        D3D12_CLEAR_FLAGS flags = D3D12_CLEAR_FLAGS(0);
    
        if (depth) {
            flags |= D3D12_CLEAR_FLAG_DEPTH;
        }
    
        if (stencil) {
            flags |= D3D12_CLEAR_FLAG_STENCIL;
        }

        p_commandlist->ClearDepthStencilView(
            ds, flags, depthvalue, stencilvalue, 0, nullptr
        );
    }
}

void xGSImpl::DisplayImpl()
{
    // transition RT's buffer to present state
    RTTransitionBarrier(true);

    // finish command list
    p_commandlist->Close();

    // execute it and reset back
    p_commandqueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&p_commandlist));

    // present content
    p_swapchain->Present(0, 0);

    // reset list and record buffer transition back to RENDER_TARGET
    p_commandlist->Reset(p_cmdallocator, nullptr);

    p_currentbuffer += 1;
    p_currentbuffer %= 2;

    // transition next buffer to RENER_TARGET and leave currently presented in PRESENT
    // state
    RTTransitionBarrier(false);

    WaitFence();

    // this flag should be set only in not fullscreen mode
    // due to possible window size changes the size of
    // default RT swapchain and DS buffers should be adjusted
    p_checkresize = true;
}

void xGSImpl::SetRenderTargetImpl()
{
    // TODO: xGSImpl::SetRenderTargetImpl
    if (p_rendertarget == nullptr) {
        CheckDefaultRTResize();

        p_commandlist->OMSetRenderTargets(
            1, &RenderTargetDescriptor(), true,
            &DepthStencilDescriptor()
        );
    }
}

void xGSImpl::SetViewportImpl(const GSrect &viewport)
{
    D3D12_VIEWPORT d3dviewport = {
        FLOAT(viewport.left), FLOAT(viewport.top),
        FLOAT(viewport.width), FLOAT(viewport.height),
        0, 1
    };

    p_commandlist->RSSetViewports(1, &d3dviewport);

    // TODO: this is temp. and probably should be implemented in other way
    //       need clear xGS spec about this behavior
    D3D12_RECT scissor = {
        viewport.left, viewport.top,
        viewport.left + viewport.width,
        viewport.top + viewport.height
    };
    p_commandlist->RSSetScissorRects(1, &scissor);
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

void xGSImpl::SetupGeometryImpl(xGSGeometryImpl *geometry)
{
    // TODO: xGSImpl::SetupGeometryImpl

    p_commandlist->IASetPrimitiveTopology(dx12_primitive_type(
        geometry->type(), geometry->patchVertices()
    ));
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
    //p_context->GenerateMips(texture->view());
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

void xGSImpl::UploadBufferData(ID3D12Resource *source, ID3D12Resource *dest, size_t destoffset, size_t destsize)
{
    p_intcommandlist->Reset(p_intcmdallocator, nullptr);

    TransitionBarrier(p_intcommandlist, dest, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    p_intcommandlist->CopyBufferRegion(dest, destoffset, source, 0, destsize);
    TransitionBarrier(p_intcommandlist, dest, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

    p_intcommandlist->Close();

    p_commandqueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&p_intcommandlist));

    WaitFence();
}

void xGSImpl::UploadTextureData(ID3D12Resource *source, ID3D12Resource *dest, size_t level, const D3D12_SUBRESOURCE_FOOTPRINT &footprint)
{
    p_intcommandlist->Reset(p_intcmdallocator, nullptr);

    D3D12_TEXTURE_COPY_LOCATION dst = {
        dest, D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX
    };
    dst.SubresourceIndex = UINT(level);

    D3D12_TEXTURE_COPY_LOCATION src = {
        source, D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT
    };
    src.PlacedFootprint.Offset = 0;
    src.PlacedFootprint.Footprint = footprint;

    TransitionBarrier(p_intcommandlist, dest, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    p_intcommandlist->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
    TransitionBarrier(p_intcommandlist, dest, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

    p_intcommandlist->Close();

    p_commandqueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&p_intcommandlist));

    WaitFence();
}


void xGSImpl::AddTextureFormatDescriptor(GSvalue format, GSint bpp, DXGI_FORMAT dxgifmt)
{
    // TODO: xGSImpl::AddTextureFormatDescriptor
    p_texturedescs.insert(std::make_pair(
        format,
        TextureFormatDescriptor(bpp, dxgifmt)
    ));
}

void xGSImpl::CheckDefaultRTResize()
{
    if (!p_checkresize || p_rendertarget) {
        return;
    }

    RECT rc;
    GetClientRect(HWND(p_widget), &rc);

    if (rc.right && rc.bottom && (rc.right != p_defaultrtwidth || rc.bottom != p_defaultrtheight)) {
        p_defaultrtwidth = rc.right;
        p_defaultrtheight = rc.bottom;

        // close current list and reset
        p_commandlist->Close();
        p_commandlist->Reset(p_cmdallocator, nullptr);

        ::Release(p_buffers[0]);
        ::Release(p_buffers[1]);

        p_swapchain->ResizeBuffers(0, p_defaultrtwidth, p_defaultrtheight, DXGI_FORMAT_UNKNOWN, 0);

        RecreateDefaultRT();
        RecreateDefaultRTDepthStencil();

        auto rt = RenderTargetDescriptor();
        auto ds = DepthStencilDescriptor();
        p_commandlist->OMSetRenderTargets(1, &rt, true, &ds);
    }

    p_checkresize = false;
}

// NOTE: p_buffers MUST be released before calling this
void xGSImpl::RecreateDefaultRT()
{
    D3D12_CPU_DESCRIPTOR_HANDLE deschandle = p_rtvstart;


    for (int n = 0; n < 2; ++n) {
        p_swapchain->GetBuffer(n, IID_PPV_ARGS(p_buffers + n));

        p_device->CreateRenderTargetView(p_buffers[n], nullptr, deschandle);
        deschandle.ptr += p_rtvsize;
    }

    // prepare current buffers for rendering
    p_currentbuffer = p_swapchain->GetCurrentBackBufferIndex();
    RTTransitionBarrier(false);
}

// NOTE: command list must be opened before calling this
void xGSImpl::RecreateDefaultRTDepthStencil()
{
    // TODO: ensure current resource is not used

    ::Release(p_depthstencil);

    D3D12_RESOURCE_DESC depthstencildesc = {
        D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        0,
        UINT64(p_defaultrtwidth),
        UINT(p_defaultrtheight),
        1, 1,
        DXGI_FORMAT_D24_UNORM_S8_UINT
    };
    depthstencildesc.SampleDesc.Count = 1;
    depthstencildesc.SampleDesc.Quality = 0;
    depthstencildesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthstencildesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_HEAP_PROPERTIES heapprops = {
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1, 1
    };

    D3D12_CLEAR_VALUE clear = {};
    clear.DepthStencil.Depth = 1.0f;
    clear.Format = depthstencildesc.Format;

    HRESULT result = p_device->CreateCommittedResource(
        &heapprops, D3D12_HEAP_FLAG_NONE,
        &depthstencildesc, D3D12_RESOURCE_STATE_COMMON, &clear,
        IID_PPV_ARGS(&p_depthstencil)
    );

    p_device->CreateDepthStencilView(
        p_depthstencil, nullptr,
        p_dsvstart
    );

    TransitionBarrier(
        p_commandlist, p_depthstencil,
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE
    );
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

D3D12_CPU_DESCRIPTOR_HANDLE xGSImpl::RenderTargetDescriptor() const
{
    D3D12_CPU_DESCRIPTOR_HANDLE result = p_rtvstart;
    if (p_rendertarget == nullptr) {
        result.ptr += p_currentbuffer * p_rtvsize;
    } else {
        // TODO
    }
    return result;
}

D3D12_CPU_DESCRIPTOR_HANDLE xGSImpl::DepthStencilDescriptor() const
{
    D3D12_CPU_DESCRIPTOR_HANDLE result = p_dsvstart;
    if (p_rendertarget == nullptr) {
        // NOTE: default RT's DSV at 0 slot
    } else {
        // TODO
    }
    return result;
}

void xGSImpl::TransitionBarrier(ID3D12GraphicsCommandList *list, ID3D12Resource *resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
    D3D12_RESOURCE_BARRIER barrier = {
        D3D12_RESOURCE_BARRIER_TYPE_TRANSITION
    };
    barrier.Transition.pResource = resource;
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter = after;

    list->ResourceBarrier(1, &barrier);
}

void xGSImpl::RTTransitionBarrier(bool topresent)
{
    TransitionBarrier(
        p_commandlist,
        p_buffers[p_currentbuffer],
        topresent ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_PRESENT,
        topresent ? D3D12_RESOURCE_STATE_PRESENT : D3D12_RESOURCE_STATE_RENDER_TARGET
    );
}

void xGSImpl::WaitFence()
{
    ++p_currentfence;

    p_commandqueue->Signal(p_fence, p_currentfence);

    if (p_fence->GetCompletedValue() < p_currentfence) {
        p_fence->SetEventOnCompletion(p_currentfence, p_event);
        WaitForSingleObject(p_event, INFINITE);
    }
}
