using System.ComponentModel;
using System.Numerics;
using Amethyst.Driver.API;
using Amethyst.Plugins.Contract;

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
    private Vector3 _lerpPosition = new(0);
    private Vector3 _lastLerpPosition = new(0);

    private Quaternion _slerpOrientation = new(0, 0, 0, 1);
    private Quaternion _slerpSlowOrientation = new(0, 0, 0, 1);
    private Quaternion _lastSlerpOrientation = new(0, 0, 0, 1);
    private Quaternion _lastSlerpSlowOrientation = new(0, 0, 0, 1);

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
    private RotationTrackingFilterOption _orientationTrackingFilterOption =
        RotationTrackingFilterOption.OrientationTrackingFilterSlerp;

    // Orientation tracking option
    private JointRotationTrackingOption _orientationTrackingOption =
        JointRotationTrackingOption.DeviceInferredRotation;

    // Position filter option
    private JointPositionTrackingOption _positionTrackingFilterOption =
        JointPositionTrackingOption.PositionTrackingFilterLerp;

    public K2AppTracker()
    {
        InitializeFilters();
    }

    // Does the managing device request no pos filtering?
    public bool NoPositionFilteringRequested { get; set; } = false;

    // Override device's GUID
    public string OverrideGuid { get; set; }

    // If the joint is overridden, overrides' ids (computed)
    public uint OverrideJointId { get; set; } = 0;

    public TrackerType Role { get; set; } = TrackerType.TrackerHanded;

    // The assigned host joint if using manual joints
    public uint SelectedTrackedJointId { get; set; } = 0;

    // Tracker data (inherited)
    public string Serial { get; set; }

    public JointRotationTrackingOption OrientationTrackingOption
    {
        get => _orientationTrackingOption;
        set
        {
            _orientationTrackingOption = value;
            OnPropertyChanged();
        }
    }

    public JointPositionTrackingOption PositionTrackingFilterOption
    {
        get => _positionTrackingFilterOption;
        set
        {
            _positionTrackingFilterOption = value;
            OnPropertyChanged();
        }
    }

    public RotationTrackingFilterOption OrientationTrackingFilterOption
    {
        get => _orientationTrackingFilterOption;
        set
        {
            _orientationTrackingFilterOption = value;
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
        var computedFilter = 
            NoPositionFilteringRequested // If filtering is force-disabled
            ? JointPositionTrackingOption.NoPositionTrackingFilter
            : filter ?? _positionTrackingFilterOption;

        return computedFilter switch
        {
            JointPositionTrackingOption.PositionTrackingFilterLerp => _lerpPosition,
            JointPositionTrackingOption.PositionTrackingFilterLowpass => _lowPassPosition,
            JointPositionTrackingOption.PositionTrackingFilterKalman => _kalmanPosition,
            JointPositionTrackingOption.PositionTrackingFilterPrediction => _predictedPosition,
            JointPositionTrackingOption.NoPositionTrackingFilter => Position,
            _ => Position
        };
    }

    // Get filtered data
    // By default, the saved filter is selected,
    // and to select it, the filter number must be < 0
    public Quaternion GetFilteredOrientation(RotationTrackingFilterOption? filter = null)
    {
        return (filter ?? _orientationTrackingFilterOption) switch
        {
            RotationTrackingFilterOption.OrientationTrackingFilterSlerp => _slerpOrientation,
            RotationTrackingFilterOption.OrientationTrackingFilterSlerpSlow => _slerpSlowOrientation,
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

    public TrackedJoint GetTrackedJoint()
    {
        return new TrackedJoint(Serial)
        {
            JointAcceleration = PoseAcceleration,
            JointAngularAcceleration = PoseAngularAcceleration,
            JointAngularVelocity = PoseAngularVelocity,
            JointOrientation = Orientation,
            JointPosition = Position,
            JointVelocity = PoseVelocity,
            PreviousJointOrientation = PreviousOrientation,
            PreviousJointPosition = PreviousPosition,
            TrackingState = IsActive 
                ? TrackedJointState.StateTracked 
                : TrackedJointState.StateNotTracked
        };
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(null));
    }
}