using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.Serialization;

namespace Amethyst.Plugins.Contract;

public class TrackedJoint
{
    private Quaternion _orientation = Quaternion.Identity;
    private Vector3 _position = Vector3.Zero;

    [SetsRequiredMembers]
    public TrackedJoint()
    {
        Name = "INVALID";
        Role = TrackedJointType.JointManual;
    }

    /// <summary>
    ///     The joint display name
    /// </summary>
    public required string Name { get; init; }

    /// <summary>
    ///     Joint role type, identifies a joint
    /// </summary>
    public required TrackedJointType Role { get; init; }

    /// <summary>
    ///     Tracker position (in meters, left-handed)
    /// </summary>
    public Vector3 Position
    {
        get => _position;
        set
        {
            PreviousPosition = _position;
            _position = value;

            PreviousPoseTimestamp = PoseTimestamp;
            PoseTimestamp = DateTime.Now.Ticks;
        }
    }

    /// <summary>
    ///     Tracker orientation (quaternion, left-handed)
    /// </summary>
    public Quaternion Orientation
    {
        get => _orientation;
        set
        {
            PreviousOrientation = _orientation;
            _orientation = value;
        }
    }

    /// <summary>
    ///     Tracker previous frame position (in meters, left-handed)
    ///     Auto-computed when you set the base joint position
    /// </summary>
    public Vector3 PreviousPosition { get; private set; } = Vector3.Zero;

    /// <summary>
    ///     Tracker previous frame orientation (quaternion, left-handed)
    ///     Auto-computed when you set the base joint orientation
    /// </summary>
    public Quaternion PreviousOrientation { get; private set; } = Quaternion.Identity;

    /// <summary>
    ///     Tracker position velocity (in meters per second, left-handed)
    /// </summary>
    public Vector3 Velocity { get; set; } = Vector3.Zero;

    /// <summary>
    ///     Tracker position acceleration (in meters per second, left-handed)
    /// </summary>
    public Vector3 Acceleration { get; set; } = Vector3.Zero;

    /// <summary>
    ///     Tracker position angular velocity (in rad/s, euler, left-handed)
    /// </summary>
    public Vector3 AngularVelocity { get; set; } = Vector3.Zero;

    /// <summary>
    ///     Tracker position angular acc (in rad/s, euler, left-handed)
    /// </summary>
    public Vector3 AngularAcceleration { get; set; } = Vector3.Zero;

    /// <summary>
    ///     Tracking state: tracked/occluded/not tracked
    /// </summary>
    public TrackedJointState TrackingState { get; set; } = TrackedJointState.StateTracked;

    /// <summary>
    ///     Pose (position) timestamp in Ticks
    ///     Auto-computed on each pose [position] change
    /// </summary>
    public long PoseTimestamp { get; private set; }

    /// <summary>
    ///     Pose [position] previous frame timestamp in Ticks
    ///     Auto-computed on each pose [position] change
    /// </summary>
    public long PreviousPoseTimestamp { get; private set; }
}

[DataContract]
public class TrackerBase
{
    [SetsRequiredMembers]
    public TrackerBase()
    {
        Serial = "INVALID";
        Role = TrackerType.TrackerHanded;
    }

    /// <summary>
    ///     Connection state: active/non-active
    /// </summary>
    [DataMember(Name = "ConnectionState")]
    public bool ConnectionState { get; set; } = false;

    /// <summary>
    ///     Tracking state: tracked/occluded/not tracked
    /// </summary>
    [DataMember(Name = "TrackingState")]
    public TrackedJointState TrackingState { get; set; } = TrackedJointState.StateNotTracked;

    /// <summary>
    ///     Serial number, or name, identifies a tracker
    /// </summary>
    [DataMember(Name = "TrackerSerial")]
    public required string Serial { get; init; }

    /// <summary>
    ///     Tracker role type
    /// </summary>
    [DataMember(Name = "TrackerRole")]
    public required TrackerType Role { get; init; }

    /// <summary>
    ///     Tracker position (in meters, left-handed)
    /// </summary>
    [DataMember(Name = "Position")]
    public Vector3 Position { get; set; } = Vector3.Zero;

    /// <summary>
    ///     Tracker position (quaternion, left-handed)
    /// </summary>
    [DataMember(Name = "Orientation")]
    public Quaternion Orientation { get; set; } = Quaternion.Identity;

    /// <summary>
    ///     Tracker position velocity (in meters per second, left-handed)
    ///     Leave null to disable physics support and auto-compute
    /// </summary>
    [DataMember(Name = "VelocityNullable")]
    public Vector3? Velocity { get; set; }

    /// <summary>
    ///     Tracker position acceleration (in meters per second, left-handed)
    ///     Leave null to disable physics support and auto-compute
    /// </summary>
    [DataMember(Name = "AccelerationNullable")]
    public Vector3? Acceleration { get; set; }

    /// <summary>
    ///     Tracker position angular velocity (in rad/s, euler, left-handed)
    ///     Leave null to disable physics support and auto-compute
    /// </summary>
    [DataMember(Name = "AngularVelocityNullable")]
    public Vector3? AngularVelocity { get; set; }

    /// <summary>
    ///     Tracker position angular acc (in rad/s, euler, left-handed)
    ///     Leave null to disable physics support and auto-compute
    /// </summary>
    [DataMember(Name = "AngularAccelerationNullable")]
    public Vector3? AngularAcceleration { get; set; }
}

public class InputActions
{
    /// <summary>
    ///     Position move values during manual calibration (x, y, z)
    /// </summary>
    public Vector3 MovePositionValues { get; set; }

    /// <summary>
    ///     Rotation adjust values during manual calibration (pitch, yaw)
    /// </summary>
    public Vector2 AdjustRotationValues { get; set; }

    /// <summary>
    ///     Fire this event to change the calibration mode pos/rot
    /// </summary>
    public EventHandler? CalibrationModeChanged { get; set; }

    /// <summary>
    ///     Fire this event to confirm manual calibration
    /// </summary>
    public EventHandler? CalibrationConfirmed { get; set; }

    /// <summary>
    ///     Used to toggle freeze from another source,
    ///     like input bindings/controllers/whatever else
    /// </summary>
    public EventHandler? TrackingFreezeToggled { get; set; }

    /// <summary>
    ///     Used to toggle skeleton flip from another source,
    ///     like input bindings/controllers/whatever else
    /// </summary>
    public EventHandler? SkeletonFlipToggled { get; set; }

    /// <summary>
    ///     [Title]
    ///     Shown when tracking freeze is toggled manually,
    ///     explains what to press to toggle it from here
    ///     Leave null or empty to skip showing anything
    /// </summary>
    public string? TrackingFreezeActionTitleString { get; set; }

    /// <summary>
    ///     [Title]
    ///     Shown when tracking freeze is toggled manually,
    ///     explains what to press to toggle it from here
    ///     Leave null or empty to skip showing anything
    /// </summary>
    public string? TrackingFreezeActionContentString { get; set; }

    /// <summary>
    ///     [Title]
    ///     Shown when skeleton flip is toggled manually,
    ///     explains what to press to toggle it from here
    ///     Leave null or empty to skip showing anything
    /// </summary>
    public string? SkeletonFlipActionTitleString { get; set; }

    /// <summary>
    ///     [Title]
    ///     Shown when skeleton flip is toggled manually,
    ///     explains what to press to toggle it from here
    ///     Leave null or empty to skip showing anything
    /// </summary>
    public string? SkeletonFlipActionContentString { get; set; }
}

// Dependency installer progress helper class
public class InstallationProgress
{
    /// <summary>
    ///     Progress value, optional, 0.0 --- 1.0
    /// </summary>
    public double? OverallProgress { get; init; }

    /// <summary>
    ///     Progress indicator, set to false for OverallProgress
    /// </summary>
    public bool IsIndeterminate { get; init; } = false;

    /// <summary>
    ///     [Title]
    ///     The current stage name of the installation
    /// </summary>
    public string StageTitle { get; init; } = string.Empty;
}

// Dependency installer worker helper class
public interface IDependency
{
    /// <summary>
    ///     [Title]
    ///     The name of the dependency to be installed
    /// </summary>
    public string Name { get; }

    /// <summary>
    ///     Whether it is a must to have this dependency installed
    ///     (IsInstalled=true) for the plugin to be working properly
    /// </summary>
    public bool IsMandatory { get; }

    /// <summary>
    ///     Check whether the dependency is (already) installed
    /// </summary>
    public bool IsInstalled { get; }

    /// <summary>
    ///     If there is a need to accept an EULA, pass its contents here
    /// </summary>
    public string InstallerEula { get; }

    /// <summary>
    ///     Perform the main installation action, reporting the progress
    /// </summary>
    /// <param name="progress">
    ///     Report the progress using InstallationProgress
    /// </param>
    /// <param name="cancellationToken">
    ///     CancellationToken for task cancellation
    /// </param>
    /// <returns>
    ///     Success?
    /// </returns>
    public Task<bool> Install(IProgress<InstallationProgress> progress, CancellationToken cancellationToken);
}


// Fix applier worker helper class
public interface IFix
{
	/// <summary>
	///     [Title]
	///     The name of the fix to be applied
	/// </summary>
	public string Name { get; }

	/// <summary>
	///     Whether it is a must to have this fix applied
	///     (IsNecessary=true) for the plugin to be working properly
	/// </summary>
	public bool IsMandatory { get; }

	/// <summary>
	///     Check whether the fix is (already) applied
	/// </summary>
	public bool IsNecessary { get; }

	/// <summary>
	///     If there is a need to accept an EULA, pass its contents here
	/// </summary>
	public string InstallerEula { get; }

	/// <summary>
	///     Perform the main installation action, reporting the progress
	/// </summary>
	/// <param name="progress">
	///     Report the progress using InstallationProgress
	/// </param>
	/// <param name="cancellationToken">
	///     CancellationToken for task cancellation
	/// </param>
	/// <param name="arg">
	///     Optional argument
	/// </param>
	/// <returns>
	///     Success?
	/// </returns>
	public Task<bool> Apply(IProgress<InstallationProgress> progress, CancellationToken cancellationToken, object? arg = null);
}

// Plugin settings helper class
public interface IPluginSettings
{
    // Get a serialized object from the plugin settings
    public T? GetSetting<T>(object key, T? fallback = default);

    // Write a serialized object to the plugin settings
    public void SetSetting<T>(object key, T? value);
}