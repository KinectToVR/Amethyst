using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Numerics;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using AmethystSupport;
using Microsoft.UI.Xaml.Media.Animation;
using Newtonsoft.Json;

namespace Amethyst.Classes;

public class AppTracker : INotifyPropertyChanged
{
    [JsonIgnore] private readonly KalmanFilter _kalmanFilter = new();

    [JsonIgnore] private readonly LowPassFilter _lowPassFilter = new(6.9f, .005f);

    [JsonIgnore] private readonly Vector3 _predictedPosition = new(0);

    // Is this tracker enabled?
    private bool _isActive;
    private bool _isOrientationOverridden;

    private bool _isPositionOverridden;
    private bool _isTrackerExpanderOpen;

    // Internal filters' data
    private Vector3 _kalmanPosition = new(0);
    private Vector3 _lastLerpPosition = new(0);
    private Quaternion _lastSlerpOrientation = new(0, 0, 0, 1);
    private Quaternion _lastSlerpSlowOrientation = new(0, 0, 0, 1);
    private Vector3 _lerpPosition = new(0);
    private Vector3 _lowPassPosition = new(0);

    // Does the managing device request no pos filtering?
    private bool _noPositionFilteringRequested;

    // Position filter update option
    private RotationTrackingFilterOption _orientationTrackingFilterOption =
        RotationTrackingFilterOption.OrientationTrackingFilterSlerpSlow;

    // Orientation tracking option
    private JointRotationTrackingOption _orientationTrackingOption =
        JointRotationTrackingOption.DeviceInferredRotation;

    // If the joint is overridden, overrides' ids (computed)
    private uint _overrideJointId;
    private bool _overridePhysics;

    // Position filter option
    private JointPositionTrackingOption _positionTrackingFilterOption =
        JointPositionTrackingOption.PositionTrackingFilterLerp;

    // The assigned host joint if using manual joints
    private uint _selectedTrackedJointId;

    private Quaternion _slerpOrientation = new(0, 0, 0, 1);
    private Quaternion _slerpSlowOrientation = new(0, 0, 0, 1);
    public Vector3 OrientationOffset = new(0, 0, 0);

    // Internal data offset
    public Vector3 PositionOffset = new(0, 0, 0);

    // OnPropertyChanged listener for containers
    [JsonIgnore] public EventHandler PropertyChangedEvent;

    [JsonIgnore] public Vector3 PoseVelocity { get; set; } = new(0, 0, 0);
    [JsonIgnore] public Vector3 PoseAcceleration { get; set; } = new(0, 0, 0);
    [JsonIgnore] public Vector3 PoseAngularAcceleration { get; set; } = new(0, 0, 0);
    [JsonIgnore] public Vector3 PoseAngularVelocity { get; set; } = new(0, 0, 0);

    // Tracker pose (inherited)
    [JsonIgnore] public Vector3 Position { get; set; } = new(0, 0, 0);
    [JsonIgnore] public Quaternion Orientation { get; set; } = new(1, 0, 0, 0);

    [JsonIgnore] public Vector3 PreviousPosition { get; set; } = new(0, 0, 0);
    [JsonIgnore] public Quaternion PreviousOrientation { get; set; } = new(1, 0, 0, 0);

    [JsonIgnore] public long PoseTimestamp { get; set; } = 0;
    [JsonIgnore] public long PreviousPoseTimestamp { get; set; } = 0;

    // Is this joint overridden?
    public bool IsPositionOverridden
    {
        get => !string.IsNullOrEmpty(OverrideGuid) && _isPositionOverridden;
        set => _isPositionOverridden = value;
    }

    public bool IsOrientationOverridden
    {
        get => !string.IsNullOrEmpty(OverrideGuid) && _isOrientationOverridden;
        set => _isOrientationOverridden = value;
    }

    [JsonIgnore]
    public bool NoPositionFilteringRequested
    {
        get => _noPositionFilteringRequested;
        set
        {
            // Guard: don't do anything on actual no changes
            if (_noPositionFilteringRequested == value) return;

            _noPositionFilteringRequested = value;
            OnPropertyChanged();
        }
    }

    // Override device's GUID
    public string OverrideGuid { get; set; } = "";

    public TrackerType Role { get; set; } = TrackerType.TrackerHanded;

    public uint SelectedTrackedJointId
    {
        get => _selectedTrackedJointId;
        set
        {
            // Guard: don't do anything on no changes
            if (_selectedTrackedJointId == value) return;

            _selectedTrackedJointId = value;
            OnPropertyChanged(); // All
            AppData.Settings.SaveSettings();
        }
    }

    public uint OverrideJointId
    {
        get => _overrideJointId;
        set
        {
            // Guard: don't do anything on no changes
            if (_overrideJointId == value) return;

            _overrideJointId = value;
            OnPropertyChanged(); // All
            AppData.Settings.SaveSettings();
        }
    }

    // Tracker data (inherited)
    public string Serial { get; set; } = "";

    public JointRotationTrackingOption OrientationTrackingOption
    {
        get => _orientationTrackingOption;
        set
        {
            // Guard: don't do anything on actual no changes
            if (_orientationTrackingOption == value) return;

            _orientationTrackingOption = value;
            OnPropertyChanged();
            AppData.Settings.SaveSettings();
        }
    }

    public JointPositionTrackingOption PositionTrackingFilterOption
    {
        get => _positionTrackingFilterOption;
        set
        {
            // Guard: don't do anything on actual no changes
            if (_positionTrackingFilterOption == value) return;

            _positionTrackingFilterOption = value;
            OnPropertyChanged();
            AppData.Settings.SaveSettings();
        }
    }

    public RotationTrackingFilterOption OrientationTrackingFilterOption
    {
        get => _orientationTrackingFilterOption;
        set
        {
            // Guard: don't do anything on actual no changes
            if (_orientationTrackingFilterOption == value) return;

            _orientationTrackingFilterOption = value;
            OnPropertyChanged();
            AppData.Settings.SaveSettings();
        }
    }

    public bool IsActive
    {
        get => _isActive && IsSupported;
        set
        {
            // Don't do anything on no changes
            if (_isActive == value) return;

            _isActive = value;
            OnPropertyChanged();
            AppData.Settings.SaveSettings();
        }
    }

    public bool IsActiveEnabled
    {
        get => _isActive;
        set
        {
            // Don't do anything on no changes
            if (_isActive == value) return;

            _isActive = value;
            OnPropertyChanged();
            AppData.Settings.SaveSettings();
        }
    }

    [JsonIgnore]
    public bool IsSupported =>
        Role is TrackerType.TrackerWaist or TrackerType.TrackerLeftFoot or TrackerType.TrackerRightFoot ||
        (TrackingDevices.CurrentServiceEndpoint?.AdditionalSupportedTrackerTypes.Contains(Role) ?? false);

    [JsonIgnore]
    public bool OverridePhysics
    {
        get => _overridePhysics;
        set
        {
            // Don't do anything on no changes
            if (_overridePhysics == value) return;

            _overridePhysics = value;
            OnPropertyChanged();
        }
    }

    [JsonIgnore]
    public string TrackerName => Interfacing.LocalizedJsonString($"/SharedStrings/Joints/Enum/{(int)Role}");

    [JsonIgnore]
    public int PositionTrackingDisplayOption
    {
        get => NoPositionFilteringRequested ? -1 : (int)PositionTrackingFilterOption;
        set
        {
            PositionTrackingFilterOption = (JointPositionTrackingOption)value;
            OnPropertyChanged("PositionTrackingDisplayOption");
            AppData.Settings.SaveSettings(); // Save it!
        }
    }

    [JsonIgnore]
    public int OrientationTrackingDisplayOption
    {
        get => (int)OrientationTrackingOption;
        set
        {
            OrientationTrackingOption = (JointRotationTrackingOption)value;
            OnPropertyChanged("OrientationTrackingDisplayOption");
            AppData.Settings.SaveSettings(); // Save it!
        }
    }

    [JsonIgnore]
    public bool AppOrientationSupported =>
        Role is TrackerType.TrackerLeftFoot or TrackerType.TrackerRightFoot &&
        TrackingDevices.BaseTrackingDevice.IsAppOrientationSupported;

    [JsonIgnore]
    public string ManagingDeviceGuid =>
        IsOverridden ? OverrideGuid : AppData.Settings.TrackingDeviceGuid;

    [JsonIgnore]
    public string PositionManagingDeviceGuid =>
        IsPositionOverridden ? OverrideGuid : AppData.Settings.TrackingDeviceGuid;

    [JsonIgnore]
    public string OrientationManagingDeviceGuid =>
        IsPositionOverridden ? OverrideGuid : AppData.Settings.TrackingDeviceGuid;

    [JsonIgnore]
    public string ManagingDevicePlaceholder => string.Format(Interfacing.LocalizedJsonString(
        "/SettingsPage/Filters/Managed"), ManagingDeviceGuid);

    [JsonIgnore]
    public bool IsTrackerExpanderOpen
    {
        get => _isTrackerExpanderOpen && IsActive;
        set
        {
            _isTrackerExpanderOpen = value;
            OnPropertyChanged("IsTrackerExpanderOpen");
        }
    }

    // The assigned host joint if using manual joints
    [JsonIgnore]
    public int SelectedBaseTrackedJointId
    {
        get => IsActive ? (int)_selectedTrackedJointId : -1;
        set
        {
            // Don't parse any invalid changed
            if (!Shared.Devices.DevicesJointsValid) return;
            _selectedTrackedJointId = value >= 0 ? (uint)value : 0;

            AppData.Settings.CheckSettings(); // Full
            AppData.Settings.SaveSettings(); // Save it!
            OnPropertyChanged(); // All
        }
    }

    [JsonIgnore]
    public int SelectedOverrideTrackedJointId
    {
        get => IsActive ? (int)_overrideJointId : -1;
        set
        {
            _overrideJointId = value >= 0 ? (uint)value : 0;

            AppData.Settings.CheckSettings(); // Full
            AppData.Settings.SaveSettings(); // Save it!
            OnPropertyChanged(); // All
        }
    }

    [JsonIgnore]
    public int SelectedOverrideJointId
    {
        // '+ 1' and '- 1' cause '0' is 'No Override' in this case
        // Note: use OverrideJointId for the "normal" (non-ui) one
        get => IsActive ? (int)_overrideJointId + 1 : -1;
        set
        {
            _overrideJointId = value > 0 ? (uint)(value - 1) : 0;

            AppData.Settings.CheckSettings(); // Full
            AppData.Settings.SaveSettings(); // Save it!
            OnPropertyChanged(); // All
        }
    }

    [JsonIgnore]
    public int SelectedOverrideJointIdForSelectedDevice
    {
        // '+ 1' and '- 1' cause '0' is 'No Override' in this case
        // Note: use OverrideJointId for the "normal" (non-ui) one
        // Note: -1 replaced with 0 cuz disabled joints are hidden
        get => IsActive
            ? IsManagedBy(AppData.Settings.SelectedTrackingDeviceGuid)
                ? (int)_overrideJointId + 1
                : 0 // Not overridden by this device
            : 0; // -1; // Disabled or not supported
        set
        {
            // Don't parse any invalid changed
            if (!Shared.Devices.DevicesJointsValid) return;

            // Update the override joint and the managing device
            var previousValue = SelectedOverrideJointIdForSelectedDevice;
            _overrideJointId = value > 0 ? (uint)(value - 1) : 0;
            OverrideGuid = AppData.Settings.SelectedTrackingDeviceGuid;

            switch (value)
            {
                // Enable overrides if just selected
                case > 0 when !IsOverridden:
                    IsPositionOverridden = true;
                    IsOrientationOverridden = true;
                    break;

                // Disable both overrides if deselected
                case <= 0:
                    IsPositionOverridden = false;
                    IsOrientationOverridden = false;
                    OverrideGuid = ""; // Reset
                    break;
            }

            lock (Interfacing.UpdateLock)
            {
                AppData.Settings.CheckSettings(); // Full
                AppData.Settings.SaveSettings(); // Save it!
            }

            if (previousValue != SelectedOverrideJointIdForSelectedDevice)
                OnPropertyChanged(); // Refresh all
        }
    }

    // Is force-updated by the base device
    [JsonIgnore]
    public bool IsAutoManaged => TrackingDevices.BaseTrackingDevice.TrackedJoints.Any(x =>
        x.Role != TrackedJointType.JointManual && x.Role == TypeUtils.TrackerTypeJointDictionary[Role]);

    // Is NOT force-updated by the base device
    [JsonIgnore] public bool IsManuallyManaged => !IsAutoManaged;

    // IsPositionOverridden || IsOrientationOverridden
    [JsonIgnore] public bool IsOverridden => IsPositionOverridden || IsOrientationOverridden;

    // IsPositionOverridden, IsOrientationOverridden
    [JsonIgnore]
    public (bool Position, bool Orientation) IsOverriddenPair
        => (IsPositionOverridden, IsOrientationOverridden);

    // Is this joint overridden by the selected device? (pos)
    [JsonIgnore]
    public bool IsPositionOverriddenBySelectedDevice
    {
        get => OverrideGuid == AppData.Settings.SelectedTrackingDeviceGuid && IsPositionOverridden;
        set
        {
            // Don't parse any invalid changed
            if (!Shared.Devices.DevicesJointsValid) return;

            // Update the managing and the override
            OverrideGuid = AppData.Settings.SelectedTrackingDeviceGuid;
            IsPositionOverridden = value;
            OnPropertyChanged(); // All
        }
    }

    // Is this joint overridden by the selected device? (ori)
    [JsonIgnore]
    public bool IsOrientationOverriddenBySelectedDevice
    {
        get => OverrideGuid == AppData.Settings.SelectedTrackingDeviceGuid && IsOrientationOverridden;
        set
        {
            // Don't parse any invalid changed
            if (!Shared.Devices.DevicesJointsValid) return;

            // Update the managing and the override
            OverrideGuid = AppData.Settings.SelectedTrackingDeviceGuid;
            IsOrientationOverridden = value;
            OnPropertyChanged(); // All
        }
    }

    [JsonIgnore]
    public bool IsOverriddenByOtherDevice =>
        !string.IsNullOrEmpty(OverrideGuid) && OverrideGuid != AppData.Settings.SelectedTrackingDeviceGuid;

    [JsonIgnore]
    public string OverriddenByOtherDeviceString => string.Format(Interfacing.LocalizedJsonString(
        "/DevicesPage/ToolTips/Overrides/Overlapping"), ManagingDeviceGuid);

    // MVVM: a connection of the transitions each tracker expander should animate
    [JsonIgnore] public TransitionCollection SettingsExpanderTransitions { get; set; } = new();

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
    public Vector3 GetCalibratedVector(Vector3 positionVector)
    {
        // Construct the calibrated pose
        return Vector3.Transform(
                   // Input, position
                   positionVector - AppData.Settings.DeviceCalibrationOrigins
                       .GetValueOrDefault(PositionManagingDeviceGuid, Vector3.Zero),
                   // Rotation
                   AppData.Settings.DeviceCalibrationRotationMatrices
                       .GetValueOrDefault(OrientationManagingDeviceGuid, Quaternion.Identity)) +
               // Translation
               AppData.Settings.DeviceCalibrationTranslationVectors
                   .GetValueOrDefault(PositionManagingDeviceGuid, Vector3.Zero) +
               // Origin
               AppData.Settings.DeviceCalibrationOrigins
                   .GetValueOrDefault(PositionManagingDeviceGuid, Vector3.Zero) + PositionOffset;
    }

    // Get tracker base
    // This is for updating the server with
    // exclusive filtered data from AppTracker
    // By default, the saved filter is selected
    // Offsets are added inside called methods
    public TrackerBase GetTrackerBase(
        JointPositionTrackingOption? posFilter = null,
        RotationTrackingFilterOption? oriFilter = null)
    {
        // Get the calibrated ori, checking if matrices are empty
        var fullOrientation = GetFullCalibratedOrientation(
            AppData.Settings.DeviceCalibrationRotationMatrices
                .GetValueOrDefault(OrientationManagingDeviceGuid, Quaternion.Identity), oriFilter);

        // Get the calibrated pos, checking if matrices are empty
        var fullPosition = GetFullCalibratedPosition(
            AppData.Settings.DeviceCalibrationRotationMatrices
                .GetValueOrDefault(OrientationManagingDeviceGuid, Quaternion.Identity),
            AppData.Settings.DeviceCalibrationTranslationVectors
                .GetValueOrDefault(PositionManagingDeviceGuid, Vector3.Zero),
            AppData.Settings.DeviceCalibrationOrigins
                .GetValueOrDefault(PositionManagingDeviceGuid, Vector3.Zero), posFilter);

        // Construct the return type
        var trackerBase = new TrackerBase
        {
            ConnectionState = IsActive,
            Role = Role,
            Serial = Serial,

            Position = new Vector3
            {
                X = fullPosition.X,
                Y = fullPosition.Y,
                Z = fullPosition.Z
            },
            Orientation = new Quaternion
            {
                W = fullOrientation.W,
                X = fullOrientation.X,
                Y = fullOrientation.Y,
                Z = fullOrientation.Z
            }
        };

        if (!OverridePhysics) return trackerBase;

        var fullVelocity = GetCalibratedVector(PoseVelocity);
        var fullAcceleration = GetCalibratedVector(PoseAcceleration);
        var fullAngularVelocity = GetCalibratedVector(PoseAngularVelocity);
        var fullAngularAcceleration = GetCalibratedVector(PoseAngularAcceleration);

        trackerBase.Velocity = new Vector3
        {
            X = fullVelocity.X,
            Y = fullVelocity.Y,
            Z = fullVelocity.Z
        };
        trackerBase.Acceleration = new Vector3
        {
            X = fullAcceleration.X,
            Y = fullAcceleration.Y,
            Z = fullAcceleration.Z
        };
        trackerBase.AngularVelocity = new Vector3
        {
            X = fullAngularVelocity.X,
            Y = fullAngularVelocity.Y,
            Z = fullAngularVelocity.Z
        };
        trackerBase.AngularAcceleration = new Vector3
        {
            X = fullAngularAcceleration.X,
            Y = fullAngularAcceleration.Y,
            Z = fullAngularAcceleration.Z
        };

        return trackerBase;
    }

    public void UpdateFilters()
    {
        // Update LowPass and Kalman filters
        _lowPassPosition = _lowPassFilter.Update(Position.Projected()).V();
        _kalmanPosition = _kalmanFilter.Update(Position.Projected()).V();

        // Update the LERP (mix) filter
        _lerpPosition = Vector3.Lerp(_lastLerpPosition, Position, 0.31f);
        _lastLerpPosition = _lerpPosition; // Backup the position

        // Update the standard SLERP filter
        _slerpOrientation = Quaternion.Slerp(
            Quaternion.Normalize(_lastSlerpOrientation),
            Quaternion.Normalize(Orientation), 0.25f);
        _lastSlerpOrientation = _slerpOrientation; // Backup

        // Update the 'slower' SLERP filter
        _slerpSlowOrientation = Quaternion.Slerp(
            Quaternion.Normalize(_lastSlerpSlowOrientation),
            Quaternion.Normalize(Orientation), 0.15f);
        _lastSlerpSlowOrientation = _slerpSlowOrientation; // Backup
    }

    public TrackedJoint GetTrackedJoint()
    {
        return new TrackedJoint
        {
            Name = Serial,
            Role = TypeUtils.TrackerTypeJointDictionary[Role],
            Acceleration = PoseAcceleration,
            AngularAcceleration = PoseAngularAcceleration,
            AngularVelocity = PoseAngularVelocity,
            Orientation = Orientation,
            Position = Position,
            Velocity = PoseVelocity,
            TrackingState = IsActive
                ? TrackedJointState.StateTracked
                : TrackedJointState.StateNotTracked
        };
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
        PropertyChangedEvent?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    // MVVM stuff
    public double BoolToOpacity(bool v)
    {
        return v ? 1.0 : 0.0;
    }

    public bool IsManagedBy(string guid)
    {
        return guid == ManagingDeviceGuid;
    }
}