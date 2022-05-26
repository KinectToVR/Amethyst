﻿#pragma once
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

class KinectV1Handler : public ktvr::K2TrackingDeviceBase_KinectBasis
{
	// A representation of the Kinect elements for the v1 api
public:
	KinectV1Handler()
	{
		//KinectV1Handler::initialize();
		LOG(INFO) << "Constructing the Kinect V1 (X360) Handler for KinectBasis K2TrackingDevice...";

		deviceType = ktvr::K2_Kinect;
		deviceName = "Xbox 360 Kinect";

		deviceCharacteristics = ktvr::K2_Character_Full;
		flipSupported = true;
		appOrientationSupported = true;

		// Mark that our device supports settings
		settingsSupported = true;
		load_settings(); // Load settings
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
		m_elevation_label = CreateTextBlock(L"Elevation angle:");
		m_elevation_spinner = CreateNumberBox(sensorAngle);

		m_message_text_block = CreateTextBlock(L"Please connect the Kinect first!");
		m_main_progress_bar = CreateProgressBar();

		// Set up elements
		m_main_progress_bar->Width(270);
		m_main_progress_bar->Progress(100);
		m_main_progress_bar->ShowPaused(true);

		m_elevation_spinner->Width(130);

		// Append the elements : Static Data
		layoutRoot->AppendElementPair(
			m_elevation_label, m_elevation_spinner);
		
		layoutRoot->AppendSingleElement(
			m_message_text_block,
			ktvr::Interface::SingleLayoutHorizontalAlignment::Center);

		layoutRoot->AppendSingleElement(
			m_main_progress_bar,
			ktvr::Interface::SingleLayoutHorizontalAlignment::Center);

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

		// Hide post-init ui elements
		m_elevation_label->Visibility(false);
		m_elevation_spinner->Visibility(false);
		_loaded = true;
	}

private:
	/* Device's own stuff */
	ktvr::Interface::TextBlock *m_elevation_label, *m_message_text_block;
	ktvr::Interface::NumberBox* m_elevation_spinner;
	ktvr::Interface::ProgressBar* m_main_progress_bar;
	bool _loaded = false;

	void save_settings()
	{
		if (std::ofstream output(
				ktvr::GetK2AppDataFileDir("Device_KinectV1_settings.xml"));
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
				ktvr::GetK2AppDataFileDir("Device_KinectV1_settings.xml"));
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
	// Return the device handler for tracking
	// but only if interfaces are the same / up-to-date
	if (0 == strcmp(ktvr::IK2API_Devices_Version, pVersionName))
	{
		static KinectV1Handler TrackingHandler; // Create a new device handler -> KinectV2

		*pReturnCode = ktvr::K2InitError_None;
		return &TrackingHandler;
	}

	// Return code for initialization
	*pReturnCode = ktvr::K2InitError_BadInterface;
}
