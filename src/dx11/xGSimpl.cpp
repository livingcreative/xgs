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

xGSImpl::~xGSImpl()
{
    if (p_systemstate == CAPTURE) {
        EndCapture(nullptr);
    }
    if (p_systemstate == RENDERER_READY) {
        DestroyRenderer(true);
    }
}

#ifdef _DEBUG
void xGSImpl::debugTrackDXError(const char *text)
{
    // TODO: xGSImpl::debugTrackDXError
}
#endif

GSbool xGSImpl::CreateRenderer(const GSrendererdescription &desc)
{
    if (!ValidateState(SYSTEM_READY, true, false, false)) {
        return GS_FALSE;
    }

    // videomode
    //

    // TODO: xGSImpl::CreateRenderer

    memset(p_timerqueries, 0, sizeof(p_timerqueries));
    p_timerindex = 0;
    p_opentimerqueries = 0;
    p_timerscount = 0;


#ifdef _DEBUG
    debugTrackDXError("xGSImpl::CreateRenderer");
#endif

    DefaultRTFormats();

    p_systemstate = RENDERER_READY;

    return error(GS_OK);
}

GSbool xGSImpl::DestroyRenderer(GSbool restorevideomode)
{
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    CleanupObjects();

    // TODO: xGSImpl::DestroyRenderer

    p_systemstate = SYSTEM_READY;

    return GS_TRUE;
}


#define GS_CREATE_OBJECT(typeconst, impltype, desctype)\
        case typeconst: {\
            impltype *object = impltype::create(this, typeconst);\
            if (object->allocate(*reinterpret_cast<const desctype*>(desc))) {\
                *result = object;\
                return error(GS_OK);\
            } else {\
                object->Release();\
                return GS_FALSE;\
            }\
        }

GSbool xGSImpl::CreateObject(GSenum type, const void *desc, void **result)
{
    // TODO: make refcounting and adding to object list only after successful creation
    switch (type) {
        GS_CREATE_OBJECT(GS_OBJECTTYPE_GEOMETRY, xGSGeometryImpl, GSgeometrydescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_GEOMETRYBUFFER, xGSGeometryBufferImpl, GSgeometrybufferdescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_DATABUFFER, xGSDataBufferImpl, GSdatabufferdescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_TEXTURE, xGSTextureImpl, GStexturedescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_FRAMEBUFFER, xGSFrameBufferImpl, GSframebufferdescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_STATE, xGSStateImpl, GSstatedescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_INPUT, xGSInputImpl, GSinputdescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_PARAMETERS, xGSParametersImpl, GSparametersdescription)
    }

    return error(GSE_INVALIDENUM);
}

#undef GS_CREATE_OBJECT

GSbool xGSImpl::CreateSamplers(const GSsamplerdescription *samplers, GSuint count)
{
    GSuint references = 0;
    for (auto &s : p_samplerlist) {
        references += s.refcount;
    }

    if (references) {
        return error(GSE_INVALIDOPERATION);
    }

    // TODO: xGSImpl::CreateSamplers

    return error(GS_OK);
}


GSbool xGSImpl::GetRenderTargetSize(GSsize &size)
{
    if (!ValidateState(RENDERER_READY, false, false, false)) {
        return GS_FALSE;
    }
    RenderTargetSize(size);
    return error(GS_OK);
}


GSbool xGSImpl::Clear(GSbool color, GSbool depth, GSbool stencil, const GScolor &colorvalue, float depthvalue, GSdword stencilvalue)
{
    // NOTE: clear doesn't work if rasterizer discard enabled
    //       depth buffer is not cleared when depth mask is FALSE

    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    // TODO: xGSImpl::Clear

    return error(GS_OK);
}

GSbool xGSImpl::Display()
{
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    // TODO: xGSImpl::Display

    return false;
}


GSbool xGSImpl::SetRenderTarget(IxGSFrameBuffer rendertarget)
{
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    if (p_rendertarget) {
        p_rendertarget->unbind();
        p_rendertarget->Release();
    }

    p_rendertarget = static_cast<xGSFrameBufferImpl*>(rendertarget);

    if (p_rendertarget) {
        p_rendertarget->AddRef();
        p_rendertarget->bind();
    }

    bool srgb = false;

    if (p_rendertarget == nullptr) {
        DefaultRTFormats();
        srgb = false; // TODO
    } else {
        p_rendertarget->getformats(p_colorformats, p_depthstencilformat);
        srgb = p_rendertarget->srgb();
    }

    // TODO: srgb enable

    // reset current state, after RT change any state should be rebound
    xGSImplBase::SetState(p_caps, static_cast<xGSStateImpl*>(nullptr));

    return error(GS_OK);
}

GSbool xGSImpl::SetViewport(const GSrect &viewport)
{
    // NOTE: actually there's no dependency on immediate mode
    //          consider ignore immediate mode
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    GSsize sz;
    RenderTargetSize(sz);

    // TODO: xGSImpl::SetViewport

    return error(GS_OK);
}

GSbool xGSImpl::SetState(IxGSState state)
{
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    if (!state) {
        return error(GSE_INVALIDOBJECT);
    }

    xGSStateImpl *stateimpl = static_cast<xGSStateImpl*>(state);

    if (!stateimpl->allocated()) {
        return error(GSE_INVALIDOBJECT);
    }

    if (!stateimpl->validate(p_colorformats, p_depthstencilformat)) {
        return error(GSE_INVALIDOBJECT);
    }

    xGSImplBase::SetState(p_caps, stateimpl);

    return error(GS_OK);
}

GSbool xGSImpl::SetInput(IxGSInput input)
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    xGSInputImpl *inputimpl = static_cast<xGSInputImpl*>(input);
    if (inputimpl == nullptr || !inputimpl->allocated()) {
        return error(GSE_INVALIDOBJECT);
    }

    if (p_state == nullptr || inputimpl->state() != p_state) {
        return error(GSE_INVALIDOPERATION);
    }

    AttachObject(p_caps, p_input, inputimpl);

    return error(GS_OK);
}

GSbool xGSImpl::SetParameters(IxGSParameters parameters)
{
    // TODO: ability to change parameters while being in immediate mode
    //      actually parameters change can be recorded into immediate sequence
    //      and executed later, when immediate buffer will be flushed
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (!parameters) {
        return error(GSE_INVALIDOBJECT);
    }

    xGSParametersImpl *parametersimpl = static_cast<xGSParametersImpl*>(parameters);

    if (!parametersimpl->allocated()) {
        return error(GSE_INVALIDOBJECT);
    }

    if (p_state == nullptr || parametersimpl->state() != p_state) {
        return error(GSE_INVALIDSTATE);
    }

    AttachObject(p_caps, p_parameters[parametersimpl->setindex()], parametersimpl);

    return error(GS_OK);
}

GSbool xGSImpl::SetStencilReference(GSuint ref)
{
    // TODO: same as SetParameters behaviour
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (!p_state) {
        return error(GSE_INVALIDSTATE);
    }

    // TODO

    return error(GSE_UNIMPLEMENTED);
}

GSbool xGSImpl::SetBlendColor(const GScolor &color)
{
    // TODO: same as SetParameters behaviour
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (!p_state) {
        return error(GSE_INVALIDSTATE);
    }

    // TODO: xGSImpl::SetBlendColor

    return error(GSE_UNIMPLEMENTED);
}

GSbool xGSImpl::SetUniformValue(GSenum set, GSenum slot, GSenum type, const void *value)
{
    // TODO: same as SetParameters behaviour
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (!p_state) {
        return error(GSE_INVALIDSTATE);
    }

    GSuint setindex = set - GSPS_0;
    if (setindex >= p_state->parameterSetCount()) {
        return error(GSE_INVALIDENUM);
    }

    const GSParameterSet &paramset = p_state->parameterSet(setindex);

    GSuint slotindex = slot - GSPS_0 + paramset.first;
    if (slotindex >= paramset.onepastlast) {
        return error(GSE_INVALIDENUM);
    }

    const xGSStateImpl::ParameterSlot &paramslot = p_state->parameterSlot(slotindex);

    if (paramslot.type != GSPD_CONSTANT) {
        return error(GSE_INVALIDOPERATION);
    }

    // TODO: check type or remove type parameter?
    if (paramslot.location != GS_DEFAULT) {
        switch (type) {
            case GSU_SCALAR:
                // TODO
                break;

            case GSU_VEC2:
                // TODO
                break;

            case GSU_VEC3:
                // TODO
                break;

            case GSU_VEC4:
                // TODO
                break;

            case GSU_MAT2:
                // TODO
                break;

            case GSU_MAT3:
                // TODO
                break;

            case GSU_MAT4:
                // TODO
                break;

            default:
                return error(GSE_INVALIDENUM);
        }
    }

    return error(GS_OK);
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

GSbool xGSImpl::DrawGeometry(IxGSGeometry geometry)
{
    SimpleDrawer drawer;
    return Draw(geometry, drawer);
}

GSbool xGSImpl::DrawGeometryInstanced(IxGSGeometry geometry, GSuint count)
{
    InstancedDrawer drawer(count);
    return Draw(geometry, drawer);
}

GSbool xGSImpl::DrawGeometries(IxGSGeometry *geometries, GSuint count)
{
    SimpleMultiDrawer drawer;
    return MultiDraw(geometries, count, drawer);
}

GSbool xGSImpl::DrawGeometriesInstanced(IxGSGeometry *geometries, GSuint count, GSuint instancecount)
{
    InstancedMultiDrawer drawer(instancecount);
    return MultiDraw(geometries, count, drawer);
}

GSbool xGSImpl::BeginCapture(GSenum mode, IxGSGeometryBuffer buffer)
{
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    if (!buffer) {
        return error(GSE_INVALIDOBJECT);
    }

    xGSGeometryBufferImpl *bufferimpl = static_cast<xGSGeometryBufferImpl*>(buffer);
    if (!bufferimpl->allocated()) {
        return error(GSE_INVALIDOBJECT);
    }

    p_capturebuffer = bufferimpl;
    p_capturebuffer->AddRef();

    // TODO: xGSImpl::BeginCapture

    p_systemstate = CAPTURE;

    return error(GS_OK);
}

GSbool xGSImpl::EndCapture(GSuint *elementcount)
{
    if (!ValidateState(CAPTURE, true, true, false)) {
        return GS_FALSE;
    }

    // TODO: xGSImpl::EndCapture

    p_capturebuffer->Release();
    p_capturebuffer = nullptr;
    p_systemstate = RENDERER_READY;

    return error(GS_OK);
}

GSbool xGSImpl::BeginImmediateDrawing(IxGSGeometryBuffer buffer, GSuint flags)
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (!buffer) {
        return error(GSE_INVALIDOBJECT);
    }

    xGSGeometryBufferImpl *bufferimpl = static_cast<xGSGeometryBufferImpl*>(buffer);
    if (!bufferimpl->allocated() || bufferimpl->type() != GS_GBTYPE_IMMEDIATE) {
        return error(GSE_INVALIDOBJECT);
    }


    // TODO: do i need parrallel buffer fill only and then rendering from filled buffer?
    //          if so, special flag should indicate filling only behaviour and
    //          in that case current input check isn't needed

    if (!p_state) {
        return error(GSE_INVALIDSTATE);
    }

#ifdef _DEBUG
    size_t primaryslot = p_state->inputPrimarySlot();
    xGSGeometryBufferImpl *boundbuffer =
        primaryslot == GS_UNDEFINED ? nullptr : p_state->input(primaryslot).buffer;
    if (p_input && boundbuffer == nullptr)  {
        boundbuffer = p_input->primaryBuffer();
    }
    if (bufferimpl != boundbuffer) {
        return error(GSE_INVALIDOBJECT);
    }
#endif

    p_immediatebuffer = bufferimpl;
    p_immediatebuffer->AddRef();

    p_immediatebuffer->BeginImmediateDrawing();

    return error(GS_OK);
}

GSbool xGSImpl::ImmediatePrimitive(GSenum type, GSuint vertexcount, GSuint indexcount, GSuint flags, GSimmediateprimitive *primitive)
{
    if (!ValidateState(RENDERER_READY, false, true, true)) {
        return GS_FALSE;
    }

    bool flushnotneeded = p_immediatebuffer->EmitPrimitive(
        type, vertexcount, indexcount,
        flags, primitive
    );

    if (!flushnotneeded) {
        // requested primitive can not be added, try to flush buffer
        p_immediatebuffer->EndImmediateDrawing();
        DrawImmediatePrimitives(p_immediatebuffer);
        p_immediatebuffer->BeginImmediateDrawing();

        if (!p_immediatebuffer->EmitPrimitive(type, vertexcount, indexcount, flags, primitive)) {
            // primitive can not be added at all
            return error(GSE_INVALIDVALUE);
        }
    }

    return error(GS_OK);
}

GSbool xGSImpl::EndImmediateDrawing()
{
    if (!ValidateState(RENDERER_READY, false, true, true)) {
        return GS_FALSE;
    }

    p_immediatebuffer->EndImmediateDrawing();
    DrawImmediatePrimitives(p_immediatebuffer);

    p_immediatebuffer->Release();
    p_immediatebuffer = nullptr;

    return error(GS_OK);
}

GSbool xGSImpl::BuildMIPs(IxGSTexture texture)
{
    if (!ValidateState(RENDERER_READY, true, false, false)) {
        return GS_FALSE;
    }

    xGSTextureImpl *tex = static_cast<xGSTextureImpl*>(texture);
    // TODO: xGSImpl::BuildMIPs

    return error(GS_OK);
}

GSbool xGSImpl::CopyImage(
    IxGSTexture src, GSuint srclevel, GSuint srcx, GSuint srcy, GSuint srcz,
    IxGSTexture dst, GSuint dstlevel, GSuint dstx, GSuint dsty, GSuint dstz,
    GSuint width, GSuint height, GSuint depth
)
{
    // TODO: checks

    xGSTextureImpl *srctex = static_cast<xGSTextureImpl*>(src);
    xGSTextureImpl *dsttex = static_cast<xGSTextureImpl*>(dst);

    // TODO: xGSImpl::CopyImage

    return error(GS_OK);
}

GSbool xGSImpl::CopyData(xGSObject *src, xGSObject *dst, GSuint readoffset, GSuint writeoffset, GSuint size, GSuint flags)
{
    if (!ValidateState(RENDERER_READY, true, false, false)) {
        return GS_FALSE;
    }

    if (src == nullptr || dst == nullptr) {
        return error(GSE_INVALIDOBJECT);
    }

    // TODO: xGSImpl::CopyData

    return error(GS_OK);
}

GSbool xGSImpl::BufferCommitment(xGSObject *buffer, GSuint offset, GSuint size, GSbool commit, GSuint flags)
{
    if (!ValidateState(RENDERER_READY, true, false, false)) {
        return GS_FALSE;
    }

    if (buffer == nullptr) {
        return error(GSE_INVALIDOBJECT);
    }

    // TODO: check for support, check for buffer is sparse

    GSenum type = static_cast<xGSUnknownObjectImpl*>(buffer)->objecttype();

    // TODO: xGSImpl::BufferCommitment

    return error(GS_OK);
}

GSbool xGSImpl::GeometryBufferCommitment(IxGSGeometryBuffer buffer, IxGSGeometry *geometries, GSuint count, GSbool commit)
{
    if (!ValidateState(RENDERER_READY, true, false, false)) {
        return GS_FALSE;
    }

    if (buffer == nullptr) {
        return error(GSE_INVALIDOBJECT);
    }

    xGSGeometryBufferImpl *impl = static_cast<xGSGeometryBufferImpl*>(buffer);

    GSuint vertexsize = impl->vertexDecl().buffer_size();
    GSuint indexsize = index_buffer_size(impl->indexFormat());

    // TODO: xGSImpl::GeometryBufferCommitment

    return error(GS_OK);
}

GSbool xGSImpl::TextureCommitment(IxGSTexture texture, GSuint level, GSuint x, GSuint y, GSuint z, GSuint width, GSuint height, GSuint depth, GSbool commit)
{
    if (!ValidateState(RENDERER_READY, true, false, false)) {
        return GS_FALSE;
    }

    if (texture == nullptr) {
        return error(GSE_INVALIDOBJECT);
    }

    xGSTextureImpl *tex = static_cast<xGSTextureImpl*>(texture);

    // xGSImpl::TextureCommitment

    return error(GS_OK);
}

GSbool xGSImpl::Compute(IxGSComputeState state, GSuint x, GSuint y, GSuint z)
{
    // TODO:

    return GS_FALSE;
}

GSbool xGSImpl::BeginTimerQuery()
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    // TODO: xGSImpl::BeginTimerQuery

    return GS_TRUE;
}

GSbool xGSImpl::EndTimerQuery()
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (p_opentimerqueries == 0) {
        return error(GSE_INVALIDOPERATION);
    }

    // TODO: xGSImpl::EndTimerQuery

    return GS_TRUE;
}

GSbool xGSAPI xGSImpl::TimstampQuery()
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    // TODO: xGSImpl::TimstampQuery

    return GS_TRUE;
}

GSbool xGSImpl::GatherTimers(GSuint flags, GSuint64 *values, GSuint count)
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (p_timerindex == 0) {
        return error(GSE_INVALIDOPERATION);
    }

    if (count >= p_timerindex) {
        // TODO: xGSImpl::GatherTimers
    }

    p_timerindex = 0;

    return GS_TRUE;
}


IxGS xGSImpl::create()
{
    if (!gs) {
        gs = new xGSImpl();
    }

    gs->AddRef();
    return gs;
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

void xGSImpl::RenderTargetSize(GSsize &size)
{
    // TODO: xGSImpl::RenderTargetSize
    //size = p_rendertarget ? p_rendertarget->size() : ;
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

void xGSImpl::DrawImmediatePrimitives(xGSGeometryBufferImpl *buffer)
{
    // TODO: think about MultiDraw implementation for this

    for (size_t n = 0; n < buffer->immediateCount(); ++n) {
        const xGSGeometryBufferImpl::Primitive &p = buffer->immediatePrimitive(n);

        if (p.indexcount == 0) {
            // TODO
        } else {
            // TODO
        }
    }
}
