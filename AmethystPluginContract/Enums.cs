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