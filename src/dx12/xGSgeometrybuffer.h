/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    dx12/xGSgeometrybuffer.h
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

        //ID3D12Buffer* vertexbuffer() const { return p_vertexbuffer; }
        //ID3D12Buffer* indexbuffer() const { return p_indexbuffer; }

        GSptr LockImpl(GSenum locktype, size_t offset, size_t size);
        void UnlockImpl();

        void BeginImmediateDrawingImpl();
        void EndImmediateDrawingImpl();

        void ReleaseRendererResources();

    private:
        ID3D12Resource *p_vertexbuffer;
        ID3D12Resource *p_indexbuffer;
        ID3D12Resource *p_lockbuffer;
        void           *p_lockmemory;
        size_t          p_lockoffset;
        size_t          p_locksize;
    };

} // namespace xGS
