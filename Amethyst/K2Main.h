#pragma once
#include <ranges>

#include "K2DeviceMath.h"

namespace k2app::main
{
	inline void K2UpdateVRPositions()
	{
		// Grab and save vr head pose here
		interfacing::updateHMDPosAndRot();
	}

	inline void K2UpdateInputBindings()
	{
		using namespace interfacing;

		/* Here, update EVR Input actions */

		// Backup the current ( OLD ) data
		const bool bak_mode_swap_state = evr_input.modeSwapActionData().bState,
		           bak_freeze_state = evr_input.trackerFreezeActionData().bState,
		           bak_flip_toggle_state = evr_input.trackerFlipToggleData().bState;

		// Update all input actions
		if (!evr_input.UpdateActionStates())
			LOG(ERROR) << "Could not update EVR Input Actions. Please check logs for further information.";

		// Update the Tracking Freeze : toggle
		// Only if the state has changed from 1 to 0: button was clicked
		if (!evr_input.trackerFreezeActionData().bState && bak_freeze_state)
		{
			LOG(INFO) << "[Input Actions] Input: Tracking freeze toggled.";
			isTrackingFrozen = !isTrackingFrozen;

			// Play a Sound and Update UI
			playAppSound(isTrackingFrozen ? sounds::AppSounds::ToggleOff : sounds::AppSounds::ToggleOn);

			shared::main::thisDispatcherQueue->TryEnqueue([&]
			{
				if (shared::general::toggleFreezeButton.get() != nullptr)
				{
					shared::general::general_tab_setup_finished = false; // Boiler
					shared::general::toggleFreezeButton->IsChecked(isTrackingFrozen);
					shared::general::toggleFreezeButton->Content(isTrackingFrozen
						                                             ? winrt::box_value(
							                                             LocalizedResourceWString(
								                                             L"GeneralPage",
								                                             L"Buttons/Skeleton/Unfreeze"))
						                                             : winrt::box_value(
							                                             LocalizedResourceWString(
								                                             L"GeneralPage",
								                                             L"Buttons/Skeleton/Freeze")));
					shared::general::general_tab_setup_finished = true; // Boiler end
				}
			});

			{
				auto _header = LocalizedJSONString(
					L"/GeneralPage/Tips/TrackingFreeze/Header_Short");

				// Change the tip depending on the currently connected controllers
				char _controller_model[1024];
				vr::VRSystem()->GetStringTrackedDeviceProperty(
					vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(
						vr::ETrackedControllerRole::TrackedControllerRole_LeftHand),
					vr::ETrackedDeviceProperty::Prop_ModelNumber_String,
					_controller_model, std::size(_controller_model));

				// For the ""s operator
				using namespace std::string_literals;

				if (findStringIC(_controller_model, "knuckles") ||
					findStringIC(_controller_model, "index"))
					stringReplaceAll(_header, L"{0}"s,
					                 LocalizedResourceWString(
						                 L"GeneralPage",
						                 L"Tips/TrackingFreeze/Buttons/Index"));

				else if (findStringIC(_controller_model, "vive"))
					stringReplaceAll(_header, L"{0}"s,
					                 LocalizedResourceWString(
						                 L"GeneralPage",
						                 L"Tips/TrackingFreeze/Buttons/VIVE"));

				else if (findStringIC(_controller_model, "mr"))
					stringReplaceAll(_header, L"{0}"s,
					                 LocalizedResourceWString(
						                 L"GeneralPage",
						                 L"Tips/TrackingFreeze/Buttons/WMR"));

				else
					stringReplaceAll(_header, L"{0}"s,
					                 LocalizedResourceWString(
						                 L"GeneralPage",
						                 L"Tips/TrackingFreeze/Buttons/Oculus"));

				ShowVRToast(isTrackingFrozen
					            ? LocalizedJSONString(
						            L"/GeneralPage/Tips/TrackingFreeze/Toast_Enabled")
					            : LocalizedJSONString(
						            L"/GeneralPage/Tips/TrackingFreeze/Toast_Disabled"), _header);
			}
		}

		// Update the Flip Toggle : toggle
		// Only if the state has changed from 1 to 0: button was clicked
		if (!evr_input.trackerFlipToggleData().bState && bak_flip_toggle_state)
		{
			LOG(INFO) << "[Input Actions] Input: Flip toggled.";

			// Also validate the result
			if (const auto& trackingDevice = TrackingDevices::getCurrentDevice(); trackingDevice.index() == 0)
			{
				// Kinect Basis
				K2Settings.isFlipEnabled =
					std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(trackingDevice)->isFlipSupported()
						? !K2Settings.isFlipEnabled // If supported
						: false; // If not supported
			}
			else if (trackingDevice.index() == 1)
			{
				// Joints Basis
				K2Settings.isFlipEnabled = false;
			}

			// Save settings
			K2Settings.saveSettings();

			// Play a Sound and Update UI
			playAppSound(K2Settings.isFlipEnabled
				             ? sounds::AppSounds::ToggleOn
				             : sounds::AppSounds::ToggleOff);

			shared::main::thisDispatcherQueue->TryEnqueue([&]
			{
				if (shared::settings::flipToggle.get() != nullptr)
				{
					shared::settings::settings_localInitFinished = false; // Boiler
					shared::settings::flipToggle->IsOn(K2Settings.isFlipEnabled);
					shared::settings::settings_localInitFinished = true; // Boiler end
				}
			});

			{
				auto _header = LocalizedJSONString(
					L"/SettingsPage/Tips/FlipToggle/Header_Short");

				// Change the tip depending on the currently connected controllers
				char _controller_model[1024];
				vr::VRSystem()->GetStringTrackedDeviceProperty(
					vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(
						vr::ETrackedControllerRole::TrackedControllerRole_LeftHand),
					vr::ETrackedDeviceProperty::Prop_ModelNumber_String,
					_controller_model, std::size(_controller_model));

				// For the ""s operator
				using namespace std::string_literals;

				if (findStringIC(_controller_model, "knuckles") ||
					findStringIC(_controller_model, "index"))
					stringReplaceAll(_header, L"{0}"s,
					                 LocalizedResourceWString(
						                 L"SettingsPage",
						                 L"Tips/FlipToggle/Buttons/Index"));

				else if (findStringIC(_controller_model, "vive"))
					stringReplaceAll(_header, L"{0}"s,
					                 LocalizedResourceWString(
						                 L"SettingsPage",
						                 L"Tips/FlipToggle/Buttons/VIVE"));

				else if (findStringIC(_controller_model, "mr"))
					stringReplaceAll(_header, L"{0}"s,
					                 LocalizedResourceWString(
						                 L"SettingsPage",
						                 L"Tips/FlipToggle/Buttons/WMR"));

				else
					stringReplaceAll(_header, L"{0}"s,
					                 LocalizedResourceWString(
						                 L"SettingsPage",
						                 L"Tips/FlipToggle/Buttons/Oculus"));

				ShowVRToast(K2Settings.isFlipEnabled
					            ? LocalizedJSONString(
						            L"/SettingsPage/Tips/FlipToggle/Toast_Enabled")
					            : LocalizedJSONString(
						            L"/SettingsPage/Tips/FlipToggle/Toast_Disabled"), _header);
			}
		}

		// Update the Calibration:Confirm : one-time switch
		// Only one-way switch this time, reset at calibration's end
		if (evr_input.confirmAndSaveActionData().bState)
			calibration_confirm = true;

		// Update the Calibration:ModeSwap : one-time switch
		// Only if the state has changed from 1 to 0: chord was done
		calibration_modeSwap =
			(!evr_input.modeSwapActionData().bState && bak_mode_swap_state);

		// Update the Calibration:FineTune : held switch
		calibration_fineTune = evr_input.fineTuneActionData().bState;

		// Update the Calibration:Joystick : vector2 x2
		calibration_joystick_positions[0][0] = evr_input.leftJoystickActionData().x;
		calibration_joystick_positions[0][1] = evr_input.leftJoystickActionData().y;

		calibration_joystick_positions[1][0] = evr_input.rightJoystickActionData().x;
		calibration_joystick_positions[1][1] = evr_input.rightJoystickActionData().y;
	}

	inline void K2ParseVREvents()
	{
		// Poll and parse all needed VR (overlay) events
		if (!vr::VRSystem())
			return;

		vr::VREvent_t vrEvent{};
		while (vr::VROverlay()->PollNextOverlayEvent(interfacing::vrOverlayHandle, &vrEvent, sizeof(vrEvent)))
		{
			switch (vrEvent.eventType)
			{
			case vr::VREvent_Quit:
				// Handle exit
				{
					LOG(INFO) << "VREvent_Quit has been called, requesting more time for handling the exit...";
					vr::VRSystem()->AcknowledgeQuit_Exiting();

					// Handle all the exit actions (if needed)
					if (!interfacing::isExitHandled)
						interfacing::handle_app_exit_n();

					// Finally exit with code 0
					exit(0);
				}
			default: break;
			}
		}
	}

	inline void K2UpdateTrackingDevices()
	{
		/* Update the base device here */
		switch (const auto& device = TrackingDevices::getCurrentDevice(); device.index())
		{
		case 0:
			{
				const auto& pDevice = std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(device);

				if (!pDevice->isSelfUpdateEnabled())
					pDevice->update(); // Update the device

				interfacing::kinectHeadPosition[pDevice->getDeviceGUID()] =
					pDevice->getTrackedJoints()[ktvr::Joint_Head].getJointPosition();
				interfacing::deviceRelativeTransformOrigin[pDevice->getDeviceGUID()] =
					pDevice->getTrackedJoints()[ktvr::Joint_SpineWaist].getJointPosition();
			}
			break;
		case 1:
			{
				const auto& pDevice = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(device);

				if (!pDevice->isSelfUpdateEnabled())
					pDevice->update(); // Update the device

				if (K2Settings.K2TrackersVector[0].selectedTrackedJointID < pDevice->getTrackedJoints().size())
					interfacing::deviceRelativeTransformOrigin[pDevice->getDeviceGUID()] = pDevice->getTrackedJoints().
						at(
							K2Settings.K2TrackersVector[0].selectedTrackedJointID).getJointPosition();
			}
			break;
		}

		/* Update the override device here (optionally) */
		for (const auto& override_id : K2Settings.overrideDeviceGUIDsMap | std::views::values)
			switch (TrackingDevices::TrackingDevicesVector[override_id].index())
			{
			case 0:
				{
					const auto& pDevice = std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(
						TrackingDevices::TrackingDevicesVector[override_id]);

					if (!pDevice->isSelfUpdateEnabled())
						pDevice->update(); // Update the device

					interfacing::kinectHeadPosition[pDevice->getDeviceGUID()] =
						pDevice->getTrackedJoints()[ktvr::Joint_Head].getJointPosition();
					interfacing::deviceRelativeTransformOrigin[pDevice->getDeviceGUID()] =
						pDevice->getTrackedJoints()[ktvr::Joint_SpineWaist].getJointPosition();
				}
				break;
			case 1:
				{
					const auto& pDevice = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
						TrackingDevices::TrackingDevicesVector[override_id]);

					if (!pDevice->isSelfUpdateEnabled())
						pDevice->update(); // Update the device

					if (K2Settings.K2TrackersVector[0].selectedTrackedJointID < pDevice->getTrackedJoints().size())
						interfacing::deviceRelativeTransformOrigin[pDevice->getDeviceGUID()] = pDevice->
							getTrackedJoints().at(
								K2Settings.K2TrackersVector[0].selectedTrackedJointID).getJointPosition();
				}
				break;
			}
	}

	inline int p_frozen_loops = 0; // Loops passed since last frozen update
	inline bool initialized_bak = false; // Backup initialized? value
	inline void K2UpdateServerTrackers()
	{
		// Update only if we're connected and running
		if (interfacing::K2AppTrackersSpawned &&
			!interfacing::serverDriverFailure)
		{
			// If tracking is frozen, only refresh
			if (interfacing::isTrackingFrozen && !K2Settings.freezeLowerOnly)
			{
				// To save resources, frozen trackers update once per 1000 frames
				// (When they're unfrozen, they go back to instant updates)
				if (p_frozen_loops >= 1000)
				{
					for (const auto& tracker : K2Settings.K2TrackersVector)
						if (tracker.data_isActive)
							ktvr::refresh_tracker_pose<false>(tracker.base_tracker);

					// Reset
					p_frozen_loops = 0;
				}
				else p_frozen_loops++;
			}
			// If the tracing's actually running
			else
			{
				for (auto& tracker : K2Settings.K2TrackersVector)
				{
					// Update orientation filters
					tracker.updateOrientationFilters();

					// Update position filters
					tracker.updatePositionFilters();
				}

				// Create a dummy update vector
				std::vector<ktvr::K2TrackerBase> k2_tracker_bases;

				// Update pose w/ filtering, options and calibration
				// Note: only position gets calibrated INSIDE trackers

				// If we've frozen everything but elbows
				bool _updateLowerBody = true;
				if (interfacing::isTrackingFrozen && K2Settings.freezeLowerOnly)
				{
					_updateLowerBody = false;

					// To save resources, frozen trackers update once per 1000 frames
					// (When they're unfrozen, they go back to instant updates)
					if (p_frozen_loops >= 1000)
					{
						// Only lower body joints
						for (auto& tracker : K2Settings.K2TrackersVector)
							if (tracker.data_isActive &&
								static_cast<int>(ITrackerType_Joint[tracker.base_tracker]) >= 16)
								ktvr::refresh_tracker_pose<false>(tracker.base_tracker);

						// Reset
						p_frozen_loops = 0;
					}
					else p_frozen_loops++;
				}

				// Update trackers in the vector recursively
				for (auto& tracker : K2Settings.K2TrackersVector)
				{
					// If lower body is omitted
					if (static_cast<int>(ITrackerType_Joint[tracker.base_tracker]) >= 16
						&& !_updateLowerBody)
						continue;

					// If the tracker is off
					if (!tracker.data_isActive) continue;

					// If overridden by second device
					if (tracker.isPositionOverridden &&
						K2Settings.overrideDeviceGUIDsMap.contains(tracker.overrideGUID))
					{
						if (K2Settings.deviceMatricesCalibrated[
							K2Settings.trackingDeviceGUIDPair.first])

							k2_tracker_bases.push_back(
								tracker.getTrackerBase
								(
									K2Settings.deviceCalibrationRotationMatrices[
										tracker.overrideGUID],
									K2Settings.deviceCalibrationTranslationVectors[
										tracker.overrideGUID],
									K2Settings.deviceCalibrationOrigins[
										tracker.overrideGUID],

									tracker.positionTrackingFilterOption,
									tracker.orientationTrackingFilterOption
								));
						else
							k2_tracker_bases.push_back(
								tracker.getTrackerBase
								(
									tracker.positionTrackingFilterOption,
									tracker.orientationTrackingFilterOption
								));
					}

					// If not overriden by another device
					else
					{
						if (K2Settings.deviceMatricesCalibrated[
							K2Settings.trackingDeviceGUIDPair.first])

							k2_tracker_bases.push_back(
								tracker.getTrackerBase
								(
									K2Settings.deviceCalibrationRotationMatrices[
										K2Settings.trackingDeviceGUIDPair.first],
									K2Settings.deviceCalibrationTranslationVectors[
										K2Settings.trackingDeviceGUIDPair.first],
									K2Settings.deviceCalibrationOrigins[
										K2Settings.trackingDeviceGUIDPair.first],

									tracker.positionTrackingFilterOption,
									tracker.orientationTrackingFilterOption
								));
						else
							k2_tracker_bases.push_back(
								tracker.getTrackerBase
								(
									tracker.positionTrackingFilterOption,
									tracker.orientationTrackingFilterOption
								));
					}
				}

				// Now update the trackers
				if (!k2_tracker_bases.empty())
					update_tracker_vector(k2_tracker_bases);
			}

			// Update status right after any change
			if (initialized_bak != interfacing::K2AppTrackersInitialized)
				std::thread([&]
				{
					// Create a dummy update vector
					std::vector<std::pair<ktvr::ITrackerType, bool>> k2_tracker_statuses;
					for (const auto& tracker : K2Settings.K2TrackersVector)
						if (tracker.data_isActive)
							k2_tracker_statuses.push_back(
								{tracker.base_tracker, interfacing::K2AppTrackersInitialized});

					// try 3 times cause why not
					for (int i = 0; i < 3; i++)
					{
						// Update status in server
						ktvr::update_tracker_state_vector<false>(k2_tracker_statuses);
						std::this_thread::sleep_for(std::chrono::milliseconds(15));

						// Update internal status
						initialized_bak = interfacing::K2AppTrackersInitialized;
					}

					// Rescan controller ids
					interfacing::vrControllerIndexes = {
						vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(
							vr::ETrackedControllerRole::TrackedControllerRole_LeftHand),

						vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(
							vr::ETrackedControllerRole::TrackedControllerRole_RightHand)
					};
				}).detach();

			// Scan for already-added body trackers from other apps
			// (If any found, disable corresponding ame's trackers/pairs)
			if (interfacing::alreadyAddedTrackersScanRequested)
			{
				// Mark the request as done
				interfacing::alreadyAddedTrackersScanRequested = false;

				// Run the worker (if applicable)
				if (K2Settings.checkForOverlappingTrackers &&
					!interfacing::isAlreadyAddedTrackersScanRunning)
					shared::main::thisDispatcherQueue->TryEnqueue(
						[&]()-> winrt::Windows::Foundation::IAsyncAction
						{
							using namespace winrt;
							interfacing::isAlreadyAddedTrackersScanRunning = true;
							bool wereChangesMade = false; // At least not yet

							// Do that on UI's background
							apartment_context ui_thread;
							co_await resume_background();

							// Search for 
							std::vector<bool> foundVRTracker;
							for (const auto& _tracker : K2Settings.K2TrackersVector)
								foundVRTracker.push_back(interfacing::findVRTracker(
									ITrackerType_Serial[_tracker.base_tracker], false, true).first);
							co_await ui_thread;

							// Disable an (already-added tracker)'s been found
							for (uint32_t tracker_index = 0;
							     tracker_index < K2Settings.K2TrackersVector.size(); tracker_index++)
								if (K2Settings.K2TrackersVector[tracker_index].data_isActive &&
									foundVRTracker[tracker_index])
								{
									// Make actual changes
									K2Settings.K2TrackersVector[tracker_index].data_isActive = false; // Deactivate
									for (const auto& expander : shared::settings::jointExpanderVector)
										expander->UpdateIsActive();

									// Do that on UI's background
									apartment_context _ui_thread;
									co_await resume_background();

									// try even 5 times cause why not
									for (int i = 0; i < 5; i++)
									{
										ktvr::set_tracker_state<false>(
											K2Settings.K2TrackersVector[tracker_index].base_tracker, false);
										Sleep(20);
									}
									co_await _ui_thread;

									// Check if we've disabled any joints from spawning and disable their mods
									interfacing::devices_check_disabled_joints();
									TrackingDevices::settings_trackersConfigChanged(false);

									// Save settings
									K2Settings.saveSettings();
									K2Settings.readSettings();
									wereChangesMade = true;
								}

							// Check if anything's changed
							interfacing::isAlreadyAddedTrackersScanRunning = false;
							if (wereChangesMade)
							{
								interfacing::ShowToast(
									interfacing::LocalizedResourceWString(
										L"SharedStrings", L"Toasts/TrackersAutoDisabled/Title"),
									interfacing::LocalizedResourceWString(
										L"SharedStrings", L"Toasts/TrackersAutoDisabled"),
									true, L"focus_trackers"); // This one's gonna be a high-priority one

								interfacing::ShowVRToast(
									interfacing::LocalizedJSONString(
										L"/SharedStrings/Toasts/TrackersAutoDisabled/Title"),
									interfacing::LocalizedJSONString(
										L"/SharedStrings/Toasts/TrackersAutoDisabled"));
							}
						});
			}
		}
	}

	// Update trackers inside the app here
	inline void K2UpdateAppTrackers()
	{
		using namespace interfacing;

		/*
		 * This is where we do EVERYTHING pose related.
		 * All positions and rotations are calculated here,
		 * depending on calibration val and configuration
		 */

		/*
		 * Calculate ALL poses for the base (first) device here
		 */

		// We can precompute the threshold as a dot product value
		// as the dot product is also defined as |a||b|cos(x)
		// and since we're using unit vectors... |a||b| = 1
		constexpr double FLIP_THRESHOLD = 0.4226182; // cos(65°)

		// Base device
		{
			// Get the currently tracking device
			const auto& _device = TrackingDevices::getCurrentDevice();

			const bool _ext_flip =
				K2Settings.isFlipEnabled &&
				K2Settings.isExternalFlipEnabled;

			const bool _ext_flip_internal =
				K2Settings.K2TrackersVector[0].data_isActive &&
				K2Settings.K2TrackersVector[0].isRotationOverridden;

			// Compose flip
			const auto _dot_facing =
				EigenUtils::NormalizedRotationVectorDot(

					// Check for external-flip
					_ext_flip

						// Check for internal overrides
						? (_ext_flip_internal

							   // Overriden internal amethyst tracker
							   ? vrPlayspaceOrientationQuaternion.inverse() *
							   K2Settings.K2TrackersVector[0].pose_orientation

							   // External VR waist tracker
							   : getVRTrackerPoseCalibrated("waist").second)

						// Default: VR HMD orientation
						: plugins::plugins_getHMDOrientationCalibrated(),

					// Check for external-flip
					_ext_flip

						// If ExtFlip is enabled compare to its calibration
						? K2Settings.externalFlipCalibrationMatrix

						// Default: use the default calibration rotation
						: K2Settings.deviceCalibrationRotationMatrices[
							K2Settings.trackingDeviceGUIDPair.first]);

			// Not in transition angle area, can compute
			if (std::abs(_dot_facing) >= FLIP_THRESHOLD)
				base_flip = _dot_facing < 0.;

			// Overwrite flip value depending on the device & settings
			// (Device type check should have already been done tho...)
			if (!K2Settings.isFlipEnabled || _device.index() == 1)base_flip = false;

			/*
			 * Trackers orientation - preparations
			 */

			for (auto& tracker : K2Settings.K2TrackersVector)
			{
				// Copy the orientation to the tracker
				tracker.pose_orientation =
					_device.index() == 0

						// SkeletonBasis Device - grab L or R depending on flip : index0
						? (base_flip

							   // If flip
							   ? std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(_device)->
							     getTrackedJoints()[
								     overrides::getFlippedJointType(
									     ITrackerType_Joint[tracker.base_tracker])].
							     getJointOrientation().inverse()

							   // If no flip
							   : std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(_device)->
							   getTrackedJoints()[ITrackerType_Joint[tracker.base_tracker]].
							   getJointOrientation()
						)

						// JointsBasis Device - select based on settings : index1
						: std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device)->
						getTrackedJoints()[tracker.selectedTrackedJointID].getJointOrientation();

				// Copy the previous orientation to the tracker
				tracker.pose_previousOrientation =
					_device.index() == 0

						// SkeletonBasis Device - grab L or R depending on flip : index0
						? (base_flip

							   // If flip
							   ? std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(_device)->
							     getTrackedJoints()[
								     overrides::getFlippedJointType(
									     ITrackerType_Joint[tracker.base_tracker])].
							     getPreviousJointOrientation().inverse()

							   // If no flip
							   : std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(_device)->
							   getTrackedJoints()[ITrackerType_Joint[tracker.base_tracker]].
							   getPreviousJointOrientation()
						)

						// JointsBasis Device - select based on settings : index1
						: std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device)->
						getTrackedJoints()[tracker.selectedTrackedJointID].getPreviousJointOrientation();

				// Optionally overwrite the rotation with HMD orientation
				// Not the "calibrated" variant, as the fix will be applied after everything else
				if (tracker.orientationTrackingOption == k2_FollowHMDRotation)
				{
					tracker.pose_orientation = EigenUtils::EulersToQuat(
						Eigen::Vector3d(0, plugins::plugins_getHMDOrientationYaw(), 0));

					tracker.pose_previousOrientation = EigenUtils::EulersToQuat(
						Eigen::Vector3d(0, plugins::plugins_getHMDOrientationYaw(), 0));
				}

				// Optionally overwrite the rotation with NONE
				if (tracker.orientationTrackingOption == k2_DisableJointRotation)
				{
					tracker.pose_orientation = Eigen::Quaterniond(1, 0, 0, 0);
					tracker.pose_previousOrientation = Eigen::Quaterniond(1, 0, 0, 0);
				}
			}

			/*
			 * Trackers orientation - preparations : No-Yaw mode
			 */

			// We've got no no-yaw mode in amethyst (✿◕‿◕✿)

			/*
			 * Trackers orientation - preparations : App / Software rotation
			 *   Note: This is only available for feet as for now
			 */

			if (_device.index() == 0 &&
				(K2Settings.K2TrackersVector[1].orientationTrackingOption == k2_SoftwareCalculatedRotation ||
					K2Settings.K2TrackersVector[2].orientationTrackingOption == k2_SoftwareCalculatedRotation))

				TrackingDevices::Math::CalculateFeetSoftwareOrientation(
					std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(_device));

			/*
			 * Trackers orientation - preparations : App / Software rotation (V2)
			 *   Note: This is only available for feet as for now
			 */

			if (_device.index() == 0 &&
				(K2Settings.K2TrackersVector[1].orientationTrackingOption == k2_SoftwareCalculatedRotation_V2 ||
					K2Settings.K2TrackersVector[2].orientationTrackingOption == k2_SoftwareCalculatedRotation_V2))

				TrackingDevices::Math::CalculateFeetSoftwareOrientation_V2(
					std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(_device));

			/*
			 * Trackers orientation - Calibration-related fixes (SkeletonBasis-only)
			 */

			if (_device.index() == 0)
			{
				// Construct an offset quaternion with the calibration yaw
				const Eigen::Quaterniond yawFlipQuaternion =
					EigenUtils::EulersToQuat(Eigen::Vector3d(0.f, _PI, 0.f)); // Just turn around the yaw

				/*
				 * Tweak the rotation a bit while we're in flip: mirror y and z
				 */

				if (base_flip) // Alter rotation a bit if in the flip mode
					for (auto& tracker : K2Settings.K2TrackersVector)
						if (tracker.orientationTrackingOption != k2_FollowHMDRotation)
						{
							{
								// Remove the pitch angle
								// Grab original orientations and make them euler angles
								Eigen::Vector3d tracker_ori_with_yaw =
									EigenUtils::QuatToEulers(tracker.pose_orientation);

								// Remove pitch from eulers and apply to the parent
								tracker.pose_orientation = EigenUtils::EulersToQuat(
									Eigen::Vector3d(
										tracker_ori_with_yaw.x(),
										-tracker_ori_with_yaw.y(),
										-tracker_ori_with_yaw.z()));

								// Apply the turn-around flip quaternion
								tracker.pose_orientation =
									yawFlipQuaternion * tracker.pose_orientation;
							}
							{
								// Remove the pitch angle
								// Grab original orientations and make them euler angles
								Eigen::Vector3d tracker_ori_with_yaw =
									EigenUtils::QuatToEulers(tracker.pose_previousOrientation);

								// Remove pitch from eulers and apply to the parent
								tracker.pose_previousOrientation = EigenUtils::EulersToQuat(
									Eigen::Vector3d(
										tracker_ori_with_yaw.x(),
										-tracker_ori_with_yaw.y(),
										-tracker_ori_with_yaw.z()));

								// Apply the turn-around flip quaternion
								tracker.pose_previousOrientation =
									yawFlipQuaternion * tracker.pose_previousOrientation;
							}
						}
			}

			/*****************************************************************************************/
			// Fix waist orientation when following hmd after a standing pose reset
			/*****************************************************************************************/

			// Loop over all trackers
			for (auto& tracker : K2Settings.K2TrackersVector)
				if (tracker.orientationTrackingOption == k2_FollowHMDRotation)
				{
					// Offset to fit the playspace
					tracker.pose_orientation =
						vrPlayspaceOrientationQuaternion.inverse() * tracker.pose_orientation;

					tracker.pose_previousOrientation =
						vrPlayspaceOrientationQuaternion.inverse() * tracker.pose_previousOrientation;
				}

			/*****************************************************************************************/
			// Push RAW poses and physics to trackers, update physics
			/*****************************************************************************************/

			if (_device.index() == 0)
			{
				const auto& _kinect =
					std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(_device);

				for (auto& tracker : K2Settings.K2TrackersVector)
				{
					const auto& _joint = _kinect->getTrackedJoints()[
						overrides::getFlippedJointType(
							ITrackerType_Joint[tracker.base_tracker], base_flip)];

					tracker.pose_poseTimestamp = _joint.getPoseTimestamp();
					tracker.pose_previousPoseTimestamp = _joint.getPreviousPoseTimestamp();

					tracker.pose_position = _joint.getJointPosition();
					tracker.pose_previousPosition = _joint.getPreviousJointPosition();

					tracker.m_no_position_filtering_requested =
						_kinect->isPositionFilterBlockingEnabled();

					// If the device overrides physics
					if (_kinect->isPhysicsOverrideEnabled())
					{
						tracker.pose_velocity = _joint.getJointVelocity();
						tracker.pose_acceleration = _joint.getJointAcceleration();
						tracker.pose_angularVelocity = _joint.getJointAngularVelocity();
						tracker.pose_angularAcceleration = _joint.getJointAngularAcceleration();
					}
					// If not and the tracker is not overriden
					else if (!tracker.isPositionOverridden ||
						!K2Settings.overrideDeviceGUIDsMap.contains(tracker.overrideGUID))
						tracker.updateInternalPhysics();
				}
			}
			else if (_device.index() == 1)
			{
				const auto& _joints =
					std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device);

				for (auto& tracker : K2Settings.K2TrackersVector)
				{
					const auto& _joint = _joints->getTrackedJoints().at(
						tracker.selectedTrackedJointID);

					tracker.pose_poseTimestamp = _joint.getPoseTimestamp();
					tracker.pose_previousPoseTimestamp = _joint.getPreviousPoseTimestamp();

					tracker.pose_position = _joint.getJointPosition();
					tracker.pose_previousPosition = _joint.getPreviousJointPosition();

					tracker.m_no_position_filtering_requested =
						_joints->isPositionFilterBlockingEnabled();

					// If the device overrides physics
					if (_joints->isPhysicsOverrideEnabled())
					{
						tracker.pose_velocity = _joint.getJointVelocity();
						tracker.pose_acceleration = _joint.getJointAcceleration();
						tracker.pose_angularVelocity = _joint.getJointAngularVelocity();
						tracker.pose_angularAcceleration = _joint.getJointAngularAcceleration();
					}
					// If not and the tracker is not overriden
					else if (!tracker.isPositionOverridden ||
						!K2Settings.overrideDeviceGUIDsMap.contains(tracker.overrideGUID))
						tracker.updateInternalPhysics();
				}
			}
		}


		/*
		 * Calculate ALL poses for the override (second) device here
		 */

		// Override
		{
			// Loop over all the overrides (auto-handles if none)

			// Loop over all the override indexes and check them
			for (auto& [_override_guid, _override_id] : K2Settings.overrideDeviceGUIDsMap)
			{
				// Check the device
				if (!TrackingDevices::deviceGUID_ID_Map.contains(_override_guid))
					continue;

				/* Strategy:
				 *   overwrite base device's poses, optionally apply flip
				 *	 note that unlike in legacy versions, flip isn't anymore
				 *	 applied on pose pushes; this will allow us to apply
				 *	 two (or even more) independent flips, after the base
				 */

				// Get the current override device
				const auto& _device = TrackingDevices::TrackingDevicesVector.at(_override_id);

				//// Compose the yaw neutral and current
				//const double _neutral_yaw =
				//	(K2Settings.isFlipEnabled && K2Settings.isExternalFlipEnabled)
				//		? K2Settings.externalFlipCalibrationYaw // Ext
				//		: K2Settings.deviceCalibrationYaws[
				//			_override_guid]; // Default

				//// Flip preparations
				//double _current_yaw = _yaw; // Default - HMD
				//if (K2Settings.isFlipEnabled && K2Settings.isExternalFlipEnabled)
				//{
				//	// If the extflip is from Amethyst
				//	if (K2Settings.K2TrackersVector[0].data_isActive &&
				//		K2Settings.K2TrackersVector[0].isRotationOverridden)
				//	{
				//		_current_yaw = _neutral_yaw; // Never flip overrides if so
				//	}
				//	// If it's from an external tracker
				//	else
				//	{
				//		_current_yaw = EigenUtils::RotationProjectedYaw( // External tracker
				//			getVRTrackerPoseCalibrated("waist").second);
				//	}
				//}

				//// Compose flip
				//const double _facing = EigenUtils::RotationProjectedYaw(
				//	EigenUtils::QuaternionFromYaw<double>(_neutral_yaw).inverse() *
				//	EigenUtils::QuaternionFromYaw<double>(_current_yaw));

				//// Note: we use -180+180 (but in radians)
				//if (_facing <= (25 * _PI / 180.0) &&
				//	_facing >= (-25 * _PI / 180.0))
				//	override_flip = false;
				//if (_facing >= (155 * _PI / 180.0) ||
				//	_facing <= (-155 * _PI / 180.0))
				//	override_flip = true;

				//// Overwrite flip value depending on device & settings
				//if (!K2Settings.isFlipEnabled // If flip is disabled

				//	// If this device handles ext-flip
				//	|| (K2Settings.isExternalFlipEnabled &&
				//		K2Settings.K2TrackersVector[0].overrideGUID == _override_guid)

				//	// If this device is a JointsBasis
				//	|| _device.index() == 1)
				//	override_flip = false;

				// Currently, flipping overrides IS NOT supported
				override_flip = false; // TODO SHOULD WE ENABLE??
				// Currently, flipping overrides IS NOT supported

				/*
				 * Trackers orientation - preparations
				 */

				// Check if we need to apply it, skip if it's set to either hmd or none
				for (auto& tracker : K2Settings.K2TrackersVector)
					if (tracker.isRotationOverridden && // If an override is enabled
						tracker.overrideGUID == _override_guid && // If it's from this device

						// If the selected rotation option allows orientation tracking
						(tracker.orientationTrackingOption == k2_DeviceInferredRotation ||
							tracker.orientationTrackingOption == k2_SoftwareCalculatedRotation ||
							tracker.orientationTrackingOption == k2_SoftwareCalculatedRotation_V2))
					{
						// Copy the orientation to the tracker
						tracker.pose_orientation =
							_device.index() == 0

								// SkeletonBasis Device - grab L or R depending on flip : index1
								? (override_flip

									   // If flip
									   ? std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(
										     _device)->
									     getTrackedJoints()[
										     overrides::getFlippedJointType(
											     static_cast<ktvr::ITrackedJointType>(
												     TrackingDevices::devices_override_joint_id(
													     _override_id, tracker.overrideJointID)))].
									     getJointOrientation().inverse() // (Inverted orientation - flipped)

									   // If no flip
									   : std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(
										   _device)->
									   getTrackedJoints()[
										   TrackingDevices::devices_override_joint_id(
											   _override_id, tracker.overrideJointID)].getJointOrientation()
								)

								// JointsBasis Device - select based on settings : index1
								: std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device)->
								getTrackedJoints()[
									TrackingDevices::devices_override_joint_id(
										_override_id, tracker.overrideJointID)].getJointOrientation();

						// Copy the orientation to the tracker
						tracker.pose_previousOrientation =
							_device.index() == 0

								// SkeletonBasis Device - grab L or R depending on flip : index1
								? (override_flip

									   // If flip
									   ? std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(
										     _device)->
									     getTrackedJoints()[
										     overrides::getFlippedJointType(
											     static_cast<ktvr::ITrackedJointType>(
												     TrackingDevices::devices_override_joint_id(
													     _override_id, tracker.overrideJointID)))].
									     getPreviousJointOrientation().inverse() // (Inverted orientation - flipped)

									   // If no flip
									   : std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(
										   _device)->
									   getTrackedJoints()[
										   TrackingDevices::devices_override_joint_id(
											   _override_id, tracker.overrideJointID)].getPreviousJointOrientation()
								)

								// JointsBasis Device - select based on settings : index1
								: std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device)->
								getTrackedJoints()[
									TrackingDevices::devices_override_joint_id(
										_override_id, tracker.overrideJointID)].getPreviousJointOrientation();
					}

				/*
				 * Trackers orientation - preparations : No-Yaw mode
				 */

				// We've got no no-yaw mode in amethyst (✿◕‿◕✿)

				/*
				 * Trackers orientation - preparations : App / Software rotation
				 */

				// We've got no math-based for override devices (✿◕‿◕✿)

				/*
				 * Trackers orientation - Calibration-related fixes (SkeletonBasis-only)
				 */

				if (_device.index() == 0)
				{
					// Construct an offset quaternion with the calibration yaw
					const Eigen::Quaterniond yawFlipQuaternion =
						EigenUtils::EulersToQuat(Eigen::Vector3d(0.f, _PI, 0.f)); // Just turn around the yaw

					/*
					 * Tweak the rotation a bit while we're in flip: mirror y and z
					 */

					if (override_flip) // Tweak trackers orientation if in flip mode
						for (auto& tracker : K2Settings.K2TrackersVector)
							if (tracker.isRotationOverridden && // If an override is enabled
								tracker.overrideGUID == _override_guid && // If it's from this device

								// If the selected rotation option forces HMD orientation
								tracker.orientationTrackingOption != k2_FollowHMDRotation)
							{
								{
									// Remove the pitch angle
									// Grab original orientations and make them euler angles
									Eigen::Vector3d tracker_ori_with_yaw =
										EigenUtils::QuatToEulers(tracker.pose_orientation);

									// Remove pitch from eulers and apply to the parent
									tracker.pose_orientation = EigenUtils::EulersToQuat(
										Eigen::Vector3d(
											tracker_ori_with_yaw.x(),
											-tracker_ori_with_yaw.y(),
											-tracker_ori_with_yaw.z()));

									// Apply the turn-around flip quaternion
									tracker.pose_orientation =
										yawFlipQuaternion * tracker.pose_orientation;
								}
								{
									// Remove the pitch angle
									// Grab original orientations and make them euler angles
									Eigen::Vector3d tracker_ori_with_yaw =
										EigenUtils::QuatToEulers(tracker.pose_previousOrientation);

									// Remove pitch from eulers and apply to the parent
									tracker.pose_previousOrientation = EigenUtils::EulersToQuat(
										Eigen::Vector3d(
											tracker_ori_with_yaw.x(),
											-tracker_ori_with_yaw.y(),
											-tracker_ori_with_yaw.z()));

									// Apply the turn-around flip quaternion
									tracker.pose_previousOrientation =
										yawFlipQuaternion * tracker.pose_previousOrientation;
								}
							}
				}

				/*****************************************************************************************/
				// Push RAW poses and physics to trackers, update physics
				/*****************************************************************************************/

				if (_device.index() == 0)
				{
					const auto& _kinect =
						std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(_device);

					for (auto& tracker : K2Settings.K2TrackersVector)
						if (tracker.isPositionOverridden && // If an override is enabled
							tracker.overrideGUID == _override_guid) // If it's from this device
						{
							const auto& _joint = _kinect->getTrackedJoints()[
								overrides::getFlippedJointType(
									static_cast<ktvr::ITrackedJointType>(
										TrackingDevices::devices_override_joint_id(
											_override_id, tracker.overrideJointID)), override_flip)];

							tracker.pose_poseTimestamp = _joint.getPoseTimestamp();
							tracker.pose_previousPoseTimestamp = _joint.getPreviousPoseTimestamp();

							tracker.pose_position = _joint.getJointPosition();
							tracker.pose_previousPosition = _joint.getPreviousJointPosition();

							tracker.m_no_position_filtering_requested =
								_kinect->isPositionFilterBlockingEnabled();

							// If the device overrides physics
							if (_kinect->isPhysicsOverrideEnabled())
							{
								tracker.pose_velocity = _joint.getJointVelocity();
								tracker.pose_acceleration = _joint.getJointAcceleration();
								tracker.pose_angularVelocity = _joint.getJointAngularVelocity();
								tracker.pose_angularAcceleration = _joint.getJointAngularAcceleration();
							}
							else tracker.updateInternalPhysics();
						}
				}
				else if (_device.index() == 1)
				{
					const auto& _joints =
						std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device);

					for (auto& tracker : K2Settings.K2TrackersVector)
						if (tracker.isPositionOverridden && // If an override is enabled
							tracker.overrideGUID == _override_guid) // If it's from this device
						{
							const auto& _joint = _joints->getTrackedJoints().at(
								tracker.overrideJointID);

							tracker.pose_poseTimestamp = _joint.getPoseTimestamp();
							tracker.pose_previousPoseTimestamp = _joint.getPreviousPoseTimestamp();

							tracker.pose_position = _joint.getJointPosition();
							tracker.pose_previousPosition = _joint.getPreviousJointPosition();

							tracker.m_no_position_filtering_requested =
								_joints->isPositionFilterBlockingEnabled();

							// If the device overrides physics
							if (_joints->isPhysicsOverrideEnabled())
							{
								tracker.pose_velocity = _joint.getJointVelocity();
								tracker.pose_acceleration = _joint.getJointAcceleration();
								tracker.pose_angularVelocity = _joint.getJointAngularVelocity();
								tracker.pose_angularAcceleration = _joint.getJointAngularAcceleration();
							}
							else tracker.updateInternalPhysics();
						}
				}
			}
		}
	}

	// The main program loop
	inline void K2MainLoop()
	{
		// Warning: this is meant to work as fire-and-forget
		LOG(INFO) << "[K2Main] Waiting for the start sem to open..";
		shared::devices::smphSignalStartMain.acquire();

		LOG(INFO) << "[K2Main] Starting the status updater loop now...";

		std::thread([&]
		{
			bool _refresh_running = false;
			while (true)
			{
				if (!_refresh_running && !interfacing::isExitingNow)
				{
					// Mark the update as pending
					_refresh_running = true;

					// Parse the request - update
					shared::main::thisDispatcherQueue->TryEnqueue([&]
					{
						interfacing::statusUIRefreshRequested = false;
						interfacing::statusUIRefreshRequested_Urgent = false;

						// Update only the currently needed one
						// (Will run only if anything has changed)
						if (interfacing::currentPageTag == L"devices")
						{
							TrackingDevices::devices_handle_refresh(false);

							if (shared::devices::deviceErrorGrid->Visibility() ==
								winrt::Microsoft::UI::Xaml::Visibility::Visible)
								interfacing::statusUIRefreshRequested = true; // Redo
						}
						else
						{
							TrackingDevices::updateTrackingDevicesUI();

							if (shared::general::errorWhatGrid->Visibility() ==
								winrt::Microsoft::UI::Xaml::Visibility::Visible)
								interfacing::statusUIRefreshRequested = true; // Redo
						}

						// We're done now
						_refresh_running = false;
					});
				}
				else
				// Wait 14s until the next refresh (or until a request)
					for (uint32_t i = 0; i < 8; i++)
					{
						for (int j = 0; j < 4; j++)
						{
							if (interfacing::statusUIRefreshRequested_Urgent)break;
							std::this_thread::sleep_for(std::chrono::milliseconds(500));
						}

						// In an outer loop for longer cooldown
						if (interfacing::statusUIRefreshRequested)break;
					}
			}
		}).detach();

		LOG(INFO) << "[K2Main] Starting the main app loop now...";

		// For limiting loop 'fps'
		using clock = std::chrono::steady_clock;
		auto next_frame = clock::now();
		const auto vr_frame_rate =
			std::clamp(vr::VRSystem()->GetFloatTrackedDeviceProperty(0, vr::Prop_DisplayFrequency_Float),
			           70.f, 130.f) + 20; // Get VR HMD fps, fallback to (70~130) + 20(Virtual)

		LOG(INFO) << "Desired loop time: " <<
			std::chrono::nanoseconds(
				static_cast<int>(1000000000. / vr_frame_rate)).count() <<
			"ns (" << vr_frame_rate << "FPS)";

		// Errors' case
		int server_tries = 0, server_loops = 0;

		while (true)
		{
			try
			{
				// run until termination
				while (true)
				{
					auto loop_start_time = std::chrono::high_resolution_clock::now();

					/* Check if we have vr framerate, not to divide by 0 and,
						if there is no vr running on hmd / error, run at 60 fps*/
					next_frame += std::chrono::nanoseconds(
						static_cast<int>(1000000000.f / vr_frame_rate));

					/* Update things here */

					K2UpdateVRPositions(); // Update HMD poses
					K2UpdateInputBindings(); // Update input
					K2ParseVREvents(); // Parse VR events

					// Skip some things if we're getting ready to exit
					if (!interfacing::isExitingNow)
					{
						K2UpdateTrackingDevices(); // Update actual tracking
						K2UpdateAppTrackers(); // Track joints from raw data
					}

					K2UpdateServerTrackers(); // Send it to the server

					// Wait until certain loop time has passed
					if (const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
							std::chrono::high_resolution_clock::now() - loop_start_time).count();
						duration <= 7000000.f) // Try to run peacefully at MAX @144hz
					{
						// Sleep until next frame if it haven't pass yet
						std::this_thread::sleep_until(next_frame);

						// Check the loop time occasionally
						if (server_loops >= 10000)
						{
							server_loops = 0; // Reset the counter
							const auto fixed_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
								std::chrono::high_resolution_clock::now() - loop_start_time).count();

							LOG(INFO) << "10000 loops have passed: this loop took " << duration <<
								"ns, the loop's time after time correction (sleep) is: " << fixed_duration << "ns";
						}
						else server_loops++;
					}

					// Cry if running below 30fps
					else if (duration > 35000000.f)
						LOG(WARNING) << "Can't keep up! The last loop took " <<
							std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(
								std::chrono::high_resolution_clock::now() - loop_start_time).count()) <<
							"ns. (Ran at approximately " <<
							std::to_string(1000000000.f / std::chrono::duration_cast<std::chrono::nanoseconds>(
								std::chrono::high_resolution_clock::now() - loop_start_time).count()) << "fps)";
				}
			}
			catch (...) // Catch everything
			{
				LOG(ERROR) << "The main loop has crashed! Restarting it now...";

				server_tries++; // One more?
				if (server_tries > 3 && server_tries <= 7)
				{
					// We've crashed the third time now. Somethin's off.. really...
					LOG(ERROR) << "Server loop has already crashed 3 times. Checking the joint config...";

					// Check the joint configuration
					TrackingDevices::devices_check_base_ids();
					TrackingDevices::devices_check_override_ids(K2Settings.trackingDeviceGUIDPair.second);
				}
				else if (server_tries > 7)
				{
					// We've crashed the seventh time now. Somethin's off.. really...
					LOG(ERROR) << "Server loop has already crashed 7 times. Giving up...";

					// Mark exiting as true
					interfacing::isExitingNow = true;

					// Mark trackers as inactive
					interfacing::K2AppTrackersInitialized = false;

					// Wait a moment
					Sleep(200);

					interfacing::_fail(-13); // -13 is the code for giving up then, I guess
					// user will be prompted to reset the config (opt)
				}
			}
		}
	}
}
