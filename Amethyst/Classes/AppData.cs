using Windows.ApplicationModel;
using Amethyst.Utils;

namespace Amethyst.Classes;

public static class AppData
{
    // K2VR web and docs API version
    public const uint ApiVersion = 0;

    // Internal version number
    public static (string Display, string Internal)
        VersionString => (Package.Current.Id.Version.AsString(), "AZ_BUILD_NUMBER");

    // Application settings
    public static AppSettings Settings { get; set; } = new();

    // Application token
    public const string ApiToken = "AZ_API_TOKEN";

    // AppCenter tokens
    public const string AppSecret = "AZ_APPCENTER_SECRET";
    public const string UpdateToken = "AZ_APPCENTER_RO_TOKEN";
}