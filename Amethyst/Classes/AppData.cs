﻿namespace Amethyst.Classes;

public static class AppData
{
    public const uint InternalVersion = 5; // Amethyst version

    public const uint ApiVersion = 0; // API version

    // Internal version number
    public static (string Display, string Internal)
        VersionString { get; } = ("1.2.0.2", "1.2.230222.1");

    // Application settings
    public static AppSettings Settings { get; set; } = new();

    // Application token
    public const string ApiToken = "AZ_API_TOKEN";
}