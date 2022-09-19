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

			shared::main::thisDispatcherQueue.get()->TryEnqueue([&]
			{
				if (shared::general::toggleFreezeButton.get() != nullptr)
				{
					shared::general::general_tab_setup_finished = false; // Boiler
					shared::general::toggleFreezeButton.get()->IsChecked(isTrackingFrozen);
					shared::general::toggleFreezeButton.get()->Content(isTrackingFrozen
						                                                   ? winrt::box_value(L"Unfreeze")
						                                                   : winrt::box_value(L"Freeze"));
					shared::general::general_tab_setup_finished = true; // Boiler end
				}
			});

			{
				auto _header = LocalizedJSONString(
					L"/GeneralPage/Tips/TrackingFreeze/Header");

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

				stringReplaceAll(_header,
				                 L"also toggle tracker freeze while in VR"s, L"toggle it"s);

				ShowVRToast(std::wstring(L"Tracking Freeze ") +
				            (isTrackingFrozen ? L"enabled!" : L"disabled!"), _header);
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

			shared::main::thisDispatcherQueue.get()->TryEnqueue([&]
			{
				if (shared::settings::flipToggle.get() != nullptr)
				{
					shared::settings::settings_localInitFinished = false; // Boiler
					shared::settings::flipToggle.get()->IsOn(K2Settings.isFlipEnabled);
					shared::settings::settings_localInitFinished = true; // Boiler end
				}
			});

			{
				auto _header = LocalizedJSONString(
					L"/SettingsPage/Tips/FlipToggle/Header");

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

				stringReplaceAll(_header,
				                 L"also toggle skeleton flip while in VR"s, L"toggle it"s);

				ShowVRToast(std::wstring(L"Skeleton Flip ") +
				            (K2Settings.isFlipEnabled ? L"enabled!" : L"disabled!"), _header);
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
				pDevice->update(); // Update the device
				interfacing::kinectHeadPosition.first = pDevice->getJointPositions()[ktvr::Joint_Head];
				interfacing::kinectWaistPosition.first = pDevice->getJointPositions()[ktvr::Joint_SpineWaist];
			}
			break;
		case 1:
			{
				const auto& pDevice = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(device);
				pDevice->update(); // Update the device
				if (K2Settings.K2TrackersVector[0].selectedTrackedJointID < pDevice->getTrackedJoints().size())
					interfacing::kinectWaistPosition.first = pDevice->getTrackedJoints().at(
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
					pDevice->update(); // Update the device
					interfacing::kinectHeadPosition.second = pDevice->getJointPositions()[ktvr::Joint_Head];
					interfacing::kinectWaistPosition.second = pDevice->getJointPositions()[ktvr::Joint_SpineWaist];
				}
				break;
			case 1:
				{
					const auto& pDevice = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
						TrackingDevices::TrackingDevicesVector[override_id]);
					pDevice->update(); // Update the device
					if (K2Settings.K2TrackersVector[0].selectedTrackedJointID < pDevice->getTrackedJoints().size())
						interfacing::kinectWaistPosition.second = pDevice->getTrackedJoints().at(
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

					// If not overridden by second device
					if (!tracker.isPositionOverridden || K2Settings.overrideDeviceGUIDsMap.empty())
					{
						if (K2Settings.isMatrixCalibrated.first)
							k2_tracker_bases.push_back(
								tracker.getTrackerBase
								(
									K2Settings.calibrationRotationMatrices.first.cast<float>(),
									K2Settings.calibrationTranslationVectors.first.cast<float>(),
									K2Settings.calibrationOrigins.first.cast<float>(),
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
					// If overriden by second device
					else
					{
						if (K2Settings.isMatrixCalibrated.second)
							k2_tracker_bases.push_back(
								tracker.getTrackerBase
								(
									K2Settings.calibrationRotationMatrices.second.cast<float>(),
									K2Settings.calibrationTranslationVectors.second.cast<float>(),
									K2Settings.calibrationOrigins.second.cast<float>(),
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
								std::make_pair(tracker.base_tracker, interfacing::K2AppTrackersInitialized));

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
					interfacing::vrControllerIndexes =
						std::make_pair(vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(
							               vr::ETrackedControllerRole::TrackedControllerRole_RightHand),
						               vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(
							               vr::ETrackedControllerRole::TrackedControllerRole_LeftHand));
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
					shared::main::thisDispatcherQueue.get()->TryEnqueue(
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

		// Get current yaw angle
		const double _yaw =
			plugins::plugins_getHMDOrientationYawCalibrated();

		/*
		 * Calculate ALL poses for the base (first) device here
		 */

		// Base device
		{
			// Get the currently tracking device
			const auto& _device = TrackingDevices::getCurrentDevice();

			// Compose the yaw neutral and current
			const double _neutral_yaw =
			((K2Settings.isFlipEnabled && K2Settings.isExternalFlipEnabled)
				 ? K2Settings.externalFlipCalibrationYaw // Ext
				 : K2Settings.calibrationYaws.first); // Default

			double _current_yaw = _yaw; // Default - HMD
			if (K2Settings.isFlipEnabled && K2Settings.isExternalFlipEnabled)
			{
				// If the extflip is from Amethyst
				if (K2Settings.K2TrackersVector[0].data_isActive &&
					K2Settings.K2TrackersVector[0].isRotationOverridden)
				{
					_current_yaw =
						EigenUtils::RotationProjectedYaw( // Overriden tracker
							vrPlayspaceOrientationQuaternion.inverse() * // VR space offset
							K2Settings.K2TrackersVector[0].pose_orientation); // Raw orientation
				}
				// If it's from an external tracker
				else
				{
					_current_yaw =
						EigenUtils::RotationProjectedYaw( // External tracker
							getVRTrackerPoseCalibrated("waist").second);
				}
			}

			// Compose flip
			const double _facing = EigenUtils::RotationProjectedYaw(
				EigenUtils::QuaternionFromYaw<double>(_neutral_yaw).inverse() *
				EigenUtils::QuaternionFromYaw<double>(_current_yaw));

			// Note: we use -180+180 (but in radians)
			if (_facing <= (25 * _PI / 180.0) &&
				_facing >= (-25 * _PI / 180.0))
				base_flip = false;
			if (_facing >= (155 * _PI / 180.0) ||
				_facing <= (-155 * _PI / 180.0))
				base_flip = true;

			// Overwrite flip value depending on device & settings
			// index() check should've already been done by the app tho
			if (!K2Settings.isFlipEnabled || _device.index() == 1)base_flip = false;

			/*
			 * Trackers orientation - preparations
			 */

			for (auto& tracker : K2Settings.K2TrackersVector)
			{
				// Copy the orientation to the tracker
				tracker.pose_orientation = _device.index() == 0

					                           // SkeletonBasis Device - grab L or R depending on flip : index0
					                           ? (base_flip

						                              // If flip
						                              ? std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(_device)->
						                              getJointOrientations()[
							                              overrides::getFlippedJointType(
								                              ITrackerType_Joint[tracker.base_tracker])].inverse()

						                              // If no flip
						                              : std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(_device)->
						                              getJointOrientations()[ITrackerType_Joint[tracker.base_tracker]]
					                           )

					                           // JointsBasis Device - select based on settings : index1
					                           : std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device)->
					                           getTrackedJoints()[tracker.selectedTrackedJointID].getJointOrientation();

				// Optionally overwrite the rotation with HMD orientation
				// Not the "calibrated" variant, as the fix will be applied after everything else
				if (tracker.orientationTrackingOption == k2_FollowHMDRotation)
					tracker.pose_orientation = EigenUtils::EulersToQuat(
						Eigen::Vector3f(0, plugins::plugins_getHMDOrientationYaw(), 0));

				// Optionally overwrite the rotation with NONE
				if (tracker.orientationTrackingOption == k2_DisableJointRotation)
					tracker.pose_orientation = Eigen::Quaternionf(1, 0, 0, 0);
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
				Eigen::Quaternionf yawFlipQuaternion =
					EigenUtils::EulersToQuat(Eigen::Vector3f(0.f, _PI, 0.f)); // Just turn around the yaw

				/*
				 * Tweak the rotation a bit while we're in flip: mirror y and z
				 * Apply calibration rotation offset: faster+better+more_chad than mere eulers
				 */

				if (base_flip)
				{
					// Alter rotation a bit in the flip mode (using the calibration matices)
					for (auto& tracker : K2Settings.K2TrackersVector)
						if (tracker.orientationTrackingOption != k2_FollowHMDRotation)
						{
							// Remove the pitch angle
							// Grab original orientations and make them euler angles
							Eigen::Vector3f tracker_ori_with_yaw =
								EigenUtils::QuatToEulers(tracker.pose_orientation);

							// Remove pitch from eulers and apply to the parent
							tracker.pose_orientation = EigenUtils::EulersToQuat(
								Eigen::Vector3f(
									tracker_ori_with_yaw.x(),
									-tracker_ori_with_yaw.y(),
									-tracker_ori_with_yaw.z()));

							// Apply the turn-around flip quaternion
							tracker.pose_orientation =
								yawFlipQuaternion * tracker.pose_orientation;

							// Fix orientations with the R calibration value
							if (tracker.orientationTrackingOption != k2_DisableJointRotation &&
								!K2Settings.calibrationRotationMatrices.first.isZero())
								tracker.pose_orientation =
									K2Settings.calibrationRotationMatrices.first.cast<float>() *
									tracker.pose_orientation;
						}
				}
				else
				{
					// Fix orientations with the R calibration value
					for (auto& tracker : K2Settings.K2TrackersVector)
						if (tracker.orientationTrackingOption != k2_DisableJointRotation &&
							tracker.orientationTrackingOption != k2_FollowHMDRotation &&
							!K2Settings.calibrationRotationMatrices.first.isZero())
							tracker.pose_orientation =
								K2Settings.calibrationRotationMatrices.first.cast<float>() *
								tracker.pose_orientation;
				}
			}

			/*****************************************************************************************/
			// Fix waist orientation when following hmd after a standing pose reset
			/*****************************************************************************************/

			// Loop over all trackers
			for (auto& tracker : K2Settings.K2TrackersVector)
				if (tracker.orientationTrackingOption == k2_FollowHMDRotation)
					// Offset to fit the playspace
					tracker.pose_orientation =
						vrPlayspaceOrientationQuaternion.inverse() * tracker.pose_orientation;

			/*****************************************************************************************/
			// Push RAW poses to trackers
			/*****************************************************************************************/

			if (_device.index() == 0)
			{
				const auto& _kinect =
					std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(_device);

				for (auto& tracker : K2Settings.K2TrackersVector)
					tracker.pose_position = _kinect->getJointPositions()[
						overrides::getFlippedJointType(
							ITrackerType_Joint[tracker.base_tracker], base_flip)];
			}
			else if (_device.index() == 1)
			{
				const auto& _joints =
					std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device);

				for (auto& tracker : K2Settings.K2TrackersVector)
					tracker.pose_position =
						_joints->getTrackedJoints().at(tracker.selectedTrackedJointID).getJointPosition();
			}
		}


		/*
		 * Calculate ALL poses for the override (second) device here
		 */

		// Override
		//{
		//	// If we even HAVE an override
		//	if (K2Settings.overrideDeviceID >= 0)
		//	{
		//		/* Strategy:
		//		 *   overwrite base device's poses, optionally apply flip
		//		 *	 note that unlike in legacy versions, flip isn't anymore
		//		 *	 applied on pose pushes; this will allow us to apply
		//		 *	 two (or even more) independent flips, after the base
		//		 */

		//		// Get the current override device
		//		const auto& _device = TrackingDevices::getCurrentOverrideDevice();

		//		// Compose the yaw neutral and current
		//		const double _neutral_yaw =
		//		((K2Settings.isFlipEnabled && K2Settings.isExternalFlipEnabled)
		//			 ? K2Settings.externalFlipCalibrationYaw // Ext
		//			 : K2Settings.calibrationYaws.second); // Default

		//		double _current_yaw = _yaw; // Default - HMD
		//		if (K2Settings.isFlipEnabled && K2Settings.isExternalFlipEnabled)
		//		{
		//			// If the extflip is from Amethyst
		//			if (K2Settings.K2TrackersVector[0].data_isActive &&
		//				K2Settings.K2TrackersVector[0].isRotationOverridden)
		//			{
		//				_current_yaw = _neutral_yaw; // Never flip overrides if so
		//			}
		//			// If it's from an external tracker
		//			else
		//			{
		//				_current_yaw = EigenUtils::RotationProjectedYaw( // External tracker
		//					getVRTrackerPoseCalibrated("waist").second);
		//			}
		//		}

		//		//// Compose flip
		//		//const double _facing = EigenUtils::RotationProjectedYaw(
		//		//	EigenUtils::QuaternionFromYaw<double>(_neutral_yaw).inverse() *
		//		//	EigenUtils::QuaternionFromYaw<double>(_current_yaw));

		//		//// Note: we use -180+180 (but in radians)
		//		//if (_facing <= (25 * _PI / 180.0) &&
		//		//	_facing >= (-25 * _PI / 180.0))
		//		//	override_flip = false;
		//		//if (_facing >= (155 * _PI / 180.0) ||
		//		//	_facing <= (-155 * _PI / 180.0))
		//		//	override_flip = true;

		//		//// Overwrite flip value depending on device & settings
		//		//// index() check should've already been done by the app tho
		//		//if (!K2Settings.isFlipEnabled || _device.index() == 1)override_flip = false;

		//		// Currently, flipping overrides IS NOT supported
		//		override_flip = false;

		//		/*
		//		 * Trackers orientation - preparations
		//		 */

		//		// Check if we need to apply it, skip if it's set to either hmd or none
		//		for (auto& tracker : K2Settings.K2TrackersVector)
		//			if (tracker.isRotationOverridden &&
		//				(tracker.orientationTrackingOption == k2_DeviceInferredRotation ||
		//					tracker.orientationTrackingOption == k2_SoftwareCalculatedRotation ||
		//					tracker.orientationTrackingOption == k2_SoftwareCalculatedRotation_V2))
		//			{
		//				// Copy the orientation to the tracker
		//				tracker.pose_orientation = _device.index() == 0

		//					                           // SkeletonBasis Device - grab L or R depending on flip : index1
		//					                           ? (override_flip

		//						                              // If flip
		//						                              ? std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(
		//							                              _device)->
		//						                              getJointOrientations()[
		//							                              overrides::getFlippedJointType(
		//								                              static_cast<ktvr::ITrackedJointType>(
		//									                              TrackingDevices::devices_override_joint_id(
		//										                              tracker.rotationOverrideJointID)))].
		//						                              inverse()

		//						                              // If no flip
		//						                              : std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(
		//							                              _device)->
		//						                              getJointOrientations()[
		//							                              TrackingDevices::devices_override_joint_id(
		//								                              tracker.rotationOverrideJointID)]
		//					                           )

		//					                           // JointsBasis Device - select based on settings : index1
		//					                           : std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device)->
		//					                           getTrackedJoints()[
		//						                           TrackingDevices::devices_override_joint_id(
		//							                           tracker.rotationOverrideJointID)].getJointOrientation();
		//			}

		//		/*
		//		 * Trackers orientation - preparations : No-Yaw mode
		//		 */

		//		// We've got no no-yaw mode in amethyst (✿◕‿◕✿)

		//		/*
		//		 * Trackers orientation - preparations : App / Software rotation
		//		 */

		//		// We've got no math-based for override devices (✿◕‿◕✿)

		//		/*
		//		 * Trackers orientation - Calibration-related fixes (SkeletonBasis-only)
		//		 */

		//		if (_device.index() == 0)
		//		{
		//			// Construct an offset quaternion with the calibration yaw
		//			Eigen::Quaternionf yawFlipQuaternion =
		//				EigenUtils::EulersToQuat(Eigen::Vector3f(0.f, _PI, 0.f)); // Just turn around the yaw

		//			/*
		//			 * Tweak the rotation a bit while we're in flip: mirror y and z
		//			 * Apply calibration rotation offset: faster+better+more_chad than mere eulers
		//			 */

		//			if (override_flip)
		//			{
		//				// Tweak trackers orientation in flip mode
		//				for (auto& tracker : K2Settings.K2TrackersVector)
		//					if (tracker.orientationTrackingOption != k2_FollowHMDRotation
		//						&& tracker.isRotationOverridden)
		//					{
		//						// Remove the pitch angle
		//						// Grab original orientations and make them euler angles
		//						Eigen::Vector3f tracker_ori_with_yaw =
		//							EigenUtils::QuatToEulers(tracker.pose_orientation);

		//						// Remove pitch from eulers and apply to the parent
		//						tracker.pose_orientation = EigenUtils::EulersToQuat(
		//							Eigen::Vector3f(
		//								tracker_ori_with_yaw.x(),
		//								-tracker_ori_with_yaw.y(),
		//								-tracker_ori_with_yaw.z()));

		//						// Apply the turn-around flip quaternion
		//						tracker.pose_orientation = yawFlipQuaternion * tracker.pose_orientation;

		//						// Fix orientations with the R calibration value
		//						if (tracker.orientationTrackingOption != k2_DisableJointRotation &&
		//							!K2Settings.calibrationRotationMatrices.second.isZero())
		//							tracker.pose_orientation =
		//								K2Settings.calibrationRotationMatrices.second.cast<float>() *
		//								tracker.pose_orientation;
		//					}
		//			}
		//			else
		//			// It'll make the tracker face the kinect
		//				for (auto& tracker : K2Settings.K2TrackersVector)
		//					if (tracker.orientationTrackingOption != k2_DisableJointRotation &&
		//						tracker.orientationTrackingOption != k2_FollowHMDRotation &&
		//						tracker.isRotationOverridden && !K2Settings.calibrationRotationMatrices.second.isZero())
		//						tracker.pose_orientation = K2Settings.calibrationRotationMatrices.second.cast<float>() *
		//							tracker.pose_orientation;
		//		}

		//		/*****************************************************************************************/
		//		// Push RAW poses to trackers
		//		/*****************************************************************************************/

		//		if (_device.index() == 0)
		//		{
		//			const auto& _kinect =
		//				std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(_device);

		//			for (auto& tracker : K2Settings.K2TrackersVector)
		//				if (tracker.isPositionOverridden)
		//					tracker.pose_position = _kinect->getJointPositions()[
		//						overrides::getFlippedJointType(
		//							static_cast<ktvr::ITrackedJointType>(
		//								TrackingDevices::devices_override_joint_id(
		//									tracker.positionOverrideJointID)), override_flip)];
		//		}
		//		else if (_device.index() == 1)
		//		{
		//			const auto& _joints =
		//				std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device);

		//			for (auto& tracker : K2Settings.K2TrackersVector)
		//				if (tracker.isPositionOverridden)
		//					tracker.pose_position =
		//						_joints->getTrackedJoints().at(tracker.positionOverrideJointID).getJointPosition();
		//		}
		//	}
		//}
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
					shared::main::thisDispatcherQueue.get()->TryEnqueue([&]
					{
						interfacing::statusUIRefreshRequested = false;
						interfacing::statusUIRefreshRequested_Urgent = false;

						// Update only the currently needed one
						// (Will run only if anything has changed)
						if (interfacing::currentPageTag == L"devices")
						{
							TrackingDevices::devices_handle_refresh(false);

							if (shared::devices::deviceErrorGrid.get()->Visibility() ==
								winrt::Microsoft::UI::Xaml::Visibility::Visible)
								interfacing::statusUIRefreshRequested = true; // Redo
						}
						else
						{
							TrackingDevices::updateTrackingDeviceUI();
							TrackingDevices::updateOverrideDeviceUI(); // Auto-handles if none

							if (shared::general::errorWhatGrid.get()->Visibility() ==
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
					if (auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
							std::chrono::high_resolution_clock::now() - loop_start_time).count();
						duration <= 10000000.f) // Try to run peacefully @100hz
					{
						std::this_thread::sleep_for(std::chrono::nanoseconds(10000000 - duration));
						if (server_loops >= 10000)
						{
							server_loops = 0; // Reset the counter
							LOG(INFO) << "10000 loops have passed: this loop took " << duration <<
								"ns, the loop's time after time correction (sleep) is: " <<
								std::chrono::duration_cast<std::chrono::nanoseconds>(
									std::chrono::high_resolution_clock::now() - loop_start_time).count() << "ns";
						}
						else server_loops++;
					}

					else if (duration > 35000000.f)
						LOG(WARNING) << "Can't keep up! The last loop took " <<
							std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(
								std::chrono::high_resolution_clock::now() - loop_start_time).count()) <<
							"ns. (Ran at approximately " <<
							std::to_string(1000000.f / std::chrono::duration_cast<std::chrono::microseconds>(
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
