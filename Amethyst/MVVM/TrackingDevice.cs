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
using System.Runtime.Loader;
using System.Threading;
using System.Threading.Tasks;
using Windows.Data.Json;
using Windows.System;
using Amethyst.Classes;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using AmethystSupport;
using static Amethyst.Classes.Interfacing;
using Microsoft.UI.Xaml.Controls;

namespace Amethyst.MVVM;

public class TrackingDevice : INotifyPropertyChanged
{
    public TrackingDevice(string name, string guid, string path, ITrackingDevice device)
    {
        Guid = guid;
        Name = name;
        Location = path;
        Device = device;
    }

    // Extensions: is this device set as base?
    public bool IsBase => TrackingDevices.IsBase(Guid);

    // Extensions: is this device set as an override?
    public bool IsOverride => TrackingDevices.IsOverride(Guid);

    // Get GUID
    [DefaultValue("INVALID")] public string Guid { get; }

    // Get Name
    [DefaultValue("UNKNOWN")] public string Name { get; }

    // Get Path
    [DefaultValue("UNKNOWN")] public string Location { get; }

    // Get Docs
    [DefaultValue(null)] public Uri ErrorDocsUri => Device.ErrorDocsUri;

    // Underlying device handler
    private ITrackingDevice Device { get; }

    // Joints' list / you need to (should) update at every update() call
    // Each must have its own role or _Manual to force user's manual set
    public List<TrackedJoint> TrackedJoints => Device.TrackedJoints.ToList();

    public (JsonObject Root, string Directory) LocalizationResourcesRoot { get; set; }

    // Is the device connected/started?
    public bool IsInitialized => Device.IsInitialized;

    // This should be updated on every frame,
    // along with joint devices
    // -> will lead to global tracking loss notification
    //    if set to false at runtime some-when
    public bool IsSkeletonTracked => Device.IsSkeletonTracked;

    // Should be set up at construction
    // This will tell Amethyst to disable all position filters on joints managed by this plugin
    public bool IsPositionFilterBlockingEnabled => Device.IsPositionFilterBlockingEnabled;

    // Should be set up at construction
    // This will tell Amethyst not to auto-manage on joints managed by this plugin
    // Includes: velocity, acceleration, angular velocity, angular acceleration
    public bool IsPhysicsOverrideEnabled => Device.IsPhysicsOverrideEnabled;

    // Should be set up at construction
    // This will tell Amethyst not to auto-update this device
    // You should register some timer to update your device yourself
    public bool IsSelfUpdateEnabled => Device.IsSelfUpdateEnabled;

    // Should be set up at construction
    // Mark this as false ALSO if your device supports 360 tracking by itself
    public bool IsFlipSupported => Device.IsFlipSupported;

    // To support settings daemon and register the layout root,
    // the device must properly report it first
    // -> will lead to showing an additional 'settings' button
    // Note: each device has to save its settings independently
    //       and may use the K2AppData from the Paths' class
    // Tip: you can hide your device's settings by marking this as 'false',
    //      and change it back to 'true' when you're ready
    public bool IsSettingsDaemonSupported => Device.IsSettingsDaemonSupported;

    // Should be set up at construction
    // This will allow Amethyst to calculate rotations by itself, additionally
    public bool IsAppOrientationSupported =>
        Device.IsAppOrientationSupported && // The device must declare it actually consists
        Device.TrackedJoints.Any(x => x.Role == TrackedJointType.JointFootLeft) &&
        Device.TrackedJoints.Any(x => x.Role == TrackedJointType.JointFootRight) &&
        Device.TrackedJoints.Any(x => x.Role == TrackedJointType.JointFootTipLeft) &&
        Device.TrackedJoints.Any(x => x.Role == TrackedJointType.JointFootTipRight) &&
        Device.TrackedJoints.Any(x => x.Role == TrackedJointType.JointKneeLeft) &&
        Device.TrackedJoints.Any(x => x.Role == TrackedJointType.JointKneeRight);

    // Settings UI root / MUST BE OF TYPE Microsoft.UI.Xaml.Controls.Page
    // Return new() of your implemented Page, and that's basically it!
    public object SettingsInterfaceRoot => Device.SettingsInterfaceRoot;

    // These will indicate the device's status [OK is (int)0]
    // Both should be updated either on call or as frequent as possible
    public int DeviceStatus => Device.DeviceStatus;

    // Device status string: to get your resources, use RequestLocalizedString
    public string DeviceStatusString => Device.DeviceStatusString;

    // Is the status okay, quick check?
    public bool StatusOk => Device.DeviceStatus == 0;

    // Is the status NOT okay, quick check?
    public bool StatusError => Device.DeviceStatus != 0;

    // Is the device used as anything, quick check?
    public bool IsUsed => IsBase || IsOverride;

    // Property changed event
    public event PropertyChangedEventHandler PropertyChanged;

    // This is called after the app loads the plugin
    public void OnLoad()
    {
        Device.OnLoad();
    }

    // This initializes/connects the device
    public void Initialize()
    {
        Device.Initialize();

        // Re-register joint changes handlers
        Device.TrackedJoints.CollectionChanged -= TrackedJoints_CollectionChanged;
        Device.TrackedJoints.CollectionChanged += TrackedJoints_CollectionChanged;
    }

    // This is called when the device is closed
    public void Shutdown()
    {
        Device.Shutdown();

        // Unregister joint changes handlers
        Device.TrackedJoints.CollectionChanged -= TrackedJoints_CollectionChanged;

        // Try to cleanup memory after the plugin
        GC.Collect(GC.MaxGeneration, GCCollectionMode.Aggressive, true, true);
    }

    // This is called to update the device (each loop)
    public void Update()
    {
        Device.Update();
    }

    // Signal the joint eg psm_id0 that it's been selected
    public void SignalJoint(int jointId)
    {
        Device.SignalJoint(jointId);
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    // MVVM stuff
    public double BoolToOpacity(bool value)
    {
        return value ? 1.0 : 0.0;
    }

    public double BoolToOpacityMultiple(bool v1, bool v2)
    {
        return v1 && v2 ? 1.0 : 0.0;
    }

    private void TrackedJoints_CollectionChanged(object sender, object e)
    {
        Shared.Main.DispatcherQueue.TryEnqueue(() =>
        {
            // Stop the pose composer for now
            lock (UpdateLock)
            {
                TrackingDevices.HandleDeviceRefresh(false);
                AppData.Settings.CheckSettings(); // Refresh
            }

            // Make all the devices refresh their props
            TrackingDevices.TrackingDevicesList.ToList()
                .ForEach(x => x.Value.OnPropertyChanged());

            // Update other statuses
            TrackingDevices.UpdateTrackingDevicesInterface();
            Shared.Events.RequestInterfaceReload(false);

            // Save the application config
            AppData.Settings.SaveSettings();
        });
    }

    public void RefreshWatchHandlers()
    {
        // Re-register joint changes handlers
        Device.TrackedJoints.CollectionChanged -= TrackedJoints_CollectionChanged;
        Device.TrackedJoints.CollectionChanged += TrackedJoints_CollectionChanged;
    }
}

public class ServiceEndpoint : INotifyPropertyChanged
{
    public ServiceEndpoint(string name, string guid, string path, IServiceEndpoint service)
    {
        Guid = guid;
        Name = name;
        Location = path;
        Service = service;
    }

    // Extensions: is this service used atm?
    public bool IsUsed => AppData.Settings.ServiceEndpointGuid == Guid;

    // Get Docs
    [DefaultValue(null)] public Uri ErrorDocsUri => Service.ErrorDocsUri;

    // Get GUID
    [DefaultValue("INVALID")] public string Guid { get; }

    // Get Name
    [DefaultValue("UNKNOWN")] public string Name { get; }

    // Get Path
    [DefaultValue("UNKNOWN")] public string Location { get; }

    // Underlying service handler
    private IServiceEndpoint Service { get; }

    // To support settings daemon and register the layout root,
    // the device must properly report it first
    // -> will lead to showing an additional 'settings' button
    // Note: each device has to save its settings independently
    //       and may use the K2AppData from the Paths' class
    // Tip: you can hide your device's settings by marking this as 'false',
    //      and change it back to 'true' when you're ready
    public bool IsSettingsDaemonSupported => Service.IsSettingsDaemonSupported;

    // Settings UI root / MUST BE OF TYPE Microsoft.UI.Xaml.Controls.Page
    // Return new() of your implemented Page, and that's basically it!
    public object SettingsInterfaceRoot => Service.SettingsInterfaceRoot;

    // These will indicate the device's status [OK is (int)0]
    // Both should be updated either on call or as frequent as possible
    public int ServiceStatus => Service.ServiceStatus;

    // Device status string: to get your resources, use RequestLocalizedString
    public string ServiceStatusString => Service.ServiceStatusString;

    // Is the status okay, quick check?
    public bool StatusOk => Service.ServiceStatus == 0;

    // Is the status NOT okay, quick check?
    public bool StatusError => Service.ServiceStatus != 0;

    // Additional supported tracker types set
    // The mandatory ones are: waist, left foot, and right foot
    public SortedSet<TrackerType> AdditionalSupportedTrackerTypes => Service.AdditionalSupportedTrackerTypes;

    // Mark as true to tell the user that they need to restart/
    // /in case they want to add more trackers after spawning
    // This is the case with OpenVR, where settings need to be reloaded
    public bool IsRestartOnChangesNeeded => Service.IsRestartOnChangesNeeded;

    // Check if Amethyst is shown in the service dashboard or similar
    // This is only available for a few actual cases, like OpenVR
    public bool IsAmethystVisible => Service.IsAmethystVisible;

    // Check running system name, this is important for input
    public string TrackingSystemName => Service.TrackingSystemName;

    // Controller input actions, for calibration and others
    // Also provides support for flip/freeze quick toggling
    // Leaving this null will result in marking the
    // manual calibration and input actions support as [false]
    public InputActions ControllerInputActions => Service.ControllerInputActions;

    // For AutoStartAmethyst: check if it's even possible
    // Mark as true to state that starting your app/service
    // / can automatically start Amethyst at the same time
    public bool CanAutoStartAmethyst => Service.CanAutoStartAmethyst;

    // Check or set if starting the service should auto-start Amethyst
    // This is only available for a few actual cases, like OpenVR
    public bool AutoStartAmethyst
    {
        get => Service.AutoStartAmethyst;
        set => Service.AutoStartAmethyst = value;
    }

    // Check or set if closing the service should auto-close Amethyst
    // This is only available for a few actual cases, like OpenVR
    public bool AutoCloseAmethyst
    {
        get => Service.AutoCloseAmethyst;
        set => Service.AutoCloseAmethyst = value;
    }

    public (JsonObject Root, string Directory) LocalizationResourcesRoot { get; set; } = new();

    // Get the absolute pose of the HMD, calibrated against the play space
    // Return null if unknown to the service or unavailable
    // You'll need to provide this to support automatic calibration
    public (Vector3 Position, Quaternion Orientation)? HeadsetPose => Service.HeadsetPose;

    // Property changed event
    public event PropertyChangedEventHandler PropertyChanged;

    // Implement if your service supports custom toasts
    // Services like OpenVR can show internal toasts
    public void DisplayToast((string Title, string Text) message)
    {
        Service.DisplayToast(message);
    }

    // Request a restart of the tracking endpoint service
    public bool? RequestServiceRestart(string reason, bool wantReply = false)
    {
        return Service.RequestServiceRestart(reason, wantReply);
    }

    // Check connection: status, serialized status, combined ping time
    public Task<(int Status, string StatusMessage, long PingTime)> TestConnection()
    {
        return Service.TestConnection();
    }

    // Find an already-existing tracker and get its pose
    // For no results found return null, also check if it's from amethyst
    public TrackerBase GetTrackerPose(string contains, bool canBeFromAmethyst = true)
    {
        return Service.GetTrackerPose(contains, canBeFromAmethyst);
    }

    // Set tracker states, add/spawn if not present yet
    // Default to the serial, update the role if needed
    // Returns the same vector with paired success property (or null)
    public Task<IEnumerable<(TrackerBase Tracker, bool Success)>> SetTrackerStates(
        IEnumerable<TrackerBase> trackerBases, bool wantReply = true)
    {
        return Service.SetTrackerStates(trackerBases, wantReply);
    }

    // Update tracker positions and physics components
    // Check physics against null, they're passed as optional
    // Returns the same vector with paired success property (or null)
    public Task<IEnumerable<(TrackerBase Tracker, bool Success)>> UpdateTrackerPoses(
        IEnumerable<TrackerBase> trackerBases, bool wantReply = true)
    {
        return Service.UpdateTrackerPoses(trackerBases, wantReply);
    }

    // This is called after the app loads the plugin
    public void OnLoad()
    {
        Service.OnLoad();
    }

    // This is called right before the pose compose
    public void Heartbeat()
    {
        Service.Heartbeat();
    }

    // This initializes/connects to the service
    public void Initialize()
    {
        Service.Initialize();
    }

    // This is called when the service is closed
    public void Shutdown()
    {
        Service.Shutdown();

        // Try to cleanup memory after the plugin
        GC.Collect(GC.MaxGeneration, GCCollectionMode.Aggressive, true, true);
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }
}

public static class ICollectionExtensions
{
    public static bool AddPlugin<T>(this ICollection<T> collection, DirectoryInfo item) where T : ComposablePartCatalog
    {
        // Delete the vendor plugin contract, just in case
        item.GetFiles("Amethyst.Plugins.Contract.dll").FirstOrDefault()?.Delete();
        item.GetFiles("Microsoft.Windows.SDK.NET.dll").FirstOrDefault()?.Delete();
        item.GetFiles("Microsoft.WinUI.dll").FirstOrDefault()?.Delete();
        item.GetFiles("WinRT.Runtime.dll").FirstOrDefault()?.Delete();

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
                if (fileInfo.Name.StartsWith("plugin"))
                    Logger.Error($"Loading {fileInfo} failed with a composition exception: " +
                                 $"Message: {e.Message}\nErrors occurred: {e.Errors}\nPossible causes: {e.RootCauses}");
                else
                    Logger.Warn($"[Non-critical] Loading {fileInfo} failed with a composition exception: " +
                                $"Message: {e.Message}\nErrors occurred: {e.Errors}\nPossible causes: {e.RootCauses}");
            }
            catch (Exception e)
            {
                if (fileInfo.Name.StartsWith("plugin"))
                    Logger.Error($"Loading {fileInfo} failed with an exception: Message: {e.Message} " +
                                 "Probably some assembly referenced by this plugin is missing.");
                else
                    Logger.Warn($"[Non-critical] Loading {fileInfo} failed with an exception: {e.Message}");
            }

        return true; // Nah, not this time
    }
}

public class ModuleAssemblyLoadContext : AssemblyLoadContext
{
    private readonly AssemblyDependencyResolver _resolver;

    internal ModuleAssemblyLoadContext(string assemblyPath) : base(false)
    {
        _resolver = new AssemblyDependencyResolver(assemblyPath);

        ResolvingUnmanagedDll += OnResolvingUnmanaged;
        Resolving += OnResolving;
    }

    private IntPtr OnResolvingUnmanaged(Assembly assembly, string unmanagedName)
    {
        var unmanagedPath = _resolver.ResolveUnmanagedDllToPath(unmanagedName);
        return unmanagedPath != null ? LoadUnmanagedDllFromPath(unmanagedPath) : IntPtr.Zero;
    }

    private Assembly OnResolving(AssemblyLoadContext context, AssemblyName assemblyName)
    {
        var assemblyPath = _resolver.ResolveAssemblyToPath(assemblyName);
        return assemblyPath != null ? LoadFromAssemblyPath(assemblyPath) : null;
    }
}

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
        return TrackingDevices.BaseTrackingDevice.TrackedJoints.Exists(x => x.Role == jointType);
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
            if (TrackingDevices.TrackingDevicesList.TryGetValue(Guid, out var device) && device is not null)
                device.RefreshWatchHandlers(); // Re-register joint changes handlers

            // Check if used in any way: as the base, an override, or the endpoint
            if (!TrackingDevices.IsBase(Guid) && !TrackingDevices.IsOverride(Guid) &&
                TrackingDevices.CurrentServiceEndpoint.Guid != Guid) return;

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
                var hPath = Path.Combine(GetProgramLocation().DirectoryName!, "K2CrashHandler", "K2CrashHandler.exe");
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
    public string Name { get; init; } = "[UNKNOWN]";
    public string Guid { get; init; } = "[INVALID]";
    public string Error { get; init; } = null;

    public string Publisher { get; init; }
    public string Website { get; init; }
    public string DeviceFolder { get; init; }

    public string DeviceUpdateUri { get; init; } = "[UNKNOWN]";
    public string DeviceVersion { get; init; } = "[UNKNOWN]";
    public string DeviceApiVersion { get; init; } = "[INVALID]";

    public TrackingDevices.PluginType PluginType { get; init; } =
        TrackingDevices.PluginType.Unknown;

    // MVVM stuff
    public TrackingDevices.PluginLoadError Status { get; init; } =
        TrackingDevices.PluginLoadError.Unknown;

    public bool LoadError => Status is not
        TrackingDevices.PluginLoadError.NoError and not
        TrackingDevices.PluginLoadError.LoadingSkipped;

    public bool LoadSuccess => Status is
        TrackingDevices.PluginLoadError.NoError or
        TrackingDevices.PluginLoadError.LoadingSkipped;

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
            if (TrackingDevices.TrackingDevicesList.ContainsKey(Guid) &&
                AppData.Settings.DisabledPluginsGuidSet.Contains(Guid))
            {
                SortedSet<string> loadedDeviceSet = new();

                // Check which devices are loaded : device plugin
                if (TrackingDevices.TrackingDevicesList.ContainsKey("K2VRTEAM-AME2-APII-DVCE-DVCEKINECTV1"))
                    loadedDeviceSet.Add("K2VRTEAM-AME2-APII-DVCE-DVCEKINECTV1");
                if (TrackingDevices.TrackingDevicesList.ContainsKey("K2VRTEAM-AME2-APII-DVCE-DVCEKINECTV2"))
                    loadedDeviceSet.Add("K2VRTEAM-AME2-APII-DVCE-DVCEKINECTV2");
                if (TrackingDevices.TrackingDevicesList.ContainsKey("K2VRTEAM-AME2-APII-DVCE-DVCEPSMOVEEX"))
                    loadedDeviceSet.Add("K2VRTEAM-AME2-APII-DVCE-DVCEPSMOVEEX");
                if (TrackingDevices.TrackingDevicesList.ContainsKey("K2VRTEAM-VEND-API1-DVCE-DVCEOWOTRACK"))
                    loadedDeviceSet.Add("K2VRTEAM-VEND-API1-DVCE-DVCEOWOTRACK");

                // If we've just disabled the last loaded device, re-enable the first
                if (TrackingDevices.TrackingDevicesList.Keys.All(
                        AppData.Settings.DisabledPluginsGuidSet.Contains) ||

                    // If this entry happens to be the last one of the official ones
                    (loadedDeviceSet.Contains(Guid) && loadedDeviceSet.All(
                        AppData.Settings.DisabledPluginsGuidSet.Contains)))

                    // Re-enable this device if upper conditions are met
                    AppData.Settings.DisabledPluginsGuidSet.Remove(Guid);
            }

            // Check if the change is valid : service endpoint
            else if (TrackingDevices.ServiceEndpointsList.ContainsKey(Guid) &&
                     AppData.Settings.DisabledPluginsGuidSet.Contains(Guid))
            {
                SortedSet<string> loadedServiceSet = new();

                // Check which services are loaded
                if (TrackingDevices.ServiceEndpointsList.ContainsKey("K2VRTEAM-AME2-APII-SNDP-SENDPTOPENVR"))
                    loadedServiceSet.Add("K2VRTEAM-AME2-APII-SNDP-SENDPTOPENVR");
                if (TrackingDevices.ServiceEndpointsList.ContainsKey("K2VRTEAM-AME2-APII-SNDP-SENDPTVRCOSC"))
                    loadedServiceSet.Add("K2VRTEAM-AME2-APII-SNDP-SENDPTVRCOSC");

                // If we've just disabled the last loaded service, re-enable the first
                if (TrackingDevices.ServiceEndpointsList.Keys.All(
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
                Shared.TeachingTips.MainPage.ReloadTeachingTip.IsOpen = true;

            // Save settings
            AppData.Settings.SaveSettings();
            OnPropertyChanged("IsLoaded");
        }
    }

    public string ErrorText => LocalizedJsonString($"/DevicesPage/Devices/Manager/Labels/{(int)Status}");

    public bool PublisherValid => !string.IsNullOrEmpty(Publisher);
    public bool WebsiteValid => !string.IsNullOrEmpty(Website);
    public bool LocationValid => !string.IsNullOrEmpty(DeviceFolder);
    public bool GuidValid => !string.IsNullOrEmpty(Guid);

    public event PropertyChangedEventHandler PropertyChanged;

    public string TrimString(string s, int l)
    {
        return s?[..Math.Min(s.Length, l)] +
               (s?.Length > l ? "..." : "");
    }

    public void ShowDeviceFolder()
    {
        SystemShell.OpenFolderAndSelectItem(DeviceFolder);
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