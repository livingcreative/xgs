/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    IxGSimpl.cpp
        xGS API object public interface implementation
*/

#pragma once

#include "IxGSImpl.h"
#include "xGSgeometry.h"
#include "xGSgeometrybuffer.h"
#include "xGSdatabuffer.h"
#include "xGStexture.h"
#include "xGSframebuffer.h"
#include "xGSstate.h"
#include "xGSinput.h"
#include "xGSparameters.h"


using namespace xGS;


IxGSImpl::~IxGSImpl()
{
    // TODO: make internal implementation of these End/Destroy funcs
    //       without some unnecessary checks?

    if (p_systemstate == CAPTURE) {
        EndCapture(nullptr);
    }
    if (p_systemstate == RENDERER_READY) {
        DestroyRenderer(true);
    }
}


GSbool IxGSImpl::CreateRenderer(const GSrendererdescription &desc)
{
    if (!ValidateState(SYSTEM_READY, true, false, false)) {
        return GS_FALSE;
    }

    CreateRendererImpl(desc);

    if (p_error != GS_OK) {
        return GS_FALSE;
    }

    p_systemstate = RENDERER_READY;

    return GS_TRUE;
}

GSbool IxGSImpl::DestroyRenderer(GSbool restorevideomode)
{
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    ::Release(p_state);
    ::Release(p_input);
    for (size_t n = 0; n < GS_MAX_PARAMETER_SETS; ++n) {
        ::Release(p_parameters[n]);
    }
    ::Release(p_rendertarget);

    ReleaseObjectList(p_statelist, "State");
    ReleaseObjectList(p_inputlist, "Input");
    ReleaseObjectList(p_parameterslist, "Parameters");
    ReleaseObjectList(p_geometrylist, "Geomtery");
    ReleaseObjectList(p_framebufferlist, "Framebuffer");
    ReleaseObjectList(p_geometrybufferlist, "GeomteryBuffer");
    ReleaseObjectList(p_databufferlist, "DataBuffer");
    ReleaseObjectList(p_texturelist, "Texture");

    DestroyRendererImpl();

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

GSbool IxGSImpl::CreateObject(GSenum type, const void *desc, void **result)
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

GSbool IxGSImpl::CreateSamplers(const GSsamplerdescription *samplers, GSuint count)
{
    GSuint references = 0;
    for (auto &s : p_samplerlist) {
        references += s.refcount;
    }

    if (references) {
        return error(GSE_INVALIDOPERATION);
    }

    // TODO: input validation

    CreateSamplersImpl(samplers, count);

    return p_error == GS_OK;
}

GSbool IxGSImpl::GetRenderTargetSize(GSsize &size)
{
    if (!ValidateState(RENDERER_READY, false, false, false)) {
        return GS_FALSE;
    }

    GetRenderTargetSizeImpl(size);

    return error(GS_OK);
}

GSbool IxGSImpl::Clear(GSbool color, GSbool depth, GSbool stencil, const GScolor &colorvalue, float depthvalue, GSdword stencilvalue)
{
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    ClearImpl(color, depth, stencil, colorvalue, depthvalue, stencilvalue);

    return error(GS_OK);
}

GSbool IxGSImpl::Display()
{
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    DisplayImpl();

    return p_error == GS_OK;
}

GSbool IxGSImpl::SetRenderTarget(IxGSFrameBuffer rendertarget)
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

    SetRenderTargetImpl();

    // reset current state, after RT change any state should be rebound
    SetStateImpl(static_cast<xGSStateImpl*>(nullptr));

    return error(GS_OK);
}

GSbool IxGSImpl::SetState(IxGSState state)
{
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    if (!state) {
        return error(GSE_INVALIDOBJECT);
    }

    xGSStateImpl *stateimpl = static_cast<xGSStateImpl*>(state);

    if (!stateimpl->validate(p_colorformats, p_depthstencilformat)) {
        return error(GSE_INVALIDOBJECT);
    }

    SetStateImpl(stateimpl);

    return error(GS_OK);
}

GSbool IxGSImpl::SetInput(IxGSInput input)
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    xGSInputImpl *inputimpl = static_cast<xGSInputImpl*>(input);
    if (inputimpl == nullptr) {
        return error(GSE_INVALIDOBJECT);
    }

    if (p_state == nullptr || inputimpl->state() != p_state) {
        return error(GSE_INVALIDOPERATION);
    }

    AttachObject(p_caps, p_input, inputimpl);

    return error(GS_OK);
}

GSbool IxGSImpl::SetParameters(IxGSParameters parameters)
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

    if (p_state == nullptr || parametersimpl->state() != p_state) {
        return error(GSE_INVALIDSTATE);
    }

    AttachObject(p_caps, p_parameters[parametersimpl->setindex()], parametersimpl);

    return error(GS_OK);
}

GSbool IxGSImpl::SetViewport(const GSrect &viewport)
{
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    SetViewportImpl(viewport);

    return error(GS_OK);
}

GSbool IxGSImpl::SetStencilReference(GSuint ref)
{
    // TODO: same as SetParameters behaviour
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (!p_state) {
        return error(GSE_INVALIDSTATE);
    }

    SetStencilReferenceImpl(ref);

    return p_error == GS_OK;
}

GSbool IxGSImpl::SetBlendColor(const GScolor &color)
{
    // TODO: same as SetParameters behaviour
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (!p_state) {
        return error(GSE_INVALIDSTATE);
    }

    SetBlendColorImpl(color);

    return error(GS_OK);
}

GSbool IxGSImpl::SetUniformValue(GSenum set, GSenum slot, GSenum type, const void *value)
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
            case GSU_VEC2:
            case GSU_VEC3:
            case GSU_VEC4:
            case GSU_MAT2:
            case GSU_MAT3:
            case GSU_MAT4:
                SetUniformValueImpl(type, paramslot.location, value);
                break;

            default:
                return error(GSE_INVALIDENUM);
        }
    }

    return error(GS_OK);
}

// NOTE:
//      following Draw* functions will compile only in "unity build" mode
//      which is the only build mode now
//      in order this to work in ordinary build (as a separate translation unit)
//      *Drawer structs should be made public in each platform implementation

GSbool IxGSImpl::DrawGeometry(IxGSGeometry geometry)
{
    SimpleDrawer drawer;
    return Draw(geometry, drawer);
}

GSbool IxGSImpl::DrawGeometryInstanced(IxGSGeometry geometry, GSuint count)
{
    InstancedDrawer drawer(count);
    return Draw(geometry, drawer);
}

GSbool IxGSImpl::DrawGeometries(IxGSGeometry *geometries, GSuint count)
{
    SimpleMultiDrawer drawer;
    return MultiDraw(geometries, count, drawer);
}

GSbool IxGSImpl::DrawGeometriesInstanced(IxGSGeometry *geometries, GSuint count, GSuint instancecount)
{
    InstancedMultiDrawer drawer(instancecount);
    return MultiDraw(geometries, count, drawer);
}

GSbool IxGSImpl::BeginCapture(GSenum mode, IxGSGeometryBuffer buffer)
{
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    if (!buffer) {
        return error(GSE_INVALIDOBJECT);
    }

    xGSGeometryBufferImpl *bufferimpl = static_cast<xGSGeometryBufferImpl*>(buffer);

    p_capturebuffer = bufferimpl;
    p_capturebuffer->AddRef();

    BeginCaptureImpl(mode);

    p_systemstate = CAPTURE;

    return error(GS_OK);
}

GSbool IxGSImpl::EndCapture(GSuint *elementcount)
{
    if (!ValidateState(CAPTURE, true, true, false)) {
        return GS_FALSE;
    }

    EndCaptureImpl(elementcount);

    p_capturebuffer->Release();
    p_capturebuffer = nullptr;
    p_systemstate = RENDERER_READY;

    return error(GS_OK);
}

GSbool IxGSImpl::BeginImmediateDrawing(IxGSGeometryBuffer buffer, GSuint flags)
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (!buffer) {
        return error(GSE_INVALIDOBJECT);
    }

    xGSGeometryBufferImpl *bufferimpl = static_cast<xGSGeometryBufferImpl*>(buffer);
    if (bufferimpl->type() != GS_GBTYPE_IMMEDIATE) {
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

GSbool IxGSImpl::ImmediatePrimitive(GSenum type, GSuint vertexcount, GSuint indexcount, GSuint flags, GSimmediateprimitive *primitive)
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

GSbool IxGSImpl::EndImmediateDrawing()
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

GSbool IxGSImpl::BuildMIPs(IxGSTexture texture)
{
    if (!ValidateState(RENDERER_READY, true, false, false)) {
        return GS_FALSE;
    }

    xGSTextureImpl *tex = static_cast<xGSTextureImpl*>(texture);

    BuildMIPsImpl(tex);

    return error(GS_OK);
}

GSbool IxGSImpl::CopyImage(
    IxGSTexture src, GSuint srclevel, GSuint srcx, GSuint srcy, GSuint srcz,
    IxGSTexture dst, GSuint dstlevel, GSuint dstx, GSuint dsty, GSuint dstz,
    GSuint width, GSuint height, GSuint depth
)
{
    // TODO: checks

    xGSTextureImpl *srctex = static_cast<xGSTextureImpl*>(src);
    xGSTextureImpl *dsttex = static_cast<xGSTextureImpl*>(dst);

    CopyImageImpl(
        srctex, srclevel, srcx, srcy, srcz,
        dsttex, dstlevel, dstx, dsty, dstz,
        width, height, depth
    );

    return error(GS_OK);
}

GSbool IxGSImpl::CopyData(xGSObject *src, xGSObject *dst, GSuint readoffset, GSuint writeoffset, GSuint size, GSuint flags)
{
    if (!ValidateState(RENDERER_READY, true, false, false)) {
        return GS_FALSE;
    }

    if (src == nullptr || dst == nullptr) {
        return error(GSE_INVALIDOBJECT);
    }

    // TODO: validate src/dst objects?

    CopyDataImpl(src, dst, readoffset, writeoffset, size, flags);

    return p_error == GS_OK;
}

GSbool IxGSImpl::BufferCommitment(xGSObject *buffer, GSuint offset, GSuint size, GSbool commit, GSuint flags)
{
    if (!ValidateState(RENDERER_READY, true, false, false)) {
        return GS_FALSE;
    }

    if (buffer == nullptr) {
        return error(GSE_INVALIDOBJECT);
    }

    // TODO: check for support, check for buffer is sparse
    //       check type

    BufferCommitmentImpl(buffer, offset, size, commit, flags);

    return p_error == GS_OK;
}

GSbool IxGSImpl::GeometryBufferCommitment(IxGSGeometryBuffer buffer, IxGSGeometry *geometries, GSuint count, GSbool commit)
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

    GeometryBufferCommitmentImpl(impl);

    for (GSuint n = 0; n < count; ++n) {
        xGSGeometryImpl *geom = static_cast<xGSGeometryImpl*>(*geometries++);

        if (geom->buffer() != impl) {
            // skip
            // TODO: think about skipping or error...
            continue;
        }

        GeometryBufferCommitmentGeometry(geom, vertexsize, indexsize, commit);
    }

    return error(GS_OK);
}

GSbool IxGSImpl::TextureCommitment(IxGSTexture texture, GSuint level, GSuint x, GSuint y, GSuint z, GSuint width, GSuint height, GSuint depth, GSbool commit)
{
    if (!ValidateState(RENDERER_READY, true, false, false)) {
        return GS_FALSE;
    }

    if (texture == nullptr) {
        return error(GSE_INVALIDOBJECT);
    }

    xGSTextureImpl *tex = static_cast<xGSTextureImpl*>(texture);

    TextureCommitmentImpl(tex, level, x, y, z, width, height, depth, commit);

    return p_error == GS_OK;
}

GSbool IxGSImpl::Compute(IxGSComputeState state, GSuint x, GSuint y, GSuint z)
{
    // TODO:
    return GS_FALSE;
}

GSbool IxGSImpl::BeginTimerQuery()
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    BeginTimerQueryImpl();

    return error(GS_OK);
}

GSbool IxGSImpl::EndTimerQuery()
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    EndTimerQueryImpl();

    return p_error == GS_OK;
}

GSbool IxGSImpl::TimestampQuery()
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    TimestampQueryImpl();

    return error(GS_OK);
}

GSbool IxGSImpl::GatherTimers(GSuint flags, GSuint64 *values, GSuint count)
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    GatherTimersImpl(flags, values, count);

    return p_error == GS_OK;
}



IxGS IxGSImpl::create()
{
    if (!gs) {
        gs = new IxGSImpl();
    }

    gs->AddRef();
    return gs;
}



void IxGSImpl::SetStateImpl(xGSStateImpl *state)
{
    AttachObject(p_caps, p_state, state);
    AttachObject(p_caps, p_input, static_cast<xGSInputImpl*>(nullptr));
    for (size_t n = 0; n < GS_MAX_PARAMETER_SETS; ++n) {
        AttachObject(p_caps, p_parameters[n], static_cast<xGSParametersImpl*>(nullptr));
    }
#ifdef _DEBUG
    if (!p_state) {
        xGSStateImpl::bindNullProgram();
    }
#endif
}

template <typename T>
GSbool IxGSImpl::Draw(IxGSGeometry geometry_to_draw, const T &drawer)
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (!geometry_to_draw) {
        return error(GSE_INVALIDOBJECT);
    }

    if (!p_state) {
        return error(GSE_INVALIDSTATE);
    }

    xGSGeometryImpl *geometry = static_cast<xGSGeometryImpl*>(geometry_to_draw);
    xGSGeometryBufferImpl *buffer = geometry->buffer();

#ifdef _DEBUG
    size_t primaryslot = p_state->inputPrimarySlot();
    xGSGeometryBufferImpl *boundbuffer =
        primaryslot == GS_UNDEFINED ? nullptr : p_state->input(primaryslot).buffer;
    if (p_input && boundbuffer == nullptr)  {
        boundbuffer = p_input->primaryBuffer();
    }
    if (buffer != boundbuffer) {
        return error(GSE_INVALIDOBJECT);
    }
#endif

    geometry->setup();

    if (geometry->indexFormat() == GS_INDEX_NONE) {
        drawer.DrawArrays(geometry->type(), geometry->baseVertex(), geometry->vertexCount());
    } else {
        drawer.DrawElementsBaseVertex(
            geometry->type(),
            geometry->indexCount(), buffer->indexFormat(),
            geometry->indexPtr(), geometry->baseVertex()
        );
    }

    return error(GS_OK);
}

template <typename T>
GSbool IxGSImpl::MultiDraw(IxGSGeometry *geometries_to_draw, GSuint count, const T &drawer)
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (!geometries_to_draw) {
        return error(GSE_INVALIDVALUE);
    }

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
#endif

    GSenum primtype = GS_NONE;
    GSenum indextype = GS_NONE;
    int first[4096];
    int counts[4096];
    GSptr indices[4096];
    size_t current = 0;

    for (GSuint n = 0; n < count; ++n) {
        xGSGeometryImpl *geometry = static_cast<xGSGeometryImpl*>(geometries_to_draw[n]);

#ifdef _DEBUG
        xGSGeometryBufferImpl *buffer = geometry->buffer();
        if (buffer != boundbuffer) {
            return error(GSE_INVALIDOBJECT);
        }
#endif

        first[current] = geometry->baseVertex();
        counts[current] = geometry->vertexCount(); // TODO: index count for indexed
        indices[current] = geometry->indexPtr(); // TODO: only for indexed
        ++current;

        // TODO: geometry setup, geometry type, index type
        //geometry->setup();
    }

    if (indextype == GS_INDEX_NONE) {
        drawer.MultiDrawArrays(primtype, first, counts, count);
    } else {
        drawer.MultiDrawElementsBaseVertex(
            primtype, counts,
            indextype, indices, count, first
        );
    }

    return error(GS_OK);
}


GSbool IxGSImpl::ValidateState(SystemState requiredstate, bool exactmatch, bool matchimmediate, bool requiredimmediate)
{
    if (p_systemstate == SYSTEM_NOTREADY) {
        return error(GSE_SYSTEMNOTREADY);
    }

    if (p_systemstate == SYSTEM_READY && requiredstate > p_systemstate) {
        return error(GSE_RENDERERNOTREADY);
    }

    if (p_systemstate == RENDERER_READY && requiredstate > p_systemstate) {
        return error(GSE_INVALIDOPERATION);
    }

    if (exactmatch && p_systemstate != requiredstate) {
        return error(GSE_INVALIDOPERATION);
    }

    bool immediatemode = p_immediatebuffer != nullptr;
    if (matchimmediate && requiredimmediate != immediatemode) {
        return error(GSE_INVALIDOPERATION);
    }

    return GS_TRUE;
}

template <typename T>
void IxGSImpl::CheckObjectList(const T &list, const std::string &listname)
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
void IxGSImpl::ReleaseObjectList(T &list, const std::string &listname)
{
    CheckObjectList(list, listname);
    for (auto &object : list) {
        object->ReleaseRendererResources();
        object->DetachFromRenderer();
    }
}

template <typename T, typename C>
void IxGSImpl::AttachObject(const C &caps, T &attachpoint, T object)
{
    if (attachpoint) {
        attachpoint->Release();
    }

    attachpoint = object;

    if (attachpoint) {
        attachpoint->AddRef();
        attachpoint->apply(caps);
    }
}
