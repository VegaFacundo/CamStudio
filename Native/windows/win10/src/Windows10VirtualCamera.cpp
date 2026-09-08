#pragma warning(disable:4244)
#pragma warning(disable:4711)

#include "Windows10VirtualCamera.h"

#include <stdio.h>
#include <olectl.h>

#include <initguid.h>

#include "Windows10VirtualCamera.h"

const CLSID CLSID_CamStudioVirtualCamera =
{
    0x7d8f6b31,
    0x4f53,
    0x4c5b,
    {0x91, 0x23, 0x7a, 0x5e, 0x44, 0x91, 0x20, 0x11}
};

static void DebugLog(const char* text, bool debug = false)
{
    if (!debug)
        return;

    HMODULE module = nullptr;

    GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCSTR>(&DebugLog),
        &module
    );

    if (!module)
        return;

    char modulePath[MAX_PATH];

    DWORD length = GetModuleFileNameA(
        module,
        modulePath,
        MAX_PATH
    );

    if (length == 0 || length >= MAX_PATH)
        return;

    char* lastSlash = strrchr(modulePath, '\\');

    if (lastSlash)
        *(lastSlash + 1) = '\0';

    char logPath[MAX_PATH];

    sprintf_s(
        logPath,
        "%scamera_debug.txt",
        modulePath
    );

    FILE* file = nullptr;

    fopen_s(&file, logPath, "a");

    if (file)
    {
        fprintf(file, "%s\n", text);
        fclose(file);
    }
}


// ============================================================
// CONFIGURACIÓN DE LA CÁMARA
// ============================================================

static const LONG CAMERA_WIDTH = 1280;
static const LONG CAMERA_HEIGHT = 720;
static const LONG CAMERA_BPP = 24;

static const LONG CAMERA_IMAGE_SIZE =
CAMERA_WIDTH *
CAMERA_HEIGHT *
3;

static const REFERENCE_TIME CAMERA_FRAME_TIME =
333333; // ~30 FPS


// ============================================================
// CVCam / Windows10VirtualCamera
// ============================================================

CUnknown* WINAPI Windows10VirtualCamera::CreateInstance(
    LPUNKNOWN lpunk,
    HRESULT* phr
)
{
    if (phr == nullptr)
        return nullptr;

    Windows10VirtualCamera* camera =
        new Windows10VirtualCamera(
            lpunk,
            phr
        );

    return camera;
}


// ============================================================

Windows10VirtualCamera::Windows10VirtualCamera(
    LPUNKNOWN lpunk,
    HRESULT* phr
)
    : CSource(
        L"Cam Studio Virtual Camera",
        lpunk,
        CLSID_CamStudioVirtualCamera,
        phr
    )
{
    DebugLog(
        "FILTER CONSTRUCTOR Windows10VirtualCamera"
    );

    if (phr == nullptr)
        return;

    DebugLog(
        "FILTER: ANTES DE CREAR STREAM"
    );

    Windows10VirtualCameraStream* stream =
        new Windows10VirtualCameraStream(
            phr,
            this
        );

    if (stream == nullptr)
    {
        DebugLog(
            "FILTER: STREAM ALLOCATION FAILED"
        );

        *phr = E_OUTOFMEMORY;
        return;
    }

    DebugLog(
        "FILTER: STREAM CREADO"
    );

    if (FAILED(*phr))
    {
        DebugLog(
            "FILTER: STREAM CONSTRUCTOR FAILED"
        );

        delete stream;
        return;
    }

    DebugLog(
        "FILTER: STREAM CONSTRUCTOR OK"
    );
}


// ============================================================

Windows10VirtualCamera::~Windows10VirtualCamera()
{
    DebugLog(
        "FILTER DESTRUCTOR Windows10VirtualCamera"
    );
}


// ============================================================
// QueryInterface DEL FILTER
// ============================================================

STDMETHODIMP Windows10VirtualCamera::QueryInterface(
    REFIID riid,
    void** ppv
)
{
    if (ppv == nullptr)
        return E_POINTER;

    /*
        IAMStreamConfig e IKsPropertySet pertenecen
        al output pin.

        Muchos programas de videollamadas consultan
        estas interfaces directamente sobre el filtro.
    */

    if (riid == __uuidof(IAMStreamConfig) ||
        riid == __uuidof(IKsPropertySet))
    {
        if (m_paStreams == nullptr ||
            m_iPins <= 0 ||
            m_paStreams[0] == nullptr)
        {
            *ppv = nullptr;
            return E_NOINTERFACE;
        }

        return m_paStreams[0]->QueryInterface(
            riid,
            ppv
        );
    }

    return CSource::QueryInterface(
        riid,
        ppv
    );
}


// ============================================================
// STREAM CONSTRUCTOR
// ============================================================

Windows10VirtualCameraStream::Windows10VirtualCameraStream(
    HRESULT* phr,
    Windows10VirtualCamera* parent
)
    : CSourceStream(
        L"Cam Studio Virtual Camera Stream",
        phr,
        parent,
        L"Video"
    ),
    parent(parent),
    sharedMemory(nullptr),
    frameDuration(CAMERA_FRAME_TIME),
    frameNumber(0)
{
    DebugLog(
        "STREAM CONSTRUCTOR"
    );


    sharedMemory =
        new CamStudioSharedMemory();

    if (sharedMemory == nullptr)
    {
        DebugLog(
            "STREAM: sharedMemory allocation FAILED"
        );

        if (phr)
            *phr = E_OUTOFMEMORY;

        return;
    }

    DebugLog(
        "STREAM: sharedMemory object CREATED"
    );

    if (sharedMemory->Open(
        CAMERA_WIDTH,
        CAMERA_HEIGHT
    ))
    {
        DebugLog(
            "STREAM: sharedMemory OPENED"
        );
    }
    else
    {
        /*
            NO hacemos fallar el constructor.

            Discord puede crear el filtro antes de que
            Cam Studio haya creado la Shared Memory.

            FillBuffer intentará leerla cuando corresponda.
        */

        DebugLog(
            "STREAM: sharedMemory NOT AVAILABLE"
        );
    }

    if (phr)
    {
        char buffer[128];

        sprintf_s(
            buffer,
            "STREAM HRESULT = 0x%08lX",
            static_cast<unsigned long>(*phr)
        );

        DebugLog(buffer);
    }
}


// ============================================================
// STREAM DESTRUCTOR
// ============================================================

Windows10VirtualCameraStream::~Windows10VirtualCameraStream()
{
    DebugLog(
        "STREAM DESTRUCTOR ENTER"
    );

    if (sharedMemory != nullptr)
    {
        delete sharedMemory;
        sharedMemory = nullptr;
    }

    DebugLog(
        "STREAM: sharedMemory DESTROYED"
    );
}


// ============================================================
// IUnknown
// ============================================================

STDMETHODIMP Windows10VirtualCameraStream::QueryInterface(
    REFIID riid,
    void** ppv
)
{
    if (ppv == nullptr)
        return E_POINTER;

    *ppv = nullptr;

    DebugLog(
        "STREAM QueryInterface ENTER"
    );

    if (riid == __uuidof(IAMStreamConfig))
    {
        *ppv =
            static_cast<IAMStreamConfig*>(this);

        AddRef();

        return S_OK;
    }

    if (riid == __uuidof(IKsPropertySet))
    {
        *ppv =
            static_cast<IKsPropertySet*>(this);

        AddRef();

        return S_OK;
    }

    return CSourceStream::QueryInterface(
        riid,
        ppv
    );
}


// ============================================================

STDMETHODIMP_(ULONG)
Windows10VirtualCameraStream::AddRef()
{
    return GetOwner()->AddRef();
}


// ============================================================

STDMETHODIMP_(ULONG)
Windows10VirtualCameraStream::Release()
{
    return GetOwner()->Release();
}


// ============================================================
// MEDIA TYPE
// ============================================================

HRESULT Windows10VirtualCameraStream::GetMediaType(
    int position,
    CMediaType* mediaType
)
{
    if (mediaType == nullptr)
        return E_POINTER;

    if (position < 0)
        return E_INVALIDARG;

    /*
        Solo ofrecemos UN formato.

        1280x720
        RGB24
        30 FPS
    */

    if (position > 0)
        return VFW_S_NO_MORE_ITEMS;

    DebugLog(
        "STREAM: GetMediaType"
    );

    VIDEOINFOHEADER* videoInfo =
        reinterpret_cast<VIDEOINFOHEADER*>(
            mediaType->AllocFormatBuffer(
                sizeof(VIDEOINFOHEADER)
            )
            );

    if (videoInfo == nullptr)
        return E_OUTOFMEMORY;

    ZeroMemory(
        videoInfo,
        sizeof(VIDEOINFOHEADER)
    );

    videoInfo->bmiHeader.biSize =
        sizeof(BITMAPINFOHEADER);

    videoInfo->bmiHeader.biWidth =
        CAMERA_WIDTH;

    /*
        DirectShow RGB normalmente usa
        bitmap bottom-up.
    */

    videoInfo->bmiHeader.biHeight =
        CAMERA_HEIGHT;

    videoInfo->bmiHeader.biPlanes =
        1;

    videoInfo->bmiHeader.biBitCount =
        CAMERA_BPP;

    videoInfo->bmiHeader.biCompression =
        BI_RGB;

    videoInfo->bmiHeader.biSizeImage =
        CAMERA_IMAGE_SIZE;

    videoInfo->AvgTimePerFrame =
        CAMERA_FRAME_TIME;

    SetRectEmpty(
        &videoInfo->rcSource
    );

    SetRectEmpty(
        &videoInfo->rcTarget
    );

    mediaType->SetType(
        &MEDIATYPE_Video
    );

    mediaType->SetSubtype(
        &MEDIASUBTYPE_RGB24
    );

    mediaType->SetFormatType(
        &FORMAT_VideoInfo
    );

    mediaType->SetTemporalCompression(
        FALSE
    );

    mediaType->SetSampleSize(
        CAMERA_IMAGE_SIZE
    );

    return S_OK;
}


// ============================================================

HRESULT Windows10VirtualCameraStream::CheckMediaType(
    const CMediaType* mediaType
)
{
    if (mediaType == nullptr)
        return E_POINTER;

    DebugLog(
        "STREAM: CheckMediaType"
    );

    if (mediaType->majortype != MEDIATYPE_Video)
        return E_INVALIDARG;

    if (mediaType->subtype != MEDIASUBTYPE_RGB24)
        return E_INVALIDARG;

    if (mediaType->formattype != FORMAT_VideoInfo)
        return E_INVALIDARG;

    VIDEOINFOHEADER* videoInfo =
        reinterpret_cast<VIDEOINFOHEADER*>(
            mediaType->Format()
            );

    if (videoInfo == nullptr)
        return E_INVALIDARG;

    char log[256];

    sprintf_s(
        log,
        "CheckMediaType: %ldx%ld %d bpp compression=%lu",
        videoInfo->bmiHeader.biWidth,
        videoInfo->bmiHeader.biHeight,
        videoInfo->bmiHeader.biBitCount,
        videoInfo->bmiHeader.biCompression
    );

    DebugLog(log);

    if (videoInfo->bmiHeader.biWidth != CAMERA_WIDTH)
        return E_INVALIDARG;

    if (videoInfo->bmiHeader.biHeight != CAMERA_HEIGHT)
        return E_INVALIDARG;

    if (videoInfo->bmiHeader.biBitCount != CAMERA_BPP)
        return E_INVALIDARG;

    if (videoInfo->bmiHeader.biCompression != BI_RGB)
        return E_INVALIDARG;

    DebugLog(
        "CheckMediaType: ACCEPTED"
    );

    return S_OK;
}


// ============================================================
// BUFFER
// ============================================================

HRESULT Windows10VirtualCameraStream::DecideBufferSize(
    IMemAllocator* allocator,
    ALLOCATOR_PROPERTIES* properties
)
{
    if (allocator == nullptr)
        return E_POINTER;

    if (properties == nullptr)
        return E_POINTER;

    DebugLog(
        "STREAM: DecideBufferSize"
    );

    properties->cBuffers = 1;

    properties->cbBuffer =
        CAMERA_IMAGE_SIZE;

    properties->cbAlign = 1;

    properties->cbPrefix = 0;

    ALLOCATOR_PROPERTIES actual;

    HRESULT hr =
        allocator->SetProperties(
            properties,
            &actual
        );

    if (FAILED(hr))
        return hr;

    if (actual.cbBuffer < CAMERA_IMAGE_SIZE)
        return E_FAIL;

    return S_OK;
}


// ============================================================
// STREAM THREAD
// ============================================================

HRESULT Windows10VirtualCameraStream::OnThreadCreate()
{
    DebugLog(
        ">>> OnThreadCreate"
    );

    frameNumber = 0;

    /*
        Por ahora no hacemos nada especial.
        CSourceStream se encarga de iniciar
        el worker thread.
    */

    return S_OK;
}


// ============================================================

HRESULT Windows10VirtualCameraStream::OnThreadDestroy()
{
    DebugLog(
        ">>> OnThreadDestroy"
    );

    return S_OK;
}


// ============================================================
// FILL BUFFER
// ============================================================

HRESULT Windows10VirtualCameraStream::FillBuffer(
    IMediaSample* sample
)
{
    if (sample == nullptr)
        return E_POINTER;

    DebugLog(
        "STREAM: FillBuffer"
    );

    BYTE* buffer = nullptr;

    HRESULT hr =
        sample->GetPointer(
            &buffer
        );

    if (FAILED(hr))
    {
        DebugLog(
            "FillBuffer: GetPointer FAILED"
        );

        return hr;
    }

    if (buffer == nullptr)
    {
        DebugLog(
            "FillBuffer: buffer NULL"
        );

        return E_POINTER;
    }

    /*
        ========================================================
        LEER SHARED MEMORY
        ========================================================
    */

    bool frameRead = false;

    if (sharedMemory != nullptr)
    {
        frameRead =
            sharedMemory->ReadFrame(
                buffer,
                CAMERA_IMAGE_SIZE
            );
    }

    if (frameRead)
    {
        DebugLog(
            "FillBuffer: FRAME FROM CAMSTUDIO"
        );
    }
    else
    {
        /*
            Si todavía no existe la Shared Memory
            o no hay frame, devolvemos NEGRO.

            Esto es intencional:
            la cámara sigue siendo válida para Discord
            aunque Cam Studio todavía no esté enviando.
        */

        DebugLog(
            "FillBuffer: NO FRAME -> BLACK"
        );

        ZeroMemory(
            buffer,
            CAMERA_IMAGE_SIZE
        );
    }

    /*
        ========================================================
        TIMING
        ========================================================
    */

    REFERENCE_TIME start =
        frameNumber *
        frameDuration;

    REFERENCE_TIME end =
        start +
        frameDuration;

    hr =
        sample->SetTime(
            &start,
            &end
        );

    if (FAILED(hr))
        return hr;

    hr =
        sample->SetSyncPoint(
            TRUE
        );

    if (FAILED(hr))
        return hr;

    hr =
        sample->SetActualDataLength(
            CAMERA_IMAGE_SIZE
        );

    if (FAILED(hr))
        return hr;

    frameNumber++;

    return S_OK;
}


// ============================================================
// IAMStreamConfig
// ============================================================

STDMETHODIMP
Windows10VirtualCameraStream::SetFormat(
    AM_MEDIA_TYPE* pmt
)
{
    if (pmt == nullptr)
        return E_POINTER;

    HRESULT hr =
        CheckMediaType(
            &CMediaType(*pmt)
        );

    if (FAILED(hr))
        return hr;

    /*
        Por ahora no permitimos cambiar
        resolución/formato.

        Cam Studio trabaja exclusivamente con:

        1280x720 RGB24 @ 30 FPS
    */

    return S_OK;
}


// ============================================================

STDMETHODIMP
Windows10VirtualCameraStream::GetFormat(
    AM_MEDIA_TYPE** ppmt
)
{
    if (ppmt == nullptr)
        return E_POINTER;

    *ppmt = nullptr;

    CMediaType mediaType;

    HRESULT hr =
        GetMediaType(
            0,
            &mediaType
        );

    if (FAILED(hr))
        return hr;

    *ppmt =
        CreateMediaType(
            &mediaType
        );

    if (*ppmt == nullptr)
        return E_OUTOFMEMORY;

    return S_OK;
}


// ============================================================

STDMETHODIMP
Windows10VirtualCameraStream::GetNumberOfCapabilities(
    int* piCount,
    int* piSize
)
{
    if (piCount == nullptr ||
        piSize == nullptr)
    {
        return E_POINTER;
    }

    /*
        Ofrecemos una sola configuración.
    */

    *piCount = 1;

    *piSize =
        sizeof(VIDEO_STREAM_CONFIG_CAPS);

    return S_OK;
}


// ============================================================

STDMETHODIMP
Windows10VirtualCameraStream::GetStreamCaps(
    int iIndex,
    AM_MEDIA_TYPE** ppmt,
    BYTE* pSCC
)
{
    if (ppmt == nullptr ||
        pSCC == nullptr)
    {
        return E_POINTER;
    }

    if (iIndex != 0)
        return E_INVALIDARG;

    *ppmt = nullptr;

    CMediaType mediaType;

    HRESULT hr =
        GetMediaType(
            0,
            &mediaType
        );

    if (FAILED(hr))
        return hr;

    *ppmt =
        CreateMediaType(
            &mediaType
        );

    if (*ppmt == nullptr)
        return E_OUTOFMEMORY;

    VIDEO_STREAM_CONFIG_CAPS* caps =
        reinterpret_cast<
        VIDEO_STREAM_CONFIG_CAPS*
        >(pSCC);

    ZeroMemory(
        caps,
        sizeof(VIDEO_STREAM_CONFIG_CAPS)
    );

    caps->guid =
        FORMAT_VideoInfo;

    caps->VideoStandard =
        AnalogVideo_None;

    caps->InputSize.cx =
        CAMERA_WIDTH;

    caps->InputSize.cy =
        CAMERA_HEIGHT;

    caps->MinCroppingSize.cx =
        CAMERA_WIDTH;

    caps->MinCroppingSize.cy =
        CAMERA_HEIGHT;

    caps->MaxCroppingSize.cx =
        CAMERA_WIDTH;

    caps->MaxCroppingSize.cy =
        CAMERA_HEIGHT;

    caps->CropGranularityX =
        CAMERA_WIDTH;

    caps->CropGranularityY =
        CAMERA_HEIGHT;

    caps->MinOutputSize.cx =
        CAMERA_WIDTH;

    caps->MinOutputSize.cy =
        CAMERA_HEIGHT;

    caps->MaxOutputSize.cx =
        CAMERA_WIDTH;

    caps->MaxOutputSize.cy =
        CAMERA_HEIGHT;

    caps->OutputGranularityX =
        1;

    caps->OutputGranularityY =
        1;

    caps->MinFrameInterval =
        CAMERA_FRAME_TIME;

    caps->MaxFrameInterval =
        CAMERA_FRAME_TIME;

    caps->MinBitsPerSecond =
        CAMERA_IMAGE_SIZE *
        8 *
        30;

    caps->MaxBitsPerSecond =
        CAMERA_IMAGE_SIZE *
        8 *
        30;

    return S_OK;
}


// ============================================================
// IKsPropertySet
// ============================================================

STDMETHODIMP
Windows10VirtualCameraStream::Set(
    REFGUID guidPropSet,
    DWORD dwID,
    void* pInstanceData,
    DWORD cbInstanceData,
    void* pPropData,
    DWORD cbPropData
)
{
    /*
        No necesitamos establecer propiedades
        por ahora.
    */

    return E_NOTIMPL;
}


// ============================================================

STDMETHODIMP
Windows10VirtualCameraStream::Get(
    REFGUID guidPropSet,
    DWORD dwPropID,
    void* pInstanceData,
    DWORD cbInstanceData,
    void* pPropData,
    DWORD cbPropData,
    DWORD* pcbReturned
)
{
    /*
        Le decimos a DirectShow que este pin
        pertenece a una categoría de CAPTURE.

        Esto es importante para que programas
        como Discord lo reconozcan como cámara.
    */

    if (guidPropSet != AMPROPSETID_Pin)
        return E_PROP_SET_UNSUPPORTED;

    if (dwPropID != AMPROPERTY_PIN_CATEGORY)
        return E_PROP_ID_UNSUPPORTED;

    if (pcbReturned != nullptr)
        *pcbReturned = sizeof(GUID);

    if (pPropData == nullptr)
        return S_OK;

    if (cbPropData < sizeof(GUID))
        return E_UNEXPECTED;

    *reinterpret_cast<GUID*>(pPropData) =
        PIN_CATEGORY_CAPTURE;

    return S_OK;
}


// ============================================================

STDMETHODIMP
Windows10VirtualCameraStream::QuerySupported(
    REFGUID guidPropSet,
    DWORD dwPropID,
    DWORD* pTypeSupport
)
{
    if (guidPropSet != AMPROPSETID_Pin)
        return E_PROP_SET_UNSUPPORTED;

    if (dwPropID != AMPROPERTY_PIN_CATEGORY)
        return E_PROP_ID_UNSUPPORTED;

    if (pTypeSupport != nullptr)
    {
        *pTypeSupport =
            KSPROPERTY_SUPPORT_GET;
    }

    return S_OK;
}