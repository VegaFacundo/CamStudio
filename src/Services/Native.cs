using System.Runtime.InteropServices;

namespace CamStudio.Services;

internal static class Native
{
    private static IntPtr _library;

    private static string GetDllName()
    {
        if (OperatingSystem.IsWindows())
        {
            var version = Environment.OSVersion.Version;

            if (version.Build >= 22000)
                return "CamStudioVirtualCameraW11.dll";

            return "CamStudioVirtualCamera.dll";
        }

        if (OperatingSystem.IsLinux())
        {
            return "libCamStudioVirtualCamera.so";
        }

        throw new PlatformNotSupportedException(
            $"Sistema operativo no soportado: {RuntimeInformation.OSDescription}"
        );
    }

    private static void LoadLibrary()
    {
        if (_library != IntPtr.Zero)
            return;

        var dllName = GetDllName();

        _library = NativeLibrary.Load(dllName);
    }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    private delegate bool CamStudioInitializeDelegate(
        int width,
        int height,
        int fps
    );

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    private delegate bool CamStudioWriteFrameDelegate(
        IntPtr data,
        int size
    );

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate void CamStudioShutdownDelegate();

    private static CamStudioInitializeDelegate? _initialize;

    private static CamStudioWriteFrameDelegate? _writeFrame;

    private static CamStudioShutdownDelegate? _shutdown;

    private static void LoadFunctions()
    {
        LoadLibrary();

        _initialize =
            Marshal.GetDelegateForFunctionPointer<CamStudioInitializeDelegate>(
                NativeLibrary.GetExport(
                    _library,
                    "CamStudioInitialize"
                )
            );

        _writeFrame =
            Marshal.GetDelegateForFunctionPointer<CamStudioWriteFrameDelegate>(
                NativeLibrary.GetExport(
                    _library,
                    "CamStudioWriteFrame"
                )
            );

        _shutdown =
            Marshal.GetDelegateForFunctionPointer<CamStudioShutdownDelegate>(
                NativeLibrary.GetExport(
                    _library,
                    "CamStudioShutdown"
                )
            );
    }

    public static bool Initialize(
        int width,
        int height,
        int fps
    )
    {
        LoadFunctions();

        return _initialize!(
            width,
            height,
            fps
        );
    }

    public static bool WriteFrame(
        byte[] data
    )
    {
        if (_writeFrame == null)
            throw new InvalidOperationException(
                "Native library not initialized."
            );

        var handle =
            GCHandle.Alloc(
                data,
                GCHandleType.Pinned
            );

        try
        {
            return _writeFrame!(
                handle.AddrOfPinnedObject(),
                data.Length
            );
        }
        finally
        {
            handle.Free();
        }
    }

    public static void Shutdown()
    {
        if (_shutdown != null)
        {
            _shutdown();
        }

        _initialize = null;
        _writeFrame = null;
        _shutdown = null;

        if (_library != IntPtr.Zero)
        {
            NativeLibrary.Free(_library);
            _library = IntPtr.Zero;
        }
    }
}