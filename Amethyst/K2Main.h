#pragma once
#include "K2Interfacing.h"
#include "TrackingDevices.h"

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
			shared::main::thisDispatcherQueue.get()->TryEnqueue([&]
			{
				winrt::Microsoft::UI::Xaml::ElementSoundPlayer::Play(
					winrt::Microsoft::UI::Xaml::ElementSoundKind::Invoke);

				if (shared::general::toggleFreezeButton.get() != nullptr)
				{
					shared::general::toggleFreezeButton.get()->IsChecked(isTrackingFrozen);
					shared::general::toggleFreezeButton.get()->Content(isTrackingFrozen
						                                                   ? winrt::box_value(L"Unfreeze")
						                                                   : winrt::box_value(L"Freeze"));
				}
			});
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
					std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported()
						? !K2Settings.isFlipEnabled // If supported
						: false; // If not supported
			}
			else if (trackingDevice.index() == 1)
			{
				// Joints Basis
				K2Settings.isFlipEnabled = !false;
			}

			// Save settings
			K2Settings.saveSettings();

			// Play a Sound and Update UI
			shared::main::thisDispatcherQueue.get()->TryEnqueue([&]
			{
				winrt::Microsoft::UI::Xaml::ElementSoundPlayer::Play(
					winrt::Microsoft::UI::Xaml::ElementSoundKind::Invoke);
			});

			if (shared::settings::flipToggle.get() != nullptr)
				shared::settings::flipToggle.get()->IsOn(K2Settings.isFlipEnabled);
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

	inline void K2UpdateTrackingDevices()
	{
		/* Update the base device here */
		switch (const auto& device = TrackingDevices::
			getCurrentDevice(); device.index())
		{
		case 0:
			{
				const auto& pDevice = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(device);
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
		if (const auto& device_pair = TrackingDevices::
			getCurrentOverrideDevice_Safe(); device_pair.first)
			switch (device_pair.second.index())
			{
			case 0:
				{
					const auto& pDevice = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(device_pair.second);
					pDevice->update(); // Update the device
					interfacing::kinectHeadPosition.second = pDevice->getJointPositions()[ktvr::Joint_Head];
					interfacing::kinectWaistPosition.second = pDevice->getJointPositions()[ktvr::Joint_SpineWaist];
				}
				break;
			case 1:
				{
					const auto& pDevice = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(device_pair.second);
					pDevice->update(); // Update the device
					if (K2Settings.K2TrackersVector[0].selectedTrackedJointID < pDevice->getTrackedJoints().size())
						interfacing::kinectWaistPosition.second = pDevice->getTrackedJoints().at(
							K2Settings.K2TrackersVector[0].selectedTrackedJointID).getJointPosition();
				}
				break;
			}
	}

	inline int p_loops = 0, // Loops passed since last status update
	           p_frozen_loops = 0; // Loops passed since last frozen update
	inline bool initialized_bak = false; // Backup initialized? value
	inline void K2UpdateServerTrackers()
	{
		// Update only if we're connected
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
					for(const auto& tracker : K2Settings.K2TrackersVector)
						if(tracker.data.isActive)
							ktvr::refresh_tracker_pose<false>(tracker.tracker);
					
					// Reset
					p_loops = 0;
				}
				else p_loops++;
			}
			// If the tracing's actually running
			else
			{
				for (auto& tracker : K2Settings.K2TrackersVector) {
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
							if (tracker.data.isActive &&
								static_cast<int>(tracker.tracker) >= 16)
								ktvr::refresh_tracker_pose<false>(tracker.tracker);
						
						// Reset
						p_loops = 0;
					}
					else p_loops++;
				}

				// Update trackers in the vector recursively
				for (auto& tracker : K2Settings.K2TrackersVector) {

					// If lower body is omitted
					if (static_cast<int>(tracker.tracker) < 16 
						&& !_updateLowerBody) continue;

					// If the tracker is off
					if (!tracker.data.isActive) continue;

					// If not overridden by second device
					if (!tracker.isPositionOverridden || K2Settings.overrideDeviceID < 0)
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

			// Update status 1/1100 loops / ~15s
			// or right after any change
			for (int i = 0; i < 3; i++)
			{
				// try 3 times
				if (p_loops >= 1100 ||
					(initialized_bak != interfacing::K2AppTrackersInitialized))
				{
					// Scan for already-added body trackers from other apps
					// (If any found, disable corresponding ame's pairs)

					// Not that's a TODO for later
					//     k2app::K2Settings.checkForOverlappingTrackers
					//if (!interfacing::isAlreadyAddedTrackersScanRunning)
					//	shared::main::thisDispatcherQueue.get()->TryEnqueue(
					//		[&]()-> winrt::Windows::Foundation::IAsyncAction
					//		{
					//			using namespace winrt;
					//			interfacing::isAlreadyAddedTrackersScanRunning = true;
					//			bool wereChangesMade = false; // At least not yet

					//			// Do that on UI's background
					//			apartment_context ui_thread;
					//			co_await resume_background();
					//			const std::array foundTrackerFromPair
					//			{
					//				interfacing::findVRTracker("waist", false, true).first,
					//				(interfacing::findVRTracker("left_foot", false, true).first ||
					//					interfacing::findVRTracker("right_foot", false, true).first),
					//				(interfacing::findVRTracker("left_elbow", false, true).first ||
					//					interfacing::findVRTracker("right_elbow", false, true).first),
					//				(interfacing::findVRTracker("left_knee", false, true).first ||
					//					interfacing::findVRTracker("right_knee", false, true).first)
					//			};
					//			co_await ui_thread;

					//			// Disable the waist tracker if found
					//			for (uint32_t tracker_pair_index = 0; 
					//				tracker_pair_index < 4; tracker_pair_index++)
					//				if (K2Settings.isJointPairEnabled[tracker_pair_index] &&
					//					foundTrackerFromPair.at(tracker_pair_index))
					//				{
					//					// Make actual changes
					//					K2Settings.isJointPairEnabled[tracker_pair_index] = false;
					//					if (shared::settings::trackerPairEnabledToggles.at(tracker_pair_index).get() !=
					//						nullptr)
					//						shared::settings::trackerPairEnabledToggles.at(tracker_pair_index).get()->
					//							IsOn(false);

					//					switch(tracker_pair_index)
					//					{
					//					case 0:
					//						ktvr::set_tracker_state<false>(interfacing::K2TrackersVector.at(0).id, false);
					//						break;
					//					case 1:
					//						ktvr::set_tracker_state<false>(interfacing::K2TrackersVector.at(1).id, false);
					//						ktvr::set_tracker_state<false>(interfacing::K2TrackersVector.at(2).id, false);
					//						break;
					//					case 2:
					//						ktvr::set_tracker_state<false>(interfacing::K2TrackersVector.at(3).id, false);
					//						ktvr::set_tracker_state<false>(interfacing::K2TrackersVector.at(4).id, false);
					//						break;
					//					case 3:
					//						ktvr::set_tracker_state<false>(interfacing::K2TrackersVector.at(5).id, false);
					//						ktvr::set_tracker_state<false>(interfacing::K2TrackersVector.at(6).id, false);
					//						break;
					//					}

					//					// Check if we've disabled any joints from spawning and disable their mods
					//					interfacing::devices_check_disabled_joints();
					//					TrackingDevices::settings_trackersConfigChanged(false);
					//					
					//					// Save settings
					//					K2Settings.saveSettings();
					//					wereChangesMade = true;
					//				}

					//			// Check if anything's changed
					//			interfacing::isAlreadyAddedTrackersScanRunning = false;
					//			if (wereChangesMade)
					//				interfacing::ShowToast(
					//					interfacing::LocalizedResourceWString(
					//						L"SharedStrings", L"Toasts/TrackersAutoDisabled/Title"),
					//					interfacing::LocalizedResourceWString(
					//						L"SharedStrings", L"Toasts/TrackersAutoDisabled/Content"),
					//					true); // This one's gonna be a high-priority one
					//		});

					// Update status in server
					for (const auto& tracker : K2Settings.K2TrackersVector)
						if(tracker.data.isActive)
							ktvr::set_tracker_state<false>(tracker.tracker,
								interfacing::K2AppTrackersInitialized);

					// Update internal status
					initialized_bak = interfacing::K2AppTrackersInitialized;

					// Reset
					p_loops = 0;
				}
				else p_loops++;
			}
		}
	}

	// For App/Software Orientation filtering
	inline Eigen::Quaternionf yawFilteringQuaternion[2] =
	{
		Eigen::Quaternionf(1, 0, 0, 0),
		Eigen::Quaternionf(1, 0, 0, 0)
	}; // L, R

	// Flip defines for the base device - iteration persistent
	inline bool base_flip = false; // Assume non flipped

	// Flip defines for the override device - iteration persistent
	inline bool override_flip = false; // Assume non flipped

	// Update trackers inside the app here
	inline void K2UpdateAppTrackers()
	{
		/*
		 * This is where we do EVERYTHING pose related.
		 * All positions and rotations are calculated here,
		 * depending on calibration val and configuration
		 */

		// Get current yaw angle
		const float _yaw = radiansToDegrees(interfacing::plugins::plugins_getHMDOrientationYaw());

		/*
		 * Calculate ALL poses for the base (first) device here
		 */

		// Base device
		{
			// Get the currently tracking device
			const auto& _device = TrackingDevices::getCurrentDevice();

			// Compose the yaw neutral and current
			const double _neutral_yaw =
			(K2Settings.isExternalFlipEnabled
				 ? radiansToDegrees(K2Settings.externalFlipCalibrationYaw) // Ext
				 : radiansToDegrees(K2Settings.calibrationYaws.first)); // Default

			const double _current_yaw =
			(K2Settings.isExternalFlipEnabled
				 ? radiansToDegrees(EigenUtils::QuatToEulers( // Ext
					 interfacing::getVRWaistTrackerPose().second).y())
				 : _yaw); // Default

			// Compose flip
			const float _facing = (_current_yaw - _neutral_yaw);

			if ( //facing <= 25 && facing >= -25 || //if we use -180+180
				(_facing <= 25 && _facing >= 0 || _facing >= 345 && _facing <= 360)) //if we use 0+360
				base_flip = false;
			if ( //facing <= -155 && facing >= -205 || //if we use -180+180
				_facing >= 155 && _facing <= 205) //if we use 0+360
				base_flip = true;

			// Overwrite flip value depending on device & settings
			// index() check should've already been done by the app tho
			if (!K2Settings.isFlipEnabled || _device.index() == 1)base_flip = false;
			
			/*
			 * Feet trackers orientation - preparations
			 */

			for (auto& tracker : K2Settings.K2TrackersVector) {

				// Copy the orientation to the left foot tracker
				tracker.pose.orientation = _device.index() == 0

					// KinectBasis Device - grab L or R depending on flip : index0
					? (base_flip

						// If flip
						? std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device)->
						getJointOrientations()[ktvr::Joint_AnkleRight].inverse()

						// If no flip
						: std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device)->
						getJointOrientations()[ktvr::Joint_AnkleLeft]
						)

					// JointsBasis Device - select based on settings : index1
					: std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device)->
					getTrackedJoints()[tracker.selectedTrackedJointID].getJointOrientation();

				// Optionally overwrite the rotation with HMD orientation
				if (tracker.orientationTrackingOption == k2_FollowHMDRotation)
					tracker.pose.orientation = EigenUtils::EulersToQuat(
						Eigen::Vector3f(0, interfacing::plugins::plugins_getHMDOrientationYaw(), 0));

				// Optionally overwrite the rotation with NONE
				if (tracker.orientationTrackingOption == k2_DisableJointRotation)
					tracker.pose.orientation = Eigen::Quaternionf(1, 0, 0, 0);
			}

			/*
			 * Feet trackers orientation - preparations : No-Yaw mode
			 */

			// We've got no no-yaw mode in amethyst (✿◕‿◕✿)

			/*
			 * Feet trackers orientation - preparations : App / Software rotation
			 *   Note: This is only available for feet as for now
			 */

			if (_device.index() == 0 &&
				(K2Settings.K2TrackersVector[1].orientationTrackingOption == k2_SoftwareCalculatedRotation ||
					K2Settings.K2TrackersVector[2].orientationTrackingOption == k2_SoftwareCalculatedRotation))
			{
				const auto& _kinect = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device);

				if (_kinect->getDeviceCharacteristics() == ktvr::K2_Character_Full ||
					_kinect->getDeviceCharacteristics() == ktvr::K2_Character_Simple)
				{
					const auto _joints = _kinect->getJointPositions();
					const auto _joint_states = _kinect->getTrackingStates();

					// Placeholders for mathbased rots
					Eigen::Quaternionf calculatedLeftFootOrientation, calculatedRightFootOrientation;

					// Capture needed joints' positions
					Eigen::Vector3f up(0, 1, 0), forward(0, 0, 1), backward(0, 0, -1),
					                ankleLeftPose(
						                _joints[ktvr::Joint_AnkleLeft].x(),
						                _joints[ktvr::Joint_AnkleLeft].y(),
						                _joints[ktvr::Joint_AnkleLeft].z()),

					                ankleRightPose(
						                _joints[ktvr::Joint_AnkleRight].x(),
						                _joints[ktvr::Joint_AnkleRight].y(),
						                _joints[ktvr::Joint_AnkleRight].z()),

					                footLeftPose(
						                _joints[ktvr::Joint_FootLeft].x(),
						                _joints[ktvr::Joint_FootLeft].y(),
						                _joints[ktvr::Joint_FootLeft].z()),

					                footRightPose(
						                _joints[ktvr::Joint_FootRight].x(),
						                _joints[ktvr::Joint_FootRight].y(),
						                _joints[ktvr::Joint_FootRight].z()),

					                kneeLeftPose(
						                _joints[ktvr::Joint_KneeLeft].x(),
						                _joints[ktvr::Joint_KneeLeft].y(),
						                _joints[ktvr::Joint_KneeLeft].z()),

					                kneeRightPose(
						                _joints[ktvr::Joint_KneeRight].x(),
						                _joints[ktvr::Joint_KneeRight].y(),
						                _joints[ktvr::Joint_KneeRight].z());

					// Calculate euler yaw foot orientation, we'll need it later
					Eigen::Vector3f
						footLeftRawOrientation = EigenUtils::DirectionQuat(
							Eigen::Vector3f(ankleLeftPose.x(), 0.f, ankleLeftPose.z()),
							Eigen::Vector3f(footLeftPose.x(), 0.f, footLeftPose.z()),
							forward).toRotationMatrix().eulerAngles(0, 1, 2),

						footRightRawOrientation = EigenUtils::DirectionQuat(
							Eigen::Vector3f(ankleRightPose.x(), 0.f, ankleRightPose.z()),
							Eigen::Vector3f(footRightPose.x(), 0.f, footRightPose.z()),
							forward).toRotationMatrix().eulerAngles(0, 1, 2);

					// Flip the yaw around, without reversing it -> we need it basing to 0
					// (what an irony that we actually need to reverse it...)
					footLeftRawOrientation.y() *= -1.f;
					footLeftRawOrientation.y() += _PI;
					footRightRawOrientation.y() *= -1.f;
					footRightRawOrientation.y() += _PI;

					// Make the yaw less sensitive
					// Decided to go for radians for the read-ability
					// (Although my code is shit anyway, and there'll be none in the end)
					float lsFixedYaw[2] = {
						radiansToDegrees(footLeftRawOrientation.y()),
						radiansToDegrees(footRightRawOrientation.y())
					}; // L, R

					// Left
					if (lsFixedYaw[0] > 180.f && lsFixedYaw[0] < 360.f)
						lsFixedYaw[0] = 360.f - abs(lsFixedYaw[0] - 360.f) * .5f;
					else if (lsFixedYaw[0] < 180.f && lsFixedYaw[0] > 0.f)
						lsFixedYaw[0] *= .5f;

					// Right
					if (lsFixedYaw[1] > 180.f && lsFixedYaw[1] < 360.f)
						lsFixedYaw[1] = 360.f - abs(lsFixedYaw[1] - 360.f) * .5f;
					else if (lsFixedYaw[1] < 180.f && lsFixedYaw[1] > 0.f)
						lsFixedYaw[1] *= .5f;

					// Apply to the base
					footLeftRawOrientation.y() = degreesToRadians(lsFixedYaw[0]); // Back to the RAD format
					footRightRawOrientation.y() = degreesToRadians(lsFixedYaw[1]);

					// Construct a helpful offsetting quaternion from the stuff we got
					// It's made like Quat->Eulers->Quat because we may need to adjust some things on-to-go
					Eigen::Quaternionf
						leftFootYawOffsetQuaternion = EigenUtils::EulersToQuat(footLeftRawOrientation),
						// There is no X and Z anyway
						rightFootYawOffsetQuaternion = EigenUtils::EulersToQuat(footRightRawOrientation);

					// Smooth a bit with a slerp
					yawFilteringQuaternion[0] = yawFilteringQuaternion[0].slerp(.25f, leftFootYawOffsetQuaternion);
					yawFilteringQuaternion[1] = yawFilteringQuaternion[1].slerp(.25f, rightFootYawOffsetQuaternion);

					// Apply to the base
					leftFootYawOffsetQuaternion = yawFilteringQuaternion[0];
					rightFootYawOffsetQuaternion = yawFilteringQuaternion[1];

					// Calculate the knee-ankle orientation, aka "Tibia"
					// We aren't disabling look-thorough yaw, since it'll be 0
					Eigen::Quaternionf
						knee_ankleLeftOrientationQuaternion = EigenUtils::DirectionQuat(
							kneeLeftPose, ankleLeftPose, forward),
						knee_ankleRightOrientationQuaternion = EigenUtils::DirectionQuat(
							kneeRightPose, ankleRightPose, forward);

					// The tuning quat
					auto
						tuneQuaternion_first = Eigen::Quaternionf(1, 0, 0, 0);

					// Now adjust some values like playspace yaw and pitch, additional rotations
					// -> they're facing purely down and Z / Y are flipped
					tuneQuaternion_first =
						EigenUtils::EulersToQuat(
							Eigen::Vector3f(
								_PI / 5.f,
								0.f,
								0.f
							));

					// Apply the fine-tuning to global variable
					knee_ankleLeftOrientationQuaternion = tuneQuaternion_first * knee_ankleLeftOrientationQuaternion;
					knee_ankleRightOrientationQuaternion = tuneQuaternion_first * knee_ankleRightOrientationQuaternion;

					// Grab original orientations and make them euler angles
					Eigen::Vector3f left_knee_ori_full = EigenUtils::QuatToEulers(knee_ankleLeftOrientationQuaternion);
					Eigen::Vector3f right_knee_ori_full =
						EigenUtils::QuatToEulers(knee_ankleRightOrientationQuaternion);

					// Try to fix yaw and roll mismatch, caused by XYZ XZY mismatch
					knee_ankleLeftOrientationQuaternion = EigenUtils::EulersToQuat(
						Eigen::Vector3f(
							left_knee_ori_full.x() - _PI / 1.6f,
							0.0, // left_knee_ori_full.z(), // actually 0.0 but okay
							-left_knee_ori_full.y()));

					knee_ankleRightOrientationQuaternion = EigenUtils::EulersToQuat(
						Eigen::Vector3f(
							right_knee_ori_full.x() - _PI / 1.6f,
							0.0, // right_knee_ori_full.z(), // actually 0.0 but okay
							-right_knee_ori_full.y()));

					if (_joint_states[ktvr::Joint_AnkleLeft] == ktvr::ITrackedJointState::State_Tracked)
						// All the rotations
						calculatedLeftFootOrientation = leftFootYawOffsetQuaternion *
							knee_ankleLeftOrientationQuaternion;
					else
						// Without the foot's yaw
						calculatedLeftFootOrientation = knee_ankleLeftOrientationQuaternion;

					if (_joint_states[ktvr::Joint_AnkleRight] == ktvr::ITrackedJointState::State_Tracked)
						// All the rotations
						calculatedRightFootOrientation = rightFootYawOffsetQuaternion *
							knee_ankleRightOrientationQuaternion;
					else
						// Without the foot's yaw
						calculatedRightFootOrientation = knee_ankleRightOrientationQuaternion;

					// The tuning quat
					auto
						leftFootFineTuneQuaternion = Eigen::Quaternionf(1, 0, 0, 0),
						rightFootFineTuneQuaternion = Eigen::Quaternionf(1, 0, 0, 0);

					// Now adjust some values like playspace yaw and pitch, additional rotations

					leftFootFineTuneQuaternion =
						EigenUtils::EulersToQuat( // Lift trackers up a bit
							Eigen::Vector3f(
								2.8623399733f, // this one's in radians alr
								0.f, //glm::radians(KinectSettings::calibration_trackers_yaw),
								0.f
							));

					rightFootFineTuneQuaternion =
						EigenUtils::EulersToQuat( // Lift trackers up a bit
							Eigen::Vector3f(
								2.8623399733f, // this one's in radians alr
								0.f, //glm::radians(KinectSettings::calibration_trackers_yaw),
								0.f
							));

					// Apply the fine-tuning to global variable
					calculatedLeftFootOrientation = leftFootFineTuneQuaternion * calculatedLeftFootOrientation;
					calculatedRightFootOrientation = rightFootFineTuneQuaternion * calculatedRightFootOrientation;

					// Push to global
					if (_kinect->isSkeletonTracked())
					{
						// Left Foot
						if (K2Settings.K2TrackersVector[1].orientationTrackingOption == k2_SoftwareCalculatedRotation)
						{
							K2Settings.K2TrackersVector[1].pose.orientation = base_flip
								// If flip
								? ktvr::quaternion_normal(calculatedRightFootOrientation).inverse()
								// If no flip
								: ktvr::quaternion_normal(calculatedLeftFootOrientation);

							// Apply fixes

							// Grab original orientations and make them euler angles
							Eigen::Vector3f left_ori_vector = EigenUtils::QuatToEulers(
								K2Settings.K2TrackersVector[1].pose.orientation);

							// Kind of a solution for flipping at too big X.
							// Found out during testing,
							// no other known mathematical reason (maybe except gimbal lock)

							/****************************************************/

							if (left_ori_vector.y() <= 0.f
								&& left_ori_vector.y() >= -1.f

								&& left_ori_vector.z() <= -1.f
								&& left_ori_vector.z() >= -_PI)

								left_ori_vector.y() += -_PI;

							/****************************************************/

							// Apply to the base
							K2Settings.K2TrackersVector[1].pose.orientation =
								EigenUtils::EulersToQuat(left_ori_vector);
						}

						// Left Foot
						if (K2Settings.K2TrackersVector[2].orientationTrackingOption == k2_SoftwareCalculatedRotation)
						{
							K2Settings.K2TrackersVector[2].pose.orientation = base_flip
								// If flip
								? ktvr::quaternion_normal(calculatedLeftFootOrientation).inverse()
								// If no flip
								: ktvr::quaternion_normal(calculatedRightFootOrientation);

							// Apply fixes

							// Grab original orientations and make them euler angles
							Eigen::Vector3f right_ori_vector = EigenUtils::QuatToEulers(
								K2Settings.K2TrackersVector[2].pose.orientation);

							// Kind of a solution for flipping at too big X.
							// Found out during testing,
							// no other known mathematical reason (maybe except gimbal lock)

							/****************************************************/

							if (right_ori_vector.y() <= 0.f
								&& right_ori_vector.y() >= -1.f

								&& right_ori_vector.z() <= -1.f
								&& right_ori_vector.z() >= -_PI)

								right_ori_vector.y() += -_PI;

							/****************************************************/

							// Apply to the base
							K2Settings.K2TrackersVector[2].pose.orientation =
								EigenUtils::EulersToQuat(right_ori_vector);
						}
					}
				}
			}

			/*
			 * Trackers orientation - Calibration-related fixes (kinectbasis-only)
			 */

			if (_device.index() == 0)
			{
				// Construct an offset quaternion with the calibration yaw
				Eigen::Quaternionf
					yawOffsetQuaternion =
						EigenUtils::EulersToQuat(Eigen::Vector3f(0.f, K2Settings.calibrationYaws.first, 0.f)),

					yawFlipQuaternion =
						EigenUtils::EulersToQuat(Eigen::Vector3f(0.f, _PI, 0.f)); // Just turn around the yaw

				/*
				 * Apply additional things here that have to be in eulers
				 * AND be the last to be applied.
				 * For example there are: flip yaw, calibration pitch...
				 * AND are connected with joints' orientations as they are,
				 * not with the headori ones.
				 */

				if (base_flip)
				{
					// Optionally disable pitch in flip mode,
					// if you want not to, just set it to true
					bool pitchOn = true;
					float pitchOffOffset = 0.0; // May be applied when pitch is off
					std::vector pitchShift(K2Settings.K2TrackersVector.size(), 0.f);

					for (uint32_t index = 0; index < K2Settings.K2TrackersVector.size(); index++) {
						if (K2Settings.K2TrackersVector[index].orientationTrackingOption == k2_DeviceInferredRotation ||
							K2Settings.K2TrackersVector[index].orientationTrackingOption == k2_SoftwareCalculatedRotation)
							pitchShift.at(index) = _PI / 6.f; // Normal offset
					}

					for (uint32_t index = 0; index < K2Settings.K2TrackersVector.size(); index++) {
						if (K2Settings.K2TrackersVector[index].orientationTrackingOption != k2_FollowHMDRotation)
						{
							// Remove the pitch angle
							// Grab original orientations and make them euler angles
							Eigen::Vector3f left_ori_with_yaw =
								EigenUtils::QuatToEulers(K2Settings.K2TrackersVector[index].pose.orientation);

							// Remove pitch from eulers and apply to the parent
							K2Settings.K2TrackersVector[index].pose.orientation = EigenUtils::EulersToQuat(
								Eigen::Vector3f(
									pitchOn ? left_ori_with_yaw.x() - pitchShift[index] : pitchOffOffset, // Disable the pitch
									(base_flip ? -1.f : 1.f) * left_ori_with_yaw.y(),
									-left_ori_with_yaw.z()));

							// Apply the turn-around flip quaternion
							K2Settings.K2TrackersVector[index].pose.orientation =
								yawFlipQuaternion * K2Settings.K2TrackersVector[index].pose.orientation;

							// It'll make the tracker face the kinect
							K2Settings.K2TrackersVector[index].pose.orientation =
								yawOffsetQuaternion * K2Settings.K2TrackersVector[index].pose.orientation;
						}
					}
				}
				else
				{
					// It'll make the tracker face the kinect
					for (auto& tracker : K2Settings.K2TrackersVector)
						if (tracker.orientationTrackingOption != k2_FollowHMDRotation)
							tracker.pose.orientation = yawOffsetQuaternion * tracker.pose.orientation;
				}
			}

			/*****************************************************************************************/
			// Fix waist orientation when following hmd after a standing pose reset
			/*****************************************************************************************/

			// Loop over all trackers
			for (auto& tracker : K2Settings.K2TrackersVector)
				if (tracker.orientationTrackingOption == k2_FollowHMDRotation)
					// Offset to fit the playspace
					tracker.pose.orientation = EigenUtils::EulersToQuat(
						Eigen::Vector3f(0., -interfacing::vrPlayspaceOrientation, 0.)) * tracker.pose.orientation;

			/*****************************************************************************************/
			// Push RAW poses to trackers
			/*****************************************************************************************/

			if (_device.index() == 0)
			{
				const auto& _kinect =
					std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device);

				for (auto& tracker : K2Settings.K2TrackersVector)
					tracker.pose.position = _kinect->getJointPositions()[
						interfacing::overrides::getFlippedJointType(
							ITrackerType_Joint[tracker.tracker], base_flip)];
			}
			else if (_device.index() == 1)
			{
				const auto& _joints =
					std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device);

				for (auto& tracker : K2Settings.K2TrackersVector)
					tracker.pose.position =
					_joints->getTrackedJoints().at(tracker.selectedTrackedJointID).getJointPosition();
			}
		}


		/*
		 * Calculate ALL poses for the override (second) device here
		 */

		// Override
		{
			// If we even HAVE an override
			if (K2Settings.overrideDeviceID >= 0)
			{
				/* Strategy:
				 *   overwrite base device's poses, optionally apply flip
				 *	 note that unlike in legacy versions, flip isn't anymore
				 *	 applied on pose pushes; this will allow us to apply
				 *	 two (or even more) independent flips, after the base
				 */

				// Get the current override device
				const auto& _device = TrackingDevices::getCurrentOverrideDevice();

				// Compose the yaw neutral and current
				const double _neutral_yaw =
				(K2Settings.isExternalFlipEnabled
					 ? radiansToDegrees(K2Settings.externalFlipCalibrationYaw) // Ext
					 : radiansToDegrees(K2Settings.calibrationYaws.second)); // Default

				const double _current_yaw =
				(K2Settings.isExternalFlipEnabled
					 ? radiansToDegrees(EigenUtils::QuatToEulers( // Ext
						 interfacing::getVRWaistTrackerPose().second).y())
					 : _yaw); // Default

				// Compose flip
				const float _facing = (_current_yaw - _neutral_yaw);

				if ( //facing <= 25 && facing >= -25 || //if we use -180+180
					(_facing <= 25 && _facing >= 0 || _facing >= 345 && _facing <= 360)) //if we use 0+360
					override_flip = false;
				if ( //facing <= -155 && facing >= -205 || //if we use -180+180
					_facing >= 155 && _facing <= 205) //if we use 0+360
					override_flip = true;

				// Overwrite flip value depending on device & settings
				// index() check should've already been done by the app tho
				if (!K2Settings.isFlipEnabled || _device.index() == 1)override_flip = false;

				/*
				 * Trackers orientation - preparations
				 */

				 // Check if we need to apply it, skip if it's set to either hmd or none
				 for (auto& tracker : K2Settings.K2TrackersVector)
					if (tracker.isRotationOverridden &&
						(tracker.orientationTrackingOption == k2_DeviceInferredRotation ||
							tracker.orientationTrackingOption == k2_SoftwareCalculatedRotation))
					{
						// Copy the orientation to the left foot tracker
						tracker.pose.orientation = _device.index() == 0

							// KinectBasis Device - grab L or R depending on flip : index1
							? (override_flip

								// If flip
								? std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device)->
								getJointOrientations()[interfacing::overrides::getFlippedJointType(
									static_cast<ktvr::ITrackedJointType>(
										TrackingDevices::devices_override_joint_id(
											tracker.rotationOverrideJointID)))].inverse()

								// If no flip
								: std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device)->
								getJointOrientations()[
									TrackingDevices::devices_override_joint_id(
										tracker.rotationOverrideJointID)]
								)

							// JointsBasis Device - select based on settings : index1
										: std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device)->
										getTrackedJoints()[
											TrackingDevices::devices_override_joint_id(
												tracker.rotationOverrideJointID)].getJointOrientation();
					}
				
				/*
				 * Feet trackers orientation - preparations : No-Yaw mode
				 */

				// We've got no no-yaw mode in amethyst (✿◕‿◕✿)

				/*
				 * Feet trackers orientation - preparations : App / Software rotation
				 */

				// We've got no math-based for override devices (✿◕‿◕✿)

				/*
				 * Feet trackers orientation - Calibration-related fixes (kinectbasis-only)
				 */

				if (_device.index() == 0)
				{
					// Construct an offset quaternion with the calibration yaw
					Eigen::Quaternionf
						yawOffsetQuaternion =
							EigenUtils::EulersToQuat(Eigen::Vector3f(0.f, K2Settings.calibrationYaws.second, 0.f)),

						yawFlipQuaternion =
							EigenUtils::EulersToQuat(Eigen::Vector3f(0.f, _PI, 0.f)); // Just turn around the yaw

					/*
					 * Apply additional things here that have to be in eulers
					 * AND be the last to be applied.
					 * For example there are: flip yaw, calibration pitch...
					 * AND are connected with joints' orientations as they are,
					 * not with the headori ones.
					 */

					if (override_flip)
					{
						// Optionally disable pitch in flip mode,
						// if you want not to, just set it to true
						bool pitchOn = true;
						float pitchOffOffset = 0.0, // May be applied when pitch is off
						      pitchShift_L = 0.f, pitchShift_R = 0.f,
						      pitchShift_EL = 0.f, pitchShift_ER = 0.f,
						      pitchShift_KL = 0.f, pitchShift_KR = 0.f;

						for (auto& tracker : K2Settings.K2TrackersVector) {
							if (tracker.orientationTrackingOption == k2_DeviceInferredRotation ||
								tracker.orientationTrackingOption == k2_SoftwareCalculatedRotation)
								pitchShift_L = _PI / 8.f; // Normal offset
							
							if (tracker.orientationTrackingOption != k2_FollowHMDRotation 
								&& tracker.isRotationOverridden)
							{
								// Remove the pitch angle
								// Grab original orientations and make them euler angles
								Eigen::Vector3f left_ori_with_yaw =
									EigenUtils::QuatToEulers(tracker.pose.orientation);

								// Remove pitch from eulers and apply to the parent
								tracker.pose.orientation = EigenUtils::EulersToQuat(
									Eigen::Vector3f(
										pitchOn ? left_ori_with_yaw.x() - pitchShift_L : pitchOffOffset,
										// Disable the pitch
										(override_flip ? -1.f : 1.f) * left_ori_with_yaw.y(),
										-left_ori_with_yaw.z()));

								// Apply the turn-around flip quaternion
								tracker.pose.orientation = yawFlipQuaternion * tracker.pose.orientation;

								// It'll make the tracker face the kinect
								tracker.pose.orientation = yawOffsetQuaternion * tracker.pose.orientation;
							}
						}
					}
					else
						// It'll make the tracker face the kinect
						for (auto& tracker : K2Settings.K2TrackersVector)
							if (tracker.orientationTrackingOption != k2_FollowHMDRotation &&
								tracker.isRotationOverridden)
								tracker.pose.orientation = yawOffsetQuaternion * tracker.pose.orientation;
				}

				/*****************************************************************************************/
				// Push RAW poses to trackers
				/*****************************************************************************************/

				if (_device.index() == 0)
				{
					const auto& _kinect =
						std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device);

					for (auto& tracker : K2Settings.K2TrackersVector)
						if (tracker.isPositionOverridden)
							tracker.pose.position = _kinect->getJointPositions()[
								interfacing::overrides::getFlippedJointType(
									static_cast<ktvr::ITrackedJointType>(
										TrackingDevices::devices_override_joint_id(
											tracker.positionOverrideJointID)), override_flip)];
				}
				else if (_device.index() == 1)
				{
					const auto& _joints =
						std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device);

					for (auto& tracker : K2Settings.K2TrackersVector)
						if (tracker.isPositionOverridden) tracker.pose.position =
							_joints->getTrackedJoints().at(tracker.positionOverrideJointID).getJointPosition();
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

		LOG(INFO) << "[K2Main] Starting the main app loop now...";

		// Errors' case
		bool server_giveUp = false;
		int server_tries = 0;

		while (!server_giveUp)
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

					K2UpdateTrackingDevices(); // Update actual tracking
					K2UpdateAppTrackers(); // Track joints from raw data

					K2UpdateServerTrackers(); // Send it to the server

					// Wait until certain loop time has passed
					if (auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
							std::chrono::high_resolution_clock::now() - loop_start_time).count();
						duration <= 12222222.f) // If we were too fast, sleep peacefully @80hz
						std::this_thread::sleep_for(std::chrono::nanoseconds(12222222 - duration));
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
					TrackingDevices::devices_check_base_ids(K2Settings.trackingDeviceID);
					TrackingDevices::devices_check_override_ids(K2Settings.trackingDeviceID);
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

					exit(-13); // -13 is the code for giving up then, I guess
					// user will be prompted to reset the config (opt)
				}
			}
		}
	}
}
