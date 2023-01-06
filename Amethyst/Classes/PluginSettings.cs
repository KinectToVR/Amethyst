using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using Newtonsoft.Json;

namespace Amethyst.Classes;

public class AppPluginSettings : INotifyPropertyChanged
{
    // ReSharper disable once MemberCanBePrivate.Global | Calibration matrices : GUID/Data
    public SortedDictionary<string, SortedDictionary<object, object>> PluginSettingsDictionary { get; set; } = new();

    // MVVM stuff
    public event PropertyChangedEventHandler PropertyChanged;

#nullable enable
    // Get a serialized object from the plugin settings
    public T? GetPluginSetting<T>(string guid, object key, T? fallback = default)
    {
        // Check if the selected plugin has any settings saved
        if (PluginSettingsDictionary.TryGetValue(guid, out var settingsRoot))
            if (settingsRoot.TryGetValue(key, out var value))
            {
                try
                {
                    if (value.GetType().IsAssignableFrom(typeof(T))) return (T)value; // Return the value if valid
                    return JsonConvert.DeserializeObject<T>(value.ToString() ?? ""); // Otherwise deserialize
                }
                catch
                {
                    Logger.Info($"Plugin {guid} settings have an invalid definition for {key}! Creating a new one...");
                    return fallback ?? default; // Just don't care further
                }
            }
            else
            {
                Logger.Info($"Plugin {guid} settings do not have a definition for {key}! Creating a new one...");
                return fallback ?? default; // Just don't care further
            }

        // Still here? We must have failed...
        Logger.Info($"Plugin {guid} settings root is invalid! Creating a new one...");
        PluginSettingsDictionary.Add(guid, new SortedDictionary<object, object>());
        return fallback ?? default; // Just don't care further
    }

    // Write a serialized object to the plugin settings
    public void SetPluginSetting(string guid, object key, object value)
    {
        // Check if the selected plugin has any settings saved
        if (!PluginSettingsDictionary.ContainsKey(guid))
        {
            Logger.Info($"Plugin {guid} settings root is invalid! Creating a new one...");
            PluginSettingsDictionary.Add(guid, new SortedDictionary<object, object>());
        }

        // Try getting the settings root and applying our changes
        if (PluginSettingsDictionary.TryGetValue(guid, out var settingsRoot))
            if (settingsRoot.ContainsKey(key))
            {
                // Return the value if valid
                settingsRoot[key] = value;
                return; // Winning it!
            }
            else
            {
                Logger.Info($"Plugin {guid} settings do not have a definition for {key}! Creating a new one...");
                settingsRoot.Add(key, value);
                return; // Winning it!
            }

        // Still here? We must have failed...
        Logger.Info($"Plugin {guid} settings root is invalid! Giving up...");
    }
#nullable disable

    // Save settings
    public void SaveSettings()
    {
        try
        {
            // Save plugin settings to $env:AppData/Amethyst/
            File.WriteAllText(
                Interfacing.GetAppDataFileDir("AmethystPluginsSettings.json"),
                JsonConvert.SerializeObject(TrackingDevices.PluginSettings, Formatting.Indented));
        }
        catch (Exception e)
        {
            Logger.Error($"Error saving plugin settings! Message: {e.Message}");
        }
    }

    // Re/Load settings
    public void ReadSettings()
    {
        try
        {
            // Read plugin settings from $env:AppData/Amethyst/
            TrackingDevices.PluginSettings = JsonConvert.DeserializeObject<AppPluginSettings>(File.ReadAllText(
                Interfacing.GetAppDataFileDir("AmethystPluginsSettings.json"))) ?? new AppPluginSettings();
        }
        catch (Exception e)
        {
            Logger.Error($"Error reading plugin settings! Message: {e.Message}");
            TrackingDevices.PluginSettings ??= new AppPluginSettings(); // Reset if null
        }
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }
}

public class PluginSettingsHelper : IPluginSettings
{
    [SetsRequiredMembers]
    public PluginSettingsHelper(string guid)
    {
        Guid = guid;
    }

    private string Guid { get; }

#nullable enable
    // Get a serialized object from the plugin settings
    public T? GetSetting<T>(object key, T? fallback = default)
    {
        if (TrackingDevices.PluginSettings is null) return fallback ?? default;
        return TrackingDevices.PluginSettings.GetPluginSetting(Guid, key, fallback);
    }

    // Write a serialized object to the plugin settings
    public void SetSetting<T>(object key, T? value)
    {
        if (value is null)
        {
            Logger.Info($"The value requested to be saved at key \"{key}\" by {Guid} is null! Aborting...");
            return; // Sanity check, discard if null for some reason
        }

        TrackingDevices.PluginSettings?.SetPluginSetting(Guid, key, value);
        TrackingDevices.PluginSettings?.SaveSettings(); // Save it btw!
    }
#nullable disable
}