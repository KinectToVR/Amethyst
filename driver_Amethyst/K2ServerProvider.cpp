#include <filesystem>
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

BOOL IsUserAdmin(VOID);
std::filesystem::path GetProgramLocation();

extern "C" __declspec(dllexport) void* HmdDriverFactory(const char* pInterfaceName, int* pReturnCode)
{
	// ktvr::GetK2AppDataFileDir will create all directories by itself

	/* If logging was set up by some other thing / assembly,
	 * "peacefully" ask it to exit and note that */
	if (google::IsGoogleLoggingInitialized())
	{
		LOG(WARNING) << "Uh-Oh! It appears that google logging was set up previously from this caller.\n " <<
			"Although, it appears GLog likes Amethyst more! (It said that itself, did you know?)\n " <<
			"Logging will be shut down, re-initialized, and forwarded to \"" <<
			ktvr::GetK2AppDataLogFileDir("Amethyst_VRDriver_").c_str() << "*.log\"";
		google::ShutdownGoogleLogging();
	}

	// Set up logging : flags
	FLAGS_logbufsecs = 0; // Set max timeout
	FLAGS_minloglevel = google::GLOG_INFO;
	FLAGS_timestamp_in_logfile_name = true;

	// Set up the logging directory
	const auto thisLogDestination = ktvr::GetK2AppDataLogFileDir("Amethyst_VRDriver_");

	// Init logging
	google::InitGoogleLogging(thisLogDestination.c_str());

	// Delete logs older than 7 days
	google::EnableLogCleaner(7);

	// Log everything >=INFO to same file
	google::SetLogDestination(google::GLOG_INFO, thisLogDestination.c_str());
	google::SetLogFilenameExtension(".log");

	LOG(INFO) << "~~~Amethyst OpenVR Driver new logging session begins here!~~~";
	LOG(INFO) << "Interface version name: " << pInterfaceName;
	LOG(INFO) << "K2API version name: " << ktvr::IAME_API_Version;

	LOG(WARNING) <<
		"If you get a \"Check failed: !IsGoogleLoggingInitialized() You called InitGoogleLogging() twice!\", "
		"please unregister all other drivers using the GLog library";

	static k2_driver::K2ServerProvider k2_server_provider;
	static K2WatchdogDriver k2_watchdog_driver;

	LOG(INFO) << "Amethyst OpenVR Driver will try to run on Amethyst API's default addresses.";

	/* Check if we're not running on admin */
	if (IsUserAdmin())
	{
		LOG(WARNING) << "SteamVR running as administrator! All app connection requests will be refused by it!";
		
		const auto CHandlerPath = GetProgramLocation()
		                          .parent_path() // win64 (driver)
		                          .parent_path() // bin (driver)
		                          .parent_path() // Amethyst (driver)
		                          .parent_path() // Amethyst (root)
			/ "K2CrashHandler" / "K2CrashHandler.exe";

		LOG(INFO) << "Got crash handler assumed location: " << CHandlerPath.string();

		if (exists(CHandlerPath))
		{
			std::thread([CHandlerPath]
			{
				ShellExecuteW(nullptr, L"open",
				              CHandlerPath.wstring().c_str(), L"vr_elevated", nullptr, SW_SHOWDEFAULT);
			}).detach();
		}
		else
		{
			LOG(WARNING) << "Crash handler exe (../../../K2CrashHandler/K2CrashHandler.exe) not found!";

			MessageBoxA(nullptr, "SteamVR is running as administrator!\n\n"
			            "The SteamVR process is currently elevated and Amethyst cannot communicate with it. "
			            "Either Steam or SteamVR was tampered with to cause this.Undo these changes and try again. "
			            "You see this error as a messagebox because the crash handler exe has not been found! "
			            "(should be at \"../../../K2CrashHandler/K2CrashHandler.exe\" reative to the driver's .dll) ",
			            "SteamVR running as admin!",
			            MB_OK);
		}
	}

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

// https://docs.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-checktokenmembership#examples
BOOL IsUserAdmin(VOID)
/*++
Routine Description: This routine returns TRUE if the caller's
process is a member of the Administrators local group. Caller is NOT
expected to be impersonating anyone and is expected to be able to
open its own process and process token.
Arguments: None.
Return Value:
   TRUE - Caller has Administrators local group.
   FALSE - Caller does not have Administrators local group. --
*/
{
	BOOL b;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup;
	b = AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&AdministratorsGroup);
	if (b)
	{
		if (!CheckTokenMembership(nullptr, AdministratorsGroup, &b))
		{
			b = FALSE;
		}
		FreeSid(AdministratorsGroup);
	}

	return (b);
}

// From k2appLLinterfacing@K2EVRInput.h
std::filesystem::path GetProgramLocation()
{
	TCHAR buffer[MAX_PATH] = {0};
	HMODULE hm = NULL;

	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
	                  GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
	                  (LPCWSTR)&HmdDriverFactory, &hm);
	GetModuleFileName(hm, buffer, MAX_PATH);

	return buffer; // Self-converts
}
