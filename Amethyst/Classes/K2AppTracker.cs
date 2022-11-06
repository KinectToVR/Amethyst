using System.ComponentModel;
using System.Numerics;
using AmethystPluginContract;

namespace Amethyst.Classes;

public class K2AppTracker : INotifyPropertyChanged
{
    // Is this tracker enabled?
    private bool _isActive;

    // Internal filters' data
    private Vector3 _kalmanPosition = new(0);

    // LERP data's backup
    private Vector3 _lowPassPosition = new(0);
    private Vector3 _predictedPosition = new(0);
    private Vector3 _LERPPosition = new(0);
    private Vector3 _lastLERPPosition = new(0);

    private Quaternion _SLERPOrientation = new(0, 0, 0, 1);
    private Quaternion _SLERPSlowOrientation = new(0, 0, 0, 1);
    private Quaternion _lastSLERPOrientation = new(0, 0, 0, 1);
    private Quaternion _lastSLERPSlowOrientation = new(0, 0, 0, 1);

    public Vector3 PoseVelocity { get; set; } = new(0, 0, 0);
    public Vector3 PoseAcceleration { get; set; } = new(0, 0, 0);
    public Vector3 PoseAngularAcceleration { get; set; } = new(0, 0, 0);
    public Vector3 PoseAngularVelocity { get; set; } = new(0, 0, 0);

    // Tracker pose (inherited)
    public Vector3 Position { get; set; } = new(0, 0, 0);
    public Quaternion Orientation { get; set; } = new(1, 0, 0, 0);

    public Vector3 PreviousPosition { get; set; } = new(0, 0, 0);
    public Quaternion PreviousOrientation { get; set; } = new(1, 0, 0, 0);

    public long PoseTimestamp { get; set; } = 0;
    public long PreviousPoseTimestamp { get; set; } = 0;

    // Internal data offset
    public Vector3 PositionOffset = new(0, 0, 0);
    public Vector3 OrientationOffset = new(0, 0, 0);

    // Is this joint overridden?
    public bool IsPositionOverridden { get; set; } = false;
    public bool IsRotationOverridden { get; set; } = false;

    // Position filter update option
    private RotationTrackingFilterOption orientationTrackingFilterOption =
        RotationTrackingFilterOption.OrientationTrackingFilter_SLERP;

    // Orientation tracking option
    private JointRotationTrackingOption orientationTrackingOption =
        JointRotationTrackingOption.DeviceInferredRotation;

    // Position filter option
    private JointPositionTrackingOption positionTrackingFilterOption =
        JointPositionTrackingOption.PositionTrackingFilter_LERP;

    public K2AppTracker()
    {
        InitializeFilters();
    }

    // Does the managing device request no pos filtering?
    public bool NoPositionFilteringRequested { get; set; } = false;

    // Override device's GUID
    public string OverrideGUID { get; set; }

    // If the joint is overridden, overrides' ids (computed)
    public uint OverrideJointID { get; set; } = 0;

    public TrackerType Role { get; set; } = TrackerType.Tracker_Handed;

    // The assigned host joint if using manual joints
    public uint SelectedTrackedJointID { get; set; } = 0;

    // Tracker data (inherited)
    public string Serial { get; set; }

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

    public RotationTrackingFilterOption OrientationTrackingFilterOption
    {
        get => orientationTrackingFilterOption;
        set
        {
            orientationTrackingFilterOption = value;
            OnPropertyChanged();
        }
    }

    public bool IsActive
    {
        get => _isActive;
        set
        {
            _isActive = value;
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
            JointPositionTrackingOption.NoPositionTrackingFilter => Position,
            _ => Position
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
            RotationTrackingFilterOption.NoOrientationTrackingFilter => Orientation,
            _ => Orientation
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