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
#include "xGSutil.h"
#include "xGSDX11util.h"
#include "IUnknownImpl.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>


namespace xGS
{

    class xGSGeometryImpl;
    class xGSGeometryBufferImpl;
    class xGSDataBufferImpl;
    class xGSTextureImpl;
    class xGSFrameBufferImpl;
    class xGSStateImpl;
    class xGSInputImpl;
    class xGSParametersImpl;


    class xGSImpl : public xGSIUnknownImpl<xGSSystem>
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

        // memory management
        GSptr allocate(GSint size);
        void free(GSptr &memory);

        // error set-up
        GSbool error(GSerror code, DebugMessageLevel level = DebugMessageLevel::Error);

        // debug logging
        void debug(DebugMessageLevel level, const char *format, ...);
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
        template <typename T> void AddObject(T *object);
        template <typename T> void RemoveObject(T *object);

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
        enum SystemState
        {
            SYSTEM_NOTREADY,
            SYSTEM_READY,
            RENDERER_READY,
            CAPTURE
        };

        GSbool ValidateState(SystemState requiredstate, bool exactmatch, bool matchimmediate, bool requiredimmediate);

        void AddTextureFormatDescriptor(GSvalue format);
        void RenderTargetSize(GSsize &size);

        template <typename T>
        void CheckObjectList(const T &list, const std::string &listname)
        {
#ifdef _DEBUG
            if (list.size()) {
                debug(DebugMessageLevel::Warning, "Not all resources released in list %s ar renderer destroy", listname.c_str());
#ifdef _MSC_VER
                _CrtDbgBreak();
#endif
            }
#endif
        }

        template <typename T>
        void ReleaseObjectList(T &list, const std::string &listname)
        {
            CheckObjectList(list, listname);
            for (auto &object : list) {
                object->ReleaseRendererResources();
                object->DetachFromRenderer();
            }
        }

        template <typename T>
        void AttachObject(T &attachpoint, T object)
        {
            if (attachpoint) {
                attachpoint->Release();
            }

            attachpoint = object;

            if (attachpoint) {
                attachpoint->AddRef();
                attachpoint->apply(p_caps);
            }
        }

        void DefaultRTFormats();

        void SetState(xGSStateImpl *state);

        template <typename T>
        GSbool Draw(IxGSGeometry geometry_to_draw, const T &drawer);
        template <typename T>
        GSbool MultiDraw(IxGSGeometry *geometries_to_draw, GSuint count, const T &drawer);

        void DrawImmediatePrimitives(xGSGeometryBufferImpl *buffer);

    private:
        struct Sampler
        {
            GSuint refcount;
        };

        typedef std::vector<Sampler>                                 SamplerList;
        typedef std::unordered_set<xGSGeometryImpl*>                 GeometryList;
        typedef std::unordered_set<xGSGeometryBufferImpl*>           GeometryBufferList;
        typedef std::unordered_set<xGSDataBufferImpl*>               DataBufferList;
        typedef std::unordered_set<xGSTextureImpl*>                  TextureList;
        typedef std::unordered_set<xGSFrameBufferImpl*>              FrameBufferList;
        typedef std::unordered_set<xGSStateImpl*>                    StateList;
        typedef std::unordered_set<xGSInputImpl*>                    InputList;
        typedef std::unordered_set<xGSParametersImpl*>               ParametersList;
        typedef std::unordered_map<GSvalue, TextureFormatDescriptor> TextureDescriptorsMap;

    private:
        static IxGS            gs;

        GSerror                p_error;
        SystemState            p_systemstate;

        SamplerList            p_samplerlist;
        GeometryList           p_geometrylist;
        GeometryBufferList     p_geometrybufferlist;
        DataBufferList         p_databufferlist;
        TextureList            p_texturelist;
        FrameBufferList        p_framebufferlist;
        StateList              p_statelist;
        InputList              p_inputlist;
        ParametersList         p_parameterslist;

        GScaps                 p_caps;
        TextureDescriptorsMap  p_texturedescs;

        xGSFrameBufferImpl    *p_rendertarget;
        GSenum                 p_colorformats[GS_MAX_FB_COLORTARGETS];
        GSenum                 p_depthstencilformat;

        xGSStateImpl          *p_state;
        xGSInputImpl          *p_input;
        xGSParametersImpl     *p_parameters[GS_MAX_PARAMETER_SETS];

        xGSGeometryBufferImpl *p_capturebuffer;

        xGSGeometryBufferImpl *p_immediatebuffer;

        int                    p_timerqueries[1024];
        GSuint                 p_timerindex;
        GSuint                 p_opentimerqueries;
        GSuint                 p_timerscount;

#ifdef _DEBUG
        GSint                  dbg_memory_allocs;
#endif
    };

} // namespace xGS
