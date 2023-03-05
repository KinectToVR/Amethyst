using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.ComponentModel.Composition.Primitives;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Threading.Tasks;
using Amethyst.Plugins.Contract;
using AmethystSupport;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using RestSharp;

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
        { TrackerType.TrackerKeyboard, TrackedJointType.JointManual }
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
        { TrackerType.TrackerKeyboard, null }
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
        { TrackerType.TrackerKeyboard, null }
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

            if (currentException is ComposablePartException { InnerException: { } } decomposablePartException)
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
    public static Task<RestResponse<T>> ExecuteGetAsync<T>(this RestClient client, string baseUrl, RestRequest request)
    {
        client.Options.BaseUrl = new Uri(baseUrl);
        return client.ExecuteGetAsync<T>(request);
    }

    public static Task<RestResponse> ExecuteGetAsync(this RestClient client, string baseUrl, RestRequest request)
    {
        client.Options.BaseUrl = new Uri(baseUrl);
        return client.ExecuteGetAsync(request);
    }

    public static Task<byte[]> ExecuteDownloadDataAsync(this RestClient client, string baseUrl, RestRequest request)
    {
        client.Options.BaseUrl = new Uri(baseUrl);
        return client.DownloadDataAsync(request);
    }

    public static Task<Stream> ExecuteDownloadStreamAsync(this RestClient client, string baseUrl, RestRequest request)
    {
        client.Options.BaseUrl = new Uri(baseUrl);
        return client.DownloadStreamAsync(request);
    }
}