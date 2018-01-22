/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    opengl/xGSdatabuffer.cpp
        DataBuffer object implementation class
*/

#include "xGSdatabuffer.h"
#include "xGSstate.h"


using namespace xGS;


xGSDataBufferImpl::xGSDataBufferImpl(xGSImpl *owner) :
    xGSObjectBase(owner),
    p_buffer(0),
    p_target(0)
{}

xGSDataBufferImpl::~xGSDataBufferImpl()
{}

GSbool xGSDataBufferImpl::AllocateImpl(const GSdatabufferdescription &desc, GSuint totalsize)
{
    switch (desc.type) {
        case GSDT_UNIFORM: p_target = GL_UNIFORM_BUFFER; break;
    }

    glGenBuffers(1, &p_buffer);
    glBindBuffer(p_target, p_buffer);
#ifdef GS_CONFIG_BUFFER_STORAGE
    glBufferStorage(p_target, p_size, nullptr, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
#else
    glBufferData(p_target, p_size, nullptr, GL_STREAM_DRAW);
#endif

    return GS_TRUE;
}

void xGSDataBufferImpl::UpdateImpl(GSuint offset, GSuint size, const GSptr data)
{
    glBindBuffer(p_target, p_buffer);
    glBufferSubData(p_target, GLintptr(offset), size, data);
}

GSptr xGSDataBufferImpl::LockImpl(GSdword access)
{
    p_locktype = GS_LOCKED;

    // TODO: access

    glBindBuffer(p_target, p_buffer);
    return glMapBuffer(p_target, GL_WRITE_ONLY);
}

void xGSDataBufferImpl::UnlockImpl()
{
    p_locktype = GS_NONE;

    glBindBuffer(p_target, p_buffer);
    glUnmapBuffer(p_target);
}

void xGSDataBufferImpl::ReleaseRendererResources()
{
    if (p_buffer) {
        glDeleteBuffers(1, &p_buffer);
    }
    p_target = 0;
    p_size = 0;
}
