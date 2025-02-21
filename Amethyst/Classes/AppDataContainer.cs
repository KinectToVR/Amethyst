using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Threading.Tasks;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using Newtonsoft.Json;
using Windows.Storage;

namespace Amethyst.Classes;

public class AppDataContainer : INotifyPropertyChanged
{
    // ReSharper disable once MemberCanBePrivate.Global
    public SortedDictionary<object, object> SettingsDictionary { get; set; } = new();

    // MVVM stuff
    public event PropertyChangedEventHandler PropertyChanged;

    // Save settings
    public void SaveSettings()
    {
        try
        {
            // Save host settings to $env:AppData/Amethyst/
            File.WriteAllText(Interfacing.GetAppDataFilePath("HostSettings.json"),
                JsonConvert.SerializeObject(this, Formatting.Indented));
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
            // Read host settings from $env:AppData/Amethyst/
            SettingsDictionary = (JsonConvert.DeserializeObject<AppDataContainer>(File.ReadAllText(
                Interfacing.GetAppDataFilePath("HostSettings.json"))) ?? new AppDataContainer()).SettingsDictionary;
        }
        catch (Exception e)
        {
            Logger.Error($"Error reading host settings! Message: {e.Message}");
            SettingsDictionary = new SortedDictionary<object, object>(); // Reset if null
        }
    }

    // Save settings
    public async Task SaveSettingsAsync(bool silent = false)
    {
        try
        {
            // Save host settings to $env:AppData/Amethyst/
            await File.WriteAllTextAsync(Interfacing.GetAppDataFilePath("HostSettings.json"),
                JsonConvert.SerializeObject(this, Formatting.Indented));
        }
        catch (Exception e)
        {
            if (!silent) Logger.Error($"Error saving host settings! Message: {e.Message}");
        }
    }

    // Re/Load settings
    public async Task ReadSettingsAsync(bool silent = false)
    {
        try
        {
            // Read host settings from $env:AppData/Amethyst/
            SettingsDictionary = (JsonConvert.DeserializeObject<AppDataContainer>(await File.ReadAllTextAsync(
                Interfacing.GetAppDataFilePath("HostSettings.json"))) ?? new AppDataContainer()).SettingsDictionary;
        }
        catch (Exception e)
        {
            if (e is FileNotFoundException) await SaveSettingsAsync(silent);
            if (!silent) Logger.Error($"Error reading host settings! Message: {e.Message}");
            SettingsDictionary = new SortedDictionary<object, object>(); // Reset if null
        }
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    public object this[object key]
    {
        get => PathsHandler.IsAmethystPackaged
            ? ApplicationData.Current.LocalSettings.Values[key?.ToString() ?? "INVALID"]
            : SettingsDictionary.GetValueOrDefault(key);
        set
        {
            if (PathsHandler.IsAmethystPackaged)
                SettingsDictionary[key] = value;
            else
                ApplicationData.Current.LocalSettings.Values[key?.ToString() ?? "INVALID"] = value;

            SaveSettings();
            OnPropertyChanged(nameof(SettingsDictionary));
        }
    }

    public void Remove(object key)
    {
        if (PathsHandler.IsAmethystPackaged)
            SettingsDictionary.Remove(key);
        else
            ApplicationData.Current.LocalSettings.Values.Remove(key?.ToString() ?? "INVALID");

        SaveSettings();
        OnPropertyChanged(nameof(SettingsDictionary));
    }
}