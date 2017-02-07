/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/xgs

    dx12/xGSgeometrybuffer.h
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

        //ID3D12Buffer* vertexbuffer() const { return p_vertexbuffer; }
        //ID3D12Buffer* indexbuffer() const { return p_indexbuffer; }

        GSptr lock(GSenum locktype, size_t offset, size_t size);
        void unlock();

        void BeginImmediateDrawing();
        void EndImmediateDrawing();

        void ReleaseRendererResources();

    private:
        //ID3D12Buffer *p_vertexbuffer;
        //ID3D12Buffer *p_indexbuffer;
        char         *p_lockmemory;
        size_t        p_lockoffset;
        size_t        p_locksize;
    };

} // namespace xGS
