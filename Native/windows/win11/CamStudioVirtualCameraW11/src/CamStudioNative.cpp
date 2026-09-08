#include "pch.h"
#include "CamStudioNative.h"
#include "CamStudioSharedMemory.h"

#include <cstdio>
#include <cstddef>

static CamStudioSharedMemory* g_sharedMemory = nullptr;

static void DebugLog(const char* text)
{
    FILE* file = nullptr;

    fopen_s(
        &file,
        "camera_native_debug.txt",
        "a"
    );

    if (file)
    {
        fprintf(file, "%s\n", text);
        fclose(file);
    }
}

extern "C"
{
    CAMSTUDIO_API bool CamStudioInitialize(
        int width,
        int height,
        int fps
    )
    {
        DebugLog("CamStudioInitialize ENTER");

        if (g_sharedMemory != nullptr)
        {
            DebugLog("SHARED MEMORY ALREADY INITIALIZED");
            return true;
        }

        if (width <= 0 || height <= 0 || fps <= 0)
        {
            DebugLog("INVALID INITIALIZATION PARAMETERS");
            return false;
        }

        g_sharedMemory = new CamStudioSharedMemory();

        if (g_sharedMemory == nullptr)
        {
            DebugLog("SHARED MEMORY OBJECT FAILED");
            return false;
        }

        if (!g_sharedMemory->CreateOrOpen(
            width,
            height))
        {
            DebugLog("SHARED MEMORY FAILED");

            delete g_sharedMemory;
            g_sharedMemory = nullptr;

            return false;
        }

        DebugLog("SHARED MEMORY CREATED");

        return true;
    }

    CAMSTUDIO_API bool CamStudioWriteFrame(
        const unsigned char* data,
        int size
    )
    {
        if (g_sharedMemory == nullptr)
        {
            DebugLog("SHARED MEMORY OBJECT NULL");
            return false;
        }

        if (data == nullptr || size <= 0)
        {
            DebugLog("INVALID FRAME");
            return false;
        }

        const bool result =
            g_sharedMemory->WriteFrame(
                data,
                static_cast<size_t>(size)
            );

        return result;
    }

    CAMSTUDIO_API void CamStudioShutdown()
    {
        DebugLog("CamStudioShutdown");

        delete g_sharedMemory;
        g_sharedMemory = nullptr;
    }
}