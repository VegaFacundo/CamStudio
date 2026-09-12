using System.Diagnostics;

namespace CamStudio.Services;

internal static class Logger
{
#if DEBUG
    public const bool Enabled = true;
#else
    public const bool Enabled = false;
#endif

    public static void Log(string message)
    {
        if (!Enabled)
            return;
        System.Diagnostics.Debug.WriteLine(message);
    }
}