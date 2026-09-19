#include "pch.h"
#include "WindowsVirtualCamera.h"

#include <mfapi.h>
#include <mfvirtualcamera.h>
#include <windows.h>
#include <cstdio>
#include <iostream>

static void DebugLog(const char* text)
{
    OutputDebugStringA(text);
    OutputDebugStringA("\n");
}

WindowsVirtualCamera::WindowsVirtualCamera()
    : m_virtualCamera(nullptr),
    m_width(0),
    m_height(0),
    m_fps(0)
{
}

WindowsVirtualCamera::~WindowsVirtualCamera()
{
    Shutdown();
}

bool WindowsVirtualCamera::Initialize(int width, int height, int fps)
{
    printf("\n=== WindowsVirtualCamera::Initialize ===\n");
    printf("Resolution: %dx%d @ %d FPS\n", width, height, fps);

    if (width <= 0 || height <= 0 || fps <= 0)
    {
        printf("ERROR: invalid parameters\n");
        return false;
    }

    m_width = width;
    m_height = height;
    m_fps = fps;

    printf("Calling MFStartup...\n");

    HRESULT hr = MFStartup(MF_VERSION);

    printf("MFStartup -> 0x%08X\n", (unsigned int)hr);

    if (FAILED(hr))
    {
        printf("ERROR: MFStartup failed\n");
        return false;
    }

    printf("Calling MFCreateVirtualCamera...\n");

    hr = MFCreateVirtualCamera(
        MFVirtualCameraType_SoftwareCameraSource,
        MFVirtualCameraLifetime_Session,
        MFVirtualCameraAccess_CurrentUser,
        L"Cam Studio Virtual Camera",
        L"{7D8F6B31-4F53-4C5B-9123-7A5E44912011}",
        nullptr,
        0,
        &m_virtualCamera
    );

    printf("MFCreateVirtualCamera -> 0x%08X\n", (unsigned int)hr);

    if (FAILED(hr))
    {
        printf("ERROR: MFCreateVirtualCamera failed\n");

        MFShutdown();
        return false;
    }

    printf("\n========================================\n");
    printf("Starting IMFVirtualCamera\n");
    printf("========================================\n");

    SetLastError(ERROR_SUCCESS);

    printf("[1] Calling m_virtualCamera->Start(nullptr)...\n");
    fflush(stdout);

    hr = m_virtualCamera->Start(nullptr);

    DWORD lastError = GetLastError();

    printf("[2] Start returned\n");
    printf("    HRESULT       = 0x%08X\n", (unsigned int)hr);
    printf("    HRESULT_CODE  = 0x%08X\n", (unsigned int)HRESULT_CODE(hr));
    printf("    GetLastError  = %lu (0x%08lX)\n",
        (unsigned long)lastError,
        (unsigned long)lastError);

    if (SUCCEEDED(hr))
    {
        printf("[3] SUCCESS - Virtual camera started\n");
        fflush(stdout);
    }
    else
    {
        printf("[3] FAILURE\n");

        // Decodificar HRESULT
        LPSTR message = nullptr;

        DWORD length = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            HRESULT_CODE(hr),
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
            (LPSTR)&message,
            0,
            nullptr
        );

        if (length > 0 && message != nullptr)
        {
            printf("    Windows message: %s", message);
            LocalFree(message);
        }
        else
        {
            printf("    FormatMessage could not decode error\n");
        }

        printf("\n");
        printf("    This means Start() failed BEFORE our DllGetClassObject.\n");
        printf("    Our COM provider was not reached.\n");

        fflush(stdout);

        m_virtualCamera->Release();
        m_virtualCamera = nullptr;

        MFShutdown();

        return false;
    }

    printf("VIRTUAL CAMERA STARTED!\n");

    return true;
}

void WindowsVirtualCamera::Shutdown()
{
    if (m_virtualCamera != nullptr)
    {
        m_virtualCamera->Stop();
        m_virtualCamera->Release();
        m_virtualCamera = nullptr;

        MFShutdown();
    }

    m_width = 0;
    m_height = 0;
    m_fps = 0;
}

bool WindowsVirtualCamera::IsRunning() const
{
    return m_virtualCamera != nullptr;
}