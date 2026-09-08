#pragma once

#ifdef CAMSTUDIOVIRTUALCAMERAW11_EXPORTS
#define CAMSTUDIO_API __declspec(dllexport)
#else
#define CAMSTUDIO_API __declspec(dllimport)
#endif

extern "C"
{
    CAMSTUDIO_API bool CamStudioInitialize(
        int width,
        int height,
        int fps
    );

    CAMSTUDIO_API bool CamStudioWriteFrame(
        const unsigned char* data,
        int size
    );

    CAMSTUDIO_API void CamStudioShutdown();
}