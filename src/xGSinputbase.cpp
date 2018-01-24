/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    xGSinputbase.cpp
        Input object implementation base class
*/

#include "xGSinputbase.h"


using namespace xGS;


xGSInputBase::xGSInputBase() :
    p_state(nullptr),
    p_primarybuffer(nullptr),
    p_buffers()
{}
