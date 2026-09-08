#pragma once

#include <mfvirtualcamera.h>

class WindowsVirtualCamera
{
public:

    WindowsVirtualCamera();

    ~WindowsVirtualCamera();

    bool Initialize(
        int width,
        int height,
        int fps
    );

    void Shutdown();

    bool IsRunning() const;

private:

    IMFVirtualCamera* m_virtualCamera;

    int m_width;
    int m_height;
    int m_fps;
};