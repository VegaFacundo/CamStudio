#include "pch.h"
#include "CamStudioNative.h"
#include "CamStudioSharedMemory.h"

#include <cstdio>
#include <cstddef>
#include "CamStudioNative.h"
#include "CamStudioSharedMemory.h"
#include "WindowsVirtualCamera.h"

static CamStudioSharedMemory* g_sharedMemory = nullptr;
static WindowsVirtualCamera* g_virtualCamera = nullptr;

static void DebugLog(const char* text)
{
    return;
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

        printf(">>> Shared memory initialized\n");
        fflush(stdout);

        g_virtualCamera = new WindowsVirtualCamera();

        if (g_virtualCamera == nullptr)
        {
            printf(">>> ERROR: Could not create WindowsVirtualCamera\n");
            fflush(stdout);

            delete g_sharedMemory;
            g_sharedMemory = nullptr;

            return false;
        }

        printf(">>> Calling WindowsVirtualCamera::Initialize\n");
        fflush(stdout);

        if (!g_virtualCamera->Initialize(width, height, fps))
        {
            printf(">>> ERROR: WindowsVirtualCamera::Initialize FAILED\n");
            fflush(stdout);

            delete g_virtualCamera;
            g_virtualCamera = nullptr;

            delete g_sharedMemory;
            g_sharedMemory = nullptr;

            return false;
        }

        printf(">>> Windows Virtual Camera initialized successfully!\n");
        fflush(stdout);

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

        if (g_virtualCamera != nullptr)
        {
            g_virtualCamera->Shutdown();

            delete g_virtualCamera;
            g_virtualCamera = nullptr;
        }

        delete g_sharedMemory;
        g_sharedMemory = nullptr;
    }
}