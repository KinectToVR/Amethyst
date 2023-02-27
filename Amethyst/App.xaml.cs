// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Globalization;
using Windows.System.UserProfile;
using Windows.UI.ViewManagement;
using Amethyst.Classes;
using Amethyst.Utils;
using Microsoft.AppCenter;
using Microsoft.AppCenter.Analytics;
using Microsoft.AppCenter.Crashes;
using Microsoft.UI.Xaml;

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

        // Listen for and log all uncaught second-chance exceptions : XamlApp
        UnhandledException += (_, e) =>
        {
            var ex = e.Exception;
            Logger.Fatal($"Unhandled Exception: {ex.GetType().Name} in {ex.Source}: {ex.Message}\n{ex.StackTrace}");

            var stc = $"{ex.GetType().Name} in {ex.Source}: {ex.Message}\n{ex.StackTrace}";
            var msg = string.Format(Interfacing.LocalizedJsonString("/CrashHandler/Content/Crash/UnknownStack"), stc);

            Interfacing.Fail(msg != "/CrashHandler/Content/Crash/UnknownStack" ? msg : stc);
            Crashes.TrackError(e.Exception); // Log the crash reason
            Environment.Exit(0); // Simulate a standard application exit
        };

        // Listen for and log all uncaught second-chance exceptions : Domain
        AppDomain.CurrentDomain.UnhandledException += (_, e) =>
        {
            var ex = (Exception)e.ExceptionObject;
            Logger.Fatal($"Unhandled Exception: {ex.GetType().Name} in {ex.Source}: {ex.Message}\n{ex.StackTrace}");

            var stc = $"{ex.GetType().Name} in {ex.Source}: {ex.Message}\n{ex.StackTrace}";
            var msg = string.Format(Interfacing.LocalizedJsonString("/CrashHandler/Content/Crash/UnknownStack"), stc);

            Interfacing.Fail(msg != "/CrashHandler/Content/Crash/UnknownStack" ? msg : stc);
            Crashes.TrackError((Exception)e.ExceptionObject); // Log the crash reason
            Environment.Exit(0); // Simulate a standard application exit
        };

        // Set default window launch size (WinUI)
        ApplicationView.PreferredLaunchViewSize = new Size(1000, 700);
        ApplicationView.PreferredLaunchWindowingMode = ApplicationViewWindowingMode.PreferredLaunchViewSize;

        // Initialize the logger
        Logger.Init(Interfacing.GetAppDataLogFileDir("Amethyst",
            $"Amethyst_{DateTime.Now:yyyyMMdd-HHmmss.ffffff}.log"));

        // Create an empty file for checking for crashes
        Interfacing.CrashFile = new FileInfo(Path.Join(Interfacing.ProgramLocation.DirectoryName, ".crash"));
        Interfacing.CrashFile.Create(); // Create the file

        try
        {
            // Try deleting the "latest" log file
            File.Delete(Interfacing.GetAppDataLogFileDir("Amethyst", "_latest.log"));
        }
        catch (Exception)
        {
            // ignored
        }

        // Log status information
        Logger.Info($"CLR Runtime version: {typeof(string).Assembly.ImageRuntimeVersion}");
        Logger.Info($"Framework build number: {Environment.Version} (.NET Core)");
        Logger.Info($"Running on {Environment.OSVersion}");

        Logger.Info($"Amethyst version: {AppData.VersionString}");
        Logger.Info($"Amethyst internal version: {AppData.InternalVersion}");
        Logger.Info($"Amethyst web API version: {AppData.ApiVersion}");
        Logger.Info("Amethyst build commit: AZ_COMMIT_SHA");

        try
        {
            // Set maximum log size to 20MB for larger log files
            Logger.Info("Setting maximum App Center log size...");
            AppCenter.SetMaxStorageSizeAsync(20 * 1024 * 1024).ContinueWith(storageTask =>
            {
                // Log as an error, we can't really do much about this one after all...
                if (!storageTask.Result) Logger.Error("App Center log exceeded the maximum size!");
            });

            // Try starting Visual Studio App Center up
            Logger.Info("Starting Visual Studio App Center...");
            AppCenter.Start("AZ_APPSECRET", typeof(Analytics), typeof(Crashes));

            // Set the code of the language used in Windows
            AppCenter.SetCountryCode(new Language(
                GlobalizationPreferences.Languages[0]).LanguageTag[3..]);

            // Make App Center send the whole log when crashed
            Crashes.GetErrorAttachments = _ => new[]
            {
                ErrorAttachmentLog.AttachmentWithText(
                    File.ReadAllText(Logger.LogFilePath), new FileInfo(Logger.LogFilePath).Name)
            };
        }
        catch (Exception e)
        {
            Logger.Warn($"Couldn't start App Center! Message: {e.Message}");
        }

        // Read saved settings
        Logger.Info("Reading base app settings...");
        AppData.Settings.ReadSettings();

        // Read plugin settings
        Logger.Info("Reading custom plugin settings...");
        AppPlugins.PluginSettings.ReadSettings();

        // Run detached to allow for async calls
        Task.Run(async () =>
        {
            try
            {
                // Toggle App Center according to our settings
                await Analytics.SetEnabledAsync(AppData.Settings.IsTelemetryEnabled);
                await Crashes.SetEnabledAsync(AppData.Settings.IsTelemetryEnabled);
            }
            catch (Exception e)
            {
                Logger.Warn($"Couldn't toggle App Center! Message: {e.Message}");
            }
        });

        // Create the strings directory in case it doesn't exist yet
        Directory.CreateDirectory(Path.Join(
            Interfacing.ProgramLocation.DirectoryName, "Assets", "Strings"));

        // Load language resources
        Interfacing.LoadJsonStringResourcesEnglish();
        Interfacing.LoadJsonStringResources(AppData.Settings.AppLanguage);

        // Setup string hot reload watchdog
        ResourceWatcher = new FileSystemWatcher
        {
            Path = Path.Join(Interfacing.ProgramLocation.DirectoryName, "Assets", "Strings"),
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
        Shared.Main.DispatcherQueue.TryEnqueue(() =>
        {
            Logger.Info("String resource files have changed, reloading!");
            Logger.Info($"What happened: {fileSystemEventArgs.ChangeType}");
            Logger.Info($"Where: {fileSystemEventArgs.FullPath} ({fileSystemEventArgs.Name})");

            // Sanity check
            if (!Shared.Main.MainWindowLoaded) return;

            // Reload language resources
            Interfacing.LoadJsonStringResourcesEnglish();
            Interfacing.LoadJsonStringResources(AppData.Settings.AppLanguage);

            // Reload plugins' language resources
            foreach (var plugin in AppPlugins.TrackingDevicesList.Values)
                Interfacing.Plugins.SetLocalizationResourcesRoot(
                    plugin.LocalizationResourcesRoot.Directory, plugin.Guid);

            foreach (var plugin in AppPlugins.ServiceEndpointsList.Values)
                Interfacing.Plugins.SetLocalizationResourcesRoot(
                    plugin.LocalizationResourcesRoot.Directory, plugin.Guid);

            // Reload everything we can
            Shared.Devices.DevicesJointsValid = false;

            // Reload plugins' interfaces
            AppPlugins.TrackingDevicesList.Values.ToList().ForEach(x => x.OnLoad());
            AppPlugins.ServiceEndpointsList.Values.ToList().ForEach(x => x.OnLoad());

            // Request page reloads
            Translator.Get.OnPropertyChanged();
            Shared.Events.RequestInterfaceReload();

            // Request manager reloads
            AppData.Settings.OnPropertyChanged();
            AppData.Settings.TrackersVector.ToList()
                .ForEach(x => x.OnPropertyChanged());

            // We're done with our changes now!
            Shared.Devices.DevicesJointsValid = true;
        });
    }
}