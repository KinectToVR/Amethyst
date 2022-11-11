using System;
using Amethyst.Plugins.Contract;
using System.Collections.Generic;
using System.ComponentModel;
using System.ComponentModel.Composition;
using System.ComponentModel.Composition.Hosting;
using System.ComponentModel.Composition.Primitives;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using Amethyst.Classes;
using Amethyst.Utils;

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
    
    private ITrackingDevice Device { get; init; }

    public List<(TrackedJointType Role, TrackedJoint Joint)> TrackedJoints => Device.TrackedJoints;

    public void OnLoad() => Device.OnLoad();
    public void Initialize() => Device.Initialize();
    public void Shutdown() => Device.Shutdown();
    public void Update() => Device.Update();

    public bool IsInitialized => Device.IsInitialized;
    public bool IsSkeletonTracked => Device.IsSkeletonTracked;
    public bool IsPositionFilterBlockingEnabled => Device.IsPositionFilterBlockingEnabled;
    public bool IsPhysicsOverrideEnabled => Device.IsPhysicsOverrideEnabled;
    public bool IsSelfUpdateEnabled => Device.IsSelfUpdateEnabled;
    public bool IsFlipSupported => Device.IsFlipSupported;
    public bool IsSettingsDaemonSupported => Device.IsSettingsDaemonSupported;
    public bool IsAppOrientationSupported => Device.IsAppOrientationSupported;

    public object SettingsInterfaceRoot => Device.SettingsInterfaceRoot;
    public int DeviceStatus => Device.DeviceStatus;
    public string DeviceStatusString => Device.DeviceStatusString;

    // Signal the joint eg psm_id0 that it's been selected
    public void SignalJoint(int jointId) => Device.SignalJoint(jointId);

    public event PropertyChangedEventHandler PropertyChanged;

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(null));
    }

    // MVVM stuff
    public double BoolToOpacity(bool value) => value ? 1.0 : 0.0;

    public bool StatusOk => Device.DeviceStatus == 0;
    public bool StatusError => Device.DeviceStatus != 0;
}

public static class ICollectionExtensions
{
    public static bool AddPlugin<T>(this ICollection<T> collection, DirectoryInfo item) where T : ComposablePartCatalog
    {
        foreach (var fileInfo in item.GetFiles("*.dll"))
            try
            {
                var assemblyFile = Assembly.LoadFile(fileInfo.FullName);
                var assemblyCatalog = new AssemblyCatalog(assemblyFile);

                if (!assemblyCatalog.Parts.Any(x => x.ExportDefinitions
                        .Any(y => y.ContractName == "AmethystPluginContract.ITrackingDevice"))) continue;

                collection.Add((T)(object)assemblyCatalog);
                return true; // This plugin is probably supported, yay!
            }
            catch (CompositionException e)
            {
                Logger.Error("Loading plugins failed with a composition exception: " +
                             $"Message: {e.Message}\nErrors occurred: {e.Errors}\nPossible causes: {e.RootCauses}");
            }
            catch (Exception e)
            {
                Logger.Error($"Loading plugins failed with an exception: Message: {e.Message}" +
                             "Probably some assembly referenced by this plugin is missing.");
            }

        return true; // Nah, not this time
    }
}