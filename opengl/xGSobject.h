/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    xGSobject.h
        base class template for all xGS object types
*/

#pragma once

#include "xGSimpl.h"


namespace xGS
{

    template <typename baseT, typename selfT>
    class xGSObject : public xGSIUnknownImpl<baseT>
    {
    public:
        xGSObject() :
            p_owner(nullptr)
        {}

        xGSObject(xGSImpl *owner) :
            p_owner(owner)
        {
            p_owner->AddObject<selfT>(static_cast<selfT*>(this));
        }

        ~xGSObject() override
        {
            if (p_owner) {
                p_owner->RemoveObject<selfT>(static_cast<selfT*>(this));
            }
        }

        void DetachFromRenderer()
        {
            p_owner = nullptr;
        }

        static selfT* create(xGSImpl *owner)
        {
            selfT *result = new selfT(owner);
            result->AddRef();
            return result;
        }

    protected:
        xGSImpl *p_owner;
    };

} // namespace xGS
