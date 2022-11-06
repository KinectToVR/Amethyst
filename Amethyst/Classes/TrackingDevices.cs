using System;
using System.Collections.Generic;
using Microsoft.UI.Xaml.Documents;

namespace Amethyst.Classes;

public static class TrackingDevices
{
    public enum PluginLoadError
    {
        Unknown, // We literally don't know what's happened
        NoError, // Everything's fine, celebration time!
        LoadingSkipped, // This device is disabled by the user
        NoDeviceFolder, // No device folder w/ files found
        NoDeviceDll, // Device dll not found at proper path
        NoDeviceDependencyDll, // Dep dll/s not found or invalid
        DeviceDllLinkError, // Could not link for some reason
        BadOrDuplicateGUID, // Empty/Bad/Duplicate device GUID
        InvalidFactory, // Device factory just gave up, now cry
        Other // Check logs, MEF probably gave us up again...
    }

    // Vector of current devices' JSON resource roots & paths
    // Note: the size must be the same as TrackingDevicesVector's
    public static List<ValueTuple<Windows.Data.Json.JsonObject, System.IO.DirectoryInfo>>
        TrackingDevicesLocalizationResourcesRootsVector;

    // Written to at the first plugin load
    public static List<ValueTuple<
            string, // Name
            string, // GUID
            PluginLoadError, // Status
            string>> // Plugin dll dir
        LoadAttemptedTrackingDevicesVector;
}