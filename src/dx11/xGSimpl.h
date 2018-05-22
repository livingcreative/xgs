/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    dx11/xGSimpl.h
        xGS API object implementation class header
            implements main API (system) object
*/

#pragma once

#include "xGS/xGS.h"
#include "xGSimplbase.h"
#include "xGSDX11util.h"
#include <memory>
#include <vector>
#include <unordered_map>


namespace xGS
{

    class xGSImpl : public xGSImplBase
    {
    public:
        xGSImpl();

    public:
        ID3D11Device* device() const { return p_device; }
        ID3D11DeviceContext* context() const { return p_context; }

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

        void SetupGeometryImpl(IxGSGeometryImpl *geometry);

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
        void GeometryBufferCommitmentGeometry(IxGSGeometryImpl *geometry, GSuint vertexsize, GSuint indexsize, GSbool commit);
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

    private:
        void AddTextureFormatDescriptor(GSvalue format, GSint bpp, DXGI_FORMAT dxgifmt);
        void RenderTargetSize(GSsize &size);

        void CheckDefaultRTResize();
        void RecreateDefaultRT();
        void RecreateDefaultRTDepthStencil();
        void DefaultRTFormats();

    protected:
        struct Sampler
        {
            ID3D11SamplerState *sampler;
            GSuint refcount;
        };

        typedef std::vector<Sampler> SamplerList;

        SamplerList p_samplerlist;
        GScaps      p_caps;

    private:
        typedef std::unordered_map<GSvalue, TextureFormatDescriptor> TextureDescriptorsMap;

        IDXGISwapChain         *p_swapchain;
        ID3D11Device           *p_device;
        ID3D11DeviceContext    *p_context;
        GSwidget                p_widget;
        bool                    p_checkresize;
        int                     p_defaultrtwidth;
        int                     p_defaultrtheight;
        ID3D11RenderTargetView *p_defaultrt;
        ID3D11DepthStencilView *p_defaultrtds;

        TextureDescriptorsMap   p_texturedescs;

        int                     p_timerqueries[1024];
        GSuint                  p_timerindex;
        GSuint                  p_opentimerqueries;
        GSuint                  p_timerscount;
    };

} // namespace xGS
