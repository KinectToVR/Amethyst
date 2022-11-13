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
    public bool IsAppOrientationSupported => Device.IsAppOrientationSupported;

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
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(null));
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
    public double HmdOrientationYaw { get; } = 0; // TODO

    // Get the HMD Yaw (exclusively), but un-wrapped aka "calibrated" using the VR room center
    public double HmdOrientationYawCalibrated { get; } = 0; // TODO

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
    public void RefreshStatusInterface() => Interfacing.StatusUiRefreshRequestedUrgent = true;

    // Get Amethyst UI language
    public string LanguageCode => AppData.Settings.AppLanguage;

    // Request a string from AME resources, empty for no match
    // Warning: The primarily searched resource is the device-provided one!
    public string RequestLocalizedString(string key, string guid) => ""; // TODO

    // Request a folder to be set as device's AME resources,
    // you can access these resources with the lower function later (after onLoad)
    // Warning: Resources are containerized and can't be accessed in-between devices!
    // Warning: The default root is "[device_folder_path]/resources/Strings"!
    public bool SetLocalizationResourcesRoot(string path, string guid) => false; // TODO
}