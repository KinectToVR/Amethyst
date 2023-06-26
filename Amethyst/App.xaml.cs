// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Web;
using Windows.ApplicationModel.Activation;
using Windows.Foundation;
using Windows.Globalization;
using Windows.Storage;
using Windows.System.UserProfile;
using Windows.UI.ViewManagement;
using Windows.Web.Http;
using Amethyst.Classes;
using Amethyst.Popups;
using Amethyst.Schedulers;
using Amethyst.Utils;
using Microsoft.AppCenter;
using Microsoft.AppCenter.Analytics;
using Microsoft.AppCenter.Crashes;
using Microsoft.UI.Xaml;
using Microsoft.Windows.AppLifecycle;
using Newtonsoft.Json.Linq;
using LaunchActivatedEventArgs = Microsoft.UI.Xaml.LaunchActivatedEventArgs;
using Amethyst.MVVM;
using Newtonsoft.Json;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst;

/// <summary>
///     Provides application-specific behavior to supplement the default Application class.
/// </summary>
public partial class App : Application
{
    private readonly List<Host> _views = new();
    private bool _canCloseViews;
    private CrashWindow _crashWindow;
    private MainWindow _mWindow;

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
            if (Interfacing.SuppressAllDomainExceptions) return; // Don't do anything

            var stc = $"{ex.GetType().Name} in {ex.Source}: {ex.Message}\n{ex.StackTrace}";
            var msg = Interfacing.LocalizedJsonString("/CrashHandler/Content/Crash/UnknownStack").Format(stc);
            Interfacing.Fail(msg != "/CrashHandler/Content/Crash/UnknownStack" ? msg : stc);

            // Make App Center send the whole log when crashed
            Crashes.GetErrorAttachments = _ => new[]
            {
                ErrorAttachmentLog.AttachmentWithText(
                    File.ReadAllText(Logger.LogFilePath), new FileInfo(Logger.LogFilePath).Name)
            };

            Crashes.TrackError(e.Exception); // Log the crash reason
            Environment.Exit(0); // Simulate a standard application exit
        };

        // Listen for and log all uncaught second-chance exceptions : Domain
        AppDomain.CurrentDomain.UnhandledException += (_, e) =>
        {
            var ex = (Exception)e.ExceptionObject;
            Logger.Fatal($"Unhandled Exception: {ex.GetType().Name} in {ex.Source}: {ex.Message}\n{ex.StackTrace}");
            if (Interfacing.SuppressAllDomainExceptions) return; // Don't do anything

            var stc = $"{ex.GetType().Name} in {ex.Source}: {ex.Message}\n{ex.StackTrace}";
            var msg = Interfacing.LocalizedJsonString("/CrashHandler/Content/Crash/UnknownStack").Format(stc);
            Interfacing.Fail(msg != "/CrashHandler/Content/Crash/UnknownStack" ? msg : stc);

            // Make App Center send the whole log when crashed
            Crashes.GetErrorAttachments = _ => new[]
            {
                ErrorAttachmentLog.AttachmentWithText(
                    File.ReadAllText(Logger.LogFilePath), new FileInfo(Logger.LogFilePath).Name)
            };

            Crashes.TrackError((Exception)e.ExceptionObject); // Log the crash reason
            Environment.Exit(0); // Simulate a standard application exit
        };

        // Set default window launch size (WinUI)
        ApplicationView.PreferredLaunchViewSize = new Size(1000, 700);
        ApplicationView.PreferredLaunchWindowingMode = ApplicationViewWindowingMode.PreferredLaunchViewSize;

        // Initialize the logger
        Logger.Init(Interfacing.GetAppDataLogFilePath(
            $"Amethyst_{DateTime.Now:yyyyMMdd-HHmmss.ffffff}.log"));

        // Create an empty file for checking for crashes
        try
        {
            Interfacing.CrashFile = new FileInfo(Path.Join(Interfacing.TemporaryFolder.Path, ".crash"));
            if (Environment.GetCommandLineArgs().ElementAtOrDefault(1)?.StartsWith("amethyst-app:crash") ?? false)
                Interfacing.CrashFile.Create(); // Create the file if not running as crash handler
        }
        catch (Exception e)
        {
            Logger.Error(e);
        }

        try
        {
            // Try deleting the "latest" log file
            File.Delete(Interfacing.GetAppDataLogFilePath("_latest.log"));
        }
        catch (Exception e)
        {
            Logger.Info(e);
        }

        // Log status information
        Logger.Info($"CLR Runtime version: {typeof(string).Assembly.ImageRuntimeVersion}");
        Logger.Info($"Framework build number: {Environment.Version} (.NET Core)");
        Logger.Info($"Running on {Environment.OSVersion}");

        Logger.Info($"Amethyst version: {AppData.VersionString}");
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
        AppSettings.DoNotSaveSettings = true;
        AppData.Settings.ReadSettings(); // Now read

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
    /// <param name="eventArgs">Details about the launch request and process.</param>
    protected override async void OnLaunched(LaunchActivatedEventArgs eventArgs)
    {
        // Check if there's any argv[1]
        var args = Environment.GetCommandLineArgs();
        Logger.Info($"Received launch arguments: {string.Join(", ", args)}");

        // Check if this startup isn't a request
        if (args.Length > 1)
            if (args[1] == "Kill" && args.Length > 2)
            {
                try
                {
                    Logger.Info($"Amethyst running in process kill slave mode! ID: {args[2]}");
                    var processToKill = Process.GetProcessById(int.Parse(args[2]));

                    // If we want to kill server, shut down monitor first
                    if (processToKill.ProcessName == "vrserver")
                        Process.GetProcessesByName("vrmonitor").ToList().ForEach(x => x.Kill());

                    processToKill.Kill(); // Now kill the actual process
                }
                catch (Exception e)
                {
                    Logger.Error(e);
                }

                Logger.Info("That's all! Shutting down now...");
                Environment.Exit(0); // Cancel further application startup
            }
            else if (args[1].StartsWith("amethyst-app:crash"))
            {
                Logger.Info("Amethyst running in crash handler mode!");
                Interfacing.SuppressAllDomainExceptions = true;

                // Disable internal sounds
                ElementSoundPlayer.State = ElementSoundPlayerState.Off;

                Logger.Info("Creating a new CrashWindow view...");
                _crashWindow = new CrashWindow(); // Create a new window

                Logger.Info($"Activating {_crashWindow.GetType()}...");
                _crashWindow.Activate(); // Activate the main window

                // Do this in the meantime
                _ = Task.Factory.StartNew(async () =>
                {
                    try
                    {
                        var client = new HttpClient();

                        using var response =
                            await client.GetAsync(new Uri("https://docs.k2vr.tech/shared/locales.json"));
                        using var content = response.Content;
                        var json = await content.ReadAsStringAsync();

                        // Optionally fall back to English
                        if (JObject.Parse(json)[Interfacing.DocsLanguageCode] == null)
                            Interfacing.DocsLanguageCode = "en";
                    }
                    catch (Exception)
                    {
                        Interfacing.DocsLanguageCode = "en";
                    }
                });

                // That's all!
                return;
            }

        // Check if activated via uri
        var activationUri = (AppInstance.GetCurrent().GetActivatedEventArgs().Data as
            ProtocolActivatedEventArgs)?.Uri;

        // Check if there's any launch arguments
        if (activationUri is not null && activationUri.Segments.Length > 0)
        {
            Logger.Info($"Activated via uri of: {activationUri}");
            switch (activationUri.Segments.First())
            {
                case "logs":
                {
                    SystemShell.OpenFolderAndSelectItem(
                        new DirectoryInfo(Interfacing.GetAppDataLogFilePath(""))
                            .GetFiles().OrderByDescending(x => x.LastWriteTime)
                            .ToList().ElementAtOrDefault(1)?.FullName
                        ?? Interfacing.GetAppDataLogFilePath(""));

                    Logger.Info("That's all! Shutting down now...");
                    Environment.Exit(0); // Cancel further application startup
                    break;
                }
                case "data-folder":
                {
                    SystemShell.OpenFolderAndSelectItem(
                        Directory.GetParent(Interfacing.LocalFolder.Path)?.FullName);

                    Logger.Info("That's all! Shutting down now...");
                    Environment.Exit(0); // Cancel further application startup
                    break;
                }
                case "mutable-folder":
                {
                    const string mutablePath = @"C:\Program Files\ModifiableWindowsApps\K2VRTeam.Amethyst.App";
                    SystemShell.OpenFolderAndSelectItem(Directory.Exists(mutablePath)
                        ? mutablePath // Check whether the global mutable folder is accessible
                        : Path.Join(Directory.GetParent( // Otherwise open the virtual fs one
                            Interfacing.LocalFolder.Path)?.FullName, "AC", "MutablePackageRoot"));

                    Logger.Info("That's all! Shutting down now...");
                    Environment.Exit(0); // Cancel further application startup
                    break;
                }
                case "vfs-folder":
                {
                    SystemShell.OpenFolderAndSelectItem(Path.Join(
                        Directory.GetParent(Interfacing.LocalFolder.Path)?.FullName, "AC"));

                    Logger.Info("That's all! Shutting down now...");
                    Environment.Exit(0); // Cancel further application startup
                    break;
                }
                case "make-local":
                {
                    try
                    {
                        // Read the query string
                        var queryDictionary = HttpUtility
                            .ParseQueryString(activationUri!.Query.TrimStart('?'));

                        // Read all needed query parameters
                        var sourcePath = queryDictionary["source"];
                        var targetFolderPath = queryDictionary["target"];

                        // Validate our parameters
                        if (sourcePath is not null)
                        {
                            // Modify our parameters
                            sourcePath = Path.Join(Interfacing.ProgramLocation.DirectoryName!, sourcePath);
                            var targetFolder = string.IsNullOrEmpty(targetFolderPath)
                                ? Interfacing.LocalFolder // Copy to LocalState if not passed
                                : await Interfacing.LocalFolder.CreateFolderAsync(targetFolderPath);

                            if (File.Exists(sourcePath))
                                await (await StorageFile.GetFileFromPathAsync(sourcePath)).CopyAsync(targetFolder);

                            else if (Directory.Exists(sourcePath))
                                new DirectoryInfo(sourcePath).CopyToFolder(
                                    (await targetFolder.CreateFolderAsync(new DirectoryInfo(sourcePath).Name,
                                        CreationCollisionOption.OpenIfExists)).Path);
                        }
                    }
                    catch (Exception e)
                    {
                        Logger.Error(e);
                    }

                    Logger.Info("That's all! Shutting down now...");
                    Environment.Exit(0); // Cancel further application startup
                    break;
                }
                case "set-defaults":
                {
                    try
                    {
                        // Read the query string
                        var queryDictionary = HttpUtility
                            .ParseQueryString(activationUri!.Query.TrimStart('?'));

                        // Read all needed query parameters
                        var trackingDevice = queryDictionary["TrackingDevice"];
                        var serviceEndpoint = queryDictionary["ServiceEndpoint"];
                        var extraTrackersValid = bool.TryParse(queryDictionary["ExtraTrackers"], out var extraTrackers);

                        Logger.Info($"Received defaults: TrackingDevice{{{trackingDevice}}}, " +
                                    $"ServiceEndpoint{{{serviceEndpoint}}}, ExtraTrackers{{{extraTrackers}}}");

                        // Create a new default config
                        await File.WriteAllTextAsync(Interfacing.GetAppDataFilePath("PluginDefaults.json"),
                            JsonConvert.SerializeObject(new DefaultSettings
                            {
                                TrackingDevice = trackingDevice,
                                ServiceEndpoint = serviceEndpoint,
                                ExtraTrackers = extraTrackersValid ? extraTrackers : null
                            }, Formatting.Indented));
                    }
                    catch (Exception e)
                    {
                        Logger.Error(e);
                    }

                    Logger.Info("That's all! Shutting down now...");
                    Environment.Exit(0); // Cancel further application startup
                    break;
                }
                case "report":
                {
                    try
                    {
                        Logger.Info("Creating a data file list base...");
                        var fileList = new List<AppDataFile>
                            { new(await Interfacing.GetAppDataFile("AmethystSettings.json")) };

                        Logger.Info("Searching for recent log files...");
                        fileList.AddRange(await new DirectoryInfo(Interfacing.GetAppDataLogFilePath(""))
                            .GetFiles().OrderByDescending(x => x.LastWriteTime).ToList().Take(3)
                            .Select(async x => new AppDataFile(await StorageFile.GetFileFromPathAsync(x.FullName)))
                            .WhenAll()); // Collect the last 3 log files and wait for them

                        try
                        {
                            if (AppData.Settings.ServiceEndpointGuid is "K2VRTEAM-AME2-APII-SNDP-SENDPTOPENVR")
                            {
                                Logger.Info("Trying to collect SteamVR server logs...");
                                var vrServerLog = new FileInfo(Path.Join(
                                    FileUtils.GetSteamInstallDirectory(), @"logs\vrserver.txt"));

                                if (vrServerLog.Exists)
                                {
                                    if (fileList.Count > 3) fileList.RemoveAt(1); // Remove the first (current) log
                                    fileList.Add(new AppDataFile(
                                        await StorageFile.GetFileFromPathAsync(vrServerLog.FullName)));
                                }
                            }
                            else
                            {
                                Logger.Info("Skipping collection of SteamVR server logs...");
                            }
                        }
                        catch (Exception e)
                        {
                            Logger.Warn(e);
                        }

                        Logger.Info("Creating a new Host:Report view...");
                        var window = new Host
                        {
                            Content = new Report(fileList)
                        };

                        Logger.Info($"Activating {window.GetType()}...");
                        window.Activate(); // Activate the main window

                        return; // That's all for now!
                    }
                    catch (Exception e)
                    {
                        Logger.Error(e);
                    }

                    Logger.Info("That's all! Shutting down now...");
                    Environment.Exit(0); // Cancel further application startup
                    break;
                }
            }
        }

        Logger.Info("Registering a named mutex for com_k2vr_amethyst...");
        try
        {
            Shared.Main.ApplicationMultiInstanceMutex = new Mutex(
                true, "com_k2vr_amethyst", out var needToCreateNew);

            if (!needToCreateNew)
            {
                Logger.Fatal(new AbandonedMutexException("Startup failed! The app is already running."));
                await "amethyst-app:crash-already-running".ToUri().LaunchAsync();

                await Task.Delay(3000);
                Environment.Exit(0); // Exit peacefully
            }
        }
        catch (Exception e)
        {
            Logger.Fatal(new AbandonedMutexException($"Startup failed! Mutex creation error: {e.Message}"));
            await "amethyst-app:crash-already-running".ToUri().LaunchAsync();

            await Task.Delay(3000);
            Environment.Exit(0); // Exit peacefully
        }

        Logger.Info("Starting the crash handler passing the app PID...");
        await ($"amethyst-app:crash-watchdog?pid={Environment.ProcessId}&log={Logger.LogFilePath}" +
               $"&crash={Path.Join(Interfacing.TemporaryFolder.Path, ".crash")}").ToUri().LaunchAsync();

        // Disable internal sounds
        ElementSoundPlayer.State = ElementSoundPlayerState.Off;

        // Create the plugin directory (if not existent)
        Directory.CreateDirectory(Path.Combine(
            Interfacing.ProgramLocation.DirectoryName!, "Plugins"));

        // Try reading the startup task config
        try
        {
            Logger.Info("Searching for scheduled startup tasks...");
            StartupController.Controller.ReadTasks();
        }
        catch (Exception e)
        {
            Logger.Error($"Reading the startup scheduler configuration failed. Message: {e.Message}");
        }

        // Execute plugin uninstalls: delete plugin files
        foreach (var action in StartupController.Controller.DeleteTasks.ToList())
            try
            {
                Logger.Info($"Parsing a startup {action.GetType()} task with name \"{action.Name}\"...");
                if (!Directory.Exists(action.PluginFolder) ||
                    action.PluginFolder == Interfacing.ProgramLocation.DirectoryName || Directory
                        .EnumerateFiles(action.PluginFolder)
                        .Any(x => x == Interfacing.ProgramLocation.FullName))
                {
                    StartupController.Controller.StartupTasks.Remove(action);
                    continue; // Remove the action as it was invalid at this time
                }

                Logger.Info("Cleaning the plugin folder now...");
                Directory.Delete(action.PluginFolder, true);

                Logger.Info("Deleting attempted scheduled startup " +
                            $"{action.GetType()} task with name \"{action.Name}\"...");

                StartupController.Controller.StartupTasks.Remove(action);
                Logger.Info($"Looks like a startup {action.GetType()} task with " +
                            $"name \"{action.Name}\" has been executed successfully!");
            }
            catch (Exception e)
            {
                Logger.Warn(e);
            }

        // Execute plugin updates: replace plugin files
        foreach (var action in StartupController.Controller.UpdateTasks.ToList())
            try
            {
                Logger.Info($"Parsing a startup {action.GetType()} task with name \"{action.Name}\"...");
                if (!Directory.Exists(action.PluginFolder) || !File.Exists(action.UpdatePackage))
                {
                    StartupController.Controller.StartupTasks.Remove(action);
                    continue; // Remove the action as it was invalid at this time
                }

                Logger.Info($"Found a plugin update package in folder \"{action.PluginFolder}\"");
                Logger.Info("Checking if the plugin folder is not locked...");
                if (FileUtils.WhoIsLocking(action.PluginFolder).Any())
                {
                    Logger.Info("Some plugin files are still locked! Showing the update dialog...");
                    Logger.Info($"Creating a new {typeof(Host)}...");
                    var dialogWindow = new Host();

                    Logger.Info("Preparing the data regarding the blocking process...");
                    dialogWindow.Content = new Blocked(new DirectoryInfo(action.PluginFolder).Name,
                        FileUtils.WhoIsLocking(action.PluginFolder)
                            .Select(x => new BlockingProcess
                            {
                                ProcessPath = new FileInfo(FileUtils.GetProcessFilename(x) ?? "tmp"),
                                Process = x,
                                IsElevated = FileUtils.IsProcessElevated(x) && !FileUtils.IsCurrentProcessElevated()
                            }).ToList())
                    {
                        // Also set the parent to close
                        ParentWindow = dialogWindow
                    };

                    Logger.Info($"Activating {dialogWindow.GetType()} now...");
                    dialogWindow.Activate(); // Activate the window now

                    var continueEvent = new SemaphoreSlim(0);
                    var result = false; // Prepare [out] results for the event handler

                    // Wait for the dialog window to close
                    dialogWindow.Closed += (sender, arg) =>
                    {
                        arg.Handled = !_canCloseViews; // Mark as handled (or not)
                        if (_canCloseViews) return; // Close if already done...

                        _views.Add(sender as Host); // Add this sender to closable list
                        result = (sender as Host)?.Result ?? false; // Grab the result
                        Logger.Info($"The dialog was closed with result {result}!");
                        continueEvent.Release(); // Unlock further program execution
                    };

                    await continueEvent.WaitAsync(); // Wait for the closed event
                    if (!result) continue; // Don't update if not resolved properly (yet)
                }

                Logger.Info("Cleaning the plugin folder now...");
                Directory.GetDirectories(action.PluginFolder).ToList().ForEach(x => Directory.Delete(x, true));
                Directory.GetFiles(action.PluginFolder, "*.*", SearchOption.AllDirectories)
                    .Where(x => x != action.UpdatePackage).ToList().ForEach(File.Delete);

                Logger.Info("Unpacking the new plugin from its archive...");
                ZipFile.ExtractToDirectory(action.UpdatePackage, action.PluginFolder, true);

                Logger.Info("Deleting the plugin update package...");
                File.Delete(action.UpdatePackage); // Cleanup after the update

                Logger.Info("Deleting attempted scheduled startup " +
                            $"{action.GetType()} task with name \"{action.Name}\"...");

                StartupController.Controller.StartupTasks.Remove(action);
                Logger.Info($"Looks like a startup {action.GetType()} task with " +
                            $"name \"{action.Name}\" has been executed successfully!");
            }
            catch (Exception e)
            {
                Logger.Warn(e);
            }

        Logger.Info("Creating a new MainWindow view...");
        _mWindow = new MainWindow(); // Create a new window

        Logger.Info($"Activating {_mWindow.GetType()}...");
        _mWindow.Activate(); // Activate the main window

        // Check for any additional work
        if (!_views.Any()) return;

        Logger.Info($"Waiting for {_mWindow.GetType()} to show up...");
        await Task.Delay(1000); // Assume this will be enough

        Logger.Info("Closing all other views now...");
        _canCloseViews = true; // Tell other views they can close
        _views.ForEach(x => x.Close()); // Try closing each
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