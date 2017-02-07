/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/xgs

    dx12/xGSmain.cpp
        "main" source, exports the only function Create for xGS
        system object construction
*/

#include "xGSimpl.h"

// "unity" build
#include "xGSutil.cpp"
#include "xGSDX12util.cpp"
#include "xGSdatabuffer.cpp"
#include "xGStexture.cpp"
#include "xGSparameters.cpp"
#include "xGSstate.cpp"
#include "xGSinput.cpp"
#include "xGSgeometrybufferbase.cpp"
#include "xGSgeometrybuffer.cpp"
#include "xGSgeometry.cpp"
#include "xGSframebuffer.cpp"
#include "xGSimplbase.cpp"
#include "xGSimpl.cpp"
#include "IxGSimpl.cpp"


using namespace xGS;


GSbool Create(IxGS *xgs)
{
    *xgs = IxGSImpl::create();
    return *xgs != nullptr;
}
