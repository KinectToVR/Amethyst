using System.ComponentModel;
using System.Numerics;
using AmethystPluginContract;

namespace Amethyst.Classes;

public class K2AppTracker : INotifyPropertyChanged
{
    // Internal filters' data
    private readonly Vector3 _kalmanPosition = new(0);
    private readonly Vector3 _LERPPosition = new(0);
    private readonly Vector3 _lowPassPosition = new(0);
    private readonly Quaternion _pose_orientation = new(1, 0, 0, 0);

    // Tracker pose (inherited)
    private readonly Vector3 _pose_position = new(0, 0, 0);
    private readonly Vector3 _predictedPosition = new(0);

    private readonly Quaternion _SLERPOrientation = new(0, 0, 0, 1);
    private readonly Quaternion _SLERPSlowOrientation = new(0, 0, 0, 1);

    // Position filter update option
    private readonly RotationTrackingFilterOption orientationTrackingFilterOption =
        RotationTrackingFilterOption.OrientationTrackingFilter_SLERP;

    // LERP data's backup
    private Vector3 _lastLERPPosition = new(0);
    private Quaternion _lastSLERPOrientation = new(0, 0, 0, 1);
    private Quaternion _lastSLERPSlowOrientation = new(0, 0, 0, 1);
    private Vector3 _pose_acceleration = new(0, 0, 0);
    private Vector3 _pose_angularAcceleration = new(0, 0, 0);
    private Vector3 _pose_angularVelocity = new(0, 0, 0);

    private long _pose_poseTimestamp = 0;
    private Quaternion _pose_previousOrientation = new(1, 0, 0, 0);
    private long _pose_previousPoseTimestamp = 0;

    private Vector3 _pose_previousPosition = new(0, 0, 0);
    private Vector3 _pose_velocity = new(0, 0, 0);

    // Is this tracker enabled?
    public bool IsActive = false;

    // Is this joint overridden?
    public bool IsPositionOverridden = false,
        IsRotationOverridden = false;

    // Does the managing device request no pos filtering?
    public bool NoPositionFilteringRequested = false;
    public Vector3 OrientationOffset = new(0, 0, 0);

    private JointRotationTrackingOption orientationTrackingOption =
        JointRotationTrackingOption.DeviceInferredRotation;

    // Override device's GUID
    public string OverrideGUID;

    // If the joint is overridden, overrides' ids (computed)
    public uint OverrideJointID = 0;

    // Internal data offset
    public Vector3 PositionOffset = new(0, 0, 0);

    // Position and orientation option
    private JointPositionTrackingOption positionTrackingFilterOption =
        JointPositionTrackingOption.PositionTrackingFilter_LERP;

    public TrackerType Role = TrackerType.Tracker_Handed;

    // The assigned host joint if using manual joints
    public uint SelectedTrackedJointID = 0;

    // Tracker data (inherited)
    public string Serial;

    public K2AppTracker()
    {
        InitializeFilters();
    }

    public JointRotationTrackingOption OrientationTrackingOption
    {
        get => orientationTrackingOption;
        set
        {
            orientationTrackingOption = value;
            OnPropertyChanged();
        }
    }

    public JointPositionTrackingOption PositionTrackingFilterOption
    {
        get => positionTrackingFilterOption;
        set
        {
            positionTrackingFilterOption = value;
            OnPropertyChanged();
        }
    }

    // Internal position filters
    //private LowPassFilter[] _lowPassFilter = new LowPassFilter[3];
    //private KalmanFilter _kalmanFilter = new KalmanFilter();

    public event PropertyChangedEventHandler PropertyChanged;

    // Get filtered data
    // By default, the saved filter is selected,
    // and to select it, the filter number must be < 0
    public Vector3 GetFilteredPosition(JointPositionTrackingOption? filter = null)
    {
        var _filter = filter ?? JointPositionTrackingOption.NoPositionTrackingFilter;

        if (!filter.HasValue) _filter = positionTrackingFilterOption;
        if (NoPositionFilteringRequested)
            _filter = JointPositionTrackingOption.NoPositionTrackingFilter;

        return _filter switch
        {
            JointPositionTrackingOption.PositionTrackingFilter_LERP => _LERPPosition,
            JointPositionTrackingOption.PositionTrackingFilter_Lowpass => _lowPassPosition,
            JointPositionTrackingOption.PositionTrackingFilter_Kalman => _kalmanPosition,
            JointPositionTrackingOption.PositionTrackingFilter_Prediction => _predictedPosition,
            JointPositionTrackingOption.NoPositionTrackingFilter => _pose_position,
            _ => _pose_position
        };
    }

    // Get filtered data
    // By default, the saved filter is selected,
    // and to select it, the filter number must be < 0
    public Quaternion GetFilteredOrientation(RotationTrackingFilterOption? filter = null)
    {
        return (filter ?? orientationTrackingFilterOption) switch
        {
            RotationTrackingFilterOption.OrientationTrackingFilter_SLERP => _SLERPOrientation,
            RotationTrackingFilterOption.OrientationTrackingFilter_SLERP_Slow => _SLERPSlowOrientation,
            RotationTrackingFilterOption.NoOrientationTrackingFilter => _pose_orientation,
            _ => _pose_orientation
        };
    }

    // Get filtered data
    // By default, the saved filter is selected,
    // and to select it, the filter number must be < 0
    // Additionally, this adds the offsets
    public Vector3 GetFullPosition(JointPositionTrackingOption? filter = null)
    {
        return GetFilteredPosition(filter) + PositionOffset;
    }

    // Get filtered data
    // By default, the saved filter is selected,
    // and to select it, the filter number must be < 0
    // Additionally, this adds the offsets
    public Quaternion GetFullOrientation(RotationTrackingFilterOption? filter = null)
    {
        return GetFilteredOrientation(filter) * Quaternion.CreateFromYawPitchRoll(
            OrientationOffset.X, OrientationOffset.Y, OrientationOffset.Z);
    }

    public void InitializeFilters()
    {
        // Low pass filter initialization

        // Kalman filter initialization
    }

    public void UpdateFilters()
    {
        // Low pass filter initialization

        // Kalman filter initialization
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(null));
    }
}