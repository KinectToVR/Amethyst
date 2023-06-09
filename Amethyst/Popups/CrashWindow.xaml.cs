using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Web;
using Windows.ApplicationModel.Activation;
using Windows.Graphics;
using Windows.System;
using Amethyst.Classes;
using Amethyst.Utils;
using Microsoft.UI;
using Microsoft.UI.Windowing;
using Microsoft.UI.Xaml;
using Microsoft.Windows.AppLifecycle;
using WinRT.Interop;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Popups;

/// <summary>
///     An empty page that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class CrashWindow : Window
{
    public enum HandlerMode
    {
        CrashWatchdog,
        None,
        AlreadyRunning,
        RunningElevated,
        CrashMessage
    }

    [Flags]
    public enum SetWindowPosFlags : uint
    {
        SwpAsyncwindowpos = 0x4000,

        SwpDefererase = 0x2000,

        SwpDrawframe = 0x0020,

        SwpFramechanged = 0x0020,

        SwpHidewindow = 0x0080,

        SwpNoactivate = 0x0010,

        SwpNocopybits = 0x0100,

        SwpNomove = 0x0002,

        SwpNoownerzorder = 0x0200,

        SwpNoredraw = 0x0008,

        SwpNoreposition = 0x0200,

        SwpNosendchanging = 0x0400,

        SwpNosize = 0x0001,

        SwpNozorder = 0x0004,

        SwpShowwindow = 0x0040
    }


    public enum SpecialWindowHandles
    {
        HwndTop = 0,
        HwndBottom = 1,
        HwndTopmost = -1,
        HwndNotopmost = -2
    }

    public enum SystemMetric
    {
        SmCxscreen = 0, // 0x00
        SmCyscreen = 1, // 0x01
        SmCxvscroll = 2, // 0x02
        SmCyhscroll = 3, // 0x03
        SmCycaption = 4, // 0x04
        SmCxborder = 5, // 0x05
        SmCyborder = 6, // 0x06
        SmCxdlgframe = 7, // 0x07
        SmCxfixedframe = 7, // 0x07
        SmCydlgframe = 8, // 0x08
        SmCyfixedframe = 8, // 0x08
        SmCyvthumb = 9, // 0x09
        SmCxhthumb = 10, // 0x0A
        SmCxicon = 11, // 0x0B
        SmCyicon = 12, // 0x0C
        SmCxcursor = 13, // 0x0D
        SmCycursor = 14, // 0x0E
        SmCymenu = 15, // 0x0F
        SmCxfullscreen = 16, // 0x10
        SmCyfullscreen = 17, // 0x11
        SmCykanjiwindow = 18, // 0x12
        SmMousepresent = 19, // 0x13
        SmCyvscroll = 20, // 0x14
        SmCxhscroll = 21, // 0x15
        SmDebug = 22, // 0x16
        SmSwapbutton = 23, // 0x17
        SmCxmin = 28, // 0x1C
        SmCymin = 29, // 0x1D
        SmCxsize = 30, // 0x1E
        SmCysize = 31, // 0x1F
        SmCxsizeframe = 32, // 0x20
        SmCxframe = 32, // 0x20
        SmCysizeframe = 33, // 0x21
        SmCyframe = 33, // 0x21
        SmCxmintrack = 34, // 0x22
        SmCymintrack = 35, // 0x23
        SmCxdoubleclk = 36, // 0x24
        SmCydoubleclk = 37, // 0x25
        SmCxiconspacing = 38, // 0x26
        SmCyiconspacing = 39, // 0x27
        SmMenudropalignment = 40, // 0x28
        SmPenwindows = 41, // 0x29
        SmDbcsenabled = 42, // 0x2A
        SmCmousebuttons = 43, // 0x2B
        SmSecure = 44, // 0x2C
        SmCxedge = 45, // 0x2D
        SmCyedge = 46, // 0x2E
        SmCxminspacing = 47, // 0x2F
        SmCyminspacing = 48, // 0x30
        SmCxsmicon = 49, // 0x31
        SmCysmicon = 50, // 0x32
        SmCysmcaption = 51, // 0x33
        SmCxsmsize = 52, // 0x34
        SmCysmsize = 53, // 0x35
        SmCxmenusize = 54, // 0x36
        SmCymenusize = 55, // 0x37
        SmArrange = 56, // 0x38
        SmCxminimized = 57, // 0x39
        SmCyminimized = 58, // 0x3A
        SmCxmaxtrack = 59, // 0x3B
        SmCymaxtrack = 60, // 0x3C
        SmCxmaximized = 61, // 0x3D
        SmCymaximized = 62, // 0x3E
        SmNetwork = 63, // 0x3F
        SmCleanboot = 67, // 0x43
        SmCxdrag = 68, // 0x44
        SmCydrag = 69, // 0x45
        SmShowsounds = 70, // 0x46
        SmCxmenucheck = 71, // 0x47
        SmCymenucheck = 72, // 0x48
        SmSlowmachine = 73, // 0x49
        SmMideastenabled = 74, // 0x4A
        SmMousewheelpresent = 75, // 0x4B
        SmXvirtualscreen = 76, // 0x4C
        SmYvirtualscreen = 77, // 0x4D
        SmCxvirtualscreen = 78, // 0x4E
        SmCyvirtualscreen = 79, // 0x4F
        SmCmonitors = 80, // 0x50
        SmSamedisplayformat = 81, // 0x51
        SmImmenabled = 82, // 0x52
        SmCxfocusborder = 83, // 0x53
        SmCyfocusborder = 84, // 0x54
        SmTabletpc = 86, // 0x56
        SmMediacenter = 87, // 0x57
        SmStarter = 88, // 0x58
        SmServerr2 = 89, // 0x59
        SmMousehorizontalwheelpresent = 91, // 0x5B
        SmCxpaddedborder = 92, // 0x5C
        SmDigitizer = 94, // 0x5E
        SmMaximumtouches = 95, // 0x5F

        SmRemotesession = 0x1000, // 0x1000
        SmShuttingdown = 0x2000, // 0x2000
        SmRemotecontrol = 0x2001, // 0x2001


        SmConvertibleslatemode = 0x2003,
        SmSystemdocked = 0x2004
    }

    public CrashWindow()
    {
        InitializeComponent();

        // Load theme config
        Logger.Info("Loading theme configurations...");
        RGrid.RequestedTheme = AppData.Settings.AppTheme switch
        {
            2 => ElementTheme.Light,
            1 => ElementTheme.Dark,
            _ => ElementTheme.Default
        };

        // Prepare placeholder strings (recovery mode)
        Logger.Info("Setting up base application interface strings...");
        string handlerTitle = Interfacing.LocalizedJsonString("/CrashHandler/Title/Recovery"),
            handlerContent = Interfacing.LocalizedJsonString("/CrashHandler/Content/Recovery"),
            primaryButtonText = Interfacing.LocalizedJsonString("/CrashHandler/PrimaryButton/Recovery"),
            secondaryButtonText = Interfacing.LocalizedJsonString("/CrashHandler/SecondaryButton/Recovery"),
            logFileLocation = "0";

        // Prepare placeholder callbacks (recovery mode)
        RoutedEventHandler primaryButtonHandler = Action_OpenCollective,
            secondaryButtonHandler = Action_Close;

        // Check if there's any launch arguments
        Logger.Info("Validating activation arguments...");
        var activationUri = (AppInstance.GetCurrent().GetActivatedEventArgs().Data as
            ProtocolActivatedEventArgs)?.Uri;

        Logger.Info($"Activated via uri of: {activationUri}");
        if (activationUri is not null && activationUri.Segments.Length > 0)
            CrashHandlerMode = activationUri.Segments.First() switch
            {
                "crash-already-running" => HandlerMode.AlreadyRunning,
                "crash-vr-elevated" => HandlerMode.RunningElevated,
                "crash-message" => HandlerMode.CrashMessage,
                "crash-watchdog" => HandlerMode.CrashWatchdog,
                _ => CrashHandlerMode // In any other case
            };

        // If we're OK then don't use recovery mode
        //    and wait + optionally parse the crash
        switch (CrashHandlerMode)
        {
            case HandlerMode.CrashWatchdog:
            {
                // Wait for the app to exit and grab the exit code
                try
                {
                    Logger.Info("Running in watchdog mode!");
                    var queryDictionary = HttpUtility
                        .ParseQueryString(activationUri!.Query.TrimStart('?'));

                    var pid = int.Parse(queryDictionary["pid"]!);
                    var process = Process.GetProcessById(pid);

                    logFileLocation = queryDictionary["log"];
                    var crashFileLocation = queryDictionary["crash"];

                    Logger.Info($"Waiting for {pid} to exit...");
                    process.EnableRaisingEvents = true;
                    process.Exited += ProcessEnded;

                    // Display the process statistics until
                    // the user closes the program.
                    do
                    {
                        // Refresh the current process property values
                        if (!process.HasExited) process.Refresh();
                    } while (!process.WaitForExit(3000));

                    try
                    {
                        // Overwrite the _latest.log log file
                        if (!string.IsNullOrEmpty(logFileLocation))
                            File.Copy(logFileLocation, Path.Combine(
                                Directory.GetParent(logFileLocation)?.ToString() ?? string.Empty,
                                "_latest.log"), true);
                    }
                    catch (Exception e)
                    {
                        Logger.Error(e);
                    }

                    // Handle normal exit
                    if (ProcessExitCode == 0)
                    {
                        Environment.Exit(0); // Exit
                        return; // Don't care anymore
                    }

                    // Don't handle post-exit crashes
                    if (!File.Exists(crashFileLocation))
                    {
                        Environment.Exit(0); // Exit
                        return; // Don't care anymore
                    }

                    // Parse the exit code into strings
                    handlerTitle = Interfacing.LocalizedJsonString("/CrashHandler/Title/Crash");
                    primaryButtonText = Interfacing.LocalizedJsonString("/CrashHandler/PrimaryButton/Crash/Default");
                    switch (ProcessExitCode)
                    {
                        case -13:
                        {
                            // Panic exit
                            primaryButtonHandler = Action_ResetConfig;
                            handlerContent = Interfacing.LocalizedJsonString("/CrashHandler/Content/Crash/Panic");
                            primaryButtonText =
                                Interfacing.LocalizedJsonString("/CrashHandler/PrimaryButton/Crash/Panic");
                        }
                            break;
                        case -12:
                        {
                            // No devices
                            primaryButtonHandler = Action_VRDocs;
                            handlerContent = Interfacing.LocalizedJsonString("/CrashHandler/Content/Crash/NoDevices");
                        }
                            break;
                        case -11:
                        {
                            // OpenVR error
                            primaryButtonHandler = Action_DeviceDocs;
                            handlerContent = Interfacing.LocalizedJsonString("/CrashHandler/Content/Crash/OpenVR");
                        }
                            break;
                        case -1073740791:
                        {
                            // OpenVR Init timeout error
                            primaryButtonHandler = Action_DeviceDocs;
                            handlerContent = Interfacing.LocalizedJsonString("/CrashHandler/Content/Crash/OpenVR");
                        }
                            break;
                        case 0:
                        {
                            Environment.Exit(0); // Exit
                            return; // Don't care anymore
                        }
                        //case -1073741189:
                        //{
                        //    // owoTrack broke our dispatcher,
                        //    // just don't give a shit
                        //    Close();
                        //}
                        //    break;
                        case -532265403:
                        {
                            // MS no package identity something,
                            // just don't give a shit
                            Environment.Exit(0); // Exit
                            return; // Don't care anymore
                        }
                        case -1:
                        {
                            // Force closed from the debugger
                            // (Or by the crash handler)
                            Environment.Exit(0); // Exit
                            return; // Don't care anymore
                        }
                        case 1:
                        {
                            // Killed by system task manger
                            Environment.Exit(0); // Exit
                            return; // Don't care anymore
                        }
                        default:
                        {
                            // Unknown
                            primaryButtonHandler = Action_Discord;
                            handlerContent = Interfacing.LocalizedJsonString("/CrashHandler/Content/Crash/Unknown");
                            primaryButtonText =
                                Interfacing.LocalizedJsonString("/CrashHandler/PrimaryButton/Crash/Unknown");
                        }
                            break;
                    }
                }
                catch (Exception)
                {
                    Environment.Exit(0); // Exit
                    return; // Don't care anymore
                }

                break;
            }

            case HandlerMode.AlreadyRunning:
            {
                // Parse the strings
                primaryButtonHandler = Action_ForceQuit;

                handlerTitle = Interfacing.LocalizedJsonString("/CrashHandler/Title/AlreadyRunning");
                handlerContent = Interfacing.LocalizedJsonString("/CrashHandler/Content/AlreadyRunning");
                primaryButtonText = Interfacing.LocalizedJsonString("/CrashHandler/PrimaryButton/AlreadyRunning");
                break;
            }

            case HandlerMode.RunningElevated:
            {
                // Parse the strings
                primaryButtonHandler = Action_ForceQuit;

                handlerTitle = Interfacing.LocalizedJsonString("/CrashHandler/Title/Elevated");
                handlerContent = Interfacing.LocalizedJsonString("/CrashHandler/Content/Crash/Elevated");
                primaryButtonText = Interfacing.LocalizedJsonString("/CrashHandler/PrimaryButton/Crash/Unknown");
                break;
            }

            case HandlerMode.CrashMessage:
            {
                // Parse the strings
                primaryButtonHandler = Action_ForceQuit;

                handlerTitle = Interfacing.LocalizedJsonString("/CrashHandler/Title/Crash");
                primaryButtonText = Interfacing.LocalizedJsonString("/CrashHandler/PrimaryButton/Crash/Default");
                handlerContent = HttpUtility.UrlDecode(activationUri?.Fragment.TrimStart('#'));
                break;
            }

            case HandlerMode.None:
            default:
            {
                // Defaults
                break;
            }
        }

        // Prepare window size consts
        const int width = 400,
            height = 295;

        try
        {
            // Set window title, drag-space and size
            Title = handlerTitle;
        }
        catch (Exception)
        {
            // ignored
        }

        SetWindowSize(WindowNative
            .GetWindowHandle(this), width, height);

        var appWindow = AppWindow.GetFromWindowId(
            Win32Interop.GetWindowIdFromWindow(
                WindowNative.GetWindowHandle(this)));

        // Fix no icon in titlebar/task view
        appWindow.SetIcon(Path.Combine(
            Directory.GetParent(Assembly.GetExecutingAssembly().Location)?.ToString() ?? string.Empty,
            "Assets", "crashhandler.ico"));

        // Custom titlebar
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
            GetSystemMetrics(SystemMetric.SmCxscreen) / 2 - width / 2,
            GetSystemMetrics(SystemMetric.SmCyscreen) / 2 - height / 2,
            width, height, 0x0040); // SWP_SHOWWINDOW

        // (Second try ...but first)
        Activate();

        // Another one
        AppWindow.Show(true);

        // And show it (forcibly)
        SetForegroundWindow(WindowNative
            .GetWindowHandle(this));

        // Construct the dialog
        DialogView = new CrashDialog
        (
            handlerTitle, handlerContent, primaryButtonText,
            secondaryButtonText, primaryButtonHandler, secondaryButtonHandler,
            CrashHandlerMode is not HandlerMode.None, logFileLocation
        );

        // And push it into the main grid
        RGrid.Children.Add(DialogView);
    }

    private static HandlerMode CrashHandlerMode { get; set; } = HandlerMode.None;
    private static int ProcessExitCode { get; set; }
    private static CrashDialog DialogView { get; set; }

    private async void Action_OpenCollective(object sender, RoutedEventArgs e)
    {
        await Launcher.LaunchUriAsync(new Uri("https://opencollective.com/k2vr/"));
    }

    private void Action_ResetConfig(object sender, RoutedEventArgs e)
    {
        try
        {
            File.Delete(Interfacing.GetAppDataFilePath("AmethystSettings.json"));
        }
        catch (Exception)
        {
            //ignored
        }
    }

    private async void Action_VRDocs(object sender, RoutedEventArgs e)
    {
        await Launcher.LaunchUriAsync(new Uri($"https://docs.k2vr.tech/{Interfacing.DocsLanguageCode}/"));
    }

    private async void Action_DeviceDocs(object sender, RoutedEventArgs e)
    {
        await Launcher.LaunchUriAsync(new Uri($"https://docs.k2vr.tech/{Interfacing.DocsLanguageCode}/"));
    }

    private async void Action_Discord(object sender, RoutedEventArgs e)
    {
        await Launcher.LaunchUriAsync(new Uri("https://discord.gg/YBQCRDG"));
    }

    private void Action_ForceQuit(object sender, RoutedEventArgs e)
    {
        foreach (var process in Process.GetProcessesByName("Amethyst"))
            process.Kill();
    }

    private void Action_Close(object sender, RoutedEventArgs e)
    {
        Close(); // Prior
        Environment.Exit(0);
    }

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern UIntPtr OpenProcess(uint dwDesiredAccess, bool bInheritHandle, uint dwProcessId);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool CloseHandle(UIntPtr hObject);

    [DllImport("User32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    public static extern uint GetDpiForWindow(IntPtr hwnd);

    [DllImport("user32.dll", EntryPoint = "SetWindowPos")]
    public static extern IntPtr SetWindowPos(IntPtr hWnd, int hWndInsertAfter,
        int x, int y, int cx, int cy, int wFlags);

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

        SetWindowPos(hwnd, (int)SpecialWindowHandles.HwndTop,
            0, 0, width, height,
            (int)SetWindowPosFlags.SwpNomove);
    }

    private void ProcessEnded(object sender, EventArgs e)
    {
        var process = sender as Process;
        if (process != null) ProcessExitCode = process.ExitCode;
    }
}