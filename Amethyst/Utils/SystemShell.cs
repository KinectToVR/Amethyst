using System;
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Runtime.InteropServices;
using System.Security;
using System.Threading.Tasks;
using Windows.System;
using Microsoft.WindowsAPICodePack.Taskbar;

namespace Amethyst.Utils
{
    /// @https://github.com/KinectToVR/Amethyst-Installer @Hekky
    public static class SystemShell
    {
        [DllImport("shell32.dll", SetLastError = true)]
        private static extern int SHOpenFolderAndSelectItems(IntPtr pidlFolder, uint cidl,
            [In] [MarshalAs(UnmanagedType.LPArray)]
            IntPtr[] apidl, uint dwFlags);

        [DllImport("shell32.dll", SetLastError = true)]
        private static extern uint SHParseDisplayName([MarshalAs(UnmanagedType.LPWStr)] string name,
            IntPtr bindingContext,
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
            var fileArray = nativeFile == IntPtr.Zero ? [nativeFolder] : new[] { nativeFile };
            SHOpenFolderAndSelectItems(nativeFolder, (uint)fileArray.Length, fileArray, 0);

            Marshal.FreeCoTaskMem(nativeFolder);
            if (nativeFile != IntPtr.Zero) Marshal.FreeCoTaskMem(nativeFile);
        }

        public static void SetTaskBarProgress(int value)
        {
            try
            {
                if (!TaskbarManager.IsPlatformSupported) return;
                TaskbarManager.Instance.SetProgressState(value > 0
                    ? TaskbarProgressBarState.Normal
                    : TaskbarProgressBarState.Indeterminate);
                TaskbarManager.Instance.SetProgressValue(value, 100);
            }
            catch (Exception)
            {
                // ignored
            }
        }

        public static void SetTaskBarState(TaskbarProgressBarState value)
        {
            try
            {
                if (!TaskbarManager.IsPlatformSupported) return;
                TaskbarManager.Instance.SetProgressState(value);
            }
            catch (Exception)
            {
                // ignored
            }
        }

        public static void FlashTaskBarState(TaskbarProgressBarState value)
        {
            try
            {
                if (!TaskbarManager.IsPlatformSupported) return;
                Task.Run(async () =>
                {
                    TaskbarManager.Instance.SetProgressState(value);
                    await Task.Delay(1000); // Wait a second, then reset to the default state
                    TaskbarManager.Instance.SetProgressState(TaskbarProgressBarState.NoProgress);
                });
            }
            catch (Exception)
            {
                // ignored
            }
        }

        /// <summary>TimeBeginPeriod(). See the Windows API documentation for details.</summary>
        [SuppressMessage("Microsoft.Interoperability", "CA1401:PInvokesShouldNotBeVisible")]
        [SuppressUnmanagedCodeSecurity]
        [DllImport("winmm.dll", EntryPoint = "timeBeginPeriod", SetLastError = true)]
        public static extern uint TimeBeginPeriod(uint uMilliseconds);

        /// <summary>TimeEndPeriod(). See the Windows API documentation for details.</summary>
        [SuppressMessage("Microsoft.Interoperability", "CA1401:PInvokesShouldNotBeVisible")]
        [SuppressUnmanagedCodeSecurity]
        [DllImport("winmm.dll", EntryPoint = "timeEndPeriod", SetLastError = true)]
        public static extern uint TimeEndPeriod(uint uMilliseconds);

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
        private object _mDispatcherQueueController;

        [DllImport("CoreMessaging.dll")]
        private static extern int CreateDispatcherQueueController(
            [In] DispatcherQueueOptions options,
            [In] [Out] [MarshalAs(UnmanagedType.IUnknown)]
            ref object dispatcherQueueController);

        public void EnsureWindowsSystemDispatcherQueueController()
        {
            if (DispatcherQueue.GetForCurrentThread() is not null)
                // one already exists, so we'll just use it.
                return;

            if (_mDispatcherQueueController is not null) return;
            DispatcherQueueOptions options;
            options.dwSize = Marshal.SizeOf(typeof(DispatcherQueueOptions));
            options.threadType = 2; // DQTYPE_THREAD_CURRENT
            options.apartmentType = 2; // DQTAT_COM_STA

            CreateDispatcherQueueController(options, ref _mDispatcherQueueController);
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct DispatcherQueueOptions
        {
            internal int dwSize;
            internal int threadType;
            internal int apartmentType;
        }
    }
}