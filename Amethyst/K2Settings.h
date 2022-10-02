#pragma once
#include "pch.h"

#include <Amethyst_API.h>
#include <fstream>

#include "K2EVRInput.h"
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
	public:
		/* Members part */

		// Current language & theme
		std::wstring appLanguage;

		// 0:system, 1:dark, 2:light
		uint32_t appTheme = 0;

		// Current joints
		std::vector<K2AppTracker> K2TrackersVector;
		bool useTrackerPairs = true; // Pair feet, elbows and knees
		bool checkForOverlappingTrackers = true; // Check for overlapping roles

		// Current tracking device: 0 is the default base device
		// First: Device's GUID / saved, Second: Index ID / generated
		std::pair<std::wstring, uint32_t> trackingDeviceGUIDPair; // -> Always set and >= 0
		std::map<std::wstring, uint32_t> overrideDeviceGUIDsMap;
		
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
		std::map<std::wstring, bool> deviceMatricesCalibrated;

		// Calibration matrices : GUID/Data
		std::map<std::wstring, Eigen::Matrix<double, 3, 3>> deviceCalibrationRotationMatrices;
		std::map<std::wstring, Eigen::Matrix<double, 1, 3>> deviceCalibrationTranslationVectors;
		std::map<std::wstring, Eigen::Vector3d> deviceCalibrationOrigins; // always 0,0,0 for auto
		std::map<std::wstring, double> deviceCalibrationYaws;

		// Calibration helpers - calibration method: auto? : GUID/Data
		std::map<std::wstring, bool> deviceAutoCalibration;

		// Calibration helpers - points number
		uint32_t calibrationPointsNumber = 3; // Always 3<=x<=5

		// Save the skeleton preview state
		bool skeletonPreviewEnabled = true;
		// If we wanna dismiss all warnings during the preview
		bool forceSkeletonPreview = false;

		// External flip device's calibration yaw
		double externalFlipCalibrationYaw = 0.;

		// If we wanna freeze only lower body trackers or all
		bool freezeLowerOnly = false;

		// If the freeze bindings teaching tip has been shown
		bool teachingTipShown_Freeze = false;

		// If the flip bindings teaching tip has been shown
		bool teachingTipShown_Flip = false;

		// Already shown toasts vector
		std::vector<std::wstring> shownToastsGuidVector;

		// Disabled (by the user) devices set
		std::set<std::wstring> disabledDevicesGuidSet;

		// If the first-launch guide's been shown
		bool firstTimeTourShown = false;

		// If the shutdown warning has been shown
		bool firstShutdownTipShown = false;

		/* Saving and loading part */

		// Save settings with boost and output file stream
		void saveSettings()
		{
			try
			{
				if (std::ofstream output(
						ktvr::GetK2AppDataFileDir(L"Amethyst_settings.xml"));
					output.fail())
				{
					LOG(ERROR) << "Settings archive serialization error: Couldn't save settings!\n";
				}
				else
				{
					cereal::XMLOutputArchive archive(output);
					LOG(INFO) << "Attempting to save settings";

					archive(
						CEREAL_NVP(appLanguage),
						CEREAL_NVP(appTheme),
						CEREAL_NVP(K2TrackersVector),
						CEREAL_NVP(useTrackerPairs),
						CEREAL_NVP(checkForOverlappingTrackers),
						CEREAL_NVP(trackingDeviceGUIDPair),
						CEREAL_NVP(overrideDeviceGUIDsMap),
						CEREAL_NVP(isFlipEnabled),
						CEREAL_NVP(isExternalFlipEnabled),
						CEREAL_NVP(externalFlipCalibrationYaw),
						CEREAL_NVP(autoSpawnEnabledJoints),
						CEREAL_NVP(enableAppSounds),
						CEREAL_NVP(appSoundsVolume),
						CEREAL_NVP(deviceMatricesCalibrated),
						CEREAL_NVP(deviceCalibrationRotationMatrices),
						CEREAL_NVP(deviceCalibrationTranslationVectors),
						CEREAL_NVP(deviceCalibrationOrigins),
						CEREAL_NVP(deviceCalibrationYaws),
						CEREAL_NVP(calibrationPointsNumber),
						CEREAL_NVP(deviceAutoCalibration),
						CEREAL_NVP(skeletonPreviewEnabled),
						CEREAL_NVP(forceSkeletonPreview),
						CEREAL_NVP(freezeLowerOnly),
						CEREAL_NVP(shownToastsGuidVector),
						CEREAL_NVP(disabledDevicesGuidSet),
						CEREAL_NVP(teachingTipShown_Freeze),
						CEREAL_NVP(teachingTipShown_Flip),
						CEREAL_NVP(firstTimeTourShown),
						CEREAL_NVP(firstShutdownTipShown)
					);

					LOG(INFO) << "Settings have been saved to file \"Amethyst_settings.xml\" (inside K2AppData)";
				}
			}
			catch (const std::exception& e)
			{
				LOG(ERROR) << "Settings archive serialization error: " << e.what();
			}
		}

		// Read class from input file stream
		void readSettings()
		{
			try
			{
				if (std::ifstream input(
						ktvr::GetK2AppDataFileDir(L"Amethyst_settings.xml"));
					input.fail())
				{
					LOG(WARNING) << "Settings archive serialization error: Couldn't read settings, re-generating!\n";
					saveSettings(); // Re-generate the file
				}
				else
				{
					LOG(INFO) << "Attempting to read settings";

					cereal::XMLInputArchive archive(input);
					archive(
						CEREAL_NVP(appLanguage),
						CEREAL_NVP(appTheme),
						CEREAL_NVP(K2TrackersVector),
						CEREAL_NVP(useTrackerPairs),
						CEREAL_NVP(checkForOverlappingTrackers),
						CEREAL_NVP(trackingDeviceGUIDPair),
						CEREAL_NVP(overrideDeviceGUIDsMap),
						CEREAL_NVP(isFlipEnabled),
						CEREAL_NVP(isExternalFlipEnabled),
						CEREAL_NVP(externalFlipCalibrationYaw),
						CEREAL_NVP(autoSpawnEnabledJoints),
						CEREAL_NVP(enableAppSounds),
						CEREAL_NVP(appSoundsVolume),
						CEREAL_NVP(deviceMatricesCalibrated),
						CEREAL_NVP(deviceCalibrationRotationMatrices),
						CEREAL_NVP(deviceCalibrationTranslationVectors),
						CEREAL_NVP(deviceCalibrationOrigins),
						CEREAL_NVP(deviceCalibrationYaws),
						CEREAL_NVP(calibrationPointsNumber),
						CEREAL_NVP(deviceAutoCalibration),
						CEREAL_NVP(skeletonPreviewEnabled),
						CEREAL_NVP(forceSkeletonPreview),
						CEREAL_NVP(freezeLowerOnly),
						CEREAL_NVP(shownToastsGuidVector),
						CEREAL_NVP(disabledDevicesGuidSet),
						CEREAL_NVP(teachingTipShown_Freeze),
						CEREAL_NVP(teachingTipShown_Flip),
						CEREAL_NVP(firstTimeTourShown),
						CEREAL_NVP(firstShutdownTipShown)
					);

					LOG(INFO) << "Settings have been read from file \"Amethyst_settings.xml\" (inside K2AppData)";
				}
			}
			catch (const std::exception& e)
			{
				LOG(ERROR) << "Settings archive serialization error: " << e.what();
			}

			// Check if the trackers vector is broken
			const bool _vector_broken = K2TrackersVector.size() < 7;

			// Optionally fix the trackers vector
			while (K2TrackersVector.size() < 7)
				K2TrackersVector.push_back(K2AppTracker());

			// Force the first 7 trackers to be the default ones : roles
			K2TrackersVector.at(0).base_tracker = ktvr::ITrackerType::Tracker_Waist;
			K2TrackersVector.at(1).base_tracker = ktvr::ITrackerType::Tracker_LeftFoot;
			K2TrackersVector.at(2).base_tracker = ktvr::ITrackerType::Tracker_RightFoot;
			K2TrackersVector.at(3).base_tracker = ktvr::ITrackerType::Tracker_LeftElbow;
			K2TrackersVector.at(4).base_tracker = ktvr::ITrackerType::Tracker_RightElbow;
			K2TrackersVector.at(5).base_tracker = ktvr::ITrackerType::Tracker_LeftKnee;
			K2TrackersVector.at(6).base_tracker = ktvr::ITrackerType::Tracker_RightKnee;

			for (auto& tracker : K2TrackersVector)
			{
				// Force the first 7 trackers to be the default ones : serials
				tracker.data_serial = ITrackerType_Role_Serial[tracker.base_tracker];

				// Force disable software orientation if used by a non-foot
				if (tracker.base_tracker != ktvr::ITrackerType::Tracker_LeftFoot &&
					tracker.base_tracker != ktvr::ITrackerType::Tracker_RightFoot &&
					(tracker.orientationTrackingOption == k2_SoftwareCalculatedRotation ||
						tracker.orientationTrackingOption == k2_SoftwareCalculatedRotation_V2))
					tracker.orientationTrackingOption = k2_DeviceInferredRotation;
			}

			// If the vector was broken, override waist & feet statuses
			if (_vector_broken)
			{
				K2TrackersVector.at(0).data_isActive = true;
				K2TrackersVector.at(1).data_isActive = true;
				K2TrackersVector.at(2).data_isActive = true;
			}

			// Scan for duplicate trackers
			std::vector<ktvr::ITrackerType> _thisK2TrackerTypes;
			for (uint32_t _tracker_index = 0; _tracker_index < K2TrackersVector.size(); _tracker_index++)
				for (const auto& _tracker_type : _thisK2TrackerTypes)
					if (K2TrackersVector[_tracker_index].base_tracker == _tracker_type)
					{
						LOG(WARNING) << "A duplicate tracker was found in the trackers vector! Removing it...";
						K2TrackersVector.erase(K2TrackersVector.begin() + _tracker_index);
					}
					else _thisK2TrackerTypes.push_back(K2TrackersVector[_tracker_index].base_tracker);

			// Check if any trackers are enabled
			// No std::ranges today...
			bool _find_result = false;
			for (const auto& tracker : K2TrackersVector)
				if (tracker.data_isActive)_find_result = true;

			// No trackers are enabled, force-enable the waist tracker
			if (!_find_result)
			{
				LOG(WARNING) << "All trackers have been disabled, force-enabling the waist tracker!";

				// Enable the waist tracker
				K2TrackersVector[0].data_isActive = true;
			}

			// Fix statuses (optional)
			if (useTrackerPairs)
			{
				K2TrackersVector.at(2).data_isActive =
					K2TrackersVector.at(1).data_isActive;
				K2TrackersVector.at(4).data_isActive =
					K2TrackersVector.at(3).data_isActive;
				K2TrackersVector.at(6).data_isActive =
					K2TrackersVector.at(5).data_isActive;

				K2TrackersVector.at(2).orientationTrackingOption =
					K2TrackersVector.at(1).orientationTrackingOption;
				K2TrackersVector.at(4).orientationTrackingOption =
					K2TrackersVector.at(3).orientationTrackingOption;
				K2TrackersVector.at(6).orientationTrackingOption =
					K2TrackersVector.at(5).orientationTrackingOption;

				K2TrackersVector.at(2).positionTrackingFilterOption =
					K2TrackersVector.at(1).positionTrackingFilterOption;
				K2TrackersVector.at(4).positionTrackingFilterOption =
					K2TrackersVector.at(3).positionTrackingFilterOption;
				K2TrackersVector.at(6).positionTrackingFilterOption =
					K2TrackersVector.at(5).positionTrackingFilterOption;

				K2TrackersVector.at(2).orientationTrackingFilterOption =
					K2TrackersVector.at(1).orientationTrackingFilterOption;
				K2TrackersVector.at(4).orientationTrackingFilterOption =
					K2TrackersVector.at(3).orientationTrackingFilterOption;
				K2TrackersVector.at(6).orientationTrackingFilterOption =
					K2TrackersVector.at(5).orientationTrackingFilterOption;
			}

			// Optionally fix volume if too big somehow
			appSoundsVolume = std::clamp(
				appSoundsVolume, static_cast<uint32_t>(0), static_cast<uint32_t>(100));

			// Optionally fix calibration points
			calibrationPointsNumber = std::clamp(
				calibrationPointsNumber, static_cast<uint32_t>(3), static_cast<uint32_t>(5));

			// Optionally fix the app theme value
			appTheme = std::clamp(
				appTheme, static_cast<uint32_t>(0), static_cast<uint32_t>(2));

			/* Optionally fix the selected language / select a new one */
			std::filesystem::path resource_path =
				interfacing::GetProgramLocation().parent_path() /
				"Assets" / "Strings" / (appLanguage + L".json");

			// If there's no specified language, fallback to {system}
			if (appLanguage.empty())
			{
				appLanguage = std::wstring(
					winrt::Windows::Globalization::Language(
						winrt::Windows::System::UserProfile::
						GlobalizationPreferences::Languages()
						.GetAt(0)).LanguageTag()).substr(0, 2);

				LOG(WARNING) << "No language specified! Trying with the system one: \"" <<
					WStringToString(appLanguage) << "\"!";

				resource_path = interfacing::GetProgramLocation().parent_path() /
					"Assets" / "Strings" / (appLanguage + L".json");
			}

			// If the specified language doesn't exist somehow, fallback to 'en'
			if (!exists(resource_path))
			{
				LOG(WARNING) << "Could not load language resources at \"" <<
					WStringToString(resource_path.wstring()) << "\", falling back to 'en' (en.json)!";

				appLanguage = L"en"; // Change to english

				resource_path = interfacing::GetProgramLocation().parent_path() /
					"Assets" / "Strings" / (appLanguage + L".json");
			}

			// If failed again, just give up
			if (!exists(resource_path))
			{
				LOG(ERROR) << "Could not load language resources at \"" <<
					WStringToString(resource_path.wstring()) << "\", the app interface will be broken!";
			}
		}

	protected:
		[[nodiscard]] virtual bool equals(const K2AppSettings& other) const
		{
			return
				appLanguage == other.appLanguage &&
				appTheme == other.appTheme &&
				K2TrackersVector == other.K2TrackersVector &&
				useTrackerPairs == other.useTrackerPairs &&
				checkForOverlappingTrackers == other.checkForOverlappingTrackers &&
				trackingDeviceGUIDPair == other.trackingDeviceGUIDPair &&
				overrideDeviceGUIDsMap == other.overrideDeviceGUIDsMap &&
				isFlipEnabled == other.isFlipEnabled &&
				isExternalFlipEnabled == other.isExternalFlipEnabled &&
				externalFlipCalibrationYaw == other.externalFlipCalibrationYaw &&
				autoSpawnEnabledJoints == other.autoSpawnEnabledJoints &&
				enableAppSounds == other.enableAppSounds &&
				appSoundsVolume == other.appSoundsVolume &&
				deviceMatricesCalibrated == other.deviceMatricesCalibrated &&
				deviceCalibrationRotationMatrices == other.deviceCalibrationRotationMatrices &&
				deviceCalibrationTranslationVectors == other.deviceCalibrationTranslationVectors &&
				deviceCalibrationOrigins == other.deviceCalibrationOrigins &&
				deviceCalibrationYaws == other.deviceCalibrationYaws &&
				calibrationPointsNumber == other.calibrationPointsNumber &&
				deviceAutoCalibration == other.deviceAutoCalibration &&
				skeletonPreviewEnabled == other.skeletonPreviewEnabled &&
				forceSkeletonPreview == other.forceSkeletonPreview &&
				freezeLowerOnly == other.freezeLowerOnly &&
				shownToastsGuidVector == other.shownToastsGuidVector &&
				disabledDevicesGuidSet == other.disabledDevicesGuidSet &&
				teachingTipShown_Freeze == other.teachingTipShown_Freeze &&
				teachingTipShown_Flip == other.teachingTipShown_Flip &&
				firstTimeTourShown == other.firstTimeTourShown &&
				firstShutdownTipShown == other.firstShutdownTipShown;
		}

	public:
		friend bool operator==(const K2AppSettings& a, const K2AppSettings& b)
		{
			return a.equals(b);
		}
	} inline K2Settings;
}
