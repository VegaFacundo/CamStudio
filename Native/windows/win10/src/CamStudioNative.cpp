#include "CamStudioNative.h"
#include "CamStudioSharedMemory.h"
#include <fstream>

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

    bool CamStudioInitialize(
        int width,
        int height,
        int fps
    )
    {
        DebugLog("JNI: CamStudioInitialize ENTER");

        if (g_sharedMemory != nullptr)
        {
            DebugLog("JNI: SHARED MEMORY ALREADY INITIALIZED");
            return true;
        }

        g_sharedMemory = new CamStudioSharedMemory();

        if (g_sharedMemory == nullptr)
        {
            DebugLog("JNI: SHARED MEMORY OBJECT FAILED");
            return false;
        }

        if (!g_sharedMemory->CreateOrOpen(
            width,
            height))
        {
            DebugLog("JNI: SHARED MEMORY FAILED");

            delete g_sharedMemory;
            g_sharedMemory = nullptr;

            return false;
        }

        DebugLog("JNI: SHARED MEMORY CREATED");

        return true;
    }


    bool CamStudioWriteFrame(
        const unsigned char* data,
        int size
    )
    {
        DebugLog("JNI: CamStudioWriteFrame ENTER");

        if (g_sharedMemory == nullptr)
        {
            DebugLog("JNI: SHARED MEMORY OBJECT NULL");
            return false;
        }

        if (data == nullptr || size <= 0)
        {
            DebugLog("JNI: INVALID FRAME");
            return false;
        }

        bool result =
            g_sharedMemory->WriteFrame(
                data,
                static_cast<size_t>(size)
            );

        DebugLog(
            result
            ? "JNI: SHARED MEMORY WRITE OK"
            : "JNI: SHARED MEMORY WRITE FAILED"
        );

        return result;
    }


    void CamStudioShutdown()
    {
        DebugLog("JNI: CamStudioShutdown");

        delete g_sharedMemory;
        g_sharedMemory = nullptr;
    }

}