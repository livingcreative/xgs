/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    IxGSimpl.h
        xGS API object public interface implementation
*/

#pragma once

#include "xGSimpl.h"


namespace xGS
{

    class IxGSImpl : public xGSImpl
    {
    public:
        ~IxGSImpl() override;

        GSbool xGSAPI CreateRenderer(const GSrendererdescription &desc) override;
        GSbool xGSAPI DestroyRenderer(GSbool restorevideomode) override;

        GSbool xGSAPI CreateObject(GSenum type, const void *desc, void **result) override;
        GSbool xGSAPI CreateSamplers(const GSsamplerdescription *samplers, GSuint count) override;

        GSbool xGSAPI GetRenderTargetSize(GSsize &size) override;

        GSbool xGSAPI Clear(GSbool color, GSbool depth, GSbool stencil, const GScolor &colorvalue, float depthvalue, GSdword stencilvalue) override;
        GSbool xGSAPI Display() override;

        GSbool xGSAPI SetRenderTarget(IxGSFrameBuffer rendertarget) override;
        GSbool xGSAPI SetState(IxGSState state) override;
        GSbool xGSAPI SetInput(IxGSInput input) override;
        GSbool xGSAPI SetParameters(IxGSParameters parameters) override;

        GSbool xGSAPI SetViewport(const GSrect &viewport) override;
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
        GSbool xGSAPI TimestampQuery() override;
        GSbool xGSAPI GatherTimers(GSuint flags, GSuint64 *values, GSuint count) override;

    public:
        static IxGS create();

    private:
        void SetStateImpl(xGSStateImpl *state);

        template <typename T>
        GSbool Draw(IxGSGeometry geometry_to_draw, const T &drawer);
        template <typename T>
        GSbool MultiDraw(IxGSGeometry *geometries_to_draw, GSuint count, const T &drawer);

    private:
        GSbool ValidateState(SystemState requiredstate, bool exactmatch, bool matchimmediate, bool requiredimmediate);

        template <typename T>
        inline void CheckObjectList(const T &list, const std::string &listname);

        template <typename T>
        inline void ReleaseObjectList(T &list, const std::string &listname);

        template <typename T, typename C>
        void AttachObject(const C &caps, T &attachpoint, T object);
    };

} // namespace xGS
