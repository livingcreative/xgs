/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    opengl/xGSgeometrybuffer.h
        GeometryBuffer object implementation class header
            this object wraps buffer objects for storing geometry (mesh) data
            and format for vertex and index data
*/

#pragma once

#include "xGSobject.h"
#include "xGSgeometrybufferbase.h"


namespace xGS
{

    // geometry buffer object
    class xGSGeometryBufferImpl : public xGSObjectImpl<xGSGeometryBufferBase, xGSGeometryBufferImpl>
    {
    public:
        xGSGeometryBufferImpl(xGSImpl *owner);
        ~xGSGeometryBufferImpl() override;

    public:
        GSvalue xGSAPI GetValue(GSenum valuetype) override;

        GSptr   xGSAPI Lock(GSenum locktype, GSdword access, void *lockdata) override;
        GSbool  xGSAPI Unlock() override;

    public:
        GSbool allocate(const GSgeometrybufferdescription &desc);

        GLuint getVertexBufferID() const { return p_vertexbuffer; }
        GLuint getIndexBufferID() const { return p_indexbuffer; }

        GSptr lock(GSenum locktype, size_t offset, size_t size);
        void unlock();

        void BeginImmediateDrawing();
        void EndImmediateDrawing();

        void ReleaseRendererResources();

    private:
        GLuint p_vertexbuffer;
        GLuint p_indexbuffer;
    };

} // namespace xGS
