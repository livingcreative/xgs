/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    opengl/xGSimpl.h
        xGS API object implementation class header
            implements main API (system) object
*/

#pragma once

#include "xGS/xGS.h"
#include "xGSimplbase.h"
#include "xGSGLutil.h"
#include <memory>
#include <vector>
#include <unordered_map>


namespace xGS
{

    class xGScontext;
    class xGScontextCreator;


    class xGSImpl : public xGSBase
    {
    public:
        xGSImpl();

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

        void DefaultRTFormats();

    protected:
        struct Sampler
        {
            GLuint sampler;
            GSuint refcount;
        };

        typedef std::vector<Sampler> SamplerList;

        SamplerList p_samplerlist;
        GScaps      p_caps;

    private:
        typedef std::unique_ptr<xGScontext> ContextPtr;
        typedef std::unordered_map<GSvalue, TextureFormatDescriptor> TextureDescriptorsMap;

        ContextPtr            p_context;

        TextureDescriptorsMap p_texturedescs;

        GLuint                p_capturequery;

        GLuint                p_timerqueries[1024];
        GSuint                p_timerindex;
        GSuint                p_opentimerqueries;
        GSuint                p_timerscount;
    };

} // namespace xGS
