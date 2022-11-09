using Amethyst.Plugins.Contract;
using System.Collections.Generic;
using System.ComponentModel;

namespace Amethyst.MVVM;

public interface TrackingDevice : ITrackingDevice
{
    // Get GUID
    [DefaultValue("INVALID")] string Guid { get; }

    // Get Name
    [DefaultValue("UNKNOWN")] string Name { get; }
}