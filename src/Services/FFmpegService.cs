using FFmpeg.AutoGen.Abstractions;
using FFmpeg.AutoGen.Bindings.DynamicallyLoaded;
using System.IO;

namespace CamStudio.Services;

public static unsafe class FFmpegService
{
    public static void Initialize()
    {
        string ffmpegPath = Path.Combine(
            AppContext.BaseDirectory,
            "ffmpeg",
            "bin"
        );

        DynamicallyLoadedBindings.LibrariesPath = ffmpegPath;

        DynamicallyLoadedBindings.ThrowErrorIfFunctionNotFound = true;

        DynamicallyLoadedBindings.Initialize();

        Console.WriteLine(
            $"FFmpeg version: {ffmpeg.av_version_info()}"
        );
    }
}