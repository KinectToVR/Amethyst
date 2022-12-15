using System.ComponentModel;
using System.Numerics;
using System.Runtime.CompilerServices;

namespace Amethyst.Plugins.Contract;

/// <summary>
///     Helper class for the plugin metadata types
/// </summary>
/// <example>
///     Sample exported plugin class decorations:
///     [ExportMetadata("Name", "SampleDevice")]
///     [ExportMetadata("Guid", "SCONTOSO-AME2-APII-DVCE-DVCESMPLDVCE")]
///     [ExportMetadata("Publisher", "Contoso")]            // Optional
///     [ExportMetadata("Website", "https://contoso.com")]  // Optional
/// </example>
public interface IPluginMetadata
{
    string Name { get; }
    string Guid { get; }

    [DefaultValue(null)] string Publisher { get; }
    [DefaultValue(null)] string Website { get; }
}

/// <summary>
///     Implement this interface to provide tracking data to Amethyst
///     Your plugin will need to be explicitly selected by the user
/// </summary>
public interface ITrackingDevice
{
    /// <summary>
    ///     Joints' list / you need to (should) update at every update() call
    ///     Each must have its own role or _Manual to force user's manual set
    /// </summary>
    List<TrackedJoint> TrackedJoints { get; }

    /// <summary>
    ///     Is the device connected/started?
    /// </summary>
    [DefaultValue(false)]
    bool IsInitialized { get; }

    /// <summary>
    ///     This should be updated on every frame,
    ///     along with joint devices
    ///     -> will lead to global tracking loss notification
    ///     if set to false at runtime some-when
    /// </summary>
    [DefaultValue(false)]
    bool IsSkeletonTracked { get; }

    /// <summary>
    ///     Should be set up at construction
    ///     This will tell Amethyst to disable all position filters on joints managed by this plugin
    /// </summary>
    [DefaultValue(false)]
    bool IsPositionFilterBlockingEnabled { get; }

    /// <summary>
    ///     Should be set up at construction
    ///     This will tell Amethyst not to auto-manage on joints managed by this plugin
    ///     Includes: velocity, acceleration, angular velocity, angular acceleration
    /// </summary>
    [DefaultValue(false)]
    bool IsPhysicsOverrideEnabled { get; }

    /// <summary>
    ///     Should be set up at construction
    ///     This will tell Amethyst not to auto-update this device
    ///     You should register some timer to update your device yourself
    /// </summary>
    [DefaultValue(false)]
    bool IsSelfUpdateEnabled { get; }

    /// <summary>
    ///     Should be set up at construction
    ///     Mark this as false ALSO if your device supports 360 tracking by itself
    /// </summary>
    [DefaultValue(false)]
    bool IsFlipSupported { get; }

    /// <summary>
    ///     Should be set up at construction
    ///     This will allow Amethyst to calculate rotations by itself, additionally
    /// </summary>
    [DefaultValue(false)]
    bool IsAppOrientationSupported { get; }

    /// <summary>
    ///     To support settings daemon and register the layout root,
    ///     the device must properly report it first
    ///     -> will lead to showing an additional 'settings' button
    ///     Note: each device has to save its settings independently
    ///     and may use the K2AppData from the Paths' class
    ///     Tip: you can hide your device's settings by marking this as 'false',
    ///     and change it back to 'true' when you're ready
    /// </summary>
    [DefaultValue(false)]
    bool IsSettingsDaemonSupported { get; }

    /// <summary>
    ///     Settings UI root / MUST BE OF TYPE Microsoft.UI.Xaml.Controls.Page
    ///     Return new() of your implemented Page, and that's basically it!
    /// </summary>
    object SettingsInterfaceRoot { get; }

    /// <summary>
    ///     These will indicate the device's status [OK is (int)0]
    ///     Both should be updated either on call or as frequent as possible
    /// </summary>
    [DefaultValue(-1)]
    int DeviceStatus { get; }

    /// <summary>
    ///     Device status string: to get your resources, use RequestLocalizedString
    /// </summary>
    [DefaultValue("Not Defined\nE_NOT_DEFINED\nStatus message not defined!")]
    string DeviceStatusString { get; }

    /// <summary>
    ///     This is called after the app loads the plugin
    /// </summary>
    void OnLoad();

    /// <summary>
    ///     This initializes/connects the device
    /// </summary>
    void Initialize();

    /// <summary>
    ///     This is called when the device is closed
    /// </summary>
    void Shutdown();

    /// <summary>
    ///     This is called to update the device (each loop)
    /// </summary>
    void Update();

    /// <summary>
    ///     Signal the joint eg psm_id0 that it's been selected
    /// </summary>
    void SignalJoint(int jointId);
}

/// <summary>
///     Implement this interface to receive tracking data from Amethyst
///     Your plugin will need to be explicitly selected by the user
/// </summary>
public interface IServiceEndpoint
{
    /// <summary>
    ///     To support settings daemon and register the layout root,
    ///     the device must properly report it first
    ///     -> will lead to showing an additional 'settings' button
    ///     Note: each device has to save its settings independently
    ///     and may use the K2AppData from the Paths' class
    ///     Tip: you can hide your device's settings by marking this as 'false',
    ///     and change it back to 'true' when you're ready
    /// </summary>
    [DefaultValue(false)]
    bool IsSettingsDaemonSupported { get; }

    /// <summary>
    ///     Settings UI root / MUST BE OF TYPE Microsoft.UI.Xaml.Controls.Page
    ///     Return new() of your implemented Page, and that's basically it!
    /// </summary>
    object SettingsInterfaceRoot { get; }

    /// <summary>
    ///     These will indicate the device's status [OK is (int)0]
    ///     Both should be updated either on call or as frequent as possible
    /// </summary>
    [DefaultValue(-1)]
    int ServiceStatus { get; }

    /// <summary>
    ///     Device status string: to get your resources, use RequestLocalizedString
    /// </summary>
    [DefaultValue("Not Defined\nE_NOT_DEFINED\nStatus message not defined!")]
    string ServiceStatusString { get; }

    /// <summary>
    ///     Additional supported tracker types set
    ///     The mandatory ones are: waist, left foot, and right foot
    /// </summary>
    public SortedSet<TrackerType> AdditionalSupportedTrackerTypes { get; }

    /// <summary>
    ///     Mark as true to tell the user that they need to restart/
    ///     /in case they want to add more trackers after spawning
    ///     This is the case with OpenVR, where settings need to be reloaded
    /// </summary>
    [DefaultValue(false)]
    public bool IsRestartOnChangesNeeded { get; }

    /// <summary>
    ///     Controller input actions, for calibration and others
    ///     Also provides support for flip/freeze quick toggling
    ///     Leaving this null will result in marking the
    ///     manual calibration and input actions support as [false]
    /// </summary>
    public InputActions ControllerInputActions { get; set; }

    /// <summary>
    ///     Check or set if starting the service should auto-start Amethyst
    ///     This is only available for a few actual cases, like OpenVR
    /// </summary>
    [DefaultValue(false)]
    public bool AutoStartAmethyst { get; set; }

    /// <summary>
    ///     Check or set if closing the service should auto-close Amethyst
    ///     This is only available for a few actual cases, like OpenVR
    /// </summary>
    [DefaultValue(true)]
    public bool AutoCloseAmethyst { get; set; }

    /// <summary>
    ///     Check if Amethyst is shown in the service dashboard or similar
    ///     This is only available for a few actual cases, like OpenVR
    /// </summary>
    [DefaultValue(true)]
    public bool IsAmethystVisible { get; }

    /// <summary>
    ///     Check running system name, this is important for input
    /// </summary>
    /// <example>
    ///     "Oculus" | "VIVE" | "Index" | "WMR" | ...
    /// </example>
    [DefaultValue("Oculus")]
    public string TrackingSystemName { get; }

    /// <summary>
    ///     Get the absolute pose of the HMD, calibrated against the play space
    ///     Return null if unknown to the service or unavailable
    ///     You'll need to provide this to support automatic calibration
    /// </summary>
    public (Vector3 Position, Quaternion Orientation)? HeadsetPose { get; }

    /// <summary>
    ///     This is called after the app loads the plugin
    /// </summary>
    void OnLoad();

    /// <summary>
    ///     This is called right before the pose compose
    /// </summary>
    void Heartbeat();

    /// <summary>
    ///     This initializes/connects to the service
    /// </summary>
    int Initialize();

    /// <summary>
    ///     This is called when the service is closed
    /// </summary>
    void Shutdown();

    /// <summary>
    ///     Implement if your service supports custom toasts
    ///     Services like OpenVR can show internal toasts
    /// </summary>
    void DisplayToast((string Title, string Text) message);

    /// <summary>
    ///     Request a restart of the tracking endpoint service
    /// </summary>
    public bool? RequestServiceRestart(string reason, bool wantReply = false);

    /// <summary>
    ///     Find an already-existing tracker and get its pose
    ///     For no results found return null, also check if it's from amethyst
    /// </summary>
    public TrackerBase GetTrackerPose(string contains, bool canBeFromAmethyst = true);

    /// <summary>
    ///     Set tracker states, add/spawn if not present yet
    ///     Default to the serial, update the role if needed
    ///     Returns the same vector with paired success property (or null)
    /// </summary>
    public Task<IEnumerable<(TrackerBase Tracker, bool Success)>> SetTrackerStates(
        IEnumerable<TrackerBase> trackerBases, bool wantReply = true);

    /// <summary>
    ///     Update tracker positions and physics components
    ///     Check physics against null, they're passed as optional
    ///     Returns the same vector with paired success property (or null)
    /// </summary>
    public Task<IEnumerable<(TrackerBase Tracker, bool Success)>> UpdateTrackerPoses(
        IEnumerable<TrackerBase> trackerBases, bool wantReply = true);

    /// <summary>
    ///     Check connection: status, serialized status, combined ping time
    /// </summary>
    public Task<(int Status, string StatusMessage, long PingTime)> TestConnection();
}

/// <summary>
///     Import this interface to invoke AME methods
/// </summary>
public interface IAmethystHost
{
    /// <summary>
    ///     Helper to get all joints' positions from the app, which are added in Amethyst.
    ///     Note: if joint's off, its trackingState will be ITrackedJointState::State_NotTracked
    ///     Note: [AppJointPoses] will always be returned raw and w/o tweaks/offsets
    ///     /     if need the final ones, please consider writing an IServiceEndpoint
    /// </summary>
    List<TrackedJoint> AppJointPoses { get; }

    /// <summary>
    ///     Get the raw HMD yaw from the current service endpoint
    ///     Note: or 0 if the current service doesn't provide one
    /// </summary>
    double HmdOrientationYaw { get; }

    /// <summary>
    ///     Get the raw HMD pose from the current service endpoint
    ///     or (zero, identity) if the service doesn't provide one
    /// </summary>
    (Vector3 Position, Quaternion Orientation) HmdPose { get; }

    /// <summary>
    ///     Get Amethyst UI language
    /// </summary>
    string LanguageCode { get; }

    /// <summary>
    ///     Get/Set a serialized object from/to the plugin settings
    ///     Access either via GetPluginSetting/SetPluginSetting /
    ///     / or an indexer ('["key"]' get / '["key"] = value' set)
    /// </summary>
    public IPluginSettings PluginSettings { get; }

    /// <summary>
    ///     Get the hook joint pose (typically Head, fallback to .First())
    ///     of the currently selected base tracking device (no overrides!)
    ///     Mark [calibrated] as [true] to get the calibrated joint pose
    ///     Note: [AppJointPoses] will always be returned raw and w/o tweaks/offsets
    ///     /     if need the final ones, please consider writing an IServiceEndpoint
    /// </summary>
    (Vector3 Position, Quaternion Orientation) GetHookJointPose(bool calibrated = false);

    /// <summary>
    ///     Get the pose of the relative transform origin joint
    ///     (typically Waist, fallback to .First(), then default)
    ///     of the currently selected base device (no overrides!)
    ///     Mark [calibrated] as [true] to get the calibrated pose
    ///     Note: [AppJointPoses] will always be returned raw and w/o tweaks/offsets
    ///     /     if need the final ones, please consider writing an IServiceEndpoint
    /// </summary>
    (Vector3 Position, Quaternion Orientation) GetTransformJointPose(bool calibrated = false);

    /// <summary>
    ///     Check if the base tracking device provides a joint with the selected role
    ///     Note: this only applies to the tracking device set as base, not overrides
    /// </summary>
    bool IsTrackedJointValid(TrackedJointType jointType);

    /// <summary>
    ///     Log a message to Amethyst logs : handler
    /// </summary>
    void Log(string message, LogSeverity severity = LogSeverity.Info, [CallerLineNumber] int lineNumber = 0,
        [CallerFilePath] string filePath = "", [CallerMemberName] string memberName = "");

    /// <summary>
    ///     Log a message to Amethyst logs : handler
    /// </summary>
    void Log(object message, LogSeverity severity = LogSeverity.Info, [CallerLineNumber] int lineNumber = 0,
        [CallerFilePath] string filePath = "", [CallerMemberName] string memberName = "");

    /// <summary>
    ///     Play a sound from the resources
    /// </summary>
    void PlayAppSound(SoundType sound);

    /// <summary>
    ///     Request a refresh of the status/name/etc. interface
    /// </summary>
    void RefreshStatusInterface();

    /// <summary>
    ///     Request a string from AME resources, empty for no match
    ///     Warning: The primarily searched resource is the device-provided one!
    /// </summary>
    string RequestLocalizedString(string key);

    /// <summary>
    ///     Request a folder to be set as device's AME resources,
    ///     you can access these resources with the lower function later (after onLoad)
    ///     Warning: Resources are containerized and can't be accessed in-between devices!
    ///     Warning: The default root is "[plugin_folder_path]/resources/Strings"!
    /// </summary>
    bool SetLocalizationResourcesRoot(string path);

    /// <summary>
    ///     Show a Windows toast notification
    /// </summary>
    void DisplayToast((string Title, string Text) message);

    /// <summary>
    ///     Request an application exit, non-fatal by default
    ///     Mark fatal as true to show the crash handler with your message
    /// </summary>
    void RequestExit(string message, bool fatal = false);
}