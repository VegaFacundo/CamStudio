#pragma once

#include <unknwn.h>

class ClassFactory final :
    public IClassFactory
{
public:
    ClassFactory();

    STDMETHODIMP QueryInterface(
        REFIID riid,
        void** ppv
    );

    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    STDMETHODIMP CreateInstance(
        IUnknown* outer,
        REFIID riid,
        void** ppv
    );

    STDMETHODIMP LockServer(
        BOOL lock
    );

private:
    LONG m_refCount;
};