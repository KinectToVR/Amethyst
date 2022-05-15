using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using Windows.Graphics;
using K2CrashHandler.Helpers;
using K2InsightsHandler;
using Microsoft.UI;
using Microsoft.UI.Windowing;
using Microsoft.UI.Xaml;
using WinRT.Interop;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace K2CrashHandler
{
    public sealed partial class MainWindow : Window
    {
        public static int ProcessExitCode;

        public MainWindow()
        {
            InitializeComponent();

            // Prepare placeholder strings (recovery mode)
            string handlerTitle = "Amethyst Recovery",
                handlerContent =
                    "Looks like you've manually ran the Crash Handler. What would you like to do?\n\n" +
                    "If the Amethyst driver for SteamVR is not being detected properly, you can re-register the SteamVR driver (button below) and then try again.",
                primaryButtonText = "Re-Register Driver",
                secondaryButtonText = "Close";

            // Prepare placeholder callbacks (recovery mode)
            RoutedEventHandler primaryButtonHandler = Action_ReRegister,
                secondaryButtonHandler = Action_Close;

            // Check if there's any argv[1]
            var args = Environment.GetCommandLineArgs();
            bool launcherMode = args.Length > 1,
                relaunchMode = false;

            if (launcherMode)
                relaunchMode = args[1].Contains("already_running");

            // If we're OK then don't use recovery mode
            //    and wait + optionally parse the crash
            if (launcherMode)
            {
                if (!relaunchMode)
                {
                    // Get argv[1] for the launch
                    var appPid = args[1];

                    // Wait for the app to exit and grab the exit code
                    try
                    {
                        var pid = int.Parse(appPid);
                        var process = Process.GetProcessById(pid);

                        process.EnableRaisingEvents = true;
                        process.Exited += ProcessEnded;

                        // Display the process statistics until
                        // the user closes the program.
                        do
                        {
                            if (!process.HasExited)
                                // Refresh the current process property values.
                                process.Refresh();
                        } while (!process.WaitForExit(3000));

                        // Handle normal exit
                        if (ProcessExitCode == 0)
                            Close(); // We're OK to exit

                        // Register AI handler
                        var handler = new InsightsHandler();
                        handler.Initialize();

                        // Parse the exit code into strings
                        handlerTitle = "Amethyst's given you up!";
                        primaryButtonText = "View Docs";
                        switch (ProcessExitCode)
                        {
                            case -13:
                            {
                                // Panic exit
                                primaryButtonHandler = Action_ResetConfig;
                                handlerContent =
                                    "The main program loop has crashed consequently more than 7 times so Amethyst was shut down.\n\n" +
                                    "Please check if all your devices / plugins are working and check logs. Optionally, you can erase the app configuration and then try running it again.";
                                primaryButtonText = "Reset Config";

                                handler.LogException(new OverflowException("Too many main loop crashes"));
                            }
                                break;
                            case -12:
                            {
                                // No devices
                                primaryButtonHandler = Action_VRDocs;
                                handlerContent =
                                    "There were no appropriate devices (plugins) available to load and use for body tracking.\n\n" +
                                    "Please check if you have all dependencies installed, like proper Kinect SDK / Runtime and other dependency libraries needed by your devices.";

                                handler.LogException(new MissingFieldException("No devices available"));
                            }
                                break;
                            case -11:
                            {
                                // OpenVR error
                                primaryButtonHandler = Action_DeviceDocs;
                                handlerContent =
                                    "The app couldn't successfully initialize OpenVR (SteamVR) and decided to give up.\n\n" +
                                    "Please check if SteamVR is running and if your HMD's present and working. Additionally, you can restart SteamVR and additionally check its logs.";

                                handler.LogException(new NotSupportedException("OpenVR initialization error"));
                            }
                                break;
                            case 0:
                            {
                                // We're OK
                                Close();
                            }
                                break;
                            default:
                            {
                                // Unknown
                                primaryButtonHandler = Action_Discord;
                                handlerContent =
                                    "Looks like some weird thing happened to the app.\n\n" +
                                    "Don't panic yet, it (probably) isn't even your fault.\n\n" +
                                    "Please try re-running the app.\nIf problem persists, grab logs and reach us on Discord.";
                                primaryButtonText = "Join Discord";

                                handler.LogException(new NotImplementedException("Something else this time"));
                            }
                                break;
                        }
                    }
                    catch (Exception)
                    {
                        Close();
                    }
                }
                else
                {
                    // Parse the strings
                    handlerTitle = "Amethyst Already Running!";
                    primaryButtonText = "View Docs";

                    primaryButtonHandler = Action_ForceQuit;
                    handlerContent =
                        "Looks like the app is already running \nand you've tried to " +
                        "launch a second instance,\nthis action is not currently supported.\n\n" +
                        "Please check if the app isn't opened.\nIf problem persists, press the 'Force Exit' button.";
                    primaryButtonText = "Force Exit";
                }
            }

            // Prepare window size consts
            const int width = 400,
                height = 295;

            // Set window title, drag-space and size
            Title = launcherMode
                ? relaunchMode
                    ? "Amethyst Already Running!"
                    : "Amethyst Crash Handler"
                : "Amethyst Recovery";

            //this.ExtendsContentIntoTitleBar = true;
            //this.SetTitleBar(DragElement);
            SetWindowSize(WindowNative
                .GetWindowHandle(this), width, height);

            var appWindow = AppWindow.GetFromWindowId(
                Win32Interop.GetWindowIdFromWindow(
                    WindowNative.GetWindowHandle(this)));

            if (AppWindowTitleBar.IsCustomizationSupported())
            {
                // Chad Windows 11

                appWindow.TitleBar.ExtendsContentIntoTitleBar = true;
                appWindow.TitleBar.SetDragRectangles(new RectInt32[]
                {
                    new(0, 0, 10000000, 30)
                });

                appWindow.TitleBar.ButtonBackgroundColor = Colors.Transparent;
                appWindow.TitleBar.ButtonInactiveBackgroundColor = Colors.Transparent;
                appWindow.TitleBar.ButtonHoverBackgroundColor =
                    appWindow.TitleBar.ButtonPressedBackgroundColor;
            }
            else
                // Poor ass Windows 10
            {
                ExtendsContentIntoTitleBar = true;
            }

            // Center the window on the screen
            SetWindowPos(WindowNative
                    .GetWindowHandle(this), 0x0000,
                GetSystemMetrics(SystemMetric.SM_CXSCREEN) / 2 - width / 2,
                GetSystemMetrics(SystemMetric.SM_CYSCREEN) / 2 - height / 2,
                width, height, 0x0040); // SWP_SHOWWINDOW

            // And show it (forcibly)
            SetForegroundWindow(WindowNative
                .GetWindowHandle(this));

            // Construct the dialog
            var dialogView = new ContentDialogView
            (
                handlerTitle,
                handlerContent,
                primaryButtonText,
                secondaryButtonText,
                primaryButtonHandler,
                secondaryButtonHandler,
                launcherMode
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

        private void Action_ResetConfig(object sender, RoutedEventArgs e)
        {
            File.Delete(Path.Combine(Environment.GetFolderPath(
                Environment.SpecialFolder.ApplicationData), "Amethyst", "Amethyst_settings.xml"));
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

        private void Action_ForceQuit(object sender, RoutedEventArgs e)
        {
            foreach (var process in Process.GetProcessesByName("Amethyst"))
                process.Kill();
        }

        private void Action_Close(object sender, RoutedEventArgs e)
        {
            Close();
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

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool SetForegroundWindow(IntPtr hWnd);

        [DllImport("user32.dll")]
        private static extern int GetSystemMetrics(SystemMetric smIndex);

        private void SetWindowSize(IntPtr hwnd, int width, int height)
        {
            var dpi = GetDpiForWindow(hwnd);
            var scalingFactor = (float)dpi / 96;
            width = (int)(width * scalingFactor);
            height = (int)(height * scalingFactor);

            SetWindowPos(hwnd, (int)SpecialWindowHandles.HWND_TOP,
                0, 0, width, height,
                (int)SetWindowPosFlags.SWP_NOMOVE);
        }

        private void ProcessEnded(object sender, EventArgs e)
        {
            var process = sender as Process;
            if (process != null) ProcessExitCode = process.ExitCode;
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

    SWP_SHOWWINDOW = 0x0040
}

public enum SystemMetric
{
    SM_CXSCREEN = 0, // 0x00
    SM_CYSCREEN = 1, // 0x01
    SM_CXVSCROLL = 2, // 0x02
    SM_CYHSCROLL = 3, // 0x03
    SM_CYCAPTION = 4, // 0x04
    SM_CXBORDER = 5, // 0x05
    SM_CYBORDER = 6, // 0x06
    SM_CXDLGFRAME = 7, // 0x07
    SM_CXFIXEDFRAME = 7, // 0x07
    SM_CYDLGFRAME = 8, // 0x08
    SM_CYFIXEDFRAME = 8, // 0x08
    SM_CYVTHUMB = 9, // 0x09
    SM_CXHTHUMB = 10, // 0x0A
    SM_CXICON = 11, // 0x0B
    SM_CYICON = 12, // 0x0C
    SM_CXCURSOR = 13, // 0x0D
    SM_CYCURSOR = 14, // 0x0E
    SM_CYMENU = 15, // 0x0F
    SM_CXFULLSCREEN = 16, // 0x10
    SM_CYFULLSCREEN = 17, // 0x11
    SM_CYKANJIWINDOW = 18, // 0x12
    SM_MOUSEPRESENT = 19, // 0x13
    SM_CYVSCROLL = 20, // 0x14
    SM_CXHSCROLL = 21, // 0x15
    SM_DEBUG = 22, // 0x16
    SM_SWAPBUTTON = 23, // 0x17
    SM_CXMIN = 28, // 0x1C
    SM_CYMIN = 29, // 0x1D
    SM_CXSIZE = 30, // 0x1E
    SM_CYSIZE = 31, // 0x1F
    SM_CXSIZEFRAME = 32, // 0x20
    SM_CXFRAME = 32, // 0x20
    SM_CYSIZEFRAME = 33, // 0x21
    SM_CYFRAME = 33, // 0x21
    SM_CXMINTRACK = 34, // 0x22
    SM_CYMINTRACK = 35, // 0x23
    SM_CXDOUBLECLK = 36, // 0x24
    SM_CYDOUBLECLK = 37, // 0x25
    SM_CXICONSPACING = 38, // 0x26
    SM_CYICONSPACING = 39, // 0x27
    SM_MENUDROPALIGNMENT = 40, // 0x28
    SM_PENWINDOWS = 41, // 0x29
    SM_DBCSENABLED = 42, // 0x2A
    SM_CMOUSEBUTTONS = 43, // 0x2B
    SM_SECURE = 44, // 0x2C
    SM_CXEDGE = 45, // 0x2D
    SM_CYEDGE = 46, // 0x2E
    SM_CXMINSPACING = 47, // 0x2F
    SM_CYMINSPACING = 48, // 0x30
    SM_CXSMICON = 49, // 0x31
    SM_CYSMICON = 50, // 0x32
    SM_CYSMCAPTION = 51, // 0x33
    SM_CXSMSIZE = 52, // 0x34
    SM_CYSMSIZE = 53, // 0x35
    SM_CXMENUSIZE = 54, // 0x36
    SM_CYMENUSIZE = 55, // 0x37
    SM_ARRANGE = 56, // 0x38
    SM_CXMINIMIZED = 57, // 0x39
    SM_CYMINIMIZED = 58, // 0x3A
    SM_CXMAXTRACK = 59, // 0x3B
    SM_CYMAXTRACK = 60, // 0x3C
    SM_CXMAXIMIZED = 61, // 0x3D
    SM_CYMAXIMIZED = 62, // 0x3E
    SM_NETWORK = 63, // 0x3F
    SM_CLEANBOOT = 67, // 0x43
    SM_CXDRAG = 68, // 0x44
    SM_CYDRAG = 69, // 0x45
    SM_SHOWSOUNDS = 70, // 0x46
    SM_CXMENUCHECK = 71, // 0x47
    SM_CYMENUCHECK = 72, // 0x48
    SM_SLOWMACHINE = 73, // 0x49
    SM_MIDEASTENABLED = 74, // 0x4A
    SM_MOUSEWHEELPRESENT = 75, // 0x4B
    SM_XVIRTUALSCREEN = 76, // 0x4C
    SM_YVIRTUALSCREEN = 77, // 0x4D
    SM_CXVIRTUALSCREEN = 78, // 0x4E
    SM_CYVIRTUALSCREEN = 79, // 0x4F
    SM_CMONITORS = 80, // 0x50
    SM_SAMEDISPLAYFORMAT = 81, // 0x51
    SM_IMMENABLED = 82, // 0x52
    SM_CXFOCUSBORDER = 83, // 0x53
    SM_CYFOCUSBORDER = 84, // 0x54
    SM_TABLETPC = 86, // 0x56
    SM_MEDIACENTER = 87, // 0x57
    SM_STARTER = 88, // 0x58
    SM_SERVERR2 = 89, // 0x59
    SM_MOUSEHORIZONTALWHEELPRESENT = 91, // 0x5B
    SM_CXPADDEDBORDER = 92, // 0x5C
    SM_DIGITIZER = 94, // 0x5E
    SM_MAXIMUMTOUCHES = 95, // 0x5F

    SM_REMOTESESSION = 0x1000, // 0x1000
    SM_SHUTTINGDOWN = 0x2000, // 0x2000
    SM_REMOTECONTROL = 0x2001, // 0x2001


    SM_CONVERTIBLESLATEMODE = 0x2003,
    SM_SYSTEMDOCKED = 0x2004
}