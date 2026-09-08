#include "pch.h"

#include "VirtualCameraMediaSource.h"
#include "VirtualCameraMediaStream.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>
#include <mferror.h>

VirtualCameraMediaSource::VirtualCameraMediaSource()
    : m_refCount(1),
    m_initialized(false),
    m_started(false),
    m_shutdown(false),
    m_width(1280),
    m_height(720),
    m_fps(30),
    m_attributes(nullptr),
    m_eventQueue(nullptr),
    m_presentationDescriptor(nullptr),
    m_streamDescriptor(nullptr),
    m_stream(nullptr),
    m_sharedMemory(nullptr)
{
}

VirtualCameraMediaSource::~VirtualCameraMediaSource()
{
    Shutdown();
}


// ============================================================
// IUnknown
// ============================================================

STDMETHODIMP
VirtualCameraMediaSource::QueryInterface(
    REFIID riid,
    void** ppv)
{
    if (ppv == nullptr)
        return E_POINTER;

    *ppv = nullptr;

    if (riid == IID_IUnknown ||
        riid == IID_IMFMediaEventGenerator ||
        riid == IID_IMFMediaSource)
    {
        *ppv =
            static_cast<IMFMediaSource*>(this);

        AddRef();

        return S_OK;
    }

    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG)
VirtualCameraMediaSource::AddRef()
{
    return InterlockedIncrement(
        &m_refCount
    );
}


STDMETHODIMP_(ULONG)
VirtualCameraMediaSource::Release()
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
// INITIALIZE
// ============================================================

HRESULT
VirtualCameraMediaSource::Initialize(
    IMFAttributes* attributes)
{
    if (m_initialized)
        return MF_E_ALREADY_INITIALIZED;

    if (m_shutdown)
        return MF_E_SHUTDOWN;


    // ========================================================
    // ATTRIBUTES
    // ========================================================

    HRESULT hr =
        MFCreateAttributes(
            &m_attributes,
            10
        );

    if (FAILED(hr))
        return hr;

    if (attributes != nullptr)
    {
        hr =
            attributes->CopyAllItems(
                m_attributes
            );

        if (FAILED(hr))
            return hr;
    }


    // ========================================================
    // EVENT QUEUE
    // ========================================================

    hr =
        MFCreateEventQueue(
            &m_eventQueue
        );

    if (FAILED(hr))
        return hr;


    // ========================================================
    // SHARED MEMORY
    // ========================================================

    m_sharedMemory =
        new CamStudioSharedMemory();

    if (m_sharedMemory == nullptr)
        return E_OUTOFMEMORY;


    /*
        El MediaSource puede ser activado por Windows
        en un proceso/contexto diferente al proceso de
        CamStudio.

        Por eso la Shared Memory se abre mediante su
        nombre global.
    */

    if (!m_sharedMemory->Open(
        m_width,
        m_height
    ))
    {
        /*
            No hacemos fallar la creación del MediaSource.

            CamStudio puede todavía no haber creado el
            mapping cuando Windows activa inicialmente
            la cámara.
        */
    }


    // ========================================================
    // MEDIA TYPE
    // ========================================================

    IMFMediaType* mediaType = nullptr;

    hr =
        CreateMediaType(
            &mediaType
        );

    if (FAILED(hr))
        return hr;


    // ========================================================
    // STREAM DESCRIPTOR
    // ========================================================

    hr =
        MFCreateStreamDescriptor(
            0,
            1,
            &mediaType,
            &m_streamDescriptor
        );

    mediaType->Release();

    if (FAILED(hr))
        return hr;


    // ========================================================
    // PRESENTATION DESCRIPTOR
    // ========================================================

    hr =
        MFCreatePresentationDescriptor(
            1,
            &m_streamDescriptor,
            &m_presentationDescriptor
        );

    if (FAILED(hr))
        return hr;


    hr =
        m_presentationDescriptor->SelectStream(
            0
        );

    if (FAILED(hr))
        return hr;


    // ========================================================
    // MEDIA STREAM
    // ========================================================

    IMFMediaType* streamMediaType = nullptr;

    hr = CreateMediaType(
        &streamMediaType
    );

    if (FAILED(hr))
        return hr;

    m_stream =
        new VirtualCameraMediaStream(
            this,
            0,
            streamMediaType,
            m_width,
            m_height,
            m_fps
        );

    streamMediaType->Release();

    if (m_stream == nullptr)
        return E_OUTOFMEMORY;


    /*
        El Stream necesita conocer su descriptor.
    */

    hr = m_stream->SetStreamDescriptor(
        m_streamDescriptor
    );

    if (FAILED(hr))
    {
        m_stream->Release();
        m_stream = nullptr;
        return hr;
    }


    m_initialized = true;

    return S_OK;
}


// ============================================================
// MEDIA TYPE
// ============================================================

HRESULT
VirtualCameraMediaSource::CreateMediaType(
    IMFMediaType** mediaType)
{
    if (mediaType == nullptr)
        return E_POINTER;

    *mediaType = nullptr;

    IMFMediaType* type = nullptr;

    HRESULT hr =
        MFCreateMediaType(
            &type
        );

    if (FAILED(hr))
        return hr;


    // Major type

    hr =
        type->SetGUID(
            MF_MT_MAJOR_TYPE,
            MFMediaType_Video
        );

    if (FAILED(hr))
    {
        type->Release();
        return hr;
    }


    // RGB24

    hr =
        type->SetGUID(
            MF_MT_SUBTYPE,
            MFVideoFormat_RGB24
        );

    if (FAILED(hr))
    {
        type->Release();
        return hr;
    }


    // Progressive

    hr =
        type->SetUINT32(
            MF_MT_INTERLACE_MODE,
            MFVideoInterlace_Progressive
        );

    if (FAILED(hr))
    {
        type->Release();
        return hr;
    }


    // Independent samples

    hr =
        type->SetUINT32(
            MF_MT_ALL_SAMPLES_INDEPENDENT,
            TRUE
        );

    if (FAILED(hr))
    {
        type->Release();
        return hr;
    }


    // Frame size

    hr =
        MFSetAttributeSize(
            type,
            MF_MT_FRAME_SIZE,
            m_width,
            m_height
        );

    if (FAILED(hr))
    {
        type->Release();
        return hr;
    }


    // FPS

    hr =
        MFSetAttributeRatio(
            type,
            MF_MT_FRAME_RATE,
            m_fps,
            1
        );

    if (FAILED(hr))
    {
        type->Release();
        return hr;
    }


    // Pixel aspect ratio

    hr =
        MFSetAttributeRatio(
            type,
            MF_MT_PIXEL_ASPECT_RATIO,
            1,
            1
        );

    if (FAILED(hr))
    {
        type->Release();
        return hr;
    }


    // RGB24 = 3 bytes per pixel

    const UINT32 imageSize =
        static_cast<UINT32>(
            m_width *
            m_height *
            3
            );

    hr =
        type->SetUINT32(
            MF_MT_SAMPLE_SIZE,
            imageSize
        );

    if (FAILED(hr))
    {
        type->Release();
        return hr;
    }


    *mediaType = type;

    return S_OK;
}


// ============================================================
// STREAM DESCRIPTOR
// ============================================================

HRESULT
VirtualCameraMediaSource::CreateStreamDescriptor(
    IMFStreamDescriptor** streamDescriptor)
{
    if (streamDescriptor == nullptr)
        return E_POINTER;

    *streamDescriptor = nullptr;

    IMFMediaType* mediaType = nullptr;

    HRESULT hr =
        CreateMediaType(
            &mediaType
        );

    if (FAILED(hr))
        return hr;

    hr =
        MFCreateStreamDescriptor(
            0,
            1,
            &mediaType,
            streamDescriptor
        );

    mediaType->Release();

    return hr;
}


// ============================================================
// IMFMediaEventGenerator
// ============================================================

STDMETHODIMP
VirtualCameraMediaSource::GetEvent(
    DWORD flags,
    IMFMediaEvent** event)
{
    if (event == nullptr)
        return E_POINTER;

    if (m_eventQueue == nullptr)
        return MF_E_SHUTDOWN;

    return m_eventQueue->GetEvent(
        flags,
        event
    );
}


STDMETHODIMP
VirtualCameraMediaSource::BeginGetEvent(
    IMFAsyncCallback* callback,
    IUnknown* state)
{
    if (m_eventQueue == nullptr)
        return MF_E_SHUTDOWN;

    return m_eventQueue->BeginGetEvent(
        callback,
        state
    );
}


STDMETHODIMP
VirtualCameraMediaSource::EndGetEvent(
    IMFAsyncResult* result,
    IMFMediaEvent** event)
{
    if (m_eventQueue == nullptr)
        return MF_E_SHUTDOWN;

    return m_eventQueue->EndGetEvent(
        result,
        event
    );
}


STDMETHODIMP
VirtualCameraMediaSource::QueueEvent(
    MediaEventType met,
    REFGUID extendedType,
    HRESULT status,
    const PROPVARIANT* value)
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
// IMFMediaSource
// ============================================================

STDMETHODIMP
VirtualCameraMediaSource::GetCharacteristics(
    DWORD* characteristics)
{
    if (characteristics == nullptr)
        return E_POINTER;

    if (m_shutdown)
        return MF_E_SHUTDOWN;

    *characteristics =
        MFMEDIASOURCE_IS_LIVE;

    return S_OK;
}


STDMETHODIMP
VirtualCameraMediaSource::CreatePresentationDescriptor(
    IMFPresentationDescriptor** descriptor)
{
    if (descriptor == nullptr)
        return E_POINTER;

    *descriptor = nullptr;

    if (m_shutdown)
        return MF_E_SHUTDOWN;

    if (!m_initialized)
        return MF_E_NOT_INITIALIZED;

    if (m_presentationDescriptor == nullptr)
        return E_UNEXPECTED;

    return m_presentationDescriptor->Clone(
        descriptor
    );
}


// ============================================================
// START
// ============================================================

STDMETHODIMP
VirtualCameraMediaSource::Start(
    IMFPresentationDescriptor* descriptor,
    const GUID* timeFormat,
    const PROPVARIANT* startPosition)
{
    UNREFERENCED_PARAMETER(
        timeFormat
    );

    UNREFERENCED_PARAMETER(
        startPosition
    );

    if (m_shutdown)
        return MF_E_SHUTDOWN;

    if (!m_initialized)
        return MF_E_NOT_INITIALIZED;

    if (descriptor == nullptr)
        return E_POINTER;


    // ========================================================
    // START STREAM
    // ========================================================

    if (m_stream != nullptr)
    {
        HRESULT hr =
            m_stream->Start();

        if (FAILED(hr))
            return hr;
    }


    m_started = true;


    // ========================================================
    // SOURCE EVENT
    // ========================================================

    if (m_eventQueue != nullptr)
    {
        m_eventQueue->QueueEventParamVar(
            MESourceStarted,
            GUID_NULL,
            S_OK,
            nullptr
        );
    }

    return S_OK;
}


// ============================================================
// STOP
// ============================================================

STDMETHODIMP
VirtualCameraMediaSource::Stop()
{
    if (m_shutdown)
        return MF_E_SHUTDOWN;

    if (!m_started)
        return MF_E_INVALID_STATE_TRANSITION;


    if (m_stream != nullptr)
    {
        HRESULT hr =
            m_stream->Stop();

        if (FAILED(hr))
            return hr;
    }


    m_started = false;


    if (m_eventQueue != nullptr)
    {
        m_eventQueue->QueueEventParamVar(
            MESourceStopped,
            GUID_NULL,
            S_OK,
            nullptr
        );
    }

    return S_OK;
}


// ============================================================
// PAUSE
// ============================================================

STDMETHODIMP
VirtualCameraMediaSource::Pause()
{
    if (m_shutdown)
        return MF_E_SHUTDOWN;

    return MF_E_INVALID_STATE_TRANSITION;
}


// ============================================================
// SHUTDOWN
// ============================================================

STDMETHODIMP
VirtualCameraMediaSource::Shutdown()
{
    if (m_shutdown)
        return S_OK;

    m_shutdown = true;
    m_started = false;


    // ========================================================
    // STREAM
    // ========================================================

    if (m_stream != nullptr)
    {
        m_stream->Shutdown();
        m_stream->Release();
        m_stream = nullptr;
    }


    // ========================================================
    // SHARED MEMORY
    // ========================================================

    if (m_sharedMemory != nullptr)
    {
        delete m_sharedMemory;
        m_sharedMemory = nullptr;
    }


    // ========================================================
    // PRESENTATION DESCRIPTOR
    // ========================================================

    if (m_presentationDescriptor != nullptr)
    {
        m_presentationDescriptor->Release();
        m_presentationDescriptor = nullptr;
    }


    // ========================================================
    // STREAM DESCRIPTOR
    // ========================================================

    if (m_streamDescriptor != nullptr)
    {
        m_streamDescriptor->Release();
        m_streamDescriptor = nullptr;
    }


    // ========================================================
    // EVENT QUEUE
    // ========================================================

    if (m_eventQueue != nullptr)
    {
        m_eventQueue->Shutdown();
        m_eventQueue->Release();
        m_eventQueue = nullptr;
    }


    // ========================================================
    // ATTRIBUTES
    // ========================================================

    if (m_attributes != nullptr)
    {
        m_attributes->Release();
        m_attributes = nullptr;
    }


    return S_OK;
}