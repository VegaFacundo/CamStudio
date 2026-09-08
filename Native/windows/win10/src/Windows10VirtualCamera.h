#pragma once

#include <streams.h>
#include <windows.h>
#include <dvdmedia.h>
#include <ks.h>
#include <ksmedia.h>

#include "CamStudioSharedMemory.h"

extern const CLSID CLSID_CamStudioVirtualCamera;

class Windows10VirtualCameraStream;

class Windows10VirtualCamera : public CSource
{
public:

    static CUnknown* WINAPI CreateInstance(
        LPUNKNOWN lpunk,
        HRESULT* phr
    );

    STDMETHODIMP QueryInterface(
        REFIID riid,
        void** ppv
    );

private:

    Windows10VirtualCamera(
        LPUNKNOWN lpunk,
        HRESULT* phr
    );

    ~Windows10VirtualCamera();

    friend class Windows10VirtualCameraStream;
};


class Windows10VirtualCameraStream :
    public CSourceStream,
    public IAMStreamConfig,
    public IKsPropertySet
{
public:

    Windows10VirtualCameraStream(
        HRESULT* phr,
        Windows10VirtualCamera* parent
    );

    ~Windows10VirtualCameraStream();

    STDMETHODIMP QueryInterface(
        REFIID riid,
        void** ppv
    );

    STDMETHODIMP_(ULONG) AddRef();

    STDMETHODIMP_(ULONG) Release();

    HRESULT GetMediaType(
        int position,
        CMediaType* mediaType
    );

    HRESULT CheckMediaType(
        const CMediaType* mediaType
    );

    HRESULT DecideBufferSize(
        IMemAllocator* allocator,
        ALLOCATOR_PROPERTIES* properties
    );

    HRESULT FillBuffer(
        IMediaSample* sample
    );

    HRESULT OnThreadCreate();

    HRESULT OnThreadDestroy();

    STDMETHODIMP SetFormat(
        AM_MEDIA_TYPE* pmt
    );

    STDMETHODIMP GetFormat(
        AM_MEDIA_TYPE** ppmt
    );

    STDMETHODIMP GetNumberOfCapabilities(
        int* piCount,
        int* piSize
    );

    STDMETHODIMP GetStreamCaps(
        int iIndex,
        AM_MEDIA_TYPE** ppmt,
        BYTE* pSCC
    );

    STDMETHODIMP Set(
        REFGUID guidPropSet,
        DWORD dwID,
        void* pInstanceData,
        DWORD cbInstanceData,
        void* pPropData,
        DWORD cbPropData
    );

    STDMETHODIMP Get(
        REFGUID guidPropSet,
        DWORD dwPropID,
        void* pInstanceData,
        DWORD cbInstanceData,
        void* pPropData,
        DWORD cbPropData,
        DWORD* pcbReturned
    );

    STDMETHODIMP QuerySupported(
        REFGUID guidPropSet,
        DWORD dwPropID,
        DWORD* pTypeSupport
    );


private:

    Windows10VirtualCamera* parent;

    CamStudioSharedMemory* sharedMemory;

    REFERENCE_TIME frameDuration;

    LONGLONG frameNumber;
};