using System;
using System.Collections.Generic;
using System.Configuration;
using System.Diagnostics;
using System.Linq;
using System.Numerics;
using System.Threading.Tasks;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using AmethystSupport;

namespace Amethyst.Classes;

public static class Main
{
    private static bool _bInitialized; // Backup initialized? value

    public static void FreezeActionToggled(object o, EventArgs eventArgs)
    {
        Logger.Info("[Input Actions] Input: Tracking freeze toggled");
        Interfacing.IsTrackingFrozen = !Interfacing.IsTrackingFrozen;

        // Play a Sound and Update UI
        AppSounds.PlayAppSound(
            Interfacing.IsTrackingFrozen ? AppSounds.AppSoundType.ToggleOff : AppSounds.AppSoundType.ToggleOn);

        if (Shared.General.ToggleFreezeButton is not null)
            Shared.Main.DispatcherQueue.TryEnqueue(() =>
            {
                Shared.General.GeneralTabSetupFinished = false; // Boiler
                Shared.General.ToggleFreezeButton.IsChecked = Interfacing.IsTrackingFrozen;
                Shared.General.ToggleFreezeButton.Content = Interfacing.IsTrackingFrozen
                    ? Interfacing.LocalizedJsonString("/GeneralPage/Buttons/Skeleton/Unfreeze")
                    : Interfacing.LocalizedJsonString("/GeneralPage/Buttons/Skeleton/Freeze");

                Shared.General.GeneralTabSetupFinished = true; // Boiler end
            });
    }

    public static void FlipActionToggled(object o, EventArgs eventArgs)
    {
        Logger.Info("[Input Actions] Input: Flip toggled");

        // Also validate the result
        AppData.Settings.IsFlipEnabled =
            TrackingDevices.BaseTrackingDevice.IsFlipSupported && AppData.Settings.IsFlipEnabled;

        // Save settings
        AppData.Settings.SaveSettings();

        // Play a Sound and Update UI
        AppSounds.PlayAppSound(
            AppData.Settings.IsFlipEnabled
                ? AppSounds.AppSoundType.ToggleOff
                : AppSounds.AppSoundType.ToggleOn);

        if (Shared.Settings.FlipToggle is not null)
            Shared.Main.DispatcherQueue.TryEnqueue(() =>
            {
                Shared.Settings.SettingsTabSetupFinished = false; // Boiler
                Shared.Settings.FlipToggle.IsOn = AppData.Settings.IsFlipEnabled;
                Shared.Settings.SettingsTabSetupFinished = true; // Boiler end
            });
    }

    private static void UpdateTrackingDevices()
    {
        // Update the base device
        if (!TrackingDevices.BaseTrackingDevice.IsSelfUpdateEnabled)
            TrackingDevices.BaseTrackingDevice.Update();

        // Copy needed device transform poses
        {
            // Create a ref to the hook [head] joint (or the first, or default joint)
            var headJoint = TrackingDevices.BaseTrackingDevice.TrackedJoints
                .FirstOrDefault(x => x.Role == TrackedJointType.JointHead,
                    TrackingDevices.BaseTrackingDevice.TrackedJoints.FirstOrDefault(new TrackedJoint()));

            // Create a ref to the rto [waist] joint (or the first, or default joint)
            var waistJoint = TrackingDevices.BaseTrackingDevice.TrackedJoints
                .FirstOrDefault(x => x.Role == TrackedJointType.JointSpineWaist,
                    TrackingDevices.BaseTrackingDevice.TrackedJoints.FirstOrDefault(new TrackedJoint()));

            // Copy the hook joint [head], relative transform origin joint [waist] pose
            Interfacing.DeviceHookJointPosition[TrackingDevices.BaseTrackingDevice.Guid] =
                (headJoint.Position, headJoint.Orientation);
            Interfacing.DeviceRelativeTransformOrigin[TrackingDevices.BaseTrackingDevice.Guid] =
                (waistJoint.Position, waistJoint.Orientation);
        }

        // Update override devices (optionally)
        foreach (var device in AppData.Settings.OverrideDevicesGuidMap.Select(overrideGuid =>
                     TrackingDevices.GetDevice(overrideGuid).Device))
        {
            if (!device.IsSelfUpdateEnabled) device.Update();

            // Create a ref to the hook [head] joint (or the first, or default joint)
            var headJoint = device.TrackedJoints.FirstOrDefault(
                x => x.Role == TrackedJointType.JointHead,
                device.TrackedJoints.FirstOrDefault(new TrackedJoint()));

            // Create a ref to the rto [waist] joint (or the first, or default joint)
            var waistJoint = device.TrackedJoints.FirstOrDefault(
                x => x.Role == TrackedJointType.JointSpineWaist,
                device.TrackedJoints.FirstOrDefault(new TrackedJoint()));

            // Copy the hook joint [head], relative transform origin joint [waist] pose
            Interfacing.DeviceHookJointPosition[device.Guid] = (headJoint.Position, headJoint.Orientation);
            Interfacing.DeviceRelativeTransformOrigin[device.Guid] = (waistJoint.Position, waistJoint.Orientation);
        }

        // Update service endpoints
        TrackingDevices.CurrentServiceEndpoint?.Heartbeat();
    }

    private static async Task UpdateServerTrackers()
    {
        // Update only if we're connected and running
        if (!Interfacing.AppTrackersSpawned || Interfacing.ServiceEndpointFailure) return;

        // Update status right after any change
        if (_bInitialized != Interfacing.AppTrackersInitialized)
            // Try 3 times (cause why not)
            for (var i = 0; i < 3; i++)
            {
                // Update status in server
                await TrackingDevices.CurrentServiceEndpoint.SetTrackerStates(AppData.Settings.TrackersVector
                    .Where(tracker => tracker.IsActive)
                    .Select(tracker =>
                    {
                        var trackerBase = tracker.GetTrackerBase();
                        trackerBase.ConnectionState = Interfacing.AppTrackersInitialized;
                        return trackerBase;
                    }));

                // Update internal status
                _bInitialized = Interfacing.AppTrackersInitialized;
            }

        // That's all if we're not running!
        if (!Interfacing.AppTrackersInitialized) return;

        // If the tracing's actually running
        if (!Interfacing.IsTrackingFrozen || AppData.Settings.FreezeLowerBodyOnly)
        {
            // Update position & orientation filters
            AppData.Settings.TrackersVector.ToList().ForEach(tracker => tracker.UpdateFilters());

            // Update pose w/ filtering, options and calibration
            // Note: only position gets calibrated INSIDE trackers

            // If we've frozen everything but elbows
            var updateLowerBody = !(Interfacing.IsTrackingFrozen && AppData.Settings.FreezeLowerBodyOnly);

            // Update the [server] trackers
            await TrackingDevices.CurrentServiceEndpoint.UpdateTrackerPoses(
                AppData.Settings.TrackersVector.Where(tracker => tracker.IsActive)
                    .Where(x => (int)TypeUtils.TrackerTypeJointDictionary[x.Role] < 16 || updateLowerBody)
                    .Select(tracker => tracker.GetTrackerBase(
                        tracker.PositionTrackingFilterOption, tracker.OrientationTrackingFilterOption)), false);
        }

        // Scan for already-added body trackers from other apps
        // (If any found, disable corresponding ame's trackers/pairs)
        if (Interfacing.AlreadyAddedTrackersScanRequested)
        {
            // Mark the request as done
            Interfacing.AlreadyAddedTrackersScanRequested = false;

            // Run the worker (if applicable)
            if (AppData.Settings.CheckForOverlappingTrackers &&
                !Interfacing.IsAlreadyAddedTrackersScanRunning)
                Shared.Main.DispatcherQueue.TryEnqueue(async () =>
                {
                    Interfacing.IsAlreadyAddedTrackersScanRunning = true;
                    var wereChangesMade = false; // At least not yet

                    // Search for 
                    foreach (var tracker in AppData.Settings.TrackersVector
                                 .Where(tracker => tracker.IsActive)
                                 .Where(tracker => TrackingDevices.CurrentServiceEndpoint
                                     .GetTrackerPose(TypeUtils.TrackerTypeSerialDictionary[tracker.Role]) != null))
                    {
                        // Make actual changes (self-updates)
                        tracker.IsActive = false;

                        // Try even 5 times (cause why not)
                        for (var i = 0; i < 5; i++)
                        {
                            var trackerBase = tracker.GetTrackerBase();
                            trackerBase.ConnectionState = false;

                            await TrackingDevices.CurrentServiceEndpoint.SetTrackerStates(new[] { trackerBase });
                            await Task.Delay(25);
                        }

                        // Check and save settings
                        AppData.Settings.CheckSettings();
                        AppData.Settings.SaveSettings();
                        wereChangesMade = true;
                    }

                    // Check if anything's changed
                    Interfacing.IsAlreadyAddedTrackersScanRunning = false;
                    if (!wereChangesMade) return;

                    Interfacing.ShowToast(
                        Interfacing.LocalizedJsonString(
                            "/SharedStrings/Toasts/TrackersAutoDisabled/Title"),
                        Interfacing.LocalizedJsonString(
                            "/SharedStrings/Toasts/TrackersAutoDisabled"),
                        true, "focus_trackers");

                    Interfacing.ShowServiceToast(
                        Interfacing.LocalizedJsonString(
                            "/SharedStrings/Toasts/TrackersAutoDisabled/Title"),
                        Interfacing.LocalizedJsonString(
                            "/SharedStrings/Toasts/TrackersAutoDisabled"));
                });
        }
    }

    // Update trackers inside the app here
    private static void UpdateAppTrackers()
    {
        // This is where we do EVERYTHING pose related.
        // All positions and rotations are calculated here,
        // depending on calibration val and configuration

        // Calculate ALL poses for the base (first) device here

        // We can precompute the threshold as a dot product value
        // as the dot product is also defined as |a||b|cos(x)
        // and since we're using unit vectors... |a||b| = 1
        const double flipThreshold = 0.4226182; // cos(65°)

        // Base device
        {
            // Get the currently tracking device
            var device = TrackingDevices.BaseTrackingDevice;

            var extFlip =
                AppData.Settings.IsFlipEnabled &&
                AppData.Settings.IsExternalFlipEnabled;

            var extFlipInternal =
                AppData.Settings.TrackersVector[0].IsActive &&
                AppData.Settings.TrackersVector[0].IsOrientationOverridden;

            // Compose flip
            var dotFacing =
                Support.OrientationDot(
                    // Check for external-flip
                    extFlip

                        // Check for internal overrides
                        ? extFlipInternal

                            // Overriden internal amethyst tracker
                            ? AppData.Settings.TrackersVector[0].Orientation.Projected()

                            // External VR waist tracker
                            : Interfacing.GetVrTrackerPoseCalibrated("waist").Orientation.Projected()

                        // Default: VR HMD orientation
                        : Interfacing.Plugins.GetHmdPose.Orientation.Projected(),

                    // Check for external-flip
                    extFlip

                        // If ExtFlip is enabled compare to its calibration
                        ? AppData.Settings.ExternalFlipCalibrationMatrix.Projected()

                        // Default: use the default calibration rotation
                        : AppData.Settings.DeviceCalibrationRotationMatrices.GetValueOrDefault(
                            AppData.Settings.TrackingDeviceGuid, Quaternion.Identity).Projected());

            // Not in transition angle area, can compute
            if (Math.Abs(dotFacing) >= flipThreshold)
                Interfacing.BaseFlip = dotFacing < 0.0;

            // Overwrite flip value depending on the device & settings
            // (Device type check should have already been done tho...)
            if (!AppData.Settings.IsFlipEnabled || !device.IsFlipSupported) Interfacing.BaseFlip = false;

            // Loop over all the added app-wise trackers
            foreach (var tracker in AppData.Settings.TrackersVector)
            {
                // Compute flip for this one joint
                var isJointFlipped = Interfacing.BaseFlip && // The device is flipped
                                     device.TrackedJoints[(int)tracker.SelectedTrackedJointId].Role !=
                                     TrackedJointType.JointManual; // The joint role isn't manual

                // Get the bound joint used for this tracker
                var joint = isJointFlipped

                    // If flip : the device contains a joint for the mirrored role
                    ? device.TrackedJoints.FirstOrDefault(
                        x => x.Role == TypeUtils.FlippedJointTypeDictionary[
                            device.TrackedJoints[(int)tracker.SelectedTrackedJointId].Role],
                        // Otherwise, default to the non-flipped (selected one)
                        device.TrackedJoints[(int)tracker.SelectedTrackedJointId])

                    // If no flip
                    : device.TrackedJoints[(int)tracker.SelectedTrackedJointId];

                // Copy the orientation to the tracker
                tracker.Orientation = tracker.OrientationTrackingOption switch
                {
                    // Optionally overwrite the rotation with HMD orientation
                    // Not the "calibrated" variant, as the fix will be applied after everything else
                    JointRotationTrackingOption.FollowHmdRotation =>
                        Quaternion.CreateFromYawPitchRoll(Support.QuaternionYaw(
                            Interfacing.Plugins.GetHmdPose.Orientation.Projected()), 0, 0),

                    // Optionally overwrite the rotation with NONE
                    JointRotationTrackingOption.DisableJointRotation => Quaternion.Identity,

                    // Default
                    _ => isJointFlipped
                        ? Quaternion.Inverse(joint.Orientation)
                        : joint.Orientation
                };

                // Copy the previous orientation to the tracker
                tracker.PreviousOrientation = tracker.OrientationTrackingOption switch
                {
                    // Optionally overwrite the rotation with HMD orientation
                    // Not the "calibrated" variant, as the fix will be applied after everything else
                    JointRotationTrackingOption.FollowHmdRotation =>
                        Quaternion.CreateFromYawPitchRoll(Support.QuaternionYaw(
                            Interfacing.Plugins.GetHmdPose.Orientation.Projected()), 0, 0),

                    // Optionally overwrite the rotation with NONE
                    JointRotationTrackingOption.DisableJointRotation => Quaternion.Identity,

                    // Default
                    _ => isJointFlipped
                        ? Quaternion.Inverse(joint.PreviousOrientation)
                        : joint.PreviousOrientation
                };

                // If math-based orientation is supported, overwrite the orientation with it
                if (device.IsAppOrientationSupported &&
                    tracker.Role is TrackerType.TrackerLeftFoot or TrackerType.TrackerRightFoot)
                    tracker.Orientation = tracker.OrientationTrackingOption switch
                    {
                        JointRotationTrackingOption.SoftwareCalculatedRotation =>
                            Support.FeetSoftwareOrientation(
                                    device.TrackedJoints.First(x =>
                                            x.Role == TypeUtils.FlipJointType(TrackedJointType.JointFootLeft,
                                                (tracker.Role != TrackerType.TrackerLeftFoot) ^ isJointFlipped))
                                        .Position.Projected(),
                                    device.TrackedJoints.First(x =>
                                            x.Role == TypeUtils.FlipJointType(TrackedJointType.JointFootTipLeft,
                                                (tracker.Role != TrackerType.TrackerLeftFoot) ^ isJointFlipped))
                                        .Position.Projected(),
                                    device.TrackedJoints.First(x =>
                                            x.Role == TypeUtils.FlipJointType(TrackedJointType.JointKneeLeft,
                                                (tracker.Role != TrackerType.TrackerLeftFoot) ^ isJointFlipped))
                                        .Position.Projected())
                                .Q().Inversed(isJointFlipped), // Also inverse if flipped (via an extension)

                        JointRotationTrackingOption.SoftwareCalculatedRotationV2 =>
                            Support.FeetSoftwareOrientationV2(
                                    device.TrackedJoints.First(x =>
                                            x.Role == TypeUtils.FlipJointType(TrackedJointType.JointFootLeft,
                                                (tracker.Role != TrackerType.TrackerLeftFoot) ^ isJointFlipped))
                                        .Position.Projected(),
                                    device.TrackedJoints.First(x =>
                                            x.Role == TypeUtils.FlipJointType(TrackedJointType.JointKneeLeft,
                                                (tracker.Role != TrackerType.TrackerLeftFoot) ^ isJointFlipped))
                                        .Position.Projected())
                                .Q().Inversed(isJointFlipped), // Also inverse if flipped (via an extension)

                        _ => tracker.Orientation
                    };

                // Apply calibration-related flipped orientation fixes
                if (isJointFlipped && tracker.OrientationTrackingOption
                    != JointRotationTrackingOption.FollowHmdRotation)
                {
                    // Note: only in flip mode
                    tracker.Orientation = Support
                        .FixFlippedOrientation(tracker.Orientation.Projected()).Q();
                    tracker.PreviousOrientation = Support
                        .FixFlippedOrientation(tracker.PreviousOrientation.Projected()).Q();
                }

                // Apply playspace-related orientation fixes
                if (tracker.OrientationTrackingOption ==
                    JointRotationTrackingOption.FollowHmdRotation)
                {
                    // Offset to fit the playspace
                    tracker.Orientation = tracker.Orientation;
                    tracker.PreviousOrientation = tracker.PreviousOrientation;
                }

                // Push raw positions and timestamps
                tracker.Position = joint.Position;
                tracker.PreviousPosition = joint.PreviousPosition;

                tracker.PoseTimestamp = joint.PoseTimestamp;
                tracker.PreviousPoseTimestamp = joint.PreviousPoseTimestamp;

                // Optionally disable position filtering (on-demand)
                tracker.NoPositionFilteringRequested = device.IsPositionFilterBlockingEnabled;

                // Push raw physics (if valid)
                if (device.IsPhysicsOverrideEnabled)
                {
                    tracker.PoseVelocity = joint.Velocity;
                    tracker.PoseAcceleration = joint.Acceleration;
                    tracker.PoseAngularVelocity = joint.AngularVelocity;
                    tracker.PoseAngularAcceleration = joint.AngularAcceleration;

                    tracker.OverridePhysics = true;
                }
                // If not and the tracker is not overriden
                else if (!tracker.IsPositionOverridden)
                {
                    tracker.OverridePhysics = false;
                }
            }
        }

        // Override devices, loop over all overrides
        foreach (var device in TrackingDevices.TrackingDevicesList.Values.Where(
                     device => AppData.Settings.OverrideDevicesGuidMap.Contains(device.Guid)))
        {
            // Strategy:
            //  overwrite base device's poses, optionally apply flip
            //  note that unlike in legacy versions, flip isn't anymore
            //  applied on pose pushes; this will allow us to apply
            //  two (or even more) independent flips, after the base

            // Currently, flipping override devices IS NOT supported
            Interfacing.OverrideFlip = false; // SHOULD WE ENABLE???
            // Currently, flipping override devices IS NOT supported

            // Loop over all the added app-wise trackers
            // which are overridden in any way (pos || ori)
            // and are managed by the current override device
            foreach (var tracker in AppData.Settings.TrackersVector
                         .Where(tracker => tracker.IsOverridden &&
                                           tracker.OverrideGuid == device.Guid))
            {
                // Compute flip for this one joint
                var isJointFlipped = Interfacing.OverrideFlip && // The device is flipped
                                     device.TrackedJoints[(int)tracker.OverrideJointId].Role !=
                                     TrackedJointType.JointManual; // The joint role isn't manual

                // Get the bound joint used for this tracker
                var joint = isJointFlipped

                    // If flip : the device contains a joint for the mirrored role
                    ? device.TrackedJoints.FirstOrDefault(
                        x => x.Role == TypeUtils.FlippedJointTypeDictionary[
                            device.TrackedJoints[(int)tracker.OverrideJointId].Role],
                        // Otherwise, default to the non-flipped (selected one)
                        device.TrackedJoints[(int)tracker.OverrideJointId])

                    // If no flip
                    : device.TrackedJoints[(int)tracker.OverrideJointId];

                // If overridden w/ orientation and the selected option is 'device'
                if (tracker.IsOrientationOverridden && tracker.OrientationTrackingOption ==
                    JointRotationTrackingOption.DeviceInferredRotation)
                {
                    // Standard, also apply calibration-related flipped orientation fixes
                    tracker.Orientation = Support
                        .FixFlippedOrientation(isJointFlipped
                            ? Quaternion.Inverse(joint.Orientation).Projected()
                            : joint.Orientation.Projected()).Q();

                    tracker.PreviousOrientation = Support
                        .FixFlippedOrientation(isJointFlipped
                            ? Quaternion.Inverse(joint.PreviousOrientation).Projected()
                            : joint.PreviousOrientation.Projected()).Q();
                }

                // ReSharper disable once InvertIf | If overridden w/ position
                if (tracker.IsPositionOverridden)
                {
                    // Push raw positions and timestamps
                    tracker.Position = joint.Position;
                    tracker.PreviousPosition = joint.PreviousPosition;

                    tracker.PoseTimestamp = joint.PoseTimestamp;
                    tracker.PreviousPoseTimestamp = joint.PreviousPoseTimestamp;

                    // Optionally disable position filtering (on-demand)
                    tracker.NoPositionFilteringRequested = device.IsPositionFilterBlockingEnabled;

                    // Push raw physics (if valid)
                    if (device.IsPhysicsOverrideEnabled)
                    {
                        tracker.PoseVelocity = joint.Velocity;
                        tracker.PoseAcceleration = joint.Acceleration;
                        tracker.PoseAngularVelocity = joint.AngularVelocity;
                        tracker.PoseAngularAcceleration = joint.AngularAcceleration;

                        tracker.OverridePhysics = true;
                    }
                    else
                    {
                        // Else entirely disable physics
                        tracker.OverridePhysics = false;
                    }
                }
            }
        }
    }

    // The main program loop
    public static async void MainLoop()
    {
        // Warning: this is meant to work as fire-and-forget
        Logger.Info("[Main] Waiting for the start sem to open..");
        Shared.Events.SemSignalStartMain.WaitOne();

        Logger.Info("[Main] Starting the main app loop now...");

        var vrFrameRate = 110L; // Try running at 110 fps
        Logger.Info($"Desired loop rate: >{vrFrameRate} 1/s");

        vrFrameRate = (long)(1.0 / ((double)vrFrameRate / TimeSpan.TicksPerSecond));
        Logger.Info($"Desired loop timing: <{vrFrameRate} Ticks/frame");

        // Errors' case
        int serverTries = 0, serverLoops = 0;
        var loopStopWatch = new Stopwatch();

        while (true)
            try
            {
                // Run until termination
                while (true)
                {
                    loopStopWatch.Restart();

                    // Skip some things if we're getting ready to exit
                    if (!Interfacing.IsExitingNow)
                        lock (Interfacing.UpdateLock)
                        {
                            UpdateTrackingDevices(); // Update actual tracking
                            UpdateAppTrackers(); // Track joints from raw data
                        }

                    await UpdateServerTrackers(); // Send it to the server

                    // Wait until certain loop time has passed
                    var diffTicks = vrFrameRate - loopStopWatch.ElapsedTicks;
                    if (diffTicks > 0)
                    {
#pragma warning disable CA1806 // Do not ignore method results
                        SystemShell.TimeBeginPeriod(1);
#pragma warning restore CA1806 // Do not ignore method results

                        // Check the loop time occasionally
                        if (serverLoops >= 10000)
                        {
                            serverLoops = 0; // Reset the counter for the next 10'000 service loops
                            var elapsedTicks = loopStopWatch.ElapsedTicks; // Cache the elapsed time
                            await Task.Delay(TimeSpan.FromTicks(diffTicks));

                            Logger.Info($"10000 loops have passed: this loop took {elapsedTicks}, " +
                                        $"the loop's time after time correction (sleep) is: {loopStopWatch.ElapsedTicks}ns");
                        }
                        else
                        {
                            serverLoops++; // Else increase passed loops counter and wait
                            await Task.Delay(TimeSpan.FromTicks(diffTicks));
                        }

#pragma warning disable CA1806 // Do not ignore method results
                        SystemShell.TimeEndPeriod(1);
#pragma warning restore CA1806 // Do not ignore method results
                    }

                    else if (loopStopWatch.ElapsedMilliseconds > 30)
                    {
                        // Cry if the loop took longer than 30ms for some weird reason
                        Logger.Warn($"Can't keep up! The last loop took {loopStopWatch.ElapsedTicks} Ticks. " +
                                    $"(Ran at approximately {1000.0 / loopStopWatch.ElapsedMilliseconds} fps (1/s))");
                    }
                }
            }
            catch (Exception e)
            {
                Logger.Fatal(new AggregateException(
                    "The main loop has crashed! Restarting it now... " +
                    $"{e.GetType().Name} in {e.Source}: {e.Message}\n{e.StackTrace}"));

                serverTries++; // One more?
                switch (serverTries)
                {
                    case > 3 and <= 7:
                        // We've crashed the third time now. Somethin's off.. really...
                        Logger.Fatal(new AggregateException(
                            "Server loop has already crashed 3 times. Checking the joint config..."));

                        // Check the joint configuration
                        AppData.Settings.CheckSettings();
                        break;

                    case > 7:
                        // We've crashed the seventh time now. Somethin's off.. really...
                        Logger.Fatal(new ApplicationException(
                            "Server loop has already crashed 7 times. Giving up..."));

                        // Mark exiting as true
                        Interfacing.IsExitingNow = true;

                        // Mark trackers as inactive
                        Interfacing.AppTrackersInitialized = false;

                        // Wait a moment
                        await Task.Delay(200);
                        Interfacing.Fail(Interfacing.LocalizedJsonString(
                            "/CrashHandler/Content/Crash/Panic"));
                        break;
                }
            }
    }
}