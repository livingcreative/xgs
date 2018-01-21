/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    IUnknown.h
        Abstract stub class for COM-linke interface compatibility
*/

#pragma once

#ifdef WIN32
    #define INTERFACECALL __stdcall
#endif

#ifdef __APPLE__
    #define INTERFACECALL
#endif

class IUnknownStub
{
public:
    virtual unsigned int INTERFACECALL QueryInterface(void *riid, void **ppvObject)
    {
        return /*E_NOTIMPL*/ 0x80004001L;
    }

    virtual unsigned int INTERFACECALL AddRef() = 0;
    virtual unsigned int INTERFACECALL Release() = 0;
};
