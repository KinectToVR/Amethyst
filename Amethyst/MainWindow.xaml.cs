// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.ComponentModel.Composition.Hosting;
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
using System.Timers;
using Windows.Storage.Streams;
using Windows.System;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Data.Json;
using Amethyst.MVVM;
using Amethyst.Plugins.Contract;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst;

/// <summary>
///     An empty window that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class MainWindow : Window
{
    private bool _mainPageLoadedOnce = false, _mainPageInitFinished = false;
    private string remoteVersionString = AppData.K2InternalVersion;

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
                    .ForEach(plugin => plugin.OnLoad());

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
        Task.Run(Main.MainLoop);

        // Disable internal sounds
        ElementSoundPlayer.State = ElementSoundPlayerState.Off;

        /* Load devices, config and compose (start) */

        // Create the plugin directory (if not existent)
        Directory.CreateDirectory(Path.Combine(
            AppDomain.CurrentDomain.BaseDirectory, "Plugins"));

        // Search the "Plugins" sub-directory for assemblies that match the imports.
        var catalog = new AggregateCatalog();

        // Add the current assembly to support invoke method exports
        Logger.Info("Exporting the plugin host plugin...");
        catalog.Catalogs.Add(new AssemblyCatalog(Assembly.GetExecutingAssembly()));

        // Iterate over all directories in .\Plugins dir and add all Plugin* dirs to catalogs
        var pluginDirectoryList = Directory.EnumerateDirectories(
            Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "Plugins"),
            "*", SearchOption.TopDirectoryOnly).ToList();

        Logger.Info("Searching for local devices now...");
        pluginDirectoryList.ForEach(pluginPath =>
            catalog.Catalogs.AddPlugin(new DirectoryInfo(pluginPath)));

        try
        {
            // Load the JSON source into buffer, parse
            Logger.Info("Searching for external devices now...");
            var jsonRoot = JsonObject.Parse(
                File.ReadAllText(Interfacing.GetK2AppDataFileDir("amethystpaths.k2path")));

            // Loop over all the external devices and append
            if (jsonRoot.ContainsKey("external_devices"))
                jsonRoot.GetNamedArray("external_devices").ToList()
                    .Select(pluginEntry => Directory.Exists(pluginEntry.ToString())).ToList()
                    .ForEach(pluginPath => catalog.Catalogs.AddPlugin(new DirectoryInfo(pluginPath.ToString())));

            else Logger.Info("No external devices found! Loading the local ones now...");
        }
        catch (Exception e)
        {
            Logger.Error($"Checking for external devices has failed, an exception occurred. Message: {e.Message}");
        }

        if (pluginDirectoryList.Count < 1)
        {
            Logger.Fatal("No plugins (tracking devices) found! Shutting down...");
            Interfacing.Fail(-12); // Exit and cause the crash handler to appear
        }

        Logger.Info($"Found {pluginDirectoryList.Count} potentially valid plugin directories...");
        try
        {
            // Match Imports with corresponding exports in all catalogs in the container
            using var container = new CompositionContainer(catalog, CompositionOptions.DisableSilentRejection);
            var plugins = container.GetExports<ITrackingDevice, ITrackingDeviceMetadata>().ToList();
            Logger.Info($"Found {plugins.Count} potentially valid plugins...");

            foreach (var plugin in plugins)
                try
                {
                    Logger.Info($"Parsing ({plugin.Metadata.Name}, {plugin.Metadata.Guid})...");

                    // Check the plugin GUID against others loaded, INVALID and null
                    if (string.IsNullOrEmpty(plugin.Metadata.Guid) || plugin.Metadata.Guid == "INVALID" ||
                        TrackingDevices.TrackingDevicesVector.ContainsKey(plugin.Metadata.Guid))
                    {
                        TrackingDevices.LoadAttemptedTrackingDevicesVector.Add((plugin.Metadata.Name,
                            plugin.Metadata.Guid, TrackingDevices.PluginLoadError.BadOrDuplicateGuid, ""));

                        Logger.Error($"({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                     "has a duplicate GUID value to another plugin, discarding it!");
                        continue; // Give up on this one :(
                    }

                    try
                    {
                        Logger.Info($"Trying to load ({plugin.Metadata.Name}, {plugin.Metadata.Guid})...");
                        Logger.Info($"Result: {plugin.Value}"); // Load the plugin
                    }
                    catch (CompositionException e)
                    {
                        Logger.Error("Loading plugin ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                     "failed with a local (plugin-wise) MEF exception: " +
                                     $"Message: {e.Message}\nErrors occurred: {e.Errors}\n" +
                                     $"Possible causes: {e.RootCauses}\nTrace: {e.StackTrace}");

                        TrackingDevices.LoadAttemptedTrackingDevicesVector.Add((plugin.Metadata.Name,
                            plugin.Metadata.Guid, TrackingDevices.PluginLoadError.NoDeviceDependencyDll, ""));
                        continue; // Give up on this one :(
                    }
                    catch (Exception e)
                    {
                        Logger.Error($"Loading plugin ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                     "failed with an exception, probably some of its dependencies are missing. " +
                                     $"Message: {e.Message}, Trace: {e.StackTrace}");

                        TrackingDevices.LoadAttemptedTrackingDevicesVector.Add((plugin.Metadata.Name,
                            plugin.Metadata.Guid, TrackingDevices.PluginLoadError.Other, ""));
                        continue; // Give up on this one :(
                    }

                    // It must be good if we're somehow still here
                    var pluginLocation = Assembly.GetAssembly(plugin.Value.GetType()).Location;
                    var pluginFolder = Directory.GetParent(pluginLocation).FullName;

                    Logger.Info($"Adding ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                "to the load-attempted device plugins list (TrackingDevices)...");
                    TrackingDevices.LoadAttemptedTrackingDevicesVector.Add((plugin.Metadata.Name,
                        plugin.Metadata.Guid, TrackingDevices.PluginLoadError.NoError, pluginFolder));

                    Logger.Info($"Creating ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                "localized strings resource context (TrackingDevices)...");
                    TrackingDevices.TrackingDevicesLocalizationResourcesRootsVector.Add(
                        plugin.Metadata.Guid, (new JsonObject(), pluginFolder));

                    Logger.Info($"Registering ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                "default root language resource context (TrackingDevices)...");
                    Interfacing.Plugins.SetLocalizationResourcesRoot(pluginFolder, plugin.Metadata.Guid);

                    Logger.Info($"Adding ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                "to the global tracking device plugins list (TrackingDevices)...");
                    TrackingDevices.TrackingDevicesVector.Add(plugin.Metadata.Guid, new TrackingDevice(
                        plugin.Metadata.Name, plugin.Metadata.Guid, pluginLocation, plugin.Value));

                    Logger.Info($"Device ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                $"with the device class library dll at: {pluginLocation}\n" +
                                $"properties:\nStatus: {plugin.Value.DeviceStatus}\n" +
                                $"Status String: {plugin.Value.DeviceStatusString}\n" +
                                $"Supports Flip: {plugin.Value.IsFlipSupported}\n" +
                                $"Initialized: {plugin.Value.IsInitialized}\n" +
                                $"Overrides Physics: {plugin.Value.IsPhysicsOverrideEnabled}\n" +
                                $"Blocks Filtering: {plugin.Value.IsPositionFilterBlockingEnabled}\n" +
                                $"Updates By Itself: {plugin.Value.IsSelfUpdateEnabled}\n" +
                                $"Supports Settings: {plugin.Value.IsSettingsDaemonSupported}\n" +
                                $"Provides Prepended Joints: {plugin.Value.TrackedJoints.Count}");
                }
                catch (Exception e)
                {
                    Logger.Error($"Loading plugin ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                 "failed with a (hopefully) caught exception. " +
                                 $"Provided exception Message: {e.Message}, Trace: {e.StackTrace}");
                }
        }
        catch (CompositionException e)
        {
            Logger.Error("Loading plugins failed with a global MEF exception: " +
                         $"Message: {e.Message}\nErrors occurred: {e.Errors}\n" +
                         $"Possible causes: {e.RootCauses}\nTrace: {e.StackTrace}");
        }

        if (TrackingDevices.TrackingDevicesVector.Count < 1)
        {
            Logger.Fatal("No plugins (tracking devices) loaded! Shutting down...");
            Interfacing.Fail(-12); // Exit and cause the crash handler to appear
        }






        /* Load devices, config and compose (end) */

        // Notify of the setup end
        _mainPageInitFinished = true;
    }

    private async void MainGrid_Loaded(object sender, RoutedEventArgs e)
    {
        // Load theme config
        Shared.Main.MainNavigationView.XamlRoot.Content.As<Grid>().RequestedTheme =
            AppData.Settings.AppTheme switch
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

    [LibraryImport("user32.dll")]
    private static partial int SendMessage(IntPtr hWnd, int wMsg, IntPtr wParam, IntPtr lParam);

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
        Interfacing.UpdatingNow = true;
        UpdatePendingFlyout.Hide();
        UpdateIconDot.Opacity = 0.0;
        await Task.Delay(500);

        // Mark the update footer as active
        UpdateIconGrid.Translation = Vector3.Zero;
        UpdateIconText.Opacity = 0.0;
        UpdateIcon.Foreground = Shared.Main.AttentionBrush;
        IconRotation.Begin();

        UpdatePendingFlyoutFooter.Text = $"Amethyst v{remoteVersionString}";
        UpdatePendingFlyoutStatusContent.Text =
            Interfacing.LocalizedJsonString("/SharedStrings/Updates/Statuses/Downloading");

        if (!Interfacing.IsNuxPending)
            UpdatePendingFlyout.ShowAt(HelpButton, new FlyoutShowOptions()
            {
                Placement = FlyoutPlacementMode.RightEdgeAlignedBottom,
                ShowMode = FlyoutShowMode.Transient
            });

        // Success? ...or nah?
        var updateError = false;

        // Reset the progressbar
        var updatePendingProgressBar = new ProgressBar
        {
            IsIndeterminate = false,
            HorizontalAlignment = HorizontalAlignment.Stretch
        };

        UpdatePendingFlyoutMainStack.Children.Add(updatePendingProgressBar);

        // Download the latest installer/updater
        // https://github.com/KinectToVR/Amethyst-Installer-Releases/releases/latest/download/Amethyst-Installer.exe

        try
        {
            using var client = new Windows.Web.Http.HttpClient();
            var installerUri = "";

            Logger.Info("Checking out the updater version... [GET]");

            // Get the installer download Uri
            try
            {
                using var response = await client.GetAsync(new Uri("https://api.k2vr.tech/v0/installer/version"));
                var getInstallerUri = await response.Content.ReadAsStringAsync();

                if (!string.IsNullOrEmpty(getInstallerUri))
                {
                    var jsonRoot = JsonObject.Parse(getInstallerUri);

                    // Parse the loaded json
                    if (jsonRoot.ContainsKey("download"))
                    {
                        installerUri = jsonRoot.GetNamedString("download");
                    }
                    else
                    {
                        Logger.Error("Installer-uri-check failed, the \"download\" key wasn't found.");
                        updateError = true;
                    }
                }
                else
                {
                    Logger.Error("Installer-uri-check failed, the string was empty.");
                    updateError = true;
                }
            }
            catch (Exception e)
            {
                Logger.Error($"Error checking the updater download Uri! Message: {e.Message}");
                updateError = true;
            }


            // Download if we're ok
            if (!updateError)
                try
                {
                    var thisFolder = await Windows.Storage.StorageFolder
                        .GetFolderFromPathAsync(Interfacing.GetK2AppDataTempDir().FullName);

                    // To save downloaded image to local storage
                    var installerFile = await thisFolder.CreateFileAsync(
                        "Amethyst-Installer.exe", Windows.Storage.CreationCollisionOption.ReplaceExisting);

                    using var fsInstallerFile = await installerFile.OpenAsync(Windows.Storage.FileAccessMode.ReadWrite);
                    using var response = await client.GetAsync(new Uri(installerUri),
                        Windows.Web.Http.HttpCompletionOption.ResponseHeadersRead);

                    using var stream = await response.Content.ReadAsInputStreamAsync();

                    ulong totalBytesRead = 0; // Already read buffer
                    var totalBytesToRead = response.Content.Headers.ContentLength.Value;
                    var downloadStatusString =
                        Interfacing.LocalizedJsonString("/SharedStrings/Updates/Statuses/Downloading");

                    while (true)
                    {
                        var buffer = new byte[64 * 1024];
                        var readBuffer = await stream.ReadAsync(
                            buffer.AsBuffer(), (uint)buffer.Length, InputStreamOptions.None);

                        // There's nothing else to read
                        if (readBuffer is null || readBuffer.Length == 0) break;

                        // Report the progress
                        totalBytesRead += readBuffer.Length;
                        Logger.Info($"Downloaded {totalBytesRead} of {totalBytesToRead} bytes...");

                        // Update the progressbar
                        var progress = (double)totalBytesRead / totalBytesToRead;

                        updatePendingProgressBar.Value = progress * 100;
                        UpdatePendingFlyoutStatusContent.Text = downloadStatusString.Replace(
                            "0", ((int)progress * 100).ToString());

                        // Write to file
                        await fsInstallerFile.WriteAsync(readBuffer);
                    }
                }
                catch (Exception e)
                {
                    Logger.Error($"Error downloading the updater! Message: {e.Message}");
                    updateError = true;
                }
        }
        catch (Exception e)
        {
            Logger.Error($"Update failed, an exception occurred. Message: {e.Message}");
            updateError = true;
        }

        // Mark the update footer as inactive
        IconRotation.Stop();
        UpdateIcon.Foreground = Shared.Main.NeutralBrush;
        UpdateIconGrid.Translation = new Vector3(0, -8, 0);
        UpdateIconText.Opacity = 1.0;

        // Check the file result and the DL result
        if (!updateError)
        {
            Process.Start(Path.Combine(Interfacing.GetProgramLocation().DirectoryName,
                    "K2CrashHandler", "K2CrashHandler.exe"), // Optional auto-restart scenario "-o"
                $" --update {(Interfacing.UpdateOnClosed ? "" : "-o")} -path \"{Interfacing.GetProgramLocation().DirectoryName}\"");

            // Exit, cleanup should be automatic
            Interfacing.UpdateOnClosed = false; // Don't re-do
            Environment.Exit(0); // Should get caught by the exit handler
        }

        // Still here? Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Error);

        updatePendingProgressBar.ShowError = true;
        UpdatePendingFlyoutStatusContent.Text =
            Interfacing.LocalizedJsonString("/SharedStrings/Updates/Statuses/Error");

        await Task.Delay(3200);
        UpdatePendingFlyout.Hide();
        await Task.Delay(500);

        // Don't give up yet
        Interfacing.UpdatingNow = false;
        if (Interfacing.UpdateFound)
            UpdateIconDot.Opacity = 1.0;

        // Remove the progressbar
        UpdatePendingFlyoutMainStack.Children.Remove(
            UpdatePendingFlyoutMainStack.Children.Last());
    }

    private async Task CheckUpdates(bool show, uint delay = 0)
    {
        // Attempt only after init
        if (!_mainPageInitFinished) return;

        {
            // Check if we're midway updating
            if (Interfacing.UpdatingNow)
            {
                // Show the updater progress flyout
                if (!Interfacing.IsNuxPending)
                    UpdatePendingFlyout.ShowAt(HelpButton, new FlyoutShowOptions()
                    {
                        Placement = FlyoutPlacementMode.RightEdgeAlignedBottom,
                        ShowMode = FlyoutShowMode.Transient
                    });

                return; // Don't proceed further
            }

            // Mark as checking
            Interfacing.CheckingUpdatesNow = true;
            await Task.Delay((int)delay);

            // Don't check if found
            if (!Interfacing.UpdateFound)
            {
                // Mark the update footer as active
                UpdateIconGrid.Translation = Vector3.Zero;
                UpdateIconText.Opacity = 0.0;
                UpdateIcon.Foreground = Shared.Main.AttentionBrush;

                // Here check for updates (via external bool)
                IconRotation.Begin();

                // Check for updates
                var updateTimer = new Task(() => Task.Delay(1000));

                // Check now
                updateTimer.Start();
                Interfacing.UpdateFound = false;

                // Dummy for holding change logs
                List<string> changesStringVector = new();

                // Check for updates
                try
                {
                    using var client = new Windows.Web.Http.HttpClient();
                    string getReleaseVersion = "", getDocsLanguages = "en";

                    // Release
                    try
                    {
                        Logger.Info("Checking for updates... [GET]");
                        using var response =
                            await client.GetAsync(new Uri($"https://api.k2vr.tech/v{AppData.K2ApiVersion}/update"));
                        getReleaseVersion = await response.Content.ReadAsStringAsync();
                    }
                    catch (Exception e)
                    {
                        Logger.Error($"Error getting the release info! Message: {e.Message}");
                    }

                    // Language
                    try
                    {
                        Logger.Info("Checking available languages... [GET]");
                        using var response =
                            await client.GetAsync(new Uri("https://docs.k2vr.tech/shared/locales.json"));
                        getDocsLanguages = await response.Content.ReadAsStringAsync();
                    }
                    catch (Exception e)
                    {
                        Logger.Error($"Error getting the language info! Message: {e.Message}");
                    }

                    // If the read string isn't empty, proceed to checking for updates
                    if (!string.IsNullOrEmpty(getReleaseVersion))
                    {
                        Logger.Info($"Update-check successful, string:\n{getReleaseVersion}");

                        // Parse the loaded json
                        var jsonHead = JsonObject.Parse(getReleaseVersion);

                        if (!jsonHead.ContainsKey("amethyst"))
                            Logger.Error("The latest release's manifest was invalid!");

                        // Parse the amethyst entry
                        var jsonRoot = jsonHead.GetNamedObject("amethyst");

                        if (!jsonRoot.ContainsKey("version") ||
                            !jsonRoot.ContainsKey("version_string") ||
                            !jsonRoot.ContainsKey("changelog"))
                        {
                            Logger.Error("The latest release's manifest was invalid!");
                        }

                        else
                        {
                            // Get the version tag (uint, fallback to latest)
                            var remoteVersion = (int)jsonRoot.GetNamedNumber("version", AppData.K2IntVersion);

                            // Get the remote version name
                            remoteVersionString = jsonRoot.GetNamedString("version_string");

                            Logger.Info($"Local version: {AppData.K2IntVersion}");
                            Logger.Info($"Remote version: {remoteVersion}");

                            Logger.Info($"Local version string: {AppData.K2InternalVersion}");
                            Logger.Info($"Remote version string: {remoteVersionString}");

                            // Check the version
                            if (AppData.K2IntVersion < remoteVersion)
                                Interfacing.UpdateFound = true;

                            // Cache the changes
                            changesStringVector = jsonRoot.GetNamedArray("changelog")
                                .Select(x => x.ToString()).ToList();
                        }
                    }
                    else
                    {
                        Logger.Error("Update-check failed, the string was empty.");
                    }

                    if (!string.IsNullOrEmpty(getDocsLanguages))
                    {
                        // Parse the loaded json
                        var jsonRoot = JsonObject.Parse(getDocsLanguages);

                        // Check if the resource root is fine & the language code exists
                        Interfacing.DocsLanguageCode = AppData.Settings.AppLanguage;

                        if (jsonRoot is null || !jsonRoot.ContainsKey(Interfacing.DocsLanguageCode))
                        {
                            Logger.Info("Docs do not contain a language with code " +
                                        $"\"{Interfacing.DocsLanguageCode}\", falling back to \"en\" (English)!");
                            Interfacing.DocsLanguageCode = "en";
                        }
                    }
                    else
                    {
                        Logger.Error("Language-check failed, the string was empty.");
                    }
                }
                catch (Exception e)
                {
                    Logger.Error($"Update failed, an exception occurred. Message: {e.Message}");
                }

                // Limit time to (min) 1s
                await updateTimer;

                // Resume to UI and stop the animation
                IconRotation.Stop();

                // Mark the update footer as inactive
                {
                    UpdateIcon.Foreground = Shared.Main.NeutralBrush;

                    UpdateIconGrid.Translation = new Vector3(0, -8, 0);
                    UpdateIconText.Opacity = 1.0;
                }

                if (Interfacing.UpdateFound)
                {
                    FlyoutHeader.Text = Interfacing.LocalizedJsonString("/SharedStrings/Updates/NewUpdateFound");
                    FlyoutFooter.Text = $"Amethyst v{remoteVersionString}";

                    var changelogString = "";
                    changesStringVector.ForEach(x => changelogString += $"- {x}\n");
                    FlyoutContent.Text = changelogString.TrimEnd('\n');
                    FlyoutContent.Margin = new Thickness(0, 0, 0, 12);

                    InstallLaterButton.Visibility = Visibility.Visible;
                    InstallNowButton.Visibility = Visibility.Visible;
                    UpdateIconDot.Opacity = 1.0;
                }
                else
                {
                    FlyoutHeader.Text = Interfacing.LocalizedJsonString("/SharedStrings/Updates/UpToDate");
                    FlyoutFooter.Text = $"Amethyst v{AppData.K2InternalVersion}";
                    FlyoutContent.Text = Interfacing.LocalizedJsonString("/SharedStrings/Updates/Suggestions");
                    FlyoutContent.Margin = new Thickness(0);

                    InstallLaterButton.Visibility = Visibility.Collapsed;
                    InstallNowButton.Visibility = Visibility.Collapsed;
                    UpdateIconDot.Opacity = 0.0;
                }
            }

            // If an update was found, show it
            // (or if the check was manual)
            if ((Interfacing.UpdateFound || show) && !Interfacing.IsNuxPending)
                UpdateFlyout.ShowAt(HelpButton, new FlyoutShowOptions()
                {
                    Placement = FlyoutPlacementMode.RightEdgeAlignedBottom,
                    ShowMode = FlyoutShowMode.Transient
                });

            // Uncheck
            Interfacing.CheckingUpdatesNow = false;
        }
    }

    private void InitializerTeachingTip_ActionButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Dismiss the current tip
        Shared.TeachingTips.Main.InitializerTeachingTip.IsOpen = false;

        // Just dismiss the tip
        Shared.Main.InterfaceBlockerGrid.Opacity = 0.0;
        Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = false;
        Interfacing.IsNuxPending = false;
    }

    private void InitializerTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Dismiss the current tip
        Shared.TeachingTips.Main.InitializerTeachingTip.IsOpen = false;

        // Navigate to the general page
        Shared.Main.MainNavigationView.SelectedItem =
            Shared.Main.MainNavigationView.MenuItems[0];

        Shared.Main.NavigateToPage("general",
            new EntranceNavigationTransitionInfo());

        // Show the next tip (general page)
        Shared.TeachingTips.General.ToggleTrackersTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        Shared.TeachingTips.General.ToggleTrackersTeachingTip.IsOpen = true;
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

    private async void UpdateButton_Tapped(object sender, Microsoft.UI.Xaml.Input.TappedRoutedEventArgs e)
    {
        // Check for updates (and show)
        if (Interfacing.CheckingUpdatesNow) return;
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
        await CheckUpdates(true);
    }

    private async void UpdateButton_Loaded(object sender, RoutedEventArgs e)
    {
        // Show the startup tour teachingtip
        if (!AppData.Settings.FirstTimeTourShown)
        {
            // Play a sound
            AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

            // Show the first tip
            Shared.Main.InterfaceBlockerGrid.Opacity = 0.35;
            Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = true;

            Shared.TeachingTips.Main.InitializerTeachingTip.IsOpen = true;
            Interfacing.IsNuxPending = true;
        }

        // Check for updates (and show)
        await CheckUpdates(false, 2000);
    }

    private void HelpButton_Tapped(object sender, Microsoft.UI.Xaml.Input.TappedRoutedEventArgs e)
    {
        // Change the docs button's text
        HelpFlyoutDocsButton.Text = Interfacing.CurrentAppState switch
        {
            "general" => Interfacing.LocalizedJsonString(
                "/SharedStrings/Buttons/Help/Docs/GeneralPage/Overview"),
            "calibration" => Interfacing.LocalizedJsonString(
                "/SharedStrings/Buttons/Help/Docs/GeneralPage/Calibration/Main"),
            "calibration_auto" => Interfacing.LocalizedJsonString(
                "/SharedStrings/Buttons/Help/Docs/GeneralPage/Calibration/Automatic"),
            "calibration_manual" => Interfacing.LocalizedJsonString(
                "/SharedStrings/Buttons/Help/Docs/GeneralPage/Calibration/Manual"),
            "offsets" => Interfacing.LocalizedJsonString(
                "/SharedStrings/Buttons/Help/Docs/GeneralPage/Offsets"),
            "settings" => Interfacing.LocalizedJsonString(
                "/SharedStrings/Buttons/Help/Docs/SettingsPage/Overview"),
            "devices" => Interfacing.LocalizedJsonString(
                "/SharedStrings/Buttons/Help/Docs/DevicesPage/Overview"),
            "overrides" => Interfacing.LocalizedJsonString(
                "/SharedStrings/Buttons/Help/Docs/DevicesPage/Overrides"),
            "info" => Interfacing.LocalizedJsonString(
                "/SharedStrings/Buttons/Help/Docs/InfoPage/OpenCollective"),
            _ => HelpFlyoutDocsButton.Text
        };

        // Show the help flyout
        HelpFlyout.ShowAt(HelpButton, new FlyoutShowOptions()
        {
            Placement = FlyoutPlacementMode.RightEdgeAlignedBottom,
            ShowMode = FlyoutShowMode.Transient
        });
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
        Interfacing.UpdateOnClosed = true;
        UpdateFlyout.Hide();
    }

    private async void InstallNowButton_Click(object sender, RoutedEventArgs e)
    {
        await ExecuteUpdates();
        UpdateFlyout.Hide();
    }

    private void HelpFlyoutDocsButton_Click(object sender, RoutedEventArgs e)
    {
        Launcher.LaunchUriAsync(new Uri(Interfacing.CurrentAppState switch
        {
            "calibration" => $"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/calibration/",
            "calibration_auto" => $"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/calibration/#3",
            "calibration_manual" => $"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/calibration/#6",
            "devices" or "offsets" or "settings" => $"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/",
            "overrides" => $"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/overrides/",
            "info" => "https://opencollective.com/k2vr",
            "general" or _ => $"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/"
        }));
    }

    private void HelpFlyoutDiscordButton_Click(object sender, RoutedEventArgs e)
    {
        Launcher.LaunchUriAsync(new Uri("https://discord.gg/YBQCRDG"));
    }

    private void HelpFlyoutDevButton_Click(object sender, RoutedEventArgs e)
    {
        Launcher.LaunchUriAsync(new Uri("https://github.com/KinectToVR"));
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
            !AppData.Settings.FirstShutdownTipShown)
        {
            ShutdownTeachingTip.IsOpen = true;

            AppData.Settings.FirstShutdownTipShown = true;
            AppData.Settings.SaveSettings(); // Save settings
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