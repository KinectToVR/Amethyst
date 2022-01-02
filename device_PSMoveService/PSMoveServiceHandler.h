﻿#pragma once
#include <Windows.h>
#include <PSMoveClient_CAPI.h>
#include <ClientConstants.h>
#include <SharedConstants.h>
#include <glog/logging.h>

#include "KinectToVR_API_Devices.h"
#include "KinectToVR_API_Paths.h"

/* Errors */
#define FACILITY_PSMS 0x301
#define E_PSMS_NOT_RUNNING			MAKE_HRESULT(SEVERITY_ERROR, FACILITY_PSMS, 2)

/* Not exported */

class PSMoveServiceHandler : public ktvr::K2TrackingDeviceBase_JointsBasis
{
	// A representation of the Kinect elements for the v1 api
public:
	PSMoveServiceHandler()
	{
		//PSMoveServiceHandler::initialize();
		PSMoveServiceHandler::initLogging();

		K2TrackingDeviceBase_JointsBasis::deviceType = ktvr::K2_Joints;
		K2TrackingDeviceBase_JointsBasis::deviceName = "PSMove Service";
	}

	void initialize() override;
	void update() override;
	void shutdown() override;
	void signalJoint(uint32_t at) override;

	virtual ~PSMoveServiceHandler()
	{
	}

	HRESULT getStatusResult() override;
	std::string statusResultString(HRESULT stat) override;

private:

	/* Configure logging and print first message */
	static void initLogging()
	{
		// ktvr::GetK2AppDataFileDir will create all directories by itself

		/* Initialize logging */
		google::InitGoogleLogging(ktvr::GetK2AppDataLogFileDir("device_PSMoveService").c_str());
		/* Log everything >=INFO to same file */
		google::SetLogDestination(google::GLOG_INFO, ktvr::GetK2AppDataLogFileDir("device_PSMoveService").c_str());
		google::SetLogFilenameExtension(".log");

		FLAGS_logbufsecs = 0; //Set max timeout
		FLAGS_minloglevel = google::GLOG_INFO;

		LOG(INFO) << "~~~K2App / PSMS new logging session begins here!~~~";
	}

	bool startup();
	void processKeyInputs();
	void rebuildPSMoveLists();
	void rebuildControllerList();

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
	
	//Constants
	double finalPSMoveScale = 1.0;
	const std::string k_trackingSystemName = "psmove";
};

/* Exported for dynamic linking */
extern "C" __declspec(dllexport) void* TrackingDeviceBaseFactory(
	const char* pVersionName, int* pReturnCode)
{
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
