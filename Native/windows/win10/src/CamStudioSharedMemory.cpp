#include "CamStudioSharedMemory.h"

#include <cstring>
#include <cstdio>

static const wchar_t* SHARED_MEMORY_NAME =
L"CamStudio_Source_0";


CamStudioSharedMemory::CamStudioSharedMemory()
    : mappingHandle(nullptr),
    sharedMemory(nullptr),
    width(0),
    height(0),
    frameSize(0),
    owner(false)
{
}


CamStudioSharedMemory::~CamStudioSharedMemory()
{
    if (sharedMemory != nullptr)
    {
        UnmapViewOfFile(sharedMemory);
        sharedMemory = nullptr;
    }

    if (mappingHandle != nullptr)
    {
        CloseHandle(mappingHandle);
        mappingHandle = nullptr;
    }
}


bool CamStudioSharedMemory::CreateOrOpen(
    LONG width,
    LONG height
)
{

    if (width <= 0 || height <= 0)
    {
        return false;
    }

    if (sharedMemory != nullptr)
    {
        return true;
    }

    this->width = width;
    this->height = height;

    this->frameSize =
        static_cast<size_t>(width) *
        static_cast<size_t>(height) *
        3;

    char log[256];

    sprintf_s(
        log,
        "SHM: frameSize=%zu",
        frameSize
    );


    SetLastError(ERROR_SUCCESS);

    mappingHandle =
        CreateFileMappingW(
            INVALID_HANDLE_VALUE,
            nullptr,
            PAGE_READWRITE,
            static_cast<DWORD>(
                (static_cast<unsigned long long>(frameSize) >> 32)
                ),
            static_cast<DWORD>(
                static_cast<unsigned long long>(frameSize) & 0xFFFFFFFF
                ),
            SHARED_MEMORY_NAME
        );

    if (mappingHandle == nullptr)
    {
        DWORD error = GetLastError();

        sprintf_s(
            log,
            "SHM: CreateFileMapping FAILED error=%lu",
            error
        );


        return false;
    }


    DWORD createError = GetLastError();

    if (createError == ERROR_ALREADY_EXISTS)
    {
        owner = false;
       
    }
    else
    {
        owner = true;
       
    }


    sharedMemory =
        static_cast<BYTE*>(
            MapViewOfFile(
                mappingHandle,
                FILE_MAP_ALL_ACCESS,
                0,
                0,
                frameSize
            )
            );

    if (sharedMemory == nullptr)
    {
        DWORD error = GetLastError();

        sprintf_s(
            log,
            "SHM: MapViewOfFile FAILED error=%lu",
            error
        );

        CloseHandle(mappingHandle);
        mappingHandle = nullptr;

        return false;
    }


    sprintf_s(
        log,
        "SHM: MAP OK address=%p",
        static_cast<void*>(sharedMemory)
    );


    return true;
}


bool CamStudioSharedMemory::WriteFrame(
    const BYTE* data,
    size_t size
)
{
    if (sharedMemory == nullptr)
    {
        

        return false;
    }

    if (data == nullptr)
    {

        return false;
    }

    if (size != frameSize)
    {
        

        return false;
    }

    std::memcpy(
        sharedMemory,
        data,
        frameSize
    );

    return true;
}


bool CamStudioSharedMemory::ReadFrame(
    BYTE* destination,
    size_t size
)
{
    if (sharedMemory == nullptr)
        return false;

    if (destination == nullptr)
        return false;

    if (size != frameSize)
        return false;

    std::memcpy(
        destination,
        sharedMemory,
        frameSize
    );

    return true;
}


bool CamStudioSharedMemory::HasFrame() const
{
    return sharedMemory != nullptr;
}


void CamStudioSharedMemory::ClearFrame()
{
    if (sharedMemory == nullptr)
        return;

    std::memset(
        sharedMemory,
        0,
        frameSize
    );
}

bool CamStudioSharedMemory::Open(
    LONG width,
    LONG height
)
{

    if (width <= 0 || height <= 0)
    {
        return false;
    }

    this->width = width;
    this->height = height;

    this->frameSize =
        static_cast<size_t>(width) *
        static_cast<size_t>(height) *
        3;


    mappingHandle =
        OpenFileMappingW(
            FILE_MAP_ALL_ACCESS,
            FALSE,
            SHARED_MEMORY_NAME
        );

    if (mappingHandle == nullptr)
    {
        DWORD error = GetLastError();

        char buffer[128];

        sprintf_s(
            buffer,
            "SHM: OpenFileMapping FAILED error=%lu",
            error
        );


        return false;
    }


    sharedMemory =
        static_cast<BYTE*>(
            MapViewOfFile(
                mappingHandle,
                FILE_MAP_ALL_ACCESS,
                0,
                0,
                frameSize
            )
            );

    if (sharedMemory == nullptr)
    {
        DWORD error = GetLastError();

        char buffer[128];

        sprintf_s(
            buffer,
            "SHM: MapViewOfFile FAILED error=%lu",
            error
        );


        CloseHandle(mappingHandle);
        mappingHandle = nullptr;

        return false;
    }

    char buffer[128];

    sprintf_s(
        buffer,
        "SHM: MAP OK address=%p",
        static_cast<void*>(sharedMemory)
    );


    return true;
}