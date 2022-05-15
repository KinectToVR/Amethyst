#pragma once
#include "pch.h"
#include <Amethyst_API.h>

#include <fstream>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/serialization/export.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/optional.hpp>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include "K2AppTracker.h"
#define _PI 3.14159265358979323846

template <typename T>
T degreesToRadians(T angleDegrees)
{
	return angleDegrees * _PI / 180.0;
}

template <typename T>
T radiansToDegrees(T angleRadians)
{
	return angleRadians * 180.0 / _PI;
}

namespace boost::serialization
{
	// Eigen serialization
	template <class Archive, typename _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
	void serialize(Archive& ar,
	               Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols>& t,
	               const unsigned int file_version
	)
	{
		for (size_t i = 0; i < t.size(); i++)
			ar & make_nvp(("m" + std::to_string(i)).c_str(), t.data()[i]);
	}

	template <class Archive, typename _Scalar>
	void serialize(Archive& ar, Eigen::Quaternion<_Scalar>& q, unsigned)
	{
		ar & make_nvp("w", q.w())
			& make_nvp("x", q.x())
			& make_nvp("y", q.y())
			& make_nvp("z", q.z());
	}

	template <class Archive, typename _Scalar>
	void serialize(Archive& ar, Eigen::Vector3<_Scalar>& v, unsigned)
	{
		ar & make_nvp("x", v.x())
			& make_nvp("y", v.y())
			& make_nvp("z", v.z());
	}
}

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
				& BOOST_SERIALIZATION_NVP(positionTrackingFilterOptions)
				& BOOST_SERIALIZATION_NVP(rotationFilterOption)
				& BOOST_SERIALIZATION_NVP(isFlipEnabled)
				& BOOST_SERIALIZATION_NVP(isExternalFlipEnabled)
				& BOOST_SERIALIZATION_NVP(externalFlipCalibrationYaw)
				& BOOST_SERIALIZATION_NVP(isJointPairEnabled)
				& BOOST_SERIALIZATION_NVP(autoSpawnEnabledJoints)
				& BOOST_SERIALIZATION_NVP(enableAppSounds)
				& BOOST_SERIALIZATION_NVP(appSoundsVolume)
				& BOOST_SERIALIZATION_NVP(isMatrixCalibrated)
				& BOOST_SERIALIZATION_NVP(calibrationRotationMatrices)
				& BOOST_SERIALIZATION_NVP(calibrationTranslationVectors)
				& BOOST_SERIALIZATION_NVP(calibrationOrigins)
				& BOOST_SERIALIZATION_NVP(calibrationYaws)
				& BOOST_SERIALIZATION_NVP(calibrationPitches)
				& BOOST_SERIALIZATION_NVP(calibrationPointsNumber)
				& BOOST_SERIALIZATION_NVP(autoCalibration)
				& BOOST_SERIALIZATION_NVP(skeletonPreviewEnabled)
				& BOOST_SERIALIZATION_NVP(forceSkeletonPreview)
				& BOOST_SERIALIZATION_NVP(freezeLowerOnly)
				& BOOST_SERIALIZATION_NVP(shownToastsGuidVector)
				& BOOST_SERIALIZATION_NVP(ratingRemainingSessions)
				& BOOST_SERIALIZATION_NVP(ratingRemainingElapsedSessions);
		}

	public:
		/* Members part */

		// Current tracking device: 0 is the default
		uint32_t trackingDeviceID = 0; // -> Always set and >= 0
		int32_t overrideDeviceID = -1;

		// Joint tracking device selected joints: 0s are the defaults
		// On the first time refresh the joints are assigned like W0 L1 R2
		std::array<uint32_t, 7> // W,L,R -> always >= 0
		selectedTrackedJointID = {0, 0, 0, 0, 0, 0, 0};

		// Waist, Left Foot, Right Foot,
		// Left Elbow, Right Elbow, Left Knee, Right Knee

		// Current override joints: W,L,R and 0 is the default for waist
		std::array<uint32_t, 7> // W,L,R -> always >= 0
			positionOverrideJointID = {0, 0, 0, 0, 0, 0, 0},
			rotationOverrideJointID = {0, 0, 0, 0, 0, 0, 0};

		// Current override joints: W,L,R and true is the default for waist
		std::array<bool, 7>
			isPositionOverriddenJoint = {true, false, false, false, false, false, false},
			isRotationOverriddenJoint = {true, false, false, false, false, false, false};

		// Joint offsets: W,L,R and pos/meters | rot/eulers(rad)
		std::array<Eigen::Vector3d, 7>
			positionJointsOffsets = {
				Eigen::Vector3d(0, 0, 0),
				Eigen::Vector3d(0, 0, 0),
				Eigen::Vector3d(0, 0, 0),
				Eigen::Vector3d(0, 0, 0),
				Eigen::Vector3d(0, 0, 0),
				Eigen::Vector3d(0, 0, 0),
				Eigen::Vector3d(0, 0, 0)
			},
			rotationJointsOffsets = {
				Eigen::Vector3d(0, 0, 0),
				Eigen::Vector3d(0, 0, 0),
				Eigen::Vector3d(0, 0, 0),
				Eigen::Vector3d(0, 0, 0),
				Eigen::Vector3d(0, 0, 0),
				Eigen::Vector3d(0, 0, 0),
				Eigen::Vector3d(0, 0, 0)
			};

		// Rotation tracking options: W,L,R and Internal is the default
		std::array<JointRotationTrackingOption, 7> jointRotationTrackingOption = {
			k2_DeviceInferredRotation,
			k2_DeviceInferredRotation,
			k2_DeviceInferredRotation,
			k2_DeviceInferredRotation,
			k2_DeviceInferredRotation,
			k2_DeviceInferredRotation,
			k2_DeviceInferredRotation
		};

		// Joint filter pos options: LERP is the default
		std::array<PositionTrackingFilterOption, 7> positionTrackingFilterOptions = {
			k2_PositionTrackingFilter_LERP,
			k2_PositionTrackingFilter_LERP,
			k2_PositionTrackingFilter_LERP,
			k2_PositionTrackingFilter_LERP,
			k2_PositionTrackingFilter_LERP,
			k2_PositionTrackingFilter_LERP,
			k2_PositionTrackingFilter_LERP
		};

		// Joint filter rot options: One-For-All and SLERP (normal) is the default
		RotationTrackingFilterOption rotationFilterOption = k2_OrientationTrackingFilter_SLERP;

		// Skeleton flip when facing away: One-For-All and on is the default
		bool isFlipEnabled = true;

		// Skeleton flip based on non-flip override devices' waist tracker
		bool isExternalFlipEnabled = false;

		// Currently enabled (spawn-able) joints: W; L,R and true is the default, LE,RE; LK,RK
		std::array<bool, 7>
		isJointPairEnabled = {true, true, false, false};

		// Automatically spawn enabled trackers on startup and off is the default
		bool autoSpawnEnabledJoints = false;

		// Enable application sounds and on is the default
		bool enableAppSounds = true;

		// App sounds' volume and *nice* is the default
		uint32_t appSoundsVolume = 69; // Always 0<x<100

		// Calibration - if we're calibrated
		std::pair<bool, bool> isMatrixCalibrated{false, false};

		// Calibration matrices : Base, Override
		std::pair<Eigen::Matrix<double, 3, 3>, Eigen::Matrix<double, 3, 3>> calibrationRotationMatrices;
		std::pair<Eigen::Matrix<double, 1, 3>, Eigen::Matrix<double, 1, 3>> calibrationTranslationVectors;
		std::pair<Eigen::Vector3d, Eigen::Vector3d> calibrationOrigins; // always 0,0,0 for auto
		std::pair<double, double> calibrationYaws{0., 0.};
		std::pair<double, double> calibrationPitches{0., 0.};

		// Calibration helpers - points number
		uint32_t calibrationPointsNumber = 3; // Always 3<=x<=5
		// Calibration helpers - calibration method: auto?
		std::pair<bool, bool> autoCalibration{false, false};

		// Save the skeleton preview state
		bool skeletonPreviewEnabled = true;
		// If we wanna dismiss all warnings during the preview
		bool forceSkeletonPreview = false;

		// External flip device's calibration yaw
		double externalFlipCalibrationYaw = 0.;

		// If we wanna freeze only lower body trackers or all
		bool freezeLowerOnly = false;

		// Already shown toasts vector
		std::vector<std::string> shownToastsGuidVector;

		// Sessions to wait for a rating notice
		int32_t ratingRemainingSessions = 7; // Start with 7, -1 to disable
		// Sessions elapsed without a rating notice
		int32_t ratingRemainingElapsedSessions = 0; // Start with 0 (not uint tho)

		/* Saving and loading part */

		// Save settings with boost and output file stream
		void saveSettings()
		{
			try
			{
				std::ofstream output(ktvr::GetK2AppDataFileDir("Amethyst_settings.xml"));

				boost::archive::xml_oarchive archive(output);
				archive << boost::serialization::make_nvp("K2AppSettings", *this);
				LOG(INFO) << "Settings have been saved to file \"Amethyst_settings.xml\" (inside K2AppData)";
			}
			catch (const boost::archive::archive_exception& e)
			{
				LOG(ERROR) << "Settings archive serialization error: " << e.what();
			}
		}

		// Read class from input file stream
		void readSettings()
		{
			try
			{
				std::ifstream input(ktvr::GetK2AppDataFileDir("Amethyst_settings.xml"));

				boost::archive::xml_iarchive archive(input);
				archive >> boost::serialization::make_nvp("K2AppSettings", *this);
				LOG(INFO) << "Settings have been read from file \"Amethyst_settings.xml\" (inside K2AppData)";

				// Optionally fix volume if too big somehow
				appSoundsVolume = std::clamp(
					appSoundsVolume, static_cast<uint32_t>(0), static_cast<uint32_t>(100));

				// Optionally fix calibration points
				calibrationPointsNumber = std::clamp(
					calibrationPointsNumber, static_cast<uint32_t>(3), static_cast<uint32_t>(5));

				// Optionally fix rating sessions
				ratingRemainingElapsedSessions = std::clamp(
					ratingRemainingElapsedSessions, 0, INT32_MAX);
			}
			catch (const boost::archive::archive_exception& e)
			{
				LOG(ERROR) << "Settings archive serialization error: " << e.what();
			}
		}
	} inline K2Settings;
}
