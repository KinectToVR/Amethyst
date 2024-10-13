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

    public static SortedSet<string> ProtectedPluginGuidSet = new()
    {
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

    public static Task<RestResponse> GetAsyncAuthorized(this RestClient client, RestRequest request)
    {
        if (AppData.Settings.GitHubToken.Valid)
            request.AddHeader("Authorization", // Optionally add the authorization token
                $"bearer {AppData.Settings.GitHubToken.Token.Decrypt()}");

        return client.GetAsync(request);
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
    public static void CopyToFolder(this DirectoryInfo source, string destination, bool inside = false)
    {
        // Create the base directory (if needed)
        if (inside) destination = Path.Join(destination, source.Name);

        // Now Create all of the directories
        foreach (var dirPath in source.GetDirectories("*", SearchOption.AllDirectories))
        {
            Logger.Info($"Creating folder {source} in {destination}\\");
            Directory.CreateDirectory(dirPath.FullName.Replace(source.FullName, destination));
        }

        // Copy all the files & Replaces any files with the same name
        foreach (var newPath in source.GetFiles("*.*", SearchOption.AllDirectories))
        {
            Logger.Info($"Copying file {source} to {destination}\\");
            newPath.CopyTo(newPath.FullName.Replace(source.FullName, destination), true);
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

public class OffsetDigitsFormatter : INumberFormatter2, INumberParser
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