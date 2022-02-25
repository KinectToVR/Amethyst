using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.ApplicationModel.Core;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.ViewManagement;
using K2CrashHandler.Helpers;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace K2CrashHandler
{
    /// <summary>
    /// An empty window that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainWindow : Window
    {
        public static int ProcessExitCode;

        public MainWindow()
        {
            InitializeComponent();

            // Prepare placeholder strings (recovery mode)
            String handlerTitle = "Amethyst Recovery",
                handlerContent =
                    "Looks like you've manually ran the Crash Handler. What would you like to do?\n\n" +
                    "If the KinectToVR driver for SteamVR is not being detected properly, you can re-register the SteamVR driver (button below) and then try again.",
                primaryButtonText = "Re-Register Driver",
                secondaryButtonText = "Close";

            // Prepare placeholder callbacks (recovery mode)
            RoutedEventHandler primaryButtonHandler = Action_ReRegister,
                secondaryButtonHandler = Action_Close;

            // Check if there's any argv[1]
            String[] args = Environment.GetCommandLineArgs();
            bool pidMode = args.Length > 1;

            // If we're OK then don't use recovery mode
            //    and wait + optionally parse the crash
            if (pidMode)
            {
                // Get argv[1] for the launch
                String appPid = args[1];

                // Wait for the app to exit and grab the exit code
                try
                {
                    int pid = int.Parse(appPid);
                    Process process = Process.GetProcessById(pid);

                    process.EnableRaisingEvents = true;
                    process.Exited += ProcessEnded;

                    // Display the process statistics until
                    // the user closes the program.
                    do
                    {
                        if (!process.HasExited)
                        {
                            // Refresh the current process property values.
                            process.Refresh();
                        }
                    } while (!process.WaitForExit(3000));

                    // Parse the exit code into strings
                    handlerTitle = "Amethyst has crashed!";
                    primaryButtonText = "View Docs";
                    switch (ProcessExitCode)
                    {
                        case -12:
                        {
                            // No devices
                            primaryButtonHandler = Action_VRDocs;
                            handlerContent =
                                "There were no appropriate devices (plugins) available to load and use for body tracking.\n\n" +
                                "Please check if you have all dependencies installed, like proper Kinect SDK / Runtime and other dependency libraries needed by your devices.";
                        }
                            break;
                        case -11:
                        {
                            // OpenVR error
                            primaryButtonHandler = Action_DeviceDocs;
                            handlerContent =
                                "The app couldn't successfully initialize OpenVR (SteamVR) and decided to give up.\n\n" +
                                "Please check if SteamVR is running and if your HMD's present and working. Additionally, you can restart SteamVR and additionally check its logs.";
                        }
                            break;
                        case 0:
                        {
                            // We're OK
                            this.Close();
                        }
                            break;
                        default:
                        {
                            // Unknown
                            primaryButtonHandler = Action_Discord;
                            handlerContent =
                                "Looks like some weird thing happened to the app.\n\n" +
                                "Don't give up, it (probably) isn't even your fault.\n\n" +
                                "Please try re-running the app.\nIf problem persists, grab logs and reach us on Discord.";
                            primaryButtonText = "Join Discord";
                            }
                            break;
                    }
                }
                catch (Exception)
                {
                    this.Close();
                }
            }

            // Set window title, drag-space and size
            this.Title = pidMode ? "KinectToVR Crash Handler" : "KinectToVR Recovery";
            this.ExtendsContentIntoTitleBar = true;
            this.SetTitleBar(DragElement);
            this.SetWindowSize(WinRT.Interop.WindowNative
                .GetWindowHandle(this), 400, 300);

            // Construct the dialog
            ContentDialogView dialogView = new ContentDialogView
            (
                handlerTitle,
                handlerContent,
                primaryButtonText,
                secondaryButtonText,
                primaryButtonHandler,
                secondaryButtonHandler,
                pidMode
            );

            // And push it into the main grid
            RGrid.Children.Add(dialogView);
        }

        private void Action_ReRegister(object sender, RoutedEventArgs e)
        {
            VRHelper helper = new();
            helper.UpdateSteamPaths();
            // TODO
        }

        private void Action_VRDocs(object sender, RoutedEventArgs e)
        {
            Process.Start("explorer", "https://k2vr.tech/docs/");
        }

        private void Action_DeviceDocs(object sender, RoutedEventArgs e)
        {
            Process.Start("explorer", "https://k2vr.tech/docs/");
        }

        private void Action_Discord(object sender, RoutedEventArgs e)
        {
            Process.Start("explorer.exe", "https://discord.gg/YBQCRDG");
        }

        private void Action_Close(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern UIntPtr OpenProcess(uint dwDesiredAccess, bool bInheritHandle, uint dwProcessId);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool CloseHandle(UIntPtr hObject);

        [DllImport("User32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        public static extern uint GetDpiForWindow(IntPtr hwnd);

        [DllImport("user32.dll", EntryPoint = "SetWindowPos")]
        public static extern IntPtr SetWindowPos(IntPtr hWnd, int hWndInsertAfter,
            int x, int Y, int cx, int cy, int wFlags);

        private void SetWindowSize(IntPtr hwnd, int width, int height)
        {
            var dpi = GetDpiForWindow(hwnd);
            float scalingFactor = (float)dpi / 96;
            width = (int)(width * scalingFactor);
            height = (int)(height * scalingFactor);

            SetWindowPos(hwnd, (int)SpecialWindowHandles.HWND_TOP,
                0, 0, width, height,
                (int)SetWindowPosFlags.SWP_NOMOVE);
        }

        private void ProcessEnded(object sender, EventArgs e)
        {
            var process = sender as Process;
            if (process != null)
            {
                ProcessExitCode = process.ExitCode;
            }
        }
    }
}

public enum SpecialWindowHandles
{
    HWND_TOP = 0,
    HWND_BOTTOM = 1,
    HWND_TOPMOST = -1,
    HWND_NOTOPMOST = -2
}

[Flags]
public enum SetWindowPosFlags : uint
{
    SWP_ASYNCWINDOWPOS = 0x4000,

    SWP_DEFERERASE = 0x2000,

    SWP_DRAWFRAME = 0x0020,

    SWP_FRAMECHANGED = 0x0020,

    SWP_HIDEWINDOW = 0x0080,

    SWP_NOACTIVATE = 0x0010,

    SWP_NOCOPYBITS = 0x0100,

    SWP_NOMOVE = 0x0002,

    SWP_NOOWNERZORDER = 0x0200,

    SWP_NOREDRAW = 0x0008,

    SWP_NOREPOSITION = 0x0200,

    SWP_NOSENDCHANGING = 0x0400,

    SWP_NOSIZE = 0x0001,

    SWP_NOZORDER = 0x0004,

    SWP_SHOWWINDOW = 0x0040,
}