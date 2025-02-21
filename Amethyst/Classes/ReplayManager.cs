using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Threading;
using System.Threading.Tasks;
using Windows.Storage;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using Newtonsoft.Json;
using System.Diagnostics;
using LibMMD;
using System.Text.RegularExpressions;
using System.Text;
using static Amethyst.Classes.Shared.Events;
using System.Runtime.Serialization;

// ReSharper disable MethodSupportsCancellation

// ReSharper disable InvertIf
namespace Amethyst.Classes;

public class ReplayManager : INotifyPropertyChanged
{
    public ReplayManager()
    {
        // Read [Recordings] from data
        AsyncUtils.RunSync(ReadSavedRecordings);
    }

    public bool IsPlaying { get; private set; }
    public bool IsRecording { get; private set; }
    public string RecordingName => Recording?.Name;

    public SemaphoreSlim RecordingSemaphore = new(0);
    private Publisher CancelEvent { get; set; } = new();
    private TrackingRecording Recording { get; set; }
    public RecordingKeyframe Frame { get; set; }
    public List<TrackingRecording> Recordings { get; private set; }

    private bool _isWorking;

    public bool IsWorking
    {
        get => _isWorking || IsPlaying || IsRecording;
        private set => _isWorking = value;
    }

    public async Task Play(TrackingRecording recording)
    {
        if (IsPlaying || IsRecording)
            throw new InvalidOperationException("Replay already active");

        if (recording is null || !Recordings.Contains(recording))
            throw new ArgumentException("Recording not found", nameof(recording));

        if (recording.FrameCount < 2)
            throw new DataException("Insufficient recording data");

        CancelEvent = new Publisher();
        _isWorking = true;

        try
        {
            var loopStopWatch = new Stopwatch();
            const long targetLoopTime = TimeSpan.TicksPerMillisecond * 33;

            var tokenSource = new CancellationTokenSource();
            CancelEvent.OnCancelled += (_, _) => tokenSource.Cancel();

            Logger.Info("[Replay] Preparing the playback session...");
            Recording = recording;

            Logger.Info("[Replay] Setting up the data...");
            var frames = Recording.PreloadData
                ? (await Recording.Frames.ToListAsync()).AsEnumerable()
                : Recording.Frames; // Don't preload if not requested to

            Logger.Info("[Replay] Starting the replay...");
            foreach (var frame in frames.TakeWhile(_ =>
                         !tokenSource.Token.IsCancellationRequested))
            {
                Frame = frame; // Overwrite the frame
                var isPlayingBackup = IsPlaying;

                IsRecording = false;
                IsPlaying = true;

                if (!isPlayingBackup)
                {
                    Logger.Info("[Replay] Marking as currently playing...");
                    RefreshMainWindowEvent?.Set();
                }

                var diff = loopStopWatch.ElapsedTicks;

#pragma warning disable CA1806 // Do not ignore method results
                SystemShell.TimeBeginPeriod(1);
#pragma warning restore CA1806 // Do not ignore method results

                await Task.Delay(TimeSpan.FromTicks(Math.Clamp(
                    targetLoopTime - diff, 0, targetLoopTime)), tokenSource.Token);

#pragma warning disable CA1806 // Do not ignore method results
                SystemShell.TimeEndPeriod(1);

                loopStopWatch.Restart();
            }
        }
        catch (Exception ex)
        {
            Logger.Error(ex);
        }

        IsPlaying = false;
        IsRecording = false;
        _isWorking = false;

        Frame = null;
        Recording = null;

        Logger.Info("[Replay] Marking as not playing anymore...");
        RefreshMainWindowEvent?.Set();
    }

    public async Task Record(TrackingRecording recording, Quaternion toReferenceRotation,
        bool matchPosition = true, bool matchOrientation = true, bool playAsRaw = true)
    {
        if (IsPlaying || IsRecording)
            throw new InvalidOperationException("Replay already active");

        CancelEvent = new Publisher();
        RecordingSemaphore = new SemaphoreSlim(0);

        Logger.Info("[Replay] Preparing the recording session...");
        Frame = null;
        Recording = null;

        IsRecording = true;
        IsPlaying = false;

        Logger.Info("[Replay] Marking as currently recording...");
        RefreshMainWindowEvent?.Set();

        recording.MatchPosition = matchPosition;
        recording.MatchOrientation = matchOrientation;
        recording.PlayAsRaw = playAsRaw;
        recording.Rotation = toReferenceRotation;
        recording.Motion = new Vmd { FrameIndex = 0 };

        Recording = recording;

        var tokenSource = new CancellationTokenSource();
        CancelEvent.OnCancelled += (_, _) => tokenSource.Cancel();

        try
        {
            Logger.Info("[Replay] Recording the tracking data...");
            await RecordingSemaphore.WaitAsync(tokenSource.Token);
        }
        catch (Exception ex)
        {
            Logger.Error(ex);
        }

        if (!tokenSource.Token.IsCancellationRequested)
        {
            Logger.Info("[Replay] Saving the recorded data...");
            Recordings.Add(Recording);
            await WriteSavedRecordings();
        }

        IsPlaying = false;
        IsRecording = false;

        Frame = null;
        Recording = null;
    }

    public void Cancel()
    {
        CancelEvent?.Cancel();
    }

    public void Stop()
    {
        RecordingSemaphore.Release();
    }

    public List<TrackedJoint> GetJointsOr(string guid, List<TrackedJoint> fallback)
    {
        if (!IsPlaying || Recording?.Motion is null || !Recording.PlayAsRaw ||
            !Frame.Joints.TryGetValue(guid, out var value)) return fallback;

        Vector3? positionOffset =
            Recording.MatchPosition && value.Any(x => x.Role is TrackedJointType.JointHead) &&
            (fallback?.Any(x => x.Role is TrackedJointType.JointHead) ?? false)
                ? fallback.First(x => x.Role is TrackedJointType.JointHead).Position -
                  value.First(x => x.Role is TrackedJointType.JointHead).Position
                : null;

        Quaternion? rotationOffset =
            Recording.MatchOrientation ? Recording.Rotation : null;

        return value.Where(x => fallback?.Any(y => y.Role == x.Role) ?? true)
            .Select(x => x.Offset(positionOffset, rotationOffset)).ToList();
    }

    public List<TrackerBase> GetTrackersOr(List<TrackerBase> fallback)
    {
        if (IsRecording && Recording?.Motion?.BoneKeyFrames is not null)
            Recording.PutTrackers(fallback);

        if (!IsPlaying || Recording?.Motion is null || Recording.PlayAsRaw)
            return fallback;

        var value = Frame.Trackers;
        if (value is null) return fallback;

        Vector3? positionOffset =
            Recording.MatchPosition && Frame.HeadsetPose.HasValue &&
            AppPlugins.CurrentServiceEndpoint.HeadsetPoseInternal.HasValue
                ? AppPlugins.CurrentServiceEndpoint.HeadsetPoseInternal.Value.Position -
                  Frame.HeadsetPose.Value
                : null;

        Quaternion? rotationOffset =
            Recording.MatchOrientation ? Recording.Rotation : null;

        return value.Where(x => fallback?.Any(y => y.Role == x.Role) ?? true)
            .Select(x => x.Offset(positionOffset, rotationOffset)).ToList();
    }

    public bool GetTrackingStateOr(string guid, bool fallback)
    {
        return (IsPlaying && Recording?.Motion is not null) || fallback;
    }

    private readonly Stopwatch _loopStopWatch = new();

    public void MarkLoopTime()
    {
        if (!IsRecording || Recording?.Motion?.BoneKeyFrames is null) return;

        if (_loopStopWatch.IsRunning && _loopStopWatch.ElapsedMilliseconds < 25) return;
        else _loopStopWatch.Restart(); // We're recording the data in 30fps

        foreach (var (key, device) in AppPlugins.TrackingDevicesList.Where(x => x.Value.IsUsed))
            Recording.PutJoints(key, device.TrackedJoints); // Also rescan tracked joints

        Recording.PutHead(AppPlugins.CurrentServiceEndpoint.HeadsetPose?.Position);
    }

    private async Task ReadSavedRecordings()
    {
        try
        {
            Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);

            var recordingsFolder = await Interfacing.LocalFolder
                .CreateFolderAsync("Recordings", CreationCollisionOption.OpenIfExists);

            Recordings = []; // Empty the cached recordings prior to any operations
            foreach (var subfolder in await recordingsFolder.GetFoldersAsync())
                try
                {
                    var settingsFile = await subfolder.TryGetFileAsync("settings.json");
                    var motionFile = await subfolder.TryGetFileAsync("motion.vmd");
                    if (settingsFile is null || motionFile is null)
                    {
                        Logger.Warn($"Folder {subfolder.Name} contains invalid recording data!");
                        continue;
                    }

                    var recording = JsonConvert.DeserializeObject<TrackingRecording>(
                        await File.ReadAllTextAsync(settingsFile.Path));
                    if (recording is null)
                    {
                        Logger.Warn($"Settings.json of {subfolder.Name} was invalid!");
                        continue;
                    }

                    await using var stream = await motionFile.OpenStreamForReadAsync();
                    var motion = VmdParser.Parse(stream);
                    if (motion is null)
                    {
                        Logger.Warn($"Motion.vmd of {subfolder.Name} was invalid!");
                        continue;
                    }

                    recording.Motion = motion;
                    recording.FolderName = subfolder.Name;
                    Recordings.Add(recording);
                }
                catch (Exception e)
                {
                    Logger.Error($"Error reading {subfolder.Name}! Message: {e.Message}");
                }
        }
        catch (Exception e)
        {
            Logger.Error($"Error reading saved recordings! Message: {e.Message}");
        }
    }

    public async Task WriteSavedRecordings()
    {
        try
        {
            var recordingsFolder = await Interfacing.LocalFolder
                .CreateFolderAsync("Recordings", CreationCollisionOption.OpenIfExists);

            foreach (var recording in Recordings.ToList())
                try
                {
                    var saveFolder = await recordingsFolder.CreateFolderAsync(recording.FolderName,
                        CreationCollisionOption.OpenIfExists);

                    await File.WriteAllTextAsync((await saveFolder.CreateFileAsync(
                            "settings.json", CreationCollisionOption.ReplaceExisting)).Path,
                        JsonConvert.SerializeObject(recording, Formatting.Indented));

                    VmdParser.Save(recording.Motion, (await saveFolder.CreateFileAsync(
                        "motion.vmd", CreationCollisionOption.ReplaceExisting)).Path);
                }
                catch (Exception ex)
                {
                    Logger.Error(ex);
                }
        }
        catch (Exception e)
        {
            Logger.Error($"Error writing saved recordings! Message: {e.Message}");
        }
    }

    public event PropertyChangedEventHandler PropertyChanged;

    public virtual void OnPropertyChanged(string propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    public class Publisher
    {
        public delegate void NotifyEventHandler(object sender, EventArgs e);

        public event NotifyEventHandler OnCancelled;

        public void Cancel()
        {
            OnNotify(EventArgs.Empty);
        }

        protected virtual void OnNotify(EventArgs e)
        {
            OnCancelled?.Invoke(this, e);
        }
    }
}

public class TrackingRecordingBase
{
    public string Name { get; set; }
    public string FolderName { get; set; } = Guid.NewGuid().ToString().ToUpper();
}

public class TrackingRecording : TrackingRecordingBase
{
    public Quaternion Rotation { get; set; }
    [JsonIgnore] public Vmd Motion { get; set; }

    public bool PlayAsRaw { get; set; }
    public bool MatchPosition { get; set; }
    public bool MatchOrientation { get; set; }
    public bool PreloadData { get; set; } = true;

    public Dictionary<int, string> GuidSet { get; set; } = [];

    private RecordingKeyframe GetFrame(int index)
    {
        if (Interfacing.ReplayManager.IsRecording) return new RecordingKeyframe();
        return new RecordingKeyframe
        {
            HeadsetPose = this.GetHead(index, out var head) ? head : null,
            Trackers = this.GetTrackers(index, out var trackers) ? trackers : null,
            Joints = this.GetJoints(index, out var joints) ? joints : null
        };
    }

    [JsonIgnore]
    public int FrameCount => Interfacing.ReplayManager.IsRecording
        ? 0 // Don't enumerate anything while recording
        : Motion?.BoneKeyFrames?.Max(x => x.FrameIndex) ?? 0;

    [JsonIgnore]
    public IEnumerable<RecordingKeyframe> Frames =>
        Interfacing.ReplayManager.IsRecording
            ? [] // Don't enumerate anything while recording
            : Motion?.BoneKeyFrames is not null
                ? Enumerable.Range(0, Motion.BoneKeyFrames.Max(x => x.FrameIndex)).Select(GetFrame)
                : null; // Get subsequent frames from the recording as IEnumerable
}

public class RecordingKeyframe
{
    public Vector3? HeadsetPose { get; set; }
    public List<TrackerBase> Trackers { get; set; } = [];

    // Dictionary because we need to record data from all used devices
    public Dictionary<string, List<TrackedJoint>> Joints { get; set; } = [];
}

public static partial class VmdExtensions
{
    public static async Task<StorageFile> TryGetFileAsync(this StorageFolder folder, string name)
    {
        try
        {
            return await folder.GetFileAsync(name);
        }
        catch (Exception ex)
        {
            return null;
        }
    }

    public static bool GetTrackers(this TrackingRecording recording, int frame, out List<TrackerBase> trackers)
    {
        var vmd = recording.Motion;
        if (vmd is null)
        {
            trackers = [];
            return false;
        }

        try
        {
            var result = vmd.BoneKeyFrames
                .Where(x => x.FrameIndex == frame)
                .Where(x => x.Name.StartsWith('.'))
                .Where(x => TypeUtils.MmdTrackerTypeDictionaryInternal.ContainsKey(x.Name))
                .Select(x => new TrackerBase
                {
                    Role = TypeUtils.MmdTrackerTypeDictionaryInternal[x.Name],
                    Serial = TypeUtils.TrackerTypeRoleSerialDictionary[
                        TypeUtils.MmdTrackerTypeDictionaryInternal[x.Name]],
                    Position = x.Position.Vec(),
                    Orientation = x.RotationQuaternion.Vec(),
                    ConnectionState = true,
                    TrackingState = TrackedJointState.StateTracked
                }).ToList();

            if (!result.Any())
                result = vmd.BoneKeyFrames
                    .Where(x => x.FrameIndex == frame)
                    .Where(x => TypeUtils.MmdTrackerTypeDictionary.ContainsKey(x.Name))
                    .Select(x => new TrackerBase
                    {
                        Role = TypeUtils.MmdTrackerTypeDictionary[x.Name],
                        Serial = TypeUtils.TrackerTypeRoleSerialDictionary[
                            TypeUtils.MmdTrackerTypeDictionary[x.Name]],
                        Position = x.Position.Vec(),
                        Orientation = x.RotationQuaternion.Vec(),
                        ConnectionState = true,
                        TrackingState = TrackedJointState.StateTracked
                    }).ToList();

            trackers = result;
            return result.Any();
        }
        catch (Exception ex)
        {
            Logger.Error(ex);
            trackers = [];
            return false;
        }
    }

    public static bool GetJoints(this TrackingRecording recording, int frame, out Dictionary<string, List<TrackedJoint>> joints)
    {
        var vmd = recording.Motion;
        if (vmd is null)
        {
            joints = [];
            return false;
        }

        try
        {
            var result = vmd.BoneKeyFrames
                .Where(x => x.FrameIndex == frame && x.Name.StartsWith('['))
                .Where(x => TypeUtils.MmdJointTypeDictionaryInternal
                    .ContainsKey(x.Name.Extract(out _, out var name) ? name : string.Empty))
                .GroupBy(x => x.Name.Extract(out var index, out _) ? index : -1)
                .Where(x => x.Key >= 0 && recording.GuidSet.ContainsKey(x.Key))
                .ToDictionary(x => recording.GuidSet[x.Key], x => x.Select(y =>
                    new TrackedJoint
                    {
                        Position = y.Position.Vec(),
                        Orientation = y.RotationQuaternion.Vec(),
                        TrackingState = TrackedJointState.StateTracked,
                        Role = y.Name.Extract(out _, out var name)
                            ? TypeUtils.MmdJointTypeDictionaryInternal[name]
                            : TrackedJointType.JointManual
                    }).ToList());

            if (!result.Any())
                result = new Dictionary<string, List<TrackedJoint>>
                {
                    {
                        AppPlugins.BaseTrackingDevice.Guid,
                        vmd.BoneKeyFrames
                            .Where(x => x.FrameIndex == frame)
                            .Where(x => TypeUtils.MmdJointTypeDictionary.ContainsKey(x.Name))
                            .Select(x => new TrackedJoint
                            {
                                Role = TypeUtils.MmdJointTypeDictionary[x.Name],
                                Position = x.Position.Vec(),
                                Orientation = x.RotationQuaternion.Vec(),
                                TrackingState = TrackedJointState.StateTracked
                            }).ToList()
                    }
                };

            joints = result;
            return result.Any();
        }
        catch (Exception ex)
        {
            Logger.Error(ex);
            joints = [];
            return false;
        }
    }

    public static bool GetHead(this TrackingRecording recording, int frame, out Vector3 head)
    {
        var vmd = recording.Motion;
        if (vmd is null)
        {
            head = Vector3.Zero;
            return false;
        }

        var result = vmd.BoneKeyFrames.FirstOrDefault(
            x => x.FrameIndex == frame && x.Name is "{Head}",
            new VmdBoneKeyFrame()).Position.Vec();

        head = result;
        return result != Vector3.Zero;
    }

    public static void PutTrackers(this TrackingRecording recording, List<TrackerBase> trackers)
    {
        if (recording?.Motion?.BoneKeyFrames is null) return;

        var vmd = recording.Motion;
        if (vmd is null) return;

        if (vmd.FrameIndex < vmd.TrackersFrameIndex) return;
        vmd.TrackersFrameIndex += 1; // For the next frame

        foreach (var tracker in trackers)
        {
            vmd.BoneKeyFrames.Add(new VmdBoneKeyFrame
            {
                FrameIndex = vmd.TrackersFrameIndex,
                Position = new Vec3f(tracker.Position),
                RotationQuaternion = new Vec4f(tracker.Orientation),
                Name = TypeUtils.TrackerTypeMmdDictionaryInternal[tracker.Role]
            });

            if (AppPlugins.BaseTrackingDevice.Guid.Contains("KINECT")) continue;
            vmd.BoneKeyFrames.Add(new VmdBoneKeyFrame
            {
                FrameIndex = vmd.TrackersFrameIndex,
                // Position = new Vec3f(tracker.Position),
                RotationQuaternion = new Vec4f(tracker.Orientation),
                Name = TypeUtils.TrackerTypeMmdDictionary[tracker.Role]
            });
        }
    }

    public static void PutJoints(this TrackingRecording recording, string guid, List<TrackedJoint> joints)
    {
        if (recording?.Motion?.BoneKeyFrames is null) return;

        var vmd = recording.Motion;
        if (vmd is null) return;

        foreach (var joint in joints)
        {
            vmd.BoneKeyFrames.Add(new VmdBoneKeyFrame
            {
                FrameIndex = vmd.FrameIndex,
                Position = new Vec3f(joint.Position),
                RotationQuaternion = new Vec4f(joint.Orientation),
                Name = $"[{recording.GuidSet.Put(guid)}]" +
                       $"{TypeUtils.JointTypeMmdDictionaryInternal[joint.Role]}"
            });

            if (!AppPlugins.BaseTrackingDevice.Guid.Contains("KINECT") ||
                guid != AppPlugins.BaseTrackingDevice.Guid) continue;
            vmd.BoneKeyFrames.Add(new VmdBoneKeyFrame
            {
                FrameIndex = vmd.FrameIndex,
                // Position = new Vec3f(joint.Position),
                RotationQuaternion = new Vec4f(joint.Orientation),
                Name = TypeUtils.JointTypeMmdDictionary[joint.Role]
            });
        }
    }

    public static void PutHead(this TrackingRecording recording, Vector3? head)
    {
        if (recording?.Motion?.BoneKeyFrames is null) return;

        var vmd = recording.Motion;
        if (vmd is null) return;

        if (head is not null)
            vmd.BoneKeyFrames.Add(new VmdBoneKeyFrame
            {
                Name = "{Head}",
                FrameIndex = vmd.FrameIndex,
                Position = new Vec3f(head.Value),
                RotationQuaternion = new Vec4f(Quaternion.Identity)
            });

        vmd.FrameIndex += 1;
    }

    public static int? IndexOf<T>(this Dictionary<int, T> dict, T value) where T : IComparable<T>
    {
        foreach (var item in dict
                     .Where(item => EqualityComparer<T>.Default.Equals(item.Value, value)))
            return item.Key;

        return null;
    }

    public static int Put<T>(this Dictionary<int, T> dict, T value) where T : IComparable<T>
    {
        var index = dict.IndexOf(value);
        if (index is null)
        {
            var newIndex = dict.Keys.Any() ? dict.Keys.Max() + 1 : 0;
            dict[newIndex] = value;
            return newIndex;
        }

        return index.Value;
    }

    public static bool Extract(this string input, out int x, out string something)
    {
        x = 0;
        something = string.Empty;

        var match = MmdStringRegex().Match(input);
        if (match.Success)
            if (int.TryParse(match.Groups[1].Value, out x))
            {
                something = match.Groups[2].Value;
                return true;
            }

        return false;
    }

    public static TrackedJoint Offset(this TrackedJoint joint, Vector3? v, Quaternion? q)
    {
        return joint.Offset(v).Offset(q);
    }

    public static TrackedJoint Offset(this TrackedJoint joint, Vector3? v)
    {
        if (v is null) return joint;
        return new TrackedJoint
        {
            Name = joint.Name,
            Role = joint.Role,
            Position = joint.Position + v.Value,
            Orientation = joint.Orientation,
            TrackingState = joint.TrackingState
        };
    }

    public static TrackedJoint Offset(this TrackedJoint joint, Quaternion? q)
    {
        if (q is null) return joint;
        return new TrackedJoint
        {
            Name = joint.Name,
            Role = joint.Role,
            Position = Vector3.Transform(joint.Position, q.Value),
            Orientation = q.Value * joint.Orientation,
            TrackingState = joint.TrackingState
        };
    }

    public static TrackerBase Offset(this TrackerBase tracker, Vector3? v, Quaternion? q)
    {
        return tracker.Offset(v).Offset(q);
    }

    public static TrackerBase Offset(this TrackerBase tracker, Vector3? v)
    {
        if (v is null) return tracker;
        return new TrackerBase
        {
            Serial = tracker.Serial,
            Role = tracker.Role,
            Position = tracker.Position + v.Value,
            Orientation = tracker.Orientation,
            TrackingState = tracker.TrackingState,
            ConnectionState = true
        };
    }

    public static TrackerBase Offset(this TrackerBase tracker, Quaternion? q)
    {
        if (q is null) return tracker;
        return new TrackerBase
        {
            Serial = tracker.Serial,
            Role = tracker.Role,
            Position = Vector3.Transform(tracker.Position, q.Value),
            Orientation = q.Value * tracker.Orientation,
            TrackingState = tracker.TrackingState,
            ConnectionState = true
        };
    }

    public static async Task<List<T>> ToListAsync<T>(this IEnumerable<T> largeEnumerable)
    {
        return await Task.Run(largeEnumerable.ToList);
    }

    [GeneratedRegex("^\\[(\\d+)\\](.+)$")]
    private static partial Regex MmdStringRegex();
}