/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    IUnknown.h
        Abstract stub class for COM-linke interface compatibility
*/

#pragma once


class IUnknownStub
{
public:
    virtual unsigned int __stdcall QueryInterface(void *riid, void **ppvObject)
    {
        return /*E_NOTIMPL*/ 0x80004001L;
    }

    virtual unsigned int __stdcall AddRef() = 0;
    virtual unsigned int __stdcall Release() = 0;
};
