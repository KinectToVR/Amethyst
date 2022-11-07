using Amethyst.Plugins.Contract;
using Grpc.Net.Client;
using Amethyst.Driver.API;

namespace Amethyst.Driver.Client;

public static class DriverClient
{
    /// <summary>
    /// Connects socket object to selected port, AME uses 7135
    /// </summary>
    /// <param name="host">AME Server Driver IP:Port</param>
    /// <returns>0: OK, -1: Exception, -2: Channel fail, -3: Stub failure</returns>
    public static int InitAmethystServer(string host = "localhost:7135")
    {
        return 0;
    }

    public static List<(TrackerType Role, bool State)> UpdateTrackerStates(List<(TrackerType Role, bool State)> states)
    {
        return new List<(TrackerType, bool)> { new(TrackerType.TrackerHanded, true) };
    }

    public static List<(TrackerType Role, bool State)> UpdateTrackerPoses(List<K2TrackerBase> trackerBases)
    {
        return new List<(TrackerType, bool)> { new(TrackerType.TrackerHanded, true) };
    }

    public static List<(TrackerType Role, bool State)> RefreshTrackerPoses(List<TrackerType> trackers)
    {
        return new List<(TrackerType, bool)> { new(TrackerType.TrackerHanded, true) };
    }

    public static bool RequestVrRestart(string reason)
    {
        return true;
    }

    public static ValueTuple<object, long, long, long> TestConnection()
    {
        return new ValueTuple<object, long, long, long>(true, 0, 0, 0);
    }
}