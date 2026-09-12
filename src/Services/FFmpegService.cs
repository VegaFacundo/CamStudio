using FFmpeg.AutoGen.Abstractions;
using FFmpeg.AutoGen.Bindings.DynamicallyLoaded;
using System.IO;
using System.Runtime.InteropServices;

namespace CamStudio.Services;

public static unsafe class FFmpegService
{
    public static void Initialize()
    {
        string platform;
        string libraryFolder;

        if (OperatingSystem.IsWindows())
        {
            platform = "windows";
            libraryFolder = "bin";
        }
        else if (OperatingSystem.IsLinux())
        {
            platform = "linux";
            libraryFolder = "lib";
        }
        else
        {
            throw new PlatformNotSupportedException(
                "Sistema operativo no soportado."
            );
        }

        string architecture = RuntimeInformation.ProcessArchitecture switch
        {
            Architecture.X64 => "arch-x64",
            Architecture.Arm64 => "arch-arm64",
            _ => throw new PlatformNotSupportedException(
                "Arquitectura no soportada."
            )
        };

        string ffmpegPath = Path.Combine(
            AppContext.BaseDirectory,
            "ffmpeg",
            platform,
            architecture,
            libraryFolder
        );

        if (!Directory.Exists(ffmpegPath))
        {
            throw new DirectoryNotFoundException(
                $"No se encontró FFmpeg: {ffmpegPath}"
            );
        }

        DynamicallyLoadedBindings.LibrariesPath = ffmpegPath;

        DynamicallyLoadedBindings.ThrowErrorIfFunctionNotFound = true;

        DynamicallyLoadedBindings.Initialize();
    }
}