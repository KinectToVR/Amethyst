#include <iostream>
#include <Windows.h>
#include <thread>
#include <openvr_driver.h>
#include "K2Tracker.h"
#include "K2ServerDriver.h"

namespace k2_driver
{
	class K2ServerProvider : public vr::IServerTrackedDeviceProvider
	{
		K2ServerDriver m_ServerDriver;

	public:
		K2ServerProvider()
		{
			LOG(INFO) << "Provider component creation has started";
		}

		vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext) override
		{
			// NOTE 1: use the driver context. Sets up a big set of globals
			VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
			LOG(INFO) << "Driver context init success";

			// Initialize communication with K2API
			const int initCode = m_ServerDriver.init_ServerDriver(); // Default IPC addresses
			LOG(INFO) << "Driver's IPC server init code: " +
				std::to_string(initCode);

			if (initCode == 0)
			{
				LOG(INFO) << "OpenVR ServerDriver init success";
				return vr::VRInitError_None;
			}

			LOG(ERROR) << "OpenVR ServerDriver init failure! Aborting...";
			return vr::VRInitError_Driver_Failed;
		}

		void Cleanup() override
		{
			LOG(INFO) << "Cleaning up...";
			m_ServerDriver.setActive(false);
		}

		const char* const* GetInterfaceVersions() override
		{
			return vr::k_InterfaceVersions;
		}

		// It's running every frame
		void RunFrame() override
		{
			// Run a server loop
			smphFrameUpdate.release();
		}

		bool ShouldBlockStandbyMode() override
		{
			return false;
		}

		void EnterStandby() override
		{
		}

		void LeaveStandby() override
		{
		}
	};
}

bool g_bExiting = false;

class K2WatchdogDriver : public vr::IVRWatchdogProvider
{
public:
	K2WatchdogDriver()
	{
		m_pWatchdogThread = nullptr;
	}

	vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext) override;
	void Cleanup() override;

private:
	std::thread* m_pWatchdogThread;
};

vr::EVRInitError K2WatchdogDriver::Init(vr::IVRDriverContext* pDriverContext)
{
	VR_INIT_WATCHDOG_DRIVER_CONTEXT(pDriverContext);
	LOG(INFO) << "Watchdog init has started...";

	g_bExiting = false;

	return vr::VRInitError_None;
}

void K2WatchdogDriver::Cleanup()
{
	g_bExiting = true;
}

extern "C" __declspec(dllexport) void* HmdDriverFactory(const char* pInterfaceName, int* pReturnCode)
{
	// ktvr::GetK2AppDataFileDir will create all directories by itself

	/* If logging was set up by some other thing / assembly,
	 * "peacefully" ask it to exit and note that */
	if (google::IsGoogleLoggingInitialized())
	{
		LOG(WARNING) << "Uh-Oh! It appears that google logging was set up previously from this caller.\n" <<
			"Although, it appears GLog likes Amethyst more! (It said that itself, did you know?)\n" <<
			"Logging will be shut down, re-initialized, and forwarded to \"" <<
			ktvr::GetK2AppDataLogFileDir("Amethyst_VRDriver_").c_str() << "*.log\"";
		google::ShutdownGoogleLogging();
	}

	/* Initialize logging */
	google::InitGoogleLogging(ktvr::GetK2AppDataLogFileDir("Amethyst_VRDriver_").c_str());
	/* Log everything >=INFO to same file */
	google::SetLogDestination(google::GLOG_INFO, ktvr::GetK2AppDataLogFileDir("Amethyst_VRDriver_").c_str());
	google::SetLogFilenameExtension(".log");

	FLAGS_logbufsecs = 0; //Set max timeout
	FLAGS_minloglevel = google::GLOG_INFO;

	LOG(WARNING) <<
		"If you get a \"Check failed: !IsGoogleLoggingInitialized() You called InitGoogleLogging() twice!\","
		"please unregister all other drivers using the GLog library";

	LOG(INFO) << "~~~Amethyst OpenVR Driver new logging session begins here!~~~";
	LOG(INFO) << "Interface version name: " << pInterfaceName;
	LOG(INFO) << "K2API version name: " << ktvr::IK2API_Version;

	static k2_driver::K2ServerProvider k2_server_provider;
	static K2WatchdogDriver k2_watchdog_driver;

	LOG(INFO) << "Amethyst OpenVR Driver will try to run on Amethyst API's default addresses.";

	if (0 == strcmp(vr::IServerTrackedDeviceProvider_Version, pInterfaceName))
	{
		return &k2_server_provider;
	}
	if (0 == strcmp(vr::IVRWatchdogProvider_Version, pInterfaceName))
	{
		return &k2_watchdog_driver;
	}

	(*pReturnCode) = vr::VRInitError_None;

	if (pReturnCode)
		*pReturnCode = vr::VRInitError_Init_InterfaceNotFound;
}
