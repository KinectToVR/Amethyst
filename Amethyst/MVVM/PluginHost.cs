using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.ComponentModel.Composition;
using System.ComponentModel.Composition.Hosting;
using System.ComponentModel.Composition.Primitives;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using Windows.Storage;
using Windows.System;
using Amethyst.Classes;
using Amethyst.Plugins.Contract;
using Amethyst.Schedulers;
using Amethyst.Utils;
using AmethystSupport;
using Microsoft.AppCenter.Crashes;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using RestSharp;
using static Amethyst.Classes.Interfacing;

namespace Amethyst.MVVM;

public class PluginHost : IAmethystHost
{
    [SetsRequiredMembers]
    public PluginHost(string guid)
    {
        Guid = guid; // Cache the guid and rebuild
        SettingsHelper = new PluginSettingsHelper(guid);
    }

    // Internal plugin guid
    private string Guid { get; }

    // Settings helper : read/write plugins settings
    private PluginSettingsHelper SettingsHelper { get; }

    // Get the plugin settings helper
    public IPluginSettings PluginSettings => SettingsHelper;

    // Helper to get all joints' positions from the app, which are added in Amethyst.
    // Note: if joint's off, its trackingState will be ITrackedJointState::State_NotTracked
    public List<TrackedJoint> AppJointPoses =>
        AppData.Settings.TrackersVector.Select(x => x.GetTrackedJoint()).ToList();

    // Get the HMD Yaw (exclusively)
    public double HmdOrientationYaw =>
        Support.QuaternionYaw(Interfacing.Plugins.GetHmdPose.Orientation.Projected());

    // Get the raw OpenVRs HMD pose
    public (Vector3 Position, Quaternion Orientation) HmdPose => Interfacing.Plugins.GetHmdPose;

    // Check if a joint with the specified role is provided by the base device
    public bool IsTrackedJointValid(TrackedJointType jointType)
    {
        return AppPlugins.BaseTrackingDevice.TrackedJoints.Exists(x => x.Role == jointType);
    }

    // Lock the main update loop while in scope with [lock (UpdateThreadLock) { }]
    public object UpdateThreadLock => UpdateLock;

    // Get the hook joint pose (typically Head, fallback to .First())
    public (Vector3 Position, Quaternion Orientation) GetHookJointPose(bool calibrated = false)
    {
        (Vector3 jointPosition, Quaternion jointOrientation) = DeviceHookJointPosition
            .TryGetValue(AppData.Settings.TrackingDeviceGuid, out var pose)
            ? pose // Copy the position if everything's fine, return a placeholder if not
            : (Vector3.Zero, Quaternion.Identity);

        if (!calibrated)
            return (jointPosition, jointOrientation);

        // Construct the calibrated pose
        return (Vector3.Transform(
                    // Input, position
                    jointPosition - AppData.Settings.DeviceCalibrationOrigins
                        .GetValueOrDefault(AppData.Settings.TrackingDeviceGuid, Vector3.Zero),
                    // Rotation
                    AppData.Settings.DeviceCalibrationRotationMatrices
                        .GetValueOrDefault(AppData.Settings.TrackingDeviceGuid, Quaternion.Identity)) +
                // Translation
                AppData.Settings.DeviceCalibrationTranslationVectors
                    .GetValueOrDefault(AppData.Settings.TrackingDeviceGuid, Vector3.Zero) +
                // Origin
                AppData.Settings.DeviceCalibrationOrigins
                    .GetValueOrDefault(AppData.Settings.TrackingDeviceGuid, Vector3.Zero),
            // Orientation
            AppData.Settings.DeviceCalibrationRotationMatrices
                .GetValueOrDefault(AppData.Settings.TrackingDeviceGuid, Quaternion.Identity) * jointOrientation);
    }

    // Get the pose of the relative transform origin joint
    public (Vector3 Position, Quaternion Orientation) GetTransformJointPose(bool calibrated = false)
    {
        (Vector3 jointPosition, Quaternion jointOrientation) = DeviceRelativeTransformOrigin
            .TryGetValue(AppData.Settings.TrackingDeviceGuid, out var pose)
            ? pose // Copy the position if everything's fine, return a placeholder if not
            : (Vector3.Zero, Quaternion.Identity);

        if (!calibrated)
            return (jointPosition, jointOrientation);

        // Construct the calibrated pose
        return (Vector3.Transform(
                    // Input, position
                    jointPosition - AppData.Settings.DeviceCalibrationOrigins
                        .GetValueOrDefault(AppData.Settings.TrackingDeviceGuid, Vector3.Zero),
                    // Rotation
                    AppData.Settings.DeviceCalibrationRotationMatrices
                        .GetValueOrDefault(AppData.Settings.TrackingDeviceGuid, Quaternion.Identity)) +
                // Translation
                AppData.Settings.DeviceCalibrationTranslationVectors
                    .GetValueOrDefault(AppData.Settings.TrackingDeviceGuid, Vector3.Zero) +
                // Origin
                AppData.Settings.DeviceCalibrationOrigins
                    .GetValueOrDefault(AppData.Settings.TrackingDeviceGuid, Vector3.Zero),
            // Orientation
            AppData.Settings.DeviceCalibrationRotationMatrices
                .GetValueOrDefault(AppData.Settings.TrackingDeviceGuid, Quaternion.Identity) * jointOrientation);
    }

    // Log a message to Amethyst logs : handler
    public void Log(string message, LogSeverity severity = LogSeverity.Info, [CallerLineNumber] int lineNumber = 0,
        [CallerFilePath] string filePath = "", [CallerMemberName] string memberName = "")
    {
        switch (severity)
        {
            case LogSeverity.Fatal:
                Logger.Fatal($"[{Guid}] " + message, lineNumber, filePath, memberName);
                Logger.Fatal(new AggregateException($"[{Guid}] " + message));
                break;

            case LogSeverity.Error:
                Logger.Error($"[{Guid}] " + message, lineNumber, filePath, memberName);
                Logger.Fatal(new AggregateException($"[{Guid}] " + message));
                break;

            case LogSeverity.Warning:
                Logger.Warn($"[{Guid}] " + message, lineNumber, filePath, memberName);
                break;

            case LogSeverity.Info:
            default:
                Logger.Info($"[{Guid}] " + message, lineNumber, filePath, memberName);
                break;
        }
    }

    // Log a message to Amethyst logs : handler
    public void Log(object message, LogSeverity severity = LogSeverity.Info, [CallerLineNumber] int lineNumber = 0,
        [CallerFilePath] string filePath = "", [CallerMemberName] string memberName = "")
    {
        switch (severity)
        {
            case LogSeverity.Fatal:
                Logger.Fatal($"[{Guid}] " + message, lineNumber, filePath, memberName);
                break;

            case LogSeverity.Error:
                Logger.Error($"[{Guid}] " + message, lineNumber, filePath, memberName);
                break;

            case LogSeverity.Warning:
                Logger.Warn($"[{Guid}] " + message, lineNumber, filePath, memberName);
                break;

            case LogSeverity.Info:
            default:
                Logger.Info($"[{Guid}] " + message, lineNumber, filePath, memberName);
                break;
        }
    }

    // Play a custom sound from app resources
    public void PlayAppSound(SoundType sound)
    {
        AppSounds.PlayAppSound((AppSounds.AppSoundType)sound);
    }

    // Request a refresh of the status/name/etc. interface
    public void RefreshStatusInterface()
    {
        Shared.Main.DispatcherQueue.TryEnqueue(() =>
        {
            Logger.Info($"{Guid} requested an interface reload, reloading now!");
            Interfacing.Plugins.RefreshApplicationInterface();

            // ReSharper disable once InvertIf | Check if the request was from a device
            if (AppPlugins.TrackingDevicesList.TryGetValue(Guid, out var device) && device is not null)
                device.RefreshWatchHandlers(); // Re-register joint changes handlers

            // Check if used in any way: as the base, an override, or the endpoint
            if (!AppPlugins.IsBase(Guid) && !AppPlugins.IsOverride(Guid) &&
                AppPlugins.CurrentServiceEndpoint.Guid != Guid) return;

            Logger.Info($"{Guid} is currently being used, refreshing its config!");
            var locked = Monitor.TryEnter(UpdateLock); // Try entering the lock

            AppData.Settings.CheckSettings(); // Full check
            AppData.Settings.SaveSettings(); // Save it!
            if (locked) Monitor.Exit(UpdateLock); // Try entering the lock
        });
    }

    // Get Amethyst UI language
    public string LanguageCode => AppData.Settings.AppLanguage;

    // Get Amethyst Docs (web) language
    public string DocsLanguageCode => Interfacing.DocsLanguageCode;

    // Request a string from AME resources, empty for no match
    // Warning: The primarily searched resource is the device-provided one!
    public string RequestLocalizedString(string key)
    {
        return Interfacing.Plugins.RequestLocalizedString(key, Guid);
    }

    // Request a folder to be set as device's AME resources,
    // you can access these resources with the lower function later (after onLoad)
    // Warning: Resources are containerized and can't be accessed in-between devices!
    // Warning: The default root is "[device_folder_path]/resources/Strings"!
    public bool SetLocalizationResourcesRoot(string path)
    {
        return Interfacing.Plugins.SetLocalizationResourcesRoot(path, Guid);
    }

    // Show a Windows toast notification
    public void DisplayToast((string Title, string Text) message)
    {
        ShowToast(message.Title, message.Text);
    }

    // Request an application exit, non-fatal by default
    // Mark fatal as true to show the crash handler with your message
    public void RequestExit(string message, bool fatal = false)
    {
        Logger.Info($"Exit (fatal: {fatal}) requested by {Guid} with message \"{message}\"!");
        Shared.Main.DispatcherQueue.TryEnqueue(async () =>
        {
            // Launch the crash handler if fatal
            if (fatal)
            {
                var hPath = Path.Combine(ProgramLocation.DirectoryName!, "K2CrashHandler", "K2CrashHandler.exe");
                if (File.Exists(hPath)) Process.Start(hPath, new[] { "plugin_message", message, Guid });
                else Logger.Warn("Crash handler exe (./K2CrashHandler/K2CrashHandler.exe) not found!");
            }

            // Handle all the exit actions (if needed)
            if (!IsExitHandled)
                await HandleAppExit(1000);

            // Finally exit with code 0
            Environment.Exit(0);
        });
    }
}

public class LoadAttemptedPlugin : INotifyPropertyChanged
{
    private RestClient _githubClient;
    private bool _updateEnqueued;

    private RestClient GithubClient => _githubClient ??= new RestClient(
        UpdateEndpoint ?? $"{Website?.TrimEnd('/')}/releases/download/latest/");

    public string Name { get; init; } = "[UNKNOWN]";
    public string Guid { get; init; } = "[INVALID]";
    public string Error { get; init; }

    public string Publisher { get; init; }
    public string Website { get; init; }
    public string UpdateEndpoint { get; init; }
    public string Folder { get; init; }

    public Version Version { get; init; } = new("0.0.0.0");

    public bool UpdateFound => !_updateEnqueued && UpdateData.Found;

    public (bool Found, string Download, Version Version, string Changelog)
        UpdateData { get; private set; } = (false, null, null, null);

    public AppPlugins.PluginType PluginType { get; init; } =
        AppPlugins.PluginType.Unknown;

    // MVVM stuff
    public AppPlugins.PluginLoadError Status { get; init; } =
        AppPlugins.PluginLoadError.Unknown;

    public bool LoadError => Status is not
        AppPlugins.PluginLoadError.NoError and not
        AppPlugins.PluginLoadError.LoadingSkipped;

    public bool LoadSuccess => Status is
        AppPlugins.PluginLoadError.NoError or
        AppPlugins.PluginLoadError.LoadingSkipped;

    public bool IsLoaded
    {
        get => LoadSuccess && !AppData.Settings.DisabledPluginsGuidSet.Contains(Guid);
        set
        {
            if (IsLoaded == value) return; // No changes
            if (!Shared.Devices.PluginsPageOpened)
            {
                OnPropertyChanged();
                return; // Sanity check
            }

            // Disable/Enable this plugin
            if (value) AppData.Settings.DisabledPluginsGuidSet.Remove(Guid);
            else AppData.Settings.DisabledPluginsGuidSet.Add(Guid);

            // Check if the change is valid : tracking provider
            if (AppPlugins.TrackingDevicesList.ContainsKey(Guid) &&
                AppData.Settings.DisabledPluginsGuidSet.Contains(Guid))
            {
                SortedSet<string> loadedDeviceSet = new();

                // Check which devices are loaded : device plugin
                if (AppPlugins.TrackingDevicesList.ContainsKey("K2VRTEAM-AME2-APII-DVCE-DVCEKINECTV1"))
                    loadedDeviceSet.Add("K2VRTEAM-AME2-APII-DVCE-DVCEKINECTV1");
                if (AppPlugins.TrackingDevicesList.ContainsKey("K2VRTEAM-AME2-APII-DVCE-DVCEKINECTV2"))
                    loadedDeviceSet.Add("K2VRTEAM-AME2-APII-DVCE-DVCEKINECTV2");
                if (AppPlugins.TrackingDevicesList.ContainsKey("K2VRTEAM-AME2-APII-DVCE-DVCEPSMOVEEX"))
                    loadedDeviceSet.Add("K2VRTEAM-AME2-APII-DVCE-DVCEPSMOVEEX");
                if (AppPlugins.TrackingDevicesList.ContainsKey("K2VRTEAM-AME2-APII-DVCE-DVCEOWOTRACK"))
                    loadedDeviceSet.Add("K2VRTEAM-AME2-APII-DVCE-DVCEOWOTRACK");

                // If we've just disabled the last loaded device, re-enable the first
                if (AppPlugins.TrackingDevicesList.Keys.All(
                        AppData.Settings.DisabledPluginsGuidSet.Contains) ||

                    // If this entry happens to be the last one of the official ones
                    (loadedDeviceSet.Contains(Guid) && loadedDeviceSet.All(
                        AppData.Settings.DisabledPluginsGuidSet.Contains)))

                    // Re-enable this device if upper conditions are met
                    AppData.Settings.DisabledPluginsGuidSet.Remove(Guid);
            }

            // Check if the change is valid : service endpoint
            else if (AppPlugins.ServiceEndpointsList.ContainsKey(Guid) &&
                     AppData.Settings.DisabledPluginsGuidSet.Contains(Guid))
            {
                SortedSet<string> loadedServiceSet = new();

                // Check which services are loaded
                if (AppPlugins.ServiceEndpointsList.ContainsKey("K2VRTEAM-AME2-APII-SNDP-SENDPTOPENVR"))
                    loadedServiceSet.Add("K2VRTEAM-AME2-APII-SNDP-SENDPTOPENVR");
                if (AppPlugins.ServiceEndpointsList.ContainsKey("K2VRTEAM-AME2-APII-SNDP-SENDPTVRCOSC"))
                    loadedServiceSet.Add("K2VRTEAM-AME2-APII-SNDP-SENDPTVRCOSC");

                // If we've just disabled the last loaded service, re-enable the first
                if (AppPlugins.ServiceEndpointsList.Keys.All(
                        AppData.Settings.DisabledPluginsGuidSet.Contains) ||

                    // If this entry happens to be the last one of the official ones
                    (loadedServiceSet.Contains(Guid) && loadedServiceSet.All(
                        AppData.Settings.DisabledPluginsGuidSet.Contains)))

                    // Re-enable this service if upper conditions are met
                    AppData.Settings.DisabledPluginsGuidSet.Remove(Guid);
            }

            // Show the reload tip on any valid changes
            // == cause the upper check would make it different
            // and it's already been assigned at the beginning
            if (Shared.Devices.PluginsPageOpened && IsLoaded == value)
            {
                Shared.TeachingTips.MainPage.ReloadInfoBar.IsOpen = true;
                Shared.TeachingTips.MainPage.ReloadInfoBar.Opacity = 1.0;
            }

            // Save settings
            AppData.Settings.SaveSettings();
            OnPropertyChanged("IsLoaded");
        }
    }

    public string ErrorText => LocalizedJsonString($"/DevicesPage/Devices/Manager/Labels/{Status}");

    public string UpdateMessage => string.Format(
        LocalizedJsonString("/DevicesPage/Devices/Manager/Labels/UpdateMessage"),
        Name, (UpdateData.Version ?? Version).ToString());

    public bool PublisherValid => !string.IsNullOrEmpty(Publisher);
    public bool WebsiteValid => !string.IsNullOrEmpty(Website);
    public bool LocationValid => !string.IsNullOrEmpty(Folder);
    public bool GuidValid => !string.IsNullOrEmpty(Guid) && Guid is not "[INVALID]" or "INVALID";
    public bool ErrorValid => !string.IsNullOrEmpty(Error);
    public bool Uninstalling { get; set; }

    public bool CanUninstall =>
        !Uninstalling && LocationValid && GuidValid && Guid
            is not "K2VRTEAM-AME2-APII-DVCE-DVCEKINECTV1"
            and not "K2VRTEAM-AME2-APII-DVCE-DVCEKINECTV2"
            and not "K2VRTEAM-AME2-APII-DVCE-DVCEPSMOVEEX"
            and not "K2VRTEAM-AME2-APII-DVCE-DVCEOWOTRACK"
            and not "K2VRTEAM-AME2-APII-SNDP-SENDPTOPENVR"
            and not "K2VRTEAM-AME2-APII-SNDP-SENDPTVRCOSC";

    public event PropertyChangedEventHandler PropertyChanged;

    public async Task CheckUpdates()
    {
        try
        {
            // Fetch manifest details and content
            var manifestResponse = await GithubClient.GetAsync(
                new RestRequest("manifest.json"));

            if (!manifestResponse.IsSuccessStatusCode || manifestResponse.Content is null)
                throw new Exception(manifestResponse.ErrorMessage);

            var manifestResult = manifestResponse.Content.TryParseJson(out var manifest);
            if (!manifestResult || manifest is null)
                throw new Exception("The manifest was invalid!");

            // Parse the received manifest
            var remoteVersion = new Version(manifest["version"]?.ToString() ?? Version.ToString());
            UpdateData = (Version.CompareTo(remoteVersion) < 0, manifest["download"]?.ToString(),
                remoteVersion, manifest["changelog"]?.ToString());

            Shared.Main.DispatcherQueue.TryEnqueue(() =>
            {
                // Also show in MainWindow
                if (UpdateFound)
                {
                    Shared.Main.PluginsUpdateInfoBar.IsOpen = true;
                    Shared.Main.PluginsUpdateInfoBar.Opacity = 1.0;
                    Shared.Events.RefreshMainWindowEvent?.Set();
                }

                // Notify about updates
                OnPropertyChanged();
            });
        }
        catch (Exception e)
        {
            Logger.Info(e);
        }
    }

    internal void EnqueueUpdates(object sender, RoutedEventArgs e)
    {
        if (!UpdateFound) return;

        // Mark as ready to update
        _updateEnqueued = true;
        if (sender is Button button)
            button.IsEnabled = false;

        // Enqueue the update in the shutdown controller
        ShutdownController.ShutdownTasks.Add(new ShutdownTask
        {
            Name = $"Update plugin {Name}",
            Priority = true, // Either update right now or skip if it was manual
            Action = ExecuteUpdates
        });

        Shared.Main.DispatcherQueue.TryEnqueue(() =>
        {
            // Also show/hide the banner in MainWindow
            if (!AppPlugins.LoadAttemptedPluginsList.Any(x => x.UpdateFound))
            {
                // Hide if all plugins have their updates queued
                Shared.Main.PluginsUpdateInfoBar.Opacity = 0.0;
                Shared.Main.PluginsUpdateInfoBar.IsOpen = false;
                Shared.Events.RefreshMainWindowEvent?.Set();
            }

            // Notify about updates
            OnPropertyChanged();
        });
    }

    public async Task<bool> ExecuteUpdates()
    {
        // Logic:
        // - Download the zip to where out plugin is, unpack
        // - Rename the zipped plugin to $({Folder}.Name).next.zip
        // After restart:
        // - Search for plugin zips matching $({Folder}.Name).next.zip
        // - Delete the old plugin and unpack the new one to root

        try
        {
            // Mark the update footer as active
            Shared.Main.PluginsUpdatePendingInfoBar.Title = string.Format(LocalizedJsonString(
                "/SharedStrings/PluginUpdates/Headers/Downloading"), Name, UpdateData.Version.ToString());

            Shared.Main.PluginsUpdatePendingInfoBar.Message = LocalizedJsonString(
                "/SharedStrings/PluginUpdates/Headers/Preparing");

            Shared.Main.PluginsUpdatePendingInfoBar.IsOpen = true;
            Shared.Main.PluginsUpdatePendingInfoBar.Opacity = 1.0;

            Shared.Main.PluginsUpdatePendingProgressBar.IsIndeterminate = true;
            Shared.Main.PluginsUpdatePendingProgressBar.ShowError = false;
            Shared.Events.RefreshMainWindowEvent?.Set();

            if (string.IsNullOrEmpty(UpdateData.Download) || !LocationValid)
            {
                // No files to download, switch to update error
                Shared.Main.PluginsUpdatePendingInfoBar.Message = string.Format(LocalizedJsonString(
                    "/SharedStrings/PluginUpdates/Statuses/Error"), Name);

                Shared.Main.PluginsUpdatePendingProgressBar.IsIndeterminate = true;
                Shared.Main.PluginsUpdatePendingProgressBar.ShowError = true;

                await Task.Delay(2500);
                Shared.Main.PluginsUpdatePendingInfoBar.Opacity = 0.0;
                Shared.Main.PluginsUpdatePendingInfoBar.IsOpen = false;
                Shared.Events.RefreshMainWindowEvent?.Set();

                await Task.Delay(1000);
                return false; // That's all
            }

            // Try downloading the plugin archive from the manifest
            Logger.Info($"Downloading the next {Name} plugin assuming it's under {UpdateData.Download}");
            try
            {
                // Update the progress message
                Shared.Main.PluginsUpdatePendingInfoBar.Message = string.Format(LocalizedJsonString(
                    "/SharedStrings/PluginUpdates/Statuses/Downloading"), Name, UpdateData.Version.ToString());

                // Create a stream reader using the received Installer Uri
                await using var stream = await GithubClient.DownloadStreamAsync(new RestRequest(UpdateData.Download));

                // Replace or create our installer file
                var pluginArchive = await (await StorageFolder
                    .GetFolderFromPathAsync(Folder)).CreateFileAsync(
                    $"{UpdateData.Download}", CreationCollisionOption.ReplaceExisting);

                // Create an output stream and push all the available data to it
                await using var fsPluginArchive = await pluginArchive.OpenStreamForWriteAsync();
                await stream!.CopyToAsync(fsPluginArchive); // The runtime will do the rest for us
            }
            catch (Exception e)
            {
                Logger.Error(new Exception($"Error downloading the plugin! Message: {e.Message}"));

                // No files to download, switch to update error
                Shared.Main.PluginsUpdatePendingInfoBar.Message = string.Format(LocalizedJsonString(
                    "/SharedStrings/PluginUpdates/Statuses/Error"), Name);

                Shared.Main.PluginsUpdatePendingProgressBar.IsIndeterminate = true;
                Shared.Main.PluginsUpdatePendingProgressBar.ShowError = true;

                await Task.Delay(2500);
                Shared.Main.PluginsUpdatePendingInfoBar.Opacity = 0.0;
                Shared.Main.PluginsUpdatePendingInfoBar.IsOpen = false;
                Shared.Events.RefreshMainWindowEvent?.Set();

                await Task.Delay(1000);
                return false; // That's all
            }

            // Rename the downloaded zip to $({Folder}.Name).next.zip
            var updateZip = Path.Join(Folder, $"{new DirectoryInfo(Folder).Name}.next.zip");
            File.Move(Path.Join(Folder, UpdateData.Download), updateZip, true);

            // Register the update in the Amethyst task scheduler
            StartupController.Controller.StartupTasks.Add(new StartupUpdateTask
            {
                Name = $"Update {Name} to version {UpdateData.Version}",
                Priority = true, PluginFolder = Folder, UpdatePackage = updateZip
            });

            // Everything's fine, show the restart notice
            Shared.Main.PluginsUpdatePendingInfoBar.Message = string.Format(LocalizedJsonString(
                "/SharedStrings/PluginUpdates/Headers/Restart"), Name);

            Shared.Main.PluginsUpdatePendingProgressBar.IsIndeterminate = false;
            Shared.Main.PluginsUpdatePendingProgressBar.ShowError = false;

            await Task.Delay(2500);
            Shared.Main.PluginsUpdatePendingInfoBar.Opacity = 0.0;
            Shared.Main.PluginsUpdatePendingInfoBar.IsOpen = false;
            Shared.Events.RefreshMainWindowEvent?.Set();

            // Still here? We're done!
            await Task.Delay(1000);
            return true; // That's all
        }
        catch (Exception e)
        {
            Logger.Info(e);

            // No files to download, switch to update error
            Shared.Main.PluginsUpdatePendingInfoBar.Message = string.Format(LocalizedJsonString(
                "/SharedStrings/PluginUpdates/Statuses/Error"), Name);

            Shared.Main.PluginsUpdatePendingProgressBar.IsIndeterminate = true;
            Shared.Main.PluginsUpdatePendingProgressBar.ShowError = true;

            await Task.Delay(2500);
            Shared.Main.PluginsUpdatePendingInfoBar.Opacity = 0.0;
            Shared.Main.PluginsUpdatePendingInfoBar.IsOpen = false;
            Shared.Events.RefreshMainWindowEvent?.Set();

            await Task.Delay(1000);
            return false; // That's all
        }
    }

    public void EnqueuePluginUninstall()
    {
        // Enqueue a delete startup action to uninstall this plugin
        StartupController.Controller.StartupTasks.Add(new StartupDeleteTask
        {
            Name = $"Delete plugin {Name} v{Version}",
            PluginFolder = Folder
        });

        // Show a badge that this plugin will be uninstalled
        Uninstalling = true;
        OnPropertyChanged();
    }

    public string TrimString(string s, int l)
    {
        return s?[..Math.Min(s.Length, l)] +
               (s?.Length > l ? "..." : "");
    }

    public void ShowDeviceFolder()
    {
        SystemShell.OpenFolderAndSelectItem(Folder);
    }

    public async void OpenDeviceWebsite()
    {
        try
        {
            await Launcher.LaunchUriAsync(new Uri(Website));
        }
        catch (Exception)
        {
            // ignored
        }
    }

    public double BoolToOpacity(bool value)
    {
        return value ? 1.0 : 0.0;
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }
}

public class AppTrackerEntry
{
    public TrackerType TrackerRole { get; set; } = TrackerType.TrackerHanded;
    public string Name => LocalizedJsonString($"/SharedStrings/Joints/Enum/{(int)TrackerRole}");
    public bool IsEnabled => AppData.Settings.TrackersVector.Any(x => x.Role == TrackerRole);
}

public static class CollectionExtensions
{
    public static bool AddPlugin<T>(this ICollection<T> collection, DirectoryInfo item) where T : ComposablePartCatalog
    {
        // Give up if the plugin directory is invalid
        if (item is null) return false;

        // Test if the directory is valid
        try
        {
            // This will fail if e.g.
            // - the "directory" is an invalid junction
            // - the "directory" is an invalid symlink
            // - the "directory" is an unknown object
            item.GetFiles();
        }
        catch (Exception e)
        {
            // Add the plugin to the 'attempted' list
            AppPlugins.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
            {
                Name = item.FullName,
                Error = e.Message,
                Folder = item.FullName,
                Status = AppPlugins.PluginLoadError.NoPluginFolder
            });

            return false; // Don't do anything stupid
        }

        // Delete the vendor plugin contract, just in case
        item.GetFiles("Amethyst.Plugins.Contract.dll").FirstOrDefault()?.Delete();
        item.GetFiles("Microsoft.Windows.SDK.NET.dll").FirstOrDefault()?.Delete();
        item.GetFiles("Microsoft.WinUI.dll").FirstOrDefault()?.Delete();
        item.GetFiles("WinRT.Runtime.dll").FirstOrDefault()?.Delete();

        // Zero possible plugin files to be loaded
        if (item.GetFiles("plugin*.dll").Length <= 0)
        {
            // Add the plugin to the 'attempted' list
            AppPlugins.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
            {
                Name = item.Name, Folder = item.FullName,
                Status = AppPlugins.PluginLoadError.NoPluginDll
            });

            Logger.Error(new FileNotFoundException(
                $"Plugin hint directory \"{item.FullName}\" doesn't contain " +
                "any loadable plugin library files (must start with 'plugin')!"));

            return false; // Nah, not this time
        }

        // Loop over all the files, load into a separate appdomain/context
        foreach (var fileInfo in item.GetFiles("plugin*.dll"))
            try
            {
                var loadContext = new ModuleAssemblyLoadContext(fileInfo.FullName);
                var assemblyFile = loadContext.LoadFromAssemblyPath(fileInfo.FullName);
                var assemblyCatalog = new AssemblyCatalog(assemblyFile);

                // Check if it's the plugin we're searching for
                if (!assemblyCatalog.Parts.Any(x => x.ExportDefinitions
                        .Any(y => y.ContractName == typeof(ITrackingDevice).FullName ||
                                  y.ContractName == typeof(IServiceEndpoint).FullName))) continue;

                collection.Add((T)(object)assemblyCatalog);
                return true; // This plugin is probably supported, yay!
            }
            catch (CompositionException e)
            {
                Crashes.TrackError(e); // Composition exception
                if (fileInfo.Name.StartsWith("plugin"))
                    Logger.Error($"Loading {fileInfo} failed with a composition exception: " +
                                 $"Message: {e.Message}\nErrors occurred: {e.Errors}\nPossible causes: {e.RootCauses}");
                else
                    Logger.Warn($"[Non-critical] Loading {fileInfo} failed with a composition exception: " +
                                $"Message: {e.Message}\nErrors occurred: {e.Errors}\nPossible causes: {e.RootCauses}");

                // Add the plugin to the 'attempted' list
                AppPlugins.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
                {
                    Name = $"{item.Name}/{fileInfo.Name}",
                    Error = $"{e.Message}\n\n{e.StackTrace}", Folder = item.FullName,
                    Status = AppPlugins.PluginLoadError.NoPluginDll
                });

                return false; // Nah, not this time
            }
            catch (Exception e)
            {
                if ((e as ReflectionTypeLoadException)?.LoaderExceptions.First() is not
                    FileNotFoundException) Crashes.TrackError(e); // Only send unknown exceptions

                if (fileInfo.Name.StartsWith("plugin"))
                    Logger.Error($"Loading {fileInfo} failed with an exception: Message: {e.Message} " +
                                 "Probably some assembly referenced by this plugin is missing.");
                else
                    Logger.Warn($"[Non-critical] Loading {fileInfo} failed with an exception: {e.Message}");

                // Add the plugin to the 'attempted' list
                AppPlugins.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
                {
                    Name = $"{item.Name}/{fileInfo.Name}",
                    Error = $"{e.Message}\n\n{e.StackTrace}", Folder = item.FullName,
                    Status = AppPlugins.PluginLoadError.NoPluginDll
                });

                return false; // Nah, not this time
            }

        return false; // Nah, not this time
    }
}