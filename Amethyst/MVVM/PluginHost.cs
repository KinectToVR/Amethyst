using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.ComponentModel.Composition;
using System.ComponentModel.Composition.Hosting;
using System.ComponentModel.Composition.Primitives;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.Runtime.Serialization;
using System.Threading;
using System.Threading.Tasks;
using System.Web;
using Windows.Storage;
using Amethyst.Classes;
using Amethyst.Installer.ViewModels;
using Amethyst.Plugins.Contract;
using Amethyst.Schedulers;
using Amethyst.Utils;
using AmethystSupport;
using Microsoft.AppCenter.Crashes;
using Microsoft.UI.Xaml;
using Newtonsoft.Json;
using RestSharp;
using static Amethyst.Classes.Interfacing;

namespace Amethyst.MVVM;

[method: SetsRequiredMembers]
public class PluginHost(string guid) : IAmethystHost
{
    // Cache the guid and rebuild
    // Internal plugin guid
    private string Guid { get; } = guid;

    // Settings helper : read/write plugins settings
    private PluginSettingsHelper SettingsHelper { get; } = new(guid);

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

    // Check if a tracker with the specified role is enabled and active
    public bool IsTrackerEnabled(TrackerType trackerType)
    {
        return AppData.Settings.TrackersVector
            .Any(x => x.Role == trackerType && x.IsActive);
    }

    // Lock the main update loop while in scope with [lock (UpdateThreadLock) { }]
    public object UpdateThreadLock => UpdateLock;

    // Get the hook joint pose (typically Head, fallback to .First())
    public (Vector3 Position, Quaternion Orientation) GetHookJointPose(bool calibrated = false)
    {
        var (jointPosition, jointOrientation) = DeviceHookJointPosition
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
        var (jointPosition, jointOrientation) = DeviceRelativeTransformOrigin
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
                AppPlugins.CurrentServiceEndpoint.Guid != Guid &&
                Guid is not "K2VRTEAM-AME2-APII-DVCE-TRACKINGRELAY") return;

            Logger.Info($"{Guid} is currently being used, refreshing its config!");
            var locked = Monitor.TryEnter(UpdateLock); // Try entering the lock

            AppData.Settings.CheckSettings(); // Full check
            AppData.Settings.SaveSettings(); // Save it!
            if (locked) Monitor.Exit(UpdateLock); // Try entering the lock
        });

        try
        {
            // Check if the relay plugin is there
            if (!AppPlugins.ServiceEndpointsList.TryGetValue("K2VRTEAM-AME2-APII-DVCE-TRACKINGRELAY", out var relay)) return;
            Logger.Info("Reloading additional devices from Amethyst Tracking Relay...");

            // Check for backfeed configuration not to "reload" ourselves away
            var isBackfeedProperty = relay.Service.GetType().GetProperty("IsBackfeed");
            if (isBackfeedProperty is null || !isBackfeedProperty.CanRead) return;
            if (isBackfeedProperty.GetValue(relay.Service) as bool? ?? false) return;

            // Get the reload property and tell the client to refresh itself
            var requestReloadProperty = relay.Service.GetType().GetProperty("RequestReload");
            if (requestReloadProperty is null || !requestReloadProperty.CanRead) return;
            ((Action<CancellationToken>)requestReloadProperty.GetValue(relay.Service))?.Invoke(default);
        }
        catch (Exception e)
        {
            Logger.Warn(e);
        }
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
            if (fatal) await $"amethyst-app:crash-message#{HttpUtility.UrlEncode(message)}".ToUri().LaunchAsync();

            // Handle all the exit actions (if needed)
            if (!IsExitHandled) await HandleAppExit(1000);

            // Finally exit with code 0
            Environment.Exit(0);
        });
    }

    // Process a key input action called from a single joint
    // The handler will check whether the action is used anywhere,
    // and trigger the linked output action if applicable
    public void ReceiveKeyInput(IKeyInputAction action, object data)
    {
        try
        {
            // Invoke all linked input actions
            AppData.Settings.TrackersVector
                .SelectMany(x => x.InputActionsMap.Where(y => y.Value?.Guid == action.Guid)
                    .Select(y => (Tracker: x.Role, Action: y.Key.LinkedAction))).ToList()
                .ForEach(x => AppPlugins.CurrentServiceEndpoint.ProcessKeyInput(x.Action, data, x.Tracker));

            // Update testing input if available
            if (InputActionBindingEntry.TreeCurrentAction is not null &&
                InputActionBindingEntry.TreeCurrentAction.Device == Guid &&
                InputActionBindingEntry.TreeCurrentAction.Guid == action.Guid)
                InputActionBindingEntry.TreeCurrentAction.TestValue = data?.ToString();
        }
        catch (Exception e)
        {
            Logger.Warn(e);
        }
    }

    // Check whether a KeyInputAction is used for anything
    // Devices may use this to skip updating unused actions
    public bool CheckInputActionIsUsed(IKeyInputAction action)
    {
        return AppData.Settings.TrackersVector
            .Any(x => x.InputActionsMap.Any(y => y.Value?.Guid == action.Guid));
    }

    // INTERNAL: Available only via reflection, not defined in the Host interface
    // Return all devices added from plugins, INCLUDING forwarded ones
    public Dictionary<string, ITrackingDevice> TrackingDevices =>
        AppPlugins.TrackingDevicesList.ToDictionary(pair => pair.Key, pair => pair.Value.Device);

    // INTERNAL: Available only via reflection, not defined in the Host interface
    // Return the device's name, based on its GUID string
    public string GetDeviceName(string deviceGuid)
    {
        return AppPlugins.TrackingDevicesList.Values.FirstOrDefault(x => x.Guid == deviceGuid, null)?.Name;
    }

    // INTERNAL: Available only via reflection, not defined in the Host interface
    // Tells the application to reload remote devices, MUST be called from the relay
    public void ReloadRemoteDevices()
    {
        AppPlugins.ReloadRemoteDevices();
    }

    // INTERNAL: Available only via reflection, not defined in the Host interface
    // Override the Relay InfoBar title, content, and button data
    // Passing null will reset the bar, and if there's no bar, passing data will show it
    public void SetRelayInfoBarOverride((string Title, string Content, string Button, Action Click, bool Closable)? infoBarData)
    {
        RelayBarOverride = infoBarData;
        Shared.Events.RefreshMainWindowEvent?.Set();
    }
}

[method: SetsRequiredMembers]
public class CoreHost(string guid) : IDependencyInstaller.ILocalizationHost
{
    // Cache the guid and rebuild
    // Internal plugin guid
    private string Guid { get; } = guid;

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

    public string DependencyLink { get; init; }
    public string DependencySource { get; init; }

    public IDependencyInstaller DependencyInstaller { get; init; }
    public DependencyInstallHandler InstallHandler { get; } = new();

    public (LocalisationFileJson Root, string Directory) LocalizationResourcesRoot { get; set; }

    public Uri DependencyLinkUri =>
        !string.IsNullOrEmpty(DependencyLink) && Uri.TryCreate(DependencyLink, UriKind.RelativeOrAbsolute, out var uri) ? uri :
        Uri.TryCreate("https://k2vr.tech", UriKind.RelativeOrAbsolute, out var uri1) ? uri1 : null;

    public Uri DependencySourceUri =>
        !string.IsNullOrEmpty(DependencySource) && Uri.TryCreate(DependencySource, UriKind.RelativeOrAbsolute, out var uri) ? uri :
        Uri.TryCreate("https://k2vr.tech", UriKind.RelativeOrAbsolute, out var uri1) ? uri1 : null;

    public Uri WebsiteUri =>
        !string.IsNullOrEmpty(Website) && Uri.TryCreate(Website, UriKind.RelativeOrAbsolute, out var uri) ? uri :
        Uri.TryCreate("https://k2vr.tech", UriKind.RelativeOrAbsolute, out var uri1) ? uri1 : null;

    public Version Version { get; init; } = new("0.0.0.0");

    public bool UpdateFound => UpdateData.Found && !Uninstalling;
    public bool UpdateAvailable => UpdateFound && _updateEnqueued;

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
            if (CurrentAppState != "plugins")
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
            if (CurrentAppState == "plugins" && IsLoaded == value)
            {
                Shared.TeachingTips.MainPage.ReloadInfoBar.IsOpen = true;
                Shared.TeachingTips.MainPage.ReloadInfoBar.Opacity = 1.0;

                AppSounds.PlayAppSound(value
                    ? AppSounds.AppSoundType.ToggleOn
                    : AppSounds.AppSoundType.ToggleOff);
            }

            // Save settings
            AppData.Settings.SaveSettings();
            OnPropertyChanged("IsLoaded");
        }
    }

    public string ErrorText => LocalizedJsonString($"/DevicesPage/Devices/Manager/Labels/{Status}");

    public string UpdateMessage =>
        LocalizedJsonString("/DevicesPage/Devices/Manager/Labels/UpdateMessage")
            .Format(Name, (UpdateData.Version ?? Version).ToString());

    public bool PublisherValid => !string.IsNullOrEmpty(Publisher);
    public bool WebsiteValid => !string.IsNullOrEmpty(Website);
    public bool LocationValid => !string.IsNullOrEmpty(Folder);
    public bool GuidValid => !string.IsNullOrEmpty(Guid) && Guid is not "[INVALID]" and not "INVALID";
    public bool ErrorValid => !string.IsNullOrEmpty(Error);
    public bool Uninstalling { get; set; }

    public bool DependencyLinkValid => !string.IsNullOrEmpty(DependencyLink);
    public bool DependencySourceValid => !string.IsNullOrEmpty(DependencySource);
    public bool DependencyLinksValid => DependencyLinkValid && DependencySourceValid;

    public bool ShowDependencyInstaller => DependencyInstaller is not null && !SetupData.LimitedHide;

    public bool ShowDependencyLinks =>
        !ShowDependencyInstaller && (DependencyLinkValid || DependencySourceValid);

    public bool LoadErrorDepMissing =>
        Status is AppPlugins.PluginLoadError.NoPluginDll or AppPlugins.PluginLoadError.NoPluginDependencyDll;

    public bool CanUninstall =>
        !IsExitPending && !Uninstalling &&
        LocationValid && GuidValid && !Guid.IsProtectedGuid();

    public CornerRadius ExpanderThickness => LoadError
        ? new CornerRadius(4, 4, 0, 0)
        : new CornerRadius(4);

    public string DependencyAdditionalLinksText => DependencyLinkValid || DependencySourceValid
        ? LocalizedJsonString("/SharedStrings/Plugins/Dep/Contents/AdditionalLinks")
        : string.Empty;

    public string DependencyDocsLinkText => DependencyLinkValid
        ? LocalizedJsonString("/SharedStrings/Plugins/Dep/Contents/PluginDocs")
        : string.Empty;

    public string DependencySeparatorText => DependencyLinksValid
        ? LocalizedJsonString("/SharedStrings/Plugins/Dep/Contents/Separator")
        : string.Empty;

    public string DependencyDownloadLinkText => DependencySourceValid
        ? LocalizedJsonString("/SharedStrings/Plugins/Dep/Contents/Download")
        : string.Empty;

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

            // Try downloading the update if found
            if (UpdateFound)
            {
                Logger.Info($"Trying to update {Name} to {UpdateData.Version}! Trying to download it now...");
                _updateEnqueued = await ExecuteUpdates(); // Try to execute basic updates, mark as queued if succeeded
            }

            // Show the update notice
            Shared.Main.DispatcherQueue.TryEnqueue(() =>
            {
                // Also show in MainWindow
                if (UpdateAvailable)
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

    internal async void ExecuteAppRestart(object sender, RoutedEventArgs e)
    {
        Logger.Info($"[{Name}->{UpdateData.Version}] Update invoked: trying to restart the app...");
        await Interfacing.ExecuteAppRestart(); // Try restarting ame
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
            if (string.IsNullOrEmpty(UpdateData.Download) || !LocationValid)
            {
                Logger.Error($"[{Name}->{UpdateData.Version}] No files to download, aborting!");
                return false; // That's all
            }

            // Try downloading the plugin archive from the manifest
            Logger.Info(
                $"[{Name}->{UpdateData.Version}] Downloading the next {Name} plugin assuming it's under {UpdateData.Download}");
            try
            {
                // Create a stream reader using the received Installer Uri
                Logger.Info($"[{Name}->{UpdateData.Version}] Preparing update streams...");
                await using var stream = await GithubClient.DownloadStreamAsync(new RestRequest(UpdateData.Download));

                // Replace or create our installer file
                Logger.Info($"[{Name}->{UpdateData.Version}] Creating the plugin archive...");
                var pluginArchive = await (await StorageFolder
                    .GetFolderFromPathAsync(Folder)).CreateFileAsync(
                    $"{UpdateData.Download}", CreationCollisionOption.ReplaceExisting);

                // Create an output stream and push all the available data to it
                Logger.Info($"[{Name}->{UpdateData.Version}] Opening the plugin archive now...");
                await using var fsPluginArchive = await pluginArchive.OpenStreamForWriteAsync();

                Logger.Info($"[{Name}->{UpdateData.Version}] Trying to copy the download-able data...");
                await stream!.CopyToAsync(fsPluginArchive); // The runtime will do the rest for us
            }
            catch (Exception e)
            {
                Logger.Error($"[{Name}->{UpdateData.Version}] Error downloading the plugin! Message: {e.Message}");
                return false; // That's all
            }

            // Rename the downloaded zip to $({Folder}.Name).next.zip
            Logger.Info($"[{Name}->{UpdateData.Version}] Renaming the plugin zip package...");
            var updateZip = Path.Join(Folder, $"{new DirectoryInfo(Folder).Name}.next.zip");
            File.Move(Path.Join(Folder, UpdateData.Download), updateZip, true);

            // Register the update in the Amethyst task scheduler
            Logger.Info($"[{Name}->{UpdateData.Version}] Scheduling a startup update task...");
            StartupController.Controller.StartupTasks.Add(new StartupUpdateTask
            {
                Name = $"Update {Name} to version {UpdateData.Version}",
                Priority = true, PluginFolder = Folder, UpdatePackage = updateZip
            });

            Logger.Info($"[{Name}->{UpdateData.Version}] Looks like that's all for now...");
            return true; // That's all
        }
        catch (Exception e)
        {
            Logger.Error(e);
            return false; // That's all
        }
    }

    public void EnqueuePluginUninstall()
    {
        // Enqueue a delete startup action to uninstall this plugin
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        Logger.Info($"Queueing a startup delete task for {Name} from {Folder}...");
        StartupController.Controller.StartupTasks.Add(
            new StartupDeleteTask
            {
                Name = $"Delete plugin {Name} v{Version}",
                Data = Folder + Guid + Version,
                PluginFolder = Folder
            });

        // Show a badge that this plugin will be uninstalled
        Uninstalling = true;

        // Show the update notice
        OnPropertyChanged(); // Refresh everything
        Shared.Main.DispatcherQueue.TryEnqueue(() =>
        {
            // Search for any pending plugin install task
            var uninstallsPending = StartupController.Controller.StartupTasks
                .Any(x => x.Name.StartsWith("Delete"));

            // Also show in MainWindow
            Shared.Main.PluginsUninstallInfoBar.IsOpen = uninstallsPending;
            Shared.Main.PluginsUninstallInfoBar.Opacity = BoolToOpacity(uninstallsPending);
            Shared.Events.RefreshMainWindowEvent?.Set();

            // Notify about updates
            OnPropertyChanged();
        });
    }

    public void CancelPluginUninstall()
    {
        // Delete the uninstall startup action
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
        StartupController.Controller.StartupTasks.Remove(
            StartupController.Controller.StartupTasks
                .FirstOrDefault(x => x.Data == Folder + Guid + Version));

        // Hide the uninstall badge
        Uninstalling = false;

        // Show the update notice
        OnPropertyChanged(); // Refresh everything
        Shared.Main.DispatcherQueue.TryEnqueue(() =>
        {
            // Search for any pending plugin install task
            var uninstallsPending = StartupController.Controller.StartupTasks
                .Any(x => x.Name.StartsWith("Delete"));

            // Also show in MainWindow
            Shared.Main.PluginsUninstallInfoBar.IsOpen = uninstallsPending;
            Shared.Main.PluginsUninstallInfoBar.Opacity = BoolToOpacity(uninstallsPending);
            Shared.Events.RefreshMainWindowEvent?.Set();

            // Notify about updates
            OnPropertyChanged();
        });
    }

    public async void InstallPluginDependencies(object sender, RoutedEventArgs e)
    {
        // Show the EULA, if provided by the installer
        foreach (var dependency in DependencyInstaller?.ListDependencies()?
                     .Where(x => !x.IsInstalled && x.IsMandatory &&
                                 !string.IsNullOrEmpty(x.InstallerEula)))
        {
            Shared.Main.EulaHeader.Text = $"{dependency.Name} EULA";
            Shared.Main.EulaText.Text = dependency.InstallerEula;

            Shared.Main.EulaFlyout.ShowAt(Shared.Main.MainGrid);
            await Shared.Main.EulaFlyoutClosed.WaitAsync();

            // Validate the result and continue/exit
            if (!Shared.Main.EulaFlyoutResult) return;
        }

        try
        {
            // Install plugin dep using the installer
            await PerformDependencyInstallation();
        }
        catch (Exception ex)
        {
            Logger.Error(ex);
        }
    }

    public void CancelDependencyInstallation()
    {
        try
        {
            InstallHandler?.TokenSource?.Cancel();
        }
        catch (Exception e)
        {
            Logger.Error(e);
        }
    }

    private async Task PerformDependencyInstallation()
    {
        /*
         * Logic
         * - Validate the DependencyInstaller
         * - Show the installation grid
         * - Disable all user input
         *
         * Success
         * - Show the last message, progress 100%S
         * - Change to 'success', wait 6s
         *
         * Failure
         * - Show the last message, progress 100%F
         * - Wait 6s, change to 'failure', wait 5s
         */

        /* Show the installer part */

        // Prepare a list of our dependencies
        var dependenciesToInstall = DependencyInstaller?.ListDependencies()
            .Where(x => !x.IsInstalled && x.IsMandatory).ToList();

        // Theoretically not possible, but check anyway
        if (dependenciesToInstall is null || !dependenciesToInstall.Any()) return;
        InstallHandler.TokenSource = new CancellationTokenSource();
        InstallHandler.DependencyName = dependenciesToInstall.First().Name;

        // Block temporarily
        InstallHandler.AllowUserInput = false;
        InstallHandler.OnPropertyChanged();
        await Task.Delay(500, InstallHandler.TokenSource.Token);

        // Show the installer grid
        InstallHandler.InstallationWorker = null;
        InstallHandler.InstallingDependencies = true;
        InstallHandler.ProgressError = false;
        InstallHandler.ProgressIndeterminate = true;
        InstallHandler.ProgressValue = 0.0;
        InstallHandler.StageName = string.Empty;
        InstallHandler.HideProgress = false;
        InstallHandler.OnPropertyChanged();

        // Unblock user input now
        await Task.Delay(500, InstallHandler.TokenSource.Token);
        InstallHandler.AllowUserInput = true;
        InstallHandler.OnPropertyChanged();

        // Loop over all dependencies and install them, give up on failures
        foreach (var dependency in dependenciesToInstall)
            try
            {
                /* Setup and start the installation */

                // Prepare the progress update handler
                var progress = new Progress<InstallationProgress>();
                InstallHandler.DependencyName = dependency.Name;
                InstallHandler.OnPropertyChanged(); // The name

                progress.ProgressChanged += (_, installationProgress) =>
                    Shared.Main.DispatcherQueue.TryEnqueue(() =>
                    {
                        // Update our progress here
                        InstallHandler.StageName = installationProgress.StageTitle;
                        InstallHandler.ProgressIndeterminate = installationProgress.IsIndeterminate;
                        InstallHandler.ProgressValue = installationProgress.OverallProgress * 100 ?? 0;

                        // Trigger a partial interface reload
                        InstallHandler.OnPropertyChanged();
                    });

                // Capture the installation thread
                InstallHandler.InstallationWorker = dependency.Install(progress, InstallHandler.TokenSource.Token);

                // Actually start the installation now
                var result = await InstallHandler.InstallationWorker;

                /* Parse the result and present it */

                // Show the progress indicator [and the last message if failed]
                InstallHandler.ProgressError = !result;
                InstallHandler.ProgressIndeterminate = false;
                InstallHandler.ProgressValue = 100;
                InstallHandler.HideProgress = true;

                if (result)
                    InstallHandler.StageName =
                        LocalizedJsonString("/SharedStrings/Plugins/Dep/Contents/Success")
                            .Format(InstallHandler.DependencyName);

                InstallHandler.OnPropertyChanged();
                await Task.Delay(6000, InstallHandler.TokenSource.Token);

                if (!result)
                {
                    InstallHandler.StageName =
                        LocalizedJsonString("/SharedStrings/Plugins/Dep/Contents/Failure")
                            .Format(InstallHandler.DependencyName);

                    InstallHandler.OnPropertyChanged();
                    await Task.Delay(5000, InstallHandler.TokenSource.Token);
                }

                // Block temporarily
                InstallHandler.AllowUserInput = false;
                InstallHandler.OnPropertyChanged();
                await Task.Delay(500, InstallHandler.TokenSource.Token);

                // Hide the installer grid
                InstallHandler.InstallingDependencies = false;
                InstallHandler.OnPropertyChanged();
                await Task.Delay(500, InstallHandler.TokenSource.Token);

                // Unblock user input now
                InstallHandler.AllowUserInput = true;
                InstallHandler.OnPropertyChanged();

                /* Show the restart notice */
                if (!result)
                {
                    InstallHandler.TokenSource.Dispose();
                    return; // Exit the whole handler
                }

                if (dependenciesToInstall.Last() == dependency)
                    Shared.Main.DispatcherQueue.TryEnqueue(() =>
                    {
                        Shared.TeachingTips.MainPage.ReloadInfoBar.IsOpen = true;
                        Shared.TeachingTips.MainPage.ReloadInfoBar.Opacity = 1.0;
                    });
            }
            catch (OperationCanceledException e)
            {
                // Show the fail information
                InstallHandler.StageName = LocalizedJsonString(
                    "/SharedStrings/Plugins/Dep/Contents/Cancelled").Format(e.Message);

                InstallHandler.ProgressError = true;
                InstallHandler.HideProgress = true;
                InstallHandler.ProgressValue = 100;

                InstallHandler.OnPropertyChanged();
                await Task.Delay(5000);

                // Hide the installer grid and 'install'
                InstallHandler.InstallingDependencies = false;
                InstallHandler.OnPropertyChanged();
                await Task.Delay(500);

                // Unblock user input now
                InstallHandler.AllowUserInput = true;
                InstallHandler.OnPropertyChanged();
            }
            catch (Exception ex)
            {
                // Show the fail information
                InstallHandler.StageName = LocalizedJsonString(
                    "/SharedStrings/Plugins/Dep/Contents/InternalException").Format(ex.Message);

                InstallHandler.ProgressError = true;
                InstallHandler.HideProgress = true;
                InstallHandler.ProgressValue = 100;

                InstallHandler.OnPropertyChanged();
                await Task.Delay(5000);

                // Hide the installer grid and 'install'
                InstallHandler.InstallingDependencies = false;
                InstallHandler.OnPropertyChanged();
                await Task.Delay(500);

                // Unblock user input now
                InstallHandler.AllowUserInput = true;
                InstallHandler.OnPropertyChanged();
            }

        // Clean up after installation
        InstallHandler.TokenSource.Dispose();
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

    public double BoolToOpacity(bool value)
    {
        return value ? 1.0 : 0.0;
    }

    public string FormatResourceString(string resourceName)
    {
        return LocalizedJsonString(resourceName).Format(Name);
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    public void PlayExpandingSound()
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    public void PlayCollapsingSound()
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }

    public class DependencyInstallHandler : INotifyPropertyChanged
    {
        private double _progressValue;
        public CancellationTokenSource TokenSource { get; set; } = new();
        public Task<bool> InstallationWorker { get; set; }

        public bool InstallingDependencies { get; set; }
        public bool DependenciesReadyToInstall => !InstallingDependencies;
        public bool AllowUserInput { get; set; } = true;

        public bool ProgressError { get; set; }
        public bool ProgressIndeterminate { get; set; }
        public bool HideProgress { get; set; }
        public bool NoProgress { get; set; }

        public double ProgressValue
        {
            get => Math.Clamp(_progressValue, 0, 100);
            set => _progressValue = value;
        }

        public string StageName { get; set; }
        public string DependencyName { get; set; }

        public string MessageString => string.IsNullOrEmpty(StageName)
            ? LocalizedJsonString("/SharedStrings/Plugins/Dep/Contents/InstallingPlaceholder").Format(DependencyName)
            : StageName;

        public string ProgressString => ProgressIndeterminate || _progressValue < 0 || HideProgress
            ? string.Empty
            : LocalizedJsonString("/SharedStrings/Plugins/Dep/Contents/ProgressPlaceholder")
                .Format((int)ProgressValue);

        public bool ShowProgressString => !string.IsNullOrEmpty(ProgressString);
        public bool CirclePending { get; set; } = true;

        public HorizontalAlignment MessageAlignment => NoProgress
            ? HorizontalAlignment.Center
            : HorizontalAlignment.Left;

        public event PropertyChangedEventHandler PropertyChanged;

        public void OnPropertyChanged(string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}

public class AppTrackerEntry
{
    public TrackerType TrackerRole { get; set; } = TrackerType.TrackerHanded;
    public string Name => LocalizedJsonString($"/SharedStrings/Joints/Enum/{(int)TrackerRole}");
    public bool IsEnabled => AppData.Settings.TrackersVector.Any(x => x.Role == TrackerRole);
}

public class InputActionEntry
{
    public InputActionEndpoint Action { get; set; }
    public bool IsEnabled { get; set; }
    public string Name => Action?.LinkedAction?.Name ?? "INVALID";
}

public class InputActionBindingEntry : INotifyPropertyChanged
{
    private InputActionSource _treeSelectedAction;
    public InputActionEndpoint Action { get; set; }
    public InputActionSource Source { get; set; }

    public string ActionName => Action?.LinkedAction?.Name ?? "INVALID";
    public string SourceName => Source?.LinkedAction?.Name ?? Source?.Name ?? "Disabled";
    public string ActionNameFormatted => $"{ActionName}:";
    public string ActionDescription => Action?.LinkedAction?.Description;
    public string SourceDescription => Source?.LinkedAction?.Description;

    public string SourceTracker => AppPlugins.GetDevice(Source?.Device, out var device)
        ? device.TrackedJoints.FirstOrDefault(x => x.Role == Source?.Tracker, null)?
            .Role.ToString() ?? (Source?.Tracker.ToString() ?? string.Empty)
        : Source?.Tracker.ToString() ?? string.Empty;

    public string SourceDevice => AppPlugins.GetDevice(Source?.Device, out var device) ? device.Name : string.Empty;

    public string SourcePath => (string.IsNullOrEmpty(SourceDevice) ? "" : $"{SourceDevice} > ") +
                                (string.IsNullOrEmpty(SourceTracker) ? "" : $"{SourceTracker} > ") + SourceName;

    public bool IsEnabled => Source is not null;
    public bool IsValid => !IsEnabled || Source?.LinkedAction is not null;
    public bool IsInvalid => !IsValid;

    public InputActionSource TreeSelectedAction
    {
        get => _treeSelectedAction;
        set
        {
            if (Equals(value, _treeSelectedAction)) return;
            _treeSelectedAction = value;
            OnPropertyChanged();
        }
    }

    public string SelectedActionName => TreeSelectedAction?.Name ?? "No selection"; // TODO
    public bool SelectedActionValid => TreeSelectedAction is not null;

    public event PropertyChangedEventHandler PropertyChanged;

    protected virtual void OnPropertyChanged(string propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    public string TestingValue => string.IsNullOrEmpty(TreeCurrentAction?.TestValue) ? "No data" : TreeCurrentAction.TestValue; // TODO

    public static InputActionSource TreeCurrentAction { get; set; }
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

        try
        {
            // Delete the vendor plugin contract, just in case
            item.GetFiles("Amethyst.Plugins.Contract.dll").FirstOrDefault()?.Delete();
            item.GetFiles("Microsoft.Windows.SDK.NET.dll").FirstOrDefault()?.Delete();
            item.GetFiles("Microsoft.WinUI.dll").FirstOrDefault()?.Delete();
            item.GetFiles("WinRT.Runtime.dll").FirstOrDefault()?.Delete();
        }
        catch (Exception e)
        {
            Logger.Warn(e);
        }

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
                {
                    Logger.Error($"Loading {fileInfo} failed with an exception: Message: {e.Message} " +
                                 "Probably some assembly referenced by this plugin is missing.");

                    try
                    {
                        // Prepare assembly resources
                        var coreAssemblies =
                            Directory.GetFiles(RuntimeEnvironment.GetRuntimeDirectory(), "*.dll").ToList();
                        coreAssemblies.Add(Path.Join(ProgramLocation.DirectoryName, "Amethyst.Plugins.Contract.dll"));

                        // Load the failed assembly for metadata retrieval
                        var metadataContext = new MetadataLoadContext(new PathAssemblyResolver(coreAssemblies))
                            .LoadFromAssemblyPath(fileInfo.FullName);

                        // Prepare a null context for instantiation
                        IDependencyInstaller installerContext = null;

                        // Find the plugin export, if exists
                        var placeholderGuid = Guid.NewGuid().ToString().ToUpper();
                        var result = metadataContext.ExportedTypes.FirstOrDefault(x => x.CustomAttributes
                            .Any(export => export.ConstructorArguments.FirstOrDefault().Value?.ToString() is "Guid"));

                        // Check whether the plugin defines a dependency installer
                        if (result?.GetMetadata<Type>("DependencyInstaller") is not null)
                            try
                            {
                                var contextResult = new AssemblyLoadContext(placeholderGuid)
                                    .LoadFromAssemblyPath(fileInfo.FullName)
                                    .GetType(result.GetMetadata<Type>(
                                        "DependencyInstaller")?.FullName ?? string.Empty, true);

                                // Instantiate the installer and capture it for the outer scope
                                installerContext = contextResult.Instantiate<IDependencyInstaller>();
                            }
                            catch (Exception ex)
                            {
                                Logger.Error(ex);
                            }

                        // Add the plugin to the 'attempted' list
                        AppPlugins.LoadAttemptedPluginsList.Add(new LoadAttemptedPlugin
                        {
                            Name = result?.GetMetadata("Name", $"{item.Name}/{fileInfo.Name}"),
                            Guid = $"{result.GetMetadata("Guid", $"{placeholderGuid}")}:INSTALLER",
                            Error = $"{e.Message}\n\n{e.StackTrace}",
                            Folder = item.FullName,
                            Status = AppPlugins.PluginLoadError.NoPluginDll,

                            DependencyLink = result?.GetMetadata("DependencyLink", string.Empty)
                                ?.Format(DocsLanguageCode),
                            DependencySource = result?.GetMetadata("DependencySource", string.Empty),
                            DependencyInstaller = installerContext
                        });

                        // Check whether the plugin defines a dependency installer
                        // ReSharper disable once InvertIf | Metadata already checked
                        if (installerContext is not null)
                            try
                            {
                                // Set the device's string resources root to its provided folder
                                // (If it wants to change it, it's gonna need to do that after OnLoad anyway)
                                Logger.Info($"Registering (" +
                                            $"{result.GetMetadata("Name", $"{item.Name}/{fileInfo.Name}")}, " +
                                            $"{result.GetMetadata("Guid", $"{item.Name}/{fileInfo.Name}")}:INSTALLER) " +
                                            "default root language resource context (AppPlugins)...");

                                Interfacing.Plugins.SetLocalizationResourcesRoot(
                                    Path.Join(fileInfo.DirectoryName, "Assets", "Strings"),
                                    $"{result.GetMetadata("Guid", $"{placeholderGuid}")}:INSTALLER");

                                Logger.Info($"Overwriting (" +
                                            $"{result.GetMetadata("Name", $"{item.Name}/{fileInfo.Name}")}, " +
                                            $"{result.GetMetadata("Guid", $"{item.Name}/{fileInfo.Name}")}:INSTALLER) " +
                                            "'s localization host (IAmethystHost)...");

                                // Allow the installer to use Amethyst APIs
                                installerContext.Host = new CoreHost(
                                    $"{result.GetMetadata("Guid", $"{placeholderGuid}")}:INSTALLER");
                            }
                            catch (Exception ex)
                            {
                                Logger.Error(ex);
                            }

                        return false; // Nah, not this time
                    }
                    catch (Exception ex)
                    {
                        Logger.Error(e);
                        Logger.Error(ex);
                    }
                }

                // For any other errors
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

[Serializable]
public class InputActionEndpoint
{
    // Action's container tracker
    [JsonProperty] public TrackerType Tracker { get; set; }

    // Action that should be called
    [JsonProperty] public string Guid { get; set; }

    // MVVM Stuff
    [JsonIgnore]
    [IgnoreDataMember]
    public IKeyInputAction LinkedAction
    {
        get
        {
            try
            {
                return AppPlugins.CurrentServiceEndpoint?.SupportedInputActions?
                    .TryGetValue(Tracker, out var actions) ?? false
                    ? actions?.First(x => x.Guid == Guid) // Find the action
                    : null; // If there's no corresponding tracker - give up now
            }
            catch (Exception)
            {
                return null;
            }
        }
    }

    [JsonIgnore] [IgnoreDataMember] public bool IsValid => LinkedAction is not null;
}

[Serializable]
public class InputActionSource
{
    // The provider device's Guid
    [JsonProperty] public string Device { get; set; }

    // The action's friendly name (cached)
    [JsonProperty] public string Name { get; set; }

    // Action's container tracker
    [JsonProperty] public TrackedJointType Tracker { get; set; }

    // Action that should be called
    [JsonProperty] public string Guid { get; set; }

    // MVVM Stuff
    [JsonIgnore]
    [IgnoreDataMember]
    public IKeyInputAction LinkedAction
    {
        get
        {
            try
            {
                return AppPlugins.TrackingDevicesList.TryGetValue(Device, out var device)
                    ? device?.TrackedJoints?.Where(x => x.Role == Tracker)
                        .Select(x => x.SupportedInputActions.FirstOrDefault(y => y.Guid == Guid, null))
                        .FirstOrDefault(x => x is not null, null) // Return the first valid action
                    : null;
            }
            catch (Exception)
            {
                return null;
            }
        }
    }

    [JsonIgnore] [IgnoreDataMember] public bool IsValid => LinkedAction is not null;
    [JsonIgnore] [IgnoreDataMember] public string TestValue = string.Empty;
}