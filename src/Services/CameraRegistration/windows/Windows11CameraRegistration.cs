using Microsoft.Win32;
using System.Diagnostics;
using System.IO;

namespace CamStudio.src.Services.CameraRegistration.Windows;

internal static class Windows11CameraRegistration
{
    private const string Clsid =
        "{7D8F6B31-4F53-4C5B-9123-7A5E44912011}";

    private const string RegisterArgument =
        "--register-windows11-camera";

    private static string ClsidKey =>
        $@"Software\Classes\CLSID\{Clsid}\InprocServer32";

    private static string DllPath =>
        Path.Combine(
            AppContext.BaseDirectory,
            "CamStudioVirtualCameraW11.dll"
        );

    public static bool EnsureRegistered()
    {
        if (!File.Exists(DllPath))
            return false;

        // Siempre sincronizar HKCU.
        if (!Register(Registry.CurrentUser))
            return false;

        // HKLM ya está correctamente registrado.
        if (IsRegistered(Registry.LocalMachine))
            return true;

        // HKLM necesita permisos de administrador.
        return RegisterAsAdministrator();
    }

    public static bool IsRegistered()
    {
        if (!File.Exists(DllPath))
            return false;

        return
            IsRegistered(Registry.CurrentUser) &&
            IsRegistered(Registry.LocalMachine);
    }

    // Método utilizado exclusivamente por la instancia elevada.
    public static bool RegisterForElevatedProcess()
    {
        if (!File.Exists(DllPath))
            return false;

        return Register(Registry.LocalMachine);
    }

    private static bool IsRegistered(RegistryKey hive)
    {
        using var key = hive.OpenSubKey(ClsidKey);

        if (key == null)
            return false;

        string? registeredPath = key.GetValue("") as string;
        string? threadingModel = key.GetValue("ThreadingModel") as string;

        return
            string.Equals(
                registeredPath,
                DllPath,
                StringComparison.OrdinalIgnoreCase
            )
            &&
            string.Equals(
                threadingModel,
                "Both",
                StringComparison.OrdinalIgnoreCase
            );
    }

    private static bool Register(RegistryKey hive)
    {
        try
        {
            using var key = hive.CreateSubKey(ClsidKey);

            if (key == null)
                return false;

            key.SetValue("", DllPath);
            key.SetValue("ThreadingModel", "Both");

            return IsRegistered(hive);
        }
        catch
        {
            return false;
        }
    }

    private static bool RegisterAsAdministrator()
    {
        try
        {
            string? processPath = Environment.ProcessPath;

            if (string.IsNullOrEmpty(processPath))
                return false;

            using var process = Process.Start(new ProcessStartInfo
            {
                FileName = processPath,
                Arguments = RegisterArgument,
                UseShellExecute = true,
                Verb = "runas"
            });

            if (process == null)
                return false;

            process.WaitForExit();

            return
                process.ExitCode == 0 &&
                IsRegistered(Registry.LocalMachine);
        }
        catch
        {
            return false;
        }
    }
}