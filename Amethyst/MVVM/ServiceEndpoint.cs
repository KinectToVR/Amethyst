using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Numerics;
using System.Threading;
using System.Threading.Tasks;
using Amethyst.Classes;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;

namespace Amethyst.MVVM;

public class ServiceEndpoint(string name, string guid, string path, Version version, IServiceEndpoint service)
    : INotifyPropertyChanged
{
    // Extensions: is this service used atm?
    public bool IsUsed => AppData.Settings.ServiceEndpointGuid == Guid;

    // Get Docs
    [DefaultValue(null)] public Uri ErrorDocsUri => Service.ErrorDocsUri;

    // Get GUID
    [DefaultValue("INVALID")] public string Guid { get; } = guid;

    // Get Name
    [DefaultValue("UNKNOWN")] public string Name { get; } = name;

    // Get Path
    [DefaultValue("UNKNOWN")] public string Location { get; } = path;

    // Get the plugin version using its host assembly
    [DefaultValue("0.0.0.0")] public Version Version { get; } = version;

    // Underlying service handler
    private IServiceEndpoint Service { get; } = service;

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

    public (LocalisationFileJson Root, string Directory) LocalizationResourcesRoot { get; set; } = new();

    // Get the absolute pose of the HMD, calibrated against the play space
    // Return null if unknown to the service or unavailable
    // You'll need to provide this to support automatic calibration
    public (Vector3 Position, Quaternion Orientation)? HeadsetPose => Service.HeadsetPose;

    // Hot reload handler
    public FileSystemWatcher AssetsWatcher { get; set; }

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
        IEnumerable<TrackerBase> trackerBases, bool wantReply = true, CancellationToken? token = null)
    {
        return Service.UpdateTrackerPoses(trackerBases, wantReply, token);
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
        try
        {
            Service.Shutdown();
        }
        catch (Exception ex)
        {
            Logger.Error(ex);
        }

        // Try to cleanup memory after the plugin
        GC.Collect(GC.MaxGeneration, GCCollectionMode.Aggressive, true, true);
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    public void AssetsChanged(object o, FileSystemEventArgs fileSystemEventArgs)
    {
        Shared.Main.DispatcherQueue.TryEnqueue(() =>
        {
            // Hot reload device string resources
            Logger.Info($"Service ({Guid}, {Name}) assets have changed, reloading!");
            Logger.Info($"What happened: {fileSystemEventArgs.ChangeType}");
            Logger.Info($"Where: {fileSystemEventArgs.FullPath} ({fileSystemEventArgs.Name})");

            // Sanity check
            if (!Shared.Main.MainWindowLoaded) return;

            // Reload plugins' language resources
            Interfacing.Plugins.SetLocalizationResourcesRoot(
                LocalizationResourcesRoot.Directory, Guid);

            // Reload everything we can
            Shared.Devices.DevicesJointsValid = false;
            Service.OnLoad(); // Reload settings

            // Force refresh all the valid pages
            Shared.Events.RequestInterfaceReload(false);

            if (AppData.Settings.ServiceEndpointGuid == Guid)
                Interfacing.UpdateServerStatus(); // Refresh

            // We're done with our changes now!
            Shared.Devices.DevicesJointsValid = true;
        });
    }
}