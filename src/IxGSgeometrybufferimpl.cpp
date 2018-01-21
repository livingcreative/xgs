/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    IxGSgeometrybufferimpl.cpp
        GeometryBuffer object public interface implementation
*/

#pragma once

#include "IxGSgeometrybufferimpl.h"


using namespace xGS;


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
    p_owner->error(GSE_UNIMPLEMENTED);
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
