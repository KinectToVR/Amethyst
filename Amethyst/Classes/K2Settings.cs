using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using Windows.Globalization;
using Windows.System.UserProfile;
using Amethyst.Utils;
using System.ComponentModel;
using Amethyst.Plugins.Contract;

namespace Amethyst.Classes;

public class K2AppSettings : INotifyPropertyChanged
{
    // Current language & theme
    public string AppLanguage { get; set; }

    // 0:system, 1:dark, 2:light
    public uint AppTheme { get; set; }

    // Current joints
    public List<K2AppTracker> K2TrackersVector { get; set; } = new();
    public bool UseTrackerPairs { get; set; } = true; // Pair feet, elbows and knees
    public bool CheckForOverlappingTrackers { get; set; } = true; // Check for overlapping roles

    // Current tracking device: 0 is the default base device
    // First: Device's GUID / saved, Second: Index ID / generated
    public (string GUID, uint ID) TrackingDeviceGuidPair { get; set; } = new(); // -> Always set and >= 0
    public SortedDictionary<string, uint> OverrideDeviceGuiDsMap { get; set; } = new();

    // Skeleton flip when facing away: One-For-All and on is the default
    public bool IsFlipEnabled { get; set; } = true;

    // Skeleton flip based on non-flip override devices' waist tracker
    public bool IsExternalFlipEnabled { get; set; } = false;

    // Automatically spawn enabled trackers on startup and off is the default
    public bool AutoSpawnEnabledJoints { get; set; } = false;

    // Enable application sounds and on is the default
    public bool EnableAppSounds { get; set; } = true;

    // App sounds' volume and *nice* is the default
    public uint AppSoundsVolume { get; set; } = 69; // Always 0<x<100

    // Calibration - if we're calibrated
    public SortedDictionary<string, bool> DeviceMatricesCalibrated { get; set; } = new();

    // Calibration helpers - calibration method: auto? : GUID/Data
    public SortedDictionary<string, bool> DeviceAutoCalibration { get; set; } = new();

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

    // External flip device's calibration rotation
    public Quaternion ExternalFlipCalibrationMatrix = new();

    // If we wanna freeze only lower body trackers or all
    public bool FreezeLowerOnly { get; set; } = false;

    // If the freeze bindings teaching tip has been shown
    public bool TeachingTipShownFreeze { get; set; } = false;

    // If the flip bindings teaching tip has been shown
    public bool TeachingTipShownFlip { get; set; } = false;

    // Already shown toasts vector
    public List<string> ShownToastsGuidVector = new();

    // Disabled (by the user) devices set
    public SortedSet<string> DisabledDevicesGuidSet = new();

    // If the first-launch guide's been shown
    public bool FirstTimeTourShown { get; set; } = false;

    // If the shutdown warning has been shown
    public bool FirstShutdownTipShown { get; set; } = false;

    // Save settings
    public void SaveSettings()
    {
        // TODO IMPL
    }

    // Re/Load settings
    public void ReadSettings()
    {
        // TODO IMPL

        // Check if the trackers vector is broken
        var vectorBroken = K2TrackersVector.Count < 7;

        // Optionally fix the trackers vector
        while (K2TrackersVector.Count < 7)
            K2TrackersVector.Add(new K2AppTracker());

        // Force the first 7 trackers to be the default ones : roles
        K2TrackersVector[0].Role = TrackerType.TrackerWaist;
        K2TrackersVector[1].Role = TrackerType.TrackerLeftFoot;
        K2TrackersVector[2].Role = TrackerType.TrackerRightFoot;
        K2TrackersVector[3].Role = TrackerType.TrackerLeftElbow;
        K2TrackersVector[4].Role = TrackerType.TrackerRightElbow;
        K2TrackersVector[5].Role = TrackerType.TrackerLeftKnee;
        K2TrackersVector[6].Role = TrackerType.TrackerRightKnee;

        foreach (var tracker in K2TrackersVector)
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
            K2TrackersVector[0].IsActive = true;
            K2TrackersVector[1].IsActive = true;
            K2TrackersVector[2].IsActive = true;
        }

        // Scan for duplicate trackers
        foreach (var tracker in K2TrackersVector.GroupBy(x => x.Role)
                     .Select(y => y.First()))
        {
            Logger.Warn("A duplicate tracker was found in the trackers vector! Removing it...");
            K2TrackersVector.Remove(tracker); // Remove the duplicate tracker
        }

        // Check if any trackers are enabled
        // -> No trackers are enabled, force-enable the waist tracker
        if (!K2TrackersVector.Any(x => x.IsActive))
        {
            Logger.Warn("All trackers were disabled, force-enabling the waist tracker!");
            K2TrackersVector[0].IsActive = true; // Enable the waist tracker
        }

        // Fix statuses (optional)
        if (UseTrackerPairs)
        {
            K2TrackersVector[2].IsActive = K2TrackersVector[1].IsActive;
            K2TrackersVector[4].IsActive = K2TrackersVector[3].IsActive;
            K2TrackersVector[6].IsActive = K2TrackersVector[5].IsActive;

            K2TrackersVector[2].OrientationTrackingOption =
                K2TrackersVector[1].OrientationTrackingOption;
            K2TrackersVector[4].OrientationTrackingOption =
                K2TrackersVector[3].OrientationTrackingOption;
            K2TrackersVector[6].OrientationTrackingOption =
                K2TrackersVector[5].OrientationTrackingOption;

            K2TrackersVector[2].PositionTrackingFilterOption =
                K2TrackersVector[1].PositionTrackingFilterOption;
            K2TrackersVector[4].PositionTrackingFilterOption =
                K2TrackersVector[3].PositionTrackingFilterOption;
            K2TrackersVector[6].PositionTrackingFilterOption =
                K2TrackersVector[5].PositionTrackingFilterOption;

            K2TrackersVector[2].OrientationTrackingFilterOption =
                K2TrackersVector[1].OrientationTrackingFilterOption;
            K2TrackersVector[4].OrientationTrackingFilterOption =
                K2TrackersVector[3].OrientationTrackingFilterOption;
            K2TrackersVector[6].OrientationTrackingFilterOption =
                K2TrackersVector[5].OrientationTrackingFilterOption;
        }

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
    }

    public event PropertyChangedEventHandler PropertyChanged;

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(null));
    }
}