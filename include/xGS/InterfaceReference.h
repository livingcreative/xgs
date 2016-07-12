/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    InterfaceReference.h
        Very basic smart-pointer class for holding xGS ref-counted objects
*/

#pragma once


template <typename T>
class InterfacePtr
{
public:
    InterfacePtr() :
        p_ref(nullptr)
    {}

    InterfacePtr(const InterfacePtr<T> &source) :
        p_ref(source.p_ref)
    {
        if (p_ref) {
            p_ref->AddRef();
        }
    }

    explicit InterfacePtr(T ref) :
        p_ref(ref)
    {}

    ~InterfacePtr()
    {
        if (p_ref) {
            p_ref->Release();
        }
    }

    void TakeOwnership(T ref)
    {
        if (p_ref) {
            p_ref->Release();
        }

        p_ref = ref;
    }

    InterfacePtr<T>& operator=(const InterfacePtr<T> &source)
    {
        if (p_ref) {
            p_ref->Release();
        }

        p_ref = source.p_ref;

        if (p_ref) {
            p_ref->AddRef();
        }

        return *this;
    }

    T operator->() const { return p_ref; }

    operator T() const { return p_ref; }

private:
    T p_ref;
};
