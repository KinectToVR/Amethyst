// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.ComponentModel.Composition;
using System.ComponentModel.Composition.Hosting;
using System.Data;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Reflection;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Runtime.Loader;
using System.Threading;
using System.Threading.Tasks;
using Windows.Data.Json;
using Windows.Graphics;
using Windows.Storage;
using Windows.Storage.Streams;
using Windows.System;
using Windows.Web.Http;
using Amethyst.Classes;
using Amethyst.MVVM;
using Amethyst.Pages;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using Microsoft.AppCenter.Analytics;
using Microsoft.AppCenter.Crashes;
using Microsoft.UI;
using Microsoft.UI.Composition;
using Microsoft.UI.Composition.SystemBackdrops;
using Microsoft.UI.Windowing;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Media.Animation;
using Microsoft.UI.Xaml.Navigation;
using Microsoft.Windows.AppNotifications;
using WinRT;
using WinRT.Interop;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst;

/// <summary>
///     An empty window that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class MainWindow : Window, INotifyPropertyChanged
{
    public delegate Task RequestApplicationUpdate(object sender, EventArgs e);

    public static RequestApplicationUpdate RequestUpdateEvent;
    public static RequestApplicationUpdate RequestUpdateFoundEvent;

    private readonly bool _mainPageInitFinished;

    private SystemBackdropConfiguration _configurationSource;
    private bool _mainPageLoadedOnce;
    private MicaController _micaController;
    private string _remoteVersionString = AppData.VersionString.Display;

    private WindowsSystemDispatcherQueueHelper _wsdqHelper; // See separate sample below for implementation

    public MainWindow()
    {
        InitializeComponent();
        TrySetMicaBackdrop();

        // Cache needed UI elements
        Shared.TeachingTips.MainPage.InitializerTeachingTip = InitializerTeachingTip;
        Shared.TeachingTips.MainPage.ReloadInfoBar = ReloadInfoBar;

        Shared.Main.MainNavigationView = NavView;
        Shared.Main.AppTitleLabel = AppTitleLabel;

        Shared.Main.InterfaceBlockerGrid = InterfaceBlockerGrid;
        Shared.Main.NavigationBlockerGrid = NavigationBlockerGrid;
        Shared.Main.MainContentFrame = ContentFrame;

        Shared.Main.NavigationItems.NavViewGeneralButtonIcon = NavViewGeneralButtonIcon;
        Shared.Main.NavigationItems.NavViewSettingsButtonIcon = NavViewSettingsButtonIcon;
        Shared.Main.NavigationItems.NavViewDevicesButtonIcon = NavViewDevicesButtonIcon;
        Shared.Main.NavigationItems.NavViewInfoButtonIcon = NavViewInfoButtonIcon;

        Shared.Main.NavigationItems.NavViewGeneralButtonLabel = NavViewGeneralButtonLabel;
        Shared.Main.NavigationItems.NavViewSettingsButtonLabel = NavViewSettingsButtonLabel;
        Shared.Main.NavigationItems.NavViewDevicesButtonLabel = NavViewDevicesButtonLabel;
        Shared.Main.NavigationItems.NavViewInfoButtonLabel = NavViewInfoButtonLabel;

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
            // Poor ass Windows 10 <1809
        {
            Logger.Warn("Time to get some updates for your archaic Windows install, man!");
            ExtendsContentIntoTitleBar = true; // Use the UWP titlebar extension method
            SetTitleBar(DragElement); // Use the UWP titlebar extension method: set drag zones
        }

        Logger.Info("Making the app dispatcher available for children views...");
        Shared.Main.DispatcherQueue = DispatcherQueue;

        try
        {
            Logger.Info("Registering for NotificationInvoked WinRT event...");
            if (!AppNotificationManager.IsSupported()) // Check for compatibility first
                throw new NotSupportedException("AppNotificationManager is not supported on this system!");

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
            Shared.Main.NotificationManager.Register(); // Try registering
        }
        catch (Exception e)
        {
            Logger.Error(e); // We couldn't set the manager up, sorry...
            Logger.Info("Resetting the notification manager...");
            Shared.Main.NotificationManager = null; // Not using it!
        }

        Logger.Info("Pushing control pages the global collection...");
        Shared.Main.Pages = new List<(string Tag, Type Page)>
        {
            ("general", typeof(General)),
            ("settings", typeof(Settings)),
            ("devices", typeof(Devices)),
            ("info", typeof(Info))
        };

        Logger.Info($"Setting up shared events for '{GetType().FullName}'...");
        RequestUpdateEvent += (_, _) => ExecuteUpdates();
        RequestUpdateFoundEvent += (_, _) =>
        {
            UpdateInfoBar.Message = string.Format(Interfacing.LocalizedJsonString(
                "/SharedStrings/Updates/NewUpdateMessage"), _remoteVersionString);

            UpdateInfoBar.IsOpen = true;
            UpdateInfoBar.Opacity = 1.0;
            return Task.CompletedTask;
        };

        Logger.Info("Registering a detached binary semaphore " +
                    $"reload handler for '{GetType().FullName}'...");

        Task.Run(() =>
        {
            Shared.Events.ReloadMainWindowEvent =
                new ManualResetEvent(false);

            while (true)
            {
                // Wait for a reload signal (blocking)
                Shared.Events.ReloadMainWindowEvent.WaitOne();

                // Reload & restart the waiting loop
                if (_mainPageLoadedOnce)
                    Shared.Main.DispatcherQueue.TryEnqueue(MainGrid_LoadedHandler);

                // Rebuild devices' settings
                // (Trick the device into rebuilding its interface)
                TrackingDevices.TrackingDevicesList.Values.ToList()
                    .ForEach(plugin => plugin.OnLoad());

                // Reset the event
                Shared.Events.ReloadMainWindowEvent.Reset();
            }
        });

        Logger.Info("Registering a named mutex for com_k2vr_amethyst...");
        try
        {
            Shared.Main.ApplicationMultiInstanceMutex = new Mutex(
                true, "com_k2vr_amethyst", out var needToCreateNew);

            if (!needToCreateNew)
            {
                Logger.Fatal(new AbandonedMutexException("Startup failed! The app is already running."));

                if (File.Exists(Path.Combine(Interfacing.GetProgramLocation().DirectoryName,
                        "K2CrashHandler", "K2CrashHandler.exe")))
                    Process.Start(Path.Combine(Interfacing.GetProgramLocation().DirectoryName,
                        "K2CrashHandler", "K2CrashHandler.exe"), "already_running");
                else
                    Logger.Warn("Crash handler exe (./K2CrashHandler/K2CrashHandler.exe) not found!");

                Task.Delay(3000);
                Environment.Exit(0); // Exit peacefully
            }
        }
        catch (Exception e)
        {
            Logger.Fatal(new AbandonedMutexException(
                $"Startup failed! Multi-instance lock mutex creation error: {e.Message}"));

            if (File.Exists(Path.Combine(Interfacing.GetProgramLocation().DirectoryName,
                    "K2CrashHandler", "K2CrashHandler.exe")))
                Process.Start(Path.Combine(Interfacing.GetProgramLocation().DirectoryName,
                    "K2CrashHandler", "K2CrashHandler.exe"), "already_running");
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
            Logger.Warn(
                $"Crash handler exe ({Path.Combine(Interfacing.GetProgramLocation().DirectoryName, "K2CrashHandler", "K2CrashHandler.exe")}) not found!");

        // Start the main loop
        Task.Run(Main.MainLoop);

        // Disable internal sounds
        ElementSoundPlayer.State = ElementSoundPlayerState.Off;

        // Create the plugin directory (if not existent)
        Directory.CreateDirectory(Path.Combine(
            Interfacing.GetProgramLocation().DirectoryName, "Plugins"));

        // Search the "Plugins" sub-directory for assemblies that match the imports.
        var catalog = new AggregateCatalog();

        // Add the current assembly to support invoke method exports
        //Logger.Info("Exporting the plugin host plugin...");
        //catalog.Catalogs.Add(new AssemblyCatalog(Assembly.GetExecutingAssembly()));

        // Iterate over all directories in .\Plugins dir and add all Plugin* dirs to catalogs
        var pluginDirectoryList = Directory.EnumerateDirectories(
            Path.Combine(Interfacing.GetProgramLocation().DirectoryName!, "Plugins"),
            "*", SearchOption.TopDirectoryOnly).ToList();

        AssemblyLoadContext.Default.LoadFromAssemblyPath(
            Assembly.GetAssembly(typeof(ITrackingDevice))!.Location);

        // Search for local plugins
        Logger.Info("Searching for local plugins now...");
        pluginDirectoryList.ForEach(pluginPath =>
            catalog.Catalogs.AddPlugin(new DirectoryInfo(pluginPath)));

        try
        {
            // Load the JSON source into buffer, parse
            Logger.Info("Searching for external plugins now...");
            var jsonRoot = JsonObject.Parse(
                File.ReadAllText(Interfacing.GetAppDataFileDir("amethystpaths.k2path")));

            // Loop over all the external plugins and append
            if (jsonRoot.ContainsKey("external_plugins"))
            {
                // Try loading all path-valid plugin entries
                jsonRoot.GetNamedArray("external_plugins").ToList()
                    .Where(pluginEntry => Directory.Exists(pluginEntry.GetString())).ToList()
                    .ForEach(pluginPath => catalog.Catalogs.AddPlugin(new DirectoryInfo(pluginPath.GetString())));

                // Write out all invalid ones
                jsonRoot.GetNamedArray("external_plugins").ToList()
                    .Where(pluginEntry => !Directory.Exists(pluginEntry.GetString())).ToList()
                    .ForEach(pluginPath =>
                    {
                        // Add the plugin to the 'attempted' list
                        TrackingDevices.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
                        {
                            Name = pluginPath.GetString(),
                            Status = TrackingDevices.PluginLoadError.NoPluginFolder
                        });

                        Logger.Error(new DirectoryNotFoundException(
                            $"Plugin hint directory \"{pluginPath.GetString()}\" doesn't exist!"));
                    });
            }

            else
            {
                Logger.Info("No external plugins found! Loading the local ones now...");
            }
        }
        catch (FileNotFoundException e)
        {
            Logger.Error($"Checking for external plugins has failed, an exception occurred. Message: {e.Message}");
            Logger.Error($"Creating new {Interfacing.GetAppDataFileDir("amethystpaths.k2path")} config...");
            File.Create(Interfacing.GetAppDataFileDir("amethystpaths.k2path")); // Create a placeholder file
        }
        catch (Exception e)
        {
            Logger.Error($"Checking for external plugins has failed, an exception occurred. Message: {e.Message}");
        }

        if (pluginDirectoryList.Count < 1)
        {
            Logger.Fatal(new CompositionException("No plugins directories found! Shutting down..."));
            Interfacing.Fail(Interfacing.LocalizedJsonString("/CrashHandler/Content/Crash/NoPlugins"));
        }

        Logger.Info($"Found {pluginDirectoryList.Count} potentially valid plugin directories...");
        try
        {
            // Loop over device catalog parts and compose
            foreach (var pluginCatalog in catalog.Catalogs)
            {
                var pCatalog = new AggregateCatalog();
                pCatalog.Catalogs.Add(pluginCatalog);

                using var container = new CompositionContainer(pCatalog,
                    CompositionOptions.DisableSilentRejection);

                Logger.Info($"Searching for tracking devices (providers) plugins within {pluginCatalog}...");
                var devicePlugins = container.GetExports<ITrackingDevice, IPluginMetadata>().ToList();

                Logger.Info($"Found {devicePlugins.Count} potentially valid exported plugin parts...");
                if (devicePlugins.Count <= 0)
                {
                    Logger.Info("Skipping this catalog part as it doesn't contain any usable plugins!");
                    continue; // Skip composing this plugin, it won't do any good!
                }

                var plugin = devicePlugins.First();
                Logger.Info($"Preparing exports for ({plugin.Metadata.Name}, {plugin.Metadata.Guid})...");

                try
                {
                    AmethystPluginHost = new PluginHost(plugin.Metadata.Guid);
                    container.ComposeParts(this);

                    Logger.Info($"Parsing ({plugin.Metadata.Name}, {plugin.Metadata.Guid})...");

                    // Check the plugin GUID against others loaded, INVALID and null
                    if (string.IsNullOrEmpty(plugin.Metadata.Guid) || plugin.Metadata.Guid == "INVALID" ||
                        TrackingDevices.TrackingDevicesList.ContainsKey(plugin.Metadata.Guid))
                    {
                        // Add the device to the 'attempted' list, mark as duplicate
                        TrackingDevices.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
                        {
                            Name = plugin.Metadata.Name,
                            Guid = plugin.Metadata.Guid,
                            PluginType = TrackingDevices.PluginType.TrackingDevice,
                            Publisher = plugin.Metadata.Publisher,
                            Website = plugin.Metadata.Website,
                            Status = TrackingDevices.PluginLoadError.BadOrDuplicateGuid
                        });

                        Logger.Error(new DuplicateNameException($"({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                                                "has a duplicate GUID value to another plugin, discarding it!"));
                        continue; // Give up on this one :(
                    }

                    // Check the plugin GUID against the ones we need to skip
                    if (AppData.Settings.DisabledPluginsGuidSet.Contains(plugin.Metadata.Guid))
                    {
                        // Add the device to the 'attempted' list, mark as duplicate
                        TrackingDevices.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
                        {
                            Name = plugin.Metadata.Name,
                            Guid = plugin.Metadata.Guid,
                            PluginType = TrackingDevices.PluginType.TrackingDevice,
                            Publisher = plugin.Metadata.Publisher,
                            Website = plugin.Metadata.Website,
                            Status = TrackingDevices.PluginLoadError.LoadingSkipped
                        });

                        Logger.Error($"({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                     "is requested to be skipped (via GUID), discarding it!");
                        continue; // Give up on this one :(
                    }

                    try
                    {
                        Logger.Info($"Trying to load ({plugin.Metadata.Name}, {plugin.Metadata.Guid})...");
                        Logger.Info($"Result: {plugin.Value}"); // Load the plugin
                    }
                    catch (CompositionException e)
                    {
                        Crashes.TrackError(e); // Composition exception
                        Logger.Error($"Loading plugin ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                     "failed with a local (plugin-wise) MEF exception: " +
                                     $"Message: {e.Message}\nErrors occurred: {e.Errors}\n" +
                                     $"Possible causes: {e.RootCauses}\nTrace: {e.StackTrace}");

                        // Add the device to the 'attempted' list, mark as possibly missing deps
                        TrackingDevices.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
                        {
                            Name = plugin.Metadata.Name,
                            Guid = plugin.Metadata.Guid,
                            PluginType = TrackingDevices.PluginType.TrackingDevice,
                            Publisher = plugin.Metadata.Publisher,
                            Website = plugin.Metadata.Website,
                            Error = e.UnwrapCompositionException().Message,
                            Status = TrackingDevices.PluginLoadError.NoPluginDependencyDll
                        });
                        continue; // Give up on this one :(
                    }
                    catch (Exception e)
                    {
                        Crashes.TrackError(e); // Other exception
                        Logger.Error($"Loading plugin ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                     "failed with an exception, probably some of its dependencies are missing. " +
                                     $"Message: {e.Message}, Trace: {e.StackTrace}");

                        // Add the device to the 'attempted' list, mark as unknown
                        TrackingDevices.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
                        {
                            Name = plugin.Metadata.Name,
                            Guid = plugin.Metadata.Guid,
                            PluginType = TrackingDevices.PluginType.TrackingDevice,
                            Publisher = plugin.Metadata.Publisher,
                            Website = plugin.Metadata.Website,
                            Error = e.Message,
                            Status = TrackingDevices.PluginLoadError.Other
                        });
                        continue; // Give up on this one :(
                    }

                    // It must be good if we're somehow still here
                    var pluginLocation = Assembly.GetAssembly(plugin.Value.GetType()).Location;
                    var pluginFolder = Directory.GetParent(pluginLocation).FullName;

                    // Add the device to the 'attempted' list, mark as all fine
                    Logger.Info($"Adding ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                "to the load-attempted device plugins list (TrackingDevices)...");
                    TrackingDevices.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
                    {
                        Name = plugin.Metadata.Name,
                        Guid = plugin.Metadata.Guid,
                        PluginType = TrackingDevices.PluginType.TrackingDevice,
                        Publisher = plugin.Metadata.Publisher,
                        Website = plugin.Metadata.Website,
                        Folder = pluginFolder,
                        Status = TrackingDevices.PluginLoadError.NoError
                    });

                    // Add the device to the global device list, add the plugin folder path
                    Logger.Info($"Adding ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                "to the global tracking device plugins list (TrackingDevices)...");
                    TrackingDevices.TrackingDevicesList.Add(plugin.Metadata.Guid, new TrackingDevice(
                        plugin.Metadata.Name, plugin.Metadata.Guid, pluginLocation, plugin.Value)
                    {
                        LocalizationResourcesRoot = (new JsonObject(), Path.Join(pluginFolder, "Assets", "Strings"))
                    });

                    // Set the device's string resources root to its provided folder
                    // (If it wants to change it, it's gonna need to do that after OnLoad anyway)
                    Logger.Info($"Registering ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                "default root language resource context (TrackingDevices)...");
                    Interfacing.Plugins.SetLocalizationResourcesRoot(
                        Path.Join(pluginFolder, "Assets", "Strings"), plugin.Metadata.Guid);

                    Logger.Info($"Telling ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                "this it's just been loaded into the core app domain...");
                    plugin.Value.OnLoad(); // Call the OnLoad handler for the first time

                    Logger.Info($"Device ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) \n" +
                                $"with the device class library dll at: {pluginLocation}\n" +
                                $"provides discoverable properties:\nStatus: {plugin.Value.DeviceStatus}\n" +
                                $"- Status String: {plugin.Value.DeviceStatusString}\n" +
                                $"- Supports Flip: {plugin.Value.IsFlipSupported}\n" +
                                $"- Initialized: {plugin.Value.IsInitialized}\n" +
                                $"- Overrides Physics: {plugin.Value.IsPhysicsOverrideEnabled}\n" +
                                $"- Blocks Filtering: {plugin.Value.IsPositionFilterBlockingEnabled}\n" +
                                $"- Updates By Itself: {plugin.Value.IsSelfUpdateEnabled}\n" +
                                $"- Supports Settings: {plugin.Value.IsSettingsDaemonSupported}\n" +
                                $"- Provides Prepended Joints: {plugin.Value.TrackedJoints.Count}");

                    // Check if the loaded device is used as anything (AppData.Settings)
                    Logger.Info($"Checking if ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) has any roles set...");
                    if (TrackingDevices.IsBase(plugin.Metadata.Guid))
                        Logger.Info($"({plugin.Metadata.Name}, {plugin.Metadata.Guid}) is a Base device!");
                    else if (TrackingDevices.IsOverride(plugin.Metadata.Guid))
                        Logger.Info($"({plugin.Metadata.Name}, {plugin.Metadata.Guid}) is an Override device!");
                    else
                        Logger.Info($"({plugin.Metadata.Name}, {plugin.Metadata.Guid}) does not serve any purpose :/");
                }
                catch (Exception e)
                {
                    // Add the device to the 'attempted' list, mark as unknown
                    Logger.Error($"Loading plugin ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                 "failed with a global outer caught exception. " +
                                 $"Provided exception Message: {e.Message}, Trace: {e.StackTrace}");

                    TrackingDevices.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
                    {
                        Name = plugin.Metadata.Name,
                        Guid = plugin.Metadata.Guid,
                        PluginType = TrackingDevices.PluginType.TrackingDevice,
                        Publisher = plugin.Metadata.Publisher,
                        Website = plugin.Metadata.Website,
                        Error = $"{e.Message}\n\n{e.StackTrace}",
                        Status = TrackingDevices.PluginLoadError.Other
                    });
                }
            }

            // Check if we have enough plugins to run the app
            if (TrackingDevices.TrackingDevicesList.Count < 1)
            {
                Logger.Fatal(new CompositionException("No plugins (tracking devices) loaded! Shutting down..."));
                Interfacing.Fail(Interfacing.LocalizedJsonString("/CrashHandler/Content/Crash/NoDevices"));
            }

            Logger.Info("Registration of tracking device plugins has ended, there are " +
                        $"[{TrackingDevices.TrackingDevicesList.Count}] valid plugins in total.");

            TrackingDevices.TrackingDevicesList.ToList().ForEach(x => Logger.Info(
                $"Loaded a valid tracking provider ({{{x.Key}}}, \"{x.Value.Name}\")"));

            // Loop over service catalog parts and compose
            foreach (var pluginCatalog in catalog.Catalogs)
            {
                var pCatalog = new AggregateCatalog();
                pCatalog.Catalogs.Add(pluginCatalog);

                using var container = new CompositionContainer(pCatalog,
                    CompositionOptions.DisableSilentRejection);

                Logger.Info($"Searching for tracking services (endpoint) plugins within {pluginCatalog}...");
                var devicePlugins = container.GetExports<IServiceEndpoint, IPluginMetadata>().ToList();

                Logger.Info($"Found {devicePlugins.Count} potentially valid exported plugin parts...");
                if (devicePlugins.Count <= 0)
                {
                    Logger.Info("Skipping this catalog part as it doesn't contain any usable plugins!");
                    continue; // Skip composing this plugin, it won't do any good!
                }

                var plugin = devicePlugins.First();
                Logger.Info($"Preparing exports for ({plugin.Metadata.Name}, {plugin.Metadata.Guid})...");

                try
                {
                    AmethystPluginHost = new PluginHost(plugin.Metadata.Guid);
                    container.ComposeParts(this);

                    Logger.Info($"Parsing ({plugin.Metadata.Name}, {plugin.Metadata.Guid})...");

                    // Check the plugin GUID against others loaded, INVALID and null
                    if (string.IsNullOrEmpty(plugin.Metadata.Guid) || plugin.Metadata.Guid == "INVALID" ||
                        TrackingDevices.ServiceEndpointsList.ContainsKey(plugin.Metadata.Guid))
                    {
                        // Add the device to the 'attempted' list, mark as duplicate
                        TrackingDevices.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
                        {
                            Name = plugin.Metadata.Name,
                            Guid = plugin.Metadata.Guid,
                            PluginType = TrackingDevices.PluginType.ServiceEndpoint,
                            Publisher = plugin.Metadata.Publisher,
                            Website = plugin.Metadata.Website,
                            Status = TrackingDevices.PluginLoadError.BadOrDuplicateGuid
                        });

                        Logger.Error(new DuplicateNameException($"({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                                                "has a duplicate GUID value to another plugin, discarding it!"));
                        continue; // Give up on this one :(
                    }

                    // Check the plugin GUID against the ones we need to skip
                    if (AppData.Settings.DisabledPluginsGuidSet.Contains(plugin.Metadata.Guid))
                    {
                        // Add the device to the 'attempted' list, mark as duplicate
                        TrackingDevices.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
                        {
                            Name = plugin.Metadata.Name,
                            Guid = plugin.Metadata.Guid,
                            PluginType = TrackingDevices.PluginType.ServiceEndpoint,
                            Publisher = plugin.Metadata.Publisher,
                            Website = plugin.Metadata.Website,
                            Status = TrackingDevices.PluginLoadError.LoadingSkipped
                        });

                        Logger.Error($"({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                     "is requested to be skipped (via GUID), discarding it!");
                        continue; // Give up on this one :(
                    }

                    try
                    {
                        Logger.Info($"Trying to load ({plugin.Metadata.Name}, {plugin.Metadata.Guid})...");
                        Logger.Info($"Result: {plugin.Value}"); // Load the plugin
                    }
                    catch (CompositionException e)
                    {
                        Crashes.TrackError(e); // Composition exception
                        Logger.Error($"Loading plugin ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                     "failed with a local (plugin-wise) MEF exception: " +
                                     $"Message: {e.Message}\nErrors occurred: {e.Errors}\n" +
                                     $"Possible causes: {e.RootCauses}\nTrace: {e.StackTrace}");

                        // Add the device to the 'attempted' list, mark as possibly missing deps
                        TrackingDevices.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
                        {
                            Name = plugin.Metadata.Name,
                            Guid = plugin.Metadata.Guid,
                            PluginType = TrackingDevices.PluginType.ServiceEndpoint,
                            Publisher = plugin.Metadata.Publisher,
                            Website = plugin.Metadata.Website,
                            Error = e.UnwrapCompositionException().Message,
                            Status = TrackingDevices.PluginLoadError.NoPluginDependencyDll
                        });
                        continue; // Give up on this one :(
                    }
                    catch (Exception e)
                    {
                        Crashes.TrackError(e); // Other exception
                        Logger.Error($"Loading plugin ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                     "failed with an exception, probably some of its dependencies are missing. " +
                                     $"Message: {e.Message}, Trace: {e.StackTrace}");

                        // Add the device to the 'attempted' list, mark as unknown
                        TrackingDevices.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
                        {
                            Name = plugin.Metadata.Name,
                            Guid = plugin.Metadata.Guid,
                            PluginType = TrackingDevices.PluginType.ServiceEndpoint,
                            Publisher = plugin.Metadata.Publisher,
                            Website = plugin.Metadata.Website,
                            Error = e.Message,
                            Status = TrackingDevices.PluginLoadError.Other
                        });
                        continue; // Give up on this one :(
                    }

                    // It must be good if we're somehow still here
                    var pluginLocation = Assembly.GetAssembly(plugin.Value.GetType()).Location;
                    var pluginFolder = Directory.GetParent(pluginLocation).FullName;

                    // Add the device to the global device list, add the plugin folder path
                    Logger.Info($"Adding ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                "to the global service endpoints plugins list (TrackingDevices)...");
                    TrackingDevices.ServiceEndpointsList.Add(plugin.Metadata.Guid, new ServiceEndpoint(
                        plugin.Metadata.Name, plugin.Metadata.Guid, pluginLocation, plugin.Value)
                    {
                        LocalizationResourcesRoot = (new JsonObject(), Path.Join(pluginFolder, "Assets", "Strings"))
                    });

                    // Set the device's string resources root to its provided folder
                    // (If it wants to change it, it's gonna need to do that after OnLoad anyway)
                    Logger.Info($"Registering ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                "default root language resource context (TrackingDevices)...");
                    Interfacing.Plugins.SetLocalizationResourcesRoot(
                        Path.Join(pluginFolder, "Assets", "Strings"), plugin.Metadata.Guid);

                    Logger.Info($"Telling ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                "this it's just been loaded into the core app domain...");
                    plugin.Value.OnLoad(); // Call the OnLoad handler for the first time

                    Logger.Info($"Service ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) \n" +
                                $"with the device class library dll at: {pluginLocation}\n" +
                                $"provides discoverable properties:\nStatus: {plugin.Value.ServiceStatus}\n" +
                                $"- Status String: {plugin.Value.ServiceStatusString}\n" +
                                $"- Supports manual calibration: {plugin.Value.ControllerInputActions != null}\n" +
                                $"- Supports automatic calibration: {plugin.Value.HeadsetPose != null}\n" +
                                $"- Needs to be restarted on changed: {plugin.Value.IsRestartOnChangesNeeded}\n" +
                                $"- Supports Settings: {plugin.Value.IsSettingsDaemonSupported}\n" +
                                $"- Additional Supported Trackers: {plugin.Value.AdditionalSupportedTrackerTypes.ToList()}");

                    // Add the device to the 'attempted' list, mark as all fine
                    Logger.Info($"Adding ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                "to the load-attempted device plugins list (TrackingDevices)...");
                    TrackingDevices.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
                    {
                        Name = plugin.Metadata.Name,
                        Guid = plugin.Metadata.Guid,
                        PluginType = TrackingDevices.PluginType.ServiceEndpoint,
                        Publisher = plugin.Metadata.Publisher,
                        Website = plugin.Metadata.Website,
                        Folder = pluginFolder,
                        Status = TrackingDevices.PluginLoadError.NoError
                    });

                    // Check if the loaded device is used as anything (AppData.Settings)
                    Logger.Info($"Checking if ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) has any roles set...");
                    Logger.Info($"({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                $"{(AppData.Settings.ServiceEndpointGuid == plugin.Metadata.Guid ? "is the selected service!" : "does not serve any purpose :/")}");

                    // Check and use service's provided [freeze] action handlers
                    if (AppData.Settings.ServiceEndpointGuid == plugin.Metadata.Guid &&
                        plugin.Value.ControllerInputActions?.TrackingFreezeToggled is not null)
                        plugin.Value.ControllerInputActions.TrackingFreezeToggled += Main.FreezeActionToggled;

                    // Check and use service's provided [flip] action handlers
                    if (AppData.Settings.ServiceEndpointGuid == plugin.Metadata.Guid &&
                        plugin.Value.ControllerInputActions?.SkeletonFlipToggled is not null)
                        plugin.Value.ControllerInputActions.SkeletonFlipToggled += Main.FlipActionToggled;
                }
                catch (Exception e)
                {
                    // Add the device to the 'attempted' list, mark as unknown
                    Crashes.TrackError(e); // Other, outer MEF container exception
                    Logger.Error($"Loading plugin ({plugin.Metadata.Name}, {plugin.Metadata.Guid}) " +
                                 "failed with a global outer caught exception. " +
                                 $"Provided exception Message: {e.Message}, Trace: {e.StackTrace}");

                    TrackingDevices.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
                    {
                        Name = plugin.Metadata.Name,
                        Guid = plugin.Metadata.Guid,
                        PluginType = TrackingDevices.PluginType.ServiceEndpoint,
                        Publisher = plugin.Metadata.Publisher,
                        Website = plugin.Metadata.Website,
                        Error = $"{e.Message}\n\n{e.StackTrace}",
                        Status = TrackingDevices.PluginLoadError.Other
                    });
                }
            }

            // Check if we have enough plugins to run the app
            if (TrackingDevices.ServiceEndpointsList.Count < 1)
            {
                Logger.Fatal(new CompositionException("No plugins (service endpoints) loaded! Shutting down..."));
                Interfacing.Fail(Interfacing.LocalizedJsonString("/CrashHandler/Content/Crash/NoServices"));
            }

            Logger.Info("Registration of service endpoint plugins has ended, there are " +
                        $"[{TrackingDevices.ServiceEndpointsList.Count}] valid plugins in total.");

            TrackingDevices.ServiceEndpointsList.ToList().ForEach(x => Logger.Info(
                $"Loaded a valid service endpoint ({{{x.Key}}}, \"{x.Value.Name}\")"));
        }
        catch (CompositionException e)
        {
            Crashes.TrackError(e); // Other, outer MEF builder exception
            Logger.Error("Loading plugins failed with a global MEF exception: " +
                         $"Message: {e.Message}\nErrors occurred: {e.Errors}\n" +
                         $"Possible causes: {e.RootCauses}\nTrace: {e.StackTrace}");
        }

        // Try reading the default config
        Logger.Info("Checking out the default configuration settings...");
        (string DeviceGuid, string ServiceGuid) defaultSettings = (null, null); // Invalid!

        if (File.Exists(Path.Join(Interfacing.GetProgramLocation().DirectoryName, "defaults.json")))
            try
            {
                // Parse the loaded json
                var jsonHead = JsonObject.Parse(File.ReadAllText(
                    Path.Join(Interfacing.GetProgramLocation().DirectoryName, "defaults.json")));

                // Check the device guid
                if (!jsonHead.ContainsKey("TrackingDevice"))
                    // Invalid configuration file, don't proceed further!
                    Logger.Error("The default configuration json file was (partially) invalid!");
                else
                    defaultSettings = (
                        jsonHead.GetNamedString("TrackingDevice"),
                        defaultSettings.ServiceGuid); // Keep the last one

                // Check the service guid
                if (!jsonHead.ContainsKey("ServiceEndpoint"))
                    // Invalid configuration file, don't proceed further!
                    Logger.Error("The default configuration json file was (partially) invalid!");
                else
                    defaultSettings = (
                        defaultSettings.DeviceGuid, // Keep the last one
                        jsonHead.GetNamedString("ServiceEndpoint"));
            }
            catch (Exception e)
            {
                Logger.Info($"Default settings checkout failed! Message: {e.Message}");
            }
        else Logger.Info("No default configuration found! [defaults.json]");

        // Validate the saved base plugin guid
        Logger.Info("Checking if the saved base device exists in loaded plugins...");
        if (!TrackingDevices.TrackingDevicesList.ContainsKey(AppData.Settings.TrackingDeviceGuid))
        {
            // Check against the defaults
            var firstValidDevice = string.IsNullOrEmpty(defaultSettings.DeviceGuid)
                ? null // Default to null for an empty default guid passed
                : TrackingDevices.GetDevice(defaultSettings.DeviceGuid).Device;

            // Check the device now
            if (firstValidDevice is null || firstValidDevice.TrackedJoints.Count <= 0)
            {
                Logger.Warn($"The requested default tracking device ({defaultSettings.DeviceGuid}) " +
                            "was invalid! Searching for any non-disabled suitable device now...");

                // Find the first device that provides any joints
                firstValidDevice = TrackingDevices.TrackingDevicesList.Values
                    .FirstOrDefault(device => device.TrackedJoints.Count > 0, null);
            }

            // Check if we've found such a device
            if (firstValidDevice is null)
            {
                Logger.Fatal(new CompositionException("No plugins valid (tracking devices) which provide " +
                                                      "at lest 1 tracked joint have been found! Shutting down..."));
                Interfacing.Fail(Interfacing.LocalizedJsonString("/CrashHandler/Content/Crash/NoJoints"));
            }

            Logger.Info($"The saved base device ({AppData.Settings.TrackingDeviceGuid}) is invalid! " +
                        $"Resetting it to the first valid tracking device: ({firstValidDevice.Guid})!");
            AppData.Settings.TrackingDeviceGuid = firstValidDevice.Guid;
        }

        Logger.Info("Updating app settings for the selected base device...");

        // Initialize the loaded base device now
        Logger.Info("Initializing the selected base device...");
        TrackingDevices.BaseTrackingDevice.Initialize();

        // Validate and initialize the loaded override devices now
        Logger.Info("Checking if saved override devices exist in loaded plugins...");
        AppData.Settings.OverrideDevicesGuidMap.RemoveWhere(overrideGuid =>
        {
            Logger.Info($"Checking if override ({overrideGuid}) exists in loaded plugins...");
            if (!TrackingDevices.TrackingDevicesList.ContainsKey(overrideGuid))
            {
                // This override guid is invalid or missing
                Logger.Info($"The saved override device ({overrideGuid}) is invalid! Resetting it to NONE!");
                return true; // Remove this invalid override!
            }

            Logger.Info($"Checking if override ({overrideGuid}) doesn't derive from the base device...");
            if (AppData.Settings.TrackingDeviceGuid == overrideGuid)
            {
                // Already being used as the base device
                Logger.Info($"({overrideGuid}) is already set as Base! Resetting it to NONE!");
                return true; // Remove this invalid override!
            }

            // Still here? We must be fine then, initialize the device
            Logger.Info($"Initializing the selected override device ({overrideGuid})...");
            TrackingDevices.GetDevice(overrideGuid).Device.Initialize();
            return false; // This override is OK, let's keep it
        });

        foreach (var overrideGuid in AppData.Settings.OverrideDevicesGuidMap)
        {
            Logger.Info($"Checking if override ({overrideGuid}) exists in loaded plugins...");
            if (!TrackingDevices.TrackingDevicesList.ContainsKey(overrideGuid))
            {
                // This override guid is invalid or missing
                Logger.Info($"The saved override device ({overrideGuid}) is invalid! Resetting it to NONE!");
                AppData.Settings.OverrideDevicesGuidMap.Remove(overrideGuid);
                continue; // Skip this one then...
            }

            Logger.Info($"Checking if override ({overrideGuid}) doesn't derive from the base device...");
            if (AppData.Settings.TrackingDeviceGuid == overrideGuid)
            {
                // Already being used as the base device
                Logger.Info($"({overrideGuid}) is already set as Base! Resetting it to NONE!");
                AppData.Settings.OverrideDevicesGuidMap.Remove(overrideGuid);
                continue; // Skip this one then...
            }

            // Still here? We must be fine then, initialize the device
            Logger.Info($"Initializing the selected override device ({overrideGuid})...");
            TrackingDevices.GetDevice(overrideGuid).Device.Initialize();
        }

        // Validate the saved service plugin guid
        Logger.Info("Checking if the saved service endpoint exists in loaded plugins...");
        if (!TrackingDevices.ServiceEndpointsList.ContainsKey(AppData.Settings.ServiceEndpointGuid))
        {
            if (!string.IsNullOrEmpty(defaultSettings.ServiceGuid) && // Check the guid first
                TrackingDevices.ServiceEndpointsList.ContainsKey(defaultSettings.ServiceGuid))
            {
                Logger.Info($"The selected service endpoint ({AppData.Settings.ServiceEndpointGuid}) is invalid! " +
                            $"Resetting it to the default one selected in defaults: ({defaultSettings.ServiceGuid})!");
                AppData.Settings.ServiceEndpointGuid = defaultSettings.ServiceGuid;
            }
            else
            {
                Logger.Info($"The default service endpoint ({AppData.Settings.ServiceEndpointGuid}) is invalid! " +
                            $"Resetting it to the first one: ({TrackingDevices.ServiceEndpointsList.First().Key})!");
                AppData.Settings.ServiceEndpointGuid = TrackingDevices.ServiceEndpointsList.First().Key;
            }
        }

        // Priority: Connect to the tracking service
        Logger.Info("Initializing the selected service endpoint...");
        TrackingDevices.CurrentServiceEndpoint.Initialize();

        Logger.Info("Checking the selected service endpoint...");
        Interfacing.ServiceEndpointSetup();

        Logger.Info("Checking application settings config for loaded plugins...");
        AppData.Settings.CheckSettings();

        // Second check and try after 3 seconds
        Task.Run(() =>
        {
            // The Base device
            if (!TrackingDevices.BaseTrackingDevice.IsInitialized)
                TrackingDevices.BaseTrackingDevice.Initialize();

            // All valid override devices
            AppData.Settings.OverrideDevicesGuidMap
                .Where(x => !TrackingDevices.GetDevice(x).Device.IsInitialized).ToList()
                .ForEach(device => TrackingDevices.GetDevice(device).Device.Initialize());
        });

        // Update the UI
        TrackingDevices.UpdateTrackingDevicesInterface();

        // Log our used device to the telemetry module
        Analytics.TrackEvent("TrackingDevice", new Dictionary<string, string>
            { { "Guid", AppData.Settings.TrackingDeviceGuid } });

        // Log our used service to the telemetry module
        Analytics.TrackEvent("ServiceEndpoint", new Dictionary<string, string>
            { { "Guid", AppData.Settings.ServiceEndpointGuid } });

        // Log our used overrides to the telemetry module
        Analytics.TrackEvent("OverrideDevices", new Dictionary<string, string>(
            AppData.Settings.OverrideDevicesGuidMap
                .Select(x => new KeyValuePair<string, string>("Guid", x))));

        // Setup device change watchdog : local devices
        var localWatcher = new FileSystemWatcher
        {
            Path = Path.Combine(Interfacing.GetProgramLocation().DirectoryName, "Plugins"),
            NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.DirectoryName,
            Filter = "*.dll", IncludeSubdirectories = true, EnableRaisingEvents = true
        };

        // Setup device change watchdog : external devices
        var externalWatcher = new FileSystemWatcher
        {
            Path = Path.Join(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "Amethyst"),
            NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.DirectoryName | NotifyFilters.Size,
            Filter = "amethystpaths.k2path", IncludeSubdirectories = true, EnableRaisingEvents = true
        };

        // Send a non-dismissible tip about reloading the app
        static void OnWatcherOnChanged(object o, FileSystemEventArgs fileSystemEventArgs)
        {
            Shared.Main.DispatcherQueue.TryEnqueue(() =>
            {
                Logger.Info("Device plugin tree has changed, you need to reload!");
                Logger.Info($"What happened: {fileSystemEventArgs.ChangeType}");
                Logger.Info($"Where: {fileSystemEventArgs.FullPath} ({fileSystemEventArgs.Name})");

                Shared.TeachingTips.MainPage.ReloadInfoBar.IsOpen = true;
                Shared.TeachingTips.MainPage.ReloadInfoBar.Opacity = 1.0;
            });
        }

        // Add event handlers : local
        localWatcher.Changed += OnWatcherOnChanged;
        localWatcher.Created += OnWatcherOnChanged;
        localWatcher.Deleted += OnWatcherOnChanged;
        localWatcher.Renamed += OnWatcherOnChanged;

        // Add event handlers : external
        externalWatcher.Changed += OnWatcherOnChanged;
        externalWatcher.Created += OnWatcherOnChanged;
        externalWatcher.Deleted += OnWatcherOnChanged;
        externalWatcher.Renamed += OnWatcherOnChanged;

        // Check settings once again and save
        AppData.Settings.CheckSettings();
        AppData.Settings.SaveSettings();

        // Notify of the setup end
        _mainPageInitFinished = true;
        Shared.Main.MainWindowLoaded = true;
    }

    [Export(typeof(IAmethystHost))] private IAmethystHost AmethystPluginHost { get; set; }

    // MVVM stuff
    public event PropertyChangedEventHandler PropertyChanged;

    private void MainGrid_Loaded(object sender, RoutedEventArgs e)
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
        MainGrid_LoadedHandler();

        // Register a theme watchdog
        NavView.XamlRoot.Content.As<Grid>().ActualThemeChanged += MainWindow_ActualThemeChanged;

        // Mark as loaded
        _mainPageLoadedOnce = true;
    }

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

        Application.Current.Resources["WindowCaptionForeground"] =
            Interfacing.ActualTheme == ElementTheme.Dark ? Colors.White : Colors.Black;

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
        Shared.Events.RequestInterfaceReload();

        OnPropertyChanged(); // Reload all
        ReloadNavigationIcons();
    }

    private void MainGrid_LoadedHandler()
    {
        OnPropertyChanged(); // All
        ReloadNavigationIcons();
    }

    private void ReloadNavigationIcons()
    {
        if (Interfacing.CurrentPageClass == "Amethyst.Pages.General")
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

        if (Interfacing.CurrentPageClass == "Amethyst.Pages.Settings")
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

        if (Interfacing.CurrentPageClass == "Amethyst.Pages.Devices")
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

        if (Interfacing.CurrentPageClass == "Amethyst.Pages.Info")
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

        HelpIcon.Foreground = Shared.Main.NeutralBrush;
    }

    private async Task ExecuteUpdates()
    {
        Interfacing.UpdatingNow = true;
        await Task.Delay(500);

        // Mark the update footer as active
        UpdatePendingInfoBar.Title = string.Format(Interfacing.LocalizedJsonString(
            "/SharedStrings/Updates/Headers/Downloading"), _remoteVersionString);

        if (!Interfacing.IsNuxPending)
        {
            UpdatePendingInfoBar.IsOpen = true;
            UpdatePendingInfoBar.Opacity = 1.0;
        }

        // Success? ...or nah?
        var updateError = false;

        // Reset the progressbar
        var updatePendingProgressBar = new ProgressBar
        {
            IsIndeterminate = true,
            HorizontalAlignment = HorizontalAlignment.Stretch
        };

        UpdatePendingFlyoutMainStack.Children.Add(updatePendingProgressBar);

        // Download the latest installer/updater
        // https://github.com/KinectToVR/Amethyst-Installer-Releases/releases/latest/download/Amethyst-Installer.exe

        try
        {
            using var client = new HttpClient();
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
                        Logger.Error(
                            new KeyNotFoundException("Installer-uri-check failed, the \"download\" key wasn't found."));
                        updateError = true;
                    }
                }
                else
                {
                    Logger.Error(new NoNullAllowedException("Installer-uri-check failed, the string was empty."));
                    updateError = true;
                }
            }
            catch (Exception e)
            {
                Logger.Error(new Exception($"Error checking the updater download Uri! Message: {e.Message}"));
                updateError = true;
            }


            // Download if we're ok
            if (!updateError)
                try
                {
                    var thisFolder = await StorageFolder
                        .GetFolderFromPathAsync(Interfacing.GetAppDataTempDir().FullName);

                    // To save downloaded image to local storage
                    var installerFile = await thisFolder.CreateFileAsync(
                        "Amethyst-Installer.exe", CreationCollisionOption.ReplaceExisting);

                    using var fsInstallerFile = await installerFile.OpenAsync(FileAccessMode.ReadWrite);
                    using var response = await client.GetAsync(new Uri(installerUri),
                        HttpCompletionOption.ResponseHeadersRead);

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

                        // Update the progress message
                        UpdatePendingInfoBar.Message = downloadStatusString.Replace(
                            "0", ((int)(100.0 * totalBytesRead / totalBytesToRead)).ToString());

                        // Write to file
                        await fsInstallerFile.WriteAsync(readBuffer);
                    }
                }
                catch (Exception e)
                {
                    Logger.Error(new Exception($"Error downloading the updater! Message: {e.Message}"));
                    updateError = true;
                }
        }
        catch (Exception e)
        {
            Logger.Error(new Exception($"Update failed, an exception occurred. Message: {e.Message}"));
            updateError = true;
        }

        // Check the file result and the DL result
        if (!updateError)
        {
            try
            {
                // Optional auto-restart scenario "-o" (happens when the user clicks 'update now')
                Process.Start(new ProcessStartInfo
                {
                    UseShellExecute = true, Verb = "runas",
                    FileName = Path.Combine(Interfacing.GetAppDataTempDir().FullName, "Amethyst-Installer.exe"),
                    Arguments =
                        $"--update{(Interfacing.UpdateOnClosed ? "" : " -o")} -path \"{Interfacing.GetProgramLocation().DirectoryName}\""
                });
            }
            catch (Win32Exception e)
            {
                if (e.NativeErrorCode == 1223)
                    Logger.Warn($"You need to pass UAC to run the installer! Message: {e.Message}");
                goto update_error; // Jump to the error scenario
            }
            catch (Exception e)
            {
                Logger.Error(new Exception($"Update failed, an exception occurred. Message: {e.Message}"));
                goto update_error; // Jump to the error scenario
            }

            // Exit, cleanup should be automatic
            Interfacing.UpdateOnClosed = false; // Don't re-do
            Environment.Exit(0); // Should get caught by the exit handler
        }

        // Jump-label
        update_error:

        // Still here? Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Error);

        updatePendingProgressBar.ShowError = true;
        UpdatePendingInfoBar.Message = Interfacing.LocalizedJsonString(
            "/SharedStrings/Updates/Statuses/Error");

        await Task.Delay(3200);
        UpdatePendingInfoBar.IsOpen = false;
        UpdatePendingInfoBar.Opacity = 0.0;
        await Task.Delay(500);

        // Don't give up yet
        Interfacing.UpdatingNow = false;
        if (Interfacing.UpdateFound)
        {
            UpdateInfoBar.IsOpen = true;
            UpdateInfoBar.Opacity = 1.0;
        }

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
                return; // Don't proceed further

            // Mark as checking
            Interfacing.CheckingUpdatesNow = true;
            await Task.Delay((int)delay);

            // Don't check if found
            if (!Interfacing.UpdateFound)
            {
                // Check now
                Interfacing.UpdateFound = false;

                // Check for updates
                try
                {
                    using var client = new HttpClient();
                    string getReleaseVersion = "", getDocsLanguages = "en";

                    // Release
                    try
                    {
                        Logger.Info("Checking for updates... [GET]");
                        using var response =
                            await client.GetAsync(new Uri($"https://api.k2vr.tech/v{AppData.ApiVersion}/update"));
                        getReleaseVersion = await response.Content.ReadAsStringAsync();
                    }
                    catch (Exception e)
                    {
                        Logger.Error(new Exception($"Error getting the release info! Message: {e.Message}"));
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
                        Logger.Error(new Exception($"Error getting the language info! Message: {e.Message}"));
                    }

                    // If the read string isn't empty, proceed to checking for updates
                    if (!string.IsNullOrEmpty(getReleaseVersion))
                    {
                        Logger.Info($"Update-check successful, string:\n{getReleaseVersion}");

                        // Parse the loaded json
                        var jsonHead = JsonObject.Parse(getReleaseVersion);

                        if (!jsonHead.ContainsKey("amethyst"))
                            Logger.Error(new InvalidDataException("The latest release's manifest was invalid!"));

                        // Parse the amethyst entry
                        var jsonRoot = jsonHead.GetNamedObject("amethyst");

                        if (!jsonRoot.ContainsKey("version") ||
                            !jsonRoot.ContainsKey("version_string"))
                        {
                            Logger.Error("The latest release's manifest was invalid!");
                        }

                        else
                        {
                            // Get the version tag (uint, fallback to latest)
                            var remoteVersion = (int)jsonRoot.GetNamedNumber("version", AppData.InternalVersion);

                            // Get the remote version name
                            _remoteVersionString = jsonRoot.GetNamedString("version_string");

                            Logger.Info($"Local version: {AppData.InternalVersion}");
                            Logger.Info($"Remote version: {remoteVersion}");

                            Logger.Info($"Local version string: {AppData.VersionString.Display}");
                            Logger.Info($"Remote version string: {_remoteVersionString}");

                            // Check the version
                            if (AppData.InternalVersion < remoteVersion)
                                Interfacing.UpdateFound = true;
                        }
                    }
                    else
                    {
                        Logger.Error(new NoNullAllowedException("Update-check failed, the string was empty."));
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
                        Logger.Error(new NoNullAllowedException("Language-check failed, the string was empty."));
                    }
                }
                catch (Exception e)
                {
                    Logger.Error($"Update failed, an exception occurred. Message: {e.Message}");
                }

                if (Interfacing.UpdateFound)
                {
                    UpdateInfoBar.Message = string.Format(Interfacing.LocalizedJsonString(
                        "/SharedStrings/Updates/NewUpdateMessage"), _remoteVersionString);

                    UpdateInfoBar.IsOpen = true;
                    UpdateInfoBar.Opacity = 1.0;
                }
            }

            // If an update was found, show it
            if ((Interfacing.UpdateFound || show) && !Interfacing.IsNuxPending)
            {
                UpdateInfoBar.IsOpen = true; // (or if the check was manual)
                UpdateInfoBar.Opacity = 1.0; // (or if the check was manual)
            }

            // Uncheck
            Interfacing.CheckingUpdatesNow = false;
        }
    }

    private void InitializerTeachingTip_ActionButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Dismiss the current tip
        Shared.TeachingTips.MainPage.InitializerTeachingTip.IsOpen = false;

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
        Shared.TeachingTips.MainPage.InitializerTeachingTip.IsOpen = false;

        // Navigate to the general page
        Shared.Main.MainNavigationView.SelectedItem =
            Shared.Main.MainNavigationView.MenuItems[0];

        Shared.Main.NavigateToPage("general",
            new EntranceNavigationTransitionInfo());

        // Show the next tip (general page)
        Shared.TeachingTips.GeneralPage.ToggleTrackersTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        Shared.TeachingTips.GeneralPage.ToggleTrackersTeachingTip.IsOpen = true;
    }

    private async void ReloadInfoBar_CloseButtonClick(object sender, RoutedEventArgs args)
    {
        Logger.Info("Reload has been invoked: turning trackers off...");

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Mark trackers as inactive
        Interfacing.AppTrackersInitialized = false;
        if (Shared.General.ToggleTrackersButton is not null)
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
            Process.Start(Interfacing.GetProgramLocation()
                .FullName.Replace(".dll", ".exe"));

            // Exit without re-handling everything
            Environment.Exit(0);
        }

        // Still here?
        Logger.Fatal(new InvalidDataException("App will not be restarted due to caller process identification error."));

        Interfacing.ShowToast(
            Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed/Title"),
            Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed"));

        Interfacing.ShowServiceToast(
            Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed/Title"),
            Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed"));
    }

    private async void NavView_Loaded(object sender, RoutedEventArgs e)
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

        // Show the startup tour teachingtip
        if (!AppData.Settings.FirstTimeTourShown)
        {
            // Play a sound
            AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

            // Show the first tip
            Shared.Main.InterfaceBlockerGrid.Opacity = 0.35;
            Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = true;

            Shared.TeachingTips.MainPage.InitializerTeachingTip.IsOpen = true;
            Interfacing.IsNuxPending = true;
        }

        // Check for updates (and show)
        await CheckUpdates(false, 2000);
    }

    private void NavView_ItemInvoked(NavigationView sender,
        NavigationViewItemInvokedEventArgs args)
    {
        Shared.Main.NavigateToPage(
            args.InvokedItemContainer.Tag.ToString(),
            new EntranceNavigationTransitionInfo());
    }

    private void NavView_BackRequested(NavigationView sender,
        NavigationViewBackRequestedEventArgs args)
    {
        if (ContentFrame.CanGoBack && (!NavView.IsPaneOpen || NavView.DisplayMode is not
                (NavigationViewDisplayMode.Compact or NavigationViewDisplayMode.Minimal)))
            ContentFrame.GoBack();
    }

    //private async void UpdateButton_Tapped(object sender, TappedRoutedEventArgs e)
    //{
    //    // Check for updates (and show)
    //    if (Interfacing.CheckingUpdatesNow) return;
    //    AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
    //    await CheckUpdates(true);
    //}

    //private async void UpdateButton_Loaded(object sender, RoutedEventArgs e)
    //{
    //    // Show the startup tour teachingtip
    //    if (!AppData.Settings.FirstTimeTourShown)
    //    {
    //        // Play a sound
    //        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

    //        // Show the first tip
    //        Shared.Main.InterfaceBlockerGrid.Opacity = 0.35;
    //        Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = true;

    //        Shared.TeachingTips.MainPage.InitializerTeachingTip.IsOpen = true;
    //        Interfacing.IsNuxPending = true;
    //    }

    //    // Check for updates (and show)
    //    await CheckUpdates(false, 2000);
    //}

    private void HelpButton_Tapped(object sender, TappedRoutedEventArgs e)
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
        HelpFlyout.ShowAt(HelpButton, new FlyoutShowOptions
        {
            Placement = FlyoutPlacementMode.RightEdgeAlignedBottom,
            ShowMode = FlyoutShowMode.Transient
        });
    }

    private void ContentFrame_NavigationFailed(object sender, NavigationFailedEventArgs e)
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

    private void InstallLaterButton_Click(InfoBar sender, object arg)
    {
        Interfacing.UpdateOnClosed = true;
    }

    private async void InstallNowButton_Click(object sender, RoutedEventArgs e)
    {
        UpdateInfoBar.IsOpen = false;
        UpdateInfoBar.Opacity = 0.0;

        await ExecuteUpdates();
    }

    private async void HelpFlyoutDocsButton_Click(object sender, RoutedEventArgs e)
    {
        await Launcher.LaunchUriAsync(new Uri(Interfacing.CurrentAppState switch
        {
            "calibration" => $"https://docs.k2vr.tech/{Interfacing.DocsLanguageCode}/calibration/",
            "calibration_auto" => $"https://docs.k2vr.tech/{Interfacing.DocsLanguageCode}/calibration/#3",
            "calibration_manual" => $"https://docs.k2vr.tech/{Interfacing.DocsLanguageCode}/calibration/#6",
            "devices" or "offsets" or "settings" => $"https://docs.k2vr.tech/{Interfacing.DocsLanguageCode}/",
            "overrides" => $"https://docs.k2vr.tech/{Interfacing.DocsLanguageCode}/overrides/",
            "info" => "https://opencollective.com/k2vr",
            "general" or _ => $"https://docs.k2vr.tech/{Interfacing.DocsLanguageCode}/"
        }));
    }

    private async void HelpFlyoutDiscordButton_Click(object sender, RoutedEventArgs e)
    {
        await Launcher.LaunchUriAsync(new Uri("https://discord.gg/YBQCRDG"));
    }

    private async void HelpFlyoutDevButton_Click(object sender, RoutedEventArgs e)
    {
        await Launcher.LaunchUriAsync(new Uri("https://github.com/KinectToVR"));
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
        if (!MicaController.IsSupported())
        {
            Logger.Info("Mica is not supported! Time to update Windows, man!");
            return false; // Mica is not supported on this system
        }

        _wsdqHelper = new WindowsSystemDispatcherQueueHelper();
        _wsdqHelper.EnsureWindowsSystemDispatcherQueueController();

        // Hooking up the policy object
        _configurationSource = new SystemBackdropConfiguration();
        Activated += Window_Activated;
        Closed += Window_Closed;
        ((FrameworkElement)Content).ActualThemeChanged += Window_ThemeChanged;

        // Initial configuration state.
        _configurationSource.IsInputActive = true;
        SetConfigurationSourceTheme();

        _micaController = new MicaController();

        // Enable the system backdrop.
        // Note: Be sure to have "using WinRT;" to support the Window.As<...>() call.
        _micaController.AddSystemBackdropTarget(this
            .As<ICompositionSupportsSystemBackdrop>());
        _micaController.SetSystemBackdropConfiguration(_configurationSource);

        // Change the window background to support mica
        MainGrid.Background = new SolidColorBrush(Colors.Transparent);
        return true; // succeeded
    }

    private void Window_Activated(object sender, WindowActivatedEventArgs args)
    {
        _configurationSource.IsInputActive = args.WindowActivationState != WindowActivationState.Deactivated;
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
            await Interfacing.HandleAppExit(1000);

            // Make sure any Mica/Acrylic controller is disposed so it doesn't try to
            // use this closed window.
            if (_micaController is not null)
            {
                _micaController.Dispose();
                _micaController = null;
            }

            Activated -= Window_Activated;
            _configurationSource = null;
        }

        try
        {
            // Call before exiting for subsequent invocations to launch a new process
            Shared.Main.NotificationManager?.Unregister();
        }
        catch (Exception)
        {
            // ignored
        }

        // Finally allow exits
        args.Handled = false;
        Environment.Exit(0);
    }

    private void Window_ThemeChanged(FrameworkElement sender, object args)
    {
        if (_configurationSource is not null) SetConfigurationSourceTheme();
    }

    private void SetConfigurationSourceTheme()
    {
        _configurationSource.Theme = ((FrameworkElement)Content).ActualTheme switch
        {
            ElementTheme.Dark => SystemBackdropTheme.Dark,
            ElementTheme.Light => SystemBackdropTheme.Light,
            ElementTheme.Default => SystemBackdropTheme.Default,
            _ => _configurationSource.Theme
        };
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    private void HelpFlyout_Opening(object sender, object e)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);

        HelpIcon.Foreground = Shared.Main.AttentionBrush;
        HelpIconGrid.Translation = Vector3.Zero;
        HelpIconText.Opacity = 0.0;
    }

    private void HelpFlyout_Closing(FlyoutBase sender, FlyoutBaseClosingEventArgs args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);

        HelpIcon.Foreground = Shared.Main.NeutralBrush;
        HelpIconGrid.Translation = new Vector3(0, -8, 0);
        HelpIconText.Opacity = 1.0;
    }

    private void InterfaceBlockerGrid_DoubleTapped(object sender, DoubleTappedRoutedEventArgs e)
    {
        if (!InitializerTeachingTip.IsOpen) return;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Focus);

        // Dismiss the current tip
        Shared.TeachingTips.MainPage.InitializerTeachingTip.IsOpen = false;

        // Just dismiss the tip
        Shared.Main.InterfaceBlockerGrid.Opacity = 0.0;
        Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = false;
        Interfacing.IsNuxPending = false;

        // We're done
        AppData.Settings.FirstTimeTourShown = true;
        AppData.Settings.SaveSettings();
    }
}