using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Amethyst.Utils;

/// @https://github.com/KinectToVR/Amethyst-Installer @Hekky
public static class SystemShell
{
    [DllImport("shell32.dll", SetLastError = true)]
    private static extern int SHOpenFolderAndSelectItems(IntPtr pidlFolder, uint cidl,
        [In] [MarshalAs(UnmanagedType.LPArray)]
        IntPtr[] apidl, uint dwFlags);

    [DllImport("shell32.dll", SetLastError = true)]
    private static extern uint SHParseDisplayName([MarshalAs(UnmanagedType.LPWStr)] string name, IntPtr bindingContext,
        [Out] out IntPtr pidl, uint sfgaoIn, [Out] out uint psfgaoOut);

    [DllImport("shell32.dll", CharSet = CharSet.Unicode)]
    private static extern uint SHGetNameFromIDList(IntPtr pidl, SIGDN sigdnName, [Out] out IntPtr ppszName);

    [DllImport("kernel32", CharSet = CharSet.Unicode, SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool DeleteFile(string name);

    // An alternative to:
    //      Process.Start("explorer.exe", $"/select,{filePath}");
    // P/Invoke version allows us to do the same task without spawning a new instance of explorer.exe
    public static void OpenFolderAndSelectItem(string filePath)
    {
        filePath = Path.GetFullPath(filePath); // Resolve absolute path
        var folderPath = Path.GetDirectoryName(filePath);
        var file = Path.GetFileName(filePath);

        SHParseDisplayName(folderPath, IntPtr.Zero, out var nativeFolder, 0, out _);
        if (nativeFolder == IntPtr.Zero) return;

        SHParseDisplayName(Path.Combine(folderPath, file), IntPtr.Zero, out var nativeFile, 0, out _);

        // Open the folder without the file selected if we can't find the file
        var fileArray = nativeFile == IntPtr.Zero ? new[] { nativeFolder } : new[] { nativeFile };
        SHOpenFolderAndSelectItems(nativeFolder, (uint)fileArray.Length, fileArray, 0);

        Marshal.FreeCoTaskMem(nativeFolder);
        if (nativeFile != IntPtr.Zero) Marshal.FreeCoTaskMem(nativeFile);
    }

    private enum SIGDN : uint
    {
        NORMALDISPLAY = 0x00000000,
        PARENTRELATIVEPARSING = 0x80018001,
        DESKTOPABSOLUTEPARSING = 0x80028000,
        PARENTRELATIVEEDITING = 0x80031001,
        DESKTOPABSOLUTEEDITING = 0x8004c000,
        FILESYSPATH = 0x80058000,
        URL = 0x80068000,
        PARENTRELATIVEFORADDRESSBAR = 0x8007c001,
        PARENTRELATIVE = 0x80080001
    }
}

internal class WindowsSystemDispatcherQueueHelper
{
    [StructLayout(LayoutKind.Sequential)]
    private struct DispatcherQueueOptions
    {
        internal int dwSize;
        internal int threadType;
        internal int apartmentType;
    }

    [DllImport("CoreMessaging.dll")]
    private static extern int CreateDispatcherQueueController(
        [In] DispatcherQueueOptions options,
        [In] [Out] [MarshalAs(UnmanagedType.IUnknown)]
        ref object dispatcherQueueController);

    private object _mDispatcherQueueController = null;

    public void EnsureWindowsSystemDispatcherQueueController()
    {
        if (Windows.System.DispatcherQueue.GetForCurrentThread() != null)
            // one already exists, so we'll just use it.
            return;

        if (_mDispatcherQueueController != null) return;
        DispatcherQueueOptions options;
        options.dwSize = Marshal.SizeOf(typeof(DispatcherQueueOptions));
        options.threadType = 2; // DQTYPE_THREAD_CURRENT
        options.apartmentType = 2; // DQTAT_COM_STA

        CreateDispatcherQueueController(options, ref _mDispatcherQueueController);
    }
}