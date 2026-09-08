#include "pch.h"
#include "CamStudioSharedMemory.h"

#include <cstring>

namespace
{
    constexpr const wchar_t* SHARED_MEMORY_NAME =
        L"CamStudioVirtualCameraSharedMemory";

    struct SharedMemoryHeader
    {
        int width;
        int height;
        int frameSize;

        uint64_t frameNumber;
    };
}

CamStudioSharedMemory::CamStudioSharedMemory()
    : m_mapping(nullptr),
    m_memory(nullptr),
    m_memorySize(0),
    m_width(0),
    m_height(0),
    m_frameNumber(0)
{
}

bool CamStudioSharedMemory::Open(
    int width,
    int height
)
{
    if (width <= 0 || height <= 0)
        return false;

    m_width = width;
    m_height = height;

    const size_t frameSize =
        static_cast<size_t>(width) *
        static_cast<size_t>(height) *
        3;

    const size_t headerSize =
        sizeof(SharedMemoryHeader);

    m_memorySize =
        headerSize + frameSize;


    m_mapping =
        OpenFileMappingW(
            FILE_MAP_ALL_ACCESS,
            FALSE,
            SHARED_MEMORY_NAME
        );

    if (m_mapping == nullptr)
        return false;


    m_memory =
        MapViewOfFile(
            m_mapping,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            m_memorySize
        );

    if (m_memory == nullptr)
    {
        CloseHandle(m_mapping);
        m_mapping = nullptr;

        return false;
    }


    auto* header =
        static_cast<SharedMemoryHeader*>(
            m_memory
            );


    if (header->width != width ||
        header->height != height)
    {
        UnmapViewOfFile(m_memory);
        m_memory = nullptr;

        CloseHandle(m_mapping);
        m_mapping = nullptr;

        return false;
    }


    m_frameNumber =
        header->frameNumber;

    return true;
}

CamStudioSharedMemory::~CamStudioSharedMemory()
{
    if (m_memory != nullptr)
    {
        UnmapViewOfFile(m_memory);
        m_memory = nullptr;
    }

    if (m_mapping != nullptr)
    {
        CloseHandle(m_mapping);
        m_mapping = nullptr;
    }
}

bool CamStudioSharedMemory::CreateOrOpen(
    int width,
    int height
)
{
    if (width <= 0 || height <= 0)
    {
        return false;
    }

    m_width = width;
    m_height = height;

    const size_t frameSize =
        static_cast<size_t>(width) *
        static_cast<size_t>(height) *
        3;

    const size_t headerSize =
        sizeof(SharedMemoryHeader);

    m_memorySize =
        headerSize + frameSize;

    m_mapping = CreateFileMappingW(
        INVALID_HANDLE_VALUE,
        nullptr,
        PAGE_READWRITE,
        static_cast<DWORD>(
            (m_memorySize >> 32) & 0xFFFFFFFF
            ),
        static_cast<DWORD>(
            m_memorySize & 0xFFFFFFFF
            ),
        SHARED_MEMORY_NAME
    );

    if (m_mapping == nullptr)
    {
        return false;
    }

    m_memory = MapViewOfFile(
        m_mapping,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        m_memorySize
    );

    if (m_memory == nullptr)
    {
        CloseHandle(m_mapping);
        m_mapping = nullptr;

        return false;
    }

    auto* header =
        static_cast<SharedMemoryHeader*>(m_memory);

    header->width = width;
    header->height = height;
    header->frameSize = 0;
    header->frameNumber = 0;

    m_frameNumber = 0;

    return true;
}

bool CamStudioSharedMemory::WriteFrame(
    const unsigned char* data,
    size_t size
)
{
    if (m_memory == nullptr)
    {
        return false;
    }

    if (data == nullptr || size == 0)
    {
        return false;
    }

    const size_t maxFrameSize =
        static_cast<size_t>(m_width) *
        static_cast<size_t>(m_height) *
        3;

    if (size > maxFrameSize)
    {
        return false;
    }

    auto* header =
        static_cast<SharedMemoryHeader*>(m_memory);

    auto* frameData =
        reinterpret_cast<unsigned char*>(
            m_memory
            ) + sizeof(SharedMemoryHeader);

    std::memcpy(
        frameData,
        data,
        size
    );

    ++m_frameNumber;

    header->frameSize =
        static_cast<int>(size);

    header->frameNumber =
        m_frameNumber;

    return true;
}

bool CamStudioSharedMemory::ReadFrame(
    unsigned char* buffer,
    size_t bufferSize,
    size_t& frameSize)
{
    frameSize = 0;

    if (m_memory == nullptr)
        return false;

    if (buffer == nullptr || bufferSize == 0)
        return false;

    auto* header =
        static_cast<SharedMemoryHeader*>(m_memory);

    if (header->frameSize <= 0)
        return false;

    if (static_cast<size_t>(header->frameSize) > bufferSize)
        return false;

    auto* frameData =
        reinterpret_cast<unsigned char*>(m_memory) +
        sizeof(SharedMemoryHeader);

    std::memcpy(
        buffer,
        frameData,
        static_cast<size_t>(header->frameSize)
    );

    frameSize =
        static_cast<size_t>(header->frameSize);

    return true;
}