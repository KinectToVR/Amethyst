using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Threading.Tasks;
using Amethyst.Classes;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media.Animation;
using Microsoft.UI.Xaml.Media.Imaging;
using static Amethyst.Classes.Interfacing;

namespace Amethyst.MVVM;

public class TrackingDevice : INotifyPropertyChanged
{
    public TrackingDevice(string name, string guid, string path, Version version, ITrackingDevice device)
    {
        // Setup device data
        Guid = guid;
        Name = name;
        Location = path;
        Version = version;
        Device = device;

        // Setup optional camera streams
        var cameraImageProperty = device.GetType().GetProperty("GetCameraImage");
        var cameraMapperProperty = device.GetType().GetProperty("MapCoordinateDelegate");
        var cameraEnabledProperty = device.GetType().GetProperty("GetIsCameraEnabled");
        var cameraEnabledSetter = device.GetType().GetProperty("SetIsCameraEnabled");

        if (!(cameraImageProperty?.CanRead ?? false) ||
            !(cameraEnabledProperty?.CanRead ?? false) ||
            !(cameraEnabledSetter?.CanRead ?? false) ||
            !(cameraMapperProperty?.CanRead ?? false)) return;

        // Cache all getters & setters for quicker access later on
        GetCameraImage = cameraImageProperty.GetValue(device) as Func<BitmapSource>;
        GetIsCameraEnabled = cameraEnabledProperty.GetValue(device) as Func<bool>;
        SetIsCameraEnabled = cameraEnabledSetter.GetValue(device) as Action<bool>;
        MapCoordinateDelegate = cameraMapperProperty.GetValue(device) as Func<Vector3, Size>;
        IsCameraSupported = true; // Mark as OK
    }

    // Extensions: is this device set as base?
    public bool IsBase => AppPlugins.IsBase(Guid);

    // Extensions: is this device set as an override?
    public bool IsOverride => AppPlugins.IsOverride(Guid);

    // Get GUID
    [DefaultValue("INVALID")] public string Guid { get; }

    // Get Name
    [DefaultValue("UNKNOWN")] public string Name { get; }

    // Get Path
    [DefaultValue("UNKNOWN")] public string Location { get; }

    // Get the plugin version using its host assembly
    [DefaultValue("0.0.0.0")] public Version Version { get; }

    // Get Docs
    [DefaultValue(null)] public Uri ErrorDocsUri => Device.ErrorDocsUri;

    // Underlying device handler
    public ITrackingDevice Device { get; }

    // Joints' list / you need to (should) update at every update() call
    // Each must have its own role or _Manual to force user's manual set
    public List<TrackedJoint> TrackedJoints => Interfacing.ReplayManager.GetJointsOr(Guid, Device.TrackedJoints.ToList());

    public (LocalisationFileJson Root, string Directory) LocalizationResourcesRoot { get; set; }

    // Is the device connected/started?
    public bool IsInitialized => Device.IsInitialized;

    // This should be updated on every frame,
    // along with joint devices
    // -> will lead to global tracking loss notification
    //    if set to false at runtime some-when
    public bool IsSkeletonTracked => Interfacing.ReplayManager.GetTrackingStateOr(Guid, Device.IsSkeletonTracked);

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

    // Hot reload handler
    public FileSystemWatcher AssetsWatcher { get; set; }

    public string[] DeviceStatusSplit
    {
        get
        {
            // Split status and message by \n
            var message = StringUtils.SplitStatusString(DeviceStatusString);
            return message is null || message.Length < 3 ? ["The status message was broken!", "E_FIX_YOUR_SHIT", "AAAAA"] : message;
        }
    }

    public string DeviceJointsCount => $"Forwarded joints: {TrackedJoints.Count}";
    public string DeviceSettingsText => LocalizedJsonString("/GeneralPage/Buttons/DeviceSettings").Format(Name);
    public bool ServiceNotRelay => AppPlugins.CurrentServiceEndpoint.Guid is not "K2VRTEAM-AME2-APII-DVCE-TRACKINGRELAY";
    public bool IsFromRelay => Guid.StartsWith("TRACKINGRELAY:");
    public string RelayText => $"From {HostMachineName}";
    public string HostMachineName { get; set; } = "Amethyst Tracking Relay";

    private Func<BitmapSource> GetCameraImage { get; }
    private Func<bool> GetIsCameraEnabled { get; }
    private Action<bool> SetIsCameraEnabled { get; }
    private Func<Vector3, Size> MapCoordinateDelegate { get; }

    public BitmapSource CameraImage => GetCameraImage?.Invoke();

    public bool IsCameraSupported { get; init; }

    public bool IsCameraEnabled
    {
        get => GetIsCameraEnabled?.Invoke() ?? false;
        set => SetIsCameraEnabled?.Invoke(value);
    }

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

    public double BoolToOpacityHalf(bool value)
    {
        return value ? 1.0 : 0.5;
    }

    public double BoolToOpacityMultiple(bool v1, bool v2)
    {
        return v1 && v2 ? 1.0 : 0.0;
    }

    private void TrackedJoints_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
    {
        if (Guid.Contains("KINECT") && Device.TrackedJoints.Count > 0) return;
        Shared.Main.DispatcherQueue.TryEnqueue(() =>
        {
            Logger.Info($"Joints changed in {Guid}! Old: {e.OldItems?.Count} New: {e.NewItems?.Count}");

            // Stop the pose composer for now
            lock (UpdateLock)
            {
                AppPlugins.HandleDeviceRefresh(false);
                AppData.Settings.CheckSettings(); // Refresh
            }

            // Make all the devices refresh their props
            AppPlugins.TrackingDevicesList.ToList()
                .ForEach(x => x.Value.OnPropertyChanged());

            // Update other statuses
            AppPlugins.UpdateTrackingDevicesInterface();
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

    public void AssetsChanged(object o, FileSystemEventArgs fileSystemEventArgs)
    {
        Shared.Main.DispatcherQueue.TryEnqueue(() =>
        {
            // Hot reload device string resources
            Logger.Info($"Device ({Guid}, {Name}) assets have changed, reloading!");
            Logger.Info($"What happened: {fileSystemEventArgs.ChangeType}");
            Logger.Info($"Where: {fileSystemEventArgs.FullPath} ({fileSystemEventArgs.Name})");

            // Sanity check
            if (!Shared.Main.MainWindowLoaded) return;

            // Reload plugins' language resources
            Interfacing.Plugins.SetLocalizationResourcesRoot(
                LocalizationResourcesRoot.Directory, Guid);

            // Reload everything we can
            Shared.Devices.DevicesJointsValid = false;
            Device.OnLoad(); // Reload settings

            // Force refresh all the valid pages
            Shared.Events.RequestInterfaceReload(false);

            // We're done with our changes now!
            Shared.Devices.DevicesJointsValid = true;
        });
    }

    public async void ViewDeviceSettings(object sender, RoutedEventArgs e)
    {
        // Go to the devices page to view device settings

        // Navigate to the settings page
        Shared.Main.MainNavigationView.SelectedItem =
            Shared.Main.MainNavigationView.MenuItems[2];

        Shared.Main.NavigateToPage("devices",
            new EntranceNavigationTransitionInfo());

        await Task.Delay(500);

        // Should already be init-ed after 500ms, but check anyway
        if (Shared.Devices.DevicesTreeView is null) return;
        var devicesListIndex = AppPlugins.TrackingDevicesList.Keys.ToList().IndexOf(Guid);
        var devicesListNode = Shared.Devices.DevicesTreeView.RootNodes[devicesListIndex];

        var skipAnimation = Shared.Devices.DevicesTreeView.SelectedNode == devicesListNode;
        Shared.Devices.DevicesTreeView.SelectedNode = devicesListNode;

        AppData.Settings.PreviousSelectedTrackingDeviceGuid = Guid;
        AppData.Settings.SelectedTrackingDeviceGuid = Guid;

        await Shared.Devices.ReloadSelectedDevice(skipAnimation);
        Shared.Events.ReloadDevicesPageEvent?.Set(); // Full reload
    }

    public (double Left, double Top) MapCoordinate(Vector3 position)
    {
        var result = MapCoordinateDelegate?.Invoke(position) ?? Size.Empty;
        //return (Left: result.Width / 1000.0f, Top: result.Height / 1000.0f);
        return (Left: result.Width, Top: result.Height);
    }

    public bool MapCoordinate(Vector3 position, out (double Left, double Top) value)
    {
        var result = MapCoordinateDelegate?.Invoke(position) ?? Size.Empty;
        //return (Left: result.Width / 1000.0f, Top: result.Height / 1000.0f);
        value = (Left: result.Width, Top: result.Height);
        return value.IsValid();
    }
}

public static class MapperExtensions
{
    public static bool IsValid(this (double Left, double Top) values)
    {
        return double.IsNormal(values.Left) && double.IsNormal(values.Top) &&
               values is not { Left: -1.0, Top: -1.0 };
    }
}