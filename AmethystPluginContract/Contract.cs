using System.ComponentModel;
using System.Numerics;

namespace Amethyst.Plugins.Contract;

public interface ITrackingDeviceMetadata
{
    string Name { get; }
    string Guid { get; }
}

[AttributeUsage(AttributeTargets.Class)]
public sealed class TrackingDeviceMetadataAttribute : Attribute, ITrackingDeviceMetadata
{
    public TrackingDeviceMetadataAttribute(string name, string guid)
    {
        Name = name;
        Guid = guid;
    }

    public string Name { get; set; }
    public string Guid { get; set; }
}

public interface ITrackingDevice
{
    // Joints' list / you need to (should) update at every update() call
    // Each must have its own role or _Manual to force user's manual set
    List<TrackedJoint> TrackedJoints { get; }

    // Is the device connected/started?
    [DefaultValue(false)] bool IsInitialized { get; }

    // This should be updated on every frame,
    // along with joint devices
    // -> will lead to global tracking loss notification
    //    if set to false at runtime somewhen
    [DefaultValue(false)] bool IsSkeletonTracked { get; }

    // Should be set up at construction
    // This will tell Amethyst to disable all position filters on joints managed by this plugin
    [DefaultValue(false)] bool IsPositionFilterBlockingEnabled { get; }

    // Should be set up at construction
    // This will tell Amethyst not to auto-manage on joints managed by this plugin
    // Includes: velocity, acceleration, angular velocity, angular acceleration
    [DefaultValue(false)] bool IsPhysicsOverrideEnabled { get; }

    // Should be set up at construction
    // This will tell Amethyst not to auto-update this device
    // You should register some timer to update your device yourself
    [DefaultValue(false)] bool IsSelfUpdateEnabled { get; }

    // Should be set up at construction
    // Mark this as false ALSO if your device supports 360 tracking by itself
    [DefaultValue(false)] bool IsFlipSupported { get; }

    // Should be set up at construction
    // This will allow Amethyst to calculate rotations by itself, additionally
    [DefaultValue(false)] bool IsAppOrientationSupported { get; }

    // To support settings daemon and register the layout root,
    // the device must properly report it first
    // -> will lead to showing an additional 'settings' button
    // Note: each device has to save its settings independently
    //       and may use the K2AppData from the Paths' class
    // Tip: you can hide your device's settings by marking this as 'false',
    //      and change it back to 'true' when you're ready
    [DefaultValue(false)] bool IsSettingsDaemonSupported { get; }

    // Settings UI root / MUST BE OF TYPE Microsoft.UI.Xaml.Controls.Page
    // Return new() of your implemented Page, and that's basically it!
    object SettingsInterfaceRoot { get; }

    // These will indicate the device's status [OK is (int)0]
    // Both should be updated either on call or as frequent as possible
    [DefaultValue(-1)] int DeviceStatus { get; }

    // Device status string: to get system locale/language, use GetUserDefaultUILanguage
    [DefaultValue("Not Defined\nE_NOT_DEFINED\nstatusResultWString behaviour not defined")]
    string DeviceStatusString { get; }

    // This is called after the app loads the plugin
    void OnLoad();

    // This initializes/connects the device
    void Initialize();

    // This is called when the device is closed
    void Shutdown();

    // This is called to update the device (each loop)
    void Update();

    // Signal the joint eg psm_id0 that it's been selected
    void SignalJoint(int jointId);
}

// Import this interface to invoke AME methods
public interface IAmethystHost
{
    // Helper to get all joints' positions from the app, which are added in Amethyst.
    // Note: if joint's off, its trackingState will be ITrackedJointState::State_NotTracked
    List<TrackedJoint> AppJointPoses { get; }

    // Get the HMD Yaw (exclusively)
    double HmdOrientationYaw { get; }

    // Get the HMD Yaw (exclusively), but un-wrapped aka "calibrated" using the VR room center
    double HmdOrientationYawCalibrated { get; }

    // Get the raw OpenVRs HMD pose
    (Vector3 Position, Quaternion Orientation) HmdPose { get; }

    // Get the OpenVRs HMD pose, but un-wrapped aka "calibrated" using the vr room center
    (Vector3 Position, Quaternion Orientation) HmdPoseCalibrated { get; }

    // Get the raw OpenVRs left controller pose
    (Vector3 Position, Quaternion Orientation) LeftControllerPose { get; }

    // Get the OpenVRs left controller pose, but un-wrapped aka "calibrated" using the vr room center
    (Vector3 Position, Quaternion Orientation) LeftControllerPoseCalibrated { get; }

    // Get the raw OpenVRs right controller pose
    (Vector3 Position, Quaternion Orientation) RightControllerPose { get; }

    // Get the OpenVRs right controller pose, but un-wrapped aka "calibrated" using the vr room center
    (Vector3 Position, Quaternion Orientation) RightControllerPoseCalibrated { get; }

    // Get Amethyst UI language
    string LanguageCode { get; }

    // Log a message to Amethyst logs : handler
    void Log(string message, LogSeverity severity);

    // Request a refresh of the status/name/etc. interface
    void RefreshStatusInterface();

    // Request a string from AME resources, empty for no match
    // Warning: The primarily searched resource is the device-provided one!
    string RequestLocalizedString(string key, string guid);

    // Request a folder to be set as device's AME resources,
    // you can access these resources with the lower function later (after onLoad)
    // Warning: Resources are containerized and can't be accessed in-between devices!
    // Warning: The default root is "[device_folder_path]/resources/Strings"!
    bool SetLocalizationResourcesRoot(string path, string guid);
}