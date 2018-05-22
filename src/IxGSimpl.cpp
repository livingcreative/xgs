/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    IxGSimpl.cpp
        xGS API object public interface implementation
*/

#pragma once

#include "IxGSimpl.h"
#include "xGStexture.h"
#include "xGSstate.h"
#include "xGSparameters.h"


using namespace xGS;


IxGSDataBufferImpl::IxGSDataBufferImpl(xGSImpl *owner) :
    xGSObjectImpl(owner)
{
    p_owner->debug(DebugMessageLevel::Information, "DataBuffer object created\n");
}

IxGSDataBufferImpl::~IxGSDataBufferImpl()
{
    ReleaseRendererResources();
    p_owner->debug(DebugMessageLevel::Information, "DataBuffer object destroyed\n");
}

GSbool IxGSDataBufferImpl::allocate(const GSdatabufferdescription &desc)
{
    switch (desc.type) {
        case GSDT_UNIFORM: break;
        default:
            return p_owner->error(GSE_INVALIDENUM);
    }

    p_size = 0;

    if (!desc.blocks) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    // TODO: refactor caps for all implementations
    GSuint alignment = p_owner->caps().ubo_alignment - 1;

    const GSuniformblock *block = desc.blocks;
    while (block->type != GSB_END) {
        UniformBlock uniformblock = {
            p_size,                    // offset
            0,                         // size
            0,                         // actual size
            block->count,              // count
            GSuint(p_uniforms.size()), // firstuniform
            GSuint(p_uniforms.size())  // onepastlastuniform
        };

        const GSuniform *uniform = block->uniforms;
        GSuint uniformoffset = 0;
        while (uniform->type != GSU_END) {
            Uniform u = {
                uniform->type, // type
                uniformoffset, // offset
                0,             // size
                0,             // stride
                0,             // totalsize
                uniform->count // count
            };

            // TODO: check stride values
            switch (uniform->type) {
                case GSU_SCALAR: u.size = 4; u.stride = 16; break;
                case GSU_VEC2:   u.size = 8; u.stride = 16; break;
                case GSU_VEC3:   u.size = 16; u.stride = 16; break;
                case GSU_VEC4:   u.size = 16; u.stride = 16; break;
                case GSU_MAT2:   u.size = 32; u.stride = 32; break;
                case GSU_MAT3:   u.size = 48; u.stride = 48; break;
                case GSU_MAT4:   u.size = 64; u.stride = 64; break;
                default:
                    return p_owner->error(GSE_INVALIDENUM);
            }

            // TODO: adjust array paddings
            u.totalsize = u.stride * uniform->count;
            uniformoffset += u.totalsize;

            uniformblock.size += u.totalsize;
            ++uniformblock.onepastlastuniform;

            p_uniforms.push_back(u);

            ++uniform;
        }

        uniformblock.actualsize = uniformblock.size;
        uniformblock.size = (uniformblock.size + alignment) & ~alignment;
        GSuint blocksize = uniformblock.size * block->count;

        p_blocks.push_back(uniformblock);

        p_size += blocksize;

        ++block;
    }

    if (p_size == 0) {
        return GS_FALSE;
    }

    if (!AllocateImpl(desc, p_size)) {
        return p_owner->error(GSE_OUTOFRESOURCES);
    }

    return p_owner->error(GS_OK);
}

GSvalue IxGSDataBufferImpl::GetValue(GSenum valuetype)
{
    p_owner->error(GSE_UNIMPLEMENTED);
    return 0;
}

GSbool IxGSDataBufferImpl::Update(GSuint offset, GSuint size, const GSptr data)
{
    if (p_size == 0) {
        return p_owner->error(GSE_INVALIDOPERATION);
    }

    UpdateImpl(offset, size, data);

    return p_owner->error(GS_OK);
}

GSbool IxGSDataBufferImpl::UpdateBlock(GSuint block, GSuint index, const GSptr data)
{
    if (p_size == 0) {
        return p_owner->error(GSE_INVALIDOPERATION);
    }

    if (block >= p_blocks.size()) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    const UniformBlock &ub = p_blocks[block];

    if (index >= ub.count) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    UpdateImpl(ub.offset + ub.size * index, ub.actualsize, data);

    return p_owner->error(GS_OK);
}

GSbool IxGSDataBufferImpl::UpdateValue(GSuint block, GSuint index, GSuint uniform, GSuint uniformindex, GSuint count, const GSptr data)
{
    if (p_size == 0) {
        return p_owner->error(GSE_INVALIDOPERATION);
    }

    if (block >= p_blocks.size()) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    const UniformBlock &ub = p_blocks[block];

    if (index >= ub.count) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    if ((uniform + ub.firstuniform) >= ub.onepastlastuniform) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    const Uniform &u = p_uniforms[ub.firstuniform + uniform];

    if (uniformindex >= u.count) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    GSuint offset = ub.offset + ub.size * index + u.offset + u.stride * uniformindex;

    UpdateImpl(offset, u.stride * count, data);

    return p_owner->error(GS_OK);
}

GSptr IxGSDataBufferImpl::Lock(GSdword access, void *lockdata)
{
    if (p_locktype) {
        p_owner->error(GSE_INVALIDOPERATION);
        return nullptr;
    }

    p_owner->error(GS_OK);
    return LockImpl(access);
}

GSbool IxGSDataBufferImpl::Unlock()
{
    if (p_locktype == GS_NONE) {
        return p_owner->error(GSE_INVALIDOPERATION);
    }

    UnlockImpl();

    return p_owner->error(GS_OK);
}



IxGSFrameBufferImpl::IxGSFrameBufferImpl(xGSImpl *owner) :
    xGSObjectImpl(owner)
{
    p_owner->debug(DebugMessageLevel::Information, "FrameBuffer object created\n");
}

IxGSFrameBufferImpl::~IxGSFrameBufferImpl()
{
    ReleaseRendererResources();
    p_owner->debug(DebugMessageLevel::Information, "FrameBuffer object destroyed\n");
}

GSbool IxGSFrameBufferImpl::allocate(const GSframebufferdescription &desc)
{
    // set description
    p_width = desc.width;
    p_height = desc.height;
    p_colortargets = umin(desc.colortargets, GSuint(GS_MAX_FB_COLORTARGETS));
    for (size_t n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        p_colorformat[n] = desc.colorformat[n];
    }
    p_depthstencilformat = desc.depthstencilformat;
    p_multisample = desc.multisample;

    if (p_width == 0 || p_height == 0) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    // initialize attachments
    if (desc.attachments) {
        const GSframebufferattachment *attachment = desc.attachments;
        while (attachment->attachment != GSA_END) {
            // TODO: replace getID() with common resource allocation check function
            if (attachment->texture && static_cast<xGSTextureImpl*>(attachment->texture)->getID() == 0) {
                return p_owner->error(GSE_INVALIDOBJECT);
            }

            switch (attachment->attachment) {
                case GSA_COLOR0:
                case GSA_COLOR1:
                case GSA_COLOR2:
                case GSA_COLOR3:
                case GSA_COLOR4:
                case GSA_COLOR5:
                case GSA_COLOR6:
                case GSA_COLOR7:
                {
                    GSuint n = attachment->attachment - GSA_COLOR0;
                    GSenum fmt = p_colorformat[n];
                    if (fmt != GS_COLOR_DEFAULT) {
                        return p_owner->error(GSE_INVALIDVALUE);
                    }

                    p_colortextures[n].attach(
                        static_cast<xGSTextureImpl*>(attachment->texture),
                        attachment->level, attachment->slice
                    );
                }
                break;

                case GSA_DEPTH:
                    if (p_depthstencilformat != GS_DEPTH_DEFAULT) {
                        return p_owner->error(GSE_INVALIDVALUE);
                    }

                    p_depthtexture.attach(
                        static_cast<xGSTextureImpl*>(attachment->texture),
                        attachment->level, attachment->slice
                    );

                    break;

                case GSA_STENCIL:

                default:
                    return p_owner->error(GSE_INVALIDENUM);
            }

            ++attachment;
        }
    }

    // check for multisample textures
    // texture samples value should match frame buffer's samples value, or should be 0
    bool incomplete = false;
    int attachmentcount = 0;
    int multisampled_attachments = 0;
    int srgb_attachments = 0;

    for (GSuint n = 0; n < p_colortargets; ++n) {
        checkAttachment(
            p_colorformat[n], p_colortextures[n], incomplete,
            attachmentcount, multisampled_attachments, srgb_attachments
        );
    }
    checkAttachment(
        p_depthstencilformat, p_depthtexture, incomplete,
        attachmentcount, multisampled_attachments, srgb_attachments
    );

    if (incomplete) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    if (srgb_attachments != 0 && srgb_attachments != p_colortargets) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    if (multisampled_attachments != 0 && multisampled_attachments != attachmentcount) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    p_srgb = srgb_attachments > 0;

    AllocateImpl(multisampled_attachments);

    return p_owner->error(GS_OK);
}

GSvalue IxGSFrameBufferImpl::GetValue(GSenum valuetype)
{
    switch (valuetype) {
        case GS_FB_TYPE:          return GS_FBTYPE_OFFSCREEN;
        case GS_FB_WIDTH:         return p_width;
        case GS_FB_HEIGHT:        return p_height;
        case GS_FB_DOUBLEBUFFER:  return GS_FALSE;
        case GS_FB_COLORTARGETS:  return p_colortargets;
        case GS_FB_DEPTHFORMAT:   return p_depthstencilformat;
        case GS_FB_STENCILFORMAT: return GS_STENCIL_NONE;
        case GS_FB_MULTISAMPLE:   return p_multisample;

        case GS_FB_COLORFORMAT0:
        case GS_FB_COLORFORMAT1:
        case GS_FB_COLORFORMAT2:
        case GS_FB_COLORFORMAT3:
        case GS_FB_COLORFORMAT4:
        case GS_FB_COLORFORMAT5:
        case GS_FB_COLORFORMAT6:
        case GS_FB_COLORFORMAT7:
            return p_colorformat[valuetype - GS_FB_COLORFORMAT0];
    }

    p_owner->error(GSE_INVALIDENUM);
    return 0;
}

void IxGSFrameBufferImpl::checkAttachment(GSenum format, const Attachment &att, bool &incomplete, int &attachments, int &multiampled_attachments, int &srgb_attachments)
{
    switch (format) {
        case GS_DEFAULT:
            ++attachments;

            if (att.p_texture == nullptr) {
                incomplete = true;
                return;
            }

            if (att.p_texture->samples() != 0 && p_multisample == 0) {
                incomplete = true;
                return;
            }

            if (att.p_texture->samples() != 0 && att.p_texture->samples() != p_multisample) {
                incomplete = true;
                return;
            }

            if (att.p_texture->samples()) {
                ++multiampled_attachments;
            }

            if (att.p_texture->format() == GS_COLOR_S_RGBA || att.p_texture->format() == GS_COLOR_S_RGBX) {
                ++srgb_attachments;
            }

            break;

        case GS_COLOR_S_RGBA:
        case GS_COLOR_S_RGBX:
            ++srgb_attachments;
            break;
    }
}



IxGSGeometryBufferImpl::IxGSGeometryBufferImpl(xGSImpl *owner) :
    xGSObjectImpl(owner)
{
    p_owner->debug(DebugMessageLevel::Information, "GeometryBuffer object created\n");
}

IxGSGeometryBufferImpl::~IxGSGeometryBufferImpl()
{
    ReleaseRendererResources();
    p_owner->debug(DebugMessageLevel::Information, "GeometryBuffer object destroyed\n");
}

GSvalue IxGSGeometryBufferImpl::GetValue(GSenum valuetype)
{
    switch (valuetype) {
        case GS_GB_TYPE:              return p_type;
        case GS_GB_VERTEXCOUNT:       return p_vertexcount;
        case GS_GB_VERTEXSIZE:        return 0; // TODO
        case GS_GB_INDEXCOUNT:        return p_indexcount;
        case GS_GB_INDEXFORMAT:       return p_indexformat;
        case GS_GB_VERTEXBYTES:       return 0; // TODO
        case GS_GB_INDEXBYTES:        return 0; // TODO
        case GS_GB_ACCESS:            return 0; // TODO
        case GS_GB_VERTICESALLOCATED: return p_currentvertex;
        case GS_GB_INDICESALLOCATED:  return p_currentindex;
    }

    p_owner->error(GSE_INVALIDENUM);
    return 0;
}

GSptr IxGSGeometryBufferImpl::Lock(GSenum locktype, GSdword access, void *lockdata)
{
    if (p_locktype || p_type == GS_GBTYPE_IMMEDIATE) {
        p_owner->error(GSE_INVALIDOPERATION);
        return nullptr;
    }

    if (locktype != GS_LOCK_VERTEXDATA && locktype != GS_LOCK_INDEXDATA) {
        p_owner->error(GSE_INVALIDENUM);
        return nullptr;
    }

    if (locktype == GS_LOCK_INDEXDATA && p_indexformat == GS_INDEX_NONE) {
        p_owner->error(GSE_INVALIDVALUE);
        return nullptr;
    }

    p_owner->error(GS_OK);
    return LockImpl(
        locktype, 0,
        locktype == GS_LOCK_VERTEXDATA ?
        p_vertexdecl.buffer_size(p_vertexcount) :
        index_buffer_size(p_indexformat, p_indexcount)
    );
}

GSbool IxGSGeometryBufferImpl::Unlock()
{
    if (p_locktype == GS_NONE || p_locktype == LOCK_IMMEDIATE) {
        return p_owner->error(GSE_INVALIDOPERATION);
    }

    UnlockImpl();

    return p_owner->error(GS_OK);
}

void IxGSGeometryBufferImpl::BeginImmediateDrawing()
{
    p_currentvertex = 0;
    p_currentindex = 0;

    p_primitives.clear();

    p_locktype = LOCK_IMMEDIATE;

    BeginImmediateDrawingImpl();
}

void IxGSGeometryBufferImpl::EndImmediateDrawing()
{
    EndImmediateDrawingImpl();

    p_vertexptr = nullptr;
    p_indexptr = nullptr;
    p_locktype = GS_NONE;

    // NOTE: primitives list isn't released here!!!
    //         because after calling EndImmediateDrawing
    //         implementation should issue rendering commands to
    //         render cached primitives
}



IxGSInputImpl::IxGSInputImpl(xGSImpl *owner) :
    xGSObjectImpl(owner)
{
    p_owner->debug(DebugMessageLevel::Information, "Input object created\n");
}

IxGSInputImpl::~IxGSInputImpl()
{
    ReleaseRendererResources();
    p_owner->debug(DebugMessageLevel::Information, "Input object destroyed\n");
}

GSbool IxGSInputImpl::allocate(const GSinputdescription &desc)
{
    if (!desc.bindings) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    xGSStateImpl *state = static_cast<xGSStateImpl*>(desc.state);
    if (!state) {
        return p_owner->error(GSE_INVALIDOBJECT);
    }

    GSuint elementbuffers = 0;
    p_buffers.resize(state->inputCount());
    for (auto &b : p_buffers) {
        b = nullptr;
    }

    const GSinputbinding *binding = desc.bindings;
    while (binding->slot != GSIS_END) {
        if (GSuint(binding->slot - GSIS_0) >= state->inputCount()) {
            // invalid slot index
            return p_owner->error(GSE_INVALIDVALUE);
        }

        if (state->input(binding->slot - GSIS_0).buffer) {
            // slot already bound to static buffer
            return p_owner->error(GSE_INVALIDVALUE);
        }

        xGSGeometryBufferImpl *buffer = static_cast<xGSGeometryBufferImpl*>(binding->buffer);

        // TODO: check layout match

        if (buffer->indexFormat() != GS_INDEX_NONE) {
            ++elementbuffers;
        }

        p_buffers[binding->slot - GSIS_0] = buffer;
        ++binding;
    }

    if (p_buffers.size() != state->inputAvailable() || elementbuffers > 1) {
        // TODO: reconsider error code
        return p_owner->error(GSE_INVALIDOPERATION);
    }

    p_state = state;
    p_state->AddRef();

    p_primarybuffer = p_buffers[p_state->inputPrimarySlot()];

    if (!AllocateImpl(elementbuffers)) {
        return p_owner->error(GSE_OUTOFRESOURCES);
    }

    return p_owner->error(GS_OK);
}



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
        GS_CREATE_OBJECT(GS_OBJECTTYPE_GEOMETRY, IxGSGeometryImpl, GSgeometrydescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_GEOMETRYBUFFER, IxGSGeometryBufferImpl, GSgeometrybufferdescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_DATABUFFER, IxGSDataBufferImpl, GSdatabufferdescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_TEXTURE, xGSTextureImpl, GStexturedescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_FRAMEBUFFER, IxGSFrameBufferImpl, GSframebufferdescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_STATE, xGSStateImpl, GSstatedescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_INPUT, IxGSInputImpl, GSinputdescription)
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

    IxGSGeometryBufferImpl *bufferimpl = static_cast<IxGSGeometryBufferImpl*>(buffer);

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

    IxGSGeometryBufferImpl *bufferimpl = static_cast<IxGSGeometryBufferImpl*>(buffer);
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

    bufferimpl->BeginImmediateDrawing();

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
        // TODO: this cast is not very nice...
        IxGSGeometryBufferImpl *bufferimpl = static_cast<IxGSGeometryBufferImpl*>(p_immediatebuffer);

        // requested primitive can not be added, try to flush buffer
        bufferimpl->EndImmediateDrawing();
        DrawImmediatePrimitives(p_immediatebuffer);
        bufferimpl->BeginImmediateDrawing();

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

    // TODO: this cast is not very nice...
    IxGSGeometryBufferImpl *bufferimpl = static_cast<IxGSGeometryBufferImpl*>(p_immediatebuffer);

    bufferimpl->EndImmediateDrawing();
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
        IxGSGeometryImpl *geom = static_cast<IxGSGeometryImpl*>(*geometries++);

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

    IxGSGeometryImpl *geometry = static_cast<IxGSGeometryImpl*>(geometry_to_draw);
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

    SetupGeometryImpl(geometry);

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
        IxGSGeometryImpl *geometry = static_cast<IxGSGeometryImpl*>(geometries_to_draw[n]);

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
