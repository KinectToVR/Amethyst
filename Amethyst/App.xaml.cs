// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.IO;
using Windows.Foundation;
using Windows.UI.ViewManagement;
using Amethyst.Classes;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using System.Linq;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst;

/// <summary>
///     Provides application-specific behavior to supplement the default Application class.
/// </summary>
public partial class App : Application
{
    private Window _mWindow;

    /// <summary>
    ///     Initializes the singleton application object.  This is the first line of authored code
    ///     executed, and as such is the logical equivalent of main() or WinMain().
    /// </summary>
    public App()
    {
        InitializeComponent();

        // Set default window launch size (WinUI)
        ApplicationView.PreferredLaunchViewSize = new Size(1000, 700);
        ApplicationView.PreferredLaunchWindowingMode = ApplicationViewWindowingMode.PreferredLaunchViewSize;

        // Initialize the logger
        Logger.Init(Interfacing.GetK2AppDataLogFileDir("Amethyst",
            $"Amethyst_{DateTime.Now:yyyyMMdd-HHmmss.ffffff}.log"));

        // Create an empty file for checking for crashes
        Interfacing.CrashFile = new FileInfo(Path.Join(Interfacing.GetProgramLocation().DirectoryName, ".crash"));
        Interfacing.CrashFile.Create(); // Create the file

        try
        {
            // Try deleting the "latest" log file
            File.Delete(Interfacing.GetK2AppDataLogFileDir("Amethyst", "_latest.log"));
        }
        catch (Exception)
        {
            // ignored
        }

        // Log status information
        Logger.Info($".NET CLR Runtime version: {typeof(string).Assembly.ImageRuntimeVersion}");
        Logger.Info($"Amethyst version: (string) {AppData.K2InternalVersion}");
        Logger.Info($"Amethyst version: (value) {AppData.K2IntVersion}");
        Logger.Info($"Amethyst web API version: {AppData.K2ApiVersion}");

        // Read saved settings
        AppData.Settings.ReadSettings();

        // Create the strings directory in case it doesn't exist yet
        Directory.CreateDirectory(Path.Join(
            Interfacing.GetProgramLocation().DirectoryName, "Assets", "Strings"));

        // Load language resources
        Interfacing.LoadJsonStringResourcesEnglish();
        Interfacing.LoadJsonStringResources(AppData.Settings.AppLanguage);

        // Setup string hot reload watchdog
        ResourceWatcher = new FileSystemWatcher
        {
            Path = Path.Join(Interfacing.GetProgramLocation().DirectoryName, "Assets", "Strings"),
            NotifyFilter = NotifyFilters.CreationTime | NotifyFilters.FileName |
                           NotifyFilters.LastWrite | NotifyFilters.DirectoryName,
            IncludeSubdirectories = true,
            Filter = "*.json",
            EnableRaisingEvents = true
        };

        // Add event handlers : local
        ResourceWatcher.Changed += OnWatcherOnChanged;
        ResourceWatcher.Created += OnWatcherOnChanged;
        ResourceWatcher.Deleted += OnWatcherOnChanged;
        ResourceWatcher.Renamed += OnWatcherOnChanged;
    }

    private FileSystemWatcher ResourceWatcher { get; }

    /// <summary>
    ///     Invoked when the application is launched normally by the end user.  Other entry points
    ///     will be used such as when the application is launched to open a specific file.
    /// </summary>
    /// <param name="args">Details about the launch request and process.</param>
    protected override void OnLaunched(LaunchActivatedEventArgs args)
    {
        _mWindow = new MainWindow();
        _mWindow.Activate();
    }

    // Send a non-dismissible tip about reloading the app
    private static void OnWatcherOnChanged(object o, FileSystemEventArgs fileSystemEventArgs)
    {
        Logger.Info("String resource files have changed, reloading!");
        Logger.Info($"What happened: {fileSystemEventArgs.ChangeType}");
        Logger.Info($"Where: {fileSystemEventArgs.FullPath} ({fileSystemEventArgs.Name})");

        // Reload language resources
        Interfacing.LoadJsonStringResourcesEnglish();
        Interfacing.LoadJsonStringResources(AppData.Settings.AppLanguage);

        // Reload plugins' language resources
        foreach (var plugin in TrackingDevices.TrackingDevicesList.Values)
            Interfacing.Plugins.SetLocalizationResourcesRoot(plugin.LocalizationResourcesRoot.Directory, plugin.Guid);
        
        // Reload everything we can
        Shared.Devices.DevicesJointsValid = false;

        // Request page reloads
        Translator.Get.OnPropertyChanged();
        Shared.Events.RequestInterfaceReload();

        // Request manager reloads
        AppData.Settings.OnPropertyChanged();
        AppData.Settings.TrackersVector.ToList()
            .ForEach(x => x.OnPropertyChanged());

        // We're done with our changes now!
        Shared.Devices.DevicesJointsValid = true;
    }
}