using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.ComponentModel.Composition.Primitives;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Globalization.NumberFormatting;
using Windows.System;
using Amethyst.Classes;
using Amethyst.Plugins.Contract;
using AmethystSupport;
using Microsoft.UI.Xaml;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using RestSharp;
using Windows.Devices.Geolocation;
using static Amethyst.Classes.Shared;
using System.Diagnostics;
using System.Reflection.Metadata;

namespace Amethyst.Utils;

// AME Enumeration helpers
public static class TypeUtils
{
    public static SortedDictionary<TrackerType, string> TrackerTypeSerialDictionary = new()
    {
        { TrackerType.TrackerHanded, "vive_tracker_handed" },
        { TrackerType.TrackerLeftFoot, "vive_tracker_left_foot" },
        { TrackerType.TrackerRightFoot, "vive_tracker_right_foot" },
        { TrackerType.TrackerLeftShoulder, "vive_tracker_left_shoulder" },
        { TrackerType.TrackerRightShoulder, "vive_tracker_right_shoulder" },
        { TrackerType.TrackerLeftElbow, "vive_tracker_left_elbow" },
        { TrackerType.TrackerRightElbow, "vive_tracker_right_elbow" },
        { TrackerType.TrackerLeftKnee, "vive_tracker_left_knee" },
        { TrackerType.TrackerRightKnee, "vive_tracker_right_knee" },
        { TrackerType.TrackerWaist, "vive_tracker_waist" },
        { TrackerType.TrackerChest, "vive_tracker_chest" },
        { TrackerType.TrackerCamera, "vive_tracker_camera" },
        { TrackerType.TrackerKeyboard, "vive_tracker_keyboard" },
        { TrackerType.TrackerHead, "vive_tracker_head" },
        { TrackerType.TrackerLeftHand, "vive_tracker_left_hand" },
        { TrackerType.TrackerRightHand, "vive_tracker_right_hand" }
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
        { TrackerType.TrackerKeyboard, "AME-KEYBOARD" },
        { TrackerType.TrackerHead, "AME-HEAD" },
        { TrackerType.TrackerLeftHand, "AME-LHAND" },
        { TrackerType.TrackerRightHand, "AME-RHAND" }
    };

    public static SortedDictionary<TrackerType, TrackedJointType> TrackerTypeJointDictionary = new()
    {
        { TrackerType.TrackerHanded, TrackedJointType.JointHandLeft },
        { TrackerType.TrackerLeftFoot, TrackedJointType.JointFootLeft },
        { TrackerType.TrackerRightFoot, TrackedJointType.JointFootRight },
        { TrackerType.TrackerLeftShoulder, TrackedJointType.JointShoulderLeft },
        { TrackerType.TrackerRightShoulder, TrackedJointType.JointShoulderRight },
        { TrackerType.TrackerLeftElbow, TrackedJointType.JointElbowLeft },
        { TrackerType.TrackerRightElbow, TrackedJointType.JointElbowRight },
        { TrackerType.TrackerLeftKnee, TrackedJointType.JointKneeLeft },
        { TrackerType.TrackerRightKnee, TrackedJointType.JointKneeRight },
        { TrackerType.TrackerWaist, TrackedJointType.JointSpineWaist },
        { TrackerType.TrackerChest, TrackedJointType.JointSpineMiddle },
        { TrackerType.TrackerCamera, TrackedJointType.JointManual },
        { TrackerType.TrackerKeyboard, TrackedJointType.JointManual },
        { TrackerType.TrackerHead, TrackedJointType.JointHead },
        { TrackerType.TrackerLeftHand, TrackedJointType.JointHandLeft },
        { TrackerType.TrackerRightHand, TrackedJointType.JointHandRight }
    };

    public static SortedDictionary<TrackerType, TrackerType?> PairedTrackerTypeDictionary = new()
    {
        { TrackerType.TrackerHanded, null },
        { TrackerType.TrackerLeftFoot, TrackerType.TrackerRightFoot },
        { TrackerType.TrackerRightFoot, null },
        { TrackerType.TrackerLeftShoulder, TrackerType.TrackerRightShoulder },
        { TrackerType.TrackerRightShoulder, null },
        { TrackerType.TrackerLeftElbow, TrackerType.TrackerRightElbow },
        { TrackerType.TrackerRightElbow, null },
        { TrackerType.TrackerLeftKnee, TrackerType.TrackerRightKnee },
        { TrackerType.TrackerRightKnee, null },
        { TrackerType.TrackerWaist, null },
        { TrackerType.TrackerChest, null },
        { TrackerType.TrackerCamera, null },
        { TrackerType.TrackerKeyboard, null },
        { TrackerType.TrackerHead, null },
        { TrackerType.TrackerLeftHand, null },
        { TrackerType.TrackerRightHand, null }
    };

    public static SortedDictionary<TrackerType, TrackerType?> PairedTrackerTypeDictionaryReverse = new()
    {
        { TrackerType.TrackerHanded, null },
        { TrackerType.TrackerLeftFoot, null },
        { TrackerType.TrackerRightFoot, TrackerType.TrackerLeftFoot },
        { TrackerType.TrackerLeftShoulder, null },
        { TrackerType.TrackerRightShoulder, TrackerType.TrackerLeftShoulder },
        { TrackerType.TrackerLeftElbow, null },
        { TrackerType.TrackerRightElbow, TrackerType.TrackerLeftElbow },
        { TrackerType.TrackerLeftKnee, null },
        { TrackerType.TrackerRightKnee, TrackerType.TrackerLeftKnee },
        { TrackerType.TrackerWaist, null },
        { TrackerType.TrackerChest, null },
        { TrackerType.TrackerCamera, null },
        { TrackerType.TrackerKeyboard, null },
        { TrackerType.TrackerHead, null },
        { TrackerType.TrackerLeftHand, null },
        { TrackerType.TrackerRightHand, null }
    };

    public static SortedDictionary<TrackedJointType, TrackerType> JointTrackerTypeDictionary = new()
    {
        { TrackedJointType.JointHandLeft, TrackerType.TrackerHanded },
        { TrackedJointType.JointFootLeft, TrackerType.TrackerLeftFoot },
        { TrackedJointType.JointFootRight, TrackerType.TrackerRightFoot },
        { TrackedJointType.JointFootTipLeft, TrackerType.TrackerLeftFoot },
        { TrackedJointType.JointFootTipRight, TrackerType.TrackerRightFoot },
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
        { TrackedJointType.JointFootLeft, TrackedJointType.JointFootRight },
        { TrackedJointType.JointFootTipLeft, TrackedJointType.JointFootTipRight },
        { TrackedJointType.JointHipRight, TrackedJointType.JointHipLeft },
        { TrackedJointType.JointKneeRight, TrackedJointType.JointKneeLeft },
        { TrackedJointType.JointFootRight, TrackedJointType.JointFootLeft },
        { TrackedJointType.JointFootTipRight, TrackedJointType.JointFootTipLeft },
        { TrackedJointType.JointManual, TrackedJointType.JointManual }
    };

    public static SortedDictionary<TrackerType, string> TrackerTypeMmdDictionary = new()
    {
        { TrackerType.TrackerHanded, "他" },
        { TrackerType.TrackerLeftFoot, "左足首" },
        { TrackerType.TrackerRightFoot, "右足首" },
        { TrackerType.TrackerLeftShoulder, "左腕" },
        { TrackerType.TrackerRightShoulder, "右腕" },
        { TrackerType.TrackerLeftElbow, "左ひじ" },
        { TrackerType.TrackerRightElbow, "右ひじ" },
        { TrackerType.TrackerLeftKnee, "左ひざ" },
        { TrackerType.TrackerRightKnee, "右ひざ" },
        { TrackerType.TrackerWaist, "下半身" },
        { TrackerType.TrackerChest, "上半身" },
        { TrackerType.TrackerCamera, "Ｃａｍ" },
        { TrackerType.TrackerKeyboard, "Ｋｅｙ" },
        { TrackerType.TrackerHead, "頭" },
        { TrackerType.TrackerLeftHand, "左手首" },
        { TrackerType.TrackerRightHand, "右手首" }
    };

    public static SortedDictionary<TrackedJointType, string> JointTypeMmdDictionary = new()
    {
        { TrackedJointType.JointHead, "頭" },
        { TrackedJointType.JointNeck, "首" },
        { TrackedJointType.JointSpineShoulder, "上半身腕" },
        { TrackedJointType.JointShoulderLeft, "左腕" },
        { TrackedJointType.JointElbowLeft, "左ひじ" },
        { TrackedJointType.JointWristLeft, "左手首" },
        { TrackedJointType.JointHandLeft, "左手" },
        { TrackedJointType.JointHandTipLeft, "左手先端" },
        { TrackedJointType.JointThumbLeft, "左親指１" },
        { TrackedJointType.JointShoulderRight, "右腕" },
        { TrackedJointType.JointElbowRight, "右ひじ" },
        { TrackedJointType.JointWristRight, "右手首" },
        { TrackedJointType.JointHandRight, "右手" },
        { TrackedJointType.JointHandTipRight, "右手先端" },
        { TrackedJointType.JointThumbRight, "右親指１" },
        { TrackedJointType.JointSpineMiddle, "上半身" },
        { TrackedJointType.JointSpineWaist, "下半身" },
        { TrackedJointType.JointHipLeft, "左足" },
        { TrackedJointType.JointKneeLeft, "左ひざ" },
        { TrackedJointType.JointFootLeft, "左足首" },
        { TrackedJointType.JointFootTipLeft, "左つま先ＩＫ" },
        { TrackedJointType.JointHipRight, "右足" },
        { TrackedJointType.JointKneeRight, "右ひざ" },
        { TrackedJointType.JointFootRight, "右足首" },
        { TrackedJointType.JointFootTipRight, "右つま先ＩＫ" },
        { TrackedJointType.JointManual, "他" }
    };

    public static SortedDictionary<string, TrackerType> MmdTrackerTypeDictionary = new()
    {
        { "他", TrackerType.TrackerHanded },
        { "左足首", TrackerType.TrackerLeftFoot },
        { "右足首", TrackerType.TrackerRightFoot },
        { "左腕", TrackerType.TrackerLeftShoulder },
        { "右腕", TrackerType.TrackerRightShoulder },
        { "左ひじ", TrackerType.TrackerLeftElbow },
        { "右ひじ", TrackerType.TrackerRightElbow },
        { "左ひざ", TrackerType.TrackerLeftKnee },
        { "右ひざ", TrackerType.TrackerRightKnee },
        { "下半身", TrackerType.TrackerWaist },
        { "上半身", TrackerType.TrackerChest },
        { "Ｃａｍ", TrackerType.TrackerCamera },
        { "Ｋｅｙ", TrackerType.TrackerKeyboard },
        { "頭", TrackerType.TrackerHead },
        { "左手首", TrackerType.TrackerLeftHand },
        { "右手首", TrackerType.TrackerRightHand }
    };

    public static SortedDictionary<string, TrackedJointType> MmdJointTypeDictionary = new()
    {
        { "頭", TrackedJointType.JointHead },
        { "首", TrackedJointType.JointNeck },
        { "上半身腕", TrackedJointType.JointSpineShoulder },
        { "左腕", TrackedJointType.JointShoulderLeft },
        { "左ひじ", TrackedJointType.JointElbowLeft },
        { "左手首", TrackedJointType.JointWristLeft },
        { "左手", TrackedJointType.JointHandLeft },
        { "左手先端", TrackedJointType.JointHandTipLeft },
        { "左親指１", TrackedJointType.JointThumbLeft },
        { "右腕", TrackedJointType.JointShoulderRight },
        { "右ひじ", TrackedJointType.JointElbowRight },
        { "右手首", TrackedJointType.JointWristRight },
        { "右手", TrackedJointType.JointHandRight },
        { "右手先端", TrackedJointType.JointHandTipRight },
        { "右親指１", TrackedJointType.JointThumbRight },
        { "上半身", TrackedJointType.JointSpineMiddle },
        { "下半身", TrackedJointType.JointSpineWaist },
        { "左足", TrackedJointType.JointHipLeft },
        { "左ひざ", TrackedJointType.JointKneeLeft },
        { "左足首", TrackedJointType.JointFootLeft },
        { "左つま先ＩＫ", TrackedJointType.JointFootTipLeft },
        { "右足", TrackedJointType.JointHipRight },
        { "右ひざ", TrackedJointType.JointKneeRight },
        { "右足首", TrackedJointType.JointFootRight },
        { "右つま先ＩＫ", TrackedJointType.JointFootTipRight },
        { "他", TrackedJointType.JointManual }
    };

    public static SortedDictionary<TrackerType, string> TrackerTypeMmdDictionaryInternal = new()
    {
        { TrackerType.TrackerHanded, ".0" },
        { TrackerType.TrackerLeftFoot, ".1" },
        { TrackerType.TrackerRightFoot, ".2" },
        { TrackerType.TrackerLeftShoulder, ".3" },
        { TrackerType.TrackerRightShoulder, ".4" },
        { TrackerType.TrackerLeftElbow, ".5" },
        { TrackerType.TrackerRightElbow, ".6" },
        { TrackerType.TrackerLeftKnee, ".7" },
        { TrackerType.TrackerRightKnee, ".8" },
        { TrackerType.TrackerWaist, ".9" },
        { TrackerType.TrackerChest, ".A" },
        { TrackerType.TrackerCamera, ".B" },
        { TrackerType.TrackerKeyboard, ".C" },
        { TrackerType.TrackerHead, ".D" },
        { TrackerType.TrackerLeftHand, ".E" },
        { TrackerType.TrackerRightHand, ".F" }
    };

    public static SortedDictionary<TrackedJointType, string> JointTypeMmdDictionaryInternal = new()
    {
        { TrackedJointType.JointHead, ".0" },
        { TrackedJointType.JointNeck, ".1" },
        { TrackedJointType.JointSpineShoulder, ".2" },
        { TrackedJointType.JointShoulderLeft, ".3" },
        { TrackedJointType.JointElbowLeft, ".4" },
        { TrackedJointType.JointWristLeft, ".5" },
        { TrackedJointType.JointHandLeft, ".6" },
        { TrackedJointType.JointHandTipLeft, ".7" },
        { TrackedJointType.JointThumbLeft, ".8" },
        { TrackedJointType.JointShoulderRight, ".9" },
        { TrackedJointType.JointElbowRight, ".A" },
        { TrackedJointType.JointWristRight, ".B" },
        { TrackedJointType.JointHandRight, ".C" },
        { TrackedJointType.JointHandTipRight, ".D" },
        { TrackedJointType.JointThumbRight, ".E" },
        { TrackedJointType.JointSpineMiddle, ".F" },
        { TrackedJointType.JointSpineWaist, ".G" },
        { TrackedJointType.JointHipLeft, ".H" },
        { TrackedJointType.JointKneeLeft, ".I" },
        { TrackedJointType.JointFootLeft, ".J" },
        { TrackedJointType.JointFootTipLeft, ".K" },
        { TrackedJointType.JointHipRight, ".L" },
        { TrackedJointType.JointKneeRight, ".M" },
        { TrackedJointType.JointFootRight, ".N" },
        { TrackedJointType.JointFootTipRight, ".O" },
        { TrackedJointType.JointManual, ".P" }
    };

    public static SortedDictionary<string, TrackerType> MmdTrackerTypeDictionaryInternal = new()
    {
        { ".0", TrackerType.TrackerHanded },
        { ".1", TrackerType.TrackerLeftFoot },
        { ".2", TrackerType.TrackerRightFoot },
        { ".3", TrackerType.TrackerLeftShoulder },
        { ".4", TrackerType.TrackerRightShoulder },
        { ".5", TrackerType.TrackerLeftElbow },
        { ".6", TrackerType.TrackerRightElbow },
        { ".7", TrackerType.TrackerLeftKnee },
        { ".8", TrackerType.TrackerRightKnee },
        { ".9", TrackerType.TrackerWaist },
        { ".A", TrackerType.TrackerChest },
        { ".B", TrackerType.TrackerCamera },
        { ".C", TrackerType.TrackerKeyboard },
        { ".D", TrackerType.TrackerHead },
        { ".E", TrackerType.TrackerLeftHand },
        { ".F", TrackerType.TrackerRightHand }
    };

    public static SortedDictionary<string, TrackedJointType> MmdJointTypeDictionaryInternal = new()
    {
        { ".0", TrackedJointType.JointHead },
        { ".1", TrackedJointType.JointNeck },
        { ".2", TrackedJointType.JointSpineShoulder },
        { ".3", TrackedJointType.JointShoulderLeft },
        { ".4", TrackedJointType.JointElbowLeft },
        { ".5", TrackedJointType.JointWristLeft },
        { ".6", TrackedJointType.JointHandLeft },
        { ".7", TrackedJointType.JointHandTipLeft },
        { ".8", TrackedJointType.JointThumbLeft },
        { ".9", TrackedJointType.JointShoulderRight },
        { ".A", TrackedJointType.JointElbowRight },
        { ".B", TrackedJointType.JointWristRight },
        { ".C", TrackedJointType.JointHandRight },
        { ".D", TrackedJointType.JointHandTipRight },
        { ".E", TrackedJointType.JointThumbRight },
        { ".F", TrackedJointType.JointSpineMiddle },
        { ".G", TrackedJointType.JointSpineWaist },
        { ".H", TrackedJointType.JointHipLeft },
        { ".I", TrackedJointType.JointKneeLeft },
        { ".J", TrackedJointType.JointFootLeft },
        { ".K", TrackedJointType.JointFootTipLeft },
        { ".L", TrackedJointType.JointHipRight },
        { ".M", TrackedJointType.JointKneeRight },
        { ".N", TrackedJointType.JointFootRight },
        { ".O", TrackedJointType.JointFootTipRight },
        { ".P", TrackedJointType.JointManual }
    };

    public static SortedSet<string> ProtectedPluginGuidSet =
    [
        "K2VRTEAM-AME2-APII-DVCE-DVCEKINECTV1",
        "K2VRTEAM-AME2-APII-DVCE-DVCEKINECTV1:SETUP",
        "K2VRTEAM-AME2-APII-DVCE-DVCEKINECTV1:INSTALLER",
        "K2VRTEAM-AME2-APII-DVCE-DVCEKINECTV2",
        "K2VRTEAM-AME2-APII-DVCE-DVCEKINECTV2:SETUP",
        "K2VRTEAM-AME2-APII-DVCE-DVCEKINECTV2:INSTALLER",
        "K2VRTEAM-AME2-APII-DVCE-DVCEPSMOVEEX",
        "K2VRTEAM-AME2-APII-DVCE-DVCEPSMOVEEX:SETUP",
        "K2VRTEAM-AME2-APII-DVCE-DVCEPSMOVEEX:INSTALLER",
        "K2VRTEAM-AME2-APII-DVCE-DVCEOWOTRACK",
        "K2VRTEAM-AME2-APII-DVCE-DVCEOWOTRACK:SETUP",
        "K2VRTEAM-AME2-APII-DVCE-DVCEOWOTRACK:INSTALLER",
        "K2VRTEAM-AME2-APII-SNDP-SENDPTOPENVR",
        "K2VRTEAM-AME2-APII-SNDP-SENDPTOPENVR:SETUP",
        "K2VRTEAM-AME2-APII-SNDP-SENDPTOPENVR:INSTALLER",
        "K2VRTEAM-AME2-APII-SNDP-SENDPTVRCOSC",
        "K2VRTEAM-AME2-APII-SNDP-SENDPTVRCOSC:SETUP",
        "K2VRTEAM-AME2-APII-SNDP-SENDPTVRCOSC:INSTALLER"
    ];

    public static TrackedJointType FlipJointType(TrackedJointType joint, bool flip)
    {
        return flip ? FlippedJointTypeDictionary[joint] : joint;
    }

    public static SVector3 Projected(this Vector3 vector)
    {
        return new SVector3(vector.X, vector.Y, vector.Z);
    }

    public static SQuaternion Projected(this Quaternion quaternion)
    {
        return new SQuaternion(quaternion.X, quaternion.Y, quaternion.Z, quaternion.W);
    }

    public static Vector3 V(this SVector3 vector)
    {
        return new Vector3(vector.X, vector.Y, vector.Z);
    }

    public static Quaternion Q(this SQuaternion quaternion)
    {
        return new Quaternion(
            quaternion.X, quaternion.Y,
            quaternion.Z, quaternion.W);
    }

    public static (Vector3 V, Quaternion Q) T(this STransform transform)
    {
        return (transform.Translation.V(), transform.Rotation.Q());
    }

    public static bool TryParseJson(this string @this, out JObject result)
    {
        var success = true;
        var settings = new JsonSerializerSettings
        {
            Error = (sender, args) =>
            {
                success = false;
                args.ErrorContext.Handled = true;
            },
            MissingMemberHandling = MissingMemberHandling.Error
        };
        result = JsonConvert.DeserializeObject<JObject>(@this, settings);
        return success;
    }

    public static JToken TryGetValue(this JObject json, string propertyName)
    {
        return json.TryGetValue(propertyName, out var value) ? value : null;
    }

    public static async Task<IEnumerable<T>> WhenAll<T>(this IEnumerable<Task<T>> tasks)
    {
        return await Task.WhenAll(tasks);
    }

    public static T GetMetadata<T>(this Type type, string name, T fallback = default)
    {
        try
        {
            // Find the metadata result
            var result = type?.CustomAttributes.FirstOrDefault(x =>
                    x.ConstructorArguments.FirstOrDefault().Value?.ToString() == name)?
                .ConstructorArguments.ElementAtOrDefault(1).Value;

            // Validate the returned value
            if (result is null) return fallback;

            // Check whether the retrieved value is a type
            // which we want to be return as instantiated
            if (!((result as Type)?.GetInterfaces().Any(x => x == typeof(IDependencyInstaller)) ?? false))
                return (T)result ?? fallback;

            // Find a constructor and instantiate the type
            return (T)(result as Type)?.GetConstructor(Type.EmptyTypes)?
                .Invoke(null) ?? fallback; // Invoke now
        }
        catch (Exception e)
        {
            Logger.Error(e);
            return fallback ?? default;
        }
    }

    public static T Instantiate<T>(this Type type)
    {
        try
        {
            // Validate the returned value
            if (type is null) return default;

            // Find a constructor and instantiate the type
            return (T)type.GetConstructor(Type.EmptyTypes)?
                .Invoke(null) ?? default; // Invoke now
        }
        catch (Exception e)
        {
            Logger.Error(e);
            return default;
        }
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

    public static Quaternion ValueOr(this SortedDictionary<string, Quaternion> dictionary,
        string key, Quaternion? fallbackValue = null)
    {
        return dictionary is not null && dictionary.TryGetValue(key, out var value)
            ? value
            : fallbackValue ?? Quaternion.Identity;
    }

    public static (Vector3 Position, Quaternion Orientation) ValueOr(
        this SortedDictionary<string, (Vector3 Position, Quaternion Orientation)> dictionary,
        string key, (Vector3 Position, Quaternion Orientation)? fallbackValue = null)
    {
        return dictionary is not null && dictionary.TryGetValue(key, out var value)
            ? value
            : fallbackValue ?? (Vector3.Zero, Quaternion.Identity);
    }
}

public static class QuaternionExtensions
{
    public static Quaternion Inversed(this Quaternion q, bool inverse = true)
    {
        return inverse ? Quaternion.Inverse(q) : q;
    }
}

public static class ExceptionExtensions
{
    public static Exception UnwrapCompositionException(this Exception exception)
    {
        if (exception is not CompositionException compositionException) return exception;
        var unwrapped = compositionException;

        while (unwrapped != null)
        {
            var firstError = unwrapped.Errors.FirstOrDefault();
            var currentException = firstError?.Exception;

            if (currentException == null) break;

            if (currentException is ComposablePartException { InnerException: not null } decomposablePartException)
            {
                if (decomposablePartException.InnerException is not CompositionException innerCompositionException)
                    return currentException.InnerException ?? exception;
                currentException = innerCompositionException;
            }

            unwrapped = currentException as CompositionException;
        }

        return exception; // Throw the original
    }
}

public static class RestExtensions
{
    public static Task<RestResponse> GetAsyncAuthorized(this RestClient client, RestRequest request)
    {
        if (AppData.Settings.GitHubToken.Valid)
            request.AddHeader("Authorization", // Optionally add the authorization token
                $"bearer {AppData.Settings.GitHubToken.Token.Decrypt()}");

        return client.GetAsync(request);
    }
}

public static class StringExtensions
{
    public static string Encrypt(this string s)
    {
        if (string.IsNullOrEmpty(s)) return s;

        var encoding = new UTF8Encoding();
        var plain = encoding.GetBytes(s);
        var secret = ProtectedData.Protect(plain, null, DataProtectionScope.CurrentUser);
        return Convert.ToBase64String(secret);
    }

    public static string Decrypt(this string s)
    {
        if (string.IsNullOrEmpty(s)) return s;

        var secret = Convert.FromBase64String(s);
        var plain = ProtectedData.Unprotect(secret, null, DataProtectionScope.CurrentUser);
        var encoding = new UTF8Encoding();
        return encoding.GetString(plain);
    }

    public static string Format(this string s, params object[] arguments)
    {
        var result = s; // Create a backup
        var formats = arguments.ToList();

        foreach (var format in formats)
            if (result.Contains($"{{{formats.IndexOf(format)}}}")) // Check whether the replace index is valid
                result = result.Replace($"{{{formats.IndexOf(format)}}}", format?.ToString() ?? string.Empty);
            else Logger.Info($"{s} doesn't contain a placeholder for index {{{formats.IndexOf(format)}}}!");

        return result; // Return the outer result
    }

    public static bool IsProtectedGuid(this string s)
    {
        return TypeUtils.ProtectedPluginGuidSet.Contains(s);
    }
}

public static class StorageExtensions
{
    public static void CopyToFolder(this DirectoryInfo source, string destination, bool inside = false, bool overwrite = true)
    {
        // Create the base directory (if needed)
        if (inside) destination = Path.Join(destination, source.Name);

        // Now Create all the directories
        foreach (var dirPath in source.GetDirectories("*", SearchOption.AllDirectories))
        {
            Logger.Info($"Creating folder {source} in {destination}\\");
            Directory.CreateDirectory(dirPath.FullName.Replace(source.FullName, destination));
        }

        // Copy all the files & Replaces any files with the same name
        foreach (var newPath in source.GetFiles("*.*", SearchOption.AllDirectories)
                     .Where(x => overwrite || !File.Exists(x.FullName.Replace(source.FullName, destination))))
        {
            Logger.Info($"Copying file {source} to {destination}\\");
            newPath.CopyTo(newPath.FullName.Replace(source.FullName, destination), overwrite);
        }
    }
}

public static class UriExtensions
{
    public static Uri ToUri(this string source)
    {
        return new Uri(source);
    }

    public static async Task LaunchAsync(this Uri uri)
    {
        try
        {
            if (await Launcher.QueryAppUriSupportAsync(uri) is LaunchQuerySupportStatus.Available ||
                uri.Scheme is "amethyst-app") await Launcher.LaunchUriAsync(uri);
            else
                Logger.Warn($"No application registered to handle uri of \"{uri.Scheme}:\"," +
                            $" query result: {await Launcher.QueryAppUriSupportAsync(uri)}");
        }
        catch (Exception e)
        {
            Logger.Error(e);
        }
    }

    public static async Task Launch(this string uri)
    {
        try
        {
            if (PathsHandler.IsAmethystPackaged)
            {
                await uri.ToUri().LaunchAsync();
            }
            else
            {
                // If we've found who asked
                if (File.Exists(Interfacing.ProgramLocation.FullName))
                {
                    var info = new ProcessStartInfo
                    {
                        FileName = Interfacing.ProgramLocation.FullName.Replace(".dll", ".exe"),
                        Arguments = uri
                    };

                    try
                    {
                        Process.Start(info);
                    }
                    catch (Exception e)
                    {
                        Logger.Fatal(e);
                    }
                }
            }
        }
        catch (Exception e)
        {
            Logger.Error(e);
        }
    }
}

public static class VersionExtensions
{
    public static Version AsVersion(this PackageVersion version)
    {
        return new Version(version.Major, version.Minor, version.Build, version.Revision);
    }

    public static string AsString(this PackageVersion version)
    {
        return $"{version.Major}.{version.Minor}.{version.Build}.{version.Revision}";
    }
}

public static class StreamExtensions
{
    public static async Task CopyToWithProgressAsync(this Stream source,
        Stream destination, Action<long> progress = null, int bufferSize = 10240)
    {
        var buffer = new byte[bufferSize];
        var total = 0L;
        int amtRead;

        do
        {
            amtRead = 0;
            while (amtRead < bufferSize)
            {
                var numBytes = await source.ReadAsync(
                    buffer, amtRead, bufferSize - amtRead);
                if (numBytes == 0) break;
                amtRead += numBytes;
            }

            total += amtRead;
            await destination.WriteAsync(buffer, 0, amtRead);
            progress?.Invoke(total);
        } while (amtRead == bufferSize);
    }
}

public class VisibilityTrigger : StateTriggerBase
{
    private FrameworkElement _element;
    private Visibility _trigger;

    public FrameworkElement Target
    {
        get => _element;
        set
        {
            _element = value;
            RefreshState();
        }
    }

    public Visibility ActiveOn
    {
        get => _trigger;
        set
        {
            _trigger = value;
            RefreshState();
        }
    }

    private void RefreshState()
    {
        SetActive(Target?.Visibility == ActiveOn);
    }
}

public partial class OffsetDigitsFormatter : INumberFormatter2, INumberParser
{
    public string FormatInt(long value)
    {
        return double.Round(value / 100.0, 2).ToString(CultureInfo.CurrentCulture);
    }

    public string FormatUInt(ulong value)
    {
        return double.Round(value / 100.0, 2).ToString(CultureInfo.CurrentCulture);
    }

    public string FormatDouble(double value)
    {
        return double.Round(value / 100.0, 2).ToString(CultureInfo.CurrentCulture);
    }

    public long? ParseInt(string text)
    {
        return long.TryParse(text.Replace(',', '.'),
            NumberStyles.Any, CultureInfo.InvariantCulture, out var result)
            ? (long)double.Round(result * 100.0, 0)
            : null;
    }

    public ulong? ParseUInt(string text)
    {
        return ulong.TryParse(text.Replace(',', '.'),
            NumberStyles.Any, CultureInfo.InvariantCulture, out var result)
            ? (ulong)double.Round(result * 100.0, 0)
            : null;
    }

    public double? ParseDouble(string text)
    {
        return double.TryParse(text.Replace(',', '.'),
            NumberStyles.Any, CultureInfo.InvariantCulture, out var result)
            ? double.Round(result * 100.0, 0)
            : null;
    }
}