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
using Windows.Data.Json;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Media.Animation;
using Microsoft.Windows.AppNotifications;

namespace Amethyst.Classes;

public static class Interfacing
{
    public const uint MaxPingCheckingThreads = 3;

    public static bool
        IsExitingNow, // App closing check
        IsExitHandled; // If actions have been done

    public static readonly object UpdateLock = new();

    // App crash check
    public static FileInfo CrashFile;

    // Update check
    public static bool
        UpdateFound = false,
        UpdateOnClosed = false,
        CheckingUpdatesNow = false,
        UpdatingNow = false;

    // Position helpers for k2 devices -> GUID, Pose
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
    public static bool K2AppTrackersSpawned,
        AppTrackersInitialized;

    // Is the tracking paused
    public static bool IsTrackingFrozen = false;

    // Server checking threads number, max num of them
    public static uint PingCheckingThreadsNumber;

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
    private static JsonObject _localResources = new(), _englishResources = new();

    // Is NUX currently opened?
    public static bool IsNuxPending = false;

    // Flip defines for the base device - iteration persistent
    public static bool BaseFlip = false; // Assume non flipped

    // Flip defines for the override device - iteration persistent
    public static bool OverrideFlip = false; // Assume non flipped

    public static bool IsServiceEndpointPresent => ServiceEndpointStatusCode == 0;

    public static FileInfo GetProgramLocation()
    {
        return new FileInfo(Assembly.GetExecutingAssembly().Location);
    }

    public static DirectoryInfo GetK2AppDataTempDir()
    {
        return Directory.CreateDirectory(Path.GetTempPath() + "Amethyst");
    }

    public static string GetK2AppDataFileDir(string relativeFilePath)
    {
        Directory.CreateDirectory(Path.Join(
            Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "Amethyst"));

        return Path.Join(Environment.GetFolderPath(
            Environment.SpecialFolder.ApplicationData), "Amethyst", relativeFilePath);
    }

    public static string GetK2AppDataLogFileDir(string relativeFolderName, string relativeFilePath)
    {
        Directory.CreateDirectory(Path.Join(
            Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
            "Amethyst", "logs", relativeFolderName));

        return Path.Join(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
            "Amethyst", "logs", relativeFolderName, relativeFilePath);
    }

    // Fail with an exit code (don't delete .crash)
    public static void Fail(string message)
    {
        IsExitHandled = true;

        // Find the crash handler and show it with a custom message
        var hPath = Path.Combine(GetProgramLocation().DirectoryName!, "K2CrashHandler", "K2CrashHandler.exe");
        if (File.Exists(hPath)) Process.Start(hPath, new[] { "message", message });
        else Logger.Warn("Crash handler exe (./K2CrashHandler/K2CrashHandler.exe) not found!");

        Environment.Exit(0);
    }

    // Show SteamVR toast / notification
    public static void ShowServiceToast(string header, string text)
    {
        TrackingDevices.CurrentServiceEndpoint.DisplayToast((header, text));
    }

    // Show an app toast / notification
    public static void ShowToast(string header, string text,
        bool highPriority = false, string action = "none")
    {
        if (string.IsNullOrEmpty(header) ||
            string.IsNullOrEmpty(text)) return;

        var payload =
            $"<toast launch=\"action={action}&amp;actionId=00000\">" +
            "<visual><binding template = \"ToastGeneric\">" +
            $"<text>{header}</text>" +
            $"<text>{text}</text>" +
            "</binding></visual></toast>";

        AppNotification toast = new(payload)
        {
            Tag = "Tag_AmethystNotifications",
            Group = "Group_AmethystNotifications",
            Priority = highPriority
                ? AppNotificationPriority.High
                : AppNotificationPriority.Default
        };

        Shared.Main.NotificationManager.Show(toast);
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
            TrackingDevices.TrackingDevicesList.Values
                .ToList().ForEach(device => device.Shutdown());

            // Disconnect all loaded services
            TrackingDevices.ServiceEndpointsList.Values
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
        if (!K2AppTrackersSpawned)
        {
            Logger.Info("[K2Interfacing] Registering trackers now...");

            // K2Driver is now auto-adding default lower body trackers.
            // That means that ids are: W-0 L-1 R-2
            // We may skip downloading them then ^_~

            Logger.Info("[K2Interfacing] App will be using K2Driver's default prepended trackers!");

            // Helper bool array
            List<bool> spawned = new();

            // Try 3 times (cause why not)
            if (AppData.Settings.TrackersVector.Count(x => x.IsActive) > 0)
                for (var i = 0; i < 3; i++)
                {
                    // Update tracker statuses in the server
                    spawned.AddRange((await TrackingDevices.CurrentServiceEndpoint.SetTrackerStates(
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
                K2AppTrackersSpawned = false;
                AppTrackersInitialized = false;

                return false;
            }
        }

        // Notify that we're good now
        K2AppTrackersSpawned = true;
        AppTrackersInitialized = true;

        /*
         * Trackers are stealing input from controllers when first added,
         * due to some weird wonky stuff happening and OpenVR not expecting them.
         * We're gonna de-spawn them for 8 frames (100ms) and re-spawn after another
         */

        await Task.Delay(100);
        AppTrackersInitialized = false;
        await Task.Delay(500);
        AppTrackersInitialized = true;

        return true;
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
                var result = await TrackingDevices.CurrentServiceEndpoint.TestConnection();

                // Dump data to variables
                PingTime = result.PingTime;

                // Log ?success
                Logger.Info($"Connection test has ended, [result: {(result.Status == 0 ? "success" : "fail")}], " +
                            $"Message: {result.StatusMessage} Tested ping time: {PingTime} [ticks]");

                // Release
                PingCheckingThreadsNumber = Math.Clamp(
                    PingCheckingThreadsNumber - 1, 0, MaxPingCheckingThreads + 1);

                // Return the result
                return (TrackingDevices.CurrentServiceEndpoint.ServiceStatus,
                    TrackingDevices.CurrentServiceEndpoint.ServiceStatusString);
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
        var result = TrackingDevices.CurrentServiceEndpoint
            .GetTrackerPose(nameContains, false);

        return (result?.Position ?? Vector3.Zero, // Check the result against null, return
            result?.Orientation ??
            TrackingDevices.CurrentServiceEndpoint.HeadsetPose?.Orientation ?? Quaternion.Identity);
    }

    public static void UpdateServerStatus()
    {
        // Check with this one, should be the same for all anyway
        if (Shared.General.ServerErrorWhatText is not null)
        {
            Shared.General.ServerErrorWhatText.Visibility =
                IsServiceEndpointPresent ? Visibility.Collapsed : Visibility.Visible;
            Shared.General.ServerErrorWhatGrid.Visibility =
                IsServiceEndpointPresent ? Visibility.Collapsed : Visibility.Visible;
            Shared.General.ServerErrorLabel.Visibility =
                IsServiceEndpointPresent ? Visibility.Collapsed : Visibility.Visible;
            Shared.General.ServiceSettingsButton.Visibility =
                IsServiceEndpointPresent ? Visibility.Collapsed : Visibility.Visible;

            // Split status and message by \n
            var message = StringUtils.SplitStatusString(ServiceEndpointStatusString);
            if (message is null || message.Length < 3)
                message = new[] { "The status message was broken!", "E_FIX_YOUR_SHIT", "AAAAA" };

            Shared.General.ServerStatusLabel.Text = message[0];
            Shared.General.ServerErrorLabel.Text = message[1];
            Shared.General.ServerErrorWhatText.Text = message[2];
        }

        // Block some things if server isn't working properly
        if (IsServiceEndpointPresent)
        {
            Shared.General.ToggleTrackersButton.IsEnabled = true;
            Shared.General.OffsetsButton.IsEnabled = true;
            return; // Unlock spawn|offsets|calibration buttons
        }

        Logger.Error("An error occurred and the app couldn't connect to K2 Server. " +
                     "Please check the upper message for more info.");

        if (Shared.General.ErrorWhatText is null) return;
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
                GetProgramLocation().DirectoryName,
                "Assets", "Strings", "locales.json");

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
                GetProgramLocation().DirectoryName,
                "Assets", "Strings", languageKey + ".json");

            // If the specified language doesn't exist somehow, fallback to 'en'
            if (!File.Exists(resourcePath))
            {
                Logger.Warn("Could not load language resources at " +
                            $"\"{resourcePath}\", falling back to 'en' (en.json)!");

                resourcePath = Path.Join(
                    GetProgramLocation().DirectoryName,
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
            _localResources = JsonObject.Parse(File.ReadAllText(resourcePath));

            // Check if the resource root is fine
            if (_localResources is null || _localResources.Count <= 0)
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
                GetProgramLocation().DirectoryName,
                "Assets", "Strings", "en.json");

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
            _englishResources = JsonObject.Parse(File.ReadAllText(resourcePath));

            // Check if the resource root is fine
            if (_englishResources is null || _englishResources.Count <= 0)
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
            if (_localResources is not null && _localResources.Count > 0)
                return _localResources.GetNamedString(resourceKey);

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

    public static class Plugins
    {
        public static (Vector3 Position, Quaternion Orientation) GetHmdPose =>
            TrackingDevices.CurrentServiceEndpoint.HeadsetPose ?? (Vector3.Zero, Quaternion.Identity);

        public static string RequestLocalizedString(string key, string guid)
        {
            try
            {
                if (string.IsNullOrEmpty(guid) || (!TrackingDevices.TrackingDevicesList.ContainsKey(guid) &&
                                                   !TrackingDevices.ServiceEndpointsList.ContainsKey(guid)))
                {
                    Logger.Info("[Requested by UNKNOWN DEVICE CALLER] " +
                                "Null, empty or invalid GUID was passed to SetLocalizationResourcesRoot, aborting!");
                    return LocalizedJsonString(key); // Just give up
                }

                // Check if the request was from a device
                if (TrackingDevices.TrackingDevicesList.TryGetValue(guid, out var device))
                {
                    // Check if the resource root is fine
                    var resourceRootDevice = device?.LocalizationResourcesRoot.Root;
                    if (resourceRootDevice is not null && resourceRootDevice.Count > 0)
                        return resourceRootDevice.GetNamedString(key); // Return if ok
                }

                // Check if the request was from a service
                if (TrackingDevices.ServiceEndpointsList.TryGetValue(guid, out var service))
                {
                    // Check if the resource root is fine
                    var resourceRootService = service?.LocalizationResourcesRoot.Root;
                    if (resourceRootService is not null && resourceRootService.Count > 0)
                        return resourceRootService.GetNamedString(key); // Return if ok
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

        public static bool SetLocalizationResourcesRoot(string path, string guid)
        {
            try
            {
                Logger.Info($"[Requested by plugin with GUID {guid}] " +
                            $"Searching for language resources with key \"{AppData.Settings.AppLanguage}\"...");

                if (string.IsNullOrEmpty(guid) || (!TrackingDevices.TrackingDevicesList.ContainsKey(guid) &&
                                                   !TrackingDevices.ServiceEndpointsList.ContainsKey(guid)))
                {
                    Logger.Info("[Requested by UNKNOWN DEVICE CALLER] " +
                                "Null, empty or invalid GUID was passed to SetLocalizationResourcesRoot, aborting!");
                    return false; // Just give up
                }

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
                if (TrackingDevices.TrackingDevicesList.TryGetValue(guid, out var device) && device is not null)
                {
                    // Parse the loaded json
                    device.LocalizationResourcesRoot =
                        (JsonObject.Parse(File.ReadAllText(resourcePath)), path);

                    // Check if the resource root is fine
                    var resourceRoot = device.LocalizationResourcesRoot.Root;
                    if (resourceRoot is null || resourceRoot.Count <= 0)
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

                    return true; // Winning it, yay!
                }

                // ReSharper disable once InvertIf | Check if the request was from a service
                if (TrackingDevices.ServiceEndpointsList.TryGetValue(guid, out var service) && service is not null)
                {
                    // Parse the loaded json
                    service.LocalizationResourcesRoot =
                        (JsonObject.Parse(File.ReadAllText(resourcePath)), path);

                    // Check if the resource root is fine
                    var resourceRoot = service.LocalizationResourcesRoot.Root;
                    if (resourceRoot is null || resourceRoot.Count <= 0)
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
            });
        }
    }
}