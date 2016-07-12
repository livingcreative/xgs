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
    class xGSObjectImpl : public xGSIUnknownImpl<baseT>
    {
    public:
        xGSObjectImpl() :
            p_owner(nullptr)
        {}

        xGSObjectImpl(xGSImpl *owner) :
            p_owner(owner)
        {
            p_owner->AddObject<selfT>(static_cast<selfT*>(this));
        }

        ~xGSObjectImpl() override
        {
            if (p_owner) {
                p_owner->RemoveObject<selfT>(static_cast<selfT*>(this));
            }
        }

        void DetachFromRenderer()
        {
            p_owner = nullptr;
        }

        GSenum objecttype() const { return p_objecttype; }

        static selfT* create(xGSImpl *owner, GSenum type)
        {
            selfT *result = new selfT(owner);
            result->p_objecttype = type;
            result->AddRef();
            return result;
        }

    protected:
        GSenum   p_objecttype;
        xGSImpl *p_owner;
    };

    class xGSUnknownObjectImpl : public xGSObjectImpl<xGSObject, xGSUnknownObjectImpl>
    {};

} // namespace xGS
