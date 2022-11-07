using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Linq.Expressions;
using System.Numerics;
using System.Reflection;
using System.Threading.Tasks;
using Microsoft.UI.Xaml.Media.Animation;
using Valve.VR;
using Amethyst.Plugins.Contract;
using Amethyst.Driver.Client;
using Amethyst.Utils;
using static System.Runtime.InteropServices.JavaScript.JSType;
using Microsoft.UI.Xaml.Documents;

namespace Amethyst.Classes;

public static class Interfacing
{
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

    // Internal version number
    public const string K2InternalVersion = "1.0.3.1";

    public const uint K2IntVersion = 3; // Amethyst version
    public const uint K2ApiVersion = 0; // API version

    public static bool
        IsExitingNow = false, // App closing check
        IsExitHandled = false; // If actions have been done

    // App crash check
    public static FileInfo CrashFile;

    // Update check
    public static bool
        UpdateFound = false,
        UpdateOnClosed = false,
        CheckingUpdatesNow = false,
        UpdatingNow = false;

    // Position helpers for k2 devices -> GUID, Pose
    public static SortedDictionary<string, Vector3>
        KinectHeadPosition, // But this one's kinect-only
        DeviceRelativeTransformOrigin; // This one applies to both

    // OpenVR playspace position
    public static Vector3 VrPlayspaceTranslation = new(0);

    // OpenVR playspace rotation
    public static Quaternion VrPlayspaceOrientationQuaternion = new(0, 0, 0, 1);

    // Current page string
    public static string CurrentPageTag = "general";
    public static string CurrentPageClass = "Amethyst.GeneralPage";

    // Current app state string (e.g. "general", "calibration_manual")
    public static string CurrentAppState = "general";

    // Currently available website language code
    public static string DocsLanguageCode = "en";

    // VR Overlay handle
    public static ulong VrOverlayHandle = OpenVR.k_ulOverlayHandleInvalid;
    public static uint VrNotificationId = 0;

    // The actual app theme (ONLY dark/light)
    public static Microsoft.UI.Xaml.ElementTheme ActualTheme =
        Microsoft.UI.Xaml.ElementTheme.Dark;

    // Application settings
    public static K2AppSettings AppSettings = new();

    // Input actions' handler
    public static K2EVRInput.SteamEVRInput EvrInput;

    // If trackers are added / initialized
    public static bool K2AppTrackersSpawned = false,
        K2AppTrackersInitialized = false;

    // Is the tracking paused
    public static bool IsTrackingFrozen = false;

    // Server checking threads number, max num of them
    public static uint PingCheckingThreadsNumber = 0,
        MaxPingCheckingThreads = 3;

    // Server interfacing data
    public static int ServerDriverStatusCode = 0;
    public static int ServerRpcStatusCode = 0;

    public static uint PingTime = 0, ParsingTime = 0;

    public static bool IsServerDriverPresent = false,
        ServerDriverFailure = false;

    public static string ServerStatusString = " \n \n ";

    // For manual calibration
    public static bool CalibrationConfirm,
        CalibrationModeSwap,
        CalibrationFineTune;

    // For manual calibration: L, R -> X, Y
    public static ((double X, double Y) LeftPosition,
        (double X, double Y) RightPosition)
        CalibrationJoystickPositions;

    // Check if we're currently scanning for trackers from other apps
    public static bool IsAlreadyAddedTrackersScanRunning = false;

    // If the already-added trackers check was requested
    public static bool AlreadyAddedTrackersScanRequested = false;

    // Fail with an exit code (don't delete .crash)
    public static void Fail(int code)
    {
        IsExitHandled = true;
        Environment.Exit(code);
    }
    
	// Show SteamVR toast / notification
	public static void ShowVRToast(string header, string text)
    {
        if (VrOverlayHandle == OpenVR.k_ulOverlayHandleInvalid ||
            string.IsNullOrEmpty(header) ||
            string.IsNullOrEmpty(text)) return;

		// Hide the current notification (if being shown)
		if (VrNotificationId != 0) // If valid
			OpenVR.Notifications.RemoveNotification(VrNotificationId);

        // Empty image data
        var notificationBitmap = new NotificationBitmap_t();
        
        // nullptr is the icon/image texture
        OpenVR.Notifications.CreateNotification(
			VrOverlayHandle, 0, EVRNotificationType.Transient,
			header + '\n' + text, EVRNotificationStyle.Application, 
            ref notificationBitmap, ref VrNotificationId);
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

        Microsoft.Windows.AppNotifications.AppNotification toast = new(payload)
        {
            Tag = "Tag_AmethystNotifications",
            Group = "Group_AmethystNotifications",
            Priority = highPriority
                ? Microsoft.Windows.AppNotifications.AppNotificationPriority.High
                : Microsoft.Windows.AppNotifications.AppNotificationPriority.Default
        };

        Shared.Main.NotificationManager.Show(toast);
    }

    public static void ProcessToastArguments(
        Microsoft.Windows.AppNotifications.AppNotificationActivatedEventArgs eventArgs)
    {
        // When a tracker's been auto-disabled
        if (eventArgs.Argument.Contains("focus_trackers")) throw new NotImplementedException("SettingsPage");

        // When you need to restart OpenVR
        if (eventArgs.Argument.Contains("focus_restart")) throw new NotImplementedException("SettingsPage");

        // When you've entered the cheater mode
        if (eventArgs.Argument.Contains("okashi"))
            Shared.Main.DispatcherQueue.TryEnqueue(() =>
            {
                // Navigate to the okashi/console page
                Shared.Main.MainNavigationView.SelectedItem =
                    Shared.Main.MainNavigationView.MenuItems[4];

                Shared.Main.NavigateToPage("console",
                    new EntranceNavigationTransitionInfo());
            });

        // Else no click action requested ("none")
    }

    public static async void HandleAppExit(int sleepMillis)
    {
        // Mark exiting as true
        IsExitingNow = true;
        Logger.Info("AppWindow.Closing handler called, starting the shutdown routine...");

        // Mark trackers as inactive
        K2AppTrackersInitialized = false;

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
            throw new NotImplementedException("Devices");
        }
        catch (Exception)
        {
            // ignored
        }
    }

    // Forward-declared from JointSelectorExpander.h
    public static void CheckDisabledJoints()
    {
        throw new NotImplementedException("Devices");
    }

    // Controllers' ID's (vr::k_unTrackedDeviceIndexInvalid for non-existent)
    public static (uint Left, uint Right) VrControllerIndexes;

    // Devices may request an explicit status refresh
    public static bool StatusUiRefreshRequested = false;
    public static bool StatusUiRefreshRequestedUrgent = false;

    // Is NUX currently opened?
    public static bool IsNuxPending = false;

    // Flip defines for the base device - iteration persistent
    public static bool BaseFlip = false; // Assume non flipped

    // Flip defines for the override device - iteration persistent
    public static bool OverrideFlip = false; // Assume non flipped

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

            // Create a dummy update vector
            List<(TrackerType Role, bool State)> trackerStatuses =
                (from tracker in AppSettings.K2TrackersVector
                    where tracker.IsActive
                    select (tracker.Role, true)).ToList();

            // Try 3 times (cause why not)
            if (trackerStatuses.Count > 0)
                for (var i = 0; i < 3; i++)
                {
                    // Update tracker statuses in the server
                    spawned.AddRange(DriverClient.UpdateTrackerStates(trackerStatuses).Select(x => x.State));
                    await Task.Delay(15);
                }

            // If one or more trackers failed to spawn
            if (spawned.Count > 0 && spawned.Contains(true))
            {
                Logger.Info("One or more trackers couldn't be spawned after 3 tries. Giving up...");

                // Cause not checking anymore
                ServerDriverFailure = true;
                K2AppTrackersSpawned = false;
                K2AppTrackersInitialized = false;

                return false;
            }
        }

        // Notify that we're good now
        K2AppTrackersSpawned = true;
        K2AppTrackersInitialized = true;

        /*
         * Trackers are stealing input from controllers when first added,
         * due to some weird wonky stuff happening and OpenVR not expecting them.
         * We're gonna de-spawn them for 8 frames (100ms) and re-spawn after another
         */

        await Task.Delay(100);
        K2AppTrackersInitialized = false;
        await Task.Delay(500);
        K2AppTrackersInitialized = true;

        return true;
    }

    public static async Task<bool> OpenVRStartup()
	{
		Logger.Info("Attempting connection to VRSystem... ");
        
        Task<(CVRSystem system, EVRInitError error)> vrTask = new(() =>
        {
            var eError = EVRInitError.None;
            return (OpenVR.Init(ref eError, EVRApplicationType.VRApplication_Overlay), eError);
        });

        Logger.Info("Waiting for the VR System to initialize...");

        if (await Task.WhenAny(vrTask, Task.Delay(5000)) == vrTask)
        {
            // We're good to go!
            Logger.Info("The async future reports that the VR System is ready!");
        }
        else
        {
            Logger.Error("The VR System took too long to initialize, giving up!");
            Environment.FailFast("The VR System took too long to initialize");
        }

		if (vrTask.Result.error != EVRInitError.None)
		{
			Logger.Error($"IVRSystem could not be initialized: EVRInitError Code {vrTask.Result.error}");
			return false; // Catastrophic failure!
		}

		// Initialize the overlay
        OpenVR.Overlay.CreateOverlay("k2vr.amethyst.desktop", "Amethyst", ref VrOverlayHandle);

		// Since we're ok, capture playspace details
        var trackingOrigin = vrTask.Result.system.GetRawZeroPoseToStandingAbsoluteTrackingPose();
        VrPlayspaceTranslation = TypeUtils.ExtractVrPosition(ref trackingOrigin);
		VrPlayspaceOrientationQuaternion = TypeUtils.ExtractVrRotation(ref trackingOrigin);
        
		// Rescan controller ids
		VrControllerIndexes = (
			OpenVR.System.GetTrackedDeviceIndexForControllerRole(
				ETrackedControllerRole.LeftHand),

            OpenVR.System.GetTrackedDeviceIndexForControllerRole(
				ETrackedControllerRole.RightHand)
		);

		Logger.Info($"VR Playspace translation: \n{VrPlayspaceTranslation}");
        Logger.Info($"VR Playspace orientation: \n{VrPlayspaceOrientationQuaternion}");
        return true; // OK
	}

    public static bool EvrActionsStartup()
    {
        Logger.Info("Attempting to set up EVR Input Actions...");

        if (!EvrInput.InitInputActions())
        {
            Logger.Error("Could not set up Input Actions. Please check the upper log for further information.");
            ShowVRToast("EVR Input Actions Init Failure!",
                "Couldn't set up Input Actions. Please check the log file for further information.");

            return false;
        }

        Logger.Info("EVR Input Actions set up OK");
        return true;
    }

    public static uint InstallVrApplicationManifest()
    {
        if (OpenVR.Applications.IsApplicationInstalled("KinectToVR.Amethyst"))
        {
            Logger.Info("Amethyst manifest is already installed");
            return 1;
        }
        if (File.Exists(Path.Join(GetProgramLocation().DirectoryName, "Amethyst.vrmanifest")))
        {
            var appError = OpenVR.Applications.AddApplicationManifest(
                Path.Join(GetProgramLocation().DirectoryName, "Amethyst.vrmanifest"), false);

            if (appError != EVRApplicationError.None)
            {
                Logger.Warn($"Amethyst manifest not installed! Error: {appError}");
                return 2;
            }

            Logger.Info($"Amethyst manifest installed at: {
                Path.Join(GetProgramLocation().DirectoryName, "Amethyst.vrmanifest")}");
            return 1;
        }

        Logger.Warn("Amethyst vr manifest (./Amethyst.vrmanifest) not found!");
        return 0;
    }

    public static void UninstallApplicationManifest()
    {
        if (OpenVR.Applications.IsApplicationInstalled("KinectToVR.Amethyst"))
        {
            OpenVR.Applications.RemoveApplicationManifest(
                Path.Join(GetProgramLocation().DirectoryName, "Amethyst.vrmanifest"));
            
            Logger.Info($"ttempted to remove Amethyst manifest at: {
                Path.Join(GetProgramLocation().DirectoryName, "Amethyst.vrmanifest")}");
        }
        if (OpenVR.Applications.IsApplicationInstalled("KinectToVR.Amethyst"))
            Logger.Warn("Amethyst manifest removal failed! It may have been installed from somewhere else too");
        else
            Logger.Info("Amethyst manifest removal succeed");
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
            var jsonObject = Windows.Data.Json.JsonObject.Parse(File.ReadAllText(resourcePath));

            // Check if the resource root is fine
            if (jsonObject == null || jsonObject.Count <= 0)
            {
                Logger.Error("The current language enumeration resource root is empty! " +
                             "App interface will be broken!");
                return languageKey; // Give up on trying
            }

            // If the language key is the current language, don't split the name
            if (AppSettings.AppLanguage == languageKey)
                return jsonObject.GetNamedObject(AppSettings.AppLanguage).GetNamedString(AppSettings.AppLanguage);

            // Else split the same way as in docs
            return jsonObject.GetNamedObject(languageKey).GetNamedString(languageKey) +
                   " (" + jsonObject.GetNamedObject(AppSettings.AppLanguage).GetNamedString(languageKey) + ")";
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
                Logger.Warn($"Could not load language resources at " +
                            $"\"{resourcePath}\", falling back to 'en' (en.json)!");

                resourcePath = Path.Join(
                    GetProgramLocation().DirectoryName,
                    "Assets", "Strings", "en.json");
            }

            // If failed again, just give up
            if (!File.Exists(resourcePath))
            {
                Logger.Warn($"Could not load language resources at " +
                            $"\"{resourcePath}\", the app interface will be broken!");
                return; // Just give up
            }

            // If everything's ok, load the resources into the current resource tree

            // Parse the loaded json
            LocalResources = Windows.Data.Json.JsonObject.Parse(File.ReadAllText(resourcePath));

            // Check if the resource root is fine
            if (LocalResources == null || LocalResources.Count <= 0)
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
    public static void LoadJSONStringResources_English()
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
                EnglishResources = LocalResources;
                return; // Just give up
            }

            // If everything's ok, load the resources into the current resource tree

            // Parse the loaded json
            EnglishResources = Windows.Data.Json.JsonObject.Parse(File.ReadAllText(resourcePath));

            // Check if the resource root is fine
            if (EnglishResources == null || EnglishResources.Count <= 0)
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
    public static string LocalizedJsonString(string resourceKey)
    {
        try
        {
            // Check if the resource root is fine
            if (LocalResources != null && LocalResources.Count > 0)
                return LocalResources.GetNamedString(resourceKey);

            Logger.Error("The current resource root is empty! App interface will be broken!");
            return resourceKey; // Just give up
        }
        catch (Exception e)
        {
            Logger.Error($"JSON error at key: \"{resourceKey}\"! Message: {e.Message}");

            // Else return they key alone
            return resourceKey;
        }
    }

    // Amethyst language resource trees
    public static Windows.Data.Json.JsonObject
        LocalResources = new(), EnglishResources = new(), LanguageEnum = new();
}