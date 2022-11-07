using System.Collections.Generic;
using Amethyst.Plugins.Contract;

namespace Amethyst.Utils;

// AME Enumeration helpers
public static class EnumUtils
{
    public static SortedDictionary<TrackerType, string> TrackerTypeSerialDictionary = new()
    {
        { TrackerType.Tracker_Handed, "vive_tracker_handed" },
        { TrackerType.Tracker_LeftFoot, "vive_tracker_left_foot" },
        { TrackerType.Tracker_RightFoot, "vive_tracker_right_foot" },
        { TrackerType.Tracker_LeftShoulder, "vive_tracker_left_Shoulder" },
        { TrackerType.Tracker_RightShoulder, "vive_tracker_right_shoulder" },
        { TrackerType.Tracker_LeftElbow, "vive_tracker_left_elbow" },
        { TrackerType.Tracker_RightElbow, "vive_tracker_right_elbow" },
        { TrackerType.Tracker_LeftKnee, "vive_tracker_left_knee" },
        { TrackerType.Tracker_RightKnee, "vive_tracker_right_knee" },
        { TrackerType.Tracker_Waist, "vive_tracker_waist" },
        { TrackerType.Tracker_Chest, "vive_tracker_chest" },
        { TrackerType.Tracker_Camera, "vive_tracker_camera" },
        { TrackerType.Tracker_Keyboard, "vive_tracker_keyboard" }
    };

    public static SortedDictionary<TrackerType, string> TrackerTypeRoleSerialDictionary = new()
    {
        { TrackerType.Tracker_Handed, "AME-HANDED" },
        { TrackerType.Tracker_LeftFoot, "AME-LFOOT" },
        { TrackerType.Tracker_RightFoot, "AME-RFOOT" },
        { TrackerType.Tracker_LeftShoulder, "AME-LSHOULDER" },
        { TrackerType.Tracker_RightShoulder, "AME-RSHOULDER" },
        { TrackerType.Tracker_LeftElbow, "AME-LELBOW" },
        { TrackerType.Tracker_RightElbow, "AME-RELBOW" },
        { TrackerType.Tracker_LeftKnee, "AME-LKNEE" },
        { TrackerType.Tracker_RightKnee, "AME-RKNEE" },
        { TrackerType.Tracker_Waist, "AME-WAIST" },
        { TrackerType.Tracker_Chest, "AME-CHEST" },
        { TrackerType.Tracker_Camera, "AME-CAMERA" },
        { TrackerType.Tracker_Keyboard, "AME-KEYBOARD" }
    };

    public static SortedDictionary<TrackerType, TrackedJointType> TrackerTypeJointDictionary = new()
    {
        { TrackerType.Tracker_Handed, TrackedJointType.Joint_HandLeft },
        { TrackerType.Tracker_LeftFoot, TrackedJointType.Joint_AnkleLeft },
        { TrackerType.Tracker_RightFoot, TrackedJointType.Joint_AnkleRight },
        { TrackerType.Tracker_LeftShoulder, TrackedJointType.Joint_ShoulderLeft },
        { TrackerType.Tracker_RightShoulder, TrackedJointType.Joint_ShoulderRight },
        { TrackerType.Tracker_LeftElbow, TrackedJointType.Joint_ElbowLeft },
        { TrackerType.Tracker_RightElbow, TrackedJointType.Joint_ElbowRight },
        { TrackerType.Tracker_LeftKnee, TrackedJointType.Joint_KneeLeft },
        { TrackerType.Tracker_RightKnee, TrackedJointType.Joint_KneeRight },
        { TrackerType.Tracker_Waist, TrackedJointType.Joint_SpineWaist },
        { TrackerType.Tracker_Chest, TrackedJointType.Joint_SpineMiddle },
        { TrackerType.Tracker_Camera, TrackedJointType.Joint_Head },
        { TrackerType.Tracker_Keyboard, TrackedJointType.Joint_HandRight }
    };

    public static SortedDictionary<TrackedJointType, TrackerType> JointTrackerTypeDictionary = new()
    {
        { TrackedJointType.Joint_HandLeft, TrackerType.Tracker_Handed },
        { TrackedJointType.Joint_AnkleLeft, TrackerType.Tracker_LeftFoot },
        { TrackedJointType.Joint_AnkleRight, TrackerType.Tracker_RightFoot },
        { TrackedJointType.Joint_ShoulderLeft, TrackerType.Tracker_LeftShoulder },
        { TrackedJointType.Joint_ShoulderRight, TrackerType.Tracker_RightShoulder },
        { TrackedJointType.Joint_ElbowLeft, TrackerType.Tracker_LeftElbow },
        { TrackedJointType.Joint_ElbowRight, TrackerType.Tracker_RightElbow },
        { TrackedJointType.Joint_KneeLeft, TrackerType.Tracker_LeftKnee },
        { TrackedJointType.Joint_KneeRight, TrackerType.Tracker_RightKnee },
        { TrackedJointType.Joint_SpineWaist, TrackerType.Tracker_Waist },
        { TrackedJointType.Joint_SpineMiddle, TrackerType.Tracker_Chest },
        { TrackedJointType.Joint_Head, TrackerType.Tracker_Camera },
        { TrackedJointType.Joint_HandRight, TrackerType.Tracker_Keyboard }
    };
}