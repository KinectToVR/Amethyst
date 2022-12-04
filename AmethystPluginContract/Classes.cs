using System.Diagnostics.CodeAnalysis;
using System.Numerics;

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
    public bool ConnectionState { get; set; } = false;

    /// <summary>
    ///     Serial number, or name, identifies a tracker
    /// </summary>
    public required string Serial { get; init; }

    /// <summary>
    ///     Tracker role type
    /// </summary>
    public required TrackerType Role { get; init; }

    /// <summary>
    ///     Tracker position (in meters, left-handed)
    /// </summary>
    public Vector3 Position { get; set; } = Vector3.Zero;

    /// <summary>
    ///     Tracker position (quaternion, left-handed)
    /// </summary>
    public Quaternion Orientation { get; set; } = Quaternion.Identity;

    /// <summary>
    ///     Tracker position velocity (in meters per second, left-handed)
    ///     Leave null to disable physics support and auto-compute
    /// </summary>
    public Vector3? Velocity { get; set; }

    /// <summary>
    ///     Tracker position acceleration (in meters per second, left-handed)
    ///     Leave null to disable physics support and auto-compute
    /// </summary>
    public Vector3? Acceleration { get; set; }

    /// <summary>
    ///     Tracker position angular velocity (in rad/s, euler, left-handed)
    ///     Leave null to disable physics support and auto-compute
    /// </summary>
    public Vector3? AngularVelocity { get; set; }

    /// <summary>
    ///     Tracker position angular acc (in rad/s, euler, left-handed)
    ///     Leave null to disable physics support and auto-compute
    /// </summary>
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
    ///     Shown when tracking freeze is toggled manually,
    ///     explains what to press to toggle it from here
    ///     Leave null or empty to skip showing anything
    /// </summary>
    public string? TrackingFreezeActionString { get; }

    /// <summary>
    ///     Shown when skeleton flip is toggled manually,
    ///     explains what to press to toggle it from here
    ///     Leave null or empty to skip showing anything
    /// </summary>
    public string? SkeletonFlipActionString { get; }
}