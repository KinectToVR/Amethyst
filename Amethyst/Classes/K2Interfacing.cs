using System;
using System.Collections.Generic;
using System.IO;
using System.Numerics;
using System.Reflection;
using Amethyst.Utils;
using AmethystPluginContract;
using Valve.VR;

namespace Amethyst.Classes;

public static class Interfacing
{
    public static FileInfo GetProgramLocation()
    {
        return new FileInfo(Assembly.GetExecutingAssembly().Location);
    }

    public static DirectoryInfo GetK2AppDataTempDir()
    {
        return Directory.CreateDirectory(Path.GetTempPath() + "Amethyst");
    }

    public static string GetK2AppDataFileDir(string relativeFilePath)
    {
        Directory.CreateDirectory(Path.Join(
            Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "Amethyst"));

        return Path.Join(Environment.GetFolderPath(
            Environment.SpecialFolder.ApplicationData), "Amethyst", relativeFilePath);
    }

    public static string GetK2AppDataLogFileDir(string relativeFolderName, string relativeFilePath)
    {
        Directory.CreateDirectory(Path.Join(
            Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
            "Amethyst", "logs", relativeFolderName));

        return Path.Join(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
            "Amethyst", "logs", relativeFolderName, relativeFilePath);
    }

    // Internal version number
    public const string K2InternalVersion = "1.0.3.1";

    public const uint K2INTVersion = 3; // Amethyst version
    public const uint K2APIVersion = 0; // API version

    public static bool
        isExitingNow = false, // App closing check
        isExitHandled = false; // If actions have been done

    // App crash check
    public static FileInfo crashFile;

    // Update check
    public static bool
        updateFound = false,
        updateOnClosed = false,
        checkingUpdatesNow = false,
        updatingNow = false;

    // Position helpers for k2 devices -> GUID, Pose
    public static SortedDictionary<string, Vector3>
        kinectHeadPosition, // But this one's kinect-only
        deviceRelativeTransformOrigin; // This one applies to both

    // OpenVR playspace position
    public static Vector3 vrPlayspaceTranslation = new(0);

    // OpenVR playspace rotation
    public static Quaternion vrPlayspaceOrientationQuaternion = new(0, 0, 0, 1);

    // Current page string
    public static string currentPageTag = "general";
    public static string currentPageClass = "Amethyst.GeneralPage";

    // Current app state string (e.g. "general", "calibration_manual")
    public static string currentAppState = "general";

    // Currently available website language code
    public static string docsLanguageCode = "en";

    // VR Overlay handle
    public static ulong vrOverlayHandle = OpenVR.k_ulOverlayHandleInvalid;
    public static uint vrNotificationID = 0;

    // The actual app theme (ONLY dark/light)
    public static Microsoft.UI.Xaml.ElementTheme actualTheme =
        Microsoft.UI.Xaml.ElementTheme.Dark;

    // Application settings
    public static K2AppSettings AppSettings = new();

    // Fail with an exit code (don't delete .crash)
    public static void Fail(int code)
    {
        isExitHandled = true;
        Environment.Exit(code);
    }

    // Return a language name by code
    // Input: The current (or deduced) language key / en
    // Returns: LANG_NATIVE (LANG_LOCALIZED) / Nihongo (Japanese)
    // https://stackoverflow.com/a/10607146/13934610
    // https://stackoverflow.com/a/51867679/13934610
    public static string GetLocalizedLanguageName(string languageKey)
    {
        try
        {
            // Load the locales.json from Assets/Strings/
            var resourcePath = Path.Join(
                GetProgramLocation().DirectoryName,
                "Assets", "Strings", "locales.json");

            // If the specified language doesn't exist somehow, fallback to 'en'
            if (!File.Exists(resourcePath))
            {
                Logger.Error("Could not load language enumeration resources at " +
                             $"\"{resourcePath}\", app interface will be broken!");
                return languageKey; // Give up on trying
            }

            // Parse the loaded json
            var jsonObject = Windows.Data.Json.JsonObject.Parse(File.ReadAllText(resourcePath));

            // Check if the resource root is fine
            if (jsonObject == null || jsonObject.Count <= 0)
            {
                Logger.Error("The current language enumeration resource root is empty! " +
                             "App interface will be broken!");
                return languageKey; // Give up on trying
            }

            // If the language key is the current language, don't split the name
            if (AppSettings.AppLanguage == languageKey)
                return jsonObject.GetNamedObject(AppSettings.AppLanguage).GetNamedString(AppSettings.AppLanguage);

            // Else split the same way as in docs
            return jsonObject.GetNamedObject(languageKey).GetNamedString(languageKey) +
                   " (" + jsonObject.GetNamedObject(AppSettings.AppLanguage).GetNamedString(languageKey) + ")";
        }
        catch (Exception e)
        {
            Logger.Error($"JSON error at key: \"{languageKey}\"! Message: {e.Message}");

            // Else return they key alone
            return languageKey;
        }
    }

    // Load the current desired resource JSON into app memory
    public static void LoadJSONStringResources(string languageKey)
    {
        try
        {
            Logger.Info($"Searching for language resources with key \"{languageKey}\"...");

            var resourcePath = Path.Join(
                GetProgramLocation().DirectoryName,
                "Assets", "Strings", languageKey + ".json");

            // If the specified language doesn't exist somehow, fallback to 'en'
            if (!File.Exists(resourcePath))
            {
                Logger.Warn($"Could not load language resources at " +
                            $"\"{resourcePath}\", falling back to 'en' (en.json)!");

                resourcePath = Path.Join(
                    GetProgramLocation().DirectoryName,
                    "Assets", "Strings", "en.json");
            }

            // If failed again, just give up
            if (!File.Exists(resourcePath))
            {
                Logger.Warn($"Could not load language resources at " +
                            $"\"{resourcePath}\", the app interface will be broken!");
                return; // Just give up
            }

            // If everything's ok, load the resources into the current resource tree

            // Parse the loaded json
            LocalResources = Windows.Data.Json.JsonObject.Parse(File.ReadAllText(resourcePath));

            // Check if the resource root is fine
            if (LocalResources == null || LocalResources.Count <= 0)
                Logger.Error("The current resource root is empty! App interface will be broken!");
            else
                Logger.Info($"Successfully loaded language resources with key \"{languageKey}\"!");
        }
        catch (Exception e)
        {
            Logger.Error($"JSON error at key: \"{languageKey}\"! Message: {e.Message}");
        }
    }

    // Load the current desired resource JSON into app memory
    public static void LoadJSONStringResources_English()
    {
        try
        {
            Logger.Info("Searching for shared (English) language resources...");

            var resourcePath = Path.Join(
                GetProgramLocation().DirectoryName,
                "Assets", "Strings", "en.json");

            // If failed again, just give up
            if (!File.Exists(resourcePath))
            {
                Logger.Warn("Could not load language resources at \"{resourcePath}\", " +
                            "falling back to the current one! The app interface may be broken!");

                // Override the current english resource tree
                EnglishResources = LocalResources;
                return; // Just give up
            }

            // If everything's ok, load the resources into the current resource tree

            // Parse the loaded json
            EnglishResources = Windows.Data.Json.JsonObject.Parse(File.ReadAllText(resourcePath));

            // Check if the resource root is fine
            if (EnglishResources == null || EnglishResources.Count <= 0)
                Logger.Error("The current resource root is empty! App interface will be broken!");
            else
                Logger.Info("Successfully loaded language resources with key \"en\"!");
        }
        catch (Exception e)
        {
            Logger.Error($"JSON error at key: \"en\"! Message: {e.Message}");
        }
    }

    // Get a string from runtime JSON resources, language from settings
    public static string LocalizedJSONString(string resourceKey)
    {
        try
        {
            // Check if the resource root is fine
            if (LocalResources != null && LocalResources.Count > 0)
                return LocalResources.GetNamedString(resourceKey);

            Logger.Error("The current resource root is empty! App interface will be broken!");
            return resourceKey; // Just give up
        }
        catch (Exception e)
        {
            Logger.Error($"JSON error at key: \"{resourceKey}\"! Message: {e.Message}");

            // Else return they key alone
            return resourceKey;
        }
    }

    // Amethyst language resource trees
    public static Windows.Data.Json.JsonObject
        LocalResources = new(), EnglishResources = new(), LanguageEnum = new();
}