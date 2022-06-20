#pragma once
#include "pch.h"

#include "K2EVRInput.h"
#include "K2Shared.h"

#include "LocalizedServerStatuses.h"

namespace winrt::Microsoft::UI::Xaml::Controls
{
	inline void AppendGridStarColumn(Controls::Grid& _grid)
	{
		Controls::ColumnDefinition _col;
		_col.Width(GridLengthHelper::FromValueAndType(1, GridUnitType::Star));

		_grid.ColumnDefinitions().Append(_col);
	}

	inline void AppendGridStarsColumn(Controls::Grid& _grid, uint32_t stars)
	{
		Controls::ColumnDefinition _col;
		_col.Width(GridLengthHelper::FromValueAndType(stars, GridUnitType::Star));

		_grid.ColumnDefinitions().Append(_col);
	}

	inline void AppendGridStarsColumnMinWidthPixels(
		Controls::Grid& _grid, uint32_t stars, uint32_t pixels)
	{
		Controls::ColumnDefinition _col;
		_col.Width(GridLengthHelper::FromValueAndType(stars, GridUnitType::Star));
		_col.MinWidth(pixels);

		_grid.ColumnDefinitions().Append(_col);
	}

	inline void AppendGridPixelsColumn(Controls::Grid& _grid, uint32_t pixels)
	{
		Controls::ColumnDefinition _col;
		_col.Width(GridLengthHelper::FromValueAndType(pixels, GridUnitType::Pixel));

		_grid.ColumnDefinitions().Append(_col);
	}

	inline void AppendGridStarRow(Controls::Grid& _grid)
	{
		Controls::RowDefinition _col;
		_col.Height(GridLengthHelper::FromValueAndType(1, GridUnitType::Star));

		_grid.RowDefinitions().Append(_col);
	}

	inline void AppendGridStarsRow(Controls::Grid& _grid, uint32_t stars)
	{
		Controls::RowDefinition _col;
		_col.Height(GridLengthHelper::FromValueAndType(stars, GridUnitType::Star));

		_grid.RowDefinitions().Append(_col);
	}

	inline void AppendGridPixelsRow(Controls::Grid& _grid, uint32_t pixels)
	{
		Controls::RowDefinition _col;
		_col.Height(GridLengthHelper::FromValueAndType(pixels, GridUnitType::Pixel));

		_grid.RowDefinitions().Append(_col);
	}
}

// The main k2/interfacing namespace
namespace k2app::interfacing
{
	// Internal version number
	inline const std::string K2InternalVersion = "1.0.2.3"; // KTVR[ver:X.X.X.X]

	// App closing check
	inline bool isExitingNow = false;

	inline std::pair<Eigen::Vector3f, Eigen::Vector3f> // Position helpers for k2 devices -> Base, Override
		kinectHeadPosition{Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 0, 0)}, // But this one's kinect-only
		kinectWaistPosition{Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 0, 0)}; // This one applies to both bases

	// OpenVR playspace position
	inline Eigen::Vector3f vrPlayspaceTranslation = Eigen::Vector3f(0, 0, 0);
	// OpenVR playspace rotation
	inline float vrPlayspaceOrientation = 0.f; // Note: radians
	inline Eigen::Quaternionf vrPlayspaceOrientationQuaternion{1, 0, 0, 0};

	// Show an app toast / notification
	inline void ShowToast(const std::wstring& header,
	                      const std::wstring& text, const bool high_priority = false)
	{
		if (header.empty() || text.empty())return;

		winrt::hstring payload =
		(LR"(<toast>
						<visual>
							<binding template = "ToastGeneric">
								<text>)" + header + LR"(</text>"
								<text>)" + text + LR"(</text>
							</binding>
						</visual>
					</toast>)").c_str();

		winrt::Microsoft::Windows::AppNotifications::AppNotification toast(payload);
		toast.Tag(L"Tag_AmeNotifications");
		toast.Group(L"Group_AmeNotifications");
		toast.Priority(static_cast<
			winrt::Microsoft::Windows::AppNotifications::AppNotificationPriority>(high_priority));

		shared::main::thisNotificationManager.get()->Show(toast);
	}

	// Get log timestamp
	inline std::string GetLogTimestamp()
	{
		// From glog/logging.cc
		struct ::tm tm_time;
		const auto tm_now = time(NULL);
		localtime_s(&tm_time, &tm_now);

		std::ostringstream time_pid_stream;
		time_pid_stream.fill('0');
		time_pid_stream << 1900 + tm_time.tm_year
			<< std::setw(2) << 1 + tm_time.tm_mon
			<< std::setw(2) << tm_time.tm_mday
			<< '-'
			<< std::setw(2) << tm_time.tm_hour
			<< std::setw(2) << tm_time.tm_min
			<< std::setw(2) << tm_time.tm_sec
			<< '.'
			<< _getpid();

		return time_pid_stream.str();
	}

	// Handle the exit, implemented in MainWindow.xaml.cpp
	void handle_app_exit(const uint32_t& p_sleep_millis = 1000);
	inline void handle_app_exit_n(void) { handle_app_exit(); } // Wrapper

	// This sessions' log file dir
	inline std::string thisLogDestination;

	// Input actions' handler
	inline K2EVRInput::SteamEVRInput evr_input;

	// If trackers are added / initialized
	inline bool K2AppTrackersSpawned = false,
	            K2AppTrackersInitialized = false;

	// Is the tracking paused
	inline bool isTrackingFrozen = false;

	// Server checking threads number, max num of them
	inline uint32_t pingCheckingThreadsNumber = 0,
	                maxPingCheckingThreads = 3;

	// Server interfacing data
	inline int serverDriverStatusCode = 0;
	inline uint32_t pingTime = 0, parsingTime = 0;
	inline bool isServerDriverPresent = false,
	            serverDriverFailure = false;
	inline std::wstring serverStatusString = L" \n \n ";

	// For manual calibration
	inline bool calibration_confirm,
	            calibration_modeSwap,
	            calibration_fineTune;

	// For manual calibration: L, R -> X, Y
	inline std::array<std::array<float, 2>, 2>
	calibration_joystick_positions;

	// Check if we're currently scanning for trackers from other apps
	inline std::atomic_bool isAlreadyAddedTrackersScanRunning = false;
	// If the already-added trackers check was requested
	inline std::atomic_bool alreadyAddedTrackersScanRequested = false;

	// Forward-declared from JointSelectorExpander.h
	inline void devices_check_disabled_joints();

	// Controllers' ID's (vr::k_unTrackedDeviceIndexInvalid for non-existent)
	inline std::pair<uint32_t, uint32_t> vrControllerIndexes{
		vr::k_unTrackedDeviceIndexInvalid, // Left
		vr::k_unTrackedDeviceIndexInvalid // Right
	};

	// Function to spawn default' enabled trackers
	inline bool SpawnEnabledTrackers()
	{
		if (!K2AppTrackersSpawned)
		{
			LOG(INFO) << "[K2Interfacing] Registering trackers now...";

			// K2Driver is now auto-adding default lower body trackers.
			// That means that ids are: W-0 L-1 R-2
			// We may skip downloading them then ^_~

			LOG(INFO) << "[K2Interfacing] App will be using K2Driver's default prepended trackers!";

			// Helper bool array
			std::vector<bool> spawned;

			// Try 3 times
			for (int i = 0; i < 3; i++)
				for (const auto& tracker : K2Settings.K2TrackersVector)
					if (tracker.data.isActive)
					{
						spawned.push_back(false);

						if (const auto& m_result =
								set_tracker_state(tracker.tracker, true); // We WANT a reply
							m_result.tracker == tracker.tracker && m_result.success)
						{
							LOG(INFO) << "Tracker with serial " + tracker.data.serial +
								" and role " +
								std::to_string(static_cast<int>(tracker.tracker)) +
								" was successfully updated with status [active]";
							spawned.back() = true;
						}

						else if (m_result.tracker != tracker.tracker && m_result.success)
							LOG(ERROR) << "Tracker with serial " + tracker.data.serial + " and id "
								+
								std::to_string(static_cast<int>(tracker.tracker)) +
								" could not be spawned due to return mismatch.";

						else
						{
							LOG(ERROR) << "Tracker with serial " + tracker.data.serial +
								" and role " +
								std::to_string(static_cast<int>(tracker.tracker)) +
								" could not be spawned due to internal server error.";
							if (!ktvr::GetLastError().empty())
								LOG(ERROR) << "Last Amethyst API error: " + ktvr::GetLastError();
						}
					}

			// If one or more trackers failed to spawn
			if (!spawned.empty() && std::ranges::find(spawned, false) != spawned.end())
			{
				LOG(INFO) << "One or more trackers couldn't be spawned after 3 tries. Giving up...";

				// Cause not checking anymore
				serverDriverFailure = true;
				K2AppTrackersSpawned = false;
				K2AppTrackersInitialized = false;

				return false;
			}
		}

		// Notify that we're good now
		K2AppTrackersSpawned = true;
		K2AppTrackersInitialized = true;

		/*
			 * Trackers are stealing input from controllers when first added,
			 * due to some weird wonky stuff happening and OpenVR not expecting them.
			 * We're gonna de-spawn them for 8 frames (100ms) and re-spawn after another
			 */

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		K2AppTrackersInitialized = false;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		K2AppTrackersInitialized = true;

		return true;
	}

	/**
		 * \brief This will init OpenVR
		 * \return Success?
		 */
	inline bool OpenVRStartup()
	{
		LOG(INFO) << "Attempting connection to VRSystem... ";

		vr::EVRInitError eError = vr::VRInitError_None;
		vr::IVRSystem* m_VRSystem = VR_Init(&eError, vr::VRApplication_Overlay);

		if (eError != vr::VRInitError_None)
		{
			LOG(ERROR) << "IVRSystem could not be initialized: EVRInitError Code " << eError;
			/*MessageBoxA(nullptr,
							std::string(
								"Couldn't initialise VR system. (Code " + std::to_string(eError) +
								")\n\nPlease check if SteamVR is installed (or running) and try again."
							).c_str(),
							"IVRSystem Init Failure!",
							MB_OK);*/

			return false; // Fail
		}

		// Since we're ok, capture playspace details
		const auto trackingOrigin = m_VRSystem->GetRawZeroPoseToStandingAbsoluteTrackingPose();

		vrPlayspaceTranslation = EigenUtils::p_cast_type<Eigen::Vector3f>(trackingOrigin);
		vrPlayspaceOrientationQuaternion = EigenUtils::p_cast_type<Eigen::Quaternionf>(trackingOrigin);
		
		vrPlayspaceOrientation = EigenUtils::RotationProjectedYaw(
			vrPlayspaceOrientationQuaternion); // Yaw angle
		
		// Rescan controller ids
		vrControllerIndexes = std::make_pair(
			vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(
				vr::ETrackedControllerRole::TrackedControllerRole_RightHand),
			vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(
				vr::ETrackedControllerRole::TrackedControllerRole_LeftHand));

		LOG(INFO) << "VR Playspace translation: \n" << vrPlayspaceTranslation;
		LOG(INFO) << "VR Playspace orientation: \n" << EigenUtils::QuatToEulers(vrPlayspaceOrientationQuaternion);
		LOG(INFO) << "VR Playspace orientation yaw-only: \n" << vrPlayspaceOrientation;

		return true; // OK
	}

	/**
		 * \brief This will init VR Input Actions
		 * \return Success?
		 */
	inline bool EVRActionsStartup()
	{
		LOG(INFO) << "Attempting to set up EVR Input Actions...";

		if (!evr_input.InitInputActions())
		{
			LOG(ERROR) << "Could not set up Input Actions. Please check the upper log for further information.";
			/*MessageBoxA(nullptr,
							std::string(
								"Couldn't set up Input Actions.\n\nPlease check the log file for further information."
							).c_str(),
							"EVR Input Actions Init Failure!",
							MB_OK);*/

			return false;
		}

		LOG(INFO) << "EVR Input Actions set up OK";
		return true;
	}

	/**
		 * \brief This will install Ame's vr manifest
		 * \return Fail:0 Success:1 Other:2
		 */
	inline uint32_t installApplicationManifest()
	{
		if (vr::VRApplications()->IsApplicationInstalled("KinectToVR.Amethyst"))
		{
			LOG(INFO) << "Amethyst manifest is already installed";
			return 1;
		}
		if (exists(boost::dll::program_location().parent_path() / "Amethyst.vrmanifest"))
		{
			const auto app_error =
				vr::VRApplications()->AddApplicationManifest(
					(boost::dll::program_location().parent_path() / "Amethyst.vrmanifest").string().c_str());

			if (app_error != vr::VRApplicationError_None)
			{
				LOG(WARNING) << "Amethyst manifest not installed! Error: (VRApplicationError) " << app_error;
				return 2;
			}
			LOG(INFO) << "Amethyst manifest installed at: " <<
				boost::dll::program_location().parent_path() / "Amethyst.vrmanifest";
			return 1;
		}
		LOG(WARNING) << "Amethyst vr manifest (./Amethyst.vrmanifest) not found!";
		return 0;
	}

	/**
		 * \brief This will uninstall Ame's vr manifest
		 */
	inline void uninstallApplicationManifest()
	{
		if (vr::VRApplications()->IsApplicationInstalled("KinectToVR.Amethyst"))
		{
			vr::VRApplications()->RemoveApplicationManifest(
				(boost::dll::program_location().parent_path() / "Amethyst.vrmanifest").string().c_str());

			LOG(INFO) << "Attempted to remove Amethyst manifest at: " <<
				boost::dll::program_location().parent_path() / "Amethyst.vrmanifest";
		}
		if (vr::VRApplications()->IsApplicationInstalled("KinectToVR.Amethyst"))
			LOG(WARNING) << "Amethyst manifest removal failed! It may have been installed from somewhere else too";
		else
			LOG(INFO) << "Amethyst manifest removal succeed";
	}

	/**
		 * \brief This will init K2API and server driver
		 * \return Success?
		 */
	inline bool TestK2ServerConnection()
	{
		// Do not spawn 1000 voids, check how many do we have
		if (pingCheckingThreadsNumber <= maxPingCheckingThreads)
		{
			// Add a new worker
			pingCheckingThreadsNumber += 1; // May be ++ too

			try
			{
				// Send a ping message and capture the data
				const auto [test_response, send_time, full_time] = ktvr::test_connection();

				// Dump data to variables
				pingTime = full_time;
				parsingTime = std::clamp( // Subtract message creation (got) time and send time
					test_response.messageTimestamp - test_response.messageManualTimestamp,
					static_cast<long long>(1), LLONG_MAX);

				// Log ?success
				LOG(INFO) <<
					"Connection test has ended, [result: " <<
					(test_response.success ? "success" : "fail") <<
					"], response code: " << test_response.result;

				// Log some data if needed
				LOG(INFO) <<
					"\nTested ping time: " << full_time << " [micros], " <<

					"call time: " <<
					std::clamp( // Subtract message creation (got) time and send time
						send_time - test_response.messageManualTimestamp,
						static_cast<long long>(0), LLONG_MAX) <<
					" [micros], " <<

					"\nparsing time: " <<
					parsingTime << // Just look at the k2api
					" [micros], "

					"flight-back time: " <<
					std::clamp( // Subtract message creation (got) time and send time
						K2API_GET_TIMESTAMP_NOW - test_response.messageManualTimestamp,
						static_cast<long long>(1), LLONG_MAX) <<
					" [micros]";

				// Release
				pingCheckingThreadsNumber = std::clamp(
					static_cast<int>(pingCheckingThreadsNumber) - 1, 0,
					static_cast<int>(maxPingCheckingThreads) + 1);

				// Return the result
				return test_response.success;
			}
			catch (const std::exception& e)
			{
				// Log ?success
				LOG(INFO) <<
					"Connection test has ended, [result: fail], got an exception";

				// Release
				pingCheckingThreadsNumber = std::clamp(
					static_cast<int>(pingCheckingThreadsNumber) - 1, 0,
					static_cast<int>(maxPingCheckingThreads) + 1);
				return false;
			}
		}

		// else
		LOG(ERROR) << "Connection checking threads exceeds 3, aborting...";
		return false;
	}

	/**
		 * \brief This will check K2API and server driver
		 * \return Success?
		 */
	inline int CheckK2ServerStatus()
	{
		if (!isServerDriverPresent)
		{
			try
			{
				/* Initialize the port */
				LOG(INFO) << "Initializing the server IPC...";
				LOG(INFO) << "K2API version name: " << ktvr::IK2API_Version;

				const auto init_code = ktvr::init_k2api();
				bool server_connected = false;

				LOG(INFO) << "Server IPC initialization " <<
					(init_code == 0 ? "succeed" : "failed") << ", exit code: " << init_code;

				/* Connection test and display ping */
				// We may wait
				LOG(INFO) << "Testing the connection...";

				for (int i = 0; i < 3; i++)
				{
					LOG(INFO) << "Starting the test no " << i + 1 << "...";
					server_connected = true; // TestK2ServerConnection();
					// Not direct assignment since it's only a one-way check
					if (server_connected)isServerDriverPresent = true;
				}

				return init_code == 0
					       ? (server_connected ? 1 : -1)
					       : -10;
			}
			catch (const std::exception& e) { return -10; }
		}

		/*
			 * codes:
			 codes:
				-10: driver is disabled
				-1: driver is workin but outdated or doomed
				10: ur pc brokey, cry about it
				1: ok
			 */
		return 1; //don't check if it was already working
	}

	/**
		 * \brief This will init K2API and server driver
		 * \return Success?
		 */
	inline void K2ServerDriverSetup()
	{
		if (!serverDriverFailure)
		{
			// Backup the status
			serverDriverStatusCode = CheckK2ServerStatus();
		}
		else
		{
			// Overwrite the status
			serverDriverStatusCode = 10; // Fatal
		}

		isServerDriverPresent = false; // Assume fail
		serverStatusString = GetLocalizedStatusWStringAutomatic(status_wtf_map);
		//L"COULD NOT CHECK STATUS (Code -12)\nE_WTF\nSomething's fucked a really big time.";

		switch (serverDriverStatusCode)
		{
		case -10:
			serverStatusString = GetLocalizedStatusWStringAutomatic(status_exception_map);
		//L"EXCEPTION WHILE CHECKING (Code -10)\nE_EXCEPTION_WHILE_CHECKING\nCheck SteamVR add-ons (NOT overlays) and enable Amethyst.";
			break;
		case -1:
			serverStatusString = GetLocalizedStatusWStringAutomatic(status_connection_error_map);
		//L"SERVER CONNECTION ERROR (Code -1)\nE_CONNECTION_ERROR\nYour Amethyst SteamVR driver may be broken or outdated.";
			break;
		case 10:
			serverStatusString = GetLocalizedStatusWStringAutomatic(status_server_failure_map);
		//L"FATAL SERVER FAILURE (Code 10)\nE_FATAL_SERVER_FAILURE\nPlease restart, check logs and write to us on Discord.";
			break;
		case 1:
			serverStatusString = GetLocalizedStatusWStringAutomatic(status_ok_map);
		//L"Success! (Code 1)\nI_OK\nEverything's good!";
			isServerDriverPresent = true; // Change to success
			break;
		default:
			serverStatusString = GetLocalizedStatusWStringAutomatic(status_api_failure_map);
		//L"COULD NOT CONNECT TO K2API (Code -11)\nE_K2API_FAILURE\nThis error shouldn't occur, actually. Something's wrong a big part.";
			break;
		}

		// LOG the status
		LOG(INFO) << "Current K2 Server status: " << WStringToString(serverStatusString);
	}

	/**
		 * \brief This will check if there's any tracker with waist role in steamvr
		 * \return Success?
		 */
	inline bool findStringIC(const std::string& strHaystack, const std::string& strNeedle)
	{
		auto it = std::ranges::search(
			strHaystack.begin(), strHaystack.end(),
			strNeedle.begin(), strNeedle.end(),
			[](const char ch1, const char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
		);
		return !it.empty();
	}

	/**
		 * \brief This will check if there's any tracker with waist role in steamvr
		 * \_log Should we print logs?
		 * \return <Success?, id>
		 */
	inline std::pair<bool, uint32_t> findVRTracker(
		const std::string& _role,
		const bool _can_be_ame = true,
		const bool _log = true)
	{
		// Loop through all devices
		for (uint32_t i = 0; i < vr::k_unMaxTrackedDeviceCount; i++)
		{
			char buf[1024];
			vr::VRSystem()->GetStringTrackedDeviceProperty(i, vr::Prop_ControllerType_String, buf, sizeof buf);

			if (strlen(buf) > 0) // If we've found anything
				LOG_IF(INFO, _log) << "Found a device with roleHint: " << buf;
			else continue; // Don't waste our time

			// If we've actually found the one
			if (findStringIC(buf, _role))
			{
				const auto stat = vr::VRSystem()->GetTrackedDeviceActivityLevel(i);
				if (stat != vr::k_EDeviceActivityLevel_UserInteraction &&
					stat != vr::k_EDeviceActivityLevel_UserInteraction_Timeout)
					continue;

				char buf_p[1024];
				vr::VRSystem()->
					GetStringTrackedDeviceProperty(i, vr::Prop_SerialNumber_String, buf_p, sizeof buf_p);

				// Log that we're finished
				LOG_IF(INFO, _log) <<
					"\nFound an active " + _role + " tracker with:\n    hint: " <<
					buf << "\n    serial: " <<
					buf_p << "\n    id: " << i;

				// Check if it's not ame's
				bool _can_return = true;

				if (!_can_be_ame)
					for (const auto& _tracker : k2app::K2Settings.K2TrackersVector)
						if (std::string(buf_p) == _tracker.data.serial)
						{
							LOG_IF(INFO, _log) <<
								"Skipping the latest found tracker because it's been added from Amethyst";
							_can_return = false; // Maybe next time, bud
						}

				// Return what we've got
				if (_can_return)
					return std::make_pair(true, i);
			}
		}

		// We've failed if the loop's finished
		LOG_IF(WARNING, _log) <<
			"Didn't find any " + _role + " tracker in SteamVR with a proper role hint (Prop_ControllerType_String)";
		return std::make_pair(false, vr::k_unTrackedDeviceIndexInvalid);
	}

	/**
		 * \brief Pull pose data from SteamVR's waist tracker (if any)
		 * \_log Should we print logs? Default: no
		 * \return <Position, Rotation>
		 */
	inline std::pair<Eigen::Vector3f, Eigen::Quaternionf> getVRWaistTrackerPose(const bool _log = false)
	{
		vr::TrackedDevicePose_t devicePose[vr::k_unMaxTrackedDeviceCount];
		vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::ETrackingUniverseOrigin::TrackingUniverseStanding, 0,
		                                                devicePose,
		                                                vr::k_unMaxTrackedDeviceCount);

		const auto waistPair = findVRTracker("waist", true, _log);

		if (waistPair.first)
		{
			// Extract pose from the returns
			// We don't care if it's invalid by any chance
			const auto waistPose = devicePose[waistPair.second];

			// Get pos & rot -> EigenUtils' gonna do this stuff for us
			return std::make_pair(EigenUtils::p_cast_type<Eigen::Vector3f>(waistPose.mDeviceToAbsoluteTracking),
			                      EigenUtils::p_cast_type<Eigen::Quaternionf>(waistPose.mDeviceToAbsoluteTracking));
		}

		LOG_IF(WARNING, _log) <<
			"Either waist tracker doesn't exist or its role hint (Prop_ControllerType_String) was invalid";

		return std::make_pair(Eigen::Vector3f::Zero(), Eigen::Quaternionf(1, 0, 0, 0));
	}

	inline void UpdateServerStatusUI()
	{
		// Update the status here
		using namespace winrt::Microsoft::UI::Xaml;

		// Disable UI (partially) if we've encountered an error
		if (shared::main::devicesItem.get() != nullptr)
		{
			//::k2app::shared::main::settingsItem.get()->IsEnabled(isServerDriverPresent);
			shared::main::devicesItem.get()->IsEnabled(isServerDriverPresent);
		}

		// Check with this one, should be the same for all anyway
		if (shared::general::serverErrorWhatText.get() != nullptr)
		{
			shared::general::serverErrorWhatText.get()->Visibility(
				isServerDriverPresent ? Visibility::Collapsed : Visibility::Visible);
			shared::general::serverErrorWhatGrid.get()->Visibility(
				isServerDriverPresent ? Visibility::Collapsed : Visibility::Visible);
			shared::general::serverErrorButtonsGrid.get()->Visibility(
				isServerDriverPresent ? Visibility::Collapsed : Visibility::Visible);
			shared::general::serverErrorLabel.get()->Visibility(
				isServerDriverPresent ? Visibility::Collapsed : Visibility::Visible);

			// Split status and message by \n
			shared::general::serverStatusLabel.get()->Text(
				split_status(serverStatusString)[0]);
			shared::general::serverErrorLabel.get()->Text(
				split_status(serverStatusString)[1]);
			shared::general::serverErrorWhatText.get()->Text(
				split_status(serverStatusString)[2]);

			// Optionally setup & show the reregister button
			shared::general::reRegisterButton.get()->Visibility(
				serverDriverStatusCode == -10
					? Visibility::Visible
					: Visibility::Collapsed);

			shared::general::serverOpenDiscordButton.get()->Height(
				serverDriverStatusCode == -10
					? 40
					: 65);
		}

		// Block some things if server isn't working properly
		if (!isServerDriverPresent)
		{
			LOG(ERROR) <<
				"An error occurred and the app couldn't connect to K2 Server. Please check the upper message for more info.";

			if (shared::general::errorWhatText.get() != nullptr)
			{
				LOG(INFO) << "[Server Error] Entering the server error state...";

				// Hide device error labels (if any)
				shared::general::errorWhatText.get()->Visibility(Visibility::Collapsed);
				shared::general::errorWhatGrid.get()->Visibility(Visibility::Collapsed);
				shared::general::errorButtonsGrid.get()->Visibility(Visibility::Collapsed);
				shared::general::trackingDeviceErrorLabel.get()->Visibility(
					Visibility::Collapsed);

				// Block spawn|offsets|calibration buttons, //disable autospawn for session (just don't save)
				shared::general::toggleTrackersButton.get()->IsEnabled(false);
				shared::general::calibrationButton.get()->IsEnabled(false);
				shared::general::offsetsButton.get()->IsEnabled(false);
				//::k2app::K2Settings.autoSpawnEnabledJoints = false;
			}
		}
	}

	// Get the quaternion representing the rotation
	inline Eigen::Quaternionf GetVRRotationFromMatrix(vr::HmdMatrix34_t matrix)
	{
		vr::HmdQuaternion_t q;

		q.w = sqrt(fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
		q.x = sqrt(fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
		q.y = sqrt(fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
		q.z = sqrt(fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;
		q.x = copysign(q.x, matrix.m[2][1] - matrix.m[1][2]);
		q.y = copysign(q.y, matrix.m[0][2] - matrix.m[2][0]);
		q.z = copysign(q.z, matrix.m[1][0] - matrix.m[0][1]);
		return EigenUtils::p_cast_type<Eigen::Quaternionf>(q);
	}

	// Get the vector representing the position
	inline Eigen::Vector3f GetVRPositionFromMatrix(vr::HmdMatrix34_t matrix)
	{
		vr::HmdVector3d_t v;

		v.v[0] = matrix.m[0][3];
		v.v[1] = matrix.m[1][3];
		v.v[2] = matrix.m[2][3];
		return EigenUtils::p_cast_type<Eigen::Vector3f>(v);
	}

	// HMD pose in OpenVR
	inline std::pair
	<
		Eigen::Vector3f, // Position
		Eigen::Quaternionf // Rotation
	>
	vrHMDPose
	{
		Eigen::Vector3f(0.f, 0.f, 0.f), // Init as zero
		Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f) // Init as non-empty
	};

	// Update HMD pose from OpenVR -> called in K2Main
	inline void updateHMDPosAndRot()
	{
		vr::TrackedDevicePose_t devicePose[vr::k_unMaxTrackedDeviceCount];
		vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::ETrackingUniverseOrigin::TrackingUniverseStanding, 0,
		                                                devicePose,
		                                                vr::k_unMaxTrackedDeviceCount);

		if (constexpr int HMD_INDEX = 0;
			devicePose[HMD_INDEX].bPoseIsValid)
		{
			if (vr::VRSystem()->GetTrackedDeviceClass(HMD_INDEX) == vr::TrackedDeviceClass_HMD)
			{
				// Extract pose from the returns
				const auto hmdPose = devicePose[HMD_INDEX];

				// Get pos & rot -> EigenUtils' gonna do this stuff for us
				auto position = EigenUtils::p_cast_type<Eigen::Vector3f>(hmdPose.mDeviceToAbsoluteTracking);
				auto quaternion = EigenUtils::p_cast_type<Eigen::Quaternionf>(hmdPose.mDeviceToAbsoluteTracking);

				vrHMDPose = std::make_pair(position, quaternion);
			}
		}
	}

	namespace plugins
	{
		inline Eigen::Vector3f plugins_getHMDPosition()
		{
			return vrHMDPose.first;
		}

		inline Eigen::Vector3f plugins_getHMDPositionCalibrated()
		{
			return vrHMDPose.first - vrPlayspaceTranslation;
		}

		inline Eigen::Quaternionf plugins_getHMDOrientation()
		{
			return vrHMDPose.second;
		}

		inline Eigen::Quaternionf plugins_getHMDOrientationCalibrated()
		{
			return vrPlayspaceOrientationQuaternion.inverse() * vrHMDPose.second;
		}

		inline std::pair<Eigen::Vector3f, Eigen::Quaternionf> plugins_getHMDPose()
		{
			return std::make_pair(plugins_getHMDPosition(), plugins_getHMDOrientation());
		}

		inline std::pair<Eigen::Vector3f, Eigen::Quaternionf> plugins_getHMDPoseCalibrated()
		{
			return std::make_pair(plugins_getHMDPositionCalibrated(), plugins_getHMDOrientationCalibrated());
		}

		// Note: this is in radians
		inline float plugins_getHMDOrientationYaw()
		{
			// Get current yaw angle
			return EigenUtils::RotationProjectedYaw(vrHMDPose.second);
		}

		// Note: this is in radians
		inline float plugins_getHMDOrientationYawCalibrated()
		{
			// Get current yaw angle (calibrated)
			return EigenUtils::RotationProjectedYaw(
				vrPlayspaceOrientationQuaternion.inverse() * vrHMDPose.second);
		}

		inline std::pair<Eigen::Vector3f, Eigen::Quaternionf> plugins_getLeftControllerPose()
		{
			vr::TrackedDevicePose_t devicePose[vr::k_unMaxTrackedDeviceCount];
			vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::ETrackingUniverseOrigin::TrackingUniverseStanding, 0,
			                                                devicePose, vr::k_unMaxTrackedDeviceCount);

			if (devicePose[vrControllerIndexes.first].bPoseIsValid)
			{
				if (vr::VRSystem()->GetTrackedDeviceClass(vrControllerIndexes.first) ==
					vr::TrackedControllerRole_LeftHand)
				{
					// Extract pose from the returns
					const auto device_pose = devicePose[vrControllerIndexes.first];

					// Get pos & rot -> EigenUtils' gonna do this stuff for us
					return std::make_pair(
						EigenUtils::p_cast_type<Eigen::Vector3f>(device_pose.mDeviceToAbsoluteTracking),
						EigenUtils::p_cast_type<Eigen::Quaternionf>(device_pose.mDeviceToAbsoluteTracking));
				}
			}
			return std::make_pair(Eigen::Vector3f(0, 0, 0),
			                      Eigen::Quaternionf(1, 0, 0, 0));
		}

		inline std::pair<Eigen::Vector3f, Eigen::Quaternionf> plugins_getLeftControllerPoseCalibrated()
		{
			vr::TrackedDevicePose_t devicePose[vr::k_unMaxTrackedDeviceCount];
			vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::ETrackingUniverseOrigin::TrackingUniverseStanding, 0,
			                                                devicePose, vr::k_unMaxTrackedDeviceCount);

			if (devicePose[vrControllerIndexes.first].bPoseIsValid)
			{
				if (vr::VRSystem()->GetTrackedDeviceClass(vrControllerIndexes.first) ==
					vr::TrackedControllerRole_LeftHand)
				{
					// Extract pose from the returns
					const auto device_pose = devicePose[vrControllerIndexes.first];

					// Get pos & rot -> EigenUtils' gonna do this stuff for us
					return std::make_pair(
						EigenUtils::p_cast_type<Eigen::Vector3f>(device_pose.mDeviceToAbsoluteTracking) -
						vrPlayspaceTranslation,
						vrPlayspaceOrientationQuaternion.inverse() * EigenUtils::p_cast_type<Eigen::Quaternionf>(
							device_pose.mDeviceToAbsoluteTracking));
				}
			}
			return std::make_pair(Eigen::Vector3f(0, 0, 0),
			                      Eigen::Quaternionf(1, 0, 0, 0));
		}

		inline std::pair<Eigen::Vector3f, Eigen::Quaternionf> plugins_getRightControllerPose()
		{
			vr::TrackedDevicePose_t devicePose[vr::k_unMaxTrackedDeviceCount];
			vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::ETrackingUniverseOrigin::TrackingUniverseStanding, 0,
			                                                devicePose, vr::k_unMaxTrackedDeviceCount);

			if (devicePose[vrControllerIndexes.second].bPoseIsValid)
			{
				if (vr::VRSystem()->GetTrackedDeviceClass(vrControllerIndexes.second) ==
					vr::TrackedControllerRole_RightHand)
				{
					// Extract pose from the returns
					const auto device_pose = devicePose[vrControllerIndexes.second];

					// Get pos & rot -> EigenUtils' gonna do this stuff for us
					return std::make_pair(
						EigenUtils::p_cast_type<Eigen::Vector3f>(device_pose.mDeviceToAbsoluteTracking),
						EigenUtils::p_cast_type<Eigen::Quaternionf>(device_pose.mDeviceToAbsoluteTracking));
				}
			}
			return std::make_pair(Eigen::Vector3f(0, 0, 0),
			                      Eigen::Quaternionf(1, 0, 0, 0));
		}

		inline std::pair<Eigen::Vector3f, Eigen::Quaternionf> plugins_getRightControllerPoseCalibrated()
		{
			vr::TrackedDevicePose_t devicePose[vr::k_unMaxTrackedDeviceCount];
			vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::ETrackingUniverseOrigin::TrackingUniverseStanding, 0,
			                                                devicePose, vr::k_unMaxTrackedDeviceCount);

			if (devicePose[vrControllerIndexes.second].bPoseIsValid)
			{
				if (vr::VRSystem()->GetTrackedDeviceClass(vrControllerIndexes.second) ==
					vr::TrackedControllerRole_RightHand)
				{
					// Extract pose from the returns
					const auto device_pose = devicePose[vrControllerIndexes.second];

					// Get pos & rot -> EigenUtils' gonna do this stuff for us
					return std::make_pair(
						EigenUtils::p_cast_type<Eigen::Vector3f>(device_pose.mDeviceToAbsoluteTracking) -
						vrPlayspaceTranslation,
						vrPlayspaceOrientationQuaternion.inverse() * EigenUtils::p_cast_type<Eigen::Quaternionf>(
							device_pose.mDeviceToAbsoluteTracking));
				}
			}
			return std::make_pair(Eigen::Vector3f(0, 0, 0),
			                      Eigen::Quaternionf(1, 0, 0, 0));
		}

		inline std::array<ktvr::K2TrackedJoint, 7> plugins_getAppJointPoses()
		{
			return std::array<ktvr::K2TrackedJoint, 7>
			{
				K2Settings.K2TrackersVector.at(0).getK2TrackedJoint(
					K2Settings.K2TrackersVector[0].data.isActive, "Waist"),
				K2Settings.K2TrackersVector.at(1).getK2TrackedJoint(
					K2Settings.K2TrackersVector[1].data.isActive, "Left Foot"),
				K2Settings.K2TrackersVector.at(2).getK2TrackedJoint(
					K2Settings.K2TrackersVector[2].data.isActive, "Right Foot"),
				K2Settings.K2TrackersVector.at(3).getK2TrackedJoint(
					K2Settings.K2TrackersVector[3].data.isActive, "Left Elbow"),
				K2Settings.K2TrackersVector.at(4).getK2TrackedJoint(
					K2Settings.K2TrackersVector[4].data.isActive, "Right Elbow"),
				K2Settings.K2TrackersVector.at(5).getK2TrackedJoint(
					K2Settings.K2TrackersVector[5].data.isActive, "Left Knee"),
				K2Settings.K2TrackersVector.at(6).getK2TrackedJoint(
					K2Settings.K2TrackersVector[6].data.isActive, "Right Knee"),
			};
		}
	}

	namespace overrides
	{
		inline ktvr::ITrackedJointType getFlippedJointType(ktvr::ITrackedJointType _joint, bool _flip = true)
		{
			if (!_flip)return _joint; // Just return the same one

			// Return the flipped joint
			switch (_joint)
			{
			default: return ktvr::Joint_Head;

			case ktvr::Joint_Head: return ktvr::Joint_Head;
			case ktvr::Joint_Neck: return ktvr::Joint_Neck;
			case ktvr::Joint_SpineShoulder: return ktvr::Joint_SpineShoulder;

			case ktvr::Joint_ShoulderLeft: return ktvr::Joint_ShoulderRight;
			case ktvr::Joint_ElbowLeft: return ktvr::Joint_ElbowRight;
			case ktvr::Joint_WristLeft: return ktvr::Joint_WristRight;
			case ktvr::Joint_HandLeft: return ktvr::Joint_HandRight;
			case ktvr::Joint_HandTipLeft: return ktvr::Joint_HandTipRight;
			case ktvr::Joint_ThumbLeft: return ktvr::Joint_ThumbRight;

			case ktvr::Joint_ShoulderRight: return ktvr::Joint_ShoulderLeft;
			case ktvr::Joint_ElbowRight: return ktvr::Joint_ElbowLeft;
			case ktvr::Joint_WristRight: return ktvr::Joint_WristLeft;
			case ktvr::Joint_HandRight: return ktvr::Joint_HandLeft;
			case ktvr::Joint_HandTipRight: return ktvr::Joint_HandTipLeft;
			case ktvr::Joint_ThumbRight: return ktvr::Joint_ThumbLeft;

			case ktvr::Joint_SpineMiddle: return ktvr::Joint_SpineMiddle;
			case ktvr::Joint_SpineWaist: return ktvr::Joint_SpineWaist;

			case ktvr::Joint_HipLeft: return ktvr::Joint_HipRight;
			case ktvr::Joint_KneeLeft: return ktvr::Joint_KneeRight;
			case ktvr::Joint_AnkleLeft: return ktvr::Joint_AnkleRight;
			case ktvr::Joint_FootLeft: return ktvr::Joint_FootRight;

			case ktvr::Joint_HipRight: return ktvr::Joint_HipLeft;
			case ktvr::Joint_KneeRight: return ktvr::Joint_KneeLeft;
			case ktvr::Joint_AnkleRight: return ktvr::Joint_AnkleLeft;
			case ktvr::Joint_FootRight: return ktvr::Joint_FootLeft;

			case ktvr::Joint_Total: return ktvr::Joint_Total;
			}
		}
	}
}


namespace k2app::interfacing
{
	namespace AppInterface
	{
		using namespace winrt::Microsoft::UI::Xaml;
		using namespace ktvr;

		class AppTextBlock final : public Interface::TextBlock
		{
		public:
			AppTextBlock(const std::wstring& text)
			{
				Create(text);
			}

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			bool Visibility() override
			{
				if (_ptr_text_block.get())
					return (_ptr_text_block.get()->Visibility()
						== Visibility::Visible);
				return true;
			}

			void Visibility(const bool& visibility) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_text_block.get())
							_ptr_text_block.get()->Visibility(
								visibility
									? Visibility::Visible
									: Visibility::Collapsed);
					});
			}

			// Width Get and Set
			uint32_t Width() override
			{
				if (_ptr_text_block.get())
					return _ptr_text_block.get()->Width();
				return 0;
			}

			void Width(const uint32_t& width) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_text_block.get())
							_ptr_text_block.get()->Width(width);
					});
			}

			// Height Get and Set
			uint32_t Height() override
			{
				if (_ptr_text_block.get())
					return _ptr_text_block.get()->Height();
				return 0;
			}

			void Height(const uint32_t& height) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_text_block.get())
							_ptr_text_block.get()->Height(height);
					});
			}

			// Text Get and Set
			std::wstring Text() override
			{
				if (_ptr_text_block.get())
					return _ptr_text_block.get()->Text().c_str();
				return L"";
			}

			void Text(const std::wstring& text) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_text_block.get())
							_ptr_text_block.get()->Text(text);
					});
			}

			// Get the underlying shared pointer
			std::shared_ptr<Controls::TextBlock> Get()
			{
				return _ptr_text_block;
			}

		protected:
			// Underlying object shared pointer
			std::shared_ptr<Controls::TextBlock> _ptr_text_block;

			// Creation: register a host and a callback
			void Create(const std::wstring& text)
			{
				// Create a XAML text block
				Controls::TextBlock _text_block;
				_text_block.Text(text.c_str());

				// Back it up
				_ptr_text_block = std::make_shared<Controls::TextBlock>(_text_block);
			}
		};

		class AppButton final : public Interface::Button
		{
		public:
			AppButton(const std::wstring& content)
			{
				Create(content);
			}

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			bool Visibility() override
			{
				if (_ptr_button.get())
					return (_ptr_button.get()->Visibility()
						== Visibility::Visible);
				return true;
			}

			void Visibility(const bool& visibility) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_button.get())
							_ptr_button.get()->Visibility(
								visibility
									? Visibility::Visible
									: Visibility::Collapsed);
					});
			}

			// Width Get and Set
			uint32_t Width() override
			{
				if (_ptr_button.get())
					return _ptr_button.get()->Width();
				return 0;
			}

			void Width(const uint32_t& width) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_button.get())
							_ptr_button.get()->Width(width);
					});
			}

			// Height Get and Set
			uint32_t Height() override
			{
				if (_ptr_button.get())
					return _ptr_button.get()->Height();
				return 0;
			}

			void Height(const uint32_t& height) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_button.get())
							_ptr_button.get()->Height(height);
					});
			}

			// IsEnabled Get and Set
			bool IsEnabled() override
			{
				if (_ptr_button.get())
					return _ptr_button.get()->IsEnabled();
				return true;
			}

			void IsEnabled(const bool& enabled) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_button.get())
							_ptr_button.get()->IsEnabled(enabled);
					});
			}

			// Label Set
			void Content(const std::wstring& content) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_button.get())
							_ptr_button.get()->Content(
								winrt::box_value(content));
					});
			}

			// Get the underlying shared pointer
			std::shared_ptr<Controls::Button> Get()
			{
				return _ptr_button;
			}

		protected:
			// Underlying object shared pointer
			std::shared_ptr<Controls::Button> _ptr_button;

			// Creation: register a host and a callback
			void Create(const std::wstring& content)
			{
				// Create a XAML button
				Controls::Button _button;
				_button.Content(
					winrt::box_value(content.c_str()));

				// Back it up
				_ptr_button = std::make_shared<Controls::Button>(_button);

				// Create a dummy callback
				const std::function<void(
						const winrt::Windows::Foundation::IInspectable& sender,
						const RoutedEventArgs& e)>
					_n_callback = [this](const winrt::Windows::Foundation::IInspectable& sender,
					                     const RoutedEventArgs& e) ->
					void
					{
						if (OnClick) // Check if not null
							OnClick(this);
					};

				// Set up the click handler to point to the base's one
				_button.Click(_n_callback);
			}
		};

		class AppNumberBox final : public Interface::NumberBox
		{
		public:
			AppNumberBox()
			{
				Create(0);
			}

			AppNumberBox(const int& value)
			{
				Create(value);
			}

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			bool Visibility() override
			{
				if (_ptr_number_box.get())
					return (_ptr_number_box.get()->Visibility()
						== Visibility::Visible);
				return true;
			}

			void Visibility(const bool& visibility) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_number_box.get())
							_ptr_number_box.get()->Visibility(
								visibility
									? Visibility::Visible
									: Visibility::Collapsed);
					});
			}

			// Width Get and Set
			uint32_t Width() override
			{
				if (_ptr_number_box.get())
					return _ptr_number_box.get()->Width();
				return 0;
			}

			void Width(const uint32_t& width) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_number_box.get())
							_ptr_number_box.get()->Width(width);
					});
			}

			// Height Get and Set
			uint32_t Height() override
			{
				if (_ptr_number_box.get())
					return _ptr_number_box.get()->Height();
				return 0;
			}

			void Height(const uint32_t& height) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_number_box.get())
							_ptr_number_box.get()->Height(height);
					});
			}

			// IsEnabled Get and Set
			bool IsEnabled() override
			{
				if (_ptr_number_box.get())
					return _ptr_number_box.get()->IsEnabled();
				return true;
			}

			void IsEnabled(const bool& enabled) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_number_box.get())
							_ptr_number_box.get()->IsEnabled(enabled);
					});
			}

			// Value Get and Set
			int Value() override
			{
				if (_ptr_number_box.get())
					return static_cast<int>(_ptr_number_box.get()->Value());
				return 0;
			}

			void Value(const int& value) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_number_box.get())
							_ptr_number_box.get()->Value(value);
					});
			}

			// Get the underlying shared pointer
			std::shared_ptr<Controls::NumberBox> Get()
			{
				return _ptr_number_box;
			}

		protected:
			// Underlying object shared pointer
			std::shared_ptr<Controls::NumberBox> _ptr_number_box;

			// Creation: register a host and a callback
			void Create(const int& value)
			{
				// Create a XAML number box
				Controls::NumberBox _number_box;
				_number_box.Value(value);

				_number_box.SpinButtonPlacementMode(Controls::NumberBoxSpinButtonPlacementMode::Inline);
				_number_box.SmallChange(1);
				_number_box.LargeChange(10);

				// Back it up
				_ptr_number_box = std::make_shared<Controls::NumberBox>(_number_box);

				// Create a dummy callback
				const std::function<void(
						const winrt::Windows::Foundation::IInspectable& sender,
						const Controls::NumberBoxValueChangedEventArgs& e)>
					_n_callback = [this](const winrt::Windows::Foundation::IInspectable& sender,
					                     const Controls::NumberBoxValueChangedEventArgs
					                     & e) ->
					void
					{
						if (OnValueChanged) // Check if not null
							OnValueChanged(this, static_cast<int>(e.NewValue()));
					};

				// Set up the click handler to point to the base's one
				_number_box.ValueChanged(_n_callback);
			}
		};

		class AppComboBox final : public Interface::ComboBox
		{
		public:
			AppComboBox()
			{
				Create({L"Dummy Entry"});
			}

			AppComboBox(const std::vector<std::wstring>& entries)
			{
				Create(entries);
			}

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			bool Visibility() override
			{
				if (_ptr_combo_box.get())
					return (_ptr_combo_box.get()->Visibility()
						== Visibility::Visible);
				return true;
			}

			void Visibility(const bool& visibility) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_combo_box.get())
							_ptr_combo_box.get()->Visibility(
								visibility
									? Visibility::Visible
									: Visibility::Collapsed);
					});
			}

			// Width Get and Set
			uint32_t Width() override
			{
				if (_ptr_combo_box.get())
					return _ptr_combo_box.get()->Width();
				return 0;
			}

			void Width(const uint32_t& width) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_combo_box.get())
							_ptr_combo_box.get()->Width(width);
					});
			}

			// Height Get and Set
			uint32_t Height() override
			{
				if (_ptr_combo_box.get())
					return _ptr_combo_box.get()->Height();
				return 0;
			}

			void Height(const uint32_t& height) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_combo_box.get())
							_ptr_combo_box.get()->Height(height);
					});
			}

			// IsEnabled Get and Set
			bool IsEnabled() override
			{
				if (_ptr_combo_box.get())
					return _ptr_combo_box.get()->IsEnabled();
				return true;
			}

			void IsEnabled(const bool& enabled) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_combo_box.get())
							_ptr_combo_box.get()->IsEnabled(enabled);
					});
			}

			// Selected Index Get and Set
			uint32_t SelectedIndex() override
			{
				if (_ptr_combo_box.get())
					return _ptr_combo_box.get()->SelectedIndex();
				return 0;
			}

			void SelectedIndex(const uint32_t& value) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_combo_box.get() &&
							_ptr_combo_box.get()->Items().Size() < value)
							_ptr_combo_box.get()->SelectedIndex(value);
					});
			}

			// Items Vector Get and Set
			std::vector<std::wstring> Items() override
			{
				if (_ptr_combo_box.get())
				{
					std::vector<std::wstring> _items;

					// Construct a funny vector
					for (auto e : _ptr_combo_box.get()->Items())
						_items.push_back(e.as<winrt::hstring>().c_str());

					return _items;
				}
				return {};
			}

			// WARNING: DON'T CALL THIS DURING ANY OTHER MODIFICATION LIKE SELECTIONCHANGED
			void Items(const std::vector<std::wstring>& entries) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_combo_box.get())
						{
							// Boiler start - reset selection to a safe spot
							_ptr_combo_box.get()->SelectedIndex(0);

							// Clear items and append the new ones
							_ptr_combo_box.get()->Items().Clear();
							for (const auto& str : entries)
								_ptr_combo_box.get()->Items().Append(
									winrt::box_value(str));

							// Boiler end - reset selection to the start
							_ptr_combo_box.get()->SelectedIndex(0);
						}
					});
			}

			// Get the underlying shared pointer
			std::shared_ptr<Controls::ComboBox> Get()
			{
				return _ptr_combo_box;
			}

		protected:
			// Underlying object shared pointer
			std::shared_ptr<Controls::ComboBox> _ptr_combo_box;

			// Creation: register a host and a callback
			void Create(const std::vector<std::wstring>& entries)
			{
				// Create a XAML number box
				Controls::ComboBox _combo_box;

				_combo_box.Items().Clear();
				for (const auto& str : entries)
					_combo_box.Items().Append(winrt::box_value(str));

				_combo_box.SelectedIndex(0);

				// Back it up
				_ptr_combo_box = std::make_shared<Controls::ComboBox>(_combo_box);

				// Create a dummy callback
				const std::function<void(
						const winrt::Windows::Foundation::IInspectable& sender,
						const Controls::SelectionChangedEventArgs& e)>
					_n_callback = [this](const winrt::Windows::Foundation::IInspectable& sender,
					                     const Controls::SelectionChangedEventArgs
					                     & e) ->
					void
					{
						if (OnSelectionChanged) // Check if not null
							OnSelectionChanged(this, _ptr_combo_box.get()->SelectedIndex());
					};

				// Set up the click handler to point to the base's one
				_combo_box.SelectionChanged(_n_callback);
			}
		};

		class AppCheckBox final : public Interface::CheckBox
		{
		public:
			AppCheckBox()
			{
				Create();
			}

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			bool Visibility() override
			{
				if (_ptr_check_box.get())
					return (_ptr_check_box.get()->Visibility()
						== Visibility::Visible);
				return true;
			}

			void Visibility(const bool& visibility) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_check_box.get())
							_ptr_check_box.get()->Visibility(
								visibility
									? Visibility::Visible
									: Visibility::Collapsed);
					});
			}

			// Width Get and Set
			uint32_t Width() override
			{
				if (_ptr_check_box.get())
					return _ptr_check_box.get()->Width();
				return 0;
			}

			void Width(const uint32_t& width) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_check_box.get())
							_ptr_check_box.get()->Width(width);
					});
			}

			// Height Get and Set
			uint32_t Height() override
			{
				if (_ptr_check_box.get())
					return _ptr_check_box.get()->Height();
				return 0;
			}

			void Height(const uint32_t& height) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_check_box.get())
							_ptr_check_box.get()->Height(height);
					});
			}

			// IsEnabled Get and Set
			bool IsEnabled() override
			{
				if (_ptr_check_box.get())
					return _ptr_check_box.get()->IsEnabled();
				return true;
			}

			void IsEnabled(const bool& enabled) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_check_box.get())
							_ptr_check_box.get()->IsEnabled(enabled);
					});
			}

			// IsChecked Get and Set
			bool IsChecked() override
			{
				if (_ptr_check_box.get())
					return _ptr_check_box.get()->IsChecked().Value();
				return false;
			}

			void IsChecked(const bool& is_checked) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_check_box.get())
							_ptr_check_box.get()->IsChecked(is_checked);
					});
			}

			// Get the underlying shared pointer
			std::shared_ptr<Controls::CheckBox> Get()
			{
				return _ptr_check_box;
			}

		protected:
			// Underlying object shared pointer
			std::shared_ptr<Controls::CheckBox> _ptr_check_box;

			// Creation: register a host and a callback
			void Create()
			{
				// Create a XAML number box
				Controls::CheckBox _check_box;

				// Back it up
				_ptr_check_box = std::make_shared<Controls::CheckBox>(_check_box);

				// Create a dummy callback
				const std::function<void(
						const winrt::Windows::Foundation::IInspectable& sender,
						const RoutedEventArgs& e)>
					_n_callback_checked = [this](const winrt::Windows::Foundation::IInspectable& sender,
					                             const RoutedEventArgs& e) ->
					void
					{
						if (OnChecked) // Check if not null
							OnChecked(this);
					};

				// Create a dummy callback
				const std::function<void(
						const winrt::Windows::Foundation::IInspectable& sender,
						const RoutedEventArgs& e)>
					_n_callback_unchecked = [this](const winrt::Windows::Foundation::IInspectable& sender,
					                               const RoutedEventArgs& e) ->
					void
					{
						if (OnUnchecked) // Check if not null
							OnUnchecked(this);
					};

				// Set up the click handler to point to the base's one
				_check_box.Checked(_n_callback_checked);
				_check_box.Unchecked(_n_callback_unchecked);
			}
		};

		class AppToggleSwitch final : public Interface::ToggleSwitch
		{
		public:
			AppToggleSwitch()
			{
				Create();
			}

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			bool Visibility() override
			{
				if (_ptr_toggle_switch.get())
					return (_ptr_toggle_switch.get()->Visibility()
						== Visibility::Visible);
				return true;
			}

			void Visibility(const bool& visibility) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_toggle_switch.get())
							_ptr_toggle_switch.get()->Visibility(
								visibility
									? Visibility::Visible
									: Visibility::Collapsed);
					});
			}

			// Width Get and Set
			uint32_t Width() override
			{
				if (_ptr_toggle_switch.get())
					return _ptr_toggle_switch.get()->Width();
				return 0;
			}

			void Width(const uint32_t& width) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_toggle_switch.get())
							_ptr_toggle_switch.get()->Width(width);
					});
			}

			// Height Get and Set
			uint32_t Height() override
			{
				if (_ptr_toggle_switch.get())
					return _ptr_toggle_switch.get()->Height();
				return 0;
			}

			void Height(const uint32_t& height) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_toggle_switch.get())
							_ptr_toggle_switch.get()->Height(height);
					});
			}

			// IsEnabled Get and Set
			bool IsEnabled() override
			{
				if (_ptr_toggle_switch.get())
					return _ptr_toggle_switch.get()->IsEnabled();
				return true;
			}

			void IsEnabled(const bool& enabled) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_toggle_switch.get())
							_ptr_toggle_switch.get()->IsEnabled(enabled);
					});
			}

			// IsChecked Get and Set
			bool IsChecked() override
			{
				if (_ptr_toggle_switch.get())
					return _ptr_toggle_switch.get()->IsOn();
				return false;
			}

			void IsChecked(const bool& is_checked) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_toggle_switch.get())
							_ptr_toggle_switch.get()->IsOn(is_checked);
					});
			}

			// Get the underlying shared pointer
			std::shared_ptr<Controls::ToggleSwitch> Get()
			{
				return _ptr_toggle_switch;
			}

		protected:
			// Underlying object shared pointer
			std::shared_ptr<Controls::ToggleSwitch> _ptr_toggle_switch;

			// Creation: register a host and a callback
			void Create()
			{
				// Create a XAML number box
				Controls::ToggleSwitch _toggle_switch;
				_toggle_switch.OnContent(winrt::box_value(L""));
				_toggle_switch.OffContent(winrt::box_value(L""));

				// Back it up
				_ptr_toggle_switch = std::make_shared<Controls::ToggleSwitch>(_toggle_switch);

				// Create a dummy callback
				const std::function<void(
						const winrt::Windows::Foundation::IInspectable& sender,
						const RoutedEventArgs& e)>
					_n_callback = [this](const winrt::Windows::Foundation::IInspectable& sender,
					                     const RoutedEventArgs& e) ->
					void
					{
						// Check which handler to raise
						if (this->Get().get()->IsOn())
						{
							if (OnChecked) // Check if not null
								OnChecked(this);
						}
						else
						{
							if (OnUnchecked) // Check if not null
								OnUnchecked(this);
						}
					};

				// Set up the click handler to point to the base's one
				_toggle_switch.Toggled(_n_callback);
			}
		};

		class AppTextBox final : public Interface::TextBox
		{
		public:
			AppTextBox()
			{
				Create();
			}

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			bool Visibility() override
			{
				if (_ptr_text_box.get())
					return (_ptr_text_box.get()->Visibility()
						== Visibility::Visible);
				return true;
			}

			void Visibility(const bool& visibility) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_text_box.get())
							_ptr_text_box.get()->Visibility(
								visibility
									? Visibility::Visible
									: Visibility::Collapsed);
					});
			}

			// Width Get and Set
			uint32_t Width() override
			{
				if (_ptr_text_box.get())
					return _ptr_text_box.get()->Width();
				return 0;
			}

			void Width(const uint32_t& width) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_text_box.get())
							_ptr_text_box.get()->Width(width);
					});
			}

			// Height Get and Set
			uint32_t Height() override
			{
				if (_ptr_text_box.get())
					return _ptr_text_box.get()->Height();
				return 0;
			}

			void Height(const uint32_t& height) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_text_box.get())
							_ptr_text_box.get()->Height(height);
					});
			}

			// Text Get and Set
			std::wstring Text() override
			{
				if (_ptr_text_box.get())
					return _ptr_text_box.get()->Text().c_str();
				return L"";
			}

			void Text(const std::wstring& text) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_text_box.get())
							_ptr_text_box.get()->Text(text);
					});
			}

			// Get the underlying shared pointer
			std::shared_ptr<Controls::TextBox> Get()
			{
				return _ptr_text_box;
			}

		protected:
			// Underlying object shared pointer
			std::shared_ptr<Controls::TextBox> _ptr_text_box;

			// Creation: register a host and a callback
			void Create()
			{
				// Create a XAML text box
				Controls::TextBox _text_box;

				// Back it up
				_ptr_text_box = std::make_shared<Controls::TextBox>(_text_box);

				// Create a dummy callback
				const std::function<void(
						const winrt::Windows::Foundation::IInspectable& sender,
						const Input::KeyRoutedEventArgs& e)>
					_n_callback = [this](const winrt::Windows::Foundation::IInspectable& sender,
					                     const Input::KeyRoutedEventArgs& e) ->
					void
					{
						if (e.Key() == winrt::Windows::System::VirtualKey::Enter)
							OnEnterKeyDown(this);
					};

				// Set up the click handler to point to the base's one
				_text_box.KeyDown(_n_callback);
			}
		};

		// Poggers Ring
		class AppProgressRing final : public Interface::ProgressRing
		{
		public:
			AppProgressRing(const int32_t& progress)
			{
				Create(progress);
			}

			AppProgressRing()
			{
				Create(-1);
			}

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			bool Visibility() override
			{
				if (_ptr_progress_ring.get())
					return (_ptr_progress_ring.get()->Visibility()
						== Visibility::Visible);
				return true;
			}

			void Visibility(const bool& visibility) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_progress_ring.get())
							_ptr_progress_ring.get()->Visibility(
								visibility
									? Visibility::Visible
									: Visibility::Collapsed);
					});
			}

			// Width Get and Set
			uint32_t Width() override
			{
				if (_ptr_progress_ring.get())
					return _ptr_progress_ring.get()->Width();
				return 0;
			}

			void Width(const uint32_t& width) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_progress_ring.get())
							_ptr_progress_ring.get()->Width(width);
					});
			}

			// Height Get and Set
			uint32_t Height() override
			{
				if (_ptr_progress_ring.get())
					return _ptr_progress_ring.get()->Height();
				return 0;
			}

			void Height(const uint32_t& height) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_progress_ring.get())
							_ptr_progress_ring.get()->Height(height);
					});
			}

			// Progress Get and Set (Set <0 to mark as indeterminate)
			int32_t Progress() override
			{
				if (_ptr_progress_ring.get())
				{
					if (_ptr_progress_ring.get()->IsIndeterminate())
						return -1;
					return _ptr_progress_ring.get()->Value();
				}
				return -1;
			}

			void Progress(const int32_t& progress) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_progress_ring.get())
						{
							if (progress < 0)
							{
								_ptr_progress_ring.get()->IsActive(true);
								_ptr_progress_ring.get()->IsIndeterminate(true);
							}
							else
							{
								_ptr_progress_ring.get()->Value(progress);
								_ptr_progress_ring.get()->IsIndeterminate(false);
							}
						}
					});
			}

			// Get the underlying shared pointer
			std::shared_ptr<Controls::ProgressRing> Get()
			{
				return _ptr_progress_ring;
			}

		protected:
			// Underlying object shared pointer
			std::shared_ptr<Controls::ProgressRing> _ptr_progress_ring;

			// Creation: register a host and a callback
			void Create(const int32_t& progress)
			{
				// Create a XAML progress ring
				Controls::ProgressRing _progress_ring;

				if (progress < 0)
				{
					_progress_ring.IsActive(true);
					_progress_ring.IsIndeterminate(true);
				}
				else
				{
					_progress_ring.Value(progress);
					_progress_ring.IsIndeterminate(false);
				}

				// Back it up
				_ptr_progress_ring = std::make_shared<Controls::ProgressRing>(_progress_ring);
			}
		};

		// Poggers Bar
		class AppProgressBar final : public Interface::ProgressBar
		{
		public:
			AppProgressBar(const int32_t& progress)
			{
				Create(progress);
			}

			AppProgressBar()
			{
				Create(-1);
			}

			/* XAML-Derived functions & handlers */

			// Visibility Get and Set
			bool Visibility() override
			{
				if (_ptr_progress_bar.get())
					return (_ptr_progress_bar.get()->Visibility()
						== Visibility::Visible);
				return true;
			}

			void Visibility(const bool& visibility) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_progress_bar.get())
							_ptr_progress_bar.get()->Visibility(
								visibility
									? Visibility::Visible
									: Visibility::Collapsed);
					});
			}

			// Width Get and Set
			uint32_t Width() override
			{
				if (_ptr_progress_bar.get())
					return _ptr_progress_bar.get()->Width();
				return 0;
			}

			void Width(const uint32_t& width) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_progress_bar.get())
							_ptr_progress_bar.get()->Width(width);
					});
			}

			// Height Get and Set
			uint32_t Height() override
			{
				if (_ptr_progress_bar.get())
					return _ptr_progress_bar.get()->Height();
				return 0;
			}

			void Height(const uint32_t& height) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_progress_bar.get())
							_ptr_progress_bar.get()->Height(height);
					});
			}

			// Progress Get and Set (Set <0 to mark as indeterminate)
			int32_t Progress() override
			{
				if (_ptr_progress_bar.get())
				{
					if (_ptr_progress_bar.get()->IsIndeterminate())
						return -1;
					return _ptr_progress_bar.get()->Value();
				}
				return -1;
			}

			void Progress(const int32_t& progress) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_progress_bar.get())
						{
							if (progress < 0)
							{
								_ptr_progress_bar.get()->IsIndeterminate(true);
							}
							else
							{
								_ptr_progress_bar.get()->Value(progress);
								_ptr_progress_bar.get()->IsIndeterminate(false);
							}
						}
					});
			}

			// Paused Get and Set
			bool ShowPaused() override
			{
				if (_ptr_progress_bar.get())
					return _ptr_progress_bar.get()->ShowPaused();
				return false;
			}

			void ShowPaused(const bool& show_paused) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_progress_bar.get())
							_ptr_progress_bar.get()->ShowPaused(show_paused);
					});
			}

			// Error Get and Set
			bool ShowError() override
			{
				if (_ptr_progress_bar.get())
					return _ptr_progress_bar.get()->ShowError();
				return false;
			}

			void ShowError(const bool& show_error) override
			{
				if (!isExitingNow)
					shared::main::thisDispatcherQueue->TryEnqueue([=, this]
					{
						if (isExitingNow)return;
						if (_ptr_progress_bar.get())
							_ptr_progress_bar.get()->ShowError(show_error);
					});
			}

			// Get the underlying shared pointer
			std::shared_ptr<Controls::ProgressBar> Get()
			{
				return _ptr_progress_bar;
			}

		protected:
			// Underlying object shared pointer
			std::shared_ptr<Controls::ProgressBar> _ptr_progress_bar;

			// Creation: register a host and a callback
			void Create(const int32_t& progress)
			{
				// Create a XAML progress bar
				Controls::ProgressBar _progress_bar;
				_progress_bar.Width(70);

				if (progress < 0)
				{
					_progress_bar.IsIndeterminate(true);
				}
				else
				{
					_progress_bar.Value(progress);
					_progress_bar.IsIndeterminate(false);
				}

				// Back it up
				_ptr_progress_bar = std::make_shared<Controls::ProgressBar>(_progress_bar);
			}
		};

		inline HorizontalAlignment horizontalAlignmentConverter(
			const Interface::SingleLayoutHorizontalAlignment& alignment)
		{
			switch (alignment)
			{
			case Interface::SingleLayoutHorizontalAlignment::Left:
				return HorizontalAlignment::Left;

			case Interface::SingleLayoutHorizontalAlignment::Center:
				return HorizontalAlignment::Center;

			case Interface::SingleLayoutHorizontalAlignment::Right:
				return HorizontalAlignment::Right;

			default:
				return HorizontalAlignment::Stretch;
			}
		}

		class AppLayoutRoot final : public Interface::LayoutRoot
		{
		public:
			AppLayoutRoot()
			{
				Create();
			}

			// Append a One-Row single element
			void AppendSingleElement(
				const Interface::Element& element,
				const Interface::SingleLayoutHorizontalAlignment& alignment) override
			{
				// Switch based on element type: all types
				switch (element.index())
				{
				// TextBlock
				case 0:
					{
						const auto& pElement = static_cast<AppTextBlock*>(
							std::get<Interface::TextBlock*>(element));

						(*pElement).Get().get()->VerticalAlignment(
							VerticalAlignment::Center);

						(*pElement).Get().get()->HorizontalAlignment(
							horizontalAlignmentConverter(alignment));

						(*pElement).Get().get()->Margin({3, 3, 3, 3});

						_ptr_stack_panel.get()->Children().Append(*(*pElement).Get());

						return;
					}
				// Button
				case 1:
					{
						const auto& pElement = static_cast<AppButton*>(std::get<Interface::Button*>(element));

						(*pElement).Get().get()->VerticalAlignment(
							VerticalAlignment::Center);

						(*pElement).Get().get()->HorizontalAlignment(
							horizontalAlignmentConverter(alignment));

						(*pElement).Get().get()->Margin({3, 3, 3, 3});

						_ptr_stack_panel.get()->Children().Append(*(*pElement).Get());

						return;
					}
				// NumberBox
				case 2:
					{
						const auto& pElement = static_cast<AppNumberBox*>(
							std::get<Interface::NumberBox*>(element));

						(*pElement).Get().get()->VerticalAlignment(
							VerticalAlignment::Center);

						(*pElement).Get().get()->HorizontalAlignment(
							horizontalAlignmentConverter(alignment));

						(*pElement).Get().get()->Margin({3, 3, 3, 3});

						_ptr_stack_panel.get()->Children().Append(*(*pElement).Get());

						return;
					}
				// ComboBox
				case 3:
					{
						const auto& pElement = static_cast<AppComboBox*>(
							std::get<Interface::ComboBox*>(element));

						(*pElement).Get().get()->VerticalAlignment(
							VerticalAlignment::Center);

						(*pElement).Get().get()->HorizontalAlignment(
							horizontalAlignmentConverter(alignment));

						(*pElement).Get().get()->Margin({3, 3, 3, 3});

						_ptr_stack_panel.get()->Children().Append(*(*pElement).Get());

						return;
					}
				// CheckBox
				case 4:
					{
						const auto& pElement = static_cast<AppCheckBox*>(
							std::get<Interface::CheckBox*>(element));

						(*pElement).Get().get()->VerticalAlignment(
							VerticalAlignment::Center);

						(*pElement).Get().get()->HorizontalAlignment(
							horizontalAlignmentConverter(alignment));

						(*pElement).Get().get()->Margin({3, 3, 3, 3});

						_ptr_stack_panel.get()->Children().Append(*(*pElement).Get());

						return;
					}
				// ToggleSwitch
				case 5:
					{
						const auto& pElement = static_cast<AppToggleSwitch*>(
							std::get<Interface::ToggleSwitch*>(element));

						(*pElement).Get().get()->VerticalAlignment(
							VerticalAlignment::Center);

						(*pElement).Get().get()->HorizontalAlignment(
							horizontalAlignmentConverter(alignment));

						(*pElement).Get().get()->Margin({3, 3, 3, 3});

						_ptr_stack_panel.get()->Children().Append(*(*pElement).Get());

						return;
					}
				// TextBox
				case 6:
					{
						const auto& pElement = static_cast<AppTextBox*>(
							std::get<Interface::TextBox*>(element));

						(*pElement).Get().get()->VerticalAlignment(
							VerticalAlignment::Center);

						(*pElement).Get().get()->HorizontalAlignment(
							horizontalAlignmentConverter(alignment));

						(*pElement).Get().get()->Margin({3, 3, 3, 3});

						_ptr_stack_panel.get()->Children().Append(*(*pElement).Get());

						return;
					}
				// ProgressRing
				case 7:
					{
						const auto& pElement = static_cast<AppProgressRing*>(
							std::get<Interface::ProgressRing*>(element));

						(*pElement).Get().get()->VerticalAlignment(
							VerticalAlignment::Center);

						(*pElement).Get().get()->HorizontalAlignment(
							horizontalAlignmentConverter(alignment));

						(*pElement).Get().get()->Margin({3, 3, 3, 3});

						_ptr_stack_panel.get()->Children().Append(*(*pElement).Get());

						return;
					}
				// ProgressBar
				case 8:
					{
						const auto& pElement = static_cast<AppProgressBar*>(
							std::get<Interface::ProgressBar*>(element));

						(*pElement).Get().get()->VerticalAlignment(
							VerticalAlignment::Center);

						(*pElement).Get().get()->HorizontalAlignment(
							horizontalAlignmentConverter(alignment));

						(*pElement).Get().get()->Margin({3, 3, 3, 3});

						_ptr_stack_panel.get()->Children().Append(*(*pElement).Get());
					}
				}
			}

			// Append a One-Row element pair : */* column space
			void AppendElementPair(const Interface::Element& first_element,
			                       const Interface::Element& second_element) override
			{
				// Set up a placeholder horizontally divided grid
				Controls::Grid _grid;
				_grid.HorizontalAlignment(HorizontalAlignment::Stretch);

				Controls::AppendGridStarColumn(_grid);
				Controls::AppendGridStarColumn(_grid);

				// Parse and append the first pair element
				{
					// Switch based on element type: all types
					switch (first_element.index())
					{
					// TextBlock
					case 0:
						{
							const auto& pElement = static_cast<AppTextBlock*>(
								std::get<Interface::TextBlock*>(first_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Left);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 0);

							break;
						}
					// Button
					case 1:
						{
							const auto& pElement = static_cast<AppButton*>(
								std::get<Interface::Button*>(first_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Left);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 0);

							break;
						}
					// NumberBox
					case 2:
						{
							const auto& pElement = static_cast<AppNumberBox*>(
								std::get<Interface::NumberBox*>(first_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Left);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 0);

							break;
						}
					// ComboBox
					case 3:
						{
							const auto& pElement = static_cast<AppComboBox*>(
								std::get<Interface::ComboBox*>(first_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Left);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 0);

							break;
						}
					// CheckBox
					case 4:
						{
							const auto& pElement = static_cast<AppCheckBox*>(
								std::get<Interface::CheckBox*>(first_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Left);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 0);

							break;
						}
					// ToggleSwitch
					case 5:
						{
							const auto& pElement = static_cast<AppToggleSwitch*>(
								std::get<Interface::ToggleSwitch*>(first_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Left);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 0);

							break;
						}
					// TextBox
					case 6:
						{
							const auto& pElement = static_cast<AppTextBox*>(
								std::get<Interface::TextBox*>(first_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Left);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 0);

							break;
						}
					// ProgressRing
					case 7:
						{
							const auto& pElement = static_cast<AppProgressRing*>(
								std::get<Interface::ProgressRing*>(first_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Left);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 0);

							break;
						}
					// ProgressBar
					case 8:
						{
							const auto& pElement = static_cast<AppProgressBar*>(
								std::get<Interface::ProgressBar*>(first_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Left);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 0);

							break;
						}
					}
				}

				// Parse and append the second pair element
				{
					// Switch based on element type: all types
					switch (second_element.index())
					{
					// TextBlock
					case 0:
						{
							const auto& pElement = static_cast<AppTextBlock*>(
								std::get<Interface::TextBlock*>(second_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Right);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 1);

							break;
						}
					// Button
					case 1:
						{
							const auto& pElement = static_cast<AppButton*>(
								std::get<Interface::Button*>(second_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Right);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 1);

							break;
						}
					// NumberBox
					case 2:
						{
							const auto& pElement = static_cast<AppNumberBox*>(
								std::get<Interface::NumberBox*>(second_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Right);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 1);

							break;
						}
					// ComboBox
					case 3:
						{
							const auto& pElement = static_cast<AppComboBox*>(
								std::get<Interface::ComboBox*>(second_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Right);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 1);

							break;
						}
					// CheckBox
					case 4:
						{
							const auto& pElement = static_cast<AppCheckBox*>(
								std::get<Interface::CheckBox*>(second_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Right);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 1);

							break;
						}
					// ToggleSwitch
					case 5:
						{
							const auto& pElement = static_cast<AppToggleSwitch*>(
								std::get<Interface::ToggleSwitch*>(second_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Right);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 1);

							break;
						}
					// TextBox
					case 6:
						{
							const auto& pElement = static_cast<AppTextBox*>(
								std::get<Interface::TextBox*>(second_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Right);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 1);

							break;
						}
					// ProgressRing
					case 7:
						{
							const auto& pElement = static_cast<AppProgressRing*>(
								std::get<Interface::ProgressRing*>(second_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Right);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 1);

							break;
						}
					// ProgressBar
					case 8:
						{
							const auto& pElement = static_cast<AppProgressBar*>(
								std::get<Interface::ProgressBar*>(second_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Right);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 1);

							break;
						}
					}
				}

				// Append the created grid to the base stack
				_ptr_stack_panel.get()->Children().Append(_grid);
			}

			// Append a One-Row element vector : */*/.../* column space
			void AppendElementVector(const std::vector<Interface::Element>
				& element_vector) override
			{
				// Set up a placeholder horizontally divided grid
				Controls::Grid _grid;
				_grid.HorizontalAlignment(HorizontalAlignment::Stretch);

				// Parse and append the elements
				for (uint32_t i = 0; i < element_vector.size(); i++)
				{
					// Append a column
					Controls::AppendGridStarColumn(_grid);

					// Parse the alignment : Centered by default
					auto _alignment = HorizontalAlignment::Center;

					// If there are more than 2 elements,
					// snap the first one to the left and the last one to the right
					if (element_vector.size() > 1) // 2 or more elements
					{
						if (i == 0)
							_alignment = HorizontalAlignment::Left;
						else if (i == element_vector.size() - 1)
							_alignment = HorizontalAlignment::Right;
					}

					// Switch based on element type: all types
					switch (const auto& element = element_vector.at(i);
						element.index())
					{
					// TextBlock
					case 0:
						{
							const auto& pElement = static_cast<AppTextBlock*>(
								std::get<Interface::TextBlock*>(element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(_alignment);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), i);

							break;
						}
					// Button
					case 1:
						{
							const auto& pElement = static_cast<AppButton*>(
								std::get<Interface::Button*>(element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(_alignment);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), i);

							break;
						}
					// NumberBox
					case 2:
						{
							const auto& pElement = static_cast<AppNumberBox*>(
								std::get<Interface::NumberBox*>(element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(_alignment);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), i);

							break;
						}
					// ComboBox
					case 3:
						{
							const auto& pElement = static_cast<AppComboBox*>(
								std::get<Interface::ComboBox*>(element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(_alignment);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), i);

							break;
						}
					// CheckBox
					case 4:
						{
							const auto& pElement = static_cast<AppCheckBox*>(
								std::get<Interface::CheckBox*>(element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(_alignment);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), i);

							break;
						}
					// ToggleSwitch
					case 5:
						{
							const auto& pElement = static_cast<AppToggleSwitch*>(
								std::get<Interface::ToggleSwitch*>(element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(_alignment);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), i);

							break;
						}
					// TextBox
					case 6:
						{
							const auto& pElement = static_cast<AppTextBox*>(
								std::get<Interface::TextBox*>(element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(_alignment);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), i);

							break;
						}
					// ProgressRing
					case 7:
						{
							const auto& pElement = static_cast<AppProgressRing*>(
								std::get<Interface::ProgressRing*>(element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(_alignment);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), i);

							break;
						}
					// ProgressBar
					case 8:
						{
							const auto& pElement = static_cast<AppProgressBar*>(
								std::get<Interface::ProgressBar*>(element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(_alignment);

							(*pElement).Get().get()->Margin({3, 3, 3, 3});

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), i);

							break;
						}
					}
				}

				// Append the created grid to the base stack
				_ptr_stack_panel.get()->Children().Append(_grid);
			}

			// Creation: register a host
			void Create()
			{
				// Create a XAML number box
				Controls::StackPanel _stack_panel;
				_stack_panel.Orientation(Controls::Orientation::Vertical);

				_stack_panel.HorizontalAlignment(HorizontalAlignment::Stretch);
				_stack_panel.VerticalAlignment(VerticalAlignment::Stretch);

				// Back it up
				_ptr_stack_panel = std::make_shared<Controls::StackPanel>(_stack_panel);
			}

			// Get the underlying shared pointer
			std::shared_ptr<Controls::StackPanel> Get()
			{
				return _ptr_stack_panel;
			}

		protected:
			std::shared_ptr<Controls::StackPanel> _ptr_stack_panel;
		};

		// Create a server-side ui element (not sliced, server-only)
		inline AppTextBlock* CreateAppTextBlock(const std::wstring& text)
		{
			return new AppTextBlock(text);
		}

		inline Interface::TextBlock* CreateAppTextBlock_Sliced(const std::wstring& text)
		{
			return new AppTextBlock(text);
		}


		// Create a server-side ui element (not sliced, server-only)
		inline AppButton* CreateAppButton(const std::wstring& content)
		{
			return new AppButton(content);
		}

		// Create a client-side ui element (sliced)
		inline Interface::Button* CreateAppButton_Sliced(const std::wstring& content)
		{
			return new AppButton(content);
		}


		// Create a server-side ui element (not sliced, server-only)
		inline AppNumberBox* CreateAppNumberBox(const int& value = 0)
		{
			return new AppNumberBox(value);
		}

		// Create a client-side ui element (sliced)
		inline Interface::NumberBox* CreateAppNumberBox_Sliced(const int& value = 0)
		{
			return new AppNumberBox(value);
		}


		// Create a server-side ui element (not sliced, server-only)
		inline AppComboBox* CreateAppComboBox(const std::vector<std::wstring>& entries)
		{
			return new AppComboBox(entries);
		}

		// Create a client-side ui element (sliced)
		inline Interface::ComboBox* CreateAppComboBox_Sliced(const std::vector<std::wstring>& entries)
		{
			return new AppComboBox(entries);
		}


		// Create a server-side ui element (not sliced, server-only)
		inline AppCheckBox* CreateAppCheckBox()
		{
			return new AppCheckBox();
		}

		// Create a client-side ui element (sliced)
		inline Interface::CheckBox* CreateAppCheckBox_Sliced()
		{
			return new AppCheckBox();
		}


		// Create a server-side ui element (not sliced, server-only)
		inline AppToggleSwitch* CreateAppToggleSwitch()
		{
			return new AppToggleSwitch();
		}

		// Create a client-side ui element (sliced)
		inline Interface::ToggleSwitch* CreateAppToggleSwitch_Sliced()
		{
			return new AppToggleSwitch();
		}


		// Create a server-side ui element (not sliced, server-only)
		inline AppTextBox* CreateAppTextBox()
		{
			return new AppTextBox();
		}

		// Create a client-side ui element (sliced)
		inline Interface::TextBox* CreateAppTextBox_Sliced()
		{
			return new AppTextBox();
		}


		// Create a server-side ui element (not sliced, server-only)
		inline AppProgressRing* CreateAppProgressRing(const int32_t& progress = -1)
		{
			return new AppProgressRing(progress);
		}

		// Create a client-side ui element (sliced)
		inline Interface::ProgressRing* CreateAppProgressRing_Sliced()
		{
			return new AppProgressRing(-1);
		}


		// Create a server-side ui element (not sliced, server-only)
		inline AppProgressBar* CreateAppProgressBar(const int32_t& progress = -1)
		{
			return new AppProgressBar(progress);
		}

		// Create a client-side ui element (sliced)
		inline Interface::ProgressBar* CreateAppProgressBar_Sliced()
		{
			return new AppProgressBar(-1);
		}
	}

	// Empty layout root for placeholders
	inline AppInterface::AppLayoutRoot* emptyLayoutRoot;
}
