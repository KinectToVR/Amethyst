using Windows.ApplicationModel;
using Amethyst.Utils;
using System.Reflection;

namespace Amethyst.Classes;

public static class AppData
{
    // K2VR web and docs API version
    public const uint ApiVersion = 0;

    // Internal version number
    public static (string Display, string Internal)
        VersionString => (Assembly.GetExecutingAssembly().GetName().Version?.ToString() ?? "1.0.0.0", "AZ_BUILD_NUMBER");

    // Application settings
    public static AppSettings Settings { get; set; } = new();

    // Application token
    public const string ApiToken = "AZ_API_TOKEN";

    // AppCenter tokens
    public const string AppSecret = "AZ_APPCENTER_SECRET";
    public const string UpdateToken = "AZ_APPCENTER_RO_TOKEN";
}