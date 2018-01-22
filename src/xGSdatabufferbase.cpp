/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    xGSdatabufferbase.cpp
        DataBuffer object implementation base class
*/

#include "xGSdatabufferbase.h"
#include "xGSstate.h"


using namespace xGS;


xGSDataBufferBase::xGSDataBufferBase() :
    p_size(0),
    p_locktype(GS_NONE)
{}
