using System.Diagnostics.CodeAnalysis;
using System.Numerics;

namespace Amethyst.Plugins.Contract;

public class TrackedJoint
{
    [SetsRequiredMembers]
    public TrackedJoint()
    {
        JointName = "INVALID";
        Role = TrackedJointType.JointManual;
    }

    public required string JointName { get; init; }
    public required TrackedJointType Role { get; init; }

    public Vector3 JointPosition
    {
        get => _jointPosition;
        set
        {
            _jointPosition = value;

            PreviousPoseTimestamp = PoseTimestamp;
            PoseTimestamp = DateTime.Now.Ticks;
        }
    }

    public Quaternion JointOrientation { get; set; } = Quaternion.Identity;

    public Vector3 PreviousJointPosition { get; set; } = Vector3.Zero;
    public Quaternion PreviousJointOrientation { get; set; } = Quaternion.Identity;

    public Vector3 JointVelocity { get; set; } = Vector3.Zero;
    public Vector3 JointAcceleration { get; set; } = Vector3.Zero;

    public Vector3 JointAngularVelocity { get; set; } = Vector3.Zero;
    public Vector3 JointAngularAcceleration { get; set; } = Vector3.Zero;

    public TrackedJointState TrackingState { get; set; } = TrackedJointState.StateTracked;

    public long PoseTimestamp { get; private set; }
    public long PreviousPoseTimestamp { get; private set; }

    private Vector3 _jointPosition = Vector3.Zero;
}