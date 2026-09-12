#include "pch.h"
#include "VirtualCameraMediaSourceActivate.h"
#include "VirtualCameraMediaSource.h"
#include <cstdio>

VirtualCameraMediaSourceActivate::VirtualCameraMediaSourceActivate()
    : m_refCount(1),
    m_attributes(nullptr),
    m_object(nullptr)
{
    MFCreateAttributes(&m_attributes, 8);
}

VirtualCameraMediaSourceActivate::~VirtualCameraMediaSourceActivate()
{
    if (m_object != nullptr)
    {
        m_object->Release();
        m_object = nullptr;
    }

    if (m_attributes != nullptr)
    {
        m_attributes->Release();
        m_attributes = nullptr;
    }
}

STDMETHODIMP VirtualCameraMediaSourceActivate::QueryInterface(
    REFIID riid,
    void** ppv)
{
    if (ppv == nullptr)
        return E_POINTER;

    *ppv = nullptr;

    if (riid == IID_IUnknown ||
        riid == IID_IMFActivate ||
        riid == IID_IMFAttributes)
    {
        *ppv = static_cast<IMFActivate*>(this);
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG)
VirtualCameraMediaSourceActivate::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(ULONG)
VirtualCameraMediaSourceActivate::Release()
{
    ULONG count = InterlockedDecrement(&m_refCount);

    if (count == 0)
        delete this;

    return count;
}

STDMETHODIMP VirtualCameraMediaSourceActivate::ActivateObject(
    REFIID riid,
    void** ppv)
{
    printf("VirtualCameraMediaSourceActivate::ActivateObject called!\n");
    if (ppv == nullptr)
        return E_POINTER;

    *ppv = nullptr;

    if (m_object != nullptr)
    {
        return m_object->QueryInterface(
            riid,
            ppv
        );
    }

    VirtualCameraMediaSource* source =
        new VirtualCameraMediaSource();

    if (source == nullptr)
        return E_OUTOFMEMORY;

    HRESULT hr =
        source->Initialize(m_attributes);

    if (FAILED(hr))
    {
        source->Release();
        return hr;
    }

    m_object = source;

    hr = m_object->QueryInterface(
        riid,
        ppv
    );

    return hr;
}

STDMETHODIMP VirtualCameraMediaSourceActivate::ShutdownObject()
{
    printf("VirtualCameraMediaSourceActivate::ActivateObject called!\n");
    if (m_object != nullptr)
    {
        IMFMediaSource* source = nullptr;

        if (SUCCEEDED(
            m_object->QueryInterface(
                IID_PPV_ARGS(&source))))
        {
            source->Shutdown();
            source->Release();
        }

        m_object->Release();
        m_object = nullptr;
    }

    return S_OK;
}

STDMETHODIMP VirtualCameraMediaSourceActivate::DetachObject()
{
    m_object = nullptr;
    return S_OK;
}

STDMETHODIMP VirtualCameraMediaSourceActivate::SetUINT32(
    REFGUID guidKey,
    UINT32 value)
{
    return m_attributes->SetUINT32(guidKey, value);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::GetItem(
    REFGUID guidKey,
    PROPVARIANT* value)
{
    return m_attributes->GetItem(guidKey, value);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::GetItemType(
    REFGUID guidKey,
    MF_ATTRIBUTE_TYPE* type)
{
    return m_attributes->GetItemType(guidKey, type);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::CompareItem(
    REFGUID guidKey,
    REFPROPVARIANT value,
    BOOL* result)
{
    return m_attributes->CompareItem(guidKey, value, result);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::Compare(
    IMFAttributes* attributes,
    MF_ATTRIBUTES_MATCH_TYPE matchType,
    BOOL* result)
{
    return m_attributes->Compare(attributes, matchType, result);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::GetUINT32(
    REFGUID guidKey,
    UINT32* value)
{
    return m_attributes->GetUINT32(guidKey, value);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::GetUINT64(
    REFGUID guidKey,
    UINT64* value)
{
    return m_attributes->GetUINT64(guidKey, value);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::GetDouble(
    REFGUID guidKey,
    double* value)
{
    return m_attributes->GetDouble(guidKey, value);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::GetGUID(
    REFGUID guidKey,
    GUID* value)
{
    return m_attributes->GetGUID(guidKey, value);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::GetStringLength(
    REFGUID guidKey,
    UINT32* length)
{
    return m_attributes->GetStringLength(guidKey, length);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::GetString(
    REFGUID guidKey,
    LPWSTR buffer,
    UINT32 size,
    UINT32* length)
{
    return m_attributes->GetString(
        guidKey,
        buffer,
        size,
        length
    );
}

STDMETHODIMP VirtualCameraMediaSourceActivate::GetAllocatedString(
    REFGUID guidKey,
    LPWSTR* value,
    UINT32* length)
{
    return m_attributes->GetAllocatedString(
        guidKey,
        value,
        length
    );
}

STDMETHODIMP VirtualCameraMediaSourceActivate::GetBlobSize(
    REFGUID guidKey,
    UINT32* size)
{
    return m_attributes->GetBlobSize(guidKey, size);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::GetBlob(
    REFGUID guidKey,
    UINT8* buffer,
    UINT32 size,
    UINT32* blobSize)
{
    return m_attributes->GetBlob(
        guidKey,
        buffer,
        size,
        blobSize
    );
}

STDMETHODIMP VirtualCameraMediaSourceActivate::GetAllocatedBlob(
    REFGUID guidKey,
    UINT8** buffer,
    UINT32* size)
{
    return m_attributes->GetAllocatedBlob(
        guidKey,
        buffer,
        size
    );
}

STDMETHODIMP VirtualCameraMediaSourceActivate::GetUnknown(
    REFGUID guidKey,
    REFIID riid,
    LPVOID* object)
{
    return m_attributes->GetUnknown(
        guidKey,
        riid,
        object
    );
}

STDMETHODIMP VirtualCameraMediaSourceActivate::SetItem(
    REFGUID guidKey,
    REFPROPVARIANT value)
{
    return m_attributes->SetItem(guidKey, value);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::DeleteItem(
    REFGUID guidKey)
{
    return m_attributes->DeleteItem(guidKey);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::DeleteAllItems()
{
    return m_attributes->DeleteAllItems();
}

STDMETHODIMP VirtualCameraMediaSourceActivate::SetUINT64(
    REFGUID guidKey,
    UINT64 value)
{
    return m_attributes->SetUINT64(guidKey, value);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::SetDouble(
    REFGUID guidKey,
    double value)
{
    return m_attributes->SetDouble(guidKey, value);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::SetGUID(
    REFGUID guidKey,
    REFGUID value)
{
    return m_attributes->SetGUID(guidKey, value);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::SetString(
    REFGUID guidKey,
    LPCWSTR value)
{
    return m_attributes->SetString(guidKey, value);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::SetBlob(
    REFGUID guidKey,
    const UINT8* buffer,
    UINT32 size)
{
    return m_attributes->SetBlob(
        guidKey,
        buffer,
        size
    );
}

STDMETHODIMP VirtualCameraMediaSourceActivate::SetUnknown(
    REFGUID guidKey,
    IUnknown* unknown)
{
    return m_attributes->SetUnknown(
        guidKey,
        unknown
    );
}

STDMETHODIMP VirtualCameraMediaSourceActivate::LockStore()
{
    return m_attributes->LockStore();
}

STDMETHODIMP VirtualCameraMediaSourceActivate::UnlockStore()
{
    return m_attributes->UnlockStore();
}

STDMETHODIMP VirtualCameraMediaSourceActivate::GetCount(
    UINT32* count)
{
    return m_attributes->GetCount(count);
}

STDMETHODIMP VirtualCameraMediaSourceActivate::GetItemByIndex(
    UINT32 index,
    GUID* key,
    PROPVARIANT* value)
{
    return m_attributes->GetItemByIndex(
        index,
        key,
        value
    );
}

STDMETHODIMP VirtualCameraMediaSourceActivate::CopyAllItems(
    IMFAttributes* destination)
{
    return m_attributes->CopyAllItems(destination);
}