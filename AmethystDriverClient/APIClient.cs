using System.Diagnostics;
using Amethyst.Driver.API;
using Google.Protobuf.WellKnownTypes;
using Grpc.Core;

namespace Amethyst.Driver.Client;

public static class DriverClient
{
    private static Channel? _channel;
    private static IK2DriverService.IK2DriverServiceClient? _service;

    /// <summary>
    ///     Connects socket object to selected port, AME uses 7135
    /// </summary>
    /// <param name="target">AME Server Driver IP:Port</param>
    /// <returns>0: OK, -1: Exception, -2: Channel fail, -3: Stub failure</returns>
    public static int InitAmethystServer(string target = "localhost:7135")
    {
        try
        {
            // Compose the channel arguments
            var channelOptions = new List<ChannelOption>
            {
                new("grpc.keepalive_time_ms", 7200000),
                new("grpc.keepalive_timeout_ms", 20000),
                new("grpc.keepalive_permit_without_calls", 1),
                new("grpc.http2.min_ping_interval_without_data_ms", 300000),
                new("grpc.http2.min_time_between_pings_ms", 10000)
            };

            // Create the RPC channel
            _channel = new Channel(target, ChannelCredentials.Insecure, channelOptions);

            // Create the RPC messaging service
            _service = new IK2DriverService.IK2DriverServiceClient(_channel);
        }
        catch (Exception)
        {
            return -10;
        }

        return 0;
    }

    public static async Task<IEnumerable<(TrackerType Role, bool State)>?> UpdateTrackerStates(
        IEnumerable<(TrackerType Role, bool State)> states, bool wantReply = true)
    {
        try
        {
            var call = _service?.SetTrackerStateVector();
            if (call is null) return wantReply ? new List<(TrackerType Role, bool State)>() : null;

            foreach (var (role, state) in states)
                await call.RequestStream.WriteAsync(
                    new ServiceRequest
                    {
                        WantReply = wantReply,
                        TrackerStateTuple = new Service_TrackerStatePair { State = state, TrackerType = role }
                    });

            await call.RequestStream.CompleteAsync();
            return wantReply
                ? call.ResponseStream.ReadAllAsync().ToBlockingEnumerable()
                    .Select(x => (x.TrackerType, x.State))
                : null;
        }
        catch (Exception)
        {
            return wantReply ? new List<(TrackerType Role, bool State)>() : null;
        }
    }

    public static async Task<IEnumerable<(TrackerType Role, bool State)>?> UpdateTrackerPoses(
        IEnumerable<K2TrackerBase> trackerBases, bool wantReply = true)
    {
        try
        {
            using var call = _service?.UpdateTrackerVector();
            if (call is null) return wantReply ? new List<(TrackerType Role, bool State)>() : null;

            foreach (var tracker in trackerBases)
                await call.RequestStream.WriteAsync(
                    new ServiceRequest
                    {
                        WantReply = true,
                        TrackerBase = tracker
                    });

            await call.RequestStream.CompleteAsync();
            return wantReply
                ? call.ResponseStream.ReadAllAsync().ToBlockingEnumerable()
                    .Select(x => (x.TrackerType, x.State))
                : null;
        }
        catch (Exception)
        {
            return wantReply ? new List<(TrackerType Role, bool State)>() : null;
        }
    }

    public static async Task<IEnumerable<(TrackerType Role, bool State)>?> RefreshTrackerPoses(
        IEnumerable<TrackerType> trackers, bool wantReply = true)
    {
        try
        {
            using var call = _service?.SetTrackerStateVector();
            if (call is null) return wantReply ? new List<(TrackerType Role, bool State)>() : null;

            foreach (var role in trackers)
                await call.RequestStream.WriteAsync(
                    new ServiceRequest
                    {
                        WantReply = true,
                        TrackerStateTuple = new Service_TrackerStatePair { TrackerType = role }
                    });

            await call.RequestStream.CompleteAsync();
            return wantReply
                ? call.ResponseStream.ReadAllAsync().ToBlockingEnumerable()
                    .Select(x => (x.TrackerType, x.State))
                : null;
        }
        catch (Exception)
        {
            return wantReply ? new List<(TrackerType Role, bool State)>() : null;
        }
    }

    public static bool? RequestVrRestart(string reason, bool wantReply = false)
    {
        try
        {
            return _service?.RequestVRRestart(new ServiceRequest
                { Message = reason, WantReply = wantReply }).State;
        }
        catch (Exception)
        {
            return wantReply ? false : null;
        }
    }

    public static async Task<(Status Status, long SendTimestamp, long ReceiveTimestamp, long ElpasedTime)>
        TestConnection()
    {
        // Note: times are fd up because c++ server assumes UNIX timestamps
        try
        {
            // Grab the current time and send the message
            var messageSendTimeStopwatch = new Stopwatch();
            var sendTime = DateTime.Now.Ticks;

            messageSendTimeStopwatch.Start();
            var response = _service?.PingDriverService(new Empty());
            messageSendTimeStopwatch.Stop();

            // Return tuple with response and elapsed time
            return (new Status(StatusCode.OK, "Ok"),
                sendTime, response?.ReceivedTimestamp.FromEpoch() ?? 0,
                messageSendTimeStopwatch.ElapsedTicks);
        }
        catch (Exception)
        {
            return (new Status(StatusCode.Unavailable, "Exception"), 0, 0, 0);
        }
    }

    private static long FromEpoch(this long ms)
    {
        var epoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
        return epoch.AddTicks(ms * (TimeSpan.TicksPerMillisecond / 1000)).Ticks;
    }
}