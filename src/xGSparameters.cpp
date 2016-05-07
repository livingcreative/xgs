#include "xGSparameters.h"


xGSparametersImpl::xGSparametersImpl(xGSimpl *owner) :
    xGSobjectImpl(owner)
{}

xGSparametersImpl::~xGSparametersImpl()
{}

bool xGSparametersImpl::Allocate(const GSparametersdesc &desc)
{
    // TODO: parameters object allocation


    return true;
}

void xGSparametersImpl::Apply()
{
    // TODO: parameters object apply
}
