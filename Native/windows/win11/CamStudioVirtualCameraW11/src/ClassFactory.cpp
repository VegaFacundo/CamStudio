#include "pch.h"
#include "ClassFactory.h"
#include "VirtualCameraMediaSourceActivate.h"
#include <cstdio>

ClassFactory::ClassFactory()
    : m_refCount(1)
{
}

STDMETHODIMP ClassFactory::QueryInterface(
    REFIID riid,
    void** ppv)
{
    if (ppv == nullptr)
        return E_POINTER;

    *ppv = nullptr;

    if (riid == IID_IUnknown ||
        riid == IID_IClassFactory)
    {
        *ppv = static_cast<IClassFactory*>(this);
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) ClassFactory::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(ULONG) ClassFactory::Release()
{
    ULONG count = InterlockedDecrement(&m_refCount);

    if (count == 0)
        delete this;

    return count;
}

STDMETHODIMP ClassFactory::CreateInstance(
    IUnknown* outer,
    REFIID riid,
    void** ppv)
{
    printf("ClassFactory::CreateInstance called!\n");
    if (ppv == nullptr)
        return E_POINTER;

    *ppv = nullptr;

    if (outer != nullptr)
        return CLASS_E_NOAGGREGATION;

    VirtualCameraMediaSourceActivate* activate =
        new VirtualCameraMediaSourceActivate();

    if (activate == nullptr)
        return E_OUTOFMEMORY;

    HRESULT hr =
        activate->QueryInterface(
            riid,
            ppv
        );

    activate->Release();

    return hr;
}

STDMETHODIMP ClassFactory::LockServer(BOOL lock)
{
    return S_OK;
}