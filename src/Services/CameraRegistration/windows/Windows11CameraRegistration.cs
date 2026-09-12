using Microsoft.Win32;
using System.IO;
namespace CamStudio.src.Services.CameraRegistration.Windows;

internal static class Windows11CameraRegistration
{
    private const string Clsid =
        "{7D8F6B31-4F53-4C5B-9123-7A5E44912011}";

    private static string ClsidKey =>
        $@"Software\Classes\CLSID\{Clsid}\InprocServer32";

    public static bool IsRegistered()
    {
        using var key =
            Registry.CurrentUser.OpenSubKey(ClsidKey);

        return key != null;
    }

    public static bool EnsureRegistered()
    {
        if (IsRegistered())
            return true;

        return Register();
    }

    private static bool Register()
    {
        string dllPath = Path.Combine(
            AppContext.BaseDirectory,
            "CamStudioVirtualCameraW11.dll"
        );

        if (!File.Exists(dllPath))
            return false;

        using var key =
            Registry.CurrentUser.CreateSubKey(ClsidKey);

        key?.SetValue(
            "",
            dllPath
        );

        key?.SetValue(
            "ThreadingModel",
            "Both"
        );

        return true;
    }
}