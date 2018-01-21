/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/xgs

    dx12/xGSimpl.h
        xGS API object implementation class header
            implements main API (system) object
*/

#pragma once

#include "xGS/xGS.h"
#include "xGSimplbase.h"
#include "xGSDX12util.h"
#include <memory>
#include <vector>
#include <unordered_map>


namespace xGS
{
    class xGSTextureImpl;


    class xGSImpl : public xGSImplBase
    {
    public:
        xGSImpl();

    public:
        ID3D12Device* device() const { return p_device; }

        // cpas
        const GScaps& caps() const { return p_caps; }

        // samplers
        GSuint samplerCount() const { return GSuint(p_samplerlist.size()); }
        void* sampler(GSuint index) const { return nullptr; }
        void referenceSampler(GSuint index) { ++p_samplerlist[index].refcount; }
        void dereferenceSampler(GSuint index) { --p_samplerlist[index].refcount; }

#ifdef _DEBUG
        void debugTrackDXError(const char *text);
#endif

    // IxGS platform specific implementation
    protected:
        void CreateRendererImpl(const GSrendererdescription &desc);
        void DestroyRendererImpl();

        void CreateSamplersImpl(const GSsamplerdescription *samplers, GSuint count);

        void GetRenderTargetSizeImpl(GSsize &size);

        void ClearImpl(GSbool color, GSbool depth, GSbool stencil, const GScolor &colorvalue, float depthvalue, GSdword stencilvalue);
        void DisplayImpl();

        void SetRenderTargetImpl();

        void SetViewportImpl(const GSrect &viewport);
        void SetStencilReferenceImpl(GSuint ref);
        void SetBlendColorImpl(const GScolor &color);
        void SetUniformValueImpl(GSenum type, GSint location, const void *value);

        void SetupGeometryImpl(xGSGeometryImpl *geometry);

        void BeginCaptureImpl(GSenum mode);
        void EndCaptureImpl(GSuint *elementcount);

        void DrawImmediatePrimitives(xGSGeometryBufferImpl *buffer);

        void BuildMIPsImpl(xGSTextureImpl *texture);

        void CopyImageImpl(
            xGSTextureImpl *src, GSuint srclevel, GSuint srcx, GSuint srcy, GSuint srcz,
            xGSTextureImpl *dst, GSuint dstlevel, GSuint dstx, GSuint dsty, GSuint dstz,
            GSuint width, GSuint height, GSuint depth
        );

        void CopyDataImpl(xGSObject *src, xGSObject *dst, GSuint readoffset, GSuint writeoffset, GSuint size, GSuint flags);

        void BufferCommitmentImpl(xGSObject *buffer, GSuint offset, GSuint size, GSbool commit, GSuint flags);
        void GeometryBufferCommitmentImpl(xGSGeometryBufferImpl *buffer);
        void GeometryBufferCommitmentGeometry(xGSGeometryImpl *geometry, GSuint vertexsize, GSuint indexsize, GSbool commit);
        void TextureCommitmentImpl(xGSTextureImpl *texture, GSuint level, GSuint x, GSuint y, GSuint z, GSuint width, GSuint height, GSuint depth, GSbool commit);

        void BeginTimerQueryImpl();
        void EndTimerQueryImpl();
        void TimestampQueryImpl();
        void GatherTimersImpl(GSuint flags, GSuint64 *values, GSuint count);

    public:
        struct TextureFormatDescriptor
        {
            GSint       bpp; // BYTES per pixel
            DXGI_FORMAT format;


            TextureFormatDescriptor() :
                bpp(0),
                format(DXGI_FORMAT_UNKNOWN)
            {}

            TextureFormatDescriptor(GSint _bpp, DXGI_FORMAT _format) :
                bpp(_bpp),
                format(_format)
            {}
        };

        GSbool GetTextureFormatDescriptor(GSvalue format, TextureFormatDescriptor &descriptor);
        //const GSpixelformat& DefaultRenderTargetFormat();

        void UploadBufferData(ID3D12Resource *source, ID3D12Resource *dest, size_t destoffset, size_t destsize);
        void UploadTextureData(ID3D12Resource *source, ID3D12Resource *dest, size_t level, const D3D12_SUBRESOURCE_FOOTPRINT &footprint);

    private:
        void AddTextureFormatDescriptor(GSvalue format, GSint bpp, DXGI_FORMAT dxgifmt);
        void RenderTargetSize(GSsize &size);

        void CheckDefaultRTResize();
        void RecreateDefaultRT();
        void RecreateDefaultRTDepthStencil();
        void DefaultRTFormats();

        D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetDescriptor() const;
        D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilDescriptor() const;

        void TransitionBarrier(ID3D12GraphicsCommandList *list, ID3D12Resource *resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
        void RTTransitionBarrier(bool topresent);

        void WaitFence();

    protected:
        struct Sampler
        {
            D3D12_CPU_DESCRIPTOR_HANDLE sampler;
            GSuint                      refcount;
        };

        typedef std::vector<Sampler> SamplerList;

        SamplerList p_samplerlist;
        GScaps      p_caps;

    private:
        typedef std::unordered_map<GSvalue, TextureFormatDescriptor> TextureDescriptorsMap;

        IDXGISwapChain3             *p_swapchain;
        ID3D12Device                *p_device;
        ID3D12Fence                 *p_fence;
        HANDLE                       p_event;
        UINT64                       p_currentfence;
        ID3D12CommandQueue          *p_commandqueue;
        // TODO: this should go to render list object when
        ID3D12CommandAllocator      *p_cmdallocator;
        ID3D12GraphicsCommandList   *p_commandlist;

        ID3D12CommandAllocator      *p_intcmdallocator;
        ID3D12GraphicsCommandList   *p_intcommandlist;

        UINT                         p_samplersize;
        UINT                         p_rtvsize;
        UINT                         p_dsvsize;
        UINT                         p_cbvsrvsize;

        // TODO: heaps should be allocated in chunked manner automatically depending
        //       on allocation of resources that use them
        ID3D12DescriptorHeap        *p_sampler;
        ID3D12DescriptorHeap        *p_rtv;
        ID3D12DescriptorHeap        *p_dsv;
        ID3D12DescriptorHeap        *p_cbvsrv;

        D3D12_CPU_DESCRIPTOR_HANDLE  p_samplerstart;
        D3D12_CPU_DESCRIPTOR_HANDLE  p_rtvstart;
        D3D12_CPU_DESCRIPTOR_HANDLE  p_dsvstart;
        D3D12_CPU_DESCRIPTOR_HANDLE  p_cbvstart;

        GSwidget                     p_widget;
        bool                         p_checkresize;
        int                          p_defaultrtwidth;
        int                          p_defaultrtheight;
        int                          p_currentbuffer;
        ID3D12Resource              *p_buffers[2];
        ID3D12Resource              *p_depthstencil;

        TextureDescriptorsMap        p_texturedescs;

        int                          p_timerqueries[1024];
        GSuint                       p_timerindex;
        GSuint                       p_opentimerqueries;
        GSuint                       p_timerscount;
    };

} // namespace xGS
