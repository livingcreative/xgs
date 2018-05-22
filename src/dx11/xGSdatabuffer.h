/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    dx11/xGSdatabuffer.h
        DataBuffer object implementation class header
            this object wraps buffer for holding constant data which is used
            by shaders in state object
*/

#pragma once

#include "xGSimplbase.h"


struct ID3D11Buffer;


namespace xGS
{

    // data buffer object
    class xGSDataBufferImpl : public xGSObjectBase<xGSDataBufferBase, xGSImpl>
    {
    public:
        xGSDataBufferImpl(xGSImpl *owner);
        ~xGSDataBufferImpl() override;

    public:
        GSbool AllocateImpl(const GSdatabufferdescription &desc, GSuint totalsize);

        void UpdateImpl(GSuint offset, GSuint size, const GSptr data);
        GSptr LockImpl(GSdword access);
        void UnlockImpl();

        void ReleaseRendererResources();

    private:
        ID3D11Buffer *p_buffer;
    };

} // namespace xGS
