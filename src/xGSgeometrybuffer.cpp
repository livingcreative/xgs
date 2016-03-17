#include "xGSgeometrybuffer.h"


xGSgeometrybufferImpl::xGSgeometrybufferImpl() :
    p_vertexbuffer(0),
    p_indexbuffer(0),
    p_locked(NONE),
    p_indexformat(GS_INDEX_NONE),
    p_vertexcount(0),
    p_indexcount(0),
    p_currentvertex(0),
    p_currentindex(0)
{}

xGSgeometrybufferImpl::~xGSgeometrybufferImpl()
{
    if (p_locked) {
        DoUnlock();
    }

    if (p_vertexbuffer) {
        glDeleteBuffers(1, &p_vertexbuffer);
    }
    if (p_indexbuffer) {
        glDeleteBuffers(1, &p_indexbuffer);
    }
}

void *xGSgeometrybufferImpl::Lock(GSlockbuffer buffer, unsigned int flags)
{
    // TODO: implement MapBufferRange

    GLenum acces = 0;

    if (flags == GS_READ) {
        acces = GL_READ_ONLY;
    }
    if (flags == GS_WRITE) {
        acces = GL_WRITE_ONLY;
    }
    if (flags == (GS_READ | GS_WRITE)) {
        acces = GL_READ_WRITE;
    }

    switch (buffer) {
        case GS_LOCK_VERTEXBUFFER:
            p_locked = VERTEX;
            glBindBuffer(GL_ARRAY_BUFFER, p_vertexbuffer);
            return glMapBuffer(GL_ARRAY_BUFFER, acces);
            break;

        case GS_LOCK_INDEXBUFFER:
            if (p_indexbuffer) {
                p_locked = INDEX;
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_indexbuffer);
                return glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, acces);
            }
            break;
    }

    // TODO: implement error codes
    return nullptr;
}

bool xGSgeometrybufferImpl::Unlock()
{
    if (!p_locked) {
        // TODO: implement error codes
        return false;
    }

    DoUnlock();

    return true;
}

bool xGSgeometrybufferImpl::Allocate(const GSgeometrybufferdesc &desc)
{
    const GSvertexcomponent *comp = desc.components;
    GLsizei vertezsize = 0;
    while (comp->type != GS_LAST_COMPONENT) {
        switch (comp->type) {
            case GS_FLOAT:
                vertezsize += sizeof(float);
                break;

            case GS_VEC2:
                vertezsize += sizeof(float) * 2;
                break;

            case GS_VEC3:
                vertezsize += sizeof(float) * 3;
                break;

            case GS_VEC4:
                vertezsize += sizeof(float) * 4;
                break;

            default:
                // TODO: implement error codes
                return false;
        }

        ++comp;
    }

    GLsizei indexsize = 0;
    p_indexformat = desc.indexformat;
    switch (desc.indexformat) {
        case GS_INDEX_WORD: indexsize = sizeof(GLushort); break;
        case GS_INDEX_DWORD: indexsize = sizeof(GLuint); break;
    }

    // TODO: consider desc.flags
    // TODO: use BufferStorage, if supported

    glGenBuffers(1, &p_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, p_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, desc.vertexcount * vertezsize, nullptr, GL_STATIC_DRAW);

    // don't allocate index buffer if index format is GS_INDEX_NONE
    if (indexsize) {
        glGenBuffers(1, &p_indexbuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_indexbuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, desc.indexcount * indexsize, nullptr, GL_STATIC_DRAW);
    }

    p_vertexcount = desc.vertexcount;
    p_indexcount = desc.indexformat == GS_INDEX_NONE ? 0 : desc.indexcount;
    p_currentvertex = 0;
    p_currentindex = 0;

    return true;
}

bool xGSgeometrybufferImpl::AllocateGeometry(unsigned int vertexcount, unsigned int indexcount, /* out */ unsigned int &basevertex, /* out */ unsigned int &baseindex)
{
    if ((vertexcount + p_currentvertex) > p_vertexcount) {
        return false;
    }

    if ((indexcount + p_currentindex) > p_indexcount) {
        return false;
    }

    basevertex = p_currentvertex;
    baseindex = p_currentindex;

    p_currentvertex += vertexcount;
    p_currentindex += indexcount;

    return true;
}

void xGSgeometrybufferImpl::DoUnlock()
{
    // NOTE: call this after all state has been checked
    switch (p_locked) {
        case GS_LOCK_VERTEXBUFFER:
            glBindBuffer(GL_ARRAY_BUFFER, p_vertexbuffer);
            glUnmapBuffer(GL_ARRAY_BUFFER);
            break;

        case GS_LOCK_INDEXBUFFER:
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_indexbuffer);
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
            break;
    }

    p_locked = NONE;
}
