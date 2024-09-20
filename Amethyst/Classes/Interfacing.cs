using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Web;
using Windows.Data.Json;
using Windows.Storage;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Media.Animation;
using Microsoft.Windows.AppLifecycle;
using Microsoft.Windows.AppNotifications;
using Newtonsoft.Json;
using System.Threading;

namespace Amethyst.Classes;

public static class Interfacing
{
    public delegate void AppSettingsReadEventHandler(object sender, EventArgs e);

    public const uint MaxPingCheckingThreads = 3;

    public static bool
        IsExitingNow, // App closing check
        IsExitHandled, // If actions have been done
        IsExitPending; // If actions are running

    public static readonly object UpdateLock = new();

    // App crash check
    public static FileInfo CrashFile;

    // Update check
    public static bool
        UpdateFound = false,
        ManualUpdate = false,
        UpdatingNow = false;

    // Update file
    public static string UpdateFileName = null;

    // Position helpers for devices -> GUID, Pose
    public static readonly SortedDictionary<string, (Vector3 Position, Quaternion Orientation)>
        DeviceHookJointPosition = new(); // This one applies to both

    public static readonly SortedDictionary<string, (Vector3 Position, Quaternion Orientation)>
        // For automatic calibration
        DeviceRelativeTransformOrigin = new(); // This one applies to both

    // Current page string
    public static string CurrentPageTag = "general";
    public static string CurrentPageClass = "Amethyst.Pages.General";

    // Current app state string (e.g. "general", "calibration_manual")
    public static string CurrentAppState = "general";

    // Currently available website language code
    public static string DocsLanguageCode = "en";

    // The actual app theme (ONLY dark/light)
    public static ElementTheme ActualTheme = ElementTheme.Dark;

    // If trackers are added / initialized
    public static bool AppTrackersSpawned,
        AppTrackersInitialized;

    // Is the tracking paused
    public static bool IsTrackingFrozen = false;

    // Server checking threads number, max num of them
    public static uint PingCheckingThreadsNumber;

    // Disable app domain exception handling
    public static bool SuppressAllDomainExceptions = false;

    // Server interfacing data
    public static int ServiceEndpointStatusCode;
    public static long PingTime;
    public static bool ServiceEndpointFailure;

    public static string ServiceEndpointStatusString = " \n \n ";

    // Check if we're currently scanning for trackers from other apps
    public static bool IsAlreadyAddedTrackersScanRunning = false;

    // If the already-added trackers check was requested
    public static bool AlreadyAddedTrackersScanRequested = false;

    // Amethyst language resource trees
    private static LocalisationFileJson _localResources = new(), _englishResources = new();

    // Is NUX currently opened?
    public static bool IsNuxPending = false;

    // Flip defines for the base device - iteration persistent
    public static bool BaseFlip = false; // Assume non flipped

    // Flip defines for the override device - iteration persistent
    public static bool OverrideFlip = false; // Assume non flipped

    // For the relay module to override the "Relay active" bar
    public static (string Title, string Content, string Button, Action Click, bool Closable)? RelayBarOverride = null;

    public static bool IsServiceEndpointPresent => ServiceEndpointStatusCode == 0;

    public static FileInfo ProgramLocation => new(Assembly.GetExecutingAssembly().Location);

    public static AppSettingsReadEventHandler AppSettingsRead { get; set; } = (_, _) => { };

    public static StorageFolder TemporaryFolder => ApplicationData.Current.TemporaryFolder;

    public static StorageFolder LocalFolder => ApplicationData.Current.LocalFolder;

    public static async Task<StorageFolder> GetPluginsFolder()
    {
        return await LocalFolder
            .CreateFolderAsync("Plugins", CreationCollisionOption.OpenIfExists);
    }

    public static async Task<StorageFolder> GetPluginsTempFolder()
    {
        return await LocalFolder
            .CreateFolderAsync("Plugins", CreationCollisionOption.OpenIfExists);
    }

    public static async Task<StorageFile> GetAppDataFile(string relativeFilePath)
    {
        return await LocalFolder.CreateFileAsync(relativeFilePath, CreationCollisionOption.OpenIfExists);
    }

    public static async Task<StorageFolder> GetAppDataPluginFolder(string relativeFilePath)
    {
        return string.IsNullOrEmpty(relativeFilePath)
            ? await GetPluginsFolder()
            : await (await GetPluginsFolder())
                .CreateFolderAsync(relativeFilePath, CreationCollisionOption.OpenIfExists);
    }

    public static async Task<StorageFolder> GetTempPluginFolder(string relativeFilePath)
    {
        return string.IsNullOrEmpty(relativeFilePath)
            ? await GetPluginsTempFolder()
            : await (await GetPluginsTempFolder())
                .CreateFolderAsync(relativeFilePath, CreationCollisionOption.OpenIfExists);
    }

    public static string GetAppDataFilePath(string relativeFilePath)
    {
        return Path.Join(LocalFolder.Path, relativeFilePath);
    }

    public static string GetAppDataLogFilePath(string relativeFilePath)
    {
        Directory.CreateDirectory(Path.Join(TemporaryFolder.Path, "Logs", "Amethyst"));
        return Path.Join(TemporaryFolder.Path, "Logs", "Amethyst", relativeFilePath);
    }

    // Fail with an exit code (don't delete .crash)
    public static void Fail(string message)
    {
        IsExitHandled = true;
        Task.Run(async Task () =>
        {
            Logger.Info($"Activating the crash handler with #message: {message}");
            await $"amethyst-app:crash-message#{HttpUtility.UrlEncode(message)}".ToUri().LaunchAsync();

            Logger.Info("Waiting...");
            await Task.Delay(1000); // Wait for the crash handler to be activated
        }).Wait();

        try
        {
            if (AppPlugins.ServiceEndpointsList.TryGetValue("K2VRTEAM-AME2-APII-DVCE-TRACKINGRELAY", out var relay))
            {
                Logger.Info("Telling connected tracking clients to shut down... (Amethyst Tracking Relay)");
                var requestShutdownProperty = relay.Service.GetType().GetProperty("RequestShutdown");
                if (requestShutdownProperty is not null && requestShutdownProperty.CanRead)
                    ((Action<string, bool, CancellationToken>)requestShutdownProperty.GetValue(relay.Service))?.Invoke(message, true, default);
            }
        }
        catch (Exception e)
        {
            Logger.Warn(e);
        }

        Environment.Exit(0); // Exit
    }

    // Show SteamVR toast / notification
    public static void ShowServiceToast(string header, string text)
    {
        AppPlugins.CurrentServiceEndpoint.DisplayToast((header, text));
    }

    // Show an app toast / notification
    public static void ShowToast(string header, string text,
        bool highPriority = false, string action = "none")
    {
        var payload =
            $"<toast launch=\"action={action}&amp;actionId=00000\">" +
            "<visual><binding template = \"ToastGeneric\">" +
            (string.IsNullOrEmpty(header) ? "" : $"<text>{header}</text>") +
            (string.IsNullOrEmpty(text) ? "" : $"<text>{text}</text>") +
            "</binding></visual></toast>";

        AppNotification toast = new(payload)
        {
            Tag = "Tag_AmethystNotifications",
            Group = "Group_AmethystNotifications",
            Priority = highPriority
                ? AppNotificationPriority.High
                : AppNotificationPriority.Default
        };

        Shared.Main.NotificationManager?.Show(toast);
    }

    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool SetForegroundWindow(nint hWnd);

    public static void ProcessToastArguments(
        AppNotificationActivatedEventArgs eventArgs)
    {
        // When a tracker's been auto-disabled
        if (eventArgs.Argument.Contains("focus_trackers"))
            Shared.Main.DispatcherQueue.TryEnqueue(
                async () =>
                {
                    // Bring Amethyst to front
                    SetForegroundWindow(Shared.Main.AppWindowId);

                    if (CurrentPageTag != "settings")
                    {
                        // Navigate to the settings page
                        Shared.Main.MainNavigationView.SelectedItem =
                            Shared.Main.MainNavigationView.MenuItems[1];

                        Shared.Main.NavigateToPage("settings",
                            new EntranceNavigationTransitionInfo());

                        await Task.Delay(500);
                    }

                    Shared.Settings.PageMainScrollViewer.UpdateLayout();
                    Shared.Settings.PageMainScrollViewer.ChangeView(null,
                        Shared.Settings.PageMainScrollViewer.ExtentHeight / 2.0, null);

                    await Task.Delay(500);

                    // Focus on the restart button
                    Shared.Settings.CheckOverlapsCheckBox.Focus(FocusState.Keyboard);
                });

        // When you need to restart OpenVR
        if (eventArgs.Argument.Contains("focus_restart"))
            Shared.Main.DispatcherQueue.TryEnqueue(
                async () =>
                {
                    // Bring Amethyst to front
                    SetForegroundWindow(Shared.Main.AppWindowId);

                    if (CurrentPageTag != "settings")
                    {
                        // Navigate to the settings page
                        Shared.Main.MainNavigationView.SelectedItem =
                            Shared.Main.MainNavigationView.MenuItems[1];

                        Shared.Main.NavigateToPage("settings",
                            new EntranceNavigationTransitionInfo());

                        await Task.Delay(500);
                    }

                    Shared.Settings.PageMainScrollViewer.UpdateLayout();
                    Shared.Settings.PageMainScrollViewer.ChangeView(null,
                        Shared.Settings.PageMainScrollViewer.ExtentHeight, null);

                    await Task.Delay(500);

                    // Focus on the restart button
                    Shared.Settings.RestartButton.Focus(FocusState.Keyboard);
                });

        // Else no click action requested ("none")
    }

    public static async Task HandleAppExit(int sleepMillis)
    {
        // Mark exiting as true
        IsExitingNow = true;
        Logger.Info("AppWindow.Closing handler called, starting the shutdown routine...");

        // Mark trackers as inactive
        AppTrackersInitialized = false;

        // Wait a moment & exit
        Logger.Info($"Shutdown actions completed, disconnecting devices and exiting in {sleepMillis}ms...");
        await Task.Delay(sleepMillis); // Sleep a bit for a proper server disconnect

        try
        {
            if (AppPlugins.ServiceEndpointsList.TryGetValue("K2VRTEAM-AME2-APII-DVCE-TRACKINGRELAY", out var relay))
            {
                Logger.Info("Telling connected tracking clients to shut down... (Amethyst Tracking Relay)");
                var requestShutdownProperty = relay.Service.GetType().GetProperty("RequestShutdown");
                if (requestShutdownProperty is not null && requestShutdownProperty.CanRead)
                    ((Action<string, bool, CancellationToken>)requestShutdownProperty.GetValue(relay.Service))?.Invoke("Server shutting down", false, default);
            }
        }
        catch (Exception e)
        {
            Logger.Warn(e);
        }

        try
        {
            // Close the multi-process mutex
            Shared.Main.ApplicationMultiInstanceMutex.ReleaseMutex();
            Shared.Main.ApplicationMultiInstanceMutex.Dispose();
        }
        catch (Exception)
        {
            // ignored
        }

        try
        {
            // Unlock the crash file
            File.Delete(CrashFile.FullName);
        }
        catch (Exception)
        {
            // ignored
        }

        // We've (mostly) done what we had to
        IsExitHandled = true;

        try
        {
            // Disconnect all loaded devices
            AppPlugins.TrackingDevicesList.Values
                .ToList().ForEach(device => device.Shutdown());

            // Disconnect all loaded services
            AppPlugins.ServiceEndpointsList.Values
                .ToList().ForEach(service => service.Shutdown());
        }
        catch (Exception)
        {
            // ignored
        }
    }

    // Function to spawn default' enabled trackers
    public static async Task<bool> SpawnEnabledTrackers()
    {
        if (!AppTrackersSpawned)
        {
            Logger.Info("[Interfacing] Spawning all active-supported trackers now...");

            // Helper bool array
            List<bool> spawned = new();

            // Try 3 times (cause why not)
            if (AppData.Settings.TrackersVector.Count(x => x.IsActive) > 0)
                for (var i = 0; i < 3; i++)
                {
                    // Update tracker statuses in the server
                    spawned.AddRange((await AppPlugins.CurrentServiceEndpoint.SetTrackerStates(
                            AppData.Settings.TrackersVector.Where(x => x.IsActive).Select(x => x.GetTrackerBase())))
                        ?.Select(x => x.Success) ?? new[] { false }); // Check if the request was actually okay
                    await Task.Delay(15);
                }

            // If one or more trackers failed to spawn
            if (spawned.Count > 0 && spawned.Contains(false))
            {
                Logger.Info("One or more trackers couldn't be spawned after 3 tries. Giving up...");

                // Cause not checking anymore
                ServiceEndpointFailure = true;
                AppTrackersSpawned = false;
                AppTrackersInitialized = false;

                return false;
            }
        }

        // Notify that we're good now
        AppTrackersSpawned = true;
        AppTrackersInitialized = true;

        return true; // We're done here!
    }

    public static async Task<(int Code, string Message)> TestServiceConnection()
    {
        // Do not spawn 1000 voids, check how many do we have
        if (PingCheckingThreadsNumber <= MaxPingCheckingThreads)
        {
            // Add a new worker
            PingCheckingThreadsNumber += 1; // May be ++ too

            try
            {
                // Send a ping message and capture the data
                var result = await AppPlugins.CurrentServiceEndpoint.TestConnection();

                // Dump data to variables
                PingTime = result.PingTime;

                // Log ?success
                Logger.Info($"Connection test has ended, [result: {(result.Status == 0 ? "success" : "fail")}], " +
                            $"Message: {result.StatusMessage} Tested ping time: {PingTime} [ticks]");

                // Release
                PingCheckingThreadsNumber = Math.Clamp(
                    PingCheckingThreadsNumber - 1, 0, MaxPingCheckingThreads + 1);

                // Return the result
                return (AppPlugins.CurrentServiceEndpoint.ServiceStatus,
                    AppPlugins.CurrentServiceEndpoint.ServiceStatusString);
            }
            catch (Exception e)
            {
                // Log ?success
                Logger.Info($"Connection test has ended, [result: fail], got an exception: {e.Message}");

                // Release
                PingCheckingThreadsNumber = Math.Clamp(
                    PingCheckingThreadsNumber - 1, 0, MaxPingCheckingThreads + 1);

                return (3, "An exception occurred.");
            }
        }

        // else
        Logger.Error("Connection checking threads exceeds 3, aborting...");
        return (14, "Too many simultaneous checking threads.");
    }

    public static async void ServiceEndpointSetup()
    {
        // Refresh the server driver status
        if (!ServiceEndpointFailure)
        {
            (ServiceEndpointStatusCode, ServiceEndpointStatusString) = await TestServiceConnection();
        }
        else
        {
            ServiceEndpointStatusCode = 10; // Fatal [reserved]
            ServiceEndpointStatusString = LocalizedJsonString("/ServerStatuses/ServerFailure");
        }

        // Play an error sound if something's wrong
        if (!IsServiceEndpointPresent)
            AppSounds.PlayAppSound(AppSounds.AppSoundType.Error);

        else
            Shared.Main.DispatcherQueue.TryEnqueue(async () =>
            {
                // Sleep a bit before checking
                await Task.Delay(1000);

                if (Shared.General.ErrorWhatText is not null &&
                    Shared.General.ErrorWhatText.Visibility == Visibility.Visible)
                    AppSounds.PlayAppSound(AppSounds.AppSoundType.Error);
            });

        // LOG the status
        Logger.Info($"Current K2 Server status: {ServiceEndpointStatusString}");
    }

    public static (Vector3 Position, Quaternion Orientation)
        GetVrTrackerPoseCalibrated(string nameContains)
    {
        var result = AppPlugins.CurrentServiceEndpoint
            .GetTrackerPose(nameContains, false);

        return (result?.Position ?? Vector3.Zero, // Check the result against null, return
            result?.Orientation ??
            AppPlugins.CurrentServiceEndpoint.HeadsetPose?.Orientation ?? Quaternion.Identity);
    }

    public static void UpdateServerStatus()
    {
        // Block some things if server isn't working properly
        if (Shared.General.ErrorWhatText is null || IsServiceEndpointPresent) return;
        Logger.Info("[Server Error] Entering the server error state...");

        // Hide device error labels (if any)
        Shared.General.ErrorWhatText.Visibility = Visibility.Collapsed;
        Shared.General.ErrorWhatGrid.Visibility = Visibility.Collapsed;
        Shared.General.ErrorButtonsGrid.Visibility = Visibility.Collapsed;
        Shared.General.TrackingDeviceErrorLabel.Visibility = Visibility.Collapsed;

        // Block spawn|offsets|calibration buttons
        Shared.General.ToggleTrackersButton.IsEnabled = false;
        Shared.General.CalibrationButton.IsEnabled = false;
        Shared.General.OffsetsButton.IsEnabled = false;
    }

    [DllImport("user32.dll")]
    public static extern nint GetActiveWindow();

    public static bool IsCurrentWindowActive()
    {
        if (Shared.Main.AppWindow is null)
            return true; // Give up k?

        return GetActiveWindow() == Shared.Main.AppWindowId;
    }

    // Return a language name by code
    // Input: The current (or deduced) language key / en
    // Returns: LANG_NATIVE (LANG_LOCALIZED) / Nihongo (Japanese)
    // https://stackoverflow.com/a/10607146/13934610
    // https://stackoverflow.com/a/51867679/13934610
    public static string GetLocalizedLanguageName(string languageKey)
    {
        try
        {
            // Load the locales.json from Assets/Strings/
            var resourcePath = Path.Join(
                ProgramLocation.DirectoryName,
                "Assets", "Strings", "locales.json");

            if (File.Exists(GetAppDataFilePath("Localization.json")))
                try
                {
                    // Parse the loaded json
                    var defaults = JsonConvert.DeserializeObject<LocalizationSettings>(
                                       File.ReadAllText(GetAppDataFilePath("Localization.json"))) ??
                                   new LocalizationSettings();

                    if (defaults.AmethystStringsFolder is not null &&
                        Directory.Exists(defaults.AmethystStringsFolder))
                    {
                        Logger.Info($"Overwriting the app strings path with {defaults.AmethystStringsFolder}!");
                        resourcePath = Path.Join(defaults.AmethystStringsFolder, "locales.json");
                    }
                }
                catch (Exception e)
                {
                    Logger.Info($"Localization settings checkout failed! Message: {e.Message}");
                }
            else Logger.Info("No default localization settings found! [Localization.json]");

            // If the specified language doesn't exist somehow, fallback to 'en'
            if (!File.Exists(resourcePath))
            {
                Logger.Error("Could not load language enumeration resources at " +
                             $"\"{resourcePath}\", app interface will be broken!");
                return languageKey; // Give up on trying
            }

            // Parse the loaded json
            var jsonObject = JsonObject.Parse(File.ReadAllText(resourcePath));

            // Check if the resource root is fine
            if (jsonObject is null || jsonObject.Count <= 0)
            {
                Logger.Error("The current language enumeration resource root is empty! " +
                             "App interface will be broken!");
                return languageKey; // Give up on trying
            }

            // If the language key is the current language, don't split the name
            if (AppData.Settings.AppLanguage == languageKey)
                return jsonObject.GetNamedObject(AppData.Settings.AppLanguage)
                    .GetNamedString(AppData.Settings.AppLanguage);

            // Else split the same way as in docs
            return jsonObject.GetNamedObject(languageKey).GetNamedString(languageKey) +
                   " (" + jsonObject.GetNamedObject(AppData.Settings.AppLanguage).GetNamedString(languageKey) + ")";
        }
        catch (Exception e)
        {
            Logger.Error($"JSON error at key: \"{languageKey}\"! Message: {e.Message}");

            // Else return they key alone
            return languageKey;
        }
    }

    // Load the current desired resource JSON into app memory
    public static void LoadJsonStringResources(string languageKey)
    {
        try
        {
            Logger.Info($"Searching for language resources with key \"{languageKey}\"...");

            var resourcePath = Path.Join(
                ProgramLocation.DirectoryName,
                "Assets", "Strings", languageKey + ".json");

            if (File.Exists(GetAppDataFilePath("Localization.json")))
                try
                {
                    // Parse the loaded json
                    var defaults = JsonConvert.DeserializeObject<LocalizationSettings>(
                                       File.ReadAllText(GetAppDataFilePath("Localization.json"))) ??
                                   new LocalizationSettings();

                    if (defaults.AmethystStringsFolder is not null &&
                        Directory.Exists(defaults.AmethystStringsFolder))
                    {
                        Logger.Info($"Overwriting the app strings path with {defaults.AmethystStringsFolder}!");
                        resourcePath = Path.Join(defaults.AmethystStringsFolder, languageKey + ".json");
                    }
                }
                catch (Exception e)
                {
                    Logger.Info($"Localization settings checkout failed! Message: {e.Message}");
                }
            else Logger.Info("No default localization settings found! [Localization.json]");

            // If the specified language doesn't exist somehow, fallback to 'en'
            if (!File.Exists(resourcePath))
            {
                Logger.Warn("Could not load language resources at " +
                            $"\"{resourcePath}\", falling back to 'en' (en.json)!");

                resourcePath = Path.Join(
                    ProgramLocation.DirectoryName,
                    "Assets", "Strings", "en.json");
            }

            // If failed again, just give up
            if (!File.Exists(resourcePath))
            {
                Logger.Warn("Could not load language resources at " +
                            $"\"{resourcePath}\", the app interface will be broken!");
                return; // Just give up
            }

            // If everything's ok, load the resources into the current resource tree

            // Parse the loaded json
            _localResources = JsonConvert.DeserializeObject<LocalisationFileJson>(File.ReadAllText(resourcePath));

            // Check if the resource root is fine
            if (_localResources is null || !_localResources.Messages.Any())
                Logger.Error("The current resource root is empty! App interface will be broken!");
            else
                Logger.Info($"Successfully loaded language resources with key \"{languageKey}\"!");
        }
        catch (Exception e)
        {
            Logger.Error($"JSON error at key: \"{languageKey}\"! Message: {e.Message}");
        }
    }

    // Load the current desired resource JSON into app memory
    public static void LoadJsonStringResourcesEnglish()
    {
        try
        {
            Logger.Info("Searching for shared (English) language resources...");

            var resourcePath = Path.Join(
                ProgramLocation.DirectoryName,
                "Assets", "Strings", "en.json");

            if (File.Exists(GetAppDataFilePath("Localization.json")))
                try
                {
                    // Parse the loaded json
                    var defaults = JsonConvert.DeserializeObject<LocalizationSettings>(
                                       File.ReadAllText(GetAppDataFilePath("Localization.json"))) ??
                                   new LocalizationSettings();

                    if (defaults.AmethystStringsFolder is not null &&
                        Directory.Exists(defaults.AmethystStringsFolder))
                    {
                        Logger.Info($"Overwriting the app strings path with {defaults.AmethystStringsFolder}!");
                        resourcePath = Path.Join(defaults.AmethystStringsFolder, "en.json");
                    }
                }
                catch (Exception e)
                {
                    Logger.Info($"Localization settings checkout failed! Message: {e.Message}");
                }
            else Logger.Info("No default localization settings found! [Localization.json]");

            // If failed again, just give up
            if (!File.Exists(resourcePath))
            {
                Logger.Warn("Could not load language resources at \"{resourcePath}\", " +
                            "falling back to the current one! The app interface may be broken!");

                // Override the current english resource tree
                _englishResources = _localResources;
                return; // Just give up
            }

            // If everything's ok, load the resources into the current resource tree

            // Parse the loaded json
            _englishResources = JsonConvert.DeserializeObject<LocalisationFileJson>(File.ReadAllText(resourcePath));

            // Check if the resource root is fine
            if (_englishResources is null || !_englishResources.Messages.Any())
                Logger.Error("The current resource root is empty! App interface will be broken!");
            else
                Logger.Info("Successfully loaded language resources with key \"en\"!");
        }
        catch (Exception e)
        {
            Logger.Error($"JSON error at key: \"en\"! Message: {e.Message}");
        }
    }

    // Get a string from runtime JSON resources, language from settings
    public static string LocalizedJsonString(string resourceKey,
        [CallerLineNumber] int lineNumber = 0, [CallerFilePath] string filePath = "",
        [CallerMemberName] string memberName = "")
    {
        try
        {
            // Check if the resource root is fine
            if (_localResources?.Messages?.Any() ?? false)
                return _localResources.Messages.FirstOrDefault(x => x.Id == resourceKey, new LocalizedMessage
                    { Translation = EnglishJsonString(resourceKey, lineNumber, filePath, memberName) }).Translation;

            Logger.Error("The current resource root is empty! App interface will be broken!");
            return resourceKey; // Just give up
        }
        catch (Exception e)
        {
            Logger.Error($"JSON error at key: \"{resourceKey}\"! Message: {e.Message}\n" +
                         "Path of the local caller that requested the localized resource string: " +
                         $"{Path.GetFileName(filePath)}::{memberName}:{lineNumber}");

            // Else return they key alone
            return resourceKey;
        }
    }

    // Get a string from runtime JSON resources, language from settings
    public static string EnglishJsonString(string resourceKey,
        [CallerLineNumber] int lineNumber = 0, [CallerFilePath] string filePath = "",
        [CallerMemberName] string memberName = "")
    {
        try
        {
            // Check if the resource root is fine
            if (_englishResources.Messages?.Any(x => x.Id == resourceKey) ?? false)
                return _englishResources.Messages.FirstOrDefault(x => x.Id == resourceKey)?.Translation ?? resourceKey;

            Logger.Error("The current resource root is empty! App interface will be broken!");
            return resourceKey; // Just give up
        }
        catch (Exception e)
        {
            Logger.Error($"JSON error at key: \"{resourceKey}\"! Message: {e.Message}\n" +
                         "Path of the local caller that requested the localized resource string: " +
                         $"{Path.GetFileName(filePath)}::{memberName}:{lineNumber}");

            // Else return they key alone
            return resourceKey;
        }
    }

    // Restart Amethyst
    public static async Task ExecuteAppRestart(bool handleExit = true, string parameters = "", bool admin = false)
    {
        Logger.Info("Restart requested: trying to restart the app...");

        // If we've found who asked
        if (File.Exists(ProgramLocation.FullName))
        {
            // Log the caller
            Logger.Info($"The current caller process is: {ProgramLocation.FullName}");

            // Exit the app
            Logger.Info("Configuration has been reset, exiting in 500ms...");

            // Don't execute the exit routine
            IsExitHandled = true;

            if (handleExit) // Handle a typical exit
                await HandleAppExit(500);

            var info = new ProcessStartInfo
            {
                FileName = ProgramLocation.FullName.Replace(".dll", ".exe"),
                Arguments = parameters // Pass same args
            };

            if (admin)
            {
                info.UseShellExecute = true;
                info.Verb = "runas";
            }

            // Restart and exit with code 0
            if (FileUtils.IsCurrentProcessElevated() && !admin)
            {
                info.Arguments = "amethyst-app:";
                info.FileName = "explorer.exe";
            }

            try
            {
                // Exit without re-handling everything
                Process.Start(info);
                Environment.Exit(0);
            }
            catch (Exception e)
            {
                Logger.Fatal(e);
            }
        }

        // Still here?
        Logger.Fatal(new InvalidDataException("App will not be restarted due to caller process identification error."));
        if (!handleExit) return;

        ShowToast(LocalizedJsonString("/SharedStrings/Toasts/RestartFailed/Title"),
            LocalizedJsonString("/SharedStrings/Toasts/RestartFailed"));
        ShowServiceToast(LocalizedJsonString("/SharedStrings/Toasts/RestartFailed/Title"),
            LocalizedJsonString("/SharedStrings/Toasts/RestartFailed"));
    }

    public static class Plugins
    {
        public static (Vector3 Position, Quaternion Orientation) GetHmdPose =>
            AppPlugins.CurrentServiceEndpoint.HeadsetPose ?? (Vector3.Zero, Quaternion.Identity);

        public static string RequestLocalizedString(string key, string guid)
        {
            try
            {
                if (string.IsNullOrEmpty(guid) || (!AppPlugins.TrackingDevicesList.ContainsKey(guid) &&
                                                   !AppPlugins.ServiceEndpointsList.ContainsKey(guid) &&
                                                   AppPlugins.LoadAttemptedPluginsList.All(x => x.Guid != guid) &&
                                                   AppPlugins.InstallerPluginsList.All(x => x.Guid != guid)))
                {
                    Logger.Info("[Requested by UNKNOWN DEVICE CALLER] " +
                                "Null, empty or invalid GUID was passed to SetLocalizationResourcesRoot, aborting!");
                    return LocalizedJsonString(key); // Just give up
                }

                // Check if the request was from a device
                if (AppPlugins.TrackingDevicesList.TryGetValue(guid, out var device))
                {
                    // Check if the resource root is fine
                    var resourceRootDevice = device?.LocalizationResourcesRoot.Root;
                    if (resourceRootDevice?.Messages?.Any(x => x.Id == key) ?? false)
                        return resourceRootDevice.Messages.FirstOrDefault(x => x.Id == key)?.Translation ?? key;
                }

                // Check if the request was from a service
                if (AppPlugins.ServiceEndpointsList.TryGetValue(guid, out var service))
                {
                    // Check if the resource root is fine
                    var resourceRootService = service?.LocalizationResourcesRoot.Root;
                    if (resourceRootService?.Messages?.Any(x => x.Id == key) ?? false)
                        return resourceRootService.Messages.FirstOrDefault(x => x.Id == key)?.Translation ?? key;
                }

                // ReSharper disable once InvertIf | Check if the request was from a service
                if (AppPlugins.LoadAttemptedPluginsList.Any(x => x.Guid == guid))
                {
                    // Prepare the plugin object
                    var plugin = AppPlugins.LoadAttemptedPluginsList.First(x => x.Guid == guid);

                    // Check if the resource root is fine
                    var resourceRootService = plugin.LocalizationResourcesRoot.Root;
                    if (resourceRootService?.Messages?.Any(x => x.Id == key) ?? false)
                        return resourceRootService.Messages.FirstOrDefault(x => x.Id == key)?.Translation ?? key;
                }

                // ReSharper disable once InvertIf | Check if the request was from a service
                if (AppPlugins.InstallerPluginsList.Any(x => x.Guid == guid))
                {
                    // Prepare the plugin object
                    var plugin = AppPlugins.InstallerPluginsList.First(x => x.Guid == guid);

                    // Check if the resource root is fine
                    var resourceRootService = plugin.LocalizationResourcesRoot.Root;
                    if (resourceRootService?.Messages?.Any(x => x.Id == key) ?? false)
                        return resourceRootService.Messages.FirstOrDefault(x => x.Id == key)?.Translation ?? key;
                }

                // Still here?!? We're screwed!
                Logger.Error($"The resource root of plugin {guid} is empty! Its interface will be broken!");
                return LocalizedJsonString(key); // Just give up
            }
            catch (Exception e)
            {
                Logger.Error($"JSON error at key: \"{key}\"! Message: {e.Message}\n" +
                             $"GUID of the {{ get; }} caller device: {guid}");

                // Else return they key alone
                return key;
            }
        }

        public static bool SetLocalizationResourcesRoot(string rootPath, string guid)
        {
            try
            {
                var path = rootPath; // Make a local copy of the strings loc path
                Logger.Info($"[Requested by plugin with GUID {guid}] " +
                            $"Searching for language resources with key \"{AppData.Settings.AppLanguage}\"...");

                if (string.IsNullOrEmpty(guid) || (!AppPlugins.TrackingDevicesList.ContainsKey(guid) &&
                                                   !AppPlugins.ServiceEndpointsList.ContainsKey(guid) &&
                                                   AppPlugins.LoadAttemptedPluginsList.All(x => x.Guid != guid) &&
                                                   AppPlugins.InstallerPluginsList.All(x => x.Guid != guid)))
                {
                    Logger.Info("[Requested by UNKNOWN DEVICE CALLER] " +
                                "Null, empty or invalid GUID was passed to SetLocalizationResourcesRoot, aborting!");
                    return false; // Just give up
                }

                if (File.Exists(GetAppDataFilePath("Localization.json")))
                    try
                    {
                        // Parse the loaded json
                        var defaults = JsonConvert.DeserializeObject<LocalizationSettings>(
                                           File.ReadAllText(GetAppDataFilePath("Localization.json"))) ??
                                       new LocalizationSettings();

                        if ((defaults.PluginStringFolders?.TryGetValue(path, out var result) ?? false) &&
                            Directory.Exists(result))
                        {
                            Logger.Info($"Overwriting {guid}'s strings path with {result}!");
                            path = result;
                        }
                    }
                    catch (Exception e)
                    {
                        Logger.Info($"Localization settings checkout failed! Message: {e.Message}");
                    }
                else Logger.Info("No default localization settings found! [Localization.json]");

                if (!Directory.Exists(path))
                {
                    Logger.Info($"[Requested by plugin with GUID {guid}] " +
                                $"Could not find any language enumeration resources in \"{path}\"! " +
                                $"Interface of device {guid} will be broken!");
                    return false; // Just give up
                }

                var resourcePath = Path.Join(path, AppData.Settings.AppLanguage + ".json");

                // If the specified language doesn't exist somehow, fallback to 'en'
                if (!File.Exists(resourcePath))
                {
                    Logger.Warn($"[Requested by plugin with GUID {guid}] " +
                                "Could not load language resources at " +
                                $"\"{resourcePath}\", falling back to 'en' (en.json)!");

                    resourcePath = Path.Join(path, "en.json");
                }

                // If failed again, just give up
                if (!File.Exists(resourcePath))
                {
                    Logger.Error($"[Requested by plugin with GUID {guid}] " +
                                 $"Could not load language resources at \"{resourcePath}\"," +
                                 $"for device {guid}! Its interface will be broken!");
                    return false; // Just give up
                }

                // ReSharper disable once InvertIf | Check if the request was from a device
                if (AppPlugins.TrackingDevicesList.TryGetValue(guid, out var device) && device is not null)
                {
                    // Parse the loaded json
                    device.LocalizationResourcesRoot =
                        (JsonConvert.DeserializeObject<LocalisationFileJson>(File.ReadAllText(resourcePath)), path);

                    // Check if the resource root is fine
                    var resourceRoot = device.LocalizationResourcesRoot.Root;
                    if (!(resourceRoot?.Messages?.Any() ?? false))
                    {
                        Logger.Error($"[Requested by device with GUID {guid}] " +
                                     $"Could not load language resources at \"{resourcePath}\"," +
                                     $"for device {guid}! Its interface will be broken!");
                        return false; // Just give up
                    }

                    // Still here? 
                    Logger.Info($"[Requested by device with GUID {guid}] " +
                                "Successfully loaded language resources with key " +
                                $"\"{AppData.Settings.AppLanguage}\"!");

                    // Setup string hot reload watchdog
                    device.AssetsWatcher = new FileSystemWatcher
                    {
                        Path = device.LocalizationResourcesRoot.Directory,
                        NotifyFilter = NotifyFilters.CreationTime | NotifyFilters.FileName |
                                       NotifyFilters.LastWrite | NotifyFilters.DirectoryName,
                        IncludeSubdirectories = true,
                        Filter = "*.json",
                        EnableRaisingEvents = true
                    };

                    // Add event handlers : local
                    device.AssetsWatcher.Changed += device.AssetsChanged;
                    device.AssetsWatcher.Created += device.AssetsChanged;
                    device.AssetsWatcher.Deleted += device.AssetsChanged;
                    device.AssetsWatcher.Renamed += device.AssetsChanged;

                    return true; // Winning it, yay!
                }

                // ReSharper disable once InvertIf | Check if the request was from a service
                if (AppPlugins.ServiceEndpointsList.TryGetValue(guid, out var service) && service is not null)
                {
                    // Parse the loaded json
                    service.LocalizationResourcesRoot =
                        (JsonConvert.DeserializeObject<LocalisationFileJson>(File.ReadAllText(resourcePath)), path);

                    // Check if the resource root is fine
                    var resourceRoot = service.LocalizationResourcesRoot.Root;
                    if (!(resourceRoot?.Messages?.Any() ?? false))
                    {
                        Logger.Error($"[Requested by service with GUID {guid}] " +
                                     $"Could not load language resources at \"{resourcePath}\"," +
                                     $"for device {guid}! Its interface will be broken!");
                        return false; // Just give up
                    }

                    // Still here? 
                    Logger.Info($"[Requested by service with GUID {guid}] " +
                                "Successfully loaded language resources with key " +
                                $"\"{AppData.Settings.AppLanguage}\"!");

                    // Setup string hot reload watchdog
                    service.AssetsWatcher = new FileSystemWatcher
                    {
                        Path = service.LocalizationResourcesRoot.Directory,
                        NotifyFilter = NotifyFilters.CreationTime | NotifyFilters.FileName |
                                       NotifyFilters.LastWrite | NotifyFilters.DirectoryName,
                        IncludeSubdirectories = true,
                        Filter = "*.json",
                        EnableRaisingEvents = true
                    };

                    // Add event handlers : local
                    service.AssetsWatcher.Changed += service.AssetsChanged;
                    service.AssetsWatcher.Created += service.AssetsChanged;
                    service.AssetsWatcher.Deleted += service.AssetsChanged;
                    service.AssetsWatcher.Renamed += service.AssetsChanged;

                    return true; // Winning it, yay!
                }

                // ReSharper disable once InvertIf | Check if the request was from a service
                if (AppPlugins.LoadAttemptedPluginsList.Any(x => x.Guid == guid))
                {
                    // Prepare the plugin object
                    var plugin = AppPlugins.LoadAttemptedPluginsList.First(x => x.Guid == guid);

                    // Parse the loaded json
                    plugin.LocalizationResourcesRoot =
                        (JsonConvert.DeserializeObject<LocalisationFileJson>(File.ReadAllText(resourcePath)), path);

                    // Check if the resource root is fine
                    var resourceRoot = plugin.LocalizationResourcesRoot.Root;
                    if (!(resourceRoot?.Messages?.Any() ?? false))
                    {
                        Logger.Error($"[Requested by plugin with GUID {guid}] " +
                                     $"Could not load language resources at \"{resourcePath}\"," +
                                     $"for device {guid}! Its interface will be broken!");
                        return false; // Just give up
                    }

                    // Still here? 
                    Logger.Info($"[Requested by plugin with GUID {guid}] " +
                                "Successfully loaded language resources with key " +
                                $"\"{AppData.Settings.AppLanguage}\"!");

                    return true; // Winning it, yay!
                }

                // ReSharper disable once InvertIf | Check if the request was from a service
                if (AppPlugins.InstallerPluginsList.Any(x => x.Guid == guid))
                {
                    // Prepare the plugin object
                    var plugin = AppPlugins.InstallerPluginsList.First(x => x.Guid == guid);

                    // Parse the loaded json
                    plugin.LocalizationResourcesRoot =
                        (JsonConvert.DeserializeObject<LocalisationFileJson>(File.ReadAllText(resourcePath)), path);

                    // Check if the resource root is fine
                    var resourceRoot = plugin.LocalizationResourcesRoot.Root;
                    if (!(resourceRoot?.Messages?.Any() ?? false))
                    {
                        Logger.Error($"[Requested by plugin with GUID {guid}] " +
                                     $"Could not load language resources at \"{resourcePath}\"," +
                                     $"for device {guid}! Its interface will be broken!");
                        return false; // Just give up
                    }

                    // Still here? 
                    Logger.Info($"[Requested by plugin with GUID {guid}] " +
                                "Successfully loaded language resources with key " +
                                $"\"{AppData.Settings.AppLanguage}\"!");

                    return true; // Winning it, yay!
                }

                Logger.Error($"[Requested by plugin with GUID {guid}] " +
                             $"Could not load language resources at \"{resourcePath}\"," +
                             $"for plugin {guid}! Its interface will be broken! " +
                             "Either the GUID was invalid, or the plugin is broken.");

                return false; // Oof, what happened?
            }
            catch (Exception e)
            {
                Logger.Error($"[Requested by plugin with GUID {guid}] " +
                             $"JSON error at key: \"{AppData.Settings.AppLanguage}\"! Message: {e.Message}");
                return false; // Just give up
            }
        }

        public static void RefreshApplicationInterface()
        {
            // Parse the request - update
            Shared.Main.DispatcherQueue.TryEnqueue(() =>
            {
                // Force refresh all the valid pages
                Shared.Events.RequestInterfaceReload(false);

                // Optionally hide other status errors
                UpdateServerStatus();
            });
        }
    }

    public static void SetupNotificationManager()
    {
        try
        {
            Logger.Info("Registering for NotificationInvoked WinRT event...");
            if (!AppNotificationManager.IsSupported()) // Check for compatibility first
                throw new NotSupportedException("AppNotificationManager is not supported on this system!");

            // To ensure all Notification handling happens in this process instance, register for
            // NotificationInvoked before calling Register(). Without this a new process will
            // be launched to handle the notification.
            AppNotificationManager.Default.NotificationInvoked +=
                (_, notificationActivatedEventArgs) => { ProcessToastArguments(notificationActivatedEventArgs); };

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
    }
}