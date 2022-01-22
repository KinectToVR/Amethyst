#pragma once
#include "pch.h"
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
			archive & BOOST_SERIALIZATION_NVP(trackingDeviceID)
				& BOOST_SERIALIZATION_NVP(overrideDeviceID)
				& BOOST_SERIALIZATION_NVP(selectedTrackedJointID)
				& BOOST_SERIALIZATION_NVP(positionOverrideJointID)
				& BOOST_SERIALIZATION_NVP(rotationOverrideJointID)
				& BOOST_SERIALIZATION_NVP(isPositionOverriddenJoint)
				& BOOST_SERIALIZATION_NVP(isRotationOverriddenJoint);
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
		std::array<Eigen::Vector3f, 3>
			positionJointsOffsets = {
				Eigen::Vector3f(0, 0, 0),
				Eigen::Vector3f(0, 0, 0),
				Eigen::Vector3f(0, 0, 0)
			},
			rotationJointsOffsets = {
				Eigen::Vector3f(0, 0, 0),
				Eigen::Vector3f(0, 0, 0),
				Eigen::Vector3f(0, 0, 0)
			};

		/* Saving and loading part */

		// Save settings with boost and output file stream
		void saveSettings()
		{
			try
			{
				std::ofstream output(ktvr::GetK2AppDataFileDir("KinectToVR_settings.xml"));

				boost::archive::xml_oarchive archive(output);
				archive << BOOST_SERIALIZATION_NVP(*this);
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
				archive >> BOOST_SERIALIZATION_NVP(*this);
				LOG(INFO) << "Settings have been saved to file \"KinectToVR_settings.xml\" (inside K2AppData)";
			}
			catch (boost::archive::archive_exception const& e)
			{
				LOG(ERROR) << "Settings archive serialization error: " << e.what();
			}
		}
	} inline K2Settings;
}
