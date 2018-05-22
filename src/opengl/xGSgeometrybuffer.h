/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    opengl/xGSgeometrybuffer.h
        GeometryBuffer object implementation class header
            this object wraps buffer objects for storing geometry (mesh) data
            and format for vertex and index data
*/

#pragma once

#include "xGSimplbase.h"


namespace xGS
{

    // geometry buffer object
    class xGSGeometryBufferImpl : public xGSObjectBase<xGSGeometryBufferBase, xGSImpl>
    {
    public:
        xGSGeometryBufferImpl(xGSImpl *owner);
        ~xGSGeometryBufferImpl() override;

    public:
        GSbool allocate(const GSgeometrybufferdescription &desc);

        GLuint vertexbuffer() const { return p_vertexbuffer; }
        GLuint indexbuffer() const { return p_indexbuffer; }

        GSptr LockImpl(GSenum locktype, size_t offset, size_t size);
        void UnlockImpl();

        void BeginImmediateDrawingImpl();
        void EndImmediateDrawingImpl();

        void ReleaseRendererResources();

    private:
        GLuint p_vertexbuffer;
        GLuint p_indexbuffer;
    };

} // namespace xGS
