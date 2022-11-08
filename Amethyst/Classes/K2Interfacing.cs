using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Linq.Expressions;
using System.Numerics;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Amethyst.Driver.API;
using Microsoft.UI.Xaml.Media.Animation;
using Valve.VR;
using Amethyst.Plugins.Contract;
using Amethyst.Driver.Client;
using Amethyst.Utils;
using Grpc.Core;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Documents;
using System.Runtime.InteropServices;

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
    public static ElementTheme ActualTheme =
        ElementTheme.Dark;

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
    public static uint PingCheckingThreadsNumber = 0;
    public const uint MaxPingCheckingThreads = 3;

    // Server interfacing data
    public static int ServerDriverStatusCode = 0;
    public static int ServerRpcStatusCode = 0;

    public static long PingTime = 0, ParsingTime = 0;

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

    // HMD pose in OpenVR
    public static (Vector3 Position, Quaternion Orientation)
        RawVrHmdPose = new(Vector3.Zero, Quaternion.Identity);

    // Amethyst language resource trees
    public static Windows.Data.Json.JsonObject
        LocalResources = new(), EnglishResources = new(), LanguageEnum = new();

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

        // null is the icon/image texture
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

    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool SetForegroundWindow(IntPtr hWnd);

    public static void ProcessToastArguments(
        Microsoft.Windows.AppNotifications.AppNotificationActivatedEventArgs eventArgs)
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
                    Shared.Settings.PageMainScrollViewer.ChangeView(
                        0, Shared.Settings.PageMainScrollViewer.ExtentHeight, 0);

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
                    Shared.Settings.PageMainScrollViewer.ChangeView(
                        0, Shared.Settings.PageMainScrollViewer.ExtentHeight, 0);

                    await Task.Delay(500);

                    // Focus on the restart button
                    Shared.Settings.RestartButton.Focus(FocusState.Keyboard);
                });

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

    public static void CheckDisabledJoints()
    {
        // Ditch this if not loaded yet
        if (Shared.Devices.JointExpanderHostStackPanel == null) return;
        Shared.Devices.DevicesJointsSetupPending = true;

        // Optionally fix combos for disabled trackers -> joint selectors for base
        //foreach (const auto&expander : jointSelectorExpanders)
        //for (std::shared_ptr<JointSelectorRow> & row : *expander->JointSelectorRows())
        //{
        //    Helpers::SetComboBoxIsEnabled_Safe(
        //        row->TrackerCombo(),
        //        row->Tracker()->data_isActive);

        //    if (!row->Tracker()->data_isActive)
        //        Helpers::SelectComboBoxItem_Safe(
        //            row->TrackerCombo(), -1); // Placeholder
        //}

        //// Optionally fix combos for disabled trackers -> joint selectors for override
        //for (const auto&expander : overrideSelectorExpanders)
        //expander->UpdateIsEnabled();

        throw new NotImplementedException("MVVM");

        Shared.Devices.DevicesJointsSetupPending = false;
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

    public static async Task<Status> TestK2ServerConnection()

    {
        // Do not spawn 1000 voids, check how many do we have
        if (PingCheckingThreadsNumber <= MaxPingCheckingThreads)
        {
            // Add a new worker
            PingCheckingThreadsNumber += 1; // May be ++ too

            try
            {
                // Send a ping message and capture the data
                var result = await DriverClient.TestConnection();

                // Dump data to variables
                PingTime = result.ElpasedTime;
                ParsingTime = result.ReceiveTimestamp - result.SendTimestamp;

                // Log ?success
                Logger.Info($"Connection test has ended, [result: {
                    (result.Status.StatusCode == StatusCode.OK ? "success" : "fail")}]");

                // Log some data if needed
                Logger.Info($"\nTested ping time: {PingTime} [micros], " +
                            $"call/parsing time: {result.ReceiveTimestamp} [micros], " +
                            $"flight-back time: {DateTime.Now.Ticks - result.ReceiveTimestamp} [micros]");

                // Release
                PingCheckingThreadsNumber = Math.Clamp(
                    PingCheckingThreadsNumber - 1, 0, MaxPingCheckingThreads + 1);

                // Return the result
                return result.Status;
            }
            catch (Exception e)
            {
                // Log ?success
                Logger.Info("Connection test has ended, [result: fail], got an exception");

                // Release
                PingCheckingThreadsNumber = Math.Clamp(
                    PingCheckingThreadsNumber - 1, 0, MaxPingCheckingThreads + 1);

                return new Status(StatusCode.Unknown, "An exception occurred.");
            }
        }

        // else
        Logger.Error("Connection checking threads exceeds 3, aborting...");
        return new Status(StatusCode.Unavailable, "Too many simultaneous checking threads.");
    }

    public static async Task<(int ServerStatus, int APIStatus)> CheckK2ServerStatus()
    {
        // Don't check if already ok
        if (IsServerDriverPresent) return (1, (int)StatusCode.OK);

        try
        {
            /* Initialize the port */
            Logger.Info("Initializing the server IPC...");
            ;
            var initCode = DriverClient.InitAmethystServer();
            Status serverStatus = new();

            Logger.Info($"Server IPC initialization {
                (initCode == 0 ? "succeed" : "failed")}, exit code: {initCode}");

            /* Connection test and display ping */
            // We may wait a bit for it though...
            // ReSharper disable once InvertIf
            if (initCode == 0)
            {
                Logger.Info("Testing the connection...");

                for (var i = 0; i < 3; i++)
                {
                    Logger.Info($"Starting the test no {i + 1}...");
                    serverStatus = await TestK2ServerConnection();

                    // Not direct assignment since it's only a one-way check
                    if (serverStatus.StatusCode == StatusCode.OK)
                        IsServerDriverPresent = true;

                    else
                        Logger.Warn("Server status check failed! " +
                                    $"Code: {serverStatus.StatusCode}, " +
                                    $"Details: {serverStatus.Detail}");
                }
            }

            return initCode == 0
                // If the API is ok
                ? serverStatus.StatusCode == StatusCode.OK
                    // If the server is/isn't ok
                    ? (1, (int)serverStatus.StatusCode)
                    : (-1, (int)serverStatus.StatusCode)
                // If the API is not ok
                : (initCode, (int)StatusCode.Unknown);
        }
        catch (Exception e)
        {
            Logger.Warn("Server status check failed! " +
                        $"Exception: {e.Message}");

            return (-10, (int)StatusCode.Unknown);
        }

        /*
         * codes:
            all ok: 1
            server could not be reached: -1
            exception when trying to reach: -10
            could not create rpc channel: -2
            could not create rpc stub: -3

            fatal run-time failure: 10
         */
    }

    public static async Task K2ServerDriverRefresh()
    {
        if (!ServerDriverFailure)
            (ServerDriverStatusCode, ServerRpcStatusCode) = await CheckK2ServerStatus();
        else // Overwrite the status
            ServerDriverStatusCode = 10; // Fatal

        IsServerDriverPresent = false; // Assume fail
        ServerStatusString = LocalizedJsonString("/ServerStatuses/WTF");
        //"COULD NOT CHECK STATUS (\u15dc\u02ec\u15dc)\nE_WTF\nSomething's fucked a really big time.";

        switch (ServerDriverStatusCode)
        {
            case 1:
                ServerStatusString = LocalizedJsonString("/ServerStatuses/Success");
                //"Success! (Code 1)\nI_OK\nEverything's good!";

                IsServerDriverPresent = true;
                break; // Change to success

            case -1:
                ServerStatusString = LocalizedJsonString("/ServerStatuses/ConnectionError")
                    .Replace("{0}", ServerRpcStatusCode.ToString());
                //"SERVER CONNECTION ERROR (Code -1:{0})\nE_CONNECTION_ERROR\nCheck SteamVR add-ons (NOT overlays) and enable Amethyst.";
                break;

            case -10:
                ServerStatusString = LocalizedJsonString("/ServerStatuses/Exception")
                    .Replace("{0}", ServerRpcStatusCode.ToString());
                //"EXCEPTION WHILE CHECKING (Code -10)\nE_EXCEPTION_WHILE_CHECKING\nCheck SteamVR add-ons (NOT overlays) and enable Amethyst.";
                break;

            case -2:
                ServerStatusString = LocalizedJsonString("/ServerStatuses/RPCChannelFailure")
                    .Replace("{0}", ServerRpcStatusCode.ToString());
                //"RPC CHANNEL FAILURE (Code -2:{0})\nE_RPC_CHAN_FAILURE\nCould not connect to localhost:7135, is it already taken?";
                break;

            case -3:
                ServerStatusString = LocalizedJsonString("/ServerStatuses/RPCStubFailure")
                    .Replace("{0}", ServerRpcStatusCode.ToString());
                //"RPC/API STUB FAILURE (Code -3:{0})\nE_RPC_STUB_FAILURE\nCould not derive IK2DriverService! Is the protocol valid?";
                break;

            case 10:
                ServerStatusString = LocalizedJsonString("/ServerStatuses/ServerFailure");
                //"FATAL SERVER FAILURE (Code 10)\nE_FATAL_SERVER_FAILURE\nPlease restart, check logs and write to us on Discord.";
                break;

            default:
                ServerStatusString = LocalizedJsonString("/ServerStatuses/WTF");
                //"COULD NOT CHECK STATUS (\u15dc\u02ec\u15dc)\nE_WTF\nSomething's fucked a really big time.";
                break;
        }
    }

    public static async void K2ServerDriverSetup()
    {
        // Refresh the server driver status
        await K2ServerDriverRefresh();

        // Play an error sound if smth's wrong
        if (ServerDriverStatusCode != 1)
            AppSounds.PlayAppSound(AppSounds.AppSoundType.Error);

        else
            Shared.Main.DispatcherQueue.TryEnqueue(async () =>
            {
                // Sleep a bit before checking
                await Task.Delay(1000);

                if (Shared.General.ErrorWhatText != null &&
                    Shared.General.ErrorWhatText.Visibility == Visibility.Visible)
                    AppSounds.PlayAppSound(AppSounds.AppSoundType.Error);
            });

        // LOG the status
        Logger.Info($"Current K2 Server status: {ServerStatusString}");
    }

    public static (bool Found, uint Index) FindVrTracker(
        string role, bool canBeAme = true, bool log = true)
    {
        // Loop through all devices
        for (uint i = 0; i < OpenVR.k_unMaxTrackedDeviceCount; i++)
        {
            StringBuilder roleStringBuilder = new(1024);
            var roleError = ETrackedPropertyError.TrackedProp_Success;
            OpenVR.System.GetStringTrackedDeviceProperty(
                i, ETrackedDeviceProperty.Prop_ControllerType_String,
                roleStringBuilder, (uint)roleStringBuilder.Capacity, ref roleError);

            if (roleStringBuilder.Length <= 0)
                continue; // Don't waste our time

            // If we've found anything
            if (log) Logger.Info($"Found a device with roleHint: {roleStringBuilder}");

            // If we've actually found the one
            if (roleStringBuilder.ToString().IndexOf(role, StringComparison.OrdinalIgnoreCase) < 0) continue;

            var status = OpenVR.System.GetTrackedDeviceActivityLevel(i);
            if (status != EDeviceActivityLevel.k_EDeviceActivityLevel_UserInteraction &&
                status != EDeviceActivityLevel.k_EDeviceActivityLevel_UserInteraction_Timeout)
                continue;

            StringBuilder serialStringBuilder = new(1024);
            var serialError = ETrackedPropertyError.TrackedProp_Success;
            OpenVR.System.GetStringTrackedDeviceProperty(i, ETrackedDeviceProperty.Prop_SerialNumber_String,
                serialStringBuilder, (uint)serialStringBuilder.Capacity, ref serialError);

            // Log that we're finished
            if (log)
                Logger.Info($"Found an active {role} tracker with:\n    " +
                            $"hint: {roleStringBuilder}\n    " +
                            $"serial: {serialStringBuilder}\n    index: {i}");

            // Check if it's not ame
            var canReturn = true;
            if (!canBeAme) // If requested
                AppSettings.K2TrackersVector.Where(
                        tracker => serialStringBuilder.ToString() == tracker.Serial).ToList()
                    .ForEach(_ =>
                    {
                        if (log) Logger.Info("Skipping the latest found tracker because it's been added from Amethyst");
                        canReturn = false; // Maybe next time, bud
                    });

            // Return what we've got
            if (canReturn) return (true, i);
        }

        if (log)
            Logger.Warn($"Didn't find any {role} tracker in SteamVR " +
                        "with a proper role hint (Prop_ControllerType_String)");

        // We've failed if the loop's finished
        return (false, OpenVR.k_unTrackedDeviceIndexInvalid);
    }

    public static (Vector3 Position, Quaternion Orientation)
        GetVRTrackerPoseCalibrated(string nameContains, bool log)
    {
        var devicePose = new TrackedDevicePose_t[OpenVR.k_unMaxTrackedDeviceCount];
        OpenVR.System.GetDeviceToAbsoluteTrackingPose(
            ETrackingUniverseOrigin.TrackingUniverseStanding, 0, devicePose);

        var waistPair = FindVrTracker(nameContains, true, log);
        if (waistPair.Found)
        {
            // Extract pose from the returns
            // We don't care if it's invalid by any chance
            var waistPose = devicePose[waistPair.Index];

            // Get pos & rot
            return (
                Vector3.Transform(TypeUtils.ExtractVrPosition(
                        ref waistPose.mDeviceToAbsoluteTracking) - VrPlayspaceTranslation,
                    Quaternion.Inverse(VrPlayspaceOrientationQuaternion)),
                Quaternion.Inverse(VrPlayspaceOrientationQuaternion) *
                TypeUtils.ExtractVrRotation(ref waistPose.mDeviceToAbsoluteTracking)
            );
        }

        if (log)
            Logger.Warn("Either waist tracker doesn't exist or its role hint (Prop_ControllerType_String) was invalid");

        // We've failed if the executor got here
        return (Vector3.Zero, Quaternion.Identity);
    }

    public static void UpdateServerStatus()
    {
        // Disable UI (partially) if we've encountered an error
        if (Shared.Main.DevicesItem != null)
            Shared.Main.DevicesItem.IsEnabled = IsServerDriverPresent;

        // Check with this one, should be the same for all anyway
        if (Shared.General.ServerErrorWhatText != null)
        {
            Shared.General.ServerErrorWhatText.Visibility =
                IsServerDriverPresent ? Visibility.Collapsed : Visibility.Visible;
            Shared.General.ServerErrorWhatGrid.Visibility =
                IsServerDriverPresent ? Visibility.Collapsed : Visibility.Visible;
            Shared.General.ServerErrorButtonsGrid.Visibility =
                IsServerDriverPresent ? Visibility.Collapsed : Visibility.Visible;
            Shared.General.ServerErrorLabel.Visibility =
                IsServerDriverPresent ? Visibility.Collapsed : Visibility.Visible;

            // Split status and message by \n
            Shared.General.ServerStatusLabel.Text =
                StringUtils.SplitStatusString(ServerStatusString)[0];
            Shared.General.ServerErrorLabel.Text =
                StringUtils.SplitStatusString(ServerStatusString)[1];
            Shared.General.ServerErrorWhatText.Text =
                StringUtils.SplitStatusString(ServerStatusString)[2];

            // Optionally setup & show the re-register button
            Shared.General.ReRegisterButton.Visibility =
                ServerDriverStatusCode == -1
                    ? Visibility.Visible
                    : Visibility.Collapsed;

            Shared.General.ServerOpenDiscordButton.Height =
                ServerDriverStatusCode == -1 ? 40 : 65;
        }

        // Block some things if server isn't working properly
        if (IsServerDriverPresent) return;
        Logger.Error("An error occurred and the app couldn't connect to K2 Server. " +
                     "Please check the upper message for more info.");

        if (Shared.General.ErrorWhatText == null) return;
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

    // Update HMD pose from OpenVR -> called in K2Main
    public static void UpdateHMDPosAndRot()
    {
        // Capture RAW HMD pose
        var devicePose = new TrackedDevicePose_t[1]; // HMD only
        OpenVR.System.GetDeviceToAbsoluteTrackingPose(
            ETrackingUniverseOrigin.TrackingUniverseStanding, 0, devicePose);

        // Assert that HMD is at index 0
        if (OpenVR.System.GetTrackedDeviceClass(0) == ETrackedDeviceClass.HMD)
            RawVrHmdPose = (TypeUtils.ExtractVrPosition(ref devicePose[0].mDeviceToAbsoluteTracking),
                TypeUtils.ExtractVrRotation(ref devicePose[0].mDeviceToAbsoluteTracking));

        // Capture play-space details
        var trackingOrigin = OpenVR.System.GetRawZeroPoseToStandingAbsoluteTrackingPose();
        VrPlayspaceTranslation = TypeUtils.ExtractVrPosition(ref trackingOrigin);
        VrPlayspaceOrientationQuaternion = TypeUtils.ExtractVrRotation(ref trackingOrigin);
    }

    [DllImport("user32.dll")]
    private static extern IntPtr GetActiveWindow();

    public static bool IsCurrentWindowActive()
    {
        if (Shared.Main.AppWindow == null)
            return true; // Give up k?

        return GetActiveWindow() == Shared.Main.AppWindowId;
    }

    public static bool IsDashboardOpen()
    {
        // Check if we're running on null
        StringBuilder systemStringBuilder = new(1024);
        var propertyError = ETrackedPropertyError.TrackedProp_Success;
        OpenVR.System.GetStringTrackedDeviceProperty(
            OpenVR.k_unTrackedDeviceIndex_Hmd, ETrackedDeviceProperty.Prop_TrackingSystemName_String,
            systemStringBuilder, 1024, ref propertyError);

        // Just return true for debug reasons
        if (systemStringBuilder.ToString().Contains("null") ||
            propertyError != ETrackedPropertyError.TrackedProp_Success)
            return true;

        // Also check if we're not idle / standby
        var status = OpenVR.System.GetTrackedDeviceActivityLevel(OpenVR.k_unTrackedDeviceIndex_Hmd);
        if (status != EDeviceActivityLevel.k_EDeviceActivityLevel_UserInteraction &&
            status != EDeviceActivityLevel.k_EDeviceActivityLevel_UserInteraction_Timeout)
            return false; // Standby - hide

        // Check if the dashboard is open
        return OpenVR.Overlay.IsDashboardVisible();
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

    public static class Plugins
    {
        public static void Log(string message, LogSeverity severity)
        {
            switch (severity)
            {
                case LogSeverity.Warning:
                    Logger.Warn(message);
                    break;
                case LogSeverity.Error:
                    Logger.Error(message);
                    break;
                case LogSeverity.Fatal:
                    Logger.Fatal(message);
                    break;
                default:
                case LogSeverity.Info:
                    Logger.Info(message);
                    break;
            }
        }

        public static (Vector3 Position, Quaternion Orientation) GetHmdPose => RawVrHmdPose;

        public static (Vector3 Position, Quaternion Orientation) GetHmdPoseCalibrated => (
            Vector3.Transform(RawVrHmdPose.Position - VrPlayspaceTranslation,
                Quaternion.Inverse(VrPlayspaceOrientationQuaternion)),
            Quaternion.Inverse(VrPlayspaceOrientationQuaternion) * RawVrHmdPose.Orientation);

        public static (Vector3 Position, Quaternion Orientation) GetLeftControllerPose()
        {
            var devicePose = new TrackedDevicePose_t[OpenVR.k_unMaxTrackedDeviceCount];
            OpenVR.System.GetDeviceToAbsoluteTrackingPose(
                ETrackingUniverseOrigin.TrackingUniverseStanding, 0, devicePose);

            // Get pos & rot -> EigenUtils' gonna do this stuff for us
            if (VrControllerIndexes.Left != OpenVR.k_unTrackedDeviceIndexInvalid)
                return (
                    TypeUtils.ExtractVrPosition(ref devicePose[VrControllerIndexes.Left].mDeviceToAbsoluteTracking),
                    TypeUtils.ExtractVrRotation(ref devicePose[VrControllerIndexes.Left].mDeviceToAbsoluteTracking));

            return (Vector3.Zero, Quaternion.Identity); // else
        }

        public static (Vector3 Position, Quaternion Orientation) GetLeftControllerPoseCalibrated()
        {
            var (position, orientation) = GetLeftControllerPose();
            return (Vector3.Transform(position - VrPlayspaceTranslation,
                    Quaternion.Inverse(VrPlayspaceOrientationQuaternion)),
                Quaternion.Inverse(VrPlayspaceOrientationQuaternion) * orientation);
        }

        public static (Vector3 Position, Quaternion Orientation) GetRightControllerPose()
        {
            var devicePose = new TrackedDevicePose_t[OpenVR.k_unMaxTrackedDeviceCount];
            OpenVR.System.GetDeviceToAbsoluteTrackingPose(
                ETrackingUniverseOrigin.TrackingUniverseStanding, 0, devicePose);

            // Get pos & rot -> EigenUtils' gonna do this stuff for us
            if (VrControllerIndexes.Right != OpenVR.k_unTrackedDeviceIndexInvalid)
                return (
                    TypeUtils.ExtractVrPosition(ref devicePose[VrControllerIndexes.Right].mDeviceToAbsoluteTracking),
                    TypeUtils.ExtractVrRotation(ref devicePose[VrControllerIndexes.Right].mDeviceToAbsoluteTracking));

            return (Vector3.Zero, Quaternion.Identity); // else
        }

        public static (Vector3 Position, Quaternion Orientation) GetRightControllerPoseCalibrated()
        {
            var (position, orientation) = GetRightControllerPose();
            return (Vector3.Transform(position - VrPlayspaceTranslation,
                    Quaternion.Inverse(VrPlayspaceOrientationQuaternion)),
                Quaternion.Inverse(VrPlayspaceOrientationQuaternion) * orientation);
        }

        public static List<TrackedJoint> GetAppJointPoses()
        {
            return AppSettings.K2TrackersVector.Select(
                tracker => tracker.GetTrackedJoint()).ToList();
        }

        public static void RequestStatusUiRefresh()
        {
            // Request an explicit status UI refresh
            StatusUiRefreshRequested = true;
        }

        public static string RequestLanguageCode => AppSettings.AppLanguage;

        public static string RequestLocalizedString(string key, string guid)
        {
            try
            {
                // Check if the resource root is fine
                if (TrackingDevices.TrackingDevicesLocalizationResourcesRootsVector
                        .TryGetValue(guid, out var value) && value.ResourceRoot != null && value.ResourceRoot.Count > 0)
                    return TrackingDevices.TrackingDevicesLocalizationResourcesRootsVector[guid]
                        .ResourceRoot.GetNamedString(key);

                // If the first try failed
                Logger.Error(
                    $"[Requested by device with guid {guid}] " +
                    $"The device GUID {guid}s resource root is empty! Its interface will be broken! " +
                    "Falling back to Amethyst string resources!");

                return LocalizedJsonString(key); // Fallback to AME
            }
            catch (Exception e)
            {
                Logger.Error(
                    $"[Requested by device with guid {guid}] " +
                    $"JSON error at key: \"{key}\"! Message: {e.Message}");

                // Else return they key alone
                return key;
            }
        }

        public static bool SetLocalizationResourcesRoot(string path, string guid)
        {
            try
            {
                Logger.Info(
                    $"[Requested by device with guid {guid}] " +
                    "Searching for language resources with key " +
                    $"\"{AppSettings.AppLanguage}\" in \"{path}\"...");

                if (!Directory.Exists(path))
                {
                    Logger.Error(
                        $"[Requested by device with guid {guid}] " +
                        "Could not find any language enumeration resources in " +
                        $"\"{path}\", the device GUID {{guid}}s interface may be broken!");

                    return false; // Give up on trying
                }

                var resourcePath = Path.Join(path, AppSettings.AppLanguage + ".json");

                // If the specified language doesn't exist somehow, fallback to 'en'
                if (!File.Exists(resourcePath))
                {
                    Logger.Warn(
                        $"[Requested by device with guid {guid}] " +
                        "Could not load language resources at " +
                        $"\"{resourcePath}\", falling back to 'en' (en.json)!");

                    resourcePath = Path.Join(GetProgramLocation()
                        .DirectoryName, "Assets", "Strings", "en.json");
                }

                // If failed again, just give up (we'll do fallbacks later)
                if (!File.Exists(resourcePath))
                {
                    Logger.Error(
                        $"[Requested by device with guid {guid}] " +
                        "Could not load language resources at " +
                        $"\"{resourcePath}\", app interface will be broken!");

                    return false; // Just give up
                }

                // If everything's ok, load the resources into the current resource tree


                // Parse the loaded json
                var jsonObject = Windows.Data.Json.JsonObject.Parse(File.ReadAllText(resourcePath));

                // Check if the resource root is fine
                if (jsonObject == null || jsonObject.Count <= 0)
                    Logger.Error(
                        $"[Requested by device with guid {guid}] " +
                        "The current resource root is empty! App interface will be broken!");
                else
                    Logger.Error(
                        $"[Requested by device with guid {guid}] " +
                        $"Successfully loaded language resources with key \"{AppSettings.AppLanguage}\"!");

                // If everything's ok, change the root
                if (TrackingDevices.TrackingDevicesLocalizationResourcesRootsVector.ContainsKey(guid))
                    TrackingDevices.TrackingDevicesLocalizationResourcesRootsVector[guid] =
                        (jsonObject, new DirectoryInfo(path));

                return true; // We're good
            }
            catch (Exception e)
            {
                Logger.Warn(
                    $"[Requested by device with guid {guid}] " +
                    $"JSON error at key: \"{AppSettings.AppLanguage}\"! " +
                    $"Message: {e.Message}");

                return false; // Just give up
            }
        }
    }
}