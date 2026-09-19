#include "CamStudioNative.h"
#include "CamStudioSharedMemory.h"
#include <fstream>

static CamStudioSharedMemory* g_sharedMemory = nullptr;

extern "C"
{

    bool CamStudioInitialize(
        int width,
        int height,
        int fps
    )
    {

        if (g_sharedMemory != nullptr)
        {
            return true;
        }

        g_sharedMemory = new CamStudioSharedMemory();

        if (g_sharedMemory == nullptr)
        {
            return false;
        }

        if (!g_sharedMemory->CreateOrOpen(
            width,
            height))
        {

            delete g_sharedMemory;
            g_sharedMemory = nullptr;

            return false;
        }


        return true;
    }


    bool CamStudioWriteFrame(
        const unsigned char* data,
        int size
    )
    {

        if (g_sharedMemory == nullptr)
        {
            return false;
        }

        if (data == nullptr || size <= 0)
        {
            return false;
        }

        bool result =
            g_sharedMemory->WriteFrame(
                data,
                static_cast<size_t>(size)
            );

        

        return result;
    }


    void CamStudioShutdown()
    {

        delete g_sharedMemory;
        g_sharedMemory = nullptr;
    }

}