using System.Windows;
using System.Windows.Shapes;

namespace CamStudio.src.Services.CameraRegistration;

internal static class NativeCameraRegistration
{
    public static bool EnsureRegistered()
    {
        if (OperatingSystem.IsWindows())
        {
            if (OperatingSystem.IsWindowsVersionAtLeast(10, 0, 22000))
                return Windows.Windows11CameraRegistration.EnsureRegistered();

            return Windows.Windows10CameraRegistration.EnsureRegistered();
        }

        if (OperatingSystem.IsLinux())
        {
            return Linux.LinuxCameraRegistration.EnsureRegistered();
        }

        throw new PlatformNotSupportedException();
    }

    public static bool IsRegistered()
    {
        if (OperatingSystem.IsWindows())
        {
            if (OperatingSystem.IsWindowsVersionAtLeast(10, 0, 22000))
                return Windows.Windows11CameraRegistration.IsRegistered();

            return Windows.Windows10CameraRegistration.IsRegistered();
        }

        if (OperatingSystem.IsLinux())
        {
            return Linux.LinuxCameraRegistration.IsRegistered();
        }

        throw new PlatformNotSupportedException();
    }
}