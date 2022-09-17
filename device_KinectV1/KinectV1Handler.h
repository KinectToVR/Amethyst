#pragma once
#include <Windows.h>
#include <Ole2.h>
#include <NuiApi.h>
#include <NuiImageCamera.h>
#include <NuiSensor.h>
#include <NuiSkeleton.h>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include "Amethyst_API_Devices.h"
#include "Amethyst_API_Paths.h"

#include <cereal/types/unordered_map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/xml.hpp>

#include <fstream>
#include <thread>

/* Not exported */

class KinectV1Handler : public ktvr::K2TrackingDeviceBase_SkeletonBasis
{
	// A representation of the Kinect elements for the v1 api
public:
	KinectV1Handler()
	{
		//KinectV1Handler::initialize();
		LOG(INFO) << "Constructing the Kinect V1 (X360) Handler for SkeletonBasis K2TrackingDevice...";
		
		deviceName = L"Xbox 360 Kinect";
		deviceCharacteristics = ktvr::K2_Character_Full;

		flipSupported = true;
		appOrientationSupported = true;

		// Mark that our device supports settings
		settingsSupported = false; // 'false' until status OK
		load_settings(); // Load settings
	}

	std::wstring getDeviceGUID() override
	{
		// This ID is unique to the official KV1 plugin!
		return L"K2VRTEAM-AME1-API1-DVCE-DVCEKINECTV1";
	}

	HANDLE kinectRGBStream = nullptr;
	HANDLE kinectDepthStream = nullptr;
	INuiSensor* kinectSensor = nullptr;
	NUI_SKELETON_FRAME skeletonFrame = {0};

	NUI_SKELETON_BONE_ORIENTATION boneOrientations[NUI_SKELETON_POSITION_COUNT];

	void initialize() override;
	void update() override;
	void shutdown() override;

	~KinectV1Handler() override
	{
	}

	HRESULT getStatusResult() override;
	std::wstring statusResultWString(HRESULT stat) override;

	void onLoad() override
	{
		// Construct the device's settings here
		if (getStatusResult() == S_OK)
		{
			NuiCameraElevationGetAngle(reinterpret_cast<long*>(&sensorAngle));
			save_settings();
		}

		// Create elements
		m_elevation_spinner = CreateNumberBox(sensorAngle);

		// Set up elements
		// m_elevation_spinner->Width(160);

		auto _text_block = CreateTextBlock(
			requestLocalizedString(L"/Plugins/KinectV1/Settings/Labels/Angle"));
		_text_block->IsPrimary(false);

		// Append the elements : Static Data
		layoutRoot->AppendElementPairStack(
			_text_block,
			m_elevation_spinner);

		// Set up particular handlers

		// "Full Calibration"
		m_elevation_spinner->OnValueChanged = [&, this](
			ktvr::Interface::NumberBox* sender, const int& new_value)
			{
				if (!initialized)return;

				sender->Value(std::clamp(new_value, -27, 27));
				sensorAngle = std::clamp(new_value, -27, 27);
				save_settings(); // Back everything up

				if (!angleSetPending)
					std::thread([&, this]
					{
						NuiCameraElevationSetAngle(sensorAngle);
						angleSetPending = false;
					}).detach();
			};

		_loaded = true;
	}

private:
	/* Device's own stuff */
	ktvr::Interface::NumberBox* m_elevation_spinner;
	bool _loaded = false;

	void save_settings()
	{
		if (std::ofstream output(
				ktvr::GetK2AppDataFileDir(L"Device_KinectV1_settings.xml"));
			output.fail())
		{
			LOG(ERROR) << "KinectV1 Device Error: Couldn't save settings!\n";
		}
		else
		{
			cereal::XMLOutputArchive archive(output);
			LOG(INFO) << "KinectV1 Device: Attempted to save settings";

			try
			{
				archive(
					CEREAL_NVP(sensorAngle)
				);
			}
			catch (...)
			{
				LOG(ERROR) << "KinectV1 Device Error: Couldn't save settings, an exception occurred!\n";
			}
		}
	}

	void load_settings()
	{
		if (std::ifstream input(
				ktvr::GetK2AppDataFileDir(L"Device_KinectV1_settings.xml"));
			input.fail())
		{
			LOG(WARNING) << "KinectV1 Device Error: Couldn't read settings, re-generating!\n";
			save_settings(); // Re-generate the file
		}
		else
		{
			LOG(INFO) << "KinectV1 Device: Attempting to read settings";

			try
			{
				cereal::XMLInputArchive archive(input);
				archive(
					//CEREAL_NVP(m_net_port),
					CEREAL_NVP(sensorAngle)
				);
			}
			catch (...)
			{
				LOG(ERROR) << "KinectV1 Device Error: Couldn't read settings, an exception occurred!\n";
			}
		}
	}

	int sensorAngle = 0;
	bool angleSetPending = false;

	bool initKinect();
	bool acquireKinectFrame(NUI_IMAGE_FRAME& imageFrame, HANDLE& rgbStream, INuiSensor*& sensor);
	void releaseKinectFrame(NUI_IMAGE_FRAME& imageFrame, HANDLE& rgbStream, INuiSensor*& sensor);

	void updateSkeletalData();

	/* For translating Kinect joint enumeration to K2 space */
	int globalIndex[25] = {3, 2, 2, 4, 5, 6, 7, 7, 7, 8, 9, 10, 11, 11, 11, 1, 0, 12, 13, 14, 15, 16, 17, 18, 19};
};

/* Exported for dynamic linking */
extern "C" __declspec(dllexport) void* TrackingDeviceBaseFactory(
	const char* pVersionName, int* pReturnCode)
{
	LOG(INFO) << "[KinectV1 Device] Interface version name: " << pVersionName;
	LOG(INFO) << "[KinectV1 Device] Amethyst API version name: " << ktvr::IAME_API_Devices_Version;

	// Return the device handler for tracking
	// but only if interfaces are the same / up-to-date
	if (0 == strcmp(ktvr::IAME_API_Devices_Version, pVersionName))
	{
		static KinectV1Handler TrackingHandler; // Create a new device handler -> KinectV2

		*pReturnCode = ktvr::K2InitError_None;
		return &TrackingHandler;
	}

	// Return code for initialization
	*pReturnCode = ktvr::K2InitError_BadInterface;
}
