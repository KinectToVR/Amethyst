using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Numerics;
using Windows.Data.Json;
using Windows.Globalization;
using Windows.System.UserProfile;
using Amethyst.MVVM;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using Newtonsoft.Json;

namespace Amethyst.Classes;

public class AppSettings : INotifyPropertyChanged
{
    // Disabled (by the user) plugins set
    public readonly SortedSet<string> DisabledPluginsGuidSet = new();

    // App sounds' volume and *nice* is the default
    private uint _appSoundsVolume = 69; // Always 0<x<100

    // Automatically spawn enabled trackers on startup and off is the default
    private bool _autoSpawnEnabledJoints;
    private bool _checkForOverlappingTrackers = true; // Check for overlapping roles

    // Enable application sounds and on is the default
    private bool _enableAppSounds = true;

    // Skeleton flip based on non-flip override devices' waist tracker
    private bool _isExternalFlipEnabled;

    // Skeleton flip when facing away: One-For-All and on is the default
    private bool _isFlipEnabled = true;
    [JsonIgnore] private string _selectedTrackingDeviceGuid;

    // External flip device's calibration rotation
    public Quaternion ExternalFlipCalibrationMatrix = new();

    [JsonIgnore] public string PreviousSelectedTrackingDeviceGuid;

    // Current language & theme
    public string AppLanguage { get; set; } = "en";

    // 0:system, 1:dark, 2:light
    public uint AppTheme { get; set; }

    // Current joints
    public ObservableCollection<AppTracker> TrackersVector { get; set; } = new();

    public bool UseTrackerPairs { get; set; } = true; // Pair feet, elbows and knees

    public bool CheckForOverlappingTrackers
    {
        get => _checkForOverlappingTrackers;
        set
        {
            _checkForOverlappingTrackers = value;
            OnPropertyChanged("CheckForOverlappingTrackers");
            AppData.Settings.SaveSettings();
        }
    }

    public string ServiceEndpointGuid { get; set; } = ""; // -> Always set
    public string TrackingDeviceGuid { get; set; } = ""; // -> Always set
    public SortedSet<string> OverrideDevicesGuidMap { get; set; } = new();

    public bool IsFlipEnabled
    {
        get => _isFlipEnabled;
        set
        {
            _isFlipEnabled = value;
            TrackingDevices.CheckFlipSupport();

            OnPropertyChanged("IsFlipEnabled");
            AppData.Settings.SaveSettings();
        }
    }

    public bool IsExternalFlipEnabled
    {
        get => _isExternalFlipEnabled;
        set
        {
            _isExternalFlipEnabled = value;
            TrackingDevices.CheckFlipSupport();
            OnPropertyChanged("IsExternalFlipEnabled");
            AppData.Settings.SaveSettings();
        }
    }

    public bool AutoSpawnEnabledJoints
    {
        get => _autoSpawnEnabledJoints;
        set
        {
            _autoSpawnEnabledJoints = value;
            OnPropertyChanged("AutoSpawnEnabledJoints");
            AppData.Settings.SaveSettings();
        }
    }

    public bool EnableAppSounds
    {
        get => _enableAppSounds;
        set
        {
            _enableAppSounds = value;
            OnPropertyChanged("EnableAppSounds");
            AppData.Settings.SaveSettings();
        }
    }

    public uint AppSoundsVolume
    {
        get => _appSoundsVolume;
        set
        {
            _appSoundsVolume = value;
            OnPropertyChanged("AppSoundsVolume");
            AppData.Settings.SaveSettings();
        }
    }

    // Calibration matrices : GUID/Data
    public SortedDictionary<string, Quaternion> DeviceCalibrationRotationMatrices { get; set; } = new();
    public SortedDictionary<string, Vector3> DeviceCalibrationTranslationVectors { get; set; } = new();
    public SortedDictionary<string, Vector3> DeviceCalibrationOrigins { get; set; } = new();

    // Calibration helpers - points number
    public uint CalibrationPointsNumber { get; set; } = 3; // Always 3<=x<=5

    // Save the skeleton preview state
    public bool SkeletonPreviewEnabled { get; set; } = true;

    // If we wanna dismiss all warnings during the preview
    public bool ForceSkeletonPreview { get; set; } = false;

    // If we wanna freeze only lower body trackers or all
    public bool FreezeLowerBodyOnly { get; set; } = false;

    // If the freeze bindings teaching tip has been shown
    public bool TeachingTipShownFreeze { get; set; } = false;

    // If the flip bindings teaching tip has been shown
    public bool TeachingTipShownFlip { get; set; } = false;

    // If the first-launch guide's been shown
    public bool FirstTimeTourShown { get; set; } = false;

    // If the shutdown warning has been shown
    public bool FirstShutdownTipShown { get; set; } = false;

    // Disable/Enable optional data
    public bool IsTelemetryEnabled { get; set; } = true;

    [JsonIgnore]
    public string SelectedTrackingDeviceGuid
    {
        get => _selectedTrackingDeviceGuid;
        set
        {
            _selectedTrackingDeviceGuid = value;
            OnPropertyChanged(); // Refresh all
        }
    }

    [JsonIgnore] public static bool CheckingSettings { get; private set; }

    // MVVM stuff
    public event PropertyChangedEventHandler PropertyChanged;

    // Save settings
    public void SaveSettings()
    {
        if (CheckingSettings) return;
        try
        {
            // Save application settings to $env:AppData/Amethyst/
            File.WriteAllText(
                Interfacing.GetAppDataFileDir("AmethystSettings.json"),
                JsonConvert.SerializeObject(AppData.Settings, Formatting.Indented));
        }
        catch (Exception e)
        {
            Logger.Error($"Error saving application settings! Message: {e.Message}");
        }
    }

    // Re/Load settings
    public void ReadSettings()
    {
        try
        {
            // Read application settings from $env:AppData/Amethyst/
            AppData.Settings = JsonConvert.DeserializeObject<AppSettings>(File.ReadAllText(
                Interfacing.GetAppDataFileDir("AmethystSettings.json"))) ?? new AppSettings();
        }
        catch (Exception e)
        {
            Logger.Error($"Error reading application settings! Message: {e.Message}");
            AppData.Settings ??= new AppSettings(); // Reset if null
        }

        CheckSettings(true);
    }

    public void CheckSettings(bool partial = false, TrackingDevice device = null)
    {
        // Basic config: global and pre-setup settings
        Logger.Info("Checking AppSettings [global] configuration...");
        CheckingSettings = true; // Don't check interface

        // Check if the trackers vector is broken
        var vectorBroken = TrackersVector.Count < 7;

        // Optionally fix the trackers vector
        while (TrackersVector.Count < 7)
            TrackersVector.Add(new AppTracker());

        // Force the first 7 trackers to be the default ones : roles
        TrackersVector[0].Role = TrackerType.TrackerWaist;
        TrackersVector[1].Role = TrackerType.TrackerLeftFoot;
        TrackersVector[2].Role = TrackerType.TrackerRightFoot;
        TrackersVector[3].Role = TrackerType.TrackerLeftElbow;
        TrackersVector[4].Role = TrackerType.TrackerRightElbow;
        TrackersVector[5].Role = TrackerType.TrackerLeftKnee;
        TrackersVector[6].Role = TrackerType.TrackerRightKnee;

        foreach (var tracker in TrackersVector)
        {
            // Force the first 7 trackers to be the default ones : serials
            tracker.Serial = TypeUtils.TrackerTypeRoleSerialDictionary[tracker.Role];

            // Force disable software orientation if used by a non-foot
            if (tracker.Role != TrackerType.TrackerLeftFoot &&
                tracker.Role != TrackerType.TrackerRightFoot &&
                tracker.OrientationTrackingOption is JointRotationTrackingOption.SoftwareCalculatedRotation
                    or JointRotationTrackingOption.SoftwareCalculatedRotationV2)
                tracker.OrientationTrackingOption = JointRotationTrackingOption.DeviceInferredRotation;
        }

        // If the vector was broken, override waist & feet statuses
        if (vectorBroken)
        {
            TrackersVector[0].IsActiveEnabled = true;
            TrackersVector[1].IsActiveEnabled = true;
            TrackersVector[2].IsActiveEnabled = true;

            // Try reading the default config
            Logger.Info("Checking out the default configuration settings...");
            (bool ExtraTrackers, bool Valid) defaultSettings = (false, false); // Invalid for now!

            if (File.Exists(Path.Join(Interfacing.GetProgramLocation().DirectoryName, "defaults.json")))
                try
                {
                    // Parse the loaded json
                    var jsonHead = JsonObject.Parse(File.ReadAllText(
                        Path.Join(Interfacing.GetProgramLocation().DirectoryName, "defaults.json")));

                    if (!jsonHead.ContainsKey("ExtraTrackers"))
                        // Invalid configuration file, don't proceed further!
                        Logger.Error("The default configuration json file was invalid!");
                    else // Read from JSON and mark as valid
                        defaultSettings = (jsonHead.GetNamedBoolean("ExtraTrackers"), true);
                }
                catch (Exception e)
                {
                    Logger.Info($"Default settings checkout failed! Message: {e.Message}");
                }
            else Logger.Info("No default configuration found! [defaults.json]");

            // Enable more trackers if valid and requested
            if (defaultSettings.Valid && defaultSettings.ExtraTrackers)
            {
                TrackersVector[3].IsActiveEnabled = true;
                TrackersVector[4].IsActiveEnabled = true;
                TrackersVector[5].IsActiveEnabled = true;
                TrackersVector[6].IsActiveEnabled = true;
            }
        }

        // Scan for duplicate trackers
        foreach (var tracker in TrackersVector.GroupBy(x => x.Role)
                     .Where(g => g.Count() > 1)
                     .Select(y => y.First()).ToList())
        {
            Logger.Warn("A duplicate tracker was found in the trackers vector! Removing it...");
            TrackersVector.Remove(tracker); // Remove the duplicate tracker
        }

        // Check if any trackers are enabled
        // -> No trackers are enabled, force-enable the waist tracker
        if (!TrackersVector.Any(x => x.IsActiveEnabled))
        {
            Logger.Warn("All trackers were disabled, force-enabling the waist tracker!");
            TrackersVector[0].IsActiveEnabled = true; // Enable the waist tracker
        }

        // Fix statuses (optional)
        var pairedTrackerTypes = new List<(TrackerType L, TrackerType R)>
        {
            (TrackerType.TrackerLeftFoot, TrackerType.TrackerRightFoot),
            (TrackerType.TrackerLeftKnee, TrackerType.TrackerRightKnee),
            (TrackerType.TrackerLeftElbow, TrackerType.TrackerRightElbow),
            (TrackerType.TrackerLeftShoulder, TrackerType.TrackerRightShoulder)
        };

        if (!partial)
            TrackersVector.Where(tracker => !tracker.IsSupported).ToList()
                .ForEach(tracker => Logger.Info(
                    $"Tracker role {tracker.Role} is not supported by " +
                    $"({TrackingDevices.CurrentServiceEndpoint.Guid}, " +
                    $"{TrackingDevices.CurrentServiceEndpoint.Name})! " +
                    "Disabling this tracker and marking as unsupported!"));

        // Check pairs' configs
        if (UseTrackerPairs)
            pairedTrackerTypes.ForEach(typePair =>
            {
                var lT = TrackersVector.ToList().Find(x => x.Role == typePair.L);
                var rT = TrackersVector.ToList().Find(x => x.Role == typePair.R);
                if (lT is null || rT is null) return;

                rT.IsActiveEnabled = lT.IsActiveEnabled;
                rT.OrientationTrackingOption = lT.OrientationTrackingOption;
                rT.PositionTrackingFilterOption = lT.PositionTrackingFilterOption;
                rT.OrientationTrackingFilterOption = lT.OrientationTrackingFilterOption;
            });

        // Check filter indexes
        Logger.Info("Checking pose filter index settings...");
        AppData.Settings.TrackersVector.ToList().ForEach(tracker =>
        {
            tracker.PositionTrackingFilterOption =
                (JointPositionTrackingOption)Math.Clamp(
                    (int)tracker.PositionTrackingFilterOption,
                    (int)JointPositionTrackingOption.PositionTrackingFilterLerp,
                    (int)JointPositionTrackingOption.NoPositionTrackingFilter);

            tracker.OrientationTrackingFilterOption =
                (RotationTrackingFilterOption)Math.Clamp(
                    (int)tracker.OrientationTrackingFilterOption,
                    (int)RotationTrackingFilterOption.OrientationTrackingFilterSlerp,
                    (int)RotationTrackingFilterOption.NoOrientationTrackingFilter);

            tracker.OrientationTrackingOption =
                (JointRotationTrackingOption)Math.Clamp(
                    (int)tracker.OrientationTrackingOption,
                    (int)JointRotationTrackingOption.DeviceInferredRotation,
                    (int)JointRotationTrackingOption.DisableJointRotation);
        });

        // Optionally fix volume if too big somehow
        AppSoundsVolume = Math.Clamp(AppSoundsVolume, 0, 100);

        // Optionally fix calibration points
        CalibrationPointsNumber = Math.Clamp(CalibrationPointsNumber, 3, 5);

        // Optionally fix the app theme value
        AppTheme = Math.Clamp(AppTheme, 0, 2);

        // Optionally fix the selected language / select a new one
        var resourcePath = Path.Join(
            Interfacing.GetProgramLocation().DirectoryName,
            "Assets", "Strings", AppLanguage + ".json");

        // If there's no specified language, fallback to {system}
        if (string.IsNullOrEmpty(AppLanguage))
        {
            AppLanguage = new Language(
                GlobalizationPreferences.Languages[0]).LanguageTag[..2];

            Logger.Warn($"No language specified! Trying with the system one: \"{AppLanguage}\"!");
            resourcePath = Path.Join(
                Interfacing.GetProgramLocation().DirectoryName, "Assets", "Strings", AppLanguage + ".json");
        }

        // If the specified language doesn't exist somehow, fallback to 'en'
        if (!File.Exists(resourcePath))
        {
            Logger.Warn($"Could not load language resources at \"{resourcePath}\", falling back to 'en' (en.json)!");

            AppLanguage = "en"; // Change to english
            resourcePath = Path.Join(
                Interfacing.GetProgramLocation().DirectoryName,
                "Assets", "Strings", AppLanguage + ".json");
        }

        // If failed again, just give up
        if (!File.Exists(resourcePath))
            Logger.Warn($"Could not load language resources at \"{resourcePath}\", the app interface will be broken!");

        // That's all for basic config!
        CheckingSettings = false;
        if (partial) return; // Don't.

        // Advanced config: runtime and tracking settings
        Logger.Info("Checking AppSettings [runtime] configuration...");
        var trackingDevice = device ?? TrackingDevices.BaseTrackingDevice;

        // Check orientation option configs : left foot
        Logger.Info("Checking left foot orientation settings...");
        if (AppData.Settings.TrackersVector[1].OrientationTrackingOption is
                JointRotationTrackingOption.SoftwareCalculatedRotation or
                JointRotationTrackingOption.SoftwareCalculatedRotationV2 &&
            trackingDevice.IsAppOrientationSupported)
            AppData.Settings.TrackersVector[1].OrientationTrackingOption =
                JointRotationTrackingOption.DeviceInferredRotation;

        // Check orientation option configs : right foot
        Logger.Info("Checking right foot orientation settings...");
        if (AppData.Settings.TrackersVector[2].OrientationTrackingOption is
                JointRotationTrackingOption.SoftwareCalculatedRotation or
                JointRotationTrackingOption.SoftwareCalculatedRotationV2 &&
            trackingDevice.IsAppOrientationSupported)
            AppData.Settings.TrackersVector[2].OrientationTrackingOption =
                JointRotationTrackingOption.DeviceInferredRotation;

        // Check joint and override GUID & IDs
        Logger.Info("Checking if saved tracker overrides exist in loaded overrides...");
        foreach (var appTracker in AppData.Settings.TrackersVector)
        {
            // Joint ID check
            Logger.Info($"Checking if tracker {appTracker.Serial} bound joint ID " +
                        $"({appTracker.SelectedTrackedJointId}) is valid for loaded plugins...");

            // Check if the specified index is valid
            if (trackingDevice.TrackedJoints.Count <= appTracker.SelectedTrackedJointId)
            {
                Logger.Info($"The saved tracker {appTracker.Serial} bound joint ID " +
                            $"({appTracker.SelectedTrackedJointId}) is invalid! Resetting it to the first one!");

                // The joint ID haven't gotten real :/ reset
                // (Joint ID 0 should be safe in most cases)
                appTracker.SelectedTrackedJointId = 0;
            }

            // Joint ID check
            Logger.Info($"Checking if device ({trackingDevice.Name}, {trackingDevice.Name}) " +
                        $"provides a pre-selected joint for tracker {appTracker.Serial} ({appTracker.Role})...");

            // Check if the device joints contain such a role
            var preselectedRoleJointIndex = trackingDevice.TrackedJoints.FindIndex(
                x => x.Role != TrackedJointType.JointManual &&
                     TypeUtils.TrackerTypeJointDictionary.TryGetValue(
                         appTracker.Role, out var gotRole) && gotRole == x.Role);

            // Overwrite the base selected joint if valid
            if (preselectedRoleJointIndex >= 0)
            {
                Logger.Info($"Device ({trackingDevice.Name}, {trackingDevice.Name}) " +
                            $"provides a pre-selected joint for tracker {appTracker.Serial} ({appTracker.Role})! " +
                            $"Setting the {nameof(appTracker.SelectedTrackedJointId)} property to it!");

                // The preselected joint ID got real O.O
                appTracker.SelectedTrackedJointId = (uint)preselectedRoleJointIndex;
            }

            // ReSharper disable once InvertIf | Override GUID check
            if (appTracker.IsOverridden)
            {
                Logger.Info($"Checking if tracker {appTracker.Serial} override " +
                            $"({appTracker.OverrideGuid}) exists in loaded plugins...");

                // Check if the override specified by the tracker is real
                if (!TrackingDevices.TrackingDevicesList.ContainsKey(appTracker.OverrideGuid))
                {
                    Logger.Info($"The saved tracker {appTracker.Serial} override " +
                                $"({appTracker.OverrideGuid}) is invalid! Resetting it to NONE!");

                    // The override haven't gotten real :/ reset
                    appTracker.OverrideGuid = "";
                    appTracker.IsPositionOverridden = false;
                    appTracker.IsOrientationOverridden = false;
                }

                // Override joint ID check
                Logger.Info($"Checking if tracker {appTracker.Serial} bound override joint ID " +
                            $"({appTracker.SelectedTrackedJointId}) is valid for loaded plugins...");

                // ReSharper disable once InvertIf | Check if the specified override index is valid
                if (TrackingDevices.GetDevice(appTracker.OverrideGuid)
                        .Device.TrackedJoints.Count <= appTracker.OverrideJointId)
                {
                    Logger.Info($"The saved tracker {appTracker.Serial} bound joint ID " +
                                $"({appTracker.OverrideJointId}) is invalid! Resetting it to the first one!");

                    // The joint ID haven't gotten real :/ reset
                    // (Joint ID 0 should be safe in most cases)
                    appTracker.OverrideJointId = 0;
                }
            }
        }
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }
}