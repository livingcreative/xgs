/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    xGSmain.cpp
        "main" source, exports the only function Create for xGS
        system object construction
*/

#include "xGSimpl.h"

// "unity" build
#include "xGSutil.cpp"
#include "xGSuniformbuffer.cpp"
#include "xGStexture.cpp"
#include "xGSparameters.cpp"
#include "xGSstate.cpp"
#include "xGSinput.cpp"
#include "xGSimpl.cpp"
#include "xGSgeometrybuffer.cpp"
#include "xGSgeometry.cpp"
#include "xGSframebuffer.cpp"

#include "xGScontextplatform.h"
#include "xGScontextplatform.cpp"


using namespace xGS;


GSbool Create(IxGS *xgs)
{
    xGScontextCreatorPlatform cc;
    *xgs = xGSImpl::create(&cc);
    return *xgs != nullptr;
}
