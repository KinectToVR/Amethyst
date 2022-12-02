namespace Amethyst.Classes;

public static class AppData
{
    // Internal version number
    public const string K2InternalVersion = "1.2.220000.1";
    public const uint K2IntVersion = 5; // Amethyst version
    public const uint K2ApiVersion = 0; // API version

    // Application settings
    public static AppSettings Settings = new();
}