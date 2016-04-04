#pragma once

#include "xGSrefcounted.h"
#include "xGSimpl.h"


template <typename T>
class xGSobjectImpl : public xGSrefcountedImpl<T>
{
public:
    xGSobjectImpl(xGSimpl *owner) :
        p_owner(owner)
    {
        p_owner->AddObject(this);
    }

    ~xGSobjectImpl() override
    {
        p_owner->RemoveObject(this);
    }

protected:
    xGSimpl *p_owner;
};

