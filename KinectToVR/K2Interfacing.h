#pragma once
#include "pch.h"

#include "K2EVRInput.h"
#include "K2Shared.h"

namespace k2app
{
	namespace interfacing
	{
		inline std::vector<K2AppTracker> K2TrackersVector{
			K2AppTracker("LHR-CB9AD1T0", ktvr::ITrackerType::Tracker_Waist),
			K2AppTracker("LHR-CB9AD1T1", ktvr::ITrackerType::Tracker_LeftFoot),
			K2AppTracker("LHR-CB9AD1T2", ktvr::ITrackerType::Tracker_RightFoot)
		};

		inline std::pair<Eigen::Vector3f, Eigen::Vector3f> // Position helpers for k2 devices -> Base, Override
			kinectHeadPosition{Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 0, 0)}, // But this one's kinect-only
			kinectWaistPosition{Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 0, 0)}; // This one applies to both bases

		// OpenVR playspace position
		inline Eigen::Vector3f vrPlayspaceTranslation = Eigen::Vector3f(0, 0, 0);
		// OpenVR playspace rotation
		inline float vrPlayspaceOrientation = 0.f; // Note: radians

		inline void ShowToast(std::string const& header, std::string const& text)
		{
			// Unsupported in AppSDK 1.0

			//using namespace winrt::Windows::UI::Notifications;
			//using namespace winrt::Windows::Data::Xml::Dom;

			//// Construct the XML toast template
			//XmlDocument document;
			//document.LoadXml(L"\
			//	<toast>\
			//		<visual>\
			//	        <binding template=\"ToastGeneric\">\
			//	            <text></text>\
			//	            <text></text>\
			//	        </binding>\
			//	    </visual>\
			//	</toast>");

			//// Populate with text and values
			//document.SelectSingleNode(L"//text[1]").InnerText(wstring_cast(header));
			//document.SelectSingleNode(L"//text[2]").InnerText(wstring_cast(text));

			//// Construct the notification
			//ToastNotification notification{ document };
			//ToastNotifier toastNotifier{ ToastNotificationManager::CreateToastNotifier() };

			//// And show it!
			//toastNotifier.Show(notification);
		}

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
		inline std::string serverStatusString = " \n \n ";

		// For manual calibration
		inline bool calibration_confirm,
		            calibration_modeSwap,
		            calibration_fineTune;

		// For manual calibration: L, R -> X, Y
		inline std::array<std::array<float, 2>, 2>
		calibration_joystick_positions;

		// Function to spawn default' enabled trackers
		inline bool SpawnDefaultEnabledTrackers()
		{
			if (!K2AppTrackersSpawned)
			{
				LOG(INFO) << "[K2Interfacing] Registering trackers now...";

				// K2Driver is now auto-adding default lower body trackers.
				// That means that ids are: W-0 L-1 R-2
				// We may skip downloading them then ^_~

				// Setup default IDs
				K2TrackersVector.at(0).id = 0; // W
				K2TrackersVector.at(1).id = 1; // L
				K2TrackersVector.at(2).id = 2; // R

				LOG(INFO) << "[K2Interfacing] App will be using K2Driver's default prepended trackers!";

				// Helper bool array
				std::array<bool, 3> spawned = {false, false, false};

				// Try 3 times
				for (int i = 0; i < 3; i++)
				{
					// Add only default trackers from the vector (0-2)
					for (int t = 0; t < 3; t++)
					{
						if (k2app::K2Settings.isJointEnabled[t])
						{
							if (K2TrackersVector.at(t).id != -1)
							{
								if (const auto& m_result =
										ktvr::set_tracker_state(K2TrackersVector.at(t).id, true); // We WANT a reply
									m_result.id == K2TrackersVector.at(t).id && m_result.success)
								{
									LOG(INFO) << "Tracker with serial " + K2TrackersVector.at(t).data.serial +
										" and id " +
										std::to_string(
											K2TrackersVector.at(t).id) +
										" was successfully updated with status [active]";
									spawned[t] = true;
								}

								else if (m_result.id != K2TrackersVector.at(t).id && m_result.success)
									LOG(ERROR) << "Tracker with serial " + K2TrackersVector.at(t).data.serial + " and id "
										+
										std::to_string(
											K2TrackersVector.at(t).id) +
										" could not be spawned due to ID mismatch.";

								else
								{
									LOG(ERROR) << "Tracker with serial " + K2TrackersVector.at(t).data.serial +
										" and id " +
										std::to_string(
											K2TrackersVector.at(t).id) +
										" could not be spawned due to internal server error.";
									if (!ktvr::GetLastError().empty())
										LOG(ERROR) << "Last K2API error: " + ktvr::GetLastError();
								}
							}
							else
								LOG(ERROR) << "Not spawning active tracker since its id is -1";
						}
						else
						{
							spawned[t] = true; // Hacky hack
							LOG(INFO) << "Not spawning tracker with serial " + K2TrackersVector.at(t).data.serial +
								" because it is disabled in settings.";
						}
					}
				}

				// If one or more trackers failed to spawn
				if (std::ranges::find(spawned.begin(), spawned.end(), false) != spawned.end())
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
				LOG(ERROR) << "IVRSystem could not be initialized: EVRInitError Code " << static_cast<int>(eError);
				MessageBoxA(nullptr,
				            std::string(
					            "Couldn't initialise VR system. (Code " + std::to_string(eError) +
					            ")\n\nPlease check if SteamVR is installed (or running) and try again."
				            ).c_str(),
				            "IVRSystem Init Failure!",
				            MB_OK);

				return false; // Fail
			}

			// Since we're ok, capture playspace details
			const auto trackingOrigin = m_VRSystem->GetRawZeroPoseToStandingAbsoluteTrackingPose();

			vrPlayspaceTranslation = EigenUtils::p_cast_type<Eigen::Vector3f>(trackingOrigin);

			double yaw = std::atan2(trackingOrigin.m[0][2], trackingOrigin.m[2][2]);
			if (yaw < 0.0)
				yaw = 2 * 3.14159265358979323846 + yaw;

			vrPlayspaceOrientation = yaw;
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
						int(pingCheckingThreadsNumber) - 1, 0,
						int(maxPingCheckingThreads) + 1);

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
						int(pingCheckingThreadsNumber) - 1, 0,
						int(maxPingCheckingThreads) + 1);
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
			std::string server_status =
				"COULD NOT CHECK STATUS (Code -12)\nE_WTF\nSomething's fucked a really big time.";

			switch (serverDriverStatusCode)
			{
			case -10:
				server_status =
					"EXCEPTION WHILE CHECKING (Code -10)\nE_EXCEPTION_WHILE_CHECKING\nCheck SteamVR add-ons (NOT overlays) and enable KinectToVR.";
				break;
			case -1:
				server_status =
					"SERVER CONNECTION ERROR (Code -1)\nE_CONNECTION_ERROR\nYour KinectToVR SteamVR driver may be broken or outdated.";
				break;
			case 10:
				server_status =
					"FATAL SERVER FAILURE (Code 10)\nE_FATAL_SERVER_FAILURE\nPlease restart, check logs and write to us on Discord.";
				break;
			case 1:
				server_status = "Success! (Code 1)\nI_OK\nEverything's good!";
				isServerDriverPresent = true; // Change to success
				break;
			default:
				server_status =
					"COULD NOT CONNECT TO K2API (Code -11)\nE_K2API_FAILURE\nThis error shouldn't occur, actually. Something's wrong a big part.";
				break;
			}

			// LOG the status
			LOG(INFO) << "Current K2 Server status: " << server_status;
			serverStatusString = server_status;
		}

		inline void UpdateServerStatusUI()
		{
			// Update the status here
			using namespace winrt::Microsoft::UI::Xaml;

			// Disable UI (partially) if we've encountered an error
			if (::k2app::shared::main::devicesItem.get() != nullptr)
			{
				//::k2app::shared::main::settingsItem.get()->IsEnabled(isServerDriverPresent);
				::k2app::shared::main::devicesItem.get()->IsEnabled(isServerDriverPresent);
			}

			// Check with this one, should be the same for all anyway
			if (::k2app::shared::general::serverErrorWhatText.get() != nullptr)
			{
				::k2app::shared::general::serverErrorWhatText.get()->Visibility(
					isServerDriverPresent ? Visibility::Collapsed : Visibility::Visible);
				::k2app::shared::general::serverErrorWhatGrid.get()->Visibility(
					isServerDriverPresent ? Visibility::Collapsed : Visibility::Visible);
				::k2app::shared::general::serverErrorButtonsGrid.get()->Visibility(
					isServerDriverPresent ? Visibility::Collapsed : Visibility::Visible);
				::k2app::shared::general::serverErrorLabel.get()->Visibility(
					isServerDriverPresent ? Visibility::Collapsed : Visibility::Visible);

				// Split status and message by \n
				::k2app::shared::general::serverStatusLabel.get()->Text(
					wstring_cast(split_status(serverStatusString)[0]));
				::k2app::shared::general::serverErrorLabel.get()->Text(
					wstring_cast(split_status(serverStatusString)[1]));
				::k2app::shared::general::serverErrorWhatText.get()->Text(
					wstring_cast(split_status(serverStatusString)[2]));
			}

			// Block some things if server isn't working properly
			if (!isServerDriverPresent)
			{
				LOG(ERROR) <<
					"An error occurred and the app couldn't connect to K2 Server. Please check the upper message for more info.";

				if (::k2app::shared::general::errorWhatText.get() != nullptr)
				{
					LOG(INFO) << "[Server Error] Entering the server error state...";

					// Hide device error labels (if any)
					::k2app::shared::general::errorWhatText.get()->Visibility(Visibility::Collapsed);
					::k2app::shared::general::errorWhatGrid.get()->Visibility(Visibility::Collapsed);
					::k2app::shared::general::errorButtonsGrid.get()->Visibility(Visibility::Collapsed);
					::k2app::shared::general::trackingDeviceErrorLabel.get()->Visibility(
						Visibility::Collapsed);

					// Block spawn|offsets|calibration buttons, //disable autospawn for session (just don't save)
					::k2app::shared::general::toggleTrackersButton.get()->IsEnabled(false);
					::k2app::shared::general::calibrationButton.get()->IsEnabled(false);
					::k2app::shared::general::offsetsButton.get()->IsEnabled(false);
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
		inline std::tuple
		<
			Eigen::Vector3f, // Position
			Eigen::Quaternionf, // Rotation
			float // Rotation - Yaw
		>
		vrHMDPose
		{
			Eigen::Vector3f(0.f, 0.f, 0.f), // Init as zero
			Eigen::Quaternionf(1.f, 0.f, 0.f, 0.f), // Init as non-empty
			0.f // Init as facing front
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

					// Get the yaw
					double yaw = std::atan2(hmdPose.mDeviceToAbsoluteTracking.m[0][2],
					                        hmdPose.mDeviceToAbsoluteTracking.m[2][2]);

					// Fix the yaw
					if (yaw < 0.0)
					{
						yaw = 2 * 3.14159265358979323846 + yaw;
					}

					vrHMDPose = std::tie(position, quaternion, yaw);
				}
			}
		}

		namespace plugins
		{
			inline Eigen::Vector3f plugins_getHMDPosition()
			{
				return std::get<Eigen::Vector3f>(vrHMDPose);
			}

			inline Eigen::Quaternionf plugins_getHMDOrientation()
			{
				return std::get<Eigen::Quaternionf>(vrHMDPose);
			}

			inline float plugins_getHMDOrientationYaw()
			{
				return std::get<float>(vrHMDPose);
			}
		}
	}
}
