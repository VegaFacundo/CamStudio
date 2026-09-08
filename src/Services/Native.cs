using System.Runtime.InteropServices;



namespace CamStudio.Services;

internal static class Native
{
    private const string DllName =
        "CamStudioVirtualCamera.dll";

    [DllImport(
        DllName,
        CallingConvention = CallingConvention.Cdecl
    )]
    [return: MarshalAs(UnmanagedType.I1)]
    private static extern bool CamStudioInitialize(
        int width,
        int height,
        int fps
    );

    [DllImport(
        DllName,
        CallingConvention = CallingConvention.Cdecl
    )]
    [return: MarshalAs(UnmanagedType.I1)]
    private static extern bool CamStudioWriteFrame(
        byte[] data,
        int size
    );

    [DllImport(
        DllName,
        CallingConvention = CallingConvention.Cdecl
    )]
    private static extern void CamStudioShutdown();

    public static bool Initialize(
        int width,
        int height,
        int fps
    )
    {
        return CamStudioInitialize(
            width,
            height,
            fps
        );
    }

    public static bool WriteFrame(
        byte[] data
    )
    {
        return CamStudioWriteFrame(
            data,
            data.Length
        );
    }

    public static void Shutdown()
    {
        CamStudioShutdown();
    }
}