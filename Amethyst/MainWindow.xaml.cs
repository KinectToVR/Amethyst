// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Reflection;
using System.Threading;
using Windows.Graphics;
using Amethyst.Classes;
using Amethyst.Utils;
using Microsoft.UI;
using Microsoft.UI.Windowing;
using Microsoft.UI.Xaml;
using Microsoft.Windows.AppNotifications;
using WinRT;
using WinRT.Interop;
using Microsoft.UI.Xaml.Controls;
using Windows.UI.ApplicationSettings;
using System.Threading.Tasks;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Media.Animation;
using static System.Runtime.InteropServices.JavaScript.JSType;
using System.Runtime.InteropServices;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst;

/// <summary>
///     An empty window that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class MainWindow : Window
{
    private bool _mainPageLoadedOnce = false, _mainPageInitFinished = false;
    private WindowsSystemDispatcherQueueHelper m_wsdqHelper; // See separate sample below for implementation
    private Microsoft.UI.Composition.SystemBackdrops.MicaController m_micaController;
    private Microsoft.UI.Composition.SystemBackdrops.SystemBackdropConfiguration m_configurationSource;

    public MainWindow()
    {
        InitializeComponent();
        TrySetMicaBackdrop();

        // Cache needed UI elements
        Shared.TeachingTips.Main.InitializerTeachingTip = InitializerTeachingTip;
        Shared.TeachingTips.Main.ReloadTeachingTip = ReloadTeachingTip;

        Shared.Main.MainNavigationView = NavView;
        Shared.Main.AppTitleLabel = AppTitleLabel;
        Shared.Main.FlyoutHeader = FlyoutHeader;
        Shared.Main.FlyoutFooter = FlyoutFooter;
        Shared.Main.FlyoutContent = FlyoutContent;

        Shared.Main.InterfaceBlockerGrid = InterfaceBlockerGrid;
        Shared.Main.NavigationBlockerGrid = NavigationBlockerGrid;

        Shared.Main.MainContentFrame = ContentFrame;
        Shared.Main.UpdateIconDot = UpdateIconDot;
        Shared.Main.UpdateFlyout = UpdateFlyout;

        Shared.Main.InstallNowButton = InstallNowButton;
        Shared.Main.InstallLaterButton = InstallLaterButton;

        Shared.Main.GeneralItem = GeneralItem;
        Shared.Main.SettingsItem = SettingsItem;
        Shared.Main.DevicesItem = DevicesItem;
        Shared.Main.InfoItem = InfoItem;
        Shared.Main.ConsoleItem = ConsoleItem;
        Shared.Main.HelpButton = HelpButton;

        Shared.Main.NavigationItems.NavViewGeneralButtonIcon = NavViewGeneralButtonIcon;
        Shared.Main.NavigationItems.NavViewSettingsButtonIcon = NavViewSettingsButtonIcon;
        Shared.Main.NavigationItems.NavViewDevicesButtonIcon = NavViewDevicesButtonIcon;
        Shared.Main.NavigationItems.NavViewInfoButtonIcon = NavViewInfoButtonIcon;
        Shared.Main.NavigationItems.NavViewOkashiButtonIcon = NavViewOkashiButtonIcon;

        Shared.Main.NavigationItems.NavViewGeneralButtonLabel = NavViewGeneralButtonLabel;
        Shared.Main.NavigationItems.NavViewSettingsButtonLabel = NavViewSettingsButtonLabel;
        Shared.Main.NavigationItems.NavViewDevicesButtonLabel = NavViewDevicesButtonLabel;
        Shared.Main.NavigationItems.NavViewInfoButtonLabel = NavViewInfoButtonLabel;
        Shared.Main.NavigationItems.NavViewOkashiButtonLabel = NavViewOkashiButtonLabel;

        // Set up
        Title = "Amethyst";

        Logger.Info("Making the app window available for children views... (Window Handle)");
        Shared.Main.AppWindowId = WindowNative.GetWindowHandle(this);

        Logger.Info("Making the app window available for children views... (XAML UI Window)");
        Shared.Main.Window = this.As<Window>();

        Logger.Info("Making the app window available for children views... (Shared App Window)");
        Shared.Main.AppWindow = AppWindow.GetFromWindowId(Win32Interop.GetWindowIdFromWindow(
            WindowNative.GetWindowHandle(this)));

        // Set titlebar/taskview icon
        Logger.Info("Setting the App Window icon...");
        Shared.Main.AppWindow.SetIcon(Path.Combine(
            Interfacing.GetProgramLocation().DirectoryName, "Assets", "ktvr.ico"));

        Logger.Info("Extending the window titlebar...");
        if (AppWindowTitleBar.IsCustomizationSupported())
        {
            // Chad Windows 11
            Shared.Main.AppWindow.TitleBar.ExtendsContentIntoTitleBar = true;
            Shared.Main.AppWindow.TitleBar.SetDragRectangles(new RectInt32[]
            {
                new(0, 0, 10000000, 30)
            });

            Shared.Main.AppWindow.TitleBar.ButtonBackgroundColor = Colors.Transparent;
            Shared.Main.AppWindow.TitleBar.ButtonInactiveBackgroundColor = Colors.Transparent;
            Shared.Main.AppWindow.TitleBar.ButtonHoverBackgroundColor =
                Shared.Main.AppWindow.TitleBar.ButtonPressedBackgroundColor;
        }
        else
            // Poor ass Windows 10
        {
            ExtendsContentIntoTitleBar = true;
            SetTitleBar(DragElement);
        }

        Logger.Info("Making the app dispatcher available for children views...");
        Shared.Main.DispatcherQueue = DispatcherQueue;

        Logger.Info("Registering for NotificationInvoked WinRT event...");

        // To ensure all Notification handling happens in this process instance, register for
        // NotificationInvoked before calling Register(). Without this a new process will
        // be launched to handle the notification.
        AppNotificationManager.Default.NotificationInvoked +=
            (_, notificationActivatedEventArgs) =>
            {
                Interfacing.ProcessToastArguments(notificationActivatedEventArgs);
            };

        Logger.Info("Creating the default notification manager...");
        Shared.Main.NotificationManager = AppNotificationManager.Default;

        Logger.Info("Registering the notification manager...");
        Shared.Main.NotificationManager.Register();

        Logger.Info("Creating and registering the default resource manager...");
        Shared.Main.ResourceManager = new Microsoft.Windows.ApplicationModel
            .Resources.ResourceManager("resources.pri");

        Logger.Info("Creating and registering the default resource context...");
        Shared.Main.ResourceContext = Shared.Main.ResourceManager.CreateResourceContext();

        Logger.Info("Pushing control pages the global collection...");
        Shared.Main.Pages = new List<(string Tag, Type Page)>
        {
            ("general", typeof(Pages.General)),
            ("settings", typeof(Pages.Settings)),
            ("devices", typeof(Pages.Devices)),
            ("info", typeof(Pages.Info))
        };

        Logger.Info("Registering a detached binary semaphore " +
                    $"reload handler for '{GetType().FullName}'...");

        Task.Run(Task() =>
        {
            while (true)
            {
                // Wait for a reload signal (blocking)
                Shared.Semaphores.ReloadMainWindowSemaphore.WaitOne();

                // Reload & restart the waiting loop
                if (_mainPageLoadedOnce)
                    Shared.Main.DispatcherQueue.TryEnqueue(async () => { await MainGrid_LoadedHandler(); });

                // Rebuild devices' settings
                // (Trick the device into rebuilding its interface)
                TrackingDevices.TrackingDevicesVector.Values.ToList()
                    .ForEach(device => device.OnLoad());

                Task.Delay(100); // Sleep a bit
            }
        });

        Logger.Info("Registering a named mutex for com_k2vr_amethyst...");
        try
        {
            Shared.Main.ApplicationMultiInstanceMutex = new Mutex(
                true, "com_k2vr_amethyst", out var needToCreateNew);

            if (!needToCreateNew)
            {
                Logger.Error("Startup failed! The app is already running.");

                if (File.Exists(Path.Combine(Interfacing.GetProgramLocation().DirectoryName,
                        "K2CrashHandler", "K2CrashHandler.exe")))
                    Process.Start(Path.Combine(Interfacing.GetProgramLocation().DirectoryName,
                        "K2CrashHandler", "K2CrashHandler.exe"));
                else
                    Logger.Warn("Crash handler exe (./K2CrashHandler/K2CrashHandler.exe) not found!");

                Task.Delay(3000);
                Environment.Exit(0); // Exit peacefully
            }
        }
        catch (Exception ex)
        {
            Logger.Error("Startup failed! Multi-instance lock mutex creation error.");

            if (File.Exists(Path.Combine(Interfacing.GetProgramLocation().DirectoryName,
                    "K2CrashHandler", "K2CrashHandler.exe")))
                Process.Start(Path.Combine(Interfacing.GetProgramLocation().DirectoryName,
                    "K2CrashHandler", "K2CrashHandler.exe"));
            else
                Logger.Warn("Crash handler exe (./K2CrashHandler/K2CrashHandler.exe) not found!");

            Task.Delay(3000);
            Environment.Exit(0); // Exit peacefully
        }

        // Priority: Launch the crash handler
        Logger.Info("Starting the crash handler passing the app PID...");

        if (File.Exists(Path.Combine(Interfacing.GetProgramLocation().DirectoryName,
                "K2CrashHandler", "K2CrashHandler.exe")))
            Process.Start(Path.Combine(Interfacing.GetProgramLocation().DirectoryName,
                "K2CrashHandler", "K2CrashHandler.exe"), $"{Environment.ProcessId} \"{Logger.LogFilePath}\"");
        else
            Logger.Warn("Crash handler exe (./K2CrashHandler/K2CrashHandler.exe) not found!");

        // Priority: Connect to OpenVR
        if (!Interfacing.OpenVRStartup().Result)
        {
            Logger.Error("Could not connect to OpenVR! The app will be shut down.");
            Interfacing.Fail(-11); // OpenVR is critical, so exit
        }

        // Priority: Set up Amethyst as a vr app
        Logger.Info("Installing the vr application manifest...");
        Interfacing.InstallVrApplicationManifest();

        // Priority: Set up VR Input Actions
        if (!Interfacing.EvrActionsStartup())
            Logger.Error("Could not set up VR Input Actions! The app will lack some functionality.");

        // Priority: Set up the API & Server
        Interfacing.K2ServerDriverSetup();

        // Start the main loop
        // TODO std::thread(k2app::main::K2MainLoop).detach();

        // Disable internal sounds
        ElementSoundPlayer.State = ElementSoundPlayerState.Off;

        // TODO load devices, config and compose

        // Notify of the setup end
        _mainPageInitFinished = true;
    }

    private async void MainGrid_Loaded(object sender, RoutedEventArgs e)
    {
        // Load theme config
        Shared.Main.MainNavigationView.XamlRoot.Content.As<Grid>().RequestedTheme =
            Interfacing.AppSettings.AppTheme switch
            {
                2 => ElementTheme.Light,
                1 => ElementTheme.Dark,
                _ => ElementTheme.Default
            };

        // Execute the handler
        await MainGrid_LoadedHandler();

        // Register a theme watchdog
        NavView.XamlRoot.Content.As<Grid>().ActualThemeChanged += MainWindow_ActualThemeChanged;
    }

    [DllImport("user32.dll")]
    public static extern int SendMessage(IntPtr hWnd, int wMsg, IntPtr wParam, IntPtr lParam);

    private void MainWindow_ActualThemeChanged(FrameworkElement sender, object args)
    {
        Interfacing.ActualTheme = NavView.ActualTheme;

        Shared.Main.AttentionBrush =
            Interfacing.ActualTheme == ElementTheme.Dark
                ? Application.Current.Resources["AttentionBrush_Dark"].As<SolidColorBrush>()
                : Application.Current.Resources["AttentionBrush_Light"].As<SolidColorBrush>();

        Shared.Main.NeutralBrush =
            Interfacing.ActualTheme == ElementTheme.Dark
                ? Application.Current.Resources["NeutralBrush_Dark"].As<SolidColorBrush>()
                : Application.Current.Resources["NeutralBrush_Light"].As<SolidColorBrush>();

        if (Application.Current.Resources.ContainsKey("WindowCaptionForeground"))
            Application.Current.Resources.Remove("WindowCaptionForeground");

        Application.Current.Resources.Add("WindowCaptionForeground",
            Interfacing.ActualTheme == ElementTheme.Dark ? Colors.White : Colors.Black);

        // Trigger a titlebar repaint
        if (Shared.Main.AppWindowId == Interfacing.GetActiveWindow())
        {
            SendMessage(Shared.Main.AppWindowId, 0x0006, new IntPtr(0), new IntPtr(0));
            SendMessage(Shared.Main.AppWindowId, 0x0006, new IntPtr(1), new IntPtr(0));
        }
        else
        {
            SendMessage(Shared.Main.AppWindowId, 0x0006, new IntPtr(1), new IntPtr(0));
            SendMessage(Shared.Main.AppWindowId, 0x0006, new IntPtr(0), new IntPtr(0));
        }

        // Overwrite titlebar colors
        Shared.Main.AppWindow.TitleBar.ButtonForegroundColor =
            Interfacing.ActualTheme == ElementTheme.Dark
                ? Colors.White
                : Colors.Black;

        Shared.Main.AppWindow.TitleBar.ButtonBackgroundColor = Colors.Transparent;
        Shared.Main.AppWindow.TitleBar.ButtonInactiveBackgroundColor = Colors.Transparent;
        Shared.Main.AppWindow.TitleBar.ButtonHoverBackgroundColor =
            Shared.Main.AppWindow.TitleBar.ButtonPressedBackgroundColor;

        // Request page reloads
        Shared.Semaphores.ReloadMainWindowSemaphore.Release();
        Shared.Semaphores.ReloadGeneralPageSemaphore.Release();
        Shared.Semaphores.ReloadSettingsPageSemaphore.Release();
        Shared.Semaphores.ReloadDevicesPageSemaphore.Release();
        Shared.Semaphores.ReloadInfoPageSemaphore.Release();

        // Mark as loaded
        _mainPageLoadedOnce = true;
    }

    private async Task MainGrid_LoadedHandler()
    {
        NavViewGeneralButtonLabel.Text = Interfacing.LocalizedJsonString("/SharedStrings/Buttons/General");
        NavViewSettingsButtonLabel.Text = Interfacing.LocalizedJsonString("/SharedStrings/Buttons/Settings");
        NavViewDevicesButtonLabel.Text = Interfacing.LocalizedJsonString("/SharedStrings/Buttons/Devices");
        NavViewInfoButtonLabel.Text = Interfacing.LocalizedJsonString("/SharedStrings/Buttons/Info");

        UpdateIconText.Text = Interfacing.LocalizedJsonString("/SharedStrings/Buttons/Updates/Header");
        HelpIconText.Text = Interfacing.LocalizedJsonString("/SharedStrings/Buttons/Help/Header");

        InstallLaterButton.Content = Interfacing.LocalizedJsonString("/SharedStrings/Buttons/Updates/Skip");
        InstallNowButton.Content = Interfacing.LocalizedJsonString("/SharedStrings/Buttons/Updates/Install");

        HelpFlyoutDiscordButton.Text = Interfacing.LocalizedJsonString("/SharedStrings/Buttons/Help/Discord");
        HelpFlyoutDevButton.Text = Interfacing.LocalizedJsonString("/SharedStrings/Buttons/Help/Developers");
        HelpFlyoutLicensesButton.Text = Interfacing.LocalizedJsonString("/SharedStrings/Buttons/Help/Licenses");

        ShutdownTeachingTip.Title = Interfacing.LocalizedJsonString("/NUX/Tip0/Title");
        ShutdownTeachingTip.Subtitle = Interfacing.LocalizedJsonString("/NUX/Tip0/Content");

        ReloadTeachingTip.Title = Interfacing.LocalizedJsonString("/DevicesPage/Devices/Reload/Title");
        ReloadTeachingTip.Subtitle = Interfacing.LocalizedJsonString("/DevicesPage/Devices/Reload/Content");
        ReloadTeachingTip.CloseButtonContent = Interfacing.LocalizedJsonString("/DevicesPage/Devices/Reload/Restart");

        InitializerTeachingTip.Title = Interfacing.LocalizedJsonString("/NUX/Tip1/Title");
        InitializerTeachingTip.Subtitle = Interfacing.LocalizedJsonString("/NUX/Tip1/Content");
        InitializerTeachingTip.CloseButtonContent = Interfacing.LocalizedJsonString("/NUX/Next");
        InitializerTeachingTip.ActionButtonContent = Interfacing.LocalizedJsonString("/NUX/Skip");

        UpdatePendingFlyoutHeader.Text = Interfacing.LocalizedJsonString("/SharedStrings/Updates/Headers/Downloading");
        UpdatePendingFlyoutStatusLabel.Text = Interfacing.LocalizedJsonString("/SharedStrings/Updates/Headers/Status");

        if (Interfacing.CurrentPageClass == "Amethyst.GeneralPage")
        {
            Shared.Main.NavigationItems.NavViewGeneralButtonIcon.Glyph = "\uEA8A";
            Shared.Main.NavigationItems.NavViewGeneralButtonIcon.Foreground = Shared.Main.AttentionBrush;

            Shared.Main.NavigationItems.NavViewGeneralButtonLabel.Opacity = 0.0;
            Shared.Main.NavigationItems.NavViewGeneralButtonIcon.Translation = Vector3.Zero;
        }
        else
        {
            Shared.Main.NavigationItems.NavViewGeneralButtonIcon.Translation = new Vector3(0, -8, 0);
            Shared.Main.NavigationItems.NavViewGeneralButtonLabel.Opacity = 1.0;

            Shared.Main.NavigationItems.NavViewGeneralButtonIcon.Foreground = Shared.Main.NeutralBrush;
            Shared.Main.NavigationItems.NavViewGeneralButtonIcon.Glyph = "\uE80F";
        }

        if (Interfacing.CurrentPageClass == "Amethyst.SettingsPage")
        {
            Shared.Main.NavigationItems.NavViewSettingsButtonIcon.Glyph = "\uF8B0";
            Shared.Main.NavigationItems.NavViewSettingsButtonIcon.Foreground = Shared.Main.AttentionBrush;

            Shared.Main.NavigationItems.NavViewSettingsButtonLabel.Opacity = 0.0;
            Shared.Main.NavigationItems.NavViewSettingsButtonIcon.Translation = Vector3.Zero;
        }
        else
        {
            Shared.Main.NavigationItems.NavViewSettingsButtonIcon.Translation = new Vector3(0, -8, 0);
            Shared.Main.NavigationItems.NavViewSettingsButtonLabel.Opacity = 1.0;

            Shared.Main.NavigationItems.NavViewSettingsButtonIcon.Foreground = Shared.Main.NeutralBrush;
            Shared.Main.NavigationItems.NavViewSettingsButtonIcon.Glyph = "\uE713";
        }

        if (Interfacing.CurrentPageClass == "Amethyst.DevicesPage")
        {
            Shared.Main.NavigationItems.NavViewDevicesButtonLabel.Opacity = 0.0;
            Shared.Main.NavigationItems.NavViewDevicesButtonIcon.Translation = Vector3.Zero;

            Shared.Main.NavigationItems.NavViewDevicesButtonIcon.Foreground = Shared.Main.AttentionBrush;

            Shared.Main.NavigationItems.NavViewDevicesButtonIcon.Glyph = "\uEBD2";
            Shared.Main.NavigationItems.NavViewDevicesButtonIcon.FontSize = 23;
        }
        else
        {
            Shared.Main.NavigationItems.NavViewDevicesButtonIcon.Translation = new Vector3(0, -8, 0);
            Shared.Main.NavigationItems.NavViewDevicesButtonLabel.Opacity = 1.0;

            Shared.Main.NavigationItems.NavViewDevicesButtonIcon.Foreground = Shared.Main.NeutralBrush;

            Shared.Main.NavigationItems.NavViewDevicesButtonIcon.Glyph = "\uF158";
            Shared.Main.NavigationItems.NavViewDevicesButtonIcon.FontSize = 20;
        }

        if (Interfacing.CurrentPageClass == "Amethyst.InfoPage")
        {
            Shared.Main.NavigationItems.NavViewInfoButtonIcon.Glyph = "\uF167";
            Shared.Main.NavigationItems.NavViewInfoButtonIcon.Foreground = Shared.Main.AttentionBrush;

            Shared.Main.NavigationItems.NavViewInfoButtonLabel.Opacity = 0.0;
            Shared.Main.NavigationItems.NavViewInfoButtonIcon.Translation = Vector3.Zero;
        }
        else
        {
            Shared.Main.NavigationItems.NavViewInfoButtonIcon.Translation = new Vector3(0, -8, 0);
            Shared.Main.NavigationItems.NavViewInfoButtonLabel.Opacity = 1.0;

            Shared.Main.NavigationItems.NavViewInfoButtonIcon.Foreground = Shared.Main.NeutralBrush;
            Shared.Main.NavigationItems.NavViewInfoButtonIcon.Glyph = "\uE946";
        }

        UpdateIcon.Foreground = Interfacing.CheckingUpdatesNow ? Shared.Main.AttentionBrush : Shared.Main.NeutralBrush;
        var oppositeTheme = Interfacing.ActualTheme == ElementTheme.Dark ? ElementTheme.Light : ElementTheme.Dark;

        await Task.Delay(30);
        HelpButton.RequestedTheme = oppositeTheme;
        NavViewGeneralButtonLabel.RequestedTheme = oppositeTheme;
        NavViewSettingsButtonLabel.RequestedTheme = oppositeTheme;
        NavViewDevicesButtonLabel.RequestedTheme = oppositeTheme;
        NavViewInfoButtonLabel.RequestedTheme = oppositeTheme;
        NavViewOkashiButtonLabel.RequestedTheme = oppositeTheme;
        UpdateIconText.RequestedTheme = oppositeTheme;
        PreviewBadgeLabel.RequestedTheme = oppositeTheme;

        await Task.Delay(30);
        HelpButton.RequestedTheme = Interfacing.ActualTheme;
        NavViewGeneralButtonLabel.RequestedTheme = Interfacing.ActualTheme;
        NavViewSettingsButtonLabel.RequestedTheme = Interfacing.ActualTheme;
        NavViewDevicesButtonLabel.RequestedTheme = Interfacing.ActualTheme;
        NavViewInfoButtonLabel.RequestedTheme = Interfacing.ActualTheme;
        NavViewOkashiButtonLabel.RequestedTheme = Interfacing.ActualTheme;
        UpdateIconText.RequestedTheme = Interfacing.ActualTheme;
        PreviewBadgeLabel.RequestedTheme = Interfacing.ActualTheme;
    }

    private async Task ExecuteUpdates()
    {
        // TODO
    }

    private async Task CheckUpdates(bool show, uint delay)
    {
        // TODO
    }

    private void InitializerTeachingTip_ActionButtonClick(TeachingTip sender, object args)
    {
        // TODO
    }

    private void InitializerTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        // TODO
    }

    private async void ReloadTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        Logger.Info("Reload has been invoked: turning trackers off...");

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Mark trackers as inactive
        Interfacing.K2AppTrackersInitialized = false;
        if (Shared.General.ToggleTrackersButton != null)
            Shared.General.ToggleTrackersButton.IsChecked = false;

        Logger.Info("Reload has been invoked: setting up exit flags...");

        // Mark exiting as true
        Interfacing.IsExitingNow = true;
        await Task.Delay(50);

        /* Restart app */

        // Literals
        Logger.Info("Reload has been invoked: trying to restart the app...");

        // If we've found who asked
        if (File.Exists(Interfacing.GetProgramLocation().ToString()))
        {
            // Log the caller
            Logger.Info($"The current caller process is: {Interfacing.GetProgramLocation()}");

            // Exit the app
            Logger.Info("Exiting in 500ms...");

            // Don't execute the exit routine
            Interfacing.IsExitHandled = true;

            // Handle a typical app exit
            await Interfacing.HandleAppExit(500);

            // Restart and exit with code 0
            Process.Start(Interfacing.GetProgramLocation().ToString());
            Environment.Exit(0);
        }

        // Still here?
        Logger.Error("App will not be restarted due to caller process identification error.");

        Interfacing.ShowToast(
            Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed/Title"),
            Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed"));

        Interfacing.ShowVRToast(
            Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed/Title"),
            Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed"));
    }

    private void NavView_Loaded(object sender, RoutedEventArgs e)
    {
        Interfacing.ActualTheme = NavView.ActualTheme;

        Shared.Main.AppWindow.TitleBar.ButtonForegroundColor =
            Interfacing.ActualTheme == ElementTheme.Dark
                ? Colors.White
                : Colors.Black;

        Shared.Main.AttentionBrush =
            Interfacing.ActualTheme == ElementTheme.Dark
                ? Application.Current.Resources["AttentionBrush_Dark"].As<SolidColorBrush>()
                : Application.Current.Resources["AttentionBrush_Light"].As<SolidColorBrush>();

        Shared.Main.NeutralBrush =
            Interfacing.ActualTheme == ElementTheme.Dark
                ? Application.Current.Resources["NeutralBrush_Dark"].As<SolidColorBrush>()
                : Application.Current.Resources["NeutralBrush_Light"].As<SolidColorBrush>();

        // NavView doesn't load any page by default, so load home page
        NavView.SelectedItem = NavView.MenuItems[0];

        // If navigation occurs on SelectionChanged, then this isn't needed.
        // Because we use ItemInvoked to navigate, we need to call Navigate
        // here to load the home page.
        Shared.Main.NavigateToPage("general", new EntranceNavigationTransitionInfo());
    }

    private void NavView_ItemInvoked(NavigationView sender,
        NavigationViewItemInvokedEventArgs args)
    {
        Shared.Main.NavigateToPage(
            args.InvokedItemContainer.Tag.ToString(),
            args.RecommendedNavigationTransitionInfo);
    }

    private void NavView_BackRequested(NavigationView sender,
        NavigationViewBackRequestedEventArgs args)
    {
        if (ContentFrame.CanGoBack && (!NavView.IsPaneOpen || NavView.DisplayMode is not
                (NavigationViewDisplayMode.Compact or NavigationViewDisplayMode.Minimal)))
            ContentFrame.GoBack();
    }

    private void UpdateButton_Tapped(object sender, Microsoft.UI.Xaml.Input.TappedRoutedEventArgs e)
    {
        // TODO
    }

    private void UpdateButton_Loaded(object sender, RoutedEventArgs e)
    {
        // TODO
    }

    private void HelpButton_Tapped(object sender, Microsoft.UI.Xaml.Input.TappedRoutedEventArgs e)
    {
        // TODO
    }

    private void ContentFrame_NavigationFailed(object sender, Microsoft.UI.Xaml.Navigation.NavigationFailedEventArgs e)
    {
        throw new Exception($"Failed to load page{e.SourcePageType.AssemblyQualifiedName}");
    }

    private void ButtonFlyout_Opening(object sender, object e)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void ButtonFlyout_Closing(FlyoutBase sender,
        FlyoutBaseClosingEventArgs args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }

    private void InstallLaterButton_Click(object sender, RoutedEventArgs e)
    {
        // TODO
    }

    private void InstallNowButton_Click(object sender, RoutedEventArgs e)
    {
        // TODO
    }

    private void HelpFlyoutDocsButton_Click(object sender, RoutedEventArgs e)
    {
        // TODO
    }

    private void HelpFlyoutDiscordButton_Click(object sender, RoutedEventArgs e)
    {
        // TODO
    }

    private void HelpFlyoutDevButton_Click(object sender, RoutedEventArgs e)
    {
        // TODO
    }

    private async void HelpFlyoutLicensesButton_Click(object sender, RoutedEventArgs e)
    {
        await Task.Delay(500);
        LicensesFlyout.ShowAt(MainGrid);
    }

    private void LicensesFlyout_Opening(object sender, object e)
    {
        Shared.Main.InterfaceBlockerGrid.Opacity = 0.35;
        Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = true;

        Interfacing.IsNuxPending = true;

        // Load the license text
        if (File.Exists(Path.Combine(Interfacing.GetProgramLocation().DirectoryName, "Assets", "Licenses.txt")))
            LicensesText.Text = File.ReadAllText(Path.Combine(
                Interfacing.GetProgramLocation().DirectoryName, "Assets", "Licenses.txt"));

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void LicensesFlyout_Closed(object sender, object e)
    {
        Shared.Main.InterfaceBlockerGrid.Opacity = 0.0;
        Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = false;

        Interfacing.IsNuxPending = false;
    }

    private bool TrySetMicaBackdrop()
    {
        if (!Microsoft.UI.Composition.SystemBackdrops.MicaController.IsSupported())
        {
            Logger.Info("Mica is not supported! Time to update Windows, man!");
            return false; // Mica is not supported on this system
        }

        m_wsdqHelper = new WindowsSystemDispatcherQueueHelper();
        m_wsdqHelper.EnsureWindowsSystemDispatcherQueueController();

        // Hooking up the policy object
        m_configurationSource = new Microsoft.UI.Composition.SystemBackdrops.SystemBackdropConfiguration();
        Activated += Window_Activated;
        Closed += Window_Closed;
        ((FrameworkElement)Content).ActualThemeChanged += Window_ThemeChanged;

        // Initial configuration state.
        m_configurationSource.IsInputActive = true;
        SetConfigurationSourceTheme();

        m_micaController = new Microsoft.UI.Composition.SystemBackdrops.MicaController();

        // Enable the system backdrop.
        // Note: Be sure to have "using WinRT;" to support the Window.As<...>() call.
        m_micaController.AddSystemBackdropTarget(this
            .As<Microsoft.UI.Composition.ICompositionSupportsSystemBackdrop>());
        m_micaController.SetSystemBackdropConfiguration(m_configurationSource);

        // Change the window background to support mica
        MainGrid.Background = new SolidColorBrush(Colors.Transparent);
        return true; // succeeded
    }

    private void Window_Activated(object sender, WindowActivatedEventArgs args)
    {
        m_configurationSource.IsInputActive = args.WindowActivationState != WindowActivationState.Deactivated;
    }

    private async void Window_Closed(object sender, WindowEventArgs args)
    {
        // Handled(true) means Cancel()
        // and Handled(false) means Continue()
        // -> Block exiting until we're done
        args.Handled = true;

        // Handle all the exit actions (if needed)
        // Show the close tip (if not shown yet)
        if (!Interfacing.IsExitHandled &&
            !Interfacing.AppSettings.FirstShutdownTipShown)
        {
            ShutdownTeachingTip.IsOpen = true;

            Interfacing.AppSettings.FirstShutdownTipShown = true;
            Interfacing.AppSettings.SaveSettings(); // Save settings
            return;
        }

        if (Interfacing.UpdateOnClosed)
            await ExecuteUpdates();

        if (!Interfacing.IsExitHandled)
        {
            // Handle the exit actions
            Interfacing.HandleAppExit(1000);

            // Make sure any Mica/Acrylic controller is disposed so it doesn't try to
            // use this closed window.
            if (m_micaController != null)
            {
                m_micaController.Dispose();
                m_micaController = null;
            }

            Activated -= Window_Activated;
            m_configurationSource = null;
        }

        // Call before exiting for subsequent invocations to launch a new process
        Shared.Main.NotificationManager.Unregister();

        // Finally allow exits
        args.Handled = false;
    }

    private void Window_ThemeChanged(FrameworkElement sender, object args)
    {
        if (m_configurationSource != null) SetConfigurationSourceTheme();
    }

    private void SetConfigurationSourceTheme()
    {
        m_configurationSource.Theme = ((FrameworkElement)Content).ActualTheme switch
        {
            ElementTheme.Dark => Microsoft.UI.Composition.SystemBackdrops.SystemBackdropTheme.Dark,
            ElementTheme.Light => Microsoft.UI.Composition.SystemBackdrops.SystemBackdropTheme.Light,
            ElementTheme.Default => Microsoft.UI.Composition.SystemBackdrops.SystemBackdropTheme.Default,
            _ => m_configurationSource.Theme
        };
    }
}