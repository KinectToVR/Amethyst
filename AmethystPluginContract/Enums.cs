﻿namespace Amethyst.Plugins.Contract;

// Global Joint Types,
// see enumeration in external/Kinect
public enum TrackedJointType
{
    JointHead,
    JointNeck,
    JointSpineShoulder,
    JointShoulderLeft,
    JointElbowLeft,
    JointWristLeft,
    JointHandLeft,
    JointHandTipLeft,
    JointThumbLeft,
    JointShoulderRight,
    JointElbowRight,
    JointWristRight,
    JointHandRight,
    JointHandTipRight,
    JointThumbRight,
    JointSpineMiddle,
    JointSpineWaist,
    JointHipLeft,
    JointKneeLeft,
    JointAnkleLeft,
    JointFootLeft,
    JointHipRight,
    JointKneeRight,
    JointAnkleRight,
    JointFootRight,
    JointManual
}

// Global joint states
public enum TrackedJointState
{
    StateNotTracked,
    StateInferred,
    StateTracked
}

// Log severity enumeration
public enum LogSeverity
{
    Info,
    Warning,
    Error,
    Fatal
}

// Rotation tracking option enumeration
public enum JointRotationTrackingOption
{
    // Default - internal
    DeviceInferredRotation,

    // Calculated rotation - Optional and feet-only
    SoftwareCalculatedRotation,

    // Calculated rotation v2 - Optional and feet-only
    SoftwareCalculatedRotationV2,

    // Copy rotation from HMD
    FollowHmdRotation,

    // Completely disable rotation
    DisableJointRotation
}

// Tracking filter option enumeration - rotation
public enum RotationTrackingFilterOption
{
    // Spherical interpolation
    OrientationTrackingFilterSlerp,

    // Spherical interpolation, but slower
    OrientationTrackingFilterSlerpSlow,

    // Filter Off
    NoOrientationTrackingFilter
}

// Tracking filter option enumeration - position
public enum JointPositionTrackingOption
{
    // Interpolation
    PositionTrackingFilterLerp,

    // Low pass filter
    PositionTrackingFilterLowpass,

    // Extended Kalman
    PositionTrackingFilterKalman,

    // Hekky^ pose prediction
    PositionTrackingFilterPrediction,

    // Filter Off
    NoPositionTrackingFilter
}