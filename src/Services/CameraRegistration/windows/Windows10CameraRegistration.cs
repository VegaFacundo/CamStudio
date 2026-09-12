namespace CamStudio.src.Services.CameraRegistration.Windows;

internal static class Windows10CameraRegistration
{
    public static bool IsRegistered()
    {
        return false;
    }

    public static bool EnsureRegistered()
    {
        return Register();
    }

    private static bool Register()
    {
        // Pendiente:
        // - registrar CLSID
        // - categoría VideoInputDeviceCategory
        // - DirectShow FilterMapper2

        return true;
    }
}