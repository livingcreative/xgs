/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    IUnknownImpl.h
        Template IUnknown interface implementation for reference counted objects
*/

#pragma once

#include "xGS/IUnknown.h"


namespace xGS
{

    template <typename T>
    class xGSIUnknownImpl : public T
    {
    public:
        xGSIUnknownImpl() :
            p_refcount(0)
        {}

        virtual ~xGSIUnknownImpl()
        {}

        unsigned int __stdcall AddRef() override
        {
            ++p_refcount;
            return p_refcount;
        }

        unsigned int __stdcall Release() override
        {
            if ((--p_refcount) == 0) {
                delete this;
                return 0;
            }
            return p_refcount;
        }

    protected:
        unsigned int p_refcount;
    };

} // namespace xGS
