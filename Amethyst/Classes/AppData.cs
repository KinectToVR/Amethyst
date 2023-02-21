namespace Amethyst.Classes;

public static class AppData
{
    // Internal version number
    public static (string Display, string Internal)
        VersionString { get; } = ("1.2.0.2", "1.2.230222.1");

    public const uint InternalVersion = 5; // Amethyst version
    public const uint ApiVersion = 0; // API version

    // Application settings
    public static AppSettings Settings { get; set; } = new();
}