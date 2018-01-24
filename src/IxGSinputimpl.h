/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    IxGSinputimpl.h
        Input object public interface implementation
*/

#pragma once

#include "xGSinput.h"


namespace xGS
{

    // input object
    class IxGSInputImpl : public xGSObjectImpl<xGSInputImpl, IxGSInputImpl>
    {
    public:
        IxGSInputImpl(xGSImpl *owner);
        ~IxGSInputImpl() override;

    public:
        GSbool allocate(const GSinputdescription &desc);
    };

} // namespace xGS
