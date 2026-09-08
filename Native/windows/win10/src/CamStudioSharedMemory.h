#pragma once

#include <windows.h>
#include <cstddef>

class CamStudioSharedMemory
{
public:

    CamStudioSharedMemory();
    ~CamStudioSharedMemory();

    bool CreateOrOpen(
        LONG width,
        LONG height
    );

    bool Open(
        LONG width,
        LONG height
    );

    bool WriteFrame(
        const BYTE* data,
        size_t size
    );

    bool ReadFrame(
        BYTE* destination,
        size_t size
    );

    bool HasFrame() const;

    void ClearFrame();

private:

    HANDLE mappingHandle;
    BYTE* sharedMemory;

    LONG width;
    LONG height;

    size_t frameSize;

    bool owner;
};