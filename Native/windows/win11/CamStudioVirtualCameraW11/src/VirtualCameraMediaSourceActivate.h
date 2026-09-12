#pragma once

#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>

class VirtualCameraMediaSourceActivate final :
    public IMFActivate
{
public:
    VirtualCameraMediaSourceActivate();
    ~VirtualCameraMediaSourceActivate();

    // IUnknown
    STDMETHODIMP QueryInterface(
        REFIID riid,
        void** ppv
    );

    STDMETHODIMP_(ULONG) AddRef();

    STDMETHODIMP_(ULONG) Release();

    // IMFActivate
    STDMETHODIMP ActivateObject(
        REFIID riid,
        void** ppv
    );

    STDMETHODIMP ShutdownObject();

    STDMETHODIMP DetachObject();

    // IMFAttributes
    STDMETHODIMP GetItem(
        REFGUID guidKey,
        PROPVARIANT* value
    );

    STDMETHODIMP GetItemType(
        REFGUID guidKey,
        MF_ATTRIBUTE_TYPE* type
    );

    STDMETHODIMP CompareItem(
        REFGUID guidKey,
        REFPROPVARIANT value,
        BOOL* result
    );

    STDMETHODIMP Compare(
        IMFAttributes* attributes,
        MF_ATTRIBUTES_MATCH_TYPE matchType,
        BOOL* result
    );

    STDMETHODIMP GetUINT32(
        REFGUID guidKey,
        UINT32* value
    );

    STDMETHODIMP GetUINT64(
        REFGUID guidKey,
        UINT64* value
    );

    STDMETHODIMP GetDouble(
        REFGUID guidKey,
        double* value
    );

    STDMETHODIMP GetGUID(
        REFGUID guidKey,
        GUID* value
    );

    STDMETHODIMP GetStringLength(
        REFGUID guidKey,
        UINT32* length
    );

    STDMETHODIMP GetString(
        REFGUID guidKey,
        LPWSTR buffer,
        UINT32 size,
        UINT32* length
    );

    STDMETHODIMP GetAllocatedString(
        REFGUID guidKey,
        LPWSTR* value,
        UINT32* length
    );

    STDMETHODIMP GetBlobSize(
        REFGUID guidKey,
        UINT32* size
    );

    STDMETHODIMP GetBlob(
        REFGUID guidKey,
        UINT8* buffer,
        UINT32 size,
        UINT32* blobSize
    );

    STDMETHODIMP GetAllocatedBlob(
        REFGUID guidKey,
        UINT8** buffer,
        UINT32* size
    );

    STDMETHODIMP GetUnknown(
        REFGUID guidKey,
        REFIID riid,
        LPVOID* object
    );

    STDMETHODIMP SetItem(
        REFGUID guidKey,
        REFPROPVARIANT value
    );

    STDMETHODIMP DeleteItem(
        REFGUID guidKey
    );

    STDMETHODIMP DeleteAllItems();

    STDMETHODIMP SetUINT32(
        REFGUID guidKey,
        UINT32 value
    );

    STDMETHODIMP SetUINT64(
        REFGUID guidKey,
        UINT64 value
    );

    STDMETHODIMP SetDouble(
        REFGUID guidKey,
        double value
    );

    STDMETHODIMP SetGUID(
        REFGUID guidKey,
        REFGUID value
    );

    STDMETHODIMP SetString(
        REFGUID guidKey,
        LPCWSTR value
    );

    STDMETHODIMP SetBlob(
        REFGUID guidKey,
        const UINT8* buffer,
        UINT32 size
    );

    STDMETHODIMP SetUnknown(
        REFGUID guidKey,
        IUnknown* unknown
    );

    STDMETHODIMP LockStore();

    STDMETHODIMP UnlockStore();

    STDMETHODIMP GetCount(
        UINT32* count
    );

    STDMETHODIMP GetItemByIndex(
        UINT32 index,
        GUID* key,
        PROPVARIANT* value
    );

    STDMETHODIMP CopyAllItems(
        IMFAttributes* destination
    );

private:
    LONG m_refCount;

    IMFAttributes* m_attributes;

    IUnknown* m_object;
};