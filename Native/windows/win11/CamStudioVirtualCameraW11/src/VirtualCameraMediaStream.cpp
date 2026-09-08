#include "pch.h"

#include "VirtualCameraMediaStream.h"
#include "VirtualCameraMediaSource.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>
#include <mferror.h>

#include <cstring>

VirtualCameraMediaStream::VirtualCameraMediaStream(
    VirtualCameraMediaSource* source,
    DWORD streamId,
    IMFMediaType* mediaType,
    int width,
    int height,
    int fps
)
    : m_refCount(1),
    m_started(false),
    m_shutdown(false),
    m_streamId(streamId),
    m_width(width),
    m_height(height),
    m_fps(fps),
    m_frameNumber(0),
    m_mediaType(nullptr),
    m_streamDescriptor(nullptr),
    m_mediaSource(
        reinterpret_cast<IMFMediaSource*>(source)
    ),
    m_eventQueue(nullptr),
    m_sharedMemory(nullptr)
{
    if (m_mediaSource != nullptr)
    {
        m_mediaSource->AddRef();
    }

    if (mediaType != nullptr)
    {
        m_mediaType = mediaType;
        m_mediaType->AddRef();
    }

    MFCreateEventQueue(
        &m_eventQueue
    );

    m_sharedMemory =
        new CamStudioSharedMemory();

    if (m_sharedMemory == nullptr)
    {
        return;
    }

    if (m_sharedMemory != nullptr)
    {
        /*
            La Shared Memory es creada por
            CamStudioNative.

            Acá solamente intentamos abrirla.
        */

        m_sharedMemory->Open(
            m_width,
            m_height
        );
    }
}


VirtualCameraMediaStream::~VirtualCameraMediaStream()
{
    Shutdown();
}


// ============================================================
// IUnknown
// ============================================================

STDMETHODIMP
VirtualCameraMediaStream::QueryInterface(
    REFIID riid,
    void** ppv
)
{
    if (ppv == nullptr)
        return E_POINTER;

    *ppv = nullptr;

    if (riid == IID_IUnknown ||
        riid == IID_IMFMediaEventGenerator ||
        riid == IID_IMFMediaStream)
    {
        *ppv =
            static_cast<IMFMediaStream*>(this);

        AddRef();

        return S_OK;
    }

    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG)
VirtualCameraMediaStream::AddRef()
{
    return InterlockedIncrement(
        &m_refCount
    );
}


STDMETHODIMP_(ULONG)
VirtualCameraMediaStream::Release()
{
    ULONG count =
        InterlockedDecrement(
            &m_refCount
        );

    if (count == 0)
        delete this;

    return count;
}


// ============================================================
// IMFMediaEventGenerator
// ============================================================

STDMETHODIMP
VirtualCameraMediaStream::GetEvent(
    DWORD flags,
    IMFMediaEvent** event
)
{
    if (m_eventQueue == nullptr)
        return MF_E_SHUTDOWN;

    return m_eventQueue->GetEvent(
        flags,
        event
    );
}


STDMETHODIMP
VirtualCameraMediaStream::BeginGetEvent(
    IMFAsyncCallback* callback,
    IUnknown* state
)
{
    if (m_eventQueue == nullptr)
        return MF_E_SHUTDOWN;

    return m_eventQueue->BeginGetEvent(
        callback,
        state
    );
}


STDMETHODIMP
VirtualCameraMediaStream::EndGetEvent(
    IMFAsyncResult* result,
    IMFMediaEvent** event
)
{
    if (m_eventQueue == nullptr)
        return MF_E_SHUTDOWN;

    return m_eventQueue->EndGetEvent(
        result,
        event
    );
}


STDMETHODIMP
VirtualCameraMediaStream::QueueEvent(
    MediaEventType met,
    REFGUID extendedType,
    HRESULT status,
    const PROPVARIANT* value
)
{
    if (m_eventQueue == nullptr)
        return MF_E_SHUTDOWN;

    return m_eventQueue->QueueEventParamVar(
        met,
        extendedType,
        status,
        value
    );
}


// ============================================================
// IMFMediaStream
// ============================================================

STDMETHODIMP
VirtualCameraMediaStream::GetMediaSource(
    IMFMediaSource** mediaSource
)
{
    if (mediaSource == nullptr)
        return E_POINTER;

    *mediaSource = nullptr;

    if (m_shutdown)
        return MF_E_SHUTDOWN;

    if (m_mediaSource == nullptr)
        return E_UNEXPECTED;

    *mediaSource = m_mediaSource;

    m_mediaSource->AddRef();

    return S_OK;
}


STDMETHODIMP
VirtualCameraMediaStream::GetStreamDescriptor(
    IMFStreamDescriptor** descriptor
)
{
    if (descriptor == nullptr)
        return E_POINTER;

    *descriptor = nullptr;

    if (m_shutdown)
        return MF_E_SHUTDOWN;

    if (m_streamDescriptor == nullptr)
        return E_UNEXPECTED;

    *descriptor =
        m_streamDescriptor;

    m_streamDescriptor->AddRef();

    return S_OK;
}


// ============================================================
// REQUEST SAMPLE
// ============================================================

STDMETHODIMP
VirtualCameraMediaStream::RequestSample(
    IUnknown* token
)
{
    if (m_shutdown)
        return MF_E_SHUTDOWN;

    if (!m_started)
        return MF_E_NOT_INITIALIZED;

    /*
        Este método es el equivalente conceptual
        de FillBuffer() en nuestra implementación
        DirectShow anterior.

        Windows solicita un frame.
    */

    IMFSample* sample = nullptr;

    HRESULT hr =
        CreateSample(
            &sample
        );

    if (FAILED(hr))
        return hr;

    hr =
        ReadFrame(
            sample
        );

    if (FAILED(hr))
    {
        sample->Release();
        return hr;
    }


    /*
        Si la aplicación pasó un token,
        debemos asociarlo al sample.
    */

    if (token != nullptr)
    {
        sample->SetUnknown(
            MFSampleExtension_Token,
            token
        );
    }


    /*
        Entregamos el sample al pipeline
        de Media Foundation.
    */

    if (m_eventQueue != nullptr)
    {
        PROPVARIANT var;

        PropVariantInit(
            &var
        );

        var.vt =
            VT_UNKNOWN;

        var.punkVal =
            sample;

        sample->AddRef();

        m_eventQueue->QueueEventParamVar(
            MEMediaSample,
            GUID_NULL,
            S_OK,
            &var
        );

        PropVariantClear(
            &var
        );
    }

    sample->Release();

    return S_OK;
}


// ============================================================
// CREATE SAMPLE
// ============================================================

HRESULT
VirtualCameraMediaStream::CreateSample(
    IMFSample** sample
)
{
    if (sample == nullptr)
        return E_POINTER;

    *sample = nullptr;

    if (m_shutdown)
        return MF_E_SHUTDOWN;


    IMFMediaBuffer* buffer = nullptr;

    const DWORD bufferSize =
        static_cast<DWORD>(
            m_width *
            m_height *
            3
            );


    HRESULT hr =
        MFCreateMemoryBuffer(
            bufferSize,
            &buffer
        );

    if (FAILED(hr))
        return hr;


    IMFSample* newSample = nullptr;

    hr =
        MFCreateSample(
            &newSample
        );

    if (FAILED(hr))
    {
        buffer->Release();
        return hr;
    }


    hr =
        newSample->AddBuffer(
            buffer
        );

    buffer->Release();

    if (FAILED(hr))
    {
        newSample->Release();
        return hr;
    }


    /*
        Timestamp en unidades de 100ns.
    */

    const LONGLONG frameDuration =
        10000000LL /
        m_fps;

    const LONGLONG timestamp =
        m_frameNumber *
        frameDuration;


    hr =
        newSample->SetSampleTime(
            timestamp
        );

    if (FAILED(hr))
    {
        newSample->Release();
        return hr;
    }


    hr =
        newSample->SetSampleDuration(
            frameDuration
        );

    if (FAILED(hr))
    {
        newSample->Release();
        return hr;
    }


    *sample =
        newSample;

    return S_OK;
}


// ============================================================
// READ FRAME
// ============================================================

HRESULT
VirtualCameraMediaStream::ReadFrame(
    IMFSample* sample
)
{
    if (sample == nullptr)
        return E_POINTER;

    if (m_sharedMemory == nullptr)
        return E_UNEXPECTED;


    IMFMediaBuffer* buffer = nullptr;

    HRESULT hr =
        sample->GetBufferByIndex(
            0,
            &buffer
        );

    if (FAILED(hr))
        return hr;


    BYTE* destination = nullptr;

    DWORD maxLength = 0;

    DWORD currentLength = 0;


    hr =
        buffer->Lock(
            &destination,
            &maxLength,
            &currentLength
        );

    if (FAILED(hr))
    {
        buffer->Release();
        return hr;
    }


    const size_t frameSize =
        static_cast<size_t>(
            m_width *
            m_height *
            3
            );


    /*
        Intentamos obtener el último frame
        disponible desde Shared Memory.
    */
    size_t actualFrameSize = 0;

    bool success =
        m_sharedMemory->ReadFrame(
            destination,
            frameSize,
            actualFrameSize
        );


    if (!success)
    {
        /*
            Si todavía no existe un frame,
            devolvemos negro.

            Esto mantiene la cámara viva.
        */

        ZeroMemory(
            destination,
            frameSize
        );
    }


    buffer->Unlock();


    buffer->SetCurrentLength(
        static_cast<DWORD>(
            frameSize
            )
    );


    buffer->Release();


    ++m_frameNumber;

    return S_OK;
}

// ============================================================
// STREAM DESCRIPTOR
// ============================================================

HRESULT
VirtualCameraMediaStream::SetStreamDescriptor(
    IMFStreamDescriptor* descriptor)
{
    if (descriptor == nullptr)
        return E_POINTER;

    if (m_shutdown)
        return MF_E_SHUTDOWN;

    if (m_streamDescriptor != nullptr)
    {
        m_streamDescriptor->Release();
        m_streamDescriptor = nullptr;
    }

    m_streamDescriptor = descriptor;
    m_streamDescriptor->AddRef();

    return S_OK;
}

// ============================================================
// CONTROL
// ============================================================

HRESULT
VirtualCameraMediaStream::Start()
{
    if (m_shutdown)
        return MF_E_SHUTDOWN;

    m_started = true;

    m_frameNumber = 0;

    if (m_eventQueue != nullptr)
    {
        m_eventQueue->QueueEventParamVar(
            MEStreamStarted,
            GUID_NULL,
            S_OK,
            nullptr
        );
    }

    return S_OK;
}


HRESULT
VirtualCameraMediaStream::Stop()
{
    if (m_shutdown)
        return MF_E_SHUTDOWN;

    m_started = false;

    if (m_eventQueue != nullptr)
    {
        m_eventQueue->QueueEventParamVar(
            MEStreamStopped,
            GUID_NULL,
            S_OK,
            nullptr
        );
    }

    return S_OK;
}


HRESULT
VirtualCameraMediaStream::Shutdown()
{
    if (m_shutdown)
        return S_OK;

    m_shutdown = true;
    m_started = false;


    if (m_sharedMemory != nullptr)
    {
        delete m_sharedMemory;
        m_sharedMemory = nullptr;
    }


    if (m_eventQueue != nullptr)
    {
        m_eventQueue->Shutdown();

        m_eventQueue->Release();

        m_eventQueue = nullptr;
    }


    if (m_streamDescriptor != nullptr)
    {
        m_streamDescriptor->Release();

        m_streamDescriptor = nullptr;
    }


    if (m_mediaType != nullptr)
    {
        m_mediaType->Release();

        m_mediaType = nullptr;
    }


    if (m_mediaSource != nullptr)
    {
        m_mediaSource->Release();

        m_mediaSource = nullptr;
    }


    return S_OK;
}