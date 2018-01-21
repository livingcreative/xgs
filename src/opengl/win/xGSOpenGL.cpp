/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    opengl/win/xGSOpenGL.cpp
        "main" source for xGS DLL on Windows
*/

#include "xGS/xGS.h"


extern "C" {

    GSbool xGSAPI xGSCreate(IxGS *xgs)
    {
        return Create(xgs);
    }

}
