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
			winrt::Microsoft::UI::Xaml::ElementSoundPlayer::Play(
				winrt::Microsoft::UI::Xaml::ElementSoundKind::Invoke);
		}

		// Update the Flip Toggle : toggle
		// Only if the state has changed from 1 to 0: button was clicked
		if (!evr_input.trackerFlipToggleData().bState && bak_flip_toggle_state)
		{
			LOG(INFO) << "[Input Actions] Input: Flip toggled.";

			// Also validate the result
			if (auto const& trackingDevice = TrackingDevices::getCurrentDevice(); trackingDevice.index() == 0)
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
			winrt::Microsoft::UI::Xaml::ElementSoundPlayer::Play(
				winrt::Microsoft::UI::Xaml::ElementSoundKind::Invoke);

			if (shared::settings::flipCheckBox.get())
				shared::settings::flipCheckBox.get()->IsChecked(K2Settings.isFlipEnabled);
		}

		// Update the Calibration:Confirm : one-time switch
		// Only one-way switch this time
		if (evr_input.confirmAndSaveActionData().bState)
			calibration_confirm = true;

		/*LOG_IF(evr_input.fineTuneActionData().bState, INFO) << "FineTune ON!";
		LOG_IF(evr_input.fineTuneActionData().bActive, INFO) << "FineTune Action ACTIVE!";*/

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
		switch (auto const& device = TrackingDevices::
			getCurrentDevice(); device.index())
		{
		case 0:
			{
				auto const& pDevice = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(device);
				pDevice->update(); // Update the device
				interfacing::kinectHeadPosition.first = pDevice->getJointPositions()[ktvr::Joint_Head];
				interfacing::kinectWaistPosition.first = pDevice->getJointPositions()[ktvr::Joint_SpineWaist];
			}
			break;
		case 1:
			{
				auto const& pDevice = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(device);
				pDevice->update(); // Update the device
				if (K2Settings.selectedTrackedJointID[0] < pDevice->getTrackedJoints().size())
					interfacing::kinectWaistPosition.first = pDevice->getTrackedJoints().at(
						K2Settings.selectedTrackedJointID[0]).getJointPosition();
			}
			break;
		}

		/* Update the override device here (optionally) */
		if (auto const& device_pair = TrackingDevices::
			getCurrentOverrideDevice_Safe(); device_pair.first)
			switch (device_pair.second.index())
			{
			case 0:
				{
					auto const& pDevice = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(device_pair.second);
					interfacing::kinectHeadPosition.second = pDevice->getJointPositions()[ktvr::Joint_Head];
					interfacing::kinectWaistPosition.second = pDevice->getJointPositions()[ktvr::Joint_SpineWaist];
				}
				break;
			case 1:
				{
					auto const& pDevice = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(device_pair.second);
					pDevice->update(); // Update the device
					if (K2Settings.selectedTrackedJointID[0] < pDevice->getTrackedJoints().size())
						interfacing::kinectWaistPosition.second = pDevice->getTrackedJoints().at(
							K2Settings.selectedTrackedJointID[0]).getJointPosition();
				}
				break;
			}
	}

	inline int p_loops = 0; // Loops passed since last status update
	inline bool initialized_bak = false; // Backup initialized? value
	inline bool flip_base = false, flip_override = false; // If flip's active
	inline void K2UpdateServerTrackers()
	{
		// Update only if we're connected
		if (interfacing::K2AppTrackersSpawned &&
			!interfacing::serverDriverFailure)
		{
			// If tracking is frozen, only refresh
			if (interfacing::isTrackingFrozen)
			{
				// Waist Tracker (NF_0)
				if (K2Settings.isJointEnabled[0])
					ktvr::refresh_tracker_pose<false>(interfacing::K2TrackersVector.at(0).id);

				// Left Foot tracker (NF_1)
				// If not overridden by second device
				if (!K2Settings.isPositionOverriddenJoint[1] || K2Settings.overrideDeviceID < 0)
				{
					if (K2Settings.isJointEnabled[flip_base ? 2 : 1])
						ktvr::refresh_tracker_pose<false>(interfacing::K2TrackersVector.at(flip_base ? 2 : 1).id);
				}
				// If overriden by second device
				else
				{
					if (K2Settings.isJointEnabled[flip_override ? 2 : 1])
						ktvr::refresh_tracker_pose<false>(interfacing::K2TrackersVector.at(flip_override ? 2 : 1).id);
				}

				// Right Foot tracker (NF_2)
				// If not overridden by second device
				if (!K2Settings.isPositionOverriddenJoint[2] || K2Settings.overrideDeviceID < 0)
				{
					if (K2Settings.isJointEnabled[flip_base ? 1 : 2])
						ktvr::refresh_tracker_pose<false>(interfacing::K2TrackersVector.at(flip_base ? 1 : 2).id);
				}
				// If overriden by second device
				else
				{
					if (K2Settings.isJointEnabled[flip_override ? 1 : 2])
						ktvr::refresh_tracker_pose<false>(interfacing::K2TrackersVector.at(flip_override ? 1 : 2).id);
				}
			}
			// If the tracing's actually running
			else
			{
				// Update orientation filters
				interfacing::K2TrackersVector.at(0).updateOrientationFilters();
				interfacing::K2TrackersVector.at(1).updateOrientationFilters();
				interfacing::K2TrackersVector.at(2).updateOrientationFilters();

				// Update position filters
				interfacing::K2TrackersVector.at(0).updatePositionFilters();
				interfacing::K2TrackersVector.at(1).updatePositionFilters();
				interfacing::K2TrackersVector.at(2).updatePositionFilters();

				// Update pose w/ filtering, options and calibration
				// Note: only position gets calibrated INSIDE trackers

				// Waist tracker (NF_0)
				if (K2Settings.isJointEnabled[0])
				{
					// If not overridden by second device
					if (!K2Settings.isPositionOverriddenJoint[0] || K2Settings.overrideDeviceID < 0)
					{
						if (K2Settings.isMatrixCalibrated.first)
							ktvr::update_tracker_pose<false>(
								interfacing::K2TrackersVector.at(0).getTrackerBase
								(
									K2Settings.calibrationRotationMatrices.first.cast<float>(),
									K2Settings.calibrationTranslationVectors.first.cast<float>(),
									K2Settings.calibrationOrigins.first.cast<float>(),
									K2Settings.positionFilterOption,
									K2Settings.rotationFilterOption
								));
						else
							ktvr::update_tracker_pose<false>(
								interfacing::K2TrackersVector.at(0).getTrackerBase
								(
									K2Settings.positionFilterOption,
									K2Settings.rotationFilterOption
								));
					}
					// If overriden by second device
					else
					{
						if (K2Settings.isMatrixCalibrated.second)
							ktvr::update_tracker_pose<false>(
								interfacing::K2TrackersVector.at(0).getTrackerBase
								(
									K2Settings.calibrationRotationMatrices.second.cast<float>(),
									K2Settings.calibrationTranslationVectors.second.cast<float>(),
									K2Settings.calibrationOrigins.second.cast<float>(),
									K2Settings.positionFilterOption,
									K2Settings.rotationFilterOption
								));
						else
							ktvr::update_tracker_pose<false>(
								interfacing::K2TrackersVector.at(0).getTrackerBase
								(
									K2Settings.positionFilterOption,
									K2Settings.rotationFilterOption
								));
					}
				}

				// Left Foot tracker (NF_1)
				// If not overridden by second device
				if (!K2Settings.isPositionOverriddenJoint[1] || K2Settings.overrideDeviceID < 0)
				{
					if (K2Settings.isJointEnabled[flip_base ? 2 : 1])
					{
						if (K2Settings.isMatrixCalibrated.first)
							ktvr::update_tracker_pose<false>(
								interfacing::K2TrackersVector.at(flip_base ? 2 : 1).getTrackerBase
								(
									K2Settings.calibrationRotationMatrices.first.cast<float>(),
									K2Settings.calibrationTranslationVectors.first.cast<float>(),
									K2Settings.calibrationOrigins.first.cast<float>(),
									K2Settings.positionFilterOption,
									K2Settings.rotationFilterOption
								));
						else
							ktvr::update_tracker_pose<false>(
								interfacing::K2TrackersVector.at(flip_base ? 2 : 1).getTrackerBase
								(
									K2Settings.positionFilterOption,
									K2Settings.rotationFilterOption
								));
					}
				}
				// If overriden by second device
				else
				{
					if (K2Settings.isJointEnabled[flip_override ? 2 : 1])
					{
						if (K2Settings.isMatrixCalibrated.second)
							ktvr::update_tracker_pose<false>(
								interfacing::K2TrackersVector.at(flip_override ? 2 : 1).getTrackerBase
								(
									K2Settings.calibrationRotationMatrices.second.cast<float>(),
									K2Settings.calibrationTranslationVectors.second.cast<float>(),
									K2Settings.calibrationOrigins.second.cast<float>(),
									K2Settings.positionFilterOption,
									K2Settings.rotationFilterOption
								));
						else
							ktvr::update_tracker_pose<false>(
								interfacing::K2TrackersVector.at(flip_override ? 2 : 1).getTrackerBase
								(
									K2Settings.positionFilterOption,
									K2Settings.rotationFilterOption
								));
					}
				}

				// Right Foot tracker (NF_2)
				// If not overridden by second device
				if (!K2Settings.isPositionOverriddenJoint[2] || K2Settings.overrideDeviceID < 0)
				{
					if (K2Settings.isJointEnabled[flip_base ? 1 : 2])
					{
						if (K2Settings.isMatrixCalibrated.first)
							ktvr::update_tracker_pose<false>(
								interfacing::K2TrackersVector.at(flip_base ? 1 : 2).getTrackerBase
								(
									K2Settings.calibrationRotationMatrices.first.cast<float>(),
									K2Settings.calibrationTranslationVectors.first.cast<float>(),
									K2Settings.calibrationOrigins.first.cast<float>(),
									K2Settings.positionFilterOption,
									K2Settings.rotationFilterOption
								));
						else
							ktvr::update_tracker_pose<false>(
								interfacing::K2TrackersVector.at(flip_base ? 1 : 2).getTrackerBase
								(
									K2Settings.positionFilterOption,
									K2Settings.rotationFilterOption
								));
					}
				}
				// If overriden by second device
				else
				{
					if (K2Settings.isJointEnabled[flip_override ? 1 : 2])
					{
						if (K2Settings.isMatrixCalibrated.second)
							ktvr::update_tracker_pose<false>(
								interfacing::K2TrackersVector.at(flip_override ? 1 : 2).getTrackerBase
								(
									K2Settings.calibrationRotationMatrices.second.cast<float>(),
									K2Settings.calibrationTranslationVectors.second.cast<float>(),
									K2Settings.calibrationOrigins.second.cast<float>(),
									K2Settings.positionFilterOption,
									K2Settings.rotationFilterOption
								));
						else
							ktvr::update_tracker_pose<false>(
								interfacing::K2TrackersVector.at(flip_override ? 1 : 2).getTrackerBase
								(
									K2Settings.positionFilterOption,
									K2Settings.rotationFilterOption
								));
					}
				}
			}

			// Update status 1/1000 loops / ~8s
			// or right after any change
			for (int i = 0; i < 3; i++)
			{
				// try 3 times
				if (p_loops >= 1000 ||
					(initialized_bak != interfacing::K2AppTrackersInitialized))
				{
					// Update status in server

					// Waist Tracker (NF_0)
					if (K2Settings.isJointEnabled[0])
						ktvr::set_tracker_state<false>(interfacing::K2TrackersVector.at(0).id,
						                               K2Settings.isJointTurnedOn[0] &&
						                               interfacing::K2AppTrackersInitialized);

					// Left Foot tracker (NF_1)
					// If not overridden by second device
					if (!K2Settings.isPositionOverriddenJoint[1] || K2Settings.overrideDeviceID < 0)
					{
						if (K2Settings.isJointEnabled[flip_base ? 2 : 1])
							ktvr::set_tracker_state<false>(interfacing::K2TrackersVector.at(1).id,
							                               K2Settings.isJointTurnedOn[flip_base ? 2 : 1] &&
							                               interfacing::K2AppTrackersInitialized);
					}
					// If overriden by second device
					else
					{
						if (K2Settings.isJointEnabled[flip_override ? 2 : 1])
							ktvr::set_tracker_state<false>(interfacing::K2TrackersVector.at(1).id,
							                               K2Settings.isJointTurnedOn[flip_override ? 2 : 1] &&
							                               interfacing::K2AppTrackersInitialized);
					}

					// Right Foot tracker (NF_2)
					// If not overridden by second device
					if (!K2Settings.isPositionOverriddenJoint[2] || K2Settings.overrideDeviceID < 0)
					{
						if (K2Settings.isJointEnabled[flip_base ? 1 : 2])
							ktvr::set_tracker_state<false>(interfacing::K2TrackersVector.at(2).id,
							                               K2Settings.isJointTurnedOn[flip_base ? 1 : 2] &&
							                               interfacing::K2AppTrackersInitialized);
					}
					// If overriden by second device
					else
					{
						if (K2Settings.isJointEnabled[flip_override ? 1 : 2])
							ktvr::set_tracker_state<false>(interfacing::K2TrackersVector.at(2).id,
							                               K2Settings.isJointTurnedOn[flip_override ? 1 : 2] &&
							                               interfacing::K2AppTrackersInitialized);
					}
					
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
		{Eigen::Quaternionf(1, 0, 0, 0)}; // L, R

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
			auto const& _device = TrackingDevices::getCurrentDevice();

			// Compose flip
			const float _facing = _yaw - radiansToDegrees(K2Settings.calibrationYaws.first);
			bool _flip = false; // Assume non flipped

			if ( //facing <= 25 && facing >= -25 || //if we use -180+180
				(_facing <= 25 && _facing >= 0 || _facing >= 345 && _facing <= 360)) //if we use 0+360
				_flip = false;
			if ( //facing <= -155 && facing >= -205 || //if we use -180+180
				_facing >= 155 && _facing <= 205) //if we use 0+360
				_flip = true;

			// Overwrite flip value depending on device & settings
			// index() check should've already been done by the app tho
			if (!K2Settings.isFlipEnabled || _device.index() == 1)_flip = false;

			// Copy _flip to outer scope
			flip_base = _flip;

			/*
			 * Waist tracker orientation - preparations
			 */

			// Copy the orientation to the waist tracker
			interfacing::K2TrackersVector.at(0).pose.orientation =
				_device.index() == 0

					// KinectBasis Device - just grab the correct joint : index0
					? std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device)->
					getJointOrientations()[ktvr::Joint_SpineWaist]

					// JointsBasis Device - select based on settings : index1
					: std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device)->
					getTrackedJoints()[K2Settings.selectedTrackedJointID[0]].getJointOrientation();

			// Optionally overwrite the rotation with HMD orientation
			if (K2Settings.jointRotationTrackingOption[0] == k2_FollowHMDRotation)
				interfacing::K2TrackersVector.at(0).pose.orientation = EigenUtils::EulersToQuat(
					Eigen::Vector3f(0, interfacing::plugins::plugins_getHMDOrientationYaw(), 0));

			// Optionally overwrite the rotation with NONE
			if (K2Settings.jointRotationTrackingOption[0] == k2_DisableJointRotation)
				interfacing::K2TrackersVector.at(0).pose.orientation = Eigen::Quaternionf(1, 0, 0, 0);

			/*
			 * Feet trackers orientation - preparations : Left Foot
			 */

			// Copy the orientation to the left foot tracker
			interfacing::K2TrackersVector.at(1).pose.orientation =
				_device.index() == 0

					// KinectBasis Device - grab L or R depending on flip : index0
					? (_flip

						   // If flip
						   ? std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device)->
						   getJointOrientations()[ktvr::Joint_AnkleRight].inverse()

						   // If no flip
						   : std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device)->
						   getJointOrientations()[ktvr::Joint_AnkleLeft]
					)

					// JointsBasis Device - select based on settings : index1
					: std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device)->
					getTrackedJoints()[K2Settings.selectedTrackedJointID[1]].getJointOrientation();

			// Optionally overwrite the rotation with HMD orientation
			if (K2Settings.jointRotationTrackingOption[1] == k2_FollowHMDRotation)
				interfacing::K2TrackersVector.at(1).pose.orientation = EigenUtils::EulersToQuat(
					Eigen::Vector3f(0, interfacing::plugins::plugins_getHMDOrientationYaw(), 0));

			// Optionally overwrite the rotation with NONE
			if (K2Settings.jointRotationTrackingOption[1] == k2_DisableJointRotation)
				interfacing::K2TrackersVector.at(1).pose.orientation = Eigen::Quaternionf(1, 0, 0, 0);

			/*
			 * Feet trackers orientation - preparations : Right Foot
			 */

			// Copy the orientation to the left foot tracker
			interfacing::K2TrackersVector.at(2).pose.orientation =
				_device.index() == 0

					// KinectBasis Device - grab L or R depending on flip : index0
					? (_flip

						   // If flip
						   ? std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device)->
						   getJointOrientations()[ktvr::Joint_AnkleLeft].inverse()

						   // If no flip
						   : std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device)->
						   getJointOrientations()[ktvr::Joint_AnkleRight]
					)

					// JointsBasis Device - select based on settings : index1
					: std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device)->
					getTrackedJoints()[K2Settings.selectedTrackedJointID[2]].getJointOrientation();

			// Optionally overwrite the rotation with HMD orientation
			if (K2Settings.jointRotationTrackingOption[2] == k2_FollowHMDRotation)
				interfacing::K2TrackersVector.at(2).pose.orientation = EigenUtils::EulersToQuat(
					Eigen::Vector3f(0, interfacing::plugins::plugins_getHMDOrientationYaw(), 0));

			// Optionally overwrite the rotation with NONE
			if (K2Settings.jointRotationTrackingOption[2] == k2_DisableJointRotation)
				interfacing::K2TrackersVector.at(2).pose.orientation = Eigen::Quaternionf(1, 0, 0, 0);

			/*
			 * Feet trackers orientation - preparations : No-Yaw mode
			 */

			// We've got no no-yaw mode in amethyst (✿◕‿◕✿)

			/*
			 * Feet trackers orientation - preparations : App / Software rotation
			 */

			if (_device.index() == 0 &&
				(K2Settings.jointRotationTrackingOption[1] == k2_SoftwareCalculatedRotation ||
					K2Settings.jointRotationTrackingOption[2] == k2_SoftwareCalculatedRotation))
			{
				auto const& _kinect = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device);

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
					Eigen::Quaternionf
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
					Eigen::Quaternionf
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
						if (K2Settings.jointRotationTrackingOption[1] == k2_SoftwareCalculatedRotation)
						{
							interfacing::K2TrackersVector.at(1).pose.orientation = _flip
								// If flip
								? ktvr::quaternion_normal(calculatedRightFootOrientation).inverse()
								// If no flip
								: ktvr::quaternion_normal(calculatedLeftFootOrientation);

							// Apply fixes

							// Grab original orientations and make them euler angles
							Eigen::Vector3f left_ori_vector = EigenUtils::QuatToEulers(
								interfacing::K2TrackersVector.at(1).pose.orientation);

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
							interfacing::K2TrackersVector.at(1).pose.orientation =
								EigenUtils::EulersToQuat(left_ori_vector);
						}

						// Left Foot
						if (K2Settings.jointRotationTrackingOption[2] == k2_SoftwareCalculatedRotation)
						{
							interfacing::K2TrackersVector.at(2).pose.orientation = _flip
								// If flip
								? ktvr::quaternion_normal(calculatedLeftFootOrientation).inverse()
								// If no flip
								: ktvr::quaternion_normal(calculatedRightFootOrientation);

							// Apply fixes

							// Grab original orientations and make them euler angles
							Eigen::Vector3f right_ori_vector = EigenUtils::QuatToEulers(
								interfacing::K2TrackersVector.at(2).pose.orientation);

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
							interfacing::K2TrackersVector.at(2).pose.orientation =
								EigenUtils::EulersToQuat(right_ori_vector);
						}
					}
				}
			}

			/*
			 * Feet trackers orientation - Calibration-related fixes (kinectbasis-only)
			 */

			if (_device.index() == 0)
			{
				// Construct an offset quaternion with the calibration yaw
				Eigen::Quaternionf
					yawOffsetQuaternion =
						EigenUtils::EulersToQuat(Eigen::Vector3f(0.f, K2Settings.calibrationYaws.first, 0.f)),

					yawFlipQuaternion =
						EigenUtils::EulersToQuat(Eigen::Vector3f(0.f, _PI, 0.f)), // Just turn around the yaw

					waistYawFlipQuaternion =
						EigenUtils::EulersToQuat(Eigen::Vector3f(
							K2Settings.autoCalibration.first ? (-_PI / 10.0) : (-_PI / 12.0) /*M_PI / 11.0*/,
							_PI /*(autoCalibration ? M_PI : 0.f)*/,
							_PI /*(autoCalibration ? M_PI : 0.f)*/)); // Turn around Y and Z + pitchShift

				/*
				 * Apply additional things here that have to be in eulers
				 * AND be the last to be applied.
				 * For example there are: flip yaw, calibration pitch...
				 * AND are connected with joints' orientations as they are,
				 * not with the headori ones.
				 */

				if (_flip)
				{
					// Optionally disable pitch in flip mode,
					// if you want not to, just set it to true
					bool pitchOn = true;
					float pitchOffOffset = 0.0, // May be applied when pitch is off
					      pitchShift_L = 0.f, pitchShift_R = 0.f;

					if (K2Settings.jointRotationTrackingOption[1] == k2_DeviceInferredRotation ||
						K2Settings.jointRotationTrackingOption[1] == k2_SoftwareCalculatedRotation)
						pitchShift_L = _PI / 4.f; // Normal offset

					if (K2Settings.jointRotationTrackingOption[2] == k2_DeviceInferredRotation ||
						K2Settings.jointRotationTrackingOption[2] == k2_SoftwareCalculatedRotation)
						pitchShift_R = _PI / 4.f; // Normal offset

					// Left Foot
					if (K2Settings.jointRotationTrackingOption[1] != k2_FollowHMDRotation)
					{
						// Remove the pitch angle
						// Grab original orientations and make them euler angles
						Eigen::Vector3f left_ori_with_yaw =
							EigenUtils::QuatToEulers(interfacing::K2TrackersVector.at(1).pose.orientation);

						// Remove pitch from eulers and apply to the parent
						interfacing::K2TrackersVector.at(1).pose.orientation = EigenUtils::EulersToQuat(
							Eigen::Vector3f(
								pitchOn ? left_ori_with_yaw.x() - pitchShift_L : pitchOffOffset, // Disable the pitch
								(/*autoCalibration &&*/ _flip ? -1.f : 1.f) * left_ori_with_yaw.y(),
								/*(autoCalibration ? -1.f : 1.f) **/ -left_ori_with_yaw.z()));

						// Apply the turn-around flip quaternion
						interfacing::K2TrackersVector.at(1).pose.orientation =
							yawFlipQuaternion * interfacing::K2TrackersVector.at(1).pose.orientation;

						// It'll make the tracker face the kinect
						interfacing::K2TrackersVector.at(1).pose.orientation =
							yawOffsetQuaternion * interfacing::K2TrackersVector.at(1).pose.orientation;
					}

					// Right Foot
					if (K2Settings.jointRotationTrackingOption[2] != k2_FollowHMDRotation)
					{
						// Remove the pitch angle
						// Grab original orientations and make them euler angles
						Eigen::Vector3f right_ori_with_yaw =
							EigenUtils::QuatToEulers(interfacing::K2TrackersVector.at(2).pose.orientation);

						interfacing::K2TrackersVector.at(2).pose.orientation = EigenUtils::EulersToQuat(
							Eigen::Vector3f(
								pitchOn ? right_ori_with_yaw.x() - pitchShift_R : pitchOffOffset, // Disable the pitch
								(/*autoCalibration &&*/ _flip ? -1.f : 1.f) * right_ori_with_yaw.y(),
								/*(autoCalibration ? -1.f : 1.f) **/ -right_ori_with_yaw.z()));

						// Apply the turn-around flip quaternion
						interfacing::K2TrackersVector.at(2).pose.orientation =
							yawFlipQuaternion * interfacing::K2TrackersVector.at(2).pose.orientation;

						// It'll make the tracker face the kinect
						interfacing::K2TrackersVector.at(2).pose.orientation =
							yawOffsetQuaternion * interfacing::K2TrackersVector.at(2).pose.orientation;
					}

					// Waist
					if (K2Settings.jointRotationTrackingOption[0] != k2_FollowHMDRotation)
					{
						// Remove the pitch angle
						// Grab original orientations and make them euler angles
						Eigen::Vector3f waist_ori_with_yaw =
							EigenUtils::QuatToEulers(interfacing::K2TrackersVector.at(0).pose.orientation);

						// Remove pitch from eulers and apply to the parent
						interfacing::K2TrackersVector.at(0).pose.orientation = EigenUtils::EulersToQuat(
							Eigen::Vector3f(
								pitchOn ? waist_ori_with_yaw.x() : pitchOffOffset, // Disable the pitch
								-waist_ori_with_yaw.y(),
								-waist_ori_with_yaw.z()));

						// Apply the turn-around flip quaternion
						interfacing::K2TrackersVector.at(0).pose.orientation =
							waistYawFlipQuaternion * interfacing::K2TrackersVector.at(0).pose.orientation;

						// It'll make the tracker face the kinect
						interfacing::K2TrackersVector.at(0).pose.orientation =
							yawOffsetQuaternion * interfacing::K2TrackersVector.at(0).pose.orientation;
					}
				}
				else
				{
					// It'll make the tracker face the kinect

					// Waist
					if (K2Settings.jointRotationTrackingOption[0] != k2_FollowHMDRotation)
						interfacing::K2TrackersVector.at(0).pose.orientation =
							yawOffsetQuaternion * interfacing::K2TrackersVector.at(0).pose.orientation;

					// Left Foot
					if (K2Settings.jointRotationTrackingOption[1] != k2_FollowHMDRotation)
						interfacing::K2TrackersVector.at(1).pose.orientation =
							yawOffsetQuaternion * interfacing::K2TrackersVector.at(1).pose.orientation;

					// Right Foot
					if (K2Settings.jointRotationTrackingOption[2] != k2_FollowHMDRotation)
						interfacing::K2TrackersVector.at(2).pose.orientation =
							yawOffsetQuaternion * interfacing::K2TrackersVector.at(2).pose.orientation;
				}

				/*****************************************************************************************/
				// Fix waist orientation in flip: tracker is rotated by pi in roll
				//     and slightly rotated in pitch. Pitch is fault of kinect being high
				//     and resolved either by a constant for autoc or by user for manualc.
				//     Origin of flipping in roll is still unknown.
				/*****************************************************************************************/

				if (_flip &&
					(K2Settings.jointRotationTrackingOption[0] == k2_DeviceInferredRotation ||
						K2Settings.jointRotationTrackingOption[0] == k2_DisableJointRotation))
				{
					// Pitch offset quaternion for hips
					Eigen::Quaternionf waist_ori_f =
						EigenUtils::EulersToQuat(
							Eigen::Vector3f(
								K2Settings.autoCalibration.first
									? (K2Settings.jointRotationTrackingOption[0] == k2_DeviceInferredRotation
										   ? -_PI / 3.6
										   : _PI / 16.)
									: -K2Settings.calibrationPitches.first, 0., _PI));

					interfacing::K2TrackersVector.at(0).pose.orientation *= waist_ori_f;
				}
			}

			/*****************************************************************************************/
			// Fix waist orientation when following hmd after a standing pose reset
			/*****************************************************************************************/

			if (K2Settings.jointRotationTrackingOption[0] == k2_FollowHMDRotation)
			{
				// Offset to fit the playspace
				interfacing::K2TrackersVector.at(0).pose.orientation = EigenUtils::EulersToQuat(
						Eigen::Vector3f(0., -interfacing::vrPlayspaceOrientation, 0.))
					* interfacing::K2TrackersVector.at(0).pose.orientation;
			}

			/*****************************************************************************************/
			// Push fixed orientations to trackers -> swap on _flip
			/*****************************************************************************************/

			if (_flip)
				std::swap(
					interfacing::K2TrackersVector.at(1).pose.orientation,
					interfacing::K2TrackersVector.at(2).pose.orientation);

			/*****************************************************************************************/
			// Push RAW poses to trackers
			/*****************************************************************************************/

			if (_device.index() == 0)
			{
				auto const& _kinect =
					std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device);

				interfacing::K2TrackersVector.at(0).pose.position =
					_kinect->getJointPositions()[ktvr::Joint_SpineWaist];
				interfacing::K2TrackersVector.at(1).pose.position =
					_kinect->getJointPositions()[ktvr::Joint_AnkleLeft];
				interfacing::K2TrackersVector.at(2).pose.position =
					_kinect->getJointPositions()[ktvr::Joint_AnkleRight];
			}
			else if (_device.index() == 1)
			{
				auto const& _joints =
					std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device);

				interfacing::K2TrackersVector.at(0).pose.position =
					_joints->getTrackedJoints().at(K2Settings.selectedTrackedJointID[0]).getJointPosition();
				interfacing::K2TrackersVector.at(1).pose.position =
					_joints->getTrackedJoints().at(K2Settings.selectedTrackedJointID[1]).getJointPosition();
				interfacing::K2TrackersVector.at(2).pose.position =
					_joints->getTrackedJoints().at(K2Settings.selectedTrackedJointID[2]).getJointPosition();
			}
			
		}


		/*
		 * Calculate ALL poses for the override (second) device here
		 */

		// Override
		// {
		// 	// If we even HAVE an override
		// 	if (K2Settings.overrideDeviceID >= 0)
		// 	{
		// 		// Strategy: do the same as for the base but check WHAT to set

		// 		// Get the current override device
		// 		auto const& _device = TrackingDevices::getCurrentOverrideDevice();

		// 		// Compose flip
		// 		const float _facing = _yaw - radiansToDegrees(K2Settings.calibrationYaws.second);
		// 		bool _flip = false; // Assume non flipped

		// 		if ( //facing <= 25 && facing >= -25 || //if we use -180+180
		// 			(_facing <= 25 && _facing >= 0 || _facing >= 345 && _facing <= 360)) //if we use 0+360
		// 			_flip = false;
		// 		if ( //facing <= -155 && facing >= -205 || //if we use -180+180
		// 			_facing >= 155 && _facing <= 205) //if we use 0+360
		// 			_flip = true;

		// 		// Overwrite flip value depending on device & settings
		// 		// index() check should've already been done by the app tho
		// 		if (!K2Settings.isFlipEnabled || _device.index() == 1)_flip = false;

		// 		// Copy _flip to outer scope
		// 		flip_override = _flip;

		// 		/*
		// 		 * Waist tracker orientation - preparations
		// 		 */

		// 		// Check if we need to apply it
		// 		if (K2Settings.isRotationOverriddenJoint[0])
		// 		{
		// 			// Copy the orientation to the waist tracker
		// 			interfacing::K2TrackersVector.at(0).pose.orientation =
		// 				_device.index() == 0

		// 					// KinectBasis Device - just grab the correct joint : index0
		// 					? std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device)->
		// 					getJointOrientations()[ktvr::Joint_SpineWaist]

		// 					// JointsBasis Device - select based on settings : index1
		// 					: std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device)->
		// 					getTrackedJoints()[K2Settings.selectedTrackedJointID[0]].getJointOrientation();

		// 			// Optionally overwrite the rotation with HMD orientation
		// 			if (K2Settings.jointRotationTrackingOption[0] == k2_FollowHMDRotation)
		// 				interfacing::K2TrackersVector.at(0).pose.orientation = EigenUtils::EulersToQuat(
		// 					Eigen::Vector3f(0, interfacing::plugins::plugins_getHMDOrientationYaw(), 0));

		// 			// Optionally overwrite the rotation with NONE
		// 			if (K2Settings.jointRotationTrackingOption[0] == k2_DisableJointRotation)
		// 				interfacing::K2TrackersVector.at(0).pose.orientation = Eigen::Quaternionf(1, 0, 0, 0);
		// 		}

		// 		/*
		// 		 * Feet trackers orientation - preparations : Left Foot
		// 		 */

		// 		// Check if we need to apply it
		// 		if (K2Settings.isRotationOverriddenJoint[1])
		// 		{
		// 			// Copy the orientation to the left foot tracker
		// 			interfacing::K2TrackersVector.at(1).pose.orientation =
		// 				_device.index() == 0

		// 					// KinectBasis Device - grab L or R depending on flip : index0
		// 					? (_flip

		// 						   // If flip
		// 						   ? std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device)->
		// 						   getJointOrientations()[ktvr::Joint_AnkleRight].inverse()

		// 						   // If no flip
		// 						   : std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device)->
		// 						   getJointOrientations()[ktvr::Joint_AnkleLeft]
		// 					)

		// 					// JointsBasis Device - select based on settings : index1
		// 					: std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device)->
		// 					getTrackedJoints()[K2Settings.selectedTrackedJointID[1]].getJointOrientation();

		// 			// Optionally overwrite the rotation with HMD orientation
		// 			if (K2Settings.jointRotationTrackingOption[1] == k2_FollowHMDRotation)
		// 				interfacing::K2TrackersVector.at(1).pose.orientation = EigenUtils::EulersToQuat(
		// 					Eigen::Vector3f(0, interfacing::plugins::plugins_getHMDOrientationYaw(), 0));

		// 			// Optionally overwrite the rotation with NONE
		// 			if (K2Settings.jointRotationTrackingOption[1] == k2_DisableJointRotation)
		// 				interfacing::K2TrackersVector.at(1).pose.orientation = Eigen::Quaternionf(1, 0, 0, 0);
		// 		}

		// 		/*
		// 		 * Feet trackers orientation - preparations : Right Foot
		// 		 */

		// 		// Check if we need to apply it
		// 		if (K2Settings.isRotationOverriddenJoint[2])
		// 		{
		// 			// Copy the orientation to the left foot tracker
		// 			interfacing::K2TrackersVector.at(2).pose.orientation =
		// 				_device.index() == 0

		// 					// KinectBasis Device - grab L or R depending on flip : index0
		// 					? (_flip

		// 						   // If flip
		// 						   ? std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device)->
		// 						   getJointOrientations()[ktvr::Joint_AnkleLeft].inverse()

		// 						   // If no flip
		// 						   : std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device)->
		// 						   getJointOrientations()[ktvr::Joint_AnkleRight]
		// 					)

		// 					// JointsBasis Device - select based on settings : index1
		// 					: std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device)->
		// 					getTrackedJoints()[K2Settings.selectedTrackedJointID[2]].getJointOrientation();

		// 			// Optionally overwrite the rotation with HMD orientation
		// 			if (K2Settings.jointRotationTrackingOption[2] == k2_FollowHMDRotation)
		// 				interfacing::K2TrackersVector.at(2).pose.orientation = EigenUtils::EulersToQuat(
		// 					Eigen::Vector3f(0, interfacing::plugins::plugins_getHMDOrientationYaw(), 0));

		// 			// Optionally overwrite the rotation with NONE
		// 			if (K2Settings.jointRotationTrackingOption[2] == k2_DisableJointRotation)
		// 				interfacing::K2TrackersVector.at(2).pose.orientation = Eigen::Quaternionf(1, 0, 0, 0);
		// 		}

		// 		/*
		// 		 * Feet trackers orientation - preparations : No-Yaw mode
		// 		 */

		// 		// We've got no no-yaw mode in amethyst (✿◕‿◕✿)

		// 		/*
		// 		 * Feet trackers orientation - preparations : App / Software rotation
		// 		 */


		// 		// Check if we need to apply it
		// 		if (K2Settings.isRotationOverriddenJoint[1] ||
		// 			K2Settings.isRotationOverriddenJoint[2])
		// 		{
		// 			if (_device.index() == 0 &&
		// 				(K2Settings.jointRotationTrackingOption[1] == k2_SoftwareCalculatedRotation ||
		// 					K2Settings.jointRotationTrackingOption[2] == k2_SoftwareCalculatedRotation))
		// 			{
		// 				auto const& _kinect = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device);

		// 				if (_kinect->getDeviceCharacteristics() == ktvr::K2_Character_Full ||
		// 					_kinect->getDeviceCharacteristics() == ktvr::K2_Character_Simple)
		// 				{
		// 					const auto _joints = _kinect->getJointPositions();
		// 					const auto _joint_states = _kinect->getTrackingStates();

		// 					// Placeholders for mathbased rots
		// 					Eigen::Quaternionf calculatedLeftFootOrientation, calculatedRightFootOrientation;

		// 					// Capture needed joints' positions
		// 					Eigen::Vector3f up(0, 1, 0), forward(0, 0, 1), backward(0, 0, -1),
		// 					                ankleLeftPose(
		// 						                _joints[ktvr::Joint_AnkleLeft].x(),
		// 						                _joints[ktvr::Joint_AnkleLeft].y(),
		// 						                _joints[ktvr::Joint_AnkleLeft].z()),

		// 					                ankleRightPose(
		// 						                _joints[ktvr::Joint_AnkleRight].x(),
		// 						                _joints[ktvr::Joint_AnkleRight].y(),
		// 						                _joints[ktvr::Joint_AnkleRight].z()),

		// 					                footLeftPose(
		// 						                _joints[ktvr::Joint_FootLeft].x(),
		// 						                _joints[ktvr::Joint_FootLeft].y(),
		// 						                _joints[ktvr::Joint_FootLeft].z()),

		// 					                footRightPose(
		// 						                _joints[ktvr::Joint_FootRight].x(),
		// 						                _joints[ktvr::Joint_FootRight].y(),
		// 						                _joints[ktvr::Joint_FootRight].z()),

		// 					                kneeLeftPose(
		// 						                _joints[ktvr::Joint_KneeLeft].x(),
		// 						                _joints[ktvr::Joint_KneeLeft].y(),
		// 						                _joints[ktvr::Joint_KneeLeft].z()),

		// 					                kneeRightPose(
		// 						                _joints[ktvr::Joint_KneeRight].x(),
		// 						                _joints[ktvr::Joint_KneeRight].y(),
		// 						                _joints[ktvr::Joint_KneeRight].z());

		// 					// Calculate euler yaw foot orientation, we'll need it later
		// 					Eigen::Vector3f
		// 						footLeftRawOrientation = EigenUtils::DirectionQuat(
		// 							Eigen::Vector3f(ankleLeftPose.x(), 0.f, ankleLeftPose.z()),
		// 							Eigen::Vector3f(footLeftPose.x(), 0.f, footLeftPose.z()),
		// 							forward).toRotationMatrix().eulerAngles(0, 1, 2),

		// 						footRightRawOrientation = EigenUtils::DirectionQuat(
		// 							Eigen::Vector3f(ankleRightPose.x(), 0.f, ankleRightPose.z()),
		// 							Eigen::Vector3f(footRightPose.x(), 0.f, footRightPose.z()),
		// 							forward).toRotationMatrix().eulerAngles(0, 1, 2);

		// 					// Flip the yaw around, without reversing it -> we need it basing to 0
		// 					// (what an irony that we actually need to reverse it...)
		// 					footLeftRawOrientation.y() *= -1.f;
		// 					footLeftRawOrientation.y() += _PI;
		// 					footRightRawOrientation.y() *= -1.f;
		// 					footRightRawOrientation.y() += _PI;

		// 					// Make the yaw less sensitive
		// 					// Decided to go for radians for the read-ability
		// 					// (Although my code is shit anyway, and there'll be none in the end)
		// 					float lsFixedYaw[2] = {
		// 						radiansToDegrees(footLeftRawOrientation.y()),
		// 						radiansToDegrees(footRightRawOrientation.y())
		// 					}; // L, R

		// 					// Left
		// 					if (lsFixedYaw[0] > 180.f && lsFixedYaw[0] < 360.f)
		// 						lsFixedYaw[0] = 360.f - abs(lsFixedYaw[0] - 360.f) * .5f;
		// 					else if (lsFixedYaw[0] < 180.f && lsFixedYaw[0] > 0.f)
		// 						lsFixedYaw[0] *= .5f;

		// 					// Right
		// 					if (lsFixedYaw[1] > 180.f && lsFixedYaw[1] < 360.f)
		// 						lsFixedYaw[1] = 360.f - abs(lsFixedYaw[1] - 360.f) * .5f;
		// 					else if (lsFixedYaw[1] < 180.f && lsFixedYaw[1] > 0.f)
		// 						lsFixedYaw[1] *= .5f;

		// 					// Apply to the base
		// 					footLeftRawOrientation.y() = degreesToRadians(lsFixedYaw[0]); // Back to the RAD format
		// 					footRightRawOrientation.y() = degreesToRadians(lsFixedYaw[1]);

		// 					// Construct a helpful offsetting quaternion from the stuff we got
		// 					// It's made like Quat->Eulers->Quat because we may need to adjust some things on-to-go
		// 					Eigen::Quaternionf
		// 						leftFootYawOffsetQuaternion = EigenUtils::EulersToQuat(footLeftRawOrientation),
		// 						// There is no X and Z anyway
		// 						rightFootYawOffsetQuaternion = EigenUtils::EulersToQuat(footRightRawOrientation);

		// 					// Smooth a bit with a slerp
		// 					yawFilteringQuaternion[0] = yawFilteringQuaternion[0].slerp(
		// 						.25f, leftFootYawOffsetQuaternion);
		// 					yawFilteringQuaternion[1] = yawFilteringQuaternion[1].slerp(
		// 						.25f, rightFootYawOffsetQuaternion);

		// 					// Apply to the base
		// 					leftFootYawOffsetQuaternion = yawFilteringQuaternion[0];
		// 					rightFootYawOffsetQuaternion = yawFilteringQuaternion[1];

		// 					// Calculate the knee-ankle orientation, aka "Tibia"
		// 					// We aren't disabling look-thorough yaw, since it'll be 0
		// 					Eigen::Quaternionf
		// 						knee_ankleLeftOrientationQuaternion = EigenUtils::DirectionQuat(
		// 							kneeLeftPose, ankleLeftPose, forward),
		// 						knee_ankleRightOrientationQuaternion = EigenUtils::DirectionQuat(
		// 							kneeRightPose, ankleRightPose, forward);

		// 					// The tuning quat
		// 					Eigen::Quaternionf
		// 						tuneQuaternion_first = Eigen::Quaternionf(1, 0, 0, 0);

		// 					// Now adjust some values like playspace yaw and pitch, additional rotations
		// 					// -> they're facing purely down and Z / Y are flipped
		// 					tuneQuaternion_first =
		// 						EigenUtils::EulersToQuat(
		// 							Eigen::Vector3f(
		// 								_PI / 5.f,
		// 								0.f,
		// 								0.f
		// 							));

		// 					// Apply the fine-tuning to global variable
		// 					knee_ankleLeftOrientationQuaternion = tuneQuaternion_first *
		// 						knee_ankleLeftOrientationQuaternion;
		// 					knee_ankleRightOrientationQuaternion = tuneQuaternion_first *
		// 						knee_ankleRightOrientationQuaternion;

		// 					// Grab original orientations and make them euler angles
		// 					Eigen::Vector3f left_knee_ori_full = EigenUtils::QuatToEulers(
		// 						knee_ankleLeftOrientationQuaternion);
		// 					Eigen::Vector3f right_knee_ori_full =
		// 						EigenUtils::QuatToEulers(knee_ankleRightOrientationQuaternion);

		// 					// Try to fix yaw and roll mismatch, caused by XYZ XZY mismatch
		// 					knee_ankleLeftOrientationQuaternion = EigenUtils::EulersToQuat(
		// 						Eigen::Vector3f(
		// 							left_knee_ori_full.x() - _PI / 1.6f,
		// 							0.0, // left_knee_ori_full.z(), // actually 0.0 but okay
		// 							-left_knee_ori_full.y()));

		// 					knee_ankleRightOrientationQuaternion = EigenUtils::EulersToQuat(
		// 						Eigen::Vector3f(
		// 							right_knee_ori_full.x() - _PI / 1.6f,
		// 							0.0, // right_knee_ori_full.z(), // actually 0.0 but okay
		// 							-right_knee_ori_full.y()));

		// 					if (_joint_states[ktvr::Joint_AnkleLeft] == ktvr::ITrackedJointState::State_Tracked)
		// 						// All the rotations
		// 						calculatedLeftFootOrientation = leftFootYawOffsetQuaternion *
		// 							knee_ankleLeftOrientationQuaternion;
		// 					else
		// 						// Without the foot's yaw
		// 						calculatedLeftFootOrientation = knee_ankleLeftOrientationQuaternion;

		// 					if (_joint_states[ktvr::Joint_AnkleRight] == ktvr::ITrackedJointState::State_Tracked)
		// 						// All the rotations
		// 						calculatedRightFootOrientation = rightFootYawOffsetQuaternion *
		// 							knee_ankleRightOrientationQuaternion;
		// 					else
		// 						// Without the foot's yaw
		// 						calculatedRightFootOrientation = knee_ankleRightOrientationQuaternion;

		// 					// The tuning quat
		// 					Eigen::Quaternionf
		// 						leftFootFineTuneQuaternion = Eigen::Quaternionf(1, 0, 0, 0),
		// 						rightFootFineTuneQuaternion = Eigen::Quaternionf(1, 0, 0, 0);

		// 					// Now adjust some values like playspace yaw and pitch, additional rotations

		// 					leftFootFineTuneQuaternion =
		// 						EigenUtils::EulersToQuat( // Lift trackers up a bit
		// 							Eigen::Vector3f(
		// 								2.8623399733f, // this one's in radians alr
		// 								0.f, //glm::radians(KinectSettings::calibration_trackers_yaw),
		// 								0.f
		// 							));

		// 					rightFootFineTuneQuaternion =
		// 						EigenUtils::EulersToQuat( // Lift trackers up a bit
		// 							Eigen::Vector3f(
		// 								2.8623399733f, // this one's in radians alr
		// 								0.f, //glm::radians(KinectSettings::calibration_trackers_yaw),
		// 								0.f
		// 							));

		// 					// Apply the fine-tuning to global variable
		// 					calculatedLeftFootOrientation = leftFootFineTuneQuaternion * calculatedLeftFootOrientation;
		// 					calculatedRightFootOrientation = rightFootFineTuneQuaternion *
		// 						calculatedRightFootOrientation;

		// 					// Push to global
		// 					if (_kinect->isSkeletonTracked())
		// 					{
		// 						// Left Foot
		// 						if (K2Settings.jointRotationTrackingOption[1] == k2_SoftwareCalculatedRotation &&
		// 							K2Settings.isRotationOverriddenJoint[1])
		// 						{
		// 							interfacing::K2TrackersVector.at(1).pose.orientation = _flip
		// 								// If flip
		// 								? ktvr::quaternion_normal(calculatedRightFootOrientation).inverse()
		// 								// If no flip
		// 								: ktvr::quaternion_normal(calculatedLeftFootOrientation);

		// 							// Apply fixes

		// 							// Grab original orientations and make them euler angles
		// 							Eigen::Vector3f left_ori_vector = EigenUtils::QuatToEulers(
		// 								interfacing::K2TrackersVector.at(1).pose.orientation);

		// 							// Kind of a solution for flipping at too big X.
		// 							// Found out during testing,
		// 							// no other known mathematical reason (maybe except gimbal lock)

		// 							/****************************************************/

		// 							if (left_ori_vector.y() <= 0.f
		// 								&& left_ori_vector.y() >= -1.f

		// 								&& left_ori_vector.z() <= -1.f
		// 								&& left_ori_vector.z() >= -_PI)

		// 								left_ori_vector.y() += -_PI;

		// 							/****************************************************/

		// 							// Apply to the base
		// 							interfacing::K2TrackersVector.at(1).pose.orientation =
		// 								EigenUtils::EulersToQuat(left_ori_vector);
		// 						}

		// 						// Left Foot
		// 						if (K2Settings.jointRotationTrackingOption[2] == k2_SoftwareCalculatedRotation &&
		// 							K2Settings.isRotationOverriddenJoint[2])
		// 						{
		// 							interfacing::K2TrackersVector.at(2).pose.orientation = _flip
		// 								// If flip
		// 								? ktvr::quaternion_normal(calculatedLeftFootOrientation).inverse()
		// 								// If no flip
		// 								: ktvr::quaternion_normal(calculatedRightFootOrientation);

		// 							// Apply fixes

		// 							// Grab original orientations and make them euler angles
		// 							Eigen::Vector3f right_ori_vector = EigenUtils::QuatToEulers(
		// 								interfacing::K2TrackersVector.at(2).pose.orientation);

		// 							// Kind of a solution for flipping at too big X.
		// 							// Found out during testing,
		// 							// no other known mathematical reason (maybe except gimbal lock)

		// 							/****************************************************/

		// 							if (right_ori_vector.y() <= 0.f
		// 								&& right_ori_vector.y() >= -1.f

		// 								&& right_ori_vector.z() <= -1.f
		// 								&& right_ori_vector.z() >= -_PI)

		// 								right_ori_vector.y() += -_PI;

		// 							/****************************************************/

		// 							// Apply to the base
		// 							interfacing::K2TrackersVector.at(2).pose.orientation =
		// 								EigenUtils::EulersToQuat(right_ori_vector);
		// 						}
		// 					}
		// 				}
		// 			}
		// 		}

		// 		/*
		// 		 * Feet trackers orientation - Calibration-related fixes (kinectbasis-only)
		// 		 */

		// 		if (_device.index() == 0)
		// 		{
		// 			// Construct an offset quaternion with the calibration yaw
		// 			Eigen::Quaternionf
		// 				yawOffsetQuaternion =
		// 					EigenUtils::EulersToQuat(Eigen::Vector3f(0.f, K2Settings.calibrationYaws.second, 0.f)),

		// 				yawFlipQuaternion =
		// 					EigenUtils::EulersToQuat(Eigen::Vector3f(0.f, _PI, 0.f)), // Just turn around the yaw

		// 				waistYawFlipQuaternion =
		// 					EigenUtils::EulersToQuat(Eigen::Vector3f(
		// 						K2Settings.autoCalibration.second ? (-_PI / 10.0) : (-_PI / 12.0) /*M_PI / 11.0*/,
		// 						_PI /*(autoCalibration ? M_PI : 0.f)*/,
		// 						_PI /*(autoCalibration ? M_PI : 0.f)*/)); // Turn around Y and Z + pitchShift

		// 			/*
		// 			 * Apply additional things here that have to be in eulers
		// 			 * AND be the last to be applied.
		// 			 * For example there are: flip yaw, calibration pitch...
		// 			 * AND are connected with joints' orientations as they are,
		// 			 * not with the headori ones.
		// 			 */

		// 			if (_flip)
		// 			{
		// 				// Optionally disable pitch in flip mode,
		// 				// if you want not to, just set it to true
		// 				bool pitchOn = true;
		// 				float pitchOffOffset = 0.0, // May be applied when pitch is off
		// 				      pitchShift_L = 0.f, pitchShift_R = 0.f;

		// 				if (K2Settings.jointRotationTrackingOption[1] == k2_DeviceInferredRotation ||
		// 					K2Settings.jointRotationTrackingOption[1] == k2_SoftwareCalculatedRotation)
		// 					pitchShift_L = _PI / 4.f; // Normal offset

		// 				if (K2Settings.jointRotationTrackingOption[2] == k2_DeviceInferredRotation ||
		// 					K2Settings.jointRotationTrackingOption[2] == k2_SoftwareCalculatedRotation)
		// 					pitchShift_R = _PI / 4.f; // Normal offset

		// 				// Left Foot
		// 				if (K2Settings.jointRotationTrackingOption[1] != k2_FollowHMDRotation &&
		// 					K2Settings.isRotationOverriddenJoint[1])
		// 				{
		// 					// Remove the pitch angle
		// 					// Grab original orientations and make them euler angles
		// 					Eigen::Vector3f left_ori_with_yaw =
		// 						EigenUtils::QuatToEulers(interfacing::K2TrackersVector.at(1).pose.orientation);

		// 					// Remove pitch from eulers and apply to the parent
		// 					interfacing::K2TrackersVector.at(1).pose.orientation = EigenUtils::EulersToQuat(
		// 						Eigen::Vector3f(
		// 							pitchOn ? left_ori_with_yaw.x() - pitchShift_L : pitchOffOffset,
		// 							// Disable the pitch
		// 							(/*autoCalibration &&*/ _flip ? -1.f : 1.f) * left_ori_with_yaw.y(),
		// 							/*(autoCalibration ? -1.f : 1.f) **/ -left_ori_with_yaw.z()));

		// 					// Apply the turn-around flip quaternion
		// 					interfacing::K2TrackersVector.at(1).pose.orientation =
		// 						yawFlipQuaternion * interfacing::K2TrackersVector.at(1).pose.orientation;

		// 					// It'll make the tracker face the kinect
		// 					interfacing::K2TrackersVector.at(1).pose.orientation =
		// 						yawOffsetQuaternion * interfacing::K2TrackersVector.at(1).pose.orientation;
		// 				}

		// 				// Right Foot
		// 				if (K2Settings.jointRotationTrackingOption[2] != k2_FollowHMDRotation &&
		// 					K2Settings.isRotationOverriddenJoint[2])
		// 				{
		// 					// Remove the pitch angle
		// 					// Grab original orientations and make them euler angles
		// 					Eigen::Vector3f right_ori_with_yaw =
		// 						EigenUtils::QuatToEulers(interfacing::K2TrackersVector.at(2).pose.orientation);

		// 					interfacing::K2TrackersVector.at(2).pose.orientation = EigenUtils::EulersToQuat(
		// 						Eigen::Vector3f(
		// 							pitchOn ? right_ori_with_yaw.x() - pitchShift_R : pitchOffOffset,
		// 							// Disable the pitch
		// 							(/*autoCalibration &&*/ _flip ? -1.f : 1.f) * right_ori_with_yaw.y(),
		// 							/*(autoCalibration ? -1.f : 1.f) **/ -right_ori_with_yaw.z()));

		// 					// Apply the turn-around flip quaternion
		// 					interfacing::K2TrackersVector.at(2).pose.orientation =
		// 						yawFlipQuaternion * interfacing::K2TrackersVector.at(2).pose.orientation;

		// 					// It'll make the tracker face the kinect
		// 					interfacing::K2TrackersVector.at(2).pose.orientation =
		// 						yawOffsetQuaternion * interfacing::K2TrackersVector.at(2).pose.orientation;
		// 				}

		// 				// Waist
		// 				if (K2Settings.jointRotationTrackingOption[0] != k2_FollowHMDRotation &&
		// 					K2Settings.isRotationOverriddenJoint[0])
		// 				{
		// 					// Remove the pitch angle
		// 					// Grab original orientations and make them euler angles
		// 					Eigen::Vector3f waist_ori_with_yaw =
		// 						EigenUtils::QuatToEulers(interfacing::K2TrackersVector.at(0).pose.orientation);

		// 					// Remove pitch from eulers and apply to the parent
		// 					interfacing::K2TrackersVector.at(0).pose.orientation = EigenUtils::EulersToQuat(
		// 						Eigen::Vector3f(
		// 							pitchOn ? waist_ori_with_yaw.x() : pitchOffOffset, // Disable the pitch
		// 							-waist_ori_with_yaw.y(),
		// 							-waist_ori_with_yaw.z()));

		// 					// Apply the turn-around flip quaternion
		// 					interfacing::K2TrackersVector.at(0).pose.orientation =
		// 						waistYawFlipQuaternion * interfacing::K2TrackersVector.at(0).pose.orientation;

		// 					// It'll make the tracker face the kinect
		// 					interfacing::K2TrackersVector.at(0).pose.orientation =
		// 						yawOffsetQuaternion * interfacing::K2TrackersVector.at(0).pose.orientation;
		// 				}
		// 			}
		// 			else
		// 			{
		// 				// It'll make the tracker face the kinect

		// 				// Waist
		// 				if (K2Settings.jointRotationTrackingOption[0] != k2_FollowHMDRotation &&
		// 					K2Settings.isRotationOverriddenJoint[0])
		// 					interfacing::K2TrackersVector.at(0).pose.orientation =
		// 						yawOffsetQuaternion * interfacing::K2TrackersVector.at(0).pose.orientation;

		// 				// Left Foot
		// 				if (K2Settings.jointRotationTrackingOption[1] != k2_FollowHMDRotation &&
		// 					K2Settings.isRotationOverriddenJoint[1])
		// 					interfacing::K2TrackersVector.at(1).pose.orientation =
		// 						yawOffsetQuaternion * interfacing::K2TrackersVector.at(1).pose.orientation;

		// 				// Right Foot
		// 				if (K2Settings.jointRotationTrackingOption[2] != k2_FollowHMDRotation &&
		// 					K2Settings.isRotationOverriddenJoint[2])
		// 					interfacing::K2TrackersVector.at(2).pose.orientation =
		// 						yawOffsetQuaternion * interfacing::K2TrackersVector.at(2).pose.orientation;
		// 			}

		// 			/*****************************************************************************************/
		// 			// Fix waist orientation in flip: tracker is rotated by pi in roll
		// 			//     and slightly rotated in pitch. Pitch is fault of kinect being high
		// 			//     and resolved either by a constant for autoc or by user for manualc.
		// 			//     Origin of flipping in roll is still unknown.
		// 			/*****************************************************************************************/

		// 			if (_flip && K2Settings.isRotationOverriddenJoint[0] &&
		// 				(K2Settings.jointRotationTrackingOption[0] == k2_DeviceInferredRotation ||
		// 					K2Settings.jointRotationTrackingOption[0] == k2_DisableJointRotation))
		// 			{
		// 				// Pitch offset quaternion for hips
		// 				Eigen::Quaternionf waist_ori_f =
		// 					EigenUtils::EulersToQuat(
		// 						Eigen::Vector3f(
		// 							K2Settings.autoCalibration.second
		// 								? (K2Settings.jointRotationTrackingOption[0] == k2_DeviceInferredRotation
		// 									   ? -_PI / 3.6
		// 									   : _PI / 16.)
		// 								: -K2Settings.calibrationPitches.second, 0., _PI));

		// 				interfacing::K2TrackersVector.at(0).pose.orientation *= waist_ori_f;
		// 			}
		// 		}

		// 		/*****************************************************************************************/
		// 		// Fix waist orientation when following hmd after a standing pose reset
		// 		/*****************************************************************************************/

		// 		if (K2Settings.jointRotationTrackingOption[0] == k2_FollowHMDRotation &&
		// 			K2Settings.isRotationOverriddenJoint[0])
		// 		{
		// 			// Offset to fit the playspace
		// 			interfacing::K2TrackersVector.at(0).pose.orientation = EigenUtils::EulersToQuat(
		// 					Eigen::Vector3f(0., -interfacing::plugins::plugins_getHMDOrientationYaw(), 0.))
		// 				* interfacing::K2TrackersVector.at(0).pose.orientation;
		// 		}

		// 		/*****************************************************************************************/
		// 		// Push RAW poses to trackers
		// 		/*****************************************************************************************/

		// 		if (_device.index() == 0)
		// 		{
		// 			auto const& _kinect =
		// 				std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_device);

		// 			if (K2Settings.isPositionOverriddenJoint[0])
		// 				interfacing::K2TrackersVector.at(0).pose.position =
		// 					_kinect->getJointPositions()[ktvr::Joint_SpineWaist];

		// 			if (K2Settings.isPositionOverriddenJoint[1])
		// 				interfacing::K2TrackersVector.at(1).pose.position =
		// 					_kinect->getJointPositions()[ktvr::Joint_AnkleLeft];

		// 			if (K2Settings.isPositionOverriddenJoint[2])
		// 				interfacing::K2TrackersVector.at(2).pose.position =
		// 					_kinect->getJointPositions()[ktvr::Joint_AnkleRight];
		// 		}
		// 		else if (_device.index() == 1)
		// 		{
		// 			auto const& _joints =
		// 				std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_device);

		// 			if (K2Settings.isPositionOverriddenJoint[0])
		// 				interfacing::K2TrackersVector.at(0).pose.position =
		// 					_joints->getTrackedJoints().at(K2Settings.selectedTrackedJointID[0]).getJointPosition();

		// 			if (K2Settings.isPositionOverriddenJoint[1])
		// 				interfacing::K2TrackersVector.at(1).pose.position =
		// 					_joints->getTrackedJoints().at(K2Settings.selectedTrackedJointID[1]).getJointPosition();

		// 			if (K2Settings.isPositionOverriddenJoint[2])
		// 				interfacing::K2TrackersVector.at(2).pose.position =
		// 					_joints->getTrackedJoints().at(K2Settings.selectedTrackedJointID[2]).getJointPosition();
		// 		}

		// 		// Slow down the rotation a bit
		// 		if (K2Settings.isRotationOverriddenJoint[0])
		// 			interfacing::K2TrackersVector.at(0).pose.orientation =
		// 				_flip
		// 					? interfacing::K2TrackersVector.
		// 					  at(0).SLERPSlowOrientation
		// 					: interfacing::K2TrackersVector.
		// 					  at(0).SLERPOrientation;

		// 		if (K2Settings.isRotationOverriddenJoint[1])
		// 			interfacing::K2TrackersVector.at(1).pose.orientation =
		// 				_flip
		// 					? interfacing::K2TrackersVector.
		// 					  at(1).SLERPSlowOrientation
		// 					: interfacing::K2TrackersVector.
		// 					  at(1).SLERPOrientation;

		// 		if (K2Settings.isRotationOverriddenJoint[2])
		// 			interfacing::K2TrackersVector.at(2).pose.orientation =
		// 				_flip
		// 					? interfacing::K2TrackersVector.
		// 					  at(2).SLERPSlowOrientation
		// 					: interfacing::K2TrackersVector.
		// 					  at(2).SLERPOrientation;
		// 	}
		// }


		/*
		 * Calculate ALL poses for the app : calibrate positions, apply offsets etc.
		 */

		{
			/*****************************************************************************************/
			// Push offset rots to trackers
			/*****************************************************************************************/

			interfacing::K2TrackersVector.at(0).orientationOffset =
				EigenUtils::EulersToQuat(K2Settings.rotationJointsOffsets[0].cast<float>());

			interfacing::K2TrackersVector.at(1).orientationOffset =
				EigenUtils::EulersToQuat(K2Settings.rotationJointsOffsets[1].cast<float>());

			interfacing::K2TrackersVector.at(2).orientationOffset =
				EigenUtils::EulersToQuat(K2Settings.rotationJointsOffsets[2].cast<float>());

			/*****************************************************************************************/
			// Push offset poses to trackers
			/*****************************************************************************************/

			interfacing::K2TrackersVector.at(0).positionOffset =
				K2Settings.positionJointsOffsets[0].cast<float>();

			interfacing::K2TrackersVector.at(1).positionOffset =
				K2Settings.positionJointsOffsets[1].cast<float>();

			interfacing::K2TrackersVector.at(2).positionOffset =
				K2Settings.positionJointsOffsets[2].cast<float>();
		}
	}

	// The main program loop
	inline void K2MainLoop()
	{
		// Warning: this is meant to work as fire-and-forget
		LOG(INFO) << "[K2Main] Waiting for the start sem to open..";
		shared::devices::smphSignalStartMain.acquire();

		LOG(INFO) << "[K2Main] Starting the main app loop now...";
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
}
