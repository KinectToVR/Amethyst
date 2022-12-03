using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Amethyst.Driver.API;
using Amethyst.Driver.Client;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using AmethystSupport;
using Valve.VR;

namespace Amethyst.Classes;

public static class Main
{
    private static int _pFrozenLoops; // Loops passed since last frozen update
    private static bool _bInitialized; // Backup initialized? value

    private static void UpdateVrPositions()
    {
        // Grab and save vr head pose here
        Interfacing.UpdateHMDPosAndRot();
    }

    private static void UpdateInputBindings()
    {
        // Here, update EVR Input actions

        // Backup the current (OLD) data
        var bModeSwapState = Interfacing.EvrInput.ModeSwapActionData.bState;
        var bFreezeState = Interfacing.EvrInput.TrackerFreezeActionData.bState;
        var bFlipToggleState = Interfacing.EvrInput.TrackerFlipToggleData.bState;

        // Update all input actions
        if (!Interfacing.EvrInput.UpdateActionStates())
            Logger.Error("Could not update EVR Input Actions. Please check logs for further information");

        // Update the Tracking Freeze : toggle
        // Only if the state has changed from 1 to 0: button was clicked
        if (!Interfacing.EvrInput.TrackerFreezeActionData.bState && bFreezeState)
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

            var header = Interfacing.LocalizedJsonString("/GeneralPage/Tips/TrackingFreeze/Header_Short");

            // Change the tip depending on the currently connected controllers
            var controllerModel = new StringBuilder(1024);
            var error = ETrackedPropertyError.TrackedProp_Success;

            OpenVR.System.GetStringTrackedDeviceProperty(
                OpenVR.System.GetTrackedDeviceIndexForControllerRole(
                    ETrackedControllerRole.LeftHand),
                ETrackedDeviceProperty.Prop_ModelNumber_String,
                controllerModel, 1024, ref error);

            if (controllerModel.ToString().Contains("knuckles", StringComparison.OrdinalIgnoreCase) ||
                controllerModel.ToString().Contains("index", StringComparison.OrdinalIgnoreCase))
                header = header.Replace("{0}",
                    Interfacing.LocalizedJsonString("/GeneralPage/Tips/TrackingFreeze/Buttons/Index"));

            else if (controllerModel.ToString().Contains("vive", StringComparison.OrdinalIgnoreCase))
                header = header.Replace("{0}",
                    Interfacing.LocalizedJsonString("/GeneralPage/Tips/TrackingFreeze/Buttons/VIVE"));

            else if (controllerModel.ToString().Contains("mr", StringComparison.OrdinalIgnoreCase))
                header = header.Replace("{0}",
                    Interfacing.LocalizedJsonString("/GeneralPage/Tips/TrackingFreeze/Buttons/WMR"));

            else
                header = header.Replace("{0}",
                    Interfacing.LocalizedJsonString("/GeneralPage/Tips/TrackingFreeze/Buttons/Oculus"));

            Interfacing.ShowVrToast(Interfacing.IsTrackingFrozen
                ? Interfacing.LocalizedJsonString("/GeneralPage/Tips/TrackingFreeze/Toast_Enabled")
                : Interfacing.LocalizedJsonString("/GeneralPage/Tips/TrackingFreeze/Toast_Disabled"), header);
        }

        // Update the Flip Toggle : toggle
        // Only if the state has changed from 1 to 0: button was clicked
        if (!Interfacing.EvrInput.TrackerFlipToggleData.bState && bFlipToggleState)
        {
            Logger.Info("[Input Actions] Input: Flip toggled");

            // Also validate the result
            AppData.Settings.IsFlipEnabled =
                TrackingDevices.GetTrackingDevice().IsFlipSupported && AppData.Settings.IsFlipEnabled;

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

            var header = Interfacing.LocalizedJsonString("/SettingsPage/Tips/FlipToggle/Header_Short");

            // Change the tip depending on the currently connected controllers
            var controllerModel = new StringBuilder(1024);
            var error = ETrackedPropertyError.TrackedProp_Success;

            OpenVR.System.GetStringTrackedDeviceProperty(
                OpenVR.System.GetTrackedDeviceIndexForControllerRole(
                    ETrackedControllerRole.LeftHand),
                ETrackedDeviceProperty.Prop_ModelNumber_String,
                controllerModel, 1024, ref error);

            if (controllerModel.ToString().Contains("knuckles", StringComparison.OrdinalIgnoreCase) ||
                controllerModel.ToString().Contains("index", StringComparison.OrdinalIgnoreCase))
                header = header.Replace("{0}",
                    Interfacing.LocalizedJsonString("/SettingsPage/Tips/FlipToggle/Buttons/Index"));

            else if (controllerModel.ToString().Contains("vive", StringComparison.OrdinalIgnoreCase))
                header = header.Replace("{0}",
                    Interfacing.LocalizedJsonString("/SettingsPage/Tips/FlipToggle/Buttons/VIVE"));

            else if (controllerModel.ToString().Contains("mr", StringComparison.OrdinalIgnoreCase))
                header = header.Replace("{0}",
                    Interfacing.LocalizedJsonString("/SettingsPage/Tips/FlipToggle/Buttons/WMR"));

            else
                header = header.Replace("{0}",
                    Interfacing.LocalizedJsonString("/SettingsPage/Tips/FlipToggle/Buttons/Oculus"));

            Interfacing.ShowVrToast(Interfacing.IsTrackingFrozen
                ? Interfacing.LocalizedJsonString("/GeneralPage/Tips/FlipToggle/Toast_Enabled")
                : Interfacing.LocalizedJsonString("/GeneralPage/Tips/FlipToggle/Toast_Disabled"), header);
        }

        // Update the Calibration:Confirm : one-time switch
        // Only one-way switch this time, reset at calibration's end
        if (Interfacing.EvrInput.ConfirmAndSaveActionData.bState)
            Interfacing.CalibrationConfirm = true;

        // Update the Calibration:ModeSwap : one-time switch
        // Only if the state has changed from 1 to 0: chord was done
        Interfacing.CalibrationModeSwap =
            !Interfacing.EvrInput.ModeSwapActionData.bState && bModeSwapState;

        // Update the Calibration:FineTune : held switch
        Interfacing.CalibrationFineTune = Interfacing.EvrInput.FineTuneActionData.bState;

        // Update the Calibration:Joystick : vector2 x2
        Interfacing.CalibrationJoystickPositions.LeftPosition =
            (Interfacing.EvrInput.LeftJoystickActionData.x, Interfacing.EvrInput.LeftJoystickActionData.y);
        Interfacing.CalibrationJoystickPositions.RightPosition =
            (Interfacing.EvrInput.RightJoystickActionData.x, Interfacing.EvrInput.RightJoystickActionData.y);
    }

    private static async Task ParseVrEvents()
    {
        // Poll and parse all needed VR (overlay) events
        if (OpenVR.System is null) return;

        var vrEvent = new VREvent_t();
        while (OpenVR.Overlay.PollNextOverlayEvent(
                   Interfacing.VrOverlayHandle, ref vrEvent, (uint)Marshal.SizeOf<VREvent_t>()))
        {
            if (vrEvent.eventType != (uint)EVREventType.VREvent_Quit) continue;

            Logger.Info("VREvent_Quit has been called, requesting more time for handling the exit...");
            OpenVR.System.AcknowledgeQuit_Exiting();

            // Handle all the exit actions (if needed)
            if (!Interfacing.IsExitHandled)
                await Interfacing.HandleAppExit(1000);

            // Finally exit with code 0
            Environment.Exit(0);
        }
    }

    private static void UpdateTrackingDevices()
    {
        // Update the base device
        if (!TrackingDevices.GetTrackingDevice().IsSelfUpdateEnabled)
            TrackingDevices.GetTrackingDevice().Update();

        // Copy the hook joint [head] position, or the 1st one, or none
        Interfacing.DeviceHookJointPosition[TrackingDevices.GetTrackingDevice().Guid] =
            TrackingDevices.GetTrackingDevice().TrackedJoints
                .FirstOrDefault(x => x.Role == TrackedJointType.JointHead,
                    TrackingDevices.GetTrackingDevice().TrackedJoints.FirstOrDefault(new TrackedJoint())).JointPosition;

        // Copy the relative hook joint [waist] position, or the 1st one, or none
        Interfacing.DeviceRelativeTransformOrigin[TrackingDevices.GetTrackingDevice().Guid] =
            TrackingDevices.GetTrackingDevice().TrackedJoints
                .FirstOrDefault(x => x.Role == TrackedJointType.JointSpineWaist,
                    TrackingDevices.GetTrackingDevice().TrackedJoints.FirstOrDefault(new TrackedJoint())).JointPosition;

        // Update override devices (optionally)
        foreach (var device in AppData.Settings.OverrideDevicesGuidMap.Select(overrideGuid =>
                     TrackingDevices.GetDevice(overrideGuid).Device))
        {
            if (!device.IsSelfUpdateEnabled) device.Update();

            // Copy the hook joint [head] position, or the 1st one, or none
            Interfacing.DeviceHookJointPosition[device.Guid] =
                device.TrackedJoints.FirstOrDefault(x => x.Role == TrackedJointType.JointHead,
                    device.TrackedJoints.FirstOrDefault(new TrackedJoint())).JointPosition;

            // Copy the relative hook joint [waist] position, or the 1st one, or none
            Interfacing.DeviceRelativeTransformOrigin[device.Guid] =
                device.TrackedJoints.FirstOrDefault(x => x.Role == TrackedJointType.JointSpineWaist,
                    device.TrackedJoints.FirstOrDefault(new TrackedJoint())).JointPosition;
        }
    }

    private static async Task UpdateServerTrackers()
    {
        // Update only if we're connected and running
        if (!Interfacing.K2AppTrackersSpawned || Interfacing.ServerDriverFailure) return;

        // If tracking is frozen, only refresh
        if (Interfacing.IsTrackingFrozen && !AppData.Settings.FreezeLowerBodyOnly)
        {
            // To save resources, frozen trackers update once per 1000 frames
            // (When they're unfrozen, they go back to instant updates)
            if (_pFrozenLoops >= 1000)
            {
                // Refresh in the server driver
                await DriverClient.RefreshTrackerPoses(AppData.Settings.TrackersVector
                    .Where(tracker => tracker.IsActive).Select(tracker => tracker.Role));

                // Reset
                _pFrozenLoops = 0;
            }
            else
            {
                _pFrozenLoops++;
            }
        }
        // If the tracing's actually running
        else
        {
            // Update position & orientation filters
            AppData.Settings.TrackersVector.ToList().ForEach(tracker => tracker.UpdateFilters());

            // Update pose w/ filtering, options and calibration
            // Note: only position gets calibrated INSIDE trackers

            // If we've frozen everything but elbows
            var updateLowerBody = true;
            if (Interfacing.IsTrackingFrozen && AppData.Settings.FreezeLowerBodyOnly)
            {
                updateLowerBody = false;

                // To save resources, frozen trackers update once per 1000 frames
                // (When they're unfrozen, they go back to instant updates)
                if (_pFrozenLoops >= 1000)
                {
                    // Refresh in the server driver
                    await DriverClient.RefreshTrackerPoses(AppData.Settings.TrackersVector
                        .Where(tracker => tracker.IsActive && (int)TypeUtils
                            .TrackerTypeJointDictionary[tracker.Role] >= 16).Select(tracker => tracker.Role));

                    _pFrozenLoops = 0; // Reset
                }
                else
                {
                    _pFrozenLoops++; // Increment the counter
                }
            }

            // Update the [server] trackers
            await DriverClient.UpdateTrackerPoses(AppData.Settings.TrackersVector.Where(tracker => tracker.IsActive)
                .Where(x => (int)TypeUtils.TrackerTypeJointDictionary[x.Role] < 16 || updateLowerBody)
                .Select(tracker => tracker.GetTrackerBase(
                    tracker.PositionTrackingFilterOption, tracker.OrientationTrackingFilterOption)));
        }

        // Update status right after any change
        if (_bInitialized != Interfacing.AppTrackersInitialized)
        {
            // Try 3 times (cause why not)
            for (var i = 0; i < 3; i++)
            {
                // Update status in server
                await DriverClient.UpdateTrackerStates(AppData.Settings.TrackersVector
                    .Where(tracker => tracker.IsActive)
                    .Select(tracker => (tracker.Role, Interfacing.AppTrackersInitialized)));

                // Update internal status
                _bInitialized = Interfacing.AppTrackersInitialized;
            }

            // Rescan controller ids
            Interfacing.VrControllerIndexes = (
                OpenVR.System.GetTrackedDeviceIndexForControllerRole(
                    ETrackedControllerRole.LeftHand),
                OpenVR.System.GetTrackedDeviceIndexForControllerRole(
                    ETrackedControllerRole.RightHand)
            );
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
                                 .Where(tracker => Interfacing.FindVrTracker(
                                     TypeUtils.TrackerTypeSerialDictionary[tracker.Role], false).Found))
                    {
                        // Make actual changes (self-updates)
                        tracker.IsActive = false;

                        // Try even 5 times (cause why not)
                        for (var i = 0; i < 5; i++)
                        {
                            await DriverClient.UpdateTrackerStates(new[] { (tracker.Role, false) });
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

                    Interfacing.ShowVrToast(
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
            var device = TrackingDevices.GetTrackingDevice();

            var extFlip =
                AppData.Settings.IsFlipEnabled &&
                AppData.Settings.IsExternalFlipEnabled;

            var extFlipInternal =
                AppData.Settings.TrackersVector[0].IsActive &&
                AppData.Settings.TrackersVector[0].IsOrientationOverridden;

            // Compose flip
            var dotFacing =
                Calibration.OrientationDot(
                    // Check for external-flip
                    extFlip

                        // Check for internal overrides
                        ? extFlipInternal

                            // Overriden internal amethyst tracker
                            ? Quaternion.Inverse(Interfacing.VrPlayspaceOrientationQuaternion) *
                              AppData.Settings.TrackersVector[0].Orientation

                            // External VR waist tracker
                            : Interfacing.GetVrTrackerPoseCalibrated("waist").Orientation

                        // Default: VR HMD orientation
                        : Interfacing.Plugins.GetHmdPoseCalibrated.Orientation,

                    // Check for external-flip
                    extFlip

                        // If ExtFlip is enabled compare to its calibration
                        ? AppData.Settings.ExternalFlipCalibrationMatrix

                        // Default: use the default calibration rotation
                        : AppData.Settings.DeviceCalibrationRotationMatrices.GetValueOrDefault(
                            AppData.Settings.TrackingDeviceGuid, Quaternion.Identity));

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
                        Quaternion.CreateFromYawPitchRoll((float)Calibration.QuaternionYaw(
                            Interfacing.Plugins.GetHmdPoseCalibrated.Orientation), 0, 0),

                    // Optionally overwrite the rotation with NONE
                    JointRotationTrackingOption.DisableJointRotation => Quaternion.Identity,

                    // Default
                    _ => isJointFlipped
                        ? Quaternion.Inverse(joint.JointOrientation)
                        : joint.JointOrientation
                };

                // Copy the previous orientation to the tracker
                tracker.PreviousOrientation = tracker.OrientationTrackingOption switch
                {
                    // Optionally overwrite the rotation with HMD orientation
                    // Not the "calibrated" variant, as the fix will be applied after everything else
                    JointRotationTrackingOption.FollowHmdRotation =>
                        Quaternion.CreateFromYawPitchRoll((float)Calibration.QuaternionYaw(
                            Interfacing.Plugins.GetHmdPoseCalibrated.Orientation), 0, 0),

                    // Optionally overwrite the rotation with NONE
                    JointRotationTrackingOption.DisableJointRotation => Quaternion.Identity,

                    // Default
                    _ => isJointFlipped
                        ? Quaternion.Inverse(joint.PreviousJointOrientation)
                        : joint.PreviousJointOrientation
                };

                // If math-based orientation is supported, overwrite the orientation with it
                if (device.IsAppOrientationSupported &&
                    tracker.Role is TrackerType.TrackerLeftFoot or TrackerType.TrackerRightFoot)
                    tracker.Orientation = tracker.OrientationTrackingOption switch
                    {
                        JointRotationTrackingOption.SoftwareCalculatedRotation =>
                            Calibration.FeetSoftwareOrientation(
                                    device.TrackedJoints.First(x =>
                                        x.Role == TypeUtils.FlipJointType(TrackedJointType.JointAnkleLeft,
                                            (tracker.Role != TrackerType.TrackerLeftFoot) ^ isJointFlipped)),
                                    device.TrackedJoints.First(x =>
                                        x.Role == TypeUtils.FlipJointType(TrackedJointType.JointFootLeft,
                                            (tracker.Role != TrackerType.TrackerLeftFoot) ^ isJointFlipped)),
                                    device.TrackedJoints.First(x =>
                                        x.Role == TypeUtils.FlipJointType(TrackedJointType.JointKneeLeft,
                                            (tracker.Role != TrackerType.TrackerLeftFoot) ^ isJointFlipped)))
                                .Inversed(isJointFlipped), // Also inverse if flipped (via an extension)

                        JointRotationTrackingOption.SoftwareCalculatedRotationV2 =>
                            Calibration.FeetSoftwareOrientationV2(
                                    device.TrackedJoints.First(x =>
                                        x.Role == TypeUtils.FlipJointType(TrackedJointType.JointAnkleLeft,
                                            (tracker.Role != TrackerType.TrackerLeftFoot) ^ isJointFlipped)),
                                    device.TrackedJoints.First(x =>
                                        x.Role == TypeUtils.FlipJointType(TrackedJointType.JointFootLeft,
                                            (tracker.Role != TrackerType.TrackerLeftFoot) ^ isJointFlipped)),
                                    device.TrackedJoints.First(x =>
                                        x.Role == TypeUtils.FlipJointType(TrackedJointType.JointKneeLeft,
                                            (tracker.Role != TrackerType.TrackerLeftFoot) ^ isJointFlipped)))
                                .Inversed(isJointFlipped), // Also inverse if flipped (via an extension)

                        _ => tracker.Orientation
                    };

                // Apply calibration-related flipped orientation fixes
                if (isJointFlipped && tracker.OrientationTrackingOption
                    != JointRotationTrackingOption.FollowHmdRotation)
                {
                    // Note: only in flip mode
                    tracker.Orientation = Calibration
                        .FixFlippedOrientation(tracker.Orientation);
                    tracker.PreviousOrientation = Calibration
                        .FixFlippedOrientation(tracker.PreviousOrientation);
                }

                // Apply playspace-related orientation fixes
                if (tracker.OrientationTrackingOption ==
                    JointRotationTrackingOption.FollowHmdRotation)
                {
                    // Offset to fit the playspace
                    tracker.Orientation =
                        Quaternion.Inverse(Interfacing.VrPlayspaceOrientationQuaternion) * tracker.Orientation;
                    tracker.PreviousOrientation =
                        Quaternion.Inverse(Interfacing.VrPlayspaceOrientationQuaternion) * tracker.PreviousOrientation;
                }

                // Push raw positions and timestamps
                tracker.Position = joint.JointPosition;
                tracker.PreviousPosition = joint.PreviousJointPosition;

                tracker.PoseTimestamp = joint.PoseTimestamp;
                tracker.PreviousPoseTimestamp = joint.PreviousPoseTimestamp;

                // Optionally disable position filtering (on-demand)
                tracker.NoPositionFilteringRequested = device.IsPositionFilterBlockingEnabled;

                // Push raw physics (if valid)
                if (device.IsPhysicsOverrideEnabled)
                {
                    tracker.PoseVelocity = joint.JointVelocity;
                    tracker.PoseAcceleration = joint.JointAcceleration;
                    tracker.PoseAngularVelocity = joint.JointAngularVelocity;
                    tracker.PoseAngularAcceleration = joint.JointAngularAcceleration;

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
                    tracker.Orientation = Calibration
                        .FixFlippedOrientation(isJointFlipped
                            ? Quaternion.Inverse(joint.JointOrientation)
                            : joint.JointOrientation);

                    tracker.PreviousOrientation = Calibration
                        .FixFlippedOrientation(isJointFlipped
                            ? Quaternion.Inverse(joint.PreviousJointOrientation)
                            : joint.PreviousJointOrientation);
                }

                // ReSharper disable once InvertIf | If overridden w/ position
                if (tracker.IsPositionOverridden)
                {
                    // Push raw positions and timestamps
                    tracker.Position = joint.JointPosition;
                    tracker.PreviousPosition = joint.PreviousJointPosition;

                    tracker.PoseTimestamp = joint.PoseTimestamp;
                    tracker.PreviousPoseTimestamp = joint.PreviousPoseTimestamp;

                    // Optionally disable position filtering (on-demand)
                    tracker.NoPositionFilteringRequested = device.IsPositionFilterBlockingEnabled;

                    // Push raw physics (if valid)
                    if (device.IsPhysicsOverrideEnabled)
                    {
                        tracker.PoseVelocity = joint.JointVelocity;
                        tracker.PoseAcceleration = joint.JointAcceleration;
                        tracker.PoseAngularVelocity = joint.JointAngularVelocity;
                        tracker.PoseAngularAcceleration = joint.JointAngularAcceleration;

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

        // For limiting loop 'fps'
        var vrError = ETrackedPropertyError.TrackedProp_Success;
        var vrFrameRate = (long)Math.Clamp(OpenVR.System.GetFloatTrackedDeviceProperty(
            0, ETrackedDeviceProperty.Prop_DisplayFrequency_Float, ref vrError), 70, 130);
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
                    // Update things here

                    UpdateVrPositions(); // Update HMD poses
                    UpdateInputBindings(); // Update input
                    await ParseVrEvents(); // Parse VR events

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
                Logger.Error($"The main loop has crashed! Restarting it now... (Message: {e.Message})");

                serverTries++; // One more?
                switch (serverTries)
                {
                    case > 3 and <= 7:
                        // We've crashed the third time now. Somethin's off.. really...
                        Logger.Error("Server loop has already crashed 3 times. Checking the joint config...");

                        // Check the joint configuration
                        AppData.Settings.CheckSettings();
                        break;

                    case > 7:
                        // We've crashed the seventh time now. Somethin's off.. really...
                        Logger.Error("Server loop has already crashed 7 times. Giving up...");

                        // Mark exiting as true
                        Interfacing.IsExitingNow = true;

                        // Mark trackers as inactive
                        Interfacing.AppTrackersInitialized = false;

                        // Wait a moment
                        await Task.Delay(200);

                        // -13 is the code for giving up then, I guess
                        // The user will be prompted to reset the config (opt)
                        Interfacing.Fail(-13);
                        break;
                }
            }
    }
}