/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    IxGSdatabufferimpl.h
        DataBuffer object public interface implementation
*/

#pragma once

#include "xGSdatabuffer.h"


namespace xGS
{

    // data buffer object
    class IxGSDataBufferImpl : public xGSObjectImpl<xGSDataBufferImpl, IxGSDataBufferImpl>
    {
    public:
        IxGSDataBufferImpl(xGSImpl *owner);
        ~IxGSDataBufferImpl() override;

    public:
        GSbool allocate(const GSdatabufferdescription &desc);

    public:
        GSvalue xGSAPI GetValue(GSenum valuetype) override;

        GSbool xGSAPI Update(GSuint offset, GSuint size, const GSptr data) override;
        GSbool xGSAPI UpdateBlock(GSuint block, GSuint index, const GSptr data) override;
        GSbool xGSAPI UpdateValue(GSuint block, GSuint index, GSuint uniform, GSuint uniformindex, GSuint count, const GSptr data) override;

        GSptr  xGSAPI Lock(GSdword access, void *lockdata) override;
        GSbool xGSAPI Unlock() override;
    };

} // namespace xGS
