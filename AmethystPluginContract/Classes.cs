using MathNet.Spatial.Euclidean;

namespace AmethystPluginContract;

public class TrackedJoint
{
    public TrackedJoint(string jointName)
    {
        JointName = jointName;
    }

    public Vector3D JointPosition
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

    public Vector3D PreviousJointPosition { get; set; } = new();
    public Quaternion PreviousJointOrientation { get; set; } = new();

    public Vector3D JointVelocity { get; set; } = new();
    public Vector3D JointAcceleration { get; set; } = new();

    public Vector3D JointAngularVelocity { get; set; } = new();
    public Vector3D JointAngularAcceleration { get; set; } = new();

    public TrackedJointState TrackingState { get; set; } = TrackedJointState.State_Tracked;

    public long PoseTimestamp { get; private set; }
    public long PreviousPoseTimestamp { get; private set; }
    public string JointName { get; }

    private Vector3D _jointPosition;
}