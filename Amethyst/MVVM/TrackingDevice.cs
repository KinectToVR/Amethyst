using System;
using Amethyst.Plugins.Contract;
using System.Collections.Generic;
using System.ComponentModel;
using System.ComponentModel.Composition;
using System.ComponentModel.Composition.Hosting;
using System.ComponentModel.Composition.Primitives;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Reflection;
using Amethyst.Classes;
using Amethyst.Utils;

namespace Amethyst.MVVM;

public class TrackingDevice : INotifyPropertyChanged
{
    public TrackingDevice(string name, string guid, string path, ITrackingDevice device)
    {
        Guid = guid;
        Name = name;
        Location = path;
        Device = device;
    }

    // Extensions: is this device set as base?
    public bool IsBase => TrackingDevices.IsBase(Guid);

    // Extensions: is this device set as an override?
    public bool IsOverride => TrackingDevices.IsOverride(Guid);

    // Get GUID
    [DefaultValue("INVALID")] public string Guid { get; }

    // Get Name
    [DefaultValue("UNKNOWN")] public string Name { get; }

    // Get Path
    [DefaultValue("UNKNOWN")] public string Location { get; }

    private ITrackingDevice Device { get; init; }

    public List<(TrackedJointType Role, TrackedJoint Joint)> TrackedJoints => Device.TrackedJoints;

    public (Windows.Data.Json.JsonObject Root, string Directory) LocalizationResourcesRoot { get; set; } = new();

    public void OnLoad()
    {
        Device.OnLoad();
    }

    public void Initialize()
    {
        Device.Initialize();
    }

    public void Shutdown()
    {
        Device.Shutdown();
    }

    public void Update()
    {
        Device.Update();
    }

    public bool IsInitialized => Device.IsInitialized;
    public bool IsSkeletonTracked => Device.IsSkeletonTracked;
    public bool IsPositionFilterBlockingEnabled => Device.IsPositionFilterBlockingEnabled;
    public bool IsPhysicsOverrideEnabled => Device.IsPhysicsOverrideEnabled;
    public bool IsSelfUpdateEnabled => Device.IsSelfUpdateEnabled;
    public bool IsFlipSupported => Device.IsFlipSupported;
    public bool IsSettingsDaemonSupported => Device.IsSettingsDaemonSupported;

    public bool IsAppOrientationSupported =>
        Device.IsAppOrientationSupported && // The device must declare it actually consists
        Device.TrackedJoints.Any(x => x.Role == TrackedJointType.JointAnkleLeft) &&
        Device.TrackedJoints.Any(x => x.Role == TrackedJointType.JointAnkleRight) &&
        Device.TrackedJoints.Any(x => x.Role == TrackedJointType.JointFootLeft) &&
        Device.TrackedJoints.Any(x => x.Role == TrackedJointType.JointFootRight) &&
        Device.TrackedJoints.Any(x => x.Role == TrackedJointType.JointKneeLeft) &&
        Device.TrackedJoints.Any(x => x.Role == TrackedJointType.JointKneeRight);

    public object SettingsInterfaceRoot => Device.SettingsInterfaceRoot;
    public int DeviceStatus => Device.DeviceStatus;
    public string DeviceStatusString => Device.DeviceStatusString;

    // Signal the joint eg psm_id0 that it's been selected
    public void SignalJoint(int jointId)
    {
        Device.SignalJoint(jointId);
    }

    public event PropertyChangedEventHandler PropertyChanged;

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    // MVVM stuff
    public double BoolToOpacity(bool value)
    {
        return value ? 1.0 : 0.0;
    }

    public bool StatusOk => Device.DeviceStatus == 0;
    public bool StatusError => Device.DeviceStatus != 0;
}

public static class ICollectionExtensions
{
    public static bool AddPlugin<T>(this ICollection<T> collection, DirectoryInfo item) where T : ComposablePartCatalog
    {
        foreach (var fileInfo in item.GetFiles("*.dll"))
            try
            {
                var assemblyFile = Assembly.LoadFile(fileInfo.FullName);
                var assemblyCatalog = new AssemblyCatalog(assemblyFile);

                if (!assemblyCatalog.Parts.Any(x => x.ExportDefinitions
                        .Any(y => y.ContractName == typeof(ITrackingDevice).FullName))) continue;

                collection.Add((T)(object)assemblyCatalog);
                return true; // This plugin is probably supported, yay!
            }
            catch (CompositionException e)
            {
                Logger.Error($"Loading {fileInfo} failed with a composition exception: " +
                             $"Message: {e.Message}\nErrors occurred: {e.Errors}\nPossible causes: {e.RootCauses}");
            }
            catch (Exception e)
            {
                Logger.Error($"Loading {fileInfo} failed with an exception: Message: {e.Message}" +
                             "Probably some assembly referenced by this plugin is missing.");
            }

        return true; // Nah, not this time
    }
}

[Export(typeof(IAmethystHost))]
public class PluginHost : IAmethystHost
{
    // Helper to get all joints' positions from the app, which are added in Amethyst.
    // Note: if joint's off, its trackingState will be ITrackedJointState::State_NotTracked
    public List<TrackedJoint> AppJointPoses =>
        AppData.Settings.TrackersVector.Select(x => x.GetTrackedJoint()).ToList();

    // Get the HMD Yaw (exclusively)
    public double HmdOrientationYaw =>
        AmethystSupport.Calibration.QuaternionYaw(Interfacing.Plugins.GetHmdPose.Orientation);

    // Get the HMD Yaw (exclusively), but un-wrapped aka "calibrated" using the VR room center
    public double HmdOrientationYawCalibrated =>
        AmethystSupport.Calibration.QuaternionYaw(Interfacing.Plugins.GetHmdPoseCalibrated.Orientation);

    // Get the raw OpenVRs HMD pose
    public (Vector3 Position, Quaternion Orientation) HmdPose => Interfacing.Plugins.GetHmdPose;

    // Get the OpenVRs HMD pose, but un-wrapped aka "calibrated" using the vr room center
    public (Vector3 Position, Quaternion Orientation) HmdPoseCalibrated => Interfacing.Plugins.GetHmdPoseCalibrated;

    // Get the raw OpenVRs left controller pose
    public (Vector3 Position, Quaternion Orientation) LeftControllerPose => Interfacing.Plugins.GetLeftControllerPose();

    // Get the OpenVRs left controller pose, but un-wrapped aka "calibrated" using the vr room center
    public (Vector3 Position, Quaternion Orientation) LeftControllerPoseCalibrated =>
        Interfacing.Plugins.GetLeftControllerPoseCalibrated();

    // Get the raw OpenVRs right controller pose
    public (Vector3 Position, Quaternion Orientation) RightControllerPose =>
        Interfacing.Plugins.GetRightControllerPose();

    // Get the OpenVRs right controller pose, but un-wrapped aka "calibrated" using the vr room center
    public (Vector3 Position, Quaternion Orientation) RightControllerPoseCalibrated =>
        Interfacing.Plugins.GetRightControllerPoseCalibrated();

    // Log a message to Amethyst logs : handler
    public void Log(string message, LogSeverity severity)
    {
        switch (severity)
        {
            case LogSeverity.Fatal:
                Logger.Fatal(message);
                break;

            case LogSeverity.Error:
                Logger.Error(message);
                break;

            case LogSeverity.Warning:
                Logger.Warn(message);
                break;

            case LogSeverity.Info:
            default:
                Logger.Info(message);
                break;
        }
    }

    // Request a refresh of the status/name/etc. interface
    public void RefreshStatusInterface()
    {
        Interfacing.StatusUiRefreshRequestedUrgent = true;
    }

    // Get Amethyst UI language
    public string LanguageCode => AppData.Settings.AppLanguage;

    // Request a string from AME resources, empty for no match
    // Warning: The primarily searched resource is the device-provided one!
    public string RequestLocalizedString(string key, string guid)
    {
        return Interfacing.Plugins.RequestLocalizedString(key, guid);
    }

    // Request a folder to be set as device's AME resources,
    // you can access these resources with the lower function later (after onLoad)
    // Warning: Resources are containerized and can't be accessed in-between devices!
    // Warning: The default root is "[device_folder_path]/resources/Strings"!
    public bool SetLocalizationResourcesRoot(string path, string guid)
    {
        return Interfacing.Plugins.SetLocalizationResourcesRoot(path, guid);
    }
}

public class LoadAttemptedPlugin : INotifyPropertyChanged
{
    public string Name { get; init; } = "[UNKNOWN]";
    public string Guid { get; init; } = "[INVALID]";

    public string DeviceFolder { get; init; } = "[INVALID]";
    public string DeviceProviderName { get; init; } = "[UNKNOWN]";
    public string DeviceUpdateUri { get; init; } = "[UNKNOWN]";
    public string DeviceVersion { get; init; } = "[UNKNOWN]";
    public string DeviceApiVersion { get; init; } = "[INVALID]";

    // MVVM stuff
    public string GetResourceString(string key)
    {
        return Interfacing.LocalizedJsonString(key);
    }

    public TrackingDevices.PluginLoadError Status { get; init; } =
        TrackingDevices.PluginLoadError.Unknown;

    public bool LoadError => Status != TrackingDevices.PluginLoadError.NoError;
    public bool LoadSuccess => Status == TrackingDevices.PluginLoadError.NoError;

    private bool _isLoaded = false;

    public bool IsLoaded
    {
        get => TrackingDevices.TrackingDevicesList.ContainsKey(Guid);
        set
        {
            if (_isLoaded == value) return; // No changes
            _isLoaded = value; // Copy to the private container

            // Disable/Enable this plugin
            if (value) AppData.Settings.DisabledDevicesGuidSet.Remove(Guid);
            else AppData.Settings.DisabledDevicesGuidSet.Add(Guid);

            // Check if the change is valid
            if (!_isLoaded)
            {
                SortedSet<string> loadedDeviceSet = new();

                // Check which devices are loaded
                if (TrackingDevices.TrackingDevicesList.ContainsKey("K2VRTEAM-AME1-API1-DVCE-DVCEKINECTV1"))
                    loadedDeviceSet.Add("K2VRTEAM-AME1-API1-DVCE-DVCEKINECTV1");
                if (TrackingDevices.TrackingDevicesList.ContainsKey("K2VRTEAM-AME1-API1-DVCE-DVCEKINECTV2"))
                    loadedDeviceSet.Add("K2VRTEAM-AME1-API1-DVCE-DVCEKINECTV2");
                if (TrackingDevices.TrackingDevicesList.ContainsKey("K2VRTEAM-AME1-API1-DVCE-DVCEPSMOVEEX"))
                    loadedDeviceSet.Add("K2VRTEAM-AME1-API1-DVCE-DVCEPSMOVEEX");
                if (TrackingDevices.TrackingDevicesList.ContainsKey("K2VRTEAM-VEND-API1-DVCE-DVCEOWOTRACK"))
                    loadedDeviceSet.Add("K2VRTEAM-VEND-API1-DVCE-DVCEOWOTRACK");

                // If we've just disabled the last loaded device, re-enable the first
                if (TrackingDevices.TrackingDevicesList.Keys.All(
                        AppData.Settings.DisabledDevicesGuidSet.Contains) ||

                    // If this device entry happens to be the last one of the official ones
                    (loadedDeviceSet.Contains(Guid) && loadedDeviceSet.All(
                        AppData.Settings.DisabledDevicesGuidSet.Contains)))
                {
                    AppData.Settings.DisabledDevicesGuidSet.Remove(Guid);
                    _isLoaded = true; // Re-enable this device
                }
            }

            // Show the reload tip on any valid changes
            // == cause the upper check would make it different
            // and it's already been assigned at the beginning
            if (Shared.Devices.PluginsPageLoadedOnce && _isLoaded == value)
                Shared.TeachingTips.MainPage.ReloadTeachingTip.IsOpen = true;

            // Save settings
            AppData.Settings.SaveSettings();
            OnPropertyChanged("IsLoaded");
        }
    }

    public string ErrorText => GetResourceString($"/DevicesPage/Devices/Manager/Labels/{(int)Status}");

    public string TrimString(string s, int l)
    {
        return s[..Math.Min(s.Length, l)];
    }

    public void ShowDeviceFolder()
    {
        SystemShell.OpenFolderAndSelectItem(DeviceFolder);
    }

    public event PropertyChangedEventHandler PropertyChanged;

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }
}