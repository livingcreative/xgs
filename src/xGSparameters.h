#pragma once

#include "xGS/xGS.h"
#include "GL/glew.h"
#include "xGSobject.h"


class xGSparametersImpl : public xGSobjectImpl<xGSparameters>
{
public:
    xGSparametersImpl(xGSimpl *owner);
    ~xGSparametersImpl() override;

    bool Allocate(const GSparametersdesc &desc);

public:
    void Apply();

private:
};
