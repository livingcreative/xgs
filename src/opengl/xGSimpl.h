/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    opengl/xGSimpl.h
        xGS API object implementation class header
            implements main API (system) object
*/

#pragma once

#include "xGS/xGS.h"
#include "../xGSimplBase.h"
#include "xGSGLutil.h"
#include <memory>
#include <vector>
#include <unordered_map>


namespace xGS
{

    class xGScontext;
    class xGScontextCreator;


    class xGSImpl : public xGSImplBase
    {
    public:
        xGSImpl(xGScontext *context);
        ~xGSImpl() override;

    public:
        // cpas
        const GScaps& caps() const { return p_caps; }

        // samplers
        GSuint samplerCount() const { return GSuint(p_samplerlist.size()); }
        GLuint sampler(GSuint index) const { return p_samplerlist[index].sampler; }
        void referenceSampler(GSuint index) { ++p_samplerlist[index].refcount; }
        void dereferenceSampler(GSuint index) { --p_samplerlist[index].refcount; }

#ifdef _DEBUG
        void debugTrackGLError(const char *text);
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
        static IxGS create(xGScontextCreator *contextcreator);

        struct TextureFormatDescriptor
        {
            GSint  bpp; // BYTES per pixel
            GLenum GLIntFormat;
            GLenum GLFormat;
            GLenum GLType;

            TextureFormatDescriptor() :
                bpp(0),
                GLIntFormat(0),
                GLFormat(0),
                GLType(0)
            {}

            TextureFormatDescriptor(GSint _bpp, GLenum _intformat, GLenum _format, GLenum _type) :
                bpp(_bpp),
                GLIntFormat(_intformat),
                GLFormat(_format),
                GLType(_type)
            {}
        };

        GSbool GetTextureFormatDescriptor(GSvalue format, TextureFormatDescriptor &descriptor);
        const GSpixelformat& DefaultRenderTargetFormat();

    private:
        void AddTextureFormatDescriptor(GSvalue format, GSint _bpp, GLenum _intformat, GLenum _format, GLenum _type);
        void RenderTargetSize(GSsize &size);

        void DefaultRTFormats();

        void DrawImmediatePrimitives(xGSGeometryBufferImpl *buffer);

    private:
        struct Sampler
        {
            GLuint sampler;
            GSuint refcount;
        };

        typedef std::unique_ptr<xGScontext>                          ContextPtr;
        typedef std::vector<Sampler>                                 SamplerList;
        typedef std::unordered_map<GSvalue, TextureFormatDescriptor> TextureDescriptorsMap;

    private:
        ContextPtr             p_context;

        SamplerList            p_samplerlist;

        GScaps                 p_caps;
        TextureDescriptorsMap  p_texturedescs;

        GLuint                 p_capturequery;

        GLuint                 p_timerqueries[1024];
        GSuint                 p_timerindex;
        GSuint                 p_opentimerqueries;
        GSuint                 p_timerscount;
    };

} // namespace xGS
