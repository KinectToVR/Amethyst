#pragma once
#include <Windows.h>
#include <PSMoveClient_CAPI.h>
#include <ClientConstants.h>
#include <SharedConstants.h>
#include <glog/logging.h>

#include "Amethyst_API_Devices.h"
#include "Amethyst_API_Paths.h"

#include <cereal/types/unordered_map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/xml.hpp>
#include <fstream>

/* Errors */
#define FACILITY_PSMS 0x301
#define E_PSMS_NOT_RUNNING			MAKE_HRESULT(SEVERITY_ERROR, FACILITY_PSMS, 2)
#define E_PSMS_NO_JOINTS			MAKE_HRESULT(SEVERITY_ERROR, FACILITY_PSMS, 3)

/* Not exported */

class PSMoveServiceHandler : public ktvr::K2TrackingDeviceBase_JointsBasis
{
	// A representation of the Kinect elements for the v1 api
public:
	PSMoveServiceHandler()
	{
		// Called from the app now
		//PSMoveServiceHandler::initialize();
		LOG(INFO) << "Constructing the PSMS Handler for JointsBasis K2TrackingDevice...";

		deviceType = ktvr::K2_Joints;
		deviceName = "PSMove Service";
		settingsSupported = true; // Until the status is OK

		// Push the placeholders in
		for (size_t i = 0; i < PSMOVESERVICE_MAX_CONTROLLER_COUNT; i++)
			trackedJoints.push_back(ktvr::K2TrackedJoint("INVALID " + std::to_string(i)));
	}

	void initialize() override;
	void update() override;
	void shutdown() override;
	void signalJoint(uint32_t at) override;

	~PSMoveServiceHandler() override
	{
	}

	void onLoad() override
	{
		// Read the settings
		load_settings();
		update_lights(m_lightsOff);

		layoutRoot->AppendSingleElement(
			CreateTextBlock(
				requestLocalizedString(L"/Plugins/PSMS/Settings/Labels/Dim")));

		auto lights_toggle = CreateToggleSwitch();
		lights_toggle->IsChecked(m_lightsOff); // Read from settings

		// Lights off
		lights_toggle->OnChecked = [&, this](auto)
		{
			m_lightsOff = true;
			update_lights(m_lightsOff);

			// Save
			save_settings();
		};

		// Lights on (reset by 0,0,0)
		lights_toggle->OnUnchecked = [&, this](auto)
		{
			m_lightsOff = false;
			update_lights(m_lightsOff);

			// Save
			save_settings();
		};

		layoutRoot->AppendElementPairStack(
			CreateTextBlock(
				L"Dim PSMS lights:"),
			lights_toggle);

		_loaded = true;
	}

	HRESULT getStatusResult() override;
	std::wstring statusResultWString(HRESULT stat) override;

private:
	bool _loaded = false;

	bool startup();
	void processKeyInputs();
	void rebuildPSMoveLists();
	void rebuildControllerList();

	void save_settings()
	{
		if (std::ofstream output(
				ktvr::GetK2AppDataFileDir("Device_PSMS_settings.xml"));
			output.fail())
		{
			LOG(ERROR) << "PSMS Device Error: Couldn't save settings!\n";
		}
		else
		{
			cereal::XMLOutputArchive archive(output);
			LOG(INFO) << "PSMS Device: Attempted to save settings";

			try
			{
				archive(CEREAL_NVP(m_lightsOff));
			}
			catch (...)
			{
				LOG(ERROR) << "PSMS Device Error: Couldn't save settings, an exception occurred!\n";
			}
		}
	}

	void load_settings()
	{
		if (std::ifstream input(
				ktvr::GetK2AppDataFileDir("Device_PSMS_settings.xml"));
			input.fail())
		{
			LOG(WARNING) << "PSMS Device Error: Couldn't read settings, re-generating!\n";
			save_settings(); // Re-generate the file
		}
		else
		{
			LOG(INFO) << "PSMS Device: Attempting to read settings";

			try
			{
				cereal::XMLInputArchive archive(input);
				archive(CEREAL_NVP(m_lightsOff));
			}
			catch (...)
			{
				LOG(ERROR) << "PSMS Device Error: Couldn't read settings, an exception occurred!\n";
			}
		}
	}

	void update_lights(const bool& off)
	{
		// They'll either be very dimmed (1,1,1) or standard (0,0,0)
		for (const auto& controller : v_controllers)
			PSM_SetControllerLEDOverrideColor(
				controller.controller->ControllerID, off, off, off);
	}

	std::chrono::milliseconds last_report_fps_timestamp;

	struct MoveWrapper_PSM
	{
		PSMController* controller = nullptr;
		Eigen::Quaternionf orientationOffset = Eigen::Quaternionf(1., 0., 0., 0.); // Recenter offset
		bool flashNow = false; // Signal
	};

	std::vector<MoveWrapper_PSM> v_controllers;

	PSMControllerList controllerList;
	PSMTrackerList trackerList;
	bool m_keepRunning = true;
	bool m_lightsOff = false;

	bool m_deviceUsable = false;
	bool m_needsRefresh = true;

	// Constants
	double finalPSMoveScale = 1.0;
	const std::string k_trackingSystemName = "psmove";
};

/* Exported for dynamic linking */
extern "C" __declspec(dllexport) void* TrackingDeviceBaseFactory(
	const char* pVersionName, int* pReturnCode)
{
	LOG(INFO) << "[PSMS Device] Interface version name: " << pVersionName;
	LOG(INFO) << "[PSMS Device] K2API version name: " << ktvr::IK2API_Devices_Version;

	// Return the device handler for tracking
	// but only if interfaces are the same / up-to-date
	if (0 == strcmp(ktvr::IK2API_Devices_Version, pVersionName))
	{
		static PSMoveServiceHandler TrackingHandler; // Create a new device handler -> KinectV2

		*pReturnCode = ktvr::K2InitError_None;
		return &TrackingHandler;
	}

	// Return code for initialization
	*pReturnCode = ktvr::K2InitError_BadInterface;
}
