#pragma once

#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>

#include "CamStudioSharedMemory.h"

class VirtualCameraMediaStream;

class VirtualCameraMediaSource final :
    public IMFMediaSource
{
public:

    VirtualCameraMediaSource();

    ~VirtualCameraMediaSource();


    // ========================================================
    // INITIALIZE
    // ========================================================

    HRESULT Initialize(
        IMFAttributes* attributes
    );


    // ========================================================
    // IUnknown
    // ========================================================

    STDMETHODIMP QueryInterface(
        REFIID riid,
        void** ppv
    );

    STDMETHODIMP_(ULONG) AddRef();

    STDMETHODIMP_(ULONG) Release();


    // ========================================================
    // IMFMediaEventGenerator
    // ========================================================

    STDMETHODIMP GetEvent(
        DWORD flags,
        IMFMediaEvent** event
    );

    STDMETHODIMP BeginGetEvent(
        IMFAsyncCallback* callback,
        IUnknown* state
    );

    STDMETHODIMP EndGetEvent(
        IMFAsyncResult* result,
        IMFMediaEvent** event
    );

    STDMETHODIMP QueueEvent(
        MediaEventType met,
        REFGUID extendedType,
        HRESULT status,
        const PROPVARIANT* value
    );


    // ========================================================
    // IMFMediaSource
    // ========================================================

    STDMETHODIMP GetCharacteristics(
        DWORD* characteristics
    );

    STDMETHODIMP CreatePresentationDescriptor(
        IMFPresentationDescriptor** descriptor
    );

    STDMETHODIMP Start(
        IMFPresentationDescriptor* descriptor,
        const GUID* timeFormat,
        const PROPVARIANT* startPosition
    );

    STDMETHODIMP Stop();

    STDMETHODIMP Pause();

    STDMETHODIMP Shutdown();


private:

    // ========================================================
    // INTERNAL
    // ========================================================

    HRESULT CreateMediaType(
        IMFMediaType** mediaType
    );

    HRESULT CreateStreamDescriptor(
        IMFStreamDescriptor** streamDescriptor
    );


private:

    LONG m_refCount;

    bool m_initialized;
    bool m_started;
    bool m_shutdown;

    int m_width;
    int m_height;
    int m_fps;


    IMFAttributes* m_attributes;

    IMFMediaEventQueue* m_eventQueue;

    IMFPresentationDescriptor* m_presentationDescriptor;

    IMFStreamDescriptor* m_streamDescriptor;


    VirtualCameraMediaStream* m_stream;

    CamStudioSharedMemory* m_sharedMemory;
};