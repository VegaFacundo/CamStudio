#pragma once

#include <windows.h>
#include <cstddef>
#include <cstdint>

class CamStudioSharedMemory
{
public:

    CamStudioSharedMemory();

    ~CamStudioSharedMemory();

    bool CreateOrOpen(
        int width,
        int height
    );

    bool Open(
        int width,
        int height
    );

    bool WriteFrame(
        const unsigned char* data,
        size_t size
    );

    bool ReadFrame(
        unsigned char* buffer,
        size_t bufferSize,
        size_t& frameSize
    );

private:

    HANDLE m_mapping;

    void* m_memory;

    size_t m_memorySize;

    int m_width;

    int m_height;

    uint64_t m_frameNumber;
};