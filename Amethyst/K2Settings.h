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

namespace k2app
{
	class K2AppSettings
	{
	private:
		friend class boost::serialization::access;

		template <class Archive>
		void serialize(Archive& archive, unsigned int version)
		{
			archive & BOOST_SERIALIZATION_NVP(K2TrackersVector)
				& BOOST_SERIALIZATION_NVP(useTrackerPairs)
				& BOOST_SERIALIZATION_NVP(checkForOverlappingTrackers)
				& BOOST_SERIALIZATION_NVP(trackingDeviceID)
				& BOOST_SERIALIZATION_NVP(overrideDeviceID)
				& BOOST_SERIALIZATION_NVP(isFlipEnabled)
				& BOOST_SERIALIZATION_NVP(isExternalFlipEnabled)
				& BOOST_SERIALIZATION_NVP(externalFlipCalibrationYaw)
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
				& BOOST_SERIALIZATION_NVP(shownToastsGuidVector);
		}

	public:
		/* Members part */

		// Current joints
		std::vector<K2AppTracker> K2TrackersVector;
		bool useTrackerPairs = true; // Pair feet, elbows and knees
		bool checkForOverlappingTrackers = true; // Check for overlapping roles

		// Current tracking device: 0 is the default
		uint32_t trackingDeviceID = 0; // -> Always set and >= 0
		int32_t overrideDeviceID = -1;

		// Skeleton flip when facing away: One-For-All and on is the default
		bool isFlipEnabled = true;

		// Skeleton flip based on non-flip override devices' waist tracker
		bool isExternalFlipEnabled = false;

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
			}
			catch (const boost::archive::archive_exception& e)
			{
				LOG(ERROR) << "Settings archive serialization error: " << e.what();
			}

			// Check if the trackers vector is broken
			const bool _vector_broken = K2TrackersVector.size() < 7;

			// Optionally fix the trackers vector
			while (K2TrackersVector.size() < 7)
				K2TrackersVector.push_back(K2AppTracker());

			// Force the first 7 trackers to be the default ones : roles
			K2TrackersVector.at(0).tracker = ktvr::ITrackerType::Tracker_Waist;
			K2TrackersVector.at(1).tracker = ktvr::ITrackerType::Tracker_LeftFoot;
			K2TrackersVector.at(2).tracker = ktvr::ITrackerType::Tracker_RightFoot;
			K2TrackersVector.at(3).tracker = ktvr::ITrackerType::Tracker_LeftElbow;
			K2TrackersVector.at(4).tracker = ktvr::ITrackerType::Tracker_RightElbow;
			K2TrackersVector.at(5).tracker = ktvr::ITrackerType::Tracker_LeftKnee;
			K2TrackersVector.at(6).tracker = ktvr::ITrackerType::Tracker_RightKnee;

			for (auto& tracker : K2TrackersVector)
			{
				// Force the first 7 trackers to be the default ones : serials
				tracker.data.serial = ITrackerType_Role_Serial[tracker.tracker];

				// Force disable software orientation if used by a non-foot
				if (tracker.tracker != ktvr::ITrackerType::Tracker_LeftFoot &&
					tracker.tracker != ktvr::ITrackerType::Tracker_RightFoot &&
					tracker.orientationTrackingOption == k2_SoftwareCalculatedRotation)
					tracker.orientationTrackingOption = k2_DeviceInferredRotation;
			}

			// If the vector was broken, override waist & feet statuses
			if (_vector_broken)
			{
				K2TrackersVector.at(0).data.isActive = true;
				K2TrackersVector.at(1).data.isActive = true;
				K2TrackersVector.at(2).data.isActive = true;
			}

			// Scan for duplicate trackers
			std::vector<ktvr::ITrackerType> _thisK2TrackerTypes;
			for (uint32_t _tracker_index = 0; _tracker_index < K2TrackersVector.size(); _tracker_index++)
				for (const auto& _tracker_type : _thisK2TrackerTypes)
					if (K2TrackersVector[_tracker_index].tracker == _tracker_type)
					{
						LOG(WARNING) << "A duplicate tracker was found in the trackers vector! Removing it...";
						K2TrackersVector.erase(K2TrackersVector.begin() + _tracker_index);
					}
					else _thisK2TrackerTypes.push_back(K2TrackersVector[_tracker_index].tracker);

			// Fix statuses (optional)
			if (useTrackerPairs)
			{
				K2TrackersVector.at(2).data.isActive =
					K2TrackersVector.at(1).data.isActive;
				K2TrackersVector.at(4).data.isActive =
					K2TrackersVector.at(3).data.isActive;
				K2TrackersVector.at(6).data.isActive =
					K2TrackersVector.at(5).data.isActive;
			}

			// Optionally fix volume if too big somehow
			appSoundsVolume = std::clamp(
				appSoundsVolume, static_cast<uint32_t>(0), static_cast<uint32_t>(100));

			// Optionally fix calibration points
			calibrationPointsNumber = std::clamp(
				calibrationPointsNumber, static_cast<uint32_t>(3), static_cast<uint32_t>(5));
		}
	} inline K2Settings;
}
