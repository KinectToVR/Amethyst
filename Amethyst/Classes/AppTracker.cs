using System.ComponentModel;
using System.Numerics;
using Amethyst.Driver.API;
using Amethyst.Plugins.Contract;

namespace Amethyst.Classes;

public class AppTracker : INotifyPropertyChanged
{
    // Is this tracker enabled?
    private bool _isActive;
    private bool _overridePhysics;

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

    public AppTracker()
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

    public bool OverridePhysics
    {
        get => _overridePhysics;
        set
        {
            _overridePhysics = value;
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
            OrientationOffset.Y, OrientationOffset.X, OrientationOffset.Z);
    }

    // Get calibrated position, w/ offsets & filters
    public Vector3 GetFullCalibratedPosition(Quaternion calibrationRotation,
        Vector3 calibrationTranslation, Vector3? calibrationOrigin = null,
        JointPositionTrackingOption? filter = null)
    {
        // Construct the calibrated pose
        return Vector3.Transform(GetFilteredPosition(filter) -
                calibrationOrigin ?? Vector3.Zero, calibrationRotation) +
            calibrationTranslation + calibrationOrigin ?? Vector3.Zero +
            PositionOffset; // Unwrap, rotate, transform, wrap, offset
    }

    // Get calibrated orientation, w/ offsets & filters
    public Quaternion GetFullCalibratedOrientation(Quaternion calibrationRotation,
        RotationTrackingFilterOption? filter = null)
    {
        // Construct the calibrated orientation
        var rawOrientation = GetFilteredOrientation(filter);

        // Construct the calibrated orientation in eigen
        // Note: Apply calibration only in some cases
        if (OrientationTrackingOption != JointRotationTrackingOption.DisableJointRotation &&
            OrientationTrackingOption != JointRotationTrackingOption.FollowHmdRotation)
            rawOrientation = calibrationRotation * rawOrientation;

        // Return the calibrated orientation with offset
        return Quaternion.CreateFromYawPitchRoll(OrientationOffset.Y,
            OrientationOffset.X, OrientationOffset.Z) * rawOrientation;
    }

    // Get filtered data
    // By default, the saved filter is selected,
    // and to select it, the filter number must be < 0
    // Additionally, this adds the offsets
    // Offset will be added after translation
    public Vector3 GetCalibratedVector(Vector3 positionVector,
        Quaternion calibrationRotation, Vector3 calibrationTranslation,
        Vector3? calibrationOrigin = null)
    {
        // Construct the calibrated pose
        return Vector3.Transform(positionVector -
                calibrationOrigin ?? Vector3.Zero, calibrationRotation) +
            calibrationTranslation + calibrationOrigin ?? Vector3.Zero +
            PositionOffset; // Unwrap, rotate, transform, wrap, offset
    }

    // Get tracker base
    // This is for updating the server with
    // exclusive filtered data from K2AppTracker
    // By default, the saved filter is selected
    // Offsets are added inside called methods
    public K2TrackerBase GetTrackerBase(Quaternion calibrationRotation,
        Vector3 calibrationTranslation, Vector3 calibrationOrigin,
        JointPositionTrackingOption? posFilter = null,
        RotationTrackingFilterOption? oriFilter = null)
    {
        // Check if matrices are empty
        var notCalibrated = calibrationRotation.IsIdentity &&
                            calibrationTranslation.Equals(Vector3.Zero) &&
                            calibrationOrigin.Equals(Vector3.Zero);

        // Construct the return type
        var trackerBase = new K2TrackerBase()
        {
            Data = new K2TrackerData { IsActive = IsActive, Role = Role, Serial = Serial },
            Tracker = Role
        };

        var fullOrientation = notCalibrated
            ? GetFullOrientation(oriFilter)
            : GetFullCalibratedOrientation(
                calibrationRotation, oriFilter);

        var fullPosition = notCalibrated
            ? GetFullPosition(posFilter)
            : GetFullCalibratedPosition(calibrationRotation,
                calibrationTranslation, calibrationOrigin, posFilter);

        trackerBase.Pose.Orientation = new K2Quaternion
        {
            W = fullOrientation.W, X = fullOrientation.X, Y = fullOrientation.Y, Z = fullOrientation.Z
        };

        trackerBase.Pose.Position = new K2Vector3
        {
            X = fullPosition.X, Y = fullPosition.Y, Z = fullPosition.Z
        };

        if (!OverridePhysics) return trackerBase;

        var fullVelocity = notCalibrated
            ? PoseVelocity
            : GetCalibratedVector(
                PoseVelocity, calibrationRotation,
                calibrationTranslation, calibrationOrigin);

        var fullAcceleration = notCalibrated
            ? PoseAcceleration
            : GetCalibratedVector(
                PoseAcceleration, calibrationRotation,
                calibrationTranslation, calibrationOrigin);

        var fullAngularVelocity = notCalibrated
            ? PoseAngularVelocity
            : GetCalibratedVector(
                PoseAngularVelocity, calibrationRotation,
                calibrationTranslation, calibrationOrigin);

        var fullAngularAcceleration = notCalibrated
            ? PoseAngularAcceleration
            : GetCalibratedVector(
                PoseAngularAcceleration, calibrationRotation,
                calibrationTranslation, calibrationOrigin);

        trackerBase.Pose.Physics.Velocity = new K2Vector3
        {
            X = fullVelocity.X,
            Y = fullVelocity.Y,
            Z = fullVelocity.Z
        };

        trackerBase.Pose.Physics.Acceleration = new K2Vector3
        {
            X = fullAcceleration.X,
            Y = fullAcceleration.Y,
            Z = fullAcceleration.Z
        };

        trackerBase.Pose.Physics.AngularVelocity = new K2Vector3
        {
            X = fullAngularVelocity.X,
            Y = fullAngularVelocity.Y,
            Z = fullAngularVelocity.Z
        };

        trackerBase.Pose.Physics.AngularAcceleration = new K2Vector3
        {
            X = fullAngularAcceleration.X,
            Y = fullAngularAcceleration.Y,
            Z = fullAngularAcceleration.Z
        };

        return trackerBase;
    }

    // Get tracker base
    // This is for updating the server with
    // exclusive filtered data from K2AppTracker
    // By default, the saved filter is selected
    // Offsets are added inside called methods
    public K2TrackerBase GetTrackerBase(
        JointPositionTrackingOption? posFilter = null,
        RotationTrackingFilterOption? oriFilter = null)
    {
        // Construct the return type
        var trackerBase = new K2TrackerBase()
        {
            Data = new K2TrackerData { IsActive = IsActive, Role = Role, Serial = Serial },
            Tracker = Role
        };

        var fullOrientation = GetFullOrientation(oriFilter);
        var fullPosition = GetFullPosition(posFilter);

        trackerBase.Pose.Orientation = new K2Quaternion
        {
            W = fullOrientation.W,
            X = fullOrientation.X,
            Y = fullOrientation.Y,
            Z = fullOrientation.Z
        };

        trackerBase.Pose.Position = new K2Vector3
        {
            X = fullPosition.X,
            Y = fullPosition.Y,
            Z = fullPosition.Z
        };

        if (!OverridePhysics) return trackerBase;

        trackerBase.Pose.Physics.Velocity = new K2Vector3
        {
            X = PoseVelocity.X,
            Y = PoseVelocity.Y,
            Z = PoseVelocity.Z
        };

        trackerBase.Pose.Physics.Acceleration = new K2Vector3
        {
            X = PoseAcceleration.X,
            Y = PoseAcceleration.Y,
            Z = PoseAcceleration.Z
        };

        trackerBase.Pose.Physics.AngularVelocity = new K2Vector3
        {
            X = PoseAngularVelocity.X,
            Y = PoseAngularVelocity.Y,
            Z = PoseAngularVelocity.Z
        };

        trackerBase.Pose.Physics.AngularAcceleration = new K2Vector3
        {
            X = PoseAngularAcceleration.X,
            Y = PoseAngularAcceleration.Y,
            Z = PoseAngularAcceleration.Z
        };

        return trackerBase;
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

    // MVVM stuff
    public string GetResourceString(string key) => Interfacing.LocalizedJsonString(key);
}