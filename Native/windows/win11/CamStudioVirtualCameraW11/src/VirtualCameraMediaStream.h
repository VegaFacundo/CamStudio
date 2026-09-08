#pragma once

#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>

#include "CamStudioSharedMemory.h"

class VirtualCameraMediaSource;

class VirtualCameraMediaStream final :
    public IMFMediaStream
{
public:

    VirtualCameraMediaStream(
        VirtualCameraMediaSource* source,
        DWORD streamId,
        IMFMediaType* mediaType,
        int width,
        int height,
        int fps
    );

    ~VirtualCameraMediaStream();

    // IUnknown
    STDMETHODIMP QueryInterface(
        REFIID riid,
        void** ppv
    );

    STDMETHODIMP_(ULONG) AddRef();

    STDMETHODIMP_(ULONG) Release();


    // IMFMediaEventGenerator
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


    // IMFMediaStream
    STDMETHODIMP GetMediaSource(
        IMFMediaSource** mediaSource
    );

    STDMETHODIMP GetStreamDescriptor(
        IMFStreamDescriptor** descriptor
    );

    STDMETHODIMP RequestSample(
        IUnknown* token
    );


    // Control
    HRESULT Start();

    HRESULT Stop();

    HRESULT Shutdown();


    // Stream descriptor
    HRESULT SetStreamDescriptor(
        IMFStreamDescriptor* descriptor
    );


private:

    HRESULT CreateSample(
        IMFSample** sample
    );

    HRESULT ReadFrame(
        IMFSample* sample
    );


private:

    LONG m_refCount;

    bool m_started;
    bool m_shutdown;

    DWORD m_streamId;

    int m_width;
    int m_height;
    int m_fps;

    LONGLONG m_frameNumber;

    IMFMediaType* m_mediaType;

    IMFStreamDescriptor* m_streamDescriptor;

    IMFMediaSource* m_mediaSource;

    IMFMediaEventQueue* m_eventQueue;

    CamStudioSharedMemory* m_sharedMemory;
};