﻿/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    dx11/xGSgeometrybuffer.h
        GeometryBuffer object implementation class header
            this object wraps buffer objects for storing geometry (mesh) data
            and format for vertex and index data
*/

#pragma once

#include "xGSimplbase.h"


struct ID3D11Buffer;


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

        ID3D11Buffer* vertexbuffer() const { return p_vertexbuffer; }
        ID3D11Buffer* indexbuffer() const { return p_indexbuffer; }

        GSptr LockImpl(GSenum locktype, size_t offset, size_t size);
        void UnlockImpl();

        void BeginImmediateDrawingImpl();
        void EndImmediateDrawingImpl();

        void ReleaseRendererResources();

    private:
        ID3D11Buffer *p_vertexbuffer;
        ID3D11Buffer *p_indexbuffer;

        char         *p_lockmemory;
        size_t        p_lockoffset;
        size_t        p_locksize;
    };

} // namespace xGS
