using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Threading.Tasks;
using Amethyst.Installer.ViewModels;
using Amethyst.MVVM;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media.Animation;
using static Amethyst.Classes.Shared;

namespace Amethyst.Classes;

public static class AppPlugins
{
    public enum PluginLoadError
    {
        Unknown, // We literally don't know what's happened
        NoError, // Everything's fine, celebration time!
        LoadingSkipped, // This device is disabled by the user
        NoPluginFolder, // No device folder w/ files found
        NoPluginDll, // Device dll not found at proper path
        NoPluginDependencyDll, // Dep dll/s not found or invalid
        PluginDllLinkError, // Could not link for some reason
        BadOrDuplicateGuid, // Empty/Bad/Duplicate device GUID
        InvalidFactory, // Device factory just gave up, now cry
        Other // Check logs, MEF probably gave us up again...
    }

    public enum PluginType
    {
        Unknown, // We literally don't know what
        TrackingDevice, // Provides tracking data
        ServiceEndpoint // Receives tracking data
    }

    // Device plugins / tracking providers
    public static readonly SortedDictionary<string, TrackingDevice> TrackingDevicesList = new();

    // Endpoint plugins / tracking receivers
    public static readonly SortedDictionary<string, ServiceEndpoint> ServiceEndpointsList = new();

    // Endpoint plugins / tracking receivers
    public static readonly SortedDictionary<string, ServiceEndpoint> InstallerList = new();

    // Written to at the first plugin load
    public static readonly ObservableCollection<LoadAttemptedPlugin> LoadAttemptedPluginsList = new();

    // Used by the installer module - device plugins
    public static readonly ObservableCollection<SetupPlugin> InstallerPluginsList = new();

    public static IEnumerable<LoadAttemptedPlugin> LoadedPluginsList => LoadAttemptedPluginsList.Where(x => x.IsLoaded);
    public static IEnumerable<LoadAttemptedPlugin> ErrorPluginsList => LoadAttemptedPluginsList.Where(x => !x.IsLoaded);

    // Plugin settings (Note: GUIDs must not overlap)
    public static AppPluginSettings PluginSettings { get; set; } = new();

    // Get the base tracking provider device
    public static TrackingDevice BaseTrackingDevice => TrackingDevicesList[AppData.Settings.TrackingDeviceGuid];

    // Get the currently in-operation tracking service
    public static ServiceEndpoint CurrentServiceEndpoint => ServiceEndpointsList[AppData.Settings.ServiceEndpointGuid];

    // Get a <exists, tracking device> by guid
    public static (bool Exists, TrackingDevice Device) GetDevice(string guid)
    {
        return (TrackingDevicesList.TryGetValue(guid, out var device), device);
    }

    // Get a <exists, tracking device> by guid
    public static (bool Exists, ServiceEndpoint Service) GetService(string guid)
    {
        return (ServiceEndpointsList.TryGetValue(guid, out var service), service);
    }

    public static void UpdateTrackingDevicesInterface()
    {
        if (TrackingDevicesList.Count < 1) return; // Just give up

        var currentDevice = BaseTrackingDevice;
        var baseStatusOk = currentDevice.DeviceStatus == 0;

        var overrideStatusOk = true;
        var failingOverrideGuid = "";

        foreach (var device in AppData.Settings.OverrideDevicesGuidMap
                     .Where(device => TrackingDevicesList[device].DeviceStatus != 0))
        {
            overrideStatusOk = false;
            failingOverrideGuid = device;
        }

        General.AdditionalDeviceErrorsHyperlinkTappedEvent =
            new Task(() => Shared.Main.DispatcherQueue.TryEnqueue(
                async () =>
                {
                    // Play a sound
                    AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

                    // Navigate to the devices page
                    Shared.Main.MainNavigationView.SelectedItem =
                        Shared.Main.MainNavigationView.MenuItems[2];

                    Shared.Main.NavigateToPage("devices",
                        new EntranceNavigationTransitionInfo());

                    await Task.Delay(500);

                    // Should already be init-ed after 500ms, but check anyway
                    if (Devices.DevicesTreeView is null) return;
                    var devicesListIndex = TrackingDevicesList.Keys.ToList().IndexOf(failingOverrideGuid);
                    var devicesListNode = Devices.DevicesTreeView.RootNodes[devicesListIndex];

                    var skipAnimation = Devices.DevicesTreeView.SelectedNode == devicesListNode;
                    Devices.DevicesTreeView.SelectedNode = devicesListNode;

                    AppData.Settings.PreviousSelectedTrackingDeviceGuid = failingOverrideGuid;
                    AppData.Settings.SelectedTrackingDeviceGuid = failingOverrideGuid;

                    await Devices.ReloadSelectedDevice(skipAnimation);
                    Events.ReloadDevicesPageEvent?.Set(); // Full reload
                }));

        // Update general tab
        if (General.ErrorWhatText is not null)
        {
            // Don't show device errors if we've got a server error
            if (!Interfacing.IsServiceEndpointPresent)
                baseStatusOk = true; // Skip if the server's failed

            var show = baseStatusOk ? Visibility.Collapsed : Visibility.Visible;
            General.ErrorWhatText.Visibility = show;
            General.ErrorWhatGrid.Visibility = show;
            General.ErrorButtonsGrid.Visibility = show;
            General.TrackingDeviceErrorLabel.Visibility = show;

            // Split status and message by \n
            var message = StringUtils.SplitStatusString(currentDevice.DeviceStatusString);
            if (message is null || message.Length < 3)
                message = new[] { "The status message was broken!", "E_FIX_YOUR_SHIT", "AAAAA" };

            General.DeviceNameLabel.Text = currentDevice.Name;
            General.DeviceStatusLabel.Text = message[0];
            General.TrackingDeviceErrorLabel.Text = message[1];
            General.ErrorWhatText.Text = message[2];

            // Dim the calibration button if can't calibrate right now
            General.CalibrationButton.Opacity =
                !General.CalibrationButton.IsEnabled || // Don't dim if disabled
                AppData.Settings.OverrideDevicesGuidMap.Count > 0 ||
                baseStatusOk // Don't dim if we have overrides or just are OK
                    ? 1.0
                    : 0.5;

            // -- or the single set base device doesn't match the service
            if (AppData.Settings.OverrideDevicesGuidMap.Count <= 0 && baseStatusOk &&
                CurrentServiceEndpoint.ControllerInputActions == null &&
                (currentDevice.TrackedJoints.All(x =>
                    x.Role != TrackedJointType.JointHead) || CurrentServiceEndpoint.HeadsetPose == null))
                General.CalibrationButton.Opacity = 0.5;

            // Hide the calibration button if no service calibration
            General.CalibrationButton.Visibility =
                // Show either if input actions (manual calibration) is available
                CurrentServiceEndpoint.ControllerInputActions != null ||
                // Or when automatic calibration *should be* available
                CurrentServiceEndpoint.HeadsetPose != null
                    ? Visibility.Visible
                    : Visibility.Collapsed;

            // Show the 'additional errors' button if there are errors
            // Skip if the server driver's not present
            General.AdditionalDeviceErrorsHyperlink.Visibility =
                overrideStatusOk || !Interfacing.IsServiceEndpointPresent
                    ? Visibility.Collapsed
                    : Visibility.Visible;

            // Set the app state (optionally)
            if (Interfacing.CurrentPageTag == "devices")
                Interfacing.CurrentAppState =
                    Devices.DevicesTreeView is not null && overrideStatusOk &&
                    AppData.Settings.OverrideDevicesGuidMap
                        .Contains(AppData.Settings.SelectedTrackingDeviceGuid)
                        ? "overrides" // Currently managing overrides
                        : "devices"; // Currently viewing tracking devices
        }

        // Update settings tab
        if (Settings.FlipDropDown is null) return;

        // Overwritten a bit earlier
        Settings.FlipToggle.IsEnabled = currentDevice.IsFlipSupported;
        Settings.FlipDropDown.IsEnabled = currentDevice.IsFlipSupported;
        Settings.FlipDropDownGrid.Opacity = currentDevice.IsFlipSupported
            ? 1.0 // If supported : display as normal
            : 0.5; // IF not : dim a bit (and probably hide)

        // Update extflip
        CheckFlipSupport();
    }

    public static void HandleDeviceRefresh(bool shouldReconnect)
    {
        // Just give up if not set up yet
        if (Devices.ErrorWhatText is null) return;

        var jointsValidBackup = Devices.DevicesJointsValid;
        Devices.DevicesJointsValid = false; // Block signals
        var currentDevice =
            GetDevice(AppData.Settings.SelectedTrackingDeviceGuid ?? AppData.Settings.TrackingDeviceGuid);

        if (shouldReconnect) currentDevice.Device.Initialize();

        // Update the status
        var statusOk = currentDevice.Device.StatusOk;
        Devices.ErrorWhatText.Visibility = statusOk ? Visibility.Collapsed : Visibility.Visible;
        Devices.DeviceErrorGrid.Visibility = statusOk ? Visibility.Collapsed : Visibility.Visible;
        Devices.TrackingDeviceErrorLabel.Visibility = statusOk ? Visibility.Collapsed : Visibility.Visible;
        Devices.TrackingDeviceChangePanel.Visibility = statusOk ? Visibility.Visible : Visibility.Collapsed;

        // Update the device
        var message = StringUtils.SplitStatusString(currentDevice.Device.DeviceStatusString);
        if (message is null || message.Length < 3)
            message = new[] { "The status message was broken!", "E_FIX_YOUR_SHIT", "AAAAA" };

        Devices.DeviceNameLabel.Text = currentDevice.Device.Name;
        Devices.DeviceStatusLabel.Text = message[0];
        Devices.TrackingDeviceErrorLabel.Text = message[1];
        Devices.ErrorWhatText.Text = message[2];

        // Update device-provided settings
        Devices.SelectedDeviceSettingsRootLayoutPanel.Visibility =
            currentDevice.Device.IsSettingsDaemonSupported ? Visibility.Visible : Visibility.Collapsed;

        Devices.SelectedDeviceSettingsRootLayoutPanel.Children.Clear();
        if (currentDevice.Device.SettingsInterfaceRoot is Page root)
            Devices.SelectedDeviceSettingsRootLayoutPanel.Children.Add(root);

        // We're done
        Devices.DevicesJointsValid = jointsValidBackup;
    }

    public static bool IsExternalFlipSupportable()
    {
        if (!BaseTrackingDevice.IsFlipSupported) return false;

        /* Now check if either waist tracker is overridden or disabled
		 * And then search in OpenVR for a one with waist role */

        // If we have an override and if it's actually affecting the waist rotation
        foreach (var overrideDeviceGuid in AppData.Settings.OverrideDevicesGuidMap
                     .Where(overrideDeviceGuid =>
                         AppData.Settings.TrackersVector[0].IsActive &&
                         AppData.Settings.TrackersVector[0].IsOrientationOverridden &&
                         AppData.Settings.TrackersVector[0].OverrideGuid == overrideDeviceGuid))
            return !TrackingDevicesList[overrideDeviceGuid].IsFlipSupported;

        // If still not, then also check if the waist is disabled by any chance
        return !AppData.Settings.TrackersVector[0].IsActive &&
               CurrentServiceEndpoint.GetTrackerPose("waist", false) != null;
    }

    public static void CheckFlipSupport()
    {
        if (Settings.ExternalFlipCheckBox is null) return;

        Settings.FlipToggle.IsEnabled = BaseTrackingDevice.IsFlipSupported;
        Settings.FlipDropDown.IsEnabled = AppData.Settings.IsFlipEnabled
                                          && Settings.FlipToggle.IsEnabled;

        Settings.FlipDropDownGrid.Opacity = Settings.FlipToggle.IsEnabled ? 1.0 : 0.5;
        Settings.FlipDropDownContainer.Visibility =
            Settings.FlipToggle.IsEnabled ? Visibility.Visible : Visibility.Collapsed;

        if (!AppData.Settings.IsFlipEnabled)
        {
            Settings.FlipDropDown.IsEnabled = false;
            Settings.FlipDropDown.IsExpanded = false;
        }
        else
        {
            Settings.FlipDropDown.IsEnabled = true;
        }

        // Everything's fine
        if (IsExternalFlipSupportable() &&
            AppData.Settings.IsFlipEnabled &&
            AppData.Settings.IsExternalFlipEnabled)
        {
            Settings.ExternalFlipStatusLabel.Text =
                Interfacing.LocalizedJsonString("/SettingsPage/Captions/ExtFlipStatus/Active");

            Settings.ExternalFlipStatusStackPanel.Visibility = Visibility.Visible;
        }
        // No tracker detected
        else if (AppData.Settings.IsExternalFlipEnabled)
        {
            Settings.ExternalFlipStatusLabel.Text =
                Interfacing.LocalizedJsonString("/SettingsPage/Captions/ExtFlipStatus/NoTracker");

            Settings.ExternalFlipStatusStackPanel.Visibility = Visibility.Visible;
        }
        // Disabled by the user
        else
        {
            Settings.ExternalFlipStatusLabel.Text =
                Interfacing.LocalizedJsonString("/SettingsPage/Captions/ExtFlipStatus/Disabled");

            Settings.ExternalFlipStatusStackPanel.Visibility = Visibility.Collapsed;
        }
    }

    public static void TrackersConfigChanged(bool showToasts = true)
    {
        // Don't react to pre-init signals
        if (!Settings.SettingsTabSetupFinished) return;
        Logger.Info("Trackers configuration has been changed!");
        if (!showToasts) Logger.Info("Any toast won't be shown this time: force-disabled");

        // Refresh trackers MVVM (everywhere)
        AppData.Settings.OnPropertyChanged("TrackersVector");
        Events.ReloadDevicesPageEvent?.Set();

        // If this is the first time and happened runtime, also show the notification
        if (Interfacing.AppTrackersSpawned && CurrentServiceEndpoint.IsRestartOnChangesNeeded)
        {
            Logger.Info("Trackers config has changed! " +
                        "The selected tracking service endpoint needs to restart now!");

            if (Settings.RestartButton is not null &&
                !Settings.RestartButton.IsEnabled && showToasts)
            {
                Interfacing.ShowToast(
                    Interfacing.LocalizedJsonString(
                        "/SharedStrings/Toasts/TrackersConfigChanged/Title"),
                    Interfacing.LocalizedJsonString(
                        "/SharedStrings/Toasts/TrackersConfigChanged").Format(CurrentServiceEndpoint.Name),
                    false, "focus_restart");

                Interfacing.ShowServiceToast(
                    Interfacing.LocalizedJsonString(
                        "/SharedStrings/Toasts/TrackersConfigChanged/Title"),
                    Interfacing.LocalizedJsonString(
                        "/SharedStrings/Toasts/TrackersConfigChanged").Format(CurrentServiceEndpoint.Name));
            }

            // Compare with saved settings and unlock the restart
            if (Settings.RestartButton is not null)
                Settings.RestartButton.IsEnabled = true;
        }

        // If restarting isn't needed
        if (!CurrentServiceEndpoint.IsRestartOnChangesNeeded)
            Logger.Info("Trackers config has changed! " +
                        "The selected tracking service endpoint doesn't need to restart, though. What a lucky day~!");

        // Enable/Disable Flip
        CheckFlipSupport();
    }

    public static void CheckOverrideIndexes(string guid)
    {
        // Do we need to save?
        var settingsChangesMade = false;

        // Check if all joints have valid IDs
        if (TrackingDevicesList.TryGetValue(guid, out var device))
            foreach (var tracker in AppData.Settings.TrackersVector.Where(tracker =>
                         tracker.OverrideGuid == guid &&
                         tracker.OverrideJointId >= device.TrackedJoints.Count))
            {
                tracker.OverrideJointId = 0;
                settingsChangesMade = true;
            }

        // Save it (if needed)
        if (settingsChangesMade)
            AppData.Settings.SaveSettings();
    }

    public static void CheckBaseIndexes()
    {
        // Do we need to save?
        var settingsChangesMade = false;

        // Check if all joints have valid IDs
        foreach (var tracker in AppData.Settings.TrackersVector.Where(tracker =>
                     tracker.SelectedTrackedJointId >= BaseTrackingDevice.TrackedJoints.Count))
        {
            tracker.SelectedTrackedJointId = 0;
            settingsChangesMade = true;
        }

        // Save it (if needed)
        if (settingsChangesMade)
            AppData.Settings.SaveSettings();
    }

    public static bool IsBase(string guid)
    {
        return AppData.Settings.TrackingDeviceGuid == guid;
    }

    public static bool IsOverride(string guid)
    {
        return AppData.Settings.OverrideDevicesGuidMap.Contains(guid);
    }
}