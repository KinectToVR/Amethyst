#pragma once
#include "pch.h"
#include "K2AppTracker.h"
#define degreesToRadians(angleDegrees) ((angleDegrees) * 3.14159265358979323846 / 180.0)
#define radiansToDegrees(angleRadians) ((angleRadians) * 180.0 / 3.14159265358979323846)

namespace k2app
{
	class K2AppSettings
	{
	private:
		friend class boost::serialization::access;

		template <class Archive>
		void serialize(Archive& archive, unsigned int version)
		{
			archive& BOOST_SERIALIZATION_NVP(trackingDeviceID)
				& BOOST_SERIALIZATION_NVP(overrideDeviceID)
				& BOOST_SERIALIZATION_NVP(selectedTrackedJointID)
				& BOOST_SERIALIZATION_NVP(positionOverrideJointID)
				& BOOST_SERIALIZATION_NVP(rotationOverrideJointID)
				& BOOST_SERIALIZATION_NVP(isPositionOverriddenJoint)
				& BOOST_SERIALIZATION_NVP(isRotationOverriddenJoint)
				& BOOST_SERIALIZATION_NVP(positionJointsOffsets)
				& BOOST_SERIALIZATION_NVP(rotationJointsOffsets)
				& BOOST_SERIALIZATION_NVP(jointRotationTrackingOption)
				& BOOST_SERIALIZATION_NVP(positionFilterOption)
				& BOOST_SERIALIZATION_NVP(rotationFilterOption)
				& BOOST_SERIALIZATION_NVP(isFlipEnabled);
		}

	public:
		/* Members part */

		// Current tracking device: 0 is the default
		uint32_t trackingDeviceID = 0; // -> Always set and >= 0
		int32_t overrideDeviceID = -1;

		// Joint tracking device selected joints: 0s are the defaults
		// On the first time refresh the joints are assigned like W0 L1 R2
		std::array<uint32_t, 3> // W,L,R -> always >= 0
		selectedTrackedJointID = {0, 0, 0};

		// Current override joints: W,L,R and 0 is the default for waist
		std::array<uint32_t, 3> // W,L,R -> always >= 0
			positionOverrideJointID = {0, 0, 0},
			rotationOverrideJointID = {0, 0, 0};

		// Current override joints: W,L,R and true is the default for waist
		std::array<bool, 3>
			isPositionOverriddenJoint = {true, false, false},
			isRotationOverriddenJoint = {true, false, false};

		// Joint offsets: W,L,R and pos/meters | rot/eulers(rad)
		std::array<Eigen::Vector3d, 3>
			positionJointsOffsets = {
				Eigen::Vector3d(0, 0, 0),
				Eigen::Vector3d(0, 0, 0),
				Eigen::Vector3d(0, 0, 0)
			},
			rotationJointsOffsets = {
				Eigen::Vector3d(0, 0, 0),
				Eigen::Vector3d(0, 0, 0),
				Eigen::Vector3d(0, 0, 0)
			};

		// Rotation tracking options: W,L,R and Internal is the default
		std::array<JointRotationTrackingOption, 3> jointRotationTrackingOption = {
			k2_DeviceInferredRotation,
			k2_DeviceInferredRotation,
			k2_DeviceInferredRotation
		};

		// Joint filter pos options: One-For-All and LERP is the default
		PositionTrackingFilterOption positionFilterOption = k2_PositionTrackingFilter_LERP;

		// Joint filter rot options: One-For-All and SLERP (normal) is the default
		RotationTrackingFilterOption rotationFilterOption = k2_OrientationTrackingFilter_SLERP;

		// Skeleton flip when facing away: One-For-All and on is the default
		bool isFlipEnabled = true;

		/* Saving and loading part */

		// Save settings with boost and output file stream
		void saveSettings()
		{
			try
			{
				std::ofstream output(ktvr::GetK2AppDataFileDir("KinectToVR_settings.xml"));

				boost::archive::xml_oarchive archive(output);
				archive << boost::serialization::make_nvp("K2AppSettings", *this);
				LOG(INFO) << "Settings have been saved to file \"KinectToVR_settings.xml\" (inside K2AppData)";
			}
			catch (boost::archive::archive_exception const& e)
			{
				LOG(ERROR) << "Settings archive serialization error: " << e.what();
			}
		}

		// Read class from input file stream
		void readSettings()
		{
			try
			{
				std::ifstream input(ktvr::GetK2AppDataFileDir("KinectToVR_settings.xml"));

				boost::archive::xml_iarchive archive(input);
				archive >> boost::serialization::make_nvp("K2AppSettings", *this);
				LOG(INFO) << "Settings have been saved to file \"KinectToVR_settings.xml\" (inside K2AppData)";
			}
			catch (boost::archive::archive_exception const& e)
			{
				LOG(ERROR) << "Settings archive serialization error: " << e.what();
			}
		}
	} inline K2Settings;
}
