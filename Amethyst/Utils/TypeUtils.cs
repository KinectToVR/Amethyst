using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using Amethyst.Driver.API;
using Amethyst.Plugins.Contract;
using Valve.VR;

namespace Amethyst.Utils;

// AME Enumeration helpers
public static class TypeUtils
{
    public static SortedDictionary<TrackerType, string> TrackerTypeSerialDictionary = new()
    {
        { TrackerType.TrackerHanded, "vive_tracker_handed" },
        { TrackerType.TrackerLeftFoot, "vive_tracker_left_foot" },
        { TrackerType.TrackerRightFoot, "vive_tracker_right_foot" },
        { TrackerType.TrackerLeftShoulder, "vive_tracker_left_Shoulder" },
        { TrackerType.TrackerRightShoulder, "vive_tracker_right_shoulder" },
        { TrackerType.TrackerLeftElbow, "vive_tracker_left_elbow" },
        { TrackerType.TrackerRightElbow, "vive_tracker_right_elbow" },
        { TrackerType.TrackerLeftKnee, "vive_tracker_left_knee" },
        { TrackerType.TrackerRightKnee, "vive_tracker_right_knee" },
        { TrackerType.TrackerWaist, "vive_tracker_waist" },
        { TrackerType.TrackerChest, "vive_tracker_chest" },
        { TrackerType.TrackerCamera, "vive_tracker_camera" },
        { TrackerType.TrackerKeyboard, "vive_tracker_keyboard" }
    };

    public static SortedDictionary<TrackerType, string> TrackerTypeRoleSerialDictionary = new()
    {
        { TrackerType.TrackerHanded, "AME-HANDED" },
        { TrackerType.TrackerLeftFoot, "AME-LFOOT" },
        { TrackerType.TrackerRightFoot, "AME-RFOOT" },
        { TrackerType.TrackerLeftShoulder, "AME-LSHOULDER" },
        { TrackerType.TrackerRightShoulder, "AME-RSHOULDER" },
        { TrackerType.TrackerLeftElbow, "AME-LELBOW" },
        { TrackerType.TrackerRightElbow, "AME-RELBOW" },
        { TrackerType.TrackerLeftKnee, "AME-LKNEE" },
        { TrackerType.TrackerRightKnee, "AME-RKNEE" },
        { TrackerType.TrackerWaist, "AME-WAIST" },
        { TrackerType.TrackerChest, "AME-CHEST" },
        { TrackerType.TrackerCamera, "AME-CAMERA" },
        { TrackerType.TrackerKeyboard, "AME-KEYBOARD" }
    };

    public static SortedDictionary<TrackerType, TrackedJointType> TrackerTypeJointDictionary = new()
    {
        { TrackerType.TrackerHanded, TrackedJointType.JointHandLeft },
        { TrackerType.TrackerLeftFoot, TrackedJointType.JointAnkleLeft },
        { TrackerType.TrackerRightFoot, TrackedJointType.JointAnkleRight },
        { TrackerType.TrackerLeftShoulder, TrackedJointType.JointShoulderLeft },
        { TrackerType.TrackerRightShoulder, TrackedJointType.JointShoulderRight },
        { TrackerType.TrackerLeftElbow, TrackedJointType.JointElbowLeft },
        { TrackerType.TrackerRightElbow, TrackedJointType.JointElbowRight },
        { TrackerType.TrackerLeftKnee, TrackedJointType.JointKneeLeft },
        { TrackerType.TrackerRightKnee, TrackedJointType.JointKneeRight },
        { TrackerType.TrackerWaist, TrackedJointType.JointSpineWaist },
        { TrackerType.TrackerChest, TrackedJointType.JointSpineMiddle },
        { TrackerType.TrackerCamera, TrackedJointType.JointHead },
        { TrackerType.TrackerKeyboard, TrackedJointType.JointHandRight }
    };

    public static SortedDictionary<TrackedJointType, TrackerType> JointTrackerTypeDictionary = new()
    {
        { TrackedJointType.JointHandLeft, TrackerType.TrackerHanded },
        { TrackedJointType.JointAnkleLeft, TrackerType.TrackerLeftFoot },
        { TrackedJointType.JointAnkleRight, TrackerType.TrackerRightFoot },
        { TrackedJointType.JointFootLeft, TrackerType.TrackerLeftFoot },
        { TrackedJointType.JointFootRight, TrackerType.TrackerRightFoot },
        { TrackedJointType.JointShoulderLeft, TrackerType.TrackerLeftShoulder },
        { TrackedJointType.JointShoulderRight, TrackerType.TrackerRightShoulder },
        { TrackedJointType.JointElbowLeft, TrackerType.TrackerLeftElbow },
        { TrackedJointType.JointElbowRight, TrackerType.TrackerRightElbow },
        { TrackedJointType.JointKneeLeft, TrackerType.TrackerLeftKnee },
        { TrackedJointType.JointKneeRight, TrackerType.TrackerRightKnee },
        { TrackedJointType.JointSpineWaist, TrackerType.TrackerWaist },
        { TrackedJointType.JointSpineMiddle, TrackerType.TrackerChest },
        { TrackedJointType.JointHead, TrackerType.TrackerCamera },
        { TrackedJointType.JointHandRight, TrackerType.TrackerKeyboard }
    };

    public static SortedDictionary<TrackedJointType, TrackedJointType> FlippedJointTypeDictionary = new()
    {
        { TrackedJointType.JointHead, TrackedJointType.JointHead },
        { TrackedJointType.JointNeck, TrackedJointType.JointNeck },
        { TrackedJointType.JointSpineShoulder, TrackedJointType.JointSpineShoulder },
        { TrackedJointType.JointShoulderLeft, TrackedJointType.JointShoulderRight },
        { TrackedJointType.JointElbowLeft, TrackedJointType.JointElbowRight },
        { TrackedJointType.JointWristLeft, TrackedJointType.JointWristRight },
        { TrackedJointType.JointHandLeft, TrackedJointType.JointHandRight },
        { TrackedJointType.JointHandTipLeft, TrackedJointType.JointHandTipRight },
        { TrackedJointType.JointThumbLeft, TrackedJointType.JointThumbRight },
        { TrackedJointType.JointShoulderRight, TrackedJointType.JointShoulderLeft },
        { TrackedJointType.JointElbowRight, TrackedJointType.JointElbowLeft },
        { TrackedJointType.JointWristRight, TrackedJointType.JointWristLeft },
        { TrackedJointType.JointHandRight, TrackedJointType.JointHandLeft },
        { TrackedJointType.JointHandTipRight, TrackedJointType.JointHandTipLeft },
        { TrackedJointType.JointThumbRight, TrackedJointType.JointThumbLeft },
        { TrackedJointType.JointSpineMiddle, TrackedJointType.JointSpineMiddle },
        { TrackedJointType.JointSpineWaist, TrackedJointType.JointSpineWaist },
        { TrackedJointType.JointHipLeft, TrackedJointType.JointHipRight },
        { TrackedJointType.JointKneeLeft, TrackedJointType.JointKneeRight },
        { TrackedJointType.JointAnkleLeft, TrackedJointType.JointAnkleRight },
        { TrackedJointType.JointFootLeft, TrackedJointType.JointFootRight },
        { TrackedJointType.JointHipRight, TrackedJointType.JointHipLeft },
        { TrackedJointType.JointKneeRight, TrackedJointType.JointKneeLeft },
        { TrackedJointType.JointAnkleRight, TrackedJointType.JointAnkleLeft },
        { TrackedJointType.JointFootRight, TrackedJointType.JointFootLeft },
        { TrackedJointType.JointManual, TrackedJointType.JointManual }
    };

    public static TrackedJointType FlipJointType(TrackedJointType joint, bool flip)
    {
        return flip ? FlippedJointTypeDictionary[joint] : joint;
    }

    // @OpenVR.NET@Extensions
    public static Vector3 ExtractVrPosition(ref HmdMatrix34_t mat)
    {
        return new Vector3(mat.m3, mat.m7, -mat.m11);
    }

    // @OpenVR.NET@Extensions
    public static Quaternion ExtractVrRotation(ref HmdMatrix34_t mat)
    {
        Quaternion q = default;
        q.W = MathF.Sqrt(MathF.Max(0, 1 + mat.m0 + mat.m5 + mat.m10)) / 2;
        q.X = MathF.Sqrt(MathF.Max(0, 1 + mat.m0 - mat.m5 - mat.m10)) / 2;
        q.Y = MathF.Sqrt(MathF.Max(0, 1 - mat.m0 + mat.m5 - mat.m10)) / 2;
        q.Z = MathF.Sqrt(MathF.Max(0, 1 - mat.m0 - mat.m5 + mat.m10)) / 2;
        q.X = MathF.CopySign(q.X, mat.m9 - mat.m6);
        q.Y = MathF.CopySign(q.Y, mat.m2 - mat.m8);
        q.Z = MathF.CopySign(q.Z, mat.m1 - mat.m4);

        var scale = 1 / q.LengthSquared();
        return new Quaternion(q.X * -scale, q.Y * -scale, q.Z * -scale, q.W * scale);
    }
}

public static class SortedDictionaryExtensions
{
    public static Vector3 ValueOr(this SortedDictionary<string, Vector3> dictionary,
        string key, Vector3? fallbackValue = null)
    {
        return dictionary is not null && dictionary.TryGetValue(key, out var value)
            ? value
            : fallbackValue ?? Vector3.Zero;
    }
}

public static class QuaternionExtensions
{
    public static Quaternion Inversed(this Quaternion q, bool inverse = true)
    {
        return inverse ? Quaternion.Inverse(q) : q;
    }
}