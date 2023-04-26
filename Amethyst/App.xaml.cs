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
using Windows.Foundation;
using Windows.Globalization;
using Windows.System.UserProfile;
using Windows.UI.ViewManagement;
using Amethyst.Classes;
using Amethyst.Popups;
using Amethyst.Schedulers;
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

            var stc = $"{ex.GetType().Name} in {ex.Source}: {ex.Message}\n{ex.StackTrace}";
            var msg = string.Format(Interfacing.LocalizedJsonString("/CrashHandler/Content/Crash/UnknownStack"), stc);
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

            var stc = $"{ex.GetType().Name} in {ex.Source}: {ex.Message}\n{ex.StackTrace}";
            var msg = string.Format(Interfacing.LocalizedJsonString("/CrashHandler/Content/Crash/UnknownStack"), stc);
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
        Logger.Init(Interfacing.GetAppDataLogFileDir("Amethyst",
            $"Amethyst_{DateTime.Now:yyyyMMdd-HHmmss.ffffff}.log"));

        // Create an empty file for checking for crashes
        try
        {
            Interfacing.CrashFile = new FileInfo(Path.Join(Interfacing.ProgramLocation.DirectoryName, ".crash"));
            Interfacing.CrashFile.Create(); // Create the file
        }
        catch (Exception e)
        {
            Logger.Error(e);
        }

        try
        {
            // Try deleting the "latest" log file
            File.Delete(Interfacing.GetAppDataLogFileDir("Amethyst", "_latest.log"));
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
    /// <param name="_">Details about the launch request and process.</param>
    protected override async void OnLaunched(LaunchActivatedEventArgs _)
    {
        // Check if there's any argv[1]
        var args = Environment.GetCommandLineArgs();
        Logger.Info($"Received launch arguments: {string.Join(", ", args)}");

        // Check if this startup isn't a request
        if (args.Length > 2 && args[1] == "Kill")
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

        Logger.Info("Registering a named mutex for com_k2vr_amethyst...");
        try
        {
            Shared.Main.ApplicationMultiInstanceMutex = new Mutex(
                true, "com_k2vr_amethyst", out var needToCreateNew);

            if (!needToCreateNew)
            {
                Logger.Fatal(new AbandonedMutexException("Startup failed! The app is already running."));

                if (File.Exists(Path.Combine(Interfacing.ProgramLocation.DirectoryName!,
                        "K2CrashHandler", "K2CrashHandler.exe")))
                    Process.Start(Path.Combine(Interfacing.ProgramLocation.DirectoryName,
                        "K2CrashHandler", "K2CrashHandler.exe"), "already_running");
                else
                    Logger.Warn("Crash handler exe (./K2CrashHandler/K2CrashHandler.exe) not found!");

                await Task.Delay(3000);
                Environment.Exit(0); // Exit peacefully
            }
        }
        catch (Exception e)
        {
            Logger.Fatal(new AbandonedMutexException(
                $"Startup failed! Multi-instance lock mutex creation error: {e.Message}"));

            if (File.Exists(Path.Combine(Interfacing.ProgramLocation.DirectoryName!,
                    "K2CrashHandler", "K2CrashHandler.exe")))
                Process.Start(Path.Combine(Interfacing.ProgramLocation.DirectoryName,
                    "K2CrashHandler", "K2CrashHandler.exe"), "already_running");
            else
                Logger.Warn("Crash handler exe (./K2CrashHandler/K2CrashHandler.exe) not found!");

            await Task.Delay(3000);
            Environment.Exit(0); // Exit peacefully
        }

        // Priority: Launch the crash handler
        Logger.Info("Starting the crash handler passing the app PID...");

        if (File.Exists(Path.Combine(Interfacing.ProgramLocation.DirectoryName!,
                "K2CrashHandler", "K2CrashHandler.exe")))
            Process.Start(Path.Combine(Interfacing.ProgramLocation.DirectoryName,
                "K2CrashHandler", "K2CrashHandler.exe"), $"{Environment.ProcessId} \"{Logger.LogFilePath}\"");
        else
            Logger.Warn(
                $"Crash handler exe ({Path.Combine(Interfacing.ProgramLocation.DirectoryName, "K2CrashHandler", "K2CrashHandler.exe")}) not found!");

        // Disable internal sounds
        ElementSoundPlayer.State = ElementSoundPlayerState.Off;

        // Create the plugin directory (if not existent)
        Directory.CreateDirectory(Path.Combine(
            Interfacing.ProgramLocation.DirectoryName, "Plugins"));

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
        if (args.Length < 2 || args[1] != "NoUpdate")
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
                        dialogWindow.Closed += (sender, _) =>
                        {
                            result = (sender as Host)?.Result ?? false;
                            Logger.Info($"The dialog was closed with result {result}!");
                            continueEvent.Release(); // Unlock further program execution
                        };

                        await continueEvent.WaitAsync(); // Wait for the closed event
                        Logger.Info($"Plugin update conflict resolved with result: {result}! Restarting...");
                        await Interfacing.ExecuteAppRestart(false, result ? "" : "NoUpdate");
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
        else
            Logger.Info("Plugin update skip requested!");

        Logger.Info("Creating a new MainWindow view...");
        _mWindow = new MainWindow();

        Logger.Info($"Activating {_mWindow.GetType()}...");
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