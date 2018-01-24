/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    xGSmain.cpp
        "main" source, exports the only function Create for xGS
        system object construction
        and also includes other xGS sources
*/

#include "xGSimpl.h"

// "unity" build - platform implementation
#include "xGSplatform.cpp"
#include "xGSdatabuffer.cpp"
#include "xGStexture.cpp"
#include "xGSparameters.cpp"
#include "xGSstate.cpp"
#include "xGSinput.cpp"
#include "xGSgeometrybuffer.cpp"
#include "xGSframebuffer.cpp"
#include "xGSimpl.cpp"

// "unity" build - common
#include "xGSutil.cpp"
#include "xGSdatabufferbase.cpp"
#include "IxGSdatabufferimpl.cpp"
#include "xGSgeometrybufferbase.cpp"
#include "IxGSgeometrybufferimpl.cpp"
#include "xGSframebufferbase.cpp"
#include "IxGSframebufferimpl.cpp"
#include "xGSinputbase.cpp"
#include "IxGSinputimpl.cpp"
#include "xGSgeometry.cpp"
#include "xGSimplbase.cpp"
#include "IxGSimpl.cpp"

using namespace xGS;


GSbool Create(IxGS *xgs)
{
    *xgs = IxGSImpl::create();
    return *xgs != nullptr;
}
