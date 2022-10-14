#pragma once
#include "K2Interfacing.h"
#include "TrackingDevices.h"

namespace TrackingDevices::Math
{

	// The upper bound of the fog volume along the y-axis (height)
	// This defines how far up the fog extends from the floor plane
	// This is hard coded because all V2 sensors are the same (afaik, I don't know if the fog height changes depending on room conditions though)
	constexpr double SOFTWARE_CALCULATED_ROTATION_FOG_THRESHOLD = 0.4;

	inline double Lerp(double from, double to, const double delta) {
		return from * (1 - delta) + to * delta;
	}

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
			const auto& _joints = _kinect->getTrackedJoints();

			// Placeholders for mathbased rots
			Eigen::Quaterniond calculatedLeftFootOrientation, calculatedRightFootOrientation;

			// Capture needed joints' positions
			Eigen::Vector3d forward(0, 0, 1),
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
			Eigen::Vector3d
				footLeftRawOrientation = EigenUtils::DirectionQuat(
					Eigen::Vector3d(ankleLeftPose.x(), 0.f, ankleLeftPose.z()),
					Eigen::Vector3d(footLeftPose.x(), 0.f, footLeftPose.z()),
					forward).toRotationMatrix().eulerAngles(0, 1, 2),

				footRightRawOrientation = EigenUtils::DirectionQuat(
					Eigen::Vector3d(ankleRightPose.x(), 0.f, ankleRightPose.z()),
					Eigen::Vector3d(footRightPose.x(), 0.f, footRightPose.z()),
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
			Eigen::Quaterniond
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
			Eigen::Quaterniond
				knee_ankleLeftOrientationQuaternion = EigenUtils::DirectionQuat(
					kneeLeftPose, ankleLeftPose, forward),
				knee_ankleRightOrientationQuaternion = EigenUtils::DirectionQuat(
					kneeRightPose, ankleRightPose, forward);

			// The tuning quat
			auto
				tuneQuaternion_first = Eigen::Quaterniond(1, 0, 0, 0);

			// Now adjust some values like playspace yaw and pitch, additional rotations
			// -> they're facing purely down and Z / Y are flipped
			tuneQuaternion_first =
				EigenUtils::EulersToQuat(
					Eigen::Vector3d(
						_PI / 5.f,
						0.f,
						0.f
					));

			// Apply the fine-tuning to global variable
			knee_ankleLeftOrientationQuaternion = tuneQuaternion_first * knee_ankleLeftOrientationQuaternion;
			knee_ankleRightOrientationQuaternion = tuneQuaternion_first * knee_ankleRightOrientationQuaternion;

			// Grab original orientations and make them euler angles
			Eigen::Vector3d left_knee_ori_full = EigenUtils::QuatToEulers(knee_ankleLeftOrientationQuaternion);
			Eigen::Vector3d right_knee_ori_full =
				EigenUtils::QuatToEulers(knee_ankleRightOrientationQuaternion);

			// Try to fix yaw and roll mismatch, caused by XYZ XZY mismatch
			knee_ankleLeftOrientationQuaternion = EigenUtils::EulersToQuat(
				Eigen::Vector3d(
					left_knee_ori_full.x() - _PI / 1.6f,
					0.0, // left_knee_ori_full.z(), // actually 0.0 but okay
					-left_knee_ori_full.y()));

			knee_ankleRightOrientationQuaternion = EigenUtils::EulersToQuat(
				Eigen::Vector3d(
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
				leftFootFineTuneQuaternion = Eigen::Quaterniond(1, 0, 0, 0),
				rightFootFineTuneQuaternion = Eigen::Quaterniond(1, 0, 0, 0);

			// Now adjust some values like playspace yaw and pitch, additional rotations

			leftFootFineTuneQuaternion =
				EigenUtils::EulersToQuat( // Lift trackers up a bit
					Eigen::Vector3d(
						2.8623399733f, // this one's in radians alr
						0.f, //glm::radians(KinectSettings::calibration_trackers_yaw),
						0.f
					));

			rightFootFineTuneQuaternion =
				EigenUtils::EulersToQuat( // Lift trackers up a bit
					Eigen::Vector3d(
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
					Eigen::Vector3d left_ori_vector = EigenUtils::QuatToEulers(
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
					Eigen::Vector3d right_ori_vector = EigenUtils::QuatToEulers(
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
			const auto& _joints = _kinect->getTrackedJoints();

			// Create placeholders for math-based rots
			auto calculatedLeftFootOrientation = Eigen::Quaterniond(1, 0, 0, 0);
			auto calculatedRightFootOrientation = Eigen::Quaterniond(1, 0, 0, 0);

			// Check if the tracking is valid
			if (_kinect->isSkeletonTracked())
			{
				// The improved approach for fixing foot rotation on the Xbox One Kinect
				// 
				// Thigh rotation is copied onto the foot, this is due to the fact that more often than not, the thigh
				// is facing the same direction as your foot. Given the foot is an unreliable mess due to the fog on
				// the Xbox One Kinect, 
				// 
				// The ankle position is stable though, so we can use it.
				{
					Eigen::Vector3d legsDir =
						_joints[base_flip ? ktvr::ITrackedJointType::Joint_KneeRight : ktvr::ITrackedJointType::Joint_KneeLeft].getJointPosition()
						- _joints[base_flip ? ktvr::ITrackedJointType::Joint_AnkleRight : ktvr::ITrackedJointType::Joint_AnkleLeft].getJointPosition();

					// Normalize the direction to have a length of 1
					legsDir.normalize();

					// tend towards 0 below the fog threshold
					// Remove the pitch entirely if within the fog area
					legsDir.y() =
						// smoothly interpolate between 0 and y^2 if below fog threshold
						Lerp(
							legsDir.y(),
							// from 0 to y^2 (where 1 is fog threshold)
							Lerp(0.0,
								legsDir.y() * legsDir.y(),
								// such that we remap the y direction relative to the threshold between 0 and 1
								std::max(std::min(SOFTWARE_CALCULATED_ROTATION_FOG_THRESHOLD, legsDir.y()), 0.0) / SOFTWARE_CALCULATED_ROTATION_FOG_THRESHOLD),
					
							// from 0.8 * fog threshold to 1.2 * fog threshold
							std::min(1.0, std::max(0.0, 0.4 * SOFTWARE_CALCULATED_ROTATION_FOG_THRESHOLD * legsDir.y() - 0.8 * SOFTWARE_CALCULATED_ROTATION_FOG_THRESHOLD)));

					// Normalize the direction to have a length of 1
					legsDir.normalize();

					Eigen::Vector3d from = Eigen::Vector3d::UnitX();
					Eigen::Vector3d base = Eigen::Vector3d::UnitX();

					calculatedLeftFootOrientation = EigenUtils::DirectionQuat(from, legsDir, base);
				}
				{
					Eigen::Vector3d legsDir =
						_joints[base_flip ? ktvr::ITrackedJointType::Joint_KneeLeft : ktvr::ITrackedJointType::Joint_KneeRight].getJointPosition()
						- _joints[base_flip ? ktvr::ITrackedJointType::Joint_AnkleLeft : ktvr::ITrackedJointType::Joint_AnkleRight].getJointPosition();

					// Normalize the direction to have a length of 1
					legsDir.normalize();

					// tend towards 0 below the fog threshold
					// Remove the pitch entirely if within the fog area
					legsDir.y() =
						// smoothly interpolate between 0 and y^2 if below fog threshold
						Lerp(
							legsDir.y(),
							// from 0 to y^2 (where 1 is fog threshold)
							Lerp(0.0,
								legsDir.y() * legsDir.y(),
								// such that we remap the y direction relative to the threshold between 0 and 1
								std::max(std::min(SOFTWARE_CALCULATED_ROTATION_FOG_THRESHOLD, legsDir.y()), 0.0) / SOFTWARE_CALCULATED_ROTATION_FOG_THRESHOLD),

							// from 0.8 * fog threshold to 1.2 * fog threshold
							std::min(1.0, std::max(0.0, 0.4 * SOFTWARE_CALCULATED_ROTATION_FOG_THRESHOLD * legsDir.y() - 0.8 * SOFTWARE_CALCULATED_ROTATION_FOG_THRESHOLD)));

					// Normalize the direction to have a length of 1
					legsDir.normalize();

					Eigen::Vector3d from = Eigen::Vector3d::UnitX();
					Eigen::Vector3d base = Eigen::Vector3d::UnitX();

					calculatedRightFootOrientation = EigenUtils::DirectionQuat(from, legsDir, base);
				}

				// Calculate the Left Foot?
				if (K2Settings.K2TrackersVector[1].orientationTrackingOption == k2_SoftwareCalculatedRotation_V2)
				{
					// Flip the orientation based on the host flip
					K2Settings.K2TrackersVector[1].pose_orientation =
						base_flip
							// If flip
							? ktvr::quaternion_normal(calculatedRightFootOrientation).inverse()

							// If no flip
							: ktvr::quaternion_normal(calculatedLeftFootOrientation);
				}

				// Calculate the Right Foot?
				if (K2Settings.K2TrackersVector[2].orientationTrackingOption == k2_SoftwareCalculatedRotation_V2)
				{
					// Flip the orientation based on the host flip
					K2Settings.K2TrackersVector[2].pose_orientation =
						base_flip
							// If flip
							? ktvr::quaternion_normal(calculatedLeftFootOrientation).inverse()

							// If no flip
							: ktvr::quaternion_normal(calculatedRightFootOrientation);
				}
			}
		}
	}
}
