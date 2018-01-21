/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    xGSobject.h
        base class template for all xGS object types
*/

#pragma once

#include "xGSimpl.h"


namespace xGS
{

    template <typename T>
    class xGSObjectBase : public xGSIUnknownImpl<T>
    {
    public:
        xGSObjectBase() :
            p_objecttype(GS_NONE),
            p_owner(nullptr)
        {}

        xGSObjectBase(xGSImpl *owner) :
            p_objecttype(GS_NONE),
            p_owner(owner)
        {}

        ~xGSObjectBase() override
        {}

        void DetachFromRenderer()
        {
            p_owner = nullptr;
        }

        GSenum objecttype() const { return p_objecttype; }

    protected:
        GSenum   p_objecttype;
        xGSImpl *p_owner;
    };

    template <typename baseT, typename selfT>
    class xGSObjectImpl : public baseT
    {
    public:
        xGSObjectImpl(xGSImpl *owner) :
            baseT(owner)
        {
            p_owner = owner;
            p_owner->AddObject<selfT>(static_cast<selfT*>(this));
        }

        ~xGSObjectImpl() override
        {
            if (p_owner) {
                p_owner->RemoveObject<selfT>(static_cast<selfT*>(this));
            }
        }

        static selfT* create(xGSImpl *owner, GSenum type)
        {
            selfT *result = new selfT(owner);
            result->p_objecttype = type;
            result->AddRef();
            return result;
        }
    };

    class xGSUnknownObjectImpl : public xGSObjectImpl<xGSObjectBase<xGSObject>, xGSUnknownObjectImpl>
    {};

} // namespace xGS
