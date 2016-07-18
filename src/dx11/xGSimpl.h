/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    dx11/xGSimpl.h
        xGS API object implementation class header
            implements main API (system) object
*/

#pragma once

#include "xGS/xGS.h"
#include "xGSimplBase.h"
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
        ~xGSImpl() override;

    public:
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

    // IxGS
    public:
        GSbool xGSAPI CreateRenderer(const GSrendererdescription &desc) override;
        GSbool xGSAPI DestroyRenderer(GSbool restorevideomode) override;

        GSbool xGSAPI CreateObject(GSenum type, const void *desc, void **result) override;
        GSbool xGSAPI CreateSamplers(const GSsamplerdescription *samplers, GSuint count) override;

        GSbool xGSAPI GetRenderTargetSize(GSsize &size) override;

        GSbool xGSAPI Clear(GSbool color, GSbool depth, GSbool stencil, const GScolor &colorvalue, float depthvalue, GSdword stencilvalue) override;
        GSbool xGSAPI Display() override;

        GSbool xGSAPI SetRenderTarget(IxGSFrameBuffer rendertarget) override;
        GSbool xGSAPI SetViewport(const GSrect &viewport) override;
        GSbool xGSAPI SetState(IxGSState state) override;
        GSbool xGSAPI SetInput(IxGSInput input) override;
        GSbool xGSAPI SetParameters(IxGSParameters parameters) override;

        GSbool xGSAPI SetStencilReference(GSuint ref) override;
        GSbool xGSAPI SetBlendColor(const GScolor &color) override;
        GSbool xGSAPI SetUniformValue(GSenum set, GSenum slot, GSenum type, const void *value) override;

        GSbool xGSAPI DrawGeometry(IxGSGeometry geometry) override;
        GSbool xGSAPI DrawGeometryInstanced(IxGSGeometry geometry, GSuint count) override;
        GSbool xGSAPI DrawGeometries(IxGSGeometry *geometries, GSuint count) override;
        GSbool xGSAPI DrawGeometriesInstanced(IxGSGeometry *geometries, GSuint count, GSuint instancecount) override;

        GSbool xGSAPI BeginCapture(GSenum mode, IxGSGeometryBuffer buffer) override;
        GSbool xGSAPI EndCapture(GSuint *elementcount) override;

        GSbool xGSAPI BeginImmediateDrawing(IxGSGeometryBuffer buffer, GSuint flags) override;
        GSbool xGSAPI ImmediatePrimitive(GSenum type, GSuint vertexcount, GSuint indexcount, GSuint flags, GSimmediateprimitive *primitive) override;
        GSbool xGSAPI EndImmediateDrawing() override;

        GSbool xGSAPI BuildMIPs(IxGSTexture texture) override;

        GSbool xGSAPI CopyImage(
            IxGSTexture src, GSuint srclevel, GSuint srcx, GSuint srcy, GSuint srcz,
            IxGSTexture dst, GSuint dstlevel, GSuint dstx, GSuint dsty, GSuint dstz,
            GSuint width, GSuint height, GSuint depth
        ) override;
        GSbool xGSAPI CopyData(xGSObject *src, xGSObject *dst, GSuint readoffset, GSuint writeoffset, GSuint size, GSuint flags) override;

        GSbool xGSAPI BufferCommitment(xGSObject *buffer, GSuint offset, GSuint size, GSbool commit, GSuint flags) override;
        GSbool xGSAPI GeometryBufferCommitment(IxGSGeometryBuffer buffer, IxGSGeometry *geometries, GSuint count, GSbool commit) override;
        GSbool xGSAPI TextureCommitment(IxGSTexture texture, GSuint level, GSuint x, GSuint y, GSuint z, GSuint width, GSuint height, GSuint depth, GSbool commit) override;

        GSbool xGSAPI Compute(IxGSComputeState state, GSuint x, GSuint y, GSuint z) override;

        GSbool xGSAPI BeginTimerQuery() override;
        GSbool xGSAPI EndTimerQuery() override;
        GSbool xGSAPI TimstampQuery() override;
        GSbool xGSAPI GatherTimers(GSuint flags, GSuint64 *values, GSuint count) override;

    public:
        static IxGS create();

        struct TextureFormatDescriptor
        {
            GSint  bpp; // BYTES per pixel

            TextureFormatDescriptor() :
                bpp(0)
            {}

            TextureFormatDescriptor(GSint _bpp) :
                bpp(_bpp)
            {}
        };

        GSbool GetTextureFormatDescriptor(GSvalue format, TextureFormatDescriptor &descriptor);
        //const GSpixelformat& DefaultRenderTargetFormat();

    private:
        void AddTextureFormatDescriptor(GSvalue format);
        void RenderTargetSize(GSsize &size);

        void DefaultRTFormats();

        void DrawImmediatePrimitives(xGSGeometryBufferImpl *buffer);

    private:
        struct Sampler
        {
            GSuint refcount;
        };

        typedef std::vector<Sampler>                                 SamplerList;
        typedef std::unordered_map<GSvalue, TextureFormatDescriptor> TextureDescriptorsMap;

    private:
        SamplerList            p_samplerlist;

        GScaps                 p_caps;
        TextureDescriptorsMap  p_texturedescs;

        int                    p_timerqueries[1024];
        GSuint                 p_timerindex;
        GSuint                 p_opentimerqueries;
        GSuint                 p_timerscount;
    };

} // namespace xGS
