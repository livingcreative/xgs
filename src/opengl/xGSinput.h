/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    xGSinput.h
        Input object implementation class header
            this object binds source of geometry data with state object
*/

#pragma once

#include "xGSobject.h"
#include "glplatform.h"
#include <vector>
#include <unordered_map>


namespace xGS
{

    class xGSStateImpl;
    class xGSGeometryImpl;
    class xGSGeometryBufferImpl;


    // input object
    class xGSInputImpl : public xGSObjectImpl<xGSInput, xGSInputImpl>
    {
    public:
        xGSInputImpl(xGSImpl *owner);
        ~xGSInputImpl() override;

    public:
        GSbool allocate(const GSinputdescription &desc);

        xGSStateImpl* state() const { return p_state; }
        GSuint bufferCount() const { return GSuint(p_buffers.size()); }
        xGSGeometryBufferImpl* buffer(GSuint index) const { return p_buffers[index]; }

        xGSGeometryBufferImpl* primaryBuffer() const { return p_primarybuffer; }

        GSbool allocated() const { return p_allocated; }

        void apply(const GScaps &caps);

        void ReleaseRendererResources();

    private:
        typedef std::vector<xGSGeometryBufferImpl*> GeometryBufferList;
        typedef std::vector<GLuint> GLuintList;

    private:
        GSbool                 p_allocated;
        xGSStateImpl          *p_state;
        xGSGeometryBufferImpl *p_primarybuffer;
        GeometryBufferList     p_buffers;
        GLuintList             p_vertexbuffers;
        GLuintList             p_strides;
        GLuintList             p_slots;
        GLuint                 p_indexbuffer;
        GLuint                 p_vertexarray;
    };

} // namespace xGS