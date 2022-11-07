using Amethyst.Plugins.Contract;

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

    public static List<ValueTuple<TrackerType, bool>> UpdateTrackerStates()
    {
        return new List<(TrackerType, bool)> { new(TrackerType.Tracker_Handed, true) };
    }

    public static List<ValueTuple<TrackerType, bool>> UpdateTrackerPoses()
    {
        return new List<(TrackerType, bool)> { new(TrackerType.Tracker_Handed, true) };
    }

    public static List<ValueTuple<TrackerType, bool>> RefreshTrackerPoses()
    {
        return new List<(TrackerType, bool)> { new(TrackerType.Tracker_Handed, true) };
    }

    public static bool RequestVRRestart()
    {
        return true;
    }

    public static ValueTuple<object, long, long, long> TestConnection()
    {
        return new ValueTuple<object, long, long, long>(true, 0, 0, 0);
    }
}