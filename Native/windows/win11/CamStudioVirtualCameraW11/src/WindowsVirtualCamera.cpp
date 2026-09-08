#include "pch.h"
#include "WindowsVirtualCamera.h"

#include <mfapi.h>
#include <mfvirtualcamera.h>

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

bool WindowsVirtualCamera::Initialize(
    int width,
    int height,
    int fps
)
{
    if (width <= 0 ||
        height <= 0 ||
        fps <= 0)
    {
        return false;
    }

    m_width = width;
    m_height = height;
    m_fps = fps;

    HRESULT hr = MFStartup(
        MF_VERSION
    );

    if (FAILED(hr))
    {
        return false;
    }

    hr = MFCreateVirtualCamera(
        MFVirtualCameraType_SoftwareCameraSource,
        MFVirtualCameraLifetime_System,
        MFVirtualCameraAccess_CurrentUser,
        L"Cam Studio Virtual Camera",
        L"{7D8F6B31-4F53-4C5B-9123-7A5E44912011}",
        nullptr,
        0,
        &m_virtualCamera
    );

    if (FAILED(hr))
    {
        MFShutdown();
        return false;
    }

    hr = m_virtualCamera->Start(nullptr);

    if (FAILED(hr))
    {
        m_virtualCamera->Release();
        m_virtualCamera = nullptr;

        MFShutdown();

        return false;
    }

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