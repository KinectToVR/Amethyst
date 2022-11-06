namespace AmethystPluginContract;

// Global Joint Types,
// see enumeration in external/Kinect
public enum TrackedJointType
{
    Joint_Head,
    Joint_Neck,
    Joint_SpineShoulder,
    Joint_ShoulderLeft,
    Joint_ElbowLeft,
    Joint_WristLeft,
    Joint_HandLeft,
    Joint_HandTipLeft,
    Joint_ThumbLeft,
    Joint_ShoulderRight,
    Joint_ElbowRight,
    Joint_WristRight,
    Joint_HandRight,
    Joint_HandTipRight,
    Joint_ThumbRight,
    Joint_SpineMiddle,
    Joint_SpineWaist,
    Joint_HipLeft,
    Joint_KneeLeft,
    Joint_AnkleLeft,
    Joint_FootLeft,
    Joint_HipRight,
    Joint_KneeRight,
    Joint_AnkleRight,
    Joint_FootRight,
    Joint_Total,
    Joint_Manual
}

// Global joint states
public enum TrackedJointState
{
    State_NotTracked,
    State_Inferred,
    State_Tracked
}

// Log severity enumeration
public enum LogSeverity
{
    INFO,
    WARNING,
    ERROR,
    FATAL
}

// OpenVR tracker type
public enum TrackerType
{
    Tracker_Handed,
    Tracker_LeftFoot,
    Tracker_RightFoot,
    Tracker_LeftShoulder,
    Tracker_RightShoulder,
    Tracker_LeftElbow,
    Tracker_RightElbow,
    Tracker_LeftKnee,
    Tracker_RightKnee,
    Tracker_Waist,
    Tracker_Chest,
    Tracker_Camera,
    Tracker_Keyboard
}

// Rotation tracking option enumeration
public enum JointRotationTrackingOption
{
    // Default - internal
    DeviceInferredRotation,

    // Calculated rotation - Optional and feet-only
    SoftwareCalculatedRotation,

    // Calculated rotation v2 - Optional and feet-only
    SoftwareCalculatedRotation_V2,

    // Copy rotation from HMD
    FollowHMDRotation,

    // Completely disable rotation
    DisableJointRotation
}

// Tracking filter option enumeration - rotation
public enum RotationTrackingFilterOption
{
    // Spherical interpolation
    OrientationTrackingFilter_SLERP,

    // Spherical interpolation, but slower
    OrientationTrackingFilter_SLERP_Slow,

    // Filter Off
    NoOrientationTrackingFilter
}

// Tracking filter option enumeration - position
public enum JointPositionTrackingOption
{
    // Interpolation
    PositionTrackingFilter_LERP,

    // Low pass filter
    PositionTrackingFilter_Lowpass,

    // Extended Kalman
    PositionTrackingFilter_Kalman,

    // Hekky^ pose prediction
    PositionTrackingFilter_Prediction,

    // Filter Off
    NoPositionTrackingFilter
}