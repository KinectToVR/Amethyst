using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Numerics;
using Amethyst.MVVM;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using AmethystSupport;
using Microsoft.UI.Xaml.Media.Animation;
using Newtonsoft.Json;

namespace Amethyst.Classes;

public class AppTracker : INotifyPropertyChanged
{
    [JsonIgnore] private readonly OneEuroFilter<Vector3> _euroFilter = new(120.0f);

    [JsonIgnore] private readonly LowPassFilter _lowPassFilter = new(6.9f, .005f);

    [JsonIgnore] private readonly Vector3 _predictedPosition = new(0);

    // Internal filters' data
    private Vector3 _euroPosition = new(0);

    // Is this tracker enabled?
    private bool _isActive;
    private bool _isOrientationOverridden;

    private bool _isPositionOverridden;
    private bool _isTrackerExpanderOpen;
    private Vector3 _lastLerpPosition = new(0);
    private Quaternion _lastSlerpOrientation = new(0, 0, 0, 1);
    private Quaternion _lastSlerpSlowOrientation = new(0, 0, 0, 1);
    private Vector3 _lerpPosition = new(0);
    private Vector3 _lowPassPosition = new(0);

    // Does the managing device request no pos filtering?

    // Position filter update option
    private RotationTrackingFilterOption _orientationTrackingFilterOption =
        RotationTrackingFilterOption.OrientationTrackingFilterSlerpSlow;

    // Orientation tracking option
    private JointRotationTrackingOption _orientationTrackingOption =
        JointRotationTrackingOption.DeviceInferredRotation;

    // If the joint is overridden, overrides' ids (computed)
    private uint _overrideJointId;

    // Position filter option
    private JointPositionTrackingOption _positionTrackingFilterOption =
        JointPositionTrackingOption.PositionTrackingFilterLerp;

    // The assigned host joint if using manual joints
    private uint _selectedTrackedJointId;

    private Quaternion _slerpOrientation = new(0, 0, 0, 1);
    private Quaternion _slerpSlowOrientation = new(0, 0, 0, 1);

    // Input actions: < SERVICE : < SERVICE ACTION DATA : DEVICE ACTION DATA > >
    // For "disabled" actions InputActionSource is null, for "hidden" - nothing
    // Use InputActionsMap for faster and easier data access, along with bindings
    public SortedDictionary<string, JsonDictionary<InputActionEndpoint, InputActionSource>> InputActions = [];
    public Vector3 OrientationOffset = new(0, 0, 0);

    // Internal data offset
    public Vector3 PositionOffset = new(0, 0, 0);

    // OnPropertyChanged listener for containers
    [JsonIgnore] public EventHandler PropertyChangedEvent;

    [JsonIgnore] public Vector3 PositionOffsetRound => PositionOffset * 100;

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
    [JsonIgnore] public TrackedJointState TrackingState { get; set; } = TrackedJointState.StateNotTracked;

    // Is this joint overridden?
    public bool IsPositionOverridden
    {
        get => !string.IsNullOrEmpty(OverrideGuid) && AppPlugins.IsOverride(OverrideGuid) && _isPositionOverridden;
        set
        {
            _isPositionOverridden = value;
            Shared.Events.RequestJointSettingsRefreshEvent?.Invoke(this, EventArgs.Empty);
        }
    }

    public bool IsOrientationOverridden
    {
        get => !string.IsNullOrEmpty(OverrideGuid) && AppPlugins.IsOverride(OverrideGuid) && _isOrientationOverridden;
        set => _isOrientationOverridden = value;
    }

    [JsonIgnore] public bool NoPositionFilteringRequested { get; set; }

    // Override device's GUID
    public string OverrideGuid { get; set; } = string.Empty;

    public TrackerType Role { get; set; } = TrackerType.TrackerHanded;

    public uint SelectedTrackedJointId
    {
        get => _selectedTrackedJointId;
        set
        {
            // Guard: don't do anything on no changes
            if (_selectedTrackedJointId == value) return;

            _selectedTrackedJointId = value;
            if (!Shared.Devices.DevicesJointsValid) return;

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
    public string Serial { get; set; } = string.Empty;

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
        (AppPlugins.CurrentServiceEndpoint?.AdditionalSupportedTrackerTypes.Contains(Role) ?? false);

    // Returns all input actions available from the currently selected service,
    // paired with their selected sources that will trigger the action if updated
    // Note: use InputActionSource.IsValid to check whether the source exists
    [JsonIgnore]
    public Dictionary<InputActionEndpoint, InputActionSource> InputActionsMap =>
        InputActions.TryGetValue(AppData.Settings.ServiceEndpointGuid, out var map) ? map : [];

    // Returns all input actions available from the currently selected service,
    // paired with their current selection state: true for used, false for hidden
    // Note: "used" can also mean that the action is shown & disabled by the user
    [JsonIgnore]
    public Dictionary<InputActionEndpoint, bool> AvailableInputActions =>
        AppPlugins.CurrentServiceEndpoint?.SupportedInputActions?.TryGetValue(Role, out var actions) ?? false
            ? actions.ToDictionary(x => new InputActionEndpoint { Tracker = Role, Guid = x.Guid },
                x => InputActionsMap.Keys.Any(y => y.Tracker == Role && y.Guid == x.Guid && y.IsValid))
            : [];

    [JsonIgnore]
    public IEnumerable<InputActionEntry> InputActionEntries =>
        AvailableInputActions.Select(x => new InputActionEntry
        {
            Action = x.Key,
            IsEnabled = x.Value
        }).OrderBy(x => x.Name);

    [JsonIgnore]
    public IEnumerable<InputActionBindingEntry> InputActionBindingEntries =>
        InputActionsMap.Select(x => new InputActionBindingEntry
        {
            Action = x.Key,
            Source = x.Value
        }).OrderBy(x => x.ActionName);

    [JsonIgnore] public bool OverridePhysics { get; set; }

    [JsonIgnore] public string TrackerName => Interfacing.LocalizedJsonString($"/SharedStrings/Joints/Enum/{(int)Role}");

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
        AppPlugins.BaseTrackingDevice.IsAppOrientationSupported;

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
    public string ManagingDevicePlaceholder =>
        Interfacing.LocalizedJsonString("/SettingsPage/Filters/Managed")
            .Format(AppPlugins.GetDevice(ManagingDeviceGuid).Device?.Name ?? "INVALID");

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
                    OverrideGuid = string.Empty; // Reset
                    break;
            }

            lock (Interfacing.UpdateLock)
            {
                AppData.Settings.CheckSettings(blockEvents: true);
                AppData.Settings.SaveSettings(); // Save it!
            }

            if (previousValue != SelectedOverrideJointIdForSelectedDevice)
                OnPropertyChanged(); // Refresh all
        }
    }

    // Is force-updated by the base device
    [JsonIgnore]
    public bool IsAutoManaged => AppPlugins.BaseTrackingDevice.TrackedJoints.Any(x =>
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
        get => OverrideGuid == AppData.Settings.SelectedTrackingDeviceGuid && IsPositionOverridden && IsActive;
        set
        {
            // Don't parse any invalid changed
            if (!Shared.Devices.DevicesJointsValid) return;

            // Update the actual override value
            IsPositionOverridden = value;

            // Update the managing and the override
            if (value && OverrideGuid != AppData.Settings.SelectedTrackingDeviceGuid)
            {
                OverrideGuid = AppData.Settings.SelectedTrackingDeviceGuid;

                IsOrientationOverridden = false; // Reset if set before
                SelectedOverrideJointIdForSelectedDevice = 1; // Reset
            }

            // Reset everything if both overrides are set to false
            if (!IsOverridden) OverrideGuid = string.Empty;
            OnPropertyChanged(); // All
        }
    }

    // Is this joint overridden by the selected device? (ori)
    [JsonIgnore]
    public bool IsOrientationOverriddenBySelectedDevice
    {
        get => OverrideGuid == AppData.Settings.SelectedTrackingDeviceGuid && IsOrientationOverridden && IsActive;
        set
        {
            // Don't parse any invalid changed
            if (!Shared.Devices.DevicesJointsValid) return;

            // Update the actual override value
            IsOrientationOverridden = value;

            // Update the managing and the override
            if (value && OverrideGuid != AppData.Settings.SelectedTrackingDeviceGuid)
            {
                OverrideGuid = AppData.Settings.SelectedTrackingDeviceGuid;

                IsPositionOverridden = false; // Reset if set before
                SelectedOverrideJointIdForSelectedDevice = 1; // Reset
            }

            // Reset everything if both overrides are set to false
            if (!IsOverridden) OverrideGuid = string.Empty;
            OnPropertyChanged(); // All
        }
    }

    [JsonIgnore]
    public bool IsOverriddenByOtherDevice =>
        !string.IsNullOrEmpty(OverrideGuid) && AppPlugins.IsOverride(OverrideGuid) &&
        OverrideGuid != AppData.Settings.SelectedTrackingDeviceGuid;

    [JsonIgnore]
    public string OverriddenByOtherDeviceString =>
        Interfacing.LocalizedJsonString("/DevicesPage/ToolTips/Overrides/Overlapping").Format(
            $"{AppPlugins.GetDevice(ManagingDeviceGuid).Device?.Name ?? "INVALID"} (GUID: {ManagingDeviceGuid})");

    // MVVM: a connection of the transitions each tracker expander should animate
    [JsonIgnore] public TransitionCollection SettingsExpanderTransitions { get; set; } = new();

    [JsonIgnore]
    public List<string> BaseDeviceJointsList =>
        AppPlugins.BaseTrackingDevice.TrackedJoints.Select(x => x.Name).ToList();

    [JsonIgnore]
    public List<string> SelectedDeviceJointsList
    {
        get
        {
            if (string.IsNullOrEmpty(AppData.Settings.SelectedTrackingDeviceGuid))
                return new List<string>
                {
                    Interfacing.LocalizedJsonString(
                        "/DevicesPage/Placeholders/Overrides/NoOverride/PlaceholderText")
                };

            var jointsList = AppPlugins.GetDevice(AppData.Settings.SelectedTrackingDeviceGuid)
                .Device.TrackedJoints.Select(x => x.Name).ToList();

            jointsList.Insert(0, Interfacing.LocalizedJsonString(
                "/DevicesPage/Placeholders/Overrides/NoOverride/PlaceholderText"));
            return jointsList;
        }
    }

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
            JointPositionTrackingOption.PositionTrackingFilterKalman => _euroPosition,
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
        var computedFilter =
            NoPositionFilteringRequested // If filtering is force-disabled
                ? RotationTrackingFilterOption.NoOrientationTrackingFilter
                : filter ?? _orientationTrackingFilterOption;

        return computedFilter switch
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
        return GetFilteredOrientation(filter) *
               Quaternion.CreateFromYawPitchRoll(
                   OrientationOffset.Y * MathF.PI / 180f,
                   OrientationOffset.X * MathF.PI / 180f,
                   OrientationOffset.Z * MathF.PI / 180f);
    }

    // Get calibrated position, w/ offsets & filters
    public Vector3 GetFullCalibratedPosition(Quaternion calibrationRotation,
        Vector3 calibrationTranslation, Vector3? calibrationOrigin = null,
        JointPositionTrackingOption? filter = null)
    {
        // Construct the calibrated pose
        return Vector3.Transform(GetFilteredPosition(filter) - (
                   calibrationOrigin ?? Vector3.Zero), calibrationRotation) +
               calibrationTranslation + (calibrationOrigin ?? Vector3.Zero) +
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
        return Quaternion.CreateFromYawPitchRoll(
            OrientationOffset.Y * MathF.PI / 180f,
            OrientationOffset.X * MathF.PI / 180f,
            OrientationOffset.Z * MathF.PI / 180f) * rawOrientation;
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
            TrackingState = TrackingState,
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
        _euroPosition = _euroFilter.Filter(Position);

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
        Shared.Main.DispatcherQueue?.TryEnqueue(() =>
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
            PropertyChangedEvent?.Invoke(this, new PropertyChangedEventArgs(propName));
        });
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

[JsonArray]
public class JsonDictionary<T, TU> : Dictionary<T, TU>
{
}