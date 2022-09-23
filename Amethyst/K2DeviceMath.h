#pragma once
#include "K2Interfacing.h"
#include "TrackingDevices.h"

namespace TrackingDevices::Math
{
	// Calculate the math-based orientation
	// Take the device pointer as the input
	// Output to K2Settings.K2TrackersVector[L:0 | R:1]
	inline void CalculateFeetSoftwareOrientation(
		ktvr::K2TrackingDeviceBase_SkeletonBasis* const& _kinect)
	{
		using namespace k2app;
		using namespace interfacing;

		/* You need to calculate ori of both feet if you wanna support flip */
		if (_kinect->getDeviceCharacteristics() == ktvr::K2_Character_Full ||
			_kinect->getDeviceCharacteristics() == ktvr::K2_Character_Simple)
		{
			auto _joints = _kinect->getTrackedJoints();

			// Placeholders for mathbased rots
			Eigen::Quaternionf calculatedLeftFootOrientation, calculatedRightFootOrientation;

			// Capture needed joints' positions
			Eigen::Vector3f forward(0, 0, 1),
			                ankleLeftPose(
				                _joints[ktvr::Joint_AnkleLeft].getJointPosition().x(),
				                _joints[ktvr::Joint_AnkleLeft].getJointPosition().y(),
				                _joints[ktvr::Joint_AnkleLeft].getJointPosition().z()),

			                ankleRightPose(
				                _joints[ktvr::Joint_AnkleRight].getJointPosition().x(),
								_joints[ktvr::Joint_AnkleRight].getJointPosition().y(),
								_joints[ktvr::Joint_AnkleRight].getJointPosition().z()),

			                footLeftPose(
				                _joints[ktvr::Joint_FootLeft].getJointPosition().x(),
								_joints[ktvr::Joint_FootLeft].getJointPosition().y(),
								_joints[ktvr::Joint_FootLeft].getJointPosition().z()),

			                footRightPose(
				                _joints[ktvr::Joint_FootRight].getJointPosition().x(),
								_joints[ktvr::Joint_FootRight].getJointPosition().y(),
								_joints[ktvr::Joint_FootRight].getJointPosition().z()),

			                kneeLeftPose(
				                _joints[ktvr::Joint_KneeLeft].getJointPosition().x(),
								_joints[ktvr::Joint_KneeLeft].getJointPosition().y(),
								_joints[ktvr::Joint_KneeLeft].getJointPosition().z()),

			                kneeRightPose(
				                _joints[ktvr::Joint_KneeRight].getJointPosition().x(),
								_joints[ktvr::Joint_KneeRight].getJointPosition().y(),
								_joints[ktvr::Joint_KneeRight].getJointPosition().z());

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
				leftFootPreFilteredQuaternion = EigenUtils::EulersToQuat(footLeftRawOrientation),
				// There is no X and Z anyway
				rightFootPreFilteredQuaternion = EigenUtils::EulersToQuat(footRightRawOrientation);

			// Smooth a bit with a slerp
			yawFilteringQuaternion[0] = yawFilteringQuaternion[0].slerp(.25f, leftFootPreFilteredQuaternion);
			yawFilteringQuaternion[1] = yawFilteringQuaternion[1].slerp(.25f, rightFootPreFilteredQuaternion);

			// Apply to the base
			leftFootPreFilteredQuaternion = yawFilteringQuaternion[0];
			rightFootPreFilteredQuaternion = yawFilteringQuaternion[1];

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

			if (_joints[ktvr::Joint_AnkleLeft].getTrackingState() == ktvr::ITrackedJointState::State_Tracked)
				// All the rotations
				calculatedLeftFootOrientation = leftFootPreFilteredQuaternion *
					knee_ankleLeftOrientationQuaternion;
			else
				// Without the foot's yaw
				calculatedLeftFootOrientation = knee_ankleLeftOrientationQuaternion;

			if (_joints[ktvr::Joint_AnkleRight].getTrackingState() == ktvr::ITrackedJointState::State_Tracked)
				// All the rotations
				calculatedRightFootOrientation = rightFootPreFilteredQuaternion *
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

			// Push to global, if valid
			if (_kinect->isSkeletonTracked())
			{
				/* Here the actual result is being applied, either to only one or both */

				// Calculate the Left Foot?
				if (K2Settings.K2TrackersVector[1].orientationTrackingOption == k2_SoftwareCalculatedRotation)
				{
					K2Settings.K2TrackersVector[1].pose_orientation =
						base_flip
							// If flip
							? ktvr::quaternion_normal(
								calculatedRightFootOrientation).
							inverse()

							// If no flip
							: ktvr::quaternion_normal(
								calculatedLeftFootOrientation);

					// Apply fixes

					// Grab original orientations and make them euler angles
					Eigen::Vector3f left_ori_vector = EigenUtils::QuatToEulers(
						K2Settings.K2TrackersVector[1].pose_orientation);

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
					K2Settings.K2TrackersVector[1].pose_orientation =
						EigenUtils::EulersToQuat(left_ori_vector);
				}

				// Calculate the Right Foot?
				if (K2Settings.K2TrackersVector[2].orientationTrackingOption == k2_SoftwareCalculatedRotation)
				{
					K2Settings.K2TrackersVector[2].pose_orientation =
						base_flip
							// If flip
							? ktvr::quaternion_normal(
								calculatedLeftFootOrientation).
							inverse()

							// If no flip
							: ktvr::quaternion_normal(
								calculatedRightFootOrientation);

					// Apply fixes

					// Grab original orientations and make them euler angles
					Eigen::Vector3f right_ori_vector = EigenUtils::QuatToEulers(
						K2Settings.K2TrackersVector[2].pose_orientation);

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
					K2Settings.K2TrackersVector[2].pose_orientation =
						EigenUtils::EulersToQuat(right_ori_vector);
				}
			}
		}
	}

	// Calculate the math-based orientation (V2)
	// Take the device pointer as the input
	// Output to K2Settings.K2TrackersVector[L:0 | R:1]
	inline void CalculateFeetSoftwareOrientation_V2(
		ktvr::K2TrackingDeviceBase_SkeletonBasis* const& _kinect)
	{
		using namespace k2app;
		using namespace interfacing;

		/* You need to calculate ori of both feet if you wanna support flip */
		if (_kinect->getDeviceCharacteristics() == ktvr::K2_Character_Full ||
			_kinect->getDeviceCharacteristics() == ktvr::K2_Character_Simple)
		{
			// Get device joints and their states
			const auto _joints = _kinect->getTrackedJoints();

			// Create placeholders for math-based rots
			auto calculatedLeftFootOrientation = Eigen::Quaternionf(1, 0, 0, 0);
			auto calculatedRightFootOrientation = Eigen::Quaternionf(1, 0, 0, 0);


			/*
			 * Calculate orientations here
			 *
			 * Cache them to [calculatedLeftFootOrientation, calculatedRightFootOrientation]
			 * and later push to the upper (tracker) scope, using the selectors below
			 *
			 * Device-provided joints and their states are in [_joints, _joint_states]
			 *
			 * This function is already registered for running from K2Main.h
			 * when [tracker].orientationTrackingOption == k2_SoftwareCalculatedRotation_V2
			 *
			 */


			// Check if the tracking is valid
			if (_kinect->isSkeletonTracked())
			{
				/* Here the actual result is being applied, either to only one or both trackers */

				// Calculate the Left Foot?
				if (K2Settings.K2TrackersVector[1].orientationTrackingOption == k2_SoftwareCalculatedRotation_V2)
				{
					// Flip the orientation based on the host flip
					K2Settings.K2TrackersVector[1].pose_orientation =
						base_flip
							// If flip
							? ktvr::quaternion_normal(
								calculatedRightFootOrientation).
							inverse()

							// If no flip
							: ktvr::quaternion_normal(
								calculatedLeftFootOrientation);

					// Standard math-based pushes some fixes here
					// Feel free to uncomment if you even need them
					// (Or just fix the fixes and remove em in both...)

					/*
					// Apply fixes

					// Grab original orientations and make them euler angles
					Eigen::Vector3f left_ori_vector = EigenUtils::QuatToEulers(
						K2Settings.K2TrackersVector[1].pose_orientation);

					// Kind of a solution for flipping at too big X.
					// Found out during testing,
					// no other known mathematical reason (maybe except gimbal lock)
					
					// ------------------------------------------
					if (left_ori_vector.y() <= 0.f
						&& left_ori_vector.y() >= -1.f

						&& left_ori_vector.z() <= -1.f
						&& left_ori_vector.z() >= -_PI)

						left_ori_vector.y() += -_PI;
					// ------------------------------------------

					// Apply to the base
					K2Settings.K2TrackersVector[1].pose_orientation =
						EigenUtils::EulersToQuat(left_ori_vector);
					*/
				}

				// Calculate the Right Foot?
				if (K2Settings.K2TrackersVector[2].orientationTrackingOption == k2_SoftwareCalculatedRotation_V2)
				{
					// Flip the orientation based on the host flip
					K2Settings.K2TrackersVector[2].pose_orientation =
						base_flip
							// If flip
							? ktvr::quaternion_normal(
								calculatedLeftFootOrientation).
							inverse()

							// If no flip
							: ktvr::quaternion_normal(
								calculatedRightFootOrientation);

					// Standard math-based pushes some fixes here
					// Feel free to uncomment if you even need them
					// (Or just fix the fixes and remove em in both...)

					/*
					// Apply fixes

					// Grab original orientations and make them euler angles
					Eigen::Vector3f right_ori_vector = EigenUtils::QuatToEulers(
						K2Settings.K2TrackersVector[2].pose_orientation);

					// Kind of a solution for flipping at too big X.
					// Found out during testing,
					// no other known mathematical reason (maybe except gimbal lock)

					// ------------------------------------------
					if (right_ori_vector.y() <= 0.f
						&& right_ori_vector.y() >= -1.f

						&& right_ori_vector.z() <= -1.f
						&& right_ori_vector.z() >= -_PI)

						right_ori_vector.y() += -_PI;
					// ------------------------------------------

					// Apply to the base
					K2Settings.K2TrackersVector[2].pose_orientation =
						EigenUtils::EulersToQuat(right_ori_vector);
					*/
				}
			}
		}
	}
}
