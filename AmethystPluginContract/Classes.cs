using System.Numerics;

namespace Amethyst.Plugins.Contract;

public class TrackedJoint
{
    public TrackedJoint(string jointName)
    {
        JointName = jointName;
    }

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

    public Quaternion JointOrientation { get; set; } = new();

    public Vector3 PreviousJointPosition { get; set; } = new();
    public Quaternion PreviousJointOrientation { get; set; } = new();

    public Vector3 JointVelocity { get; set; } = new();
    public Vector3 JointAcceleration { get; set; } = new();

    public Vector3 JointAngularVelocity { get; set; } = new();
    public Vector3 JointAngularAcceleration { get; set; } = new();

    public TrackedJointState TrackingState { get; set; } = TrackedJointState.StateTracked;

    public long PoseTimestamp { get; private set; }
    public long PreviousPoseTimestamp { get; private set; }
    public string JointName { get; }

    private Vector3 _jointPosition;
}