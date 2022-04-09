#pragma once
#include "pch.h"

#include "K2EVRInput.h"
#include "K2Shared.h"

namespace k2app
{
	namespace interfacing
	{
		// Internal version number
		inline const std::string K2InternalVersion = "1.0.1.3"; // KTVR[ver:X.X.X.X]

		inline std::vector<K2AppTracker> K2TrackersVector{
			K2AppTracker("AME-00WAIST0", ktvr::ITrackerType::Tracker_Waist),
			K2AppTracker("AME-L0FOOT00", ktvr::ITrackerType::Tracker_LeftFoot),
			K2AppTracker("AME-R0FOOT00", ktvr::ITrackerType::Tracker_RightFoot),
			K2AppTracker("AME-L0ELBOW0", ktvr::ITrackerType::Tracker_LeftElbow, 0.41),
			K2AppTracker("AME-R0ELBOW0", ktvr::ITrackerType::Tracker_RightElbow, 0.41),
			K2AppTracker("AME-L0KNEE00", ktvr::ITrackerType::Tracker_LeftKnee),
			K2AppTracker("AME-R0KNEE00", ktvr::ITrackerType::Tracker_RightKnee)
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
			if (header.empty() || text.empty())return;

			winrt::hstring payload =
				wstring_cast(R"(<toast>
						<visual>
							<binding template = "ToastGeneric">
								<text>)" + header + R"(</text>"
								<text>)" + text + R"(</text>
							</binding>
						</visual>
					</toast>)").c_str();

			winrt::Microsoft::Windows::AppNotifications::AppNotification toast(payload);
			toast.Tag(L"Tag_AmeNotifications");
			toast.Group(L"Group_AmeNotifications");

			shared::main::thisNotificationManager.get()->Show(toast);
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

				K2TrackersVector.at(3).id = 3; // LE
				K2TrackersVector.at(4).id = 4; // RE
				K2TrackersVector.at(5).id = 5; // LK
				K2TrackersVector.at(6).id = 6; // RK

				LOG(INFO) << "[K2Interfacing] App will be using K2Driver's default prepended trackers!";

				// Helper bool array
				std::array<bool, 7> spawned = {false, false, false, false, false, false, false};

				// Try 3 times
				for (int i = 0; i < 3; i++)
				{
					// Add only default trackers from the vector (0-2) & (3-6)
					for (int t = 0; t < 7; t++)
					{
						if (k2app::K2Settings.isJointPairEnabled[floor((t + 1) / 2)])
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
							LOG(ERROR) << "Not spawning this tracker since it's disabled, marking as success.";
							spawned[t] = true;
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
				LOG(ERROR) << "IVRSystem could not be initialized: EVRInitError Code " << static_cast<int>(eError);
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

			double yaw = std::atan2(trackingOrigin.m[0][2], trackingOrigin.m[2][2]);
			if (yaw < 0.0)
				yaw = 2 * _PI + yaw;

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
		inline std::pair<bool, uint32_t> findVRWaistTracker(const bool _log = true)
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
				if (findStringIC(buf, "waist"))
				{
					char buf_p[1024];
					vr::VRSystem()->
						GetStringTrackedDeviceProperty(i, vr::Prop_SerialNumber_String, buf_p, sizeof buf_p);

					// Log that we're finished
					LOG_IF(INFO, _log) <<
						"\nFound an active waist tracker with:\n    hint: " <<
						buf << "\n    serial: " <<
						buf_p << "\n    id: " << i;

					// Return what we've got
					return std::make_pair(true, i);
				}
			}

			// We've failed if the loop's finished
			LOG_IF(WARNING, _log) <<
				"Didn't find any waist tracker in SteamVR with a proper role hint (Prop_ControllerType_String)";
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

			const auto waistPair = findVRWaistTracker(_log);

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

		// Check if we've disabled any joints from spawning and disable their mods
		inline void devices_check_disabled_joints()
		{
			using namespace shared::devices;

			// Ditch this if not loaded yet
			if (waistJointOptionBox.get() == nullptr)return;

			// Optionally fix combos for disabled trackers -> joint selectors for base
			waistJointOptionBox.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[0]);
			if (!k2app::K2Settings.isJointPairEnabled[0])
				waistJointOptionBox.get()->SelectedIndex(-1); // Show the placeholder

			leftFootJointOptionBox.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[1]);
			if (!k2app::K2Settings.isJointPairEnabled[1])
				leftFootJointOptionBox.get()->SelectedIndex(-1); // Show the placeholder

			rightFootJointOptionBox.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[1]);
			if (!k2app::K2Settings.isJointPairEnabled[1])
				rightFootJointOptionBox.get()->SelectedIndex(-1); // Show the placeholder

			leftElbowJointOptionBox.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[2]);
			if (!k2app::K2Settings.isJointPairEnabled[2])
				leftElbowJointOptionBox.get()->SelectedIndex(-1); // Show the placeholder

			rightElbowJointOptionBox.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[2]);
			if (!k2app::K2Settings.isJointPairEnabled[2])
				rightElbowJointOptionBox.get()->SelectedIndex(-1); // Show the placeholder

			leftKneeJointOptionBox.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[3]);
			if (!k2app::K2Settings.isJointPairEnabled[3])
				leftKneeJointOptionBox.get()->SelectedIndex(-1); // Show the placeholder

			rightKneeJointOptionBox.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[3]);
			if (!k2app::K2Settings.isJointPairEnabled[3])
				rightKneeJointOptionBox.get()->SelectedIndex(-1); // Show the placeholder

			// Optionally fix combos for disabled trackers -> joint selectors for override
			waistPositionOverrideOptionBox.get()->IsEnabled(
				k2app::K2Settings.isJointPairEnabled[0] && k2app::K2Settings.isPositionOverriddenJoint[0]);
			waistRotationOverrideOptionBox.get()->IsEnabled(
				k2app::K2Settings.isJointPairEnabled[0] && k2app::K2Settings.isRotationOverriddenJoint[0]);

			overrideWaistPosition.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[0]);
			overrideWaistRotation.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[0]);

			if (!k2app::K2Settings.isJointPairEnabled[0])
			{
				overrideWaistPosition.get()->IsChecked(false);
				overrideWaistRotation.get()->IsChecked(false);

				waistPositionOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
				waistRotationOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
			}

			leftFootPositionOverrideOptionBox.get()->IsEnabled(
				k2app::K2Settings.isJointPairEnabled[1] && k2app::K2Settings.isPositionOverriddenJoint[1]);
			leftFootRotationOverrideOptionBox.get()->IsEnabled(
				k2app::K2Settings.isJointPairEnabled[1] && k2app::K2Settings.isRotationOverriddenJoint[1]);

			overrideLeftFootPosition.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[1]);
			overrideLeftFootRotation.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[1]);

			if (!k2app::K2Settings.isJointPairEnabled[1])
			{
				overrideLeftFootPosition.get()->IsChecked(false);
				overrideLeftFootRotation.get()->IsChecked(false);

				leftFootPositionOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
				leftFootRotationOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
			}

			rightFootPositionOverrideOptionBox.get()->IsEnabled(
				k2app::K2Settings.isJointPairEnabled[1] && k2app::K2Settings.isPositionOverriddenJoint[2]);
			rightFootRotationOverrideOptionBox.get()->IsEnabled(
				k2app::K2Settings.isJointPairEnabled[1] && k2app::K2Settings.isRotationOverriddenJoint[2]);

			overrideRightFootPosition.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[1]);
			overrideRightFootRotation.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[1]);

			if (!k2app::K2Settings.isJointPairEnabled[1])
			{
				overrideRightFootPosition.get()->IsChecked(false);
				overrideRightFootRotation.get()->IsChecked(false);

				rightFootPositionOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
				rightFootRotationOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
			}

			leftElbowPositionOverrideOptionBox.get()->IsEnabled(
				k2app::K2Settings.isJointPairEnabled[2] && k2app::K2Settings.isPositionOverriddenJoint[3]);
			leftElbowRotationOverrideOptionBox.get()->IsEnabled(
				k2app::K2Settings.isJointPairEnabled[2] && k2app::K2Settings.isRotationOverriddenJoint[3]);

			overrideLeftElbowPosition.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[2]);
			overrideLeftElbowRotation.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[2]);

			if (!k2app::K2Settings.isJointPairEnabled[2])
			{
				overrideLeftElbowPosition.get()->IsChecked(false);
				overrideLeftElbowRotation.get()->IsChecked(false);

				leftElbowPositionOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
				leftElbowRotationOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
			}

			rightElbowPositionOverrideOptionBox.get()->IsEnabled(
				k2app::K2Settings.isJointPairEnabled[2] && k2app::K2Settings.isPositionOverriddenJoint[4]);
			rightElbowRotationOverrideOptionBox.get()->IsEnabled(
				k2app::K2Settings.isJointPairEnabled[2] && k2app::K2Settings.isRotationOverriddenJoint[4]);

			overrideRightElbowPosition.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[2]);
			overrideRightElbowRotation.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[2]);

			if (!k2app::K2Settings.isJointPairEnabled[2])
			{
				overrideRightElbowPosition.get()->IsChecked(false);
				overrideRightElbowRotation.get()->IsChecked(false);

				rightElbowPositionOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
				rightElbowRotationOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
			}

			leftKneePositionOverrideOptionBox.get()->IsEnabled(
				k2app::K2Settings.isJointPairEnabled[3] && k2app::K2Settings.isPositionOverriddenJoint[5]);
			leftKneeRotationOverrideOptionBox.get()->IsEnabled(
				k2app::K2Settings.isJointPairEnabled[3] && k2app::K2Settings.isRotationOverriddenJoint[5]);

			overrideLeftKneePosition.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[3]);
			overrideLeftKneeRotation.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[3]);

			if (!k2app::K2Settings.isJointPairEnabled[3])
			{
				overrideLeftKneePosition.get()->IsChecked(false);
				overrideLeftKneeRotation.get()->IsChecked(false);

				leftKneePositionOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
				leftKneeRotationOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
			}

			rightKneePositionOverrideOptionBox.get()->IsEnabled(
				k2app::K2Settings.isJointPairEnabled[3] && k2app::K2Settings.isPositionOverriddenJoint[6]);
			rightKneeRotationOverrideOptionBox.get()->IsEnabled(
				k2app::K2Settings.isJointPairEnabled[3] && k2app::K2Settings.isRotationOverriddenJoint[6]);

			overrideRightKneePosition.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[3]);
			overrideRightKneeRotation.get()->IsEnabled(k2app::K2Settings.isJointPairEnabled[3]);

			if (!k2app::K2Settings.isJointPairEnabled[3])
			{
				overrideRightKneePosition.get()->IsChecked(false);
				overrideRightKneeRotation.get()->IsChecked(false);

				rightKneePositionOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
				rightKneeRotationOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
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
						yaw = 2 * _PI + yaw;
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

			// Note: this is in radians
			inline float plugins_getHMDOrientationYaw()
			{
				return std::get<float>(vrHMDPose);
			}

			inline std::array<ktvr::K2TrackedJoint, 7> plugins_getAppJointPoses()
			{
				return std::array<ktvr::K2TrackedJoint, 7>
				{
					K2TrackersVector.at(0).getK2TrackedJoint(K2Settings.isJointPairEnabled[0], "Waist"),
					K2TrackersVector.at(1).getK2TrackedJoint(K2Settings.isJointPairEnabled[1], "Left Foot"),
					K2TrackersVector.at(2).getK2TrackedJoint(K2Settings.isJointPairEnabled[1], "Right Foot"),
					K2TrackersVector.at(3).getK2TrackedJoint(K2Settings.isJointPairEnabled[2], "Left Elbow"),
					K2TrackersVector.at(4).getK2TrackedJoint(K2Settings.isJointPairEnabled[2], "Right Elbow"),
					K2TrackersVector.at(5).getK2TrackedJoint(K2Settings.isJointPairEnabled[3], "Left Knee"),
					K2TrackersVector.at(6).getK2TrackedJoint(K2Settings.isJointPairEnabled[3], "Right Knee"),
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

		namespace AppInterface
		{
			using namespace winrt::Microsoft::UI::Xaml;
			using namespace ktvr;

			class AppTextBlock final : public Interface::TextBlock
			{
			public:
				AppTextBlock(std::string const& text)
				{
					Create(text);
				}

				/* XAML-Derived functions & handlers */

				// Text Get and Set
				std::string Text() override
				{
					if (_ptr_text_block.get())
						return string_cast(
							_ptr_text_block.get()->Text().c_str());
					else return "";
				}

				void Text(std::string const& text) override
				{
					if (_ptr_text_block.get())
						_ptr_text_block.get()->Text(
							wstring_cast(text));
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
				void Create(std::string const& text)
				{
					// Create a XAML button
					Controls::TextBlock _text_block;
					_text_block.Text(wstring_cast(text).c_str());

					// Back it up
					_ptr_text_block = std::make_shared<Controls::TextBlock>(_text_block);
				}
			};

			class AppButton final : public Interface::Button
			{
			public:
				AppButton(std::string const& content)
				{
					Create(content);
				}

				/* XAML-Derived functions & handlers */

				// IsEnabled Get and Set
				bool IsEnabled() override
				{
					if (_ptr_button.get())
						return _ptr_button.get()->IsEnabled();
					else return true;
				}

				void IsEnabled(bool const& enabled) override
				{
					if (_ptr_button.get())
						_ptr_button.get()->IsEnabled(enabled);
				}

				// Label Set
				void Content(std::string const& content) override
				{
					if (_ptr_button.get())
						_ptr_button.get()->Content(
							winrt::box_value(wstring_cast(content)));
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
				void Create(std::string const& content)
				{
					// Create a XAML button
					Controls::Button _button;
					_button.Content(
						winrt::box_value(wstring_cast(content).c_str()));

					// Back it up
					_ptr_button = std::make_shared<Controls::Button>(_button);

					// Create a dummy callback
					std::function<void(
						winrt::Windows::Foundation::IInspectable const& sender,
						winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)>
						const _n_callback = [this](winrt::Windows::Foundation::IInspectable const& sender,
							winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e) ->
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

				AppNumberBox(int const& value)
				{
					Create(value);
				}

				/* XAML-Derived functions & handlers */

				// IsEnabled Get and Set
				bool IsEnabled() override
				{
					if (_ptr_number_box.get())
						return _ptr_number_box.get()->IsEnabled();
					else return true;
				}

				void IsEnabled(bool const& enabled) override
				{
					if (_ptr_number_box.get())
						_ptr_number_box.get()->IsEnabled(enabled);
				}

				// Value Get and Set
				int Value() override
				{
					if (_ptr_number_box.get())
						return (int)_ptr_number_box.get()->Value();
					else return 0;
				}

				void Value(int const& value) override
				{
					if (_ptr_number_box.get())
						_ptr_number_box.get()->Value(value);
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
				void Create(int const& value)
				{
					// Create a XAML number box
					Controls::NumberBox _number_box;
					_number_box.Value(value);

					_number_box.SpinButtonPlacementMode(Controls::NumberBoxSpinButtonPlacementMode::Compact);
					_number_box.SmallChange(1);
					_number_box.LargeChange(10);

					// Back it up
					_ptr_number_box = std::make_shared<Controls::NumberBox>(_number_box);

					// Create a dummy callback
					std::function<void(
						winrt::Windows::Foundation::IInspectable const& sender,
						winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& e)>
						const _n_callback = [this](winrt::Windows::Foundation::IInspectable const& sender,
							winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& e) ->
						void
					{
						if (OnValueChanged) // Check if not null
							OnValueChanged(this, (int)e.NewValue());
					};

					// Set up the click handler to point to the base's one
					_number_box.ValueChanged(_n_callback);
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

				// IsEnabled Get and Set
				bool IsEnabled() override
				{
					if (_ptr_check_box.get())
						return _ptr_check_box.get()->IsEnabled();
					else return true;
				}

				void IsEnabled(bool const& enabled) override
				{
					if (_ptr_check_box.get())
						_ptr_check_box.get()->IsEnabled(enabled);
				}

				// IsChecked Get and Set
				bool IsChecked() override
				{
					if (_ptr_check_box.get())
						return _ptr_check_box.get()->IsChecked().Value();
					else return false;
				}

				void IsChecked(bool const& is_checked) override
				{
					if (_ptr_check_box.get())
						_ptr_check_box.get()->IsChecked(is_checked);
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
					std::function<void(
						winrt::Windows::Foundation::IInspectable const& sender,
						winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)>
						const _n_callback_checked = [this](winrt::Windows::Foundation::IInspectable const& sender,
							winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e) ->
						void
					{
						if (OnChecked) // Check if not null
							OnChecked(this);
					};

					// Create a dummy callback
					std::function<void(
						winrt::Windows::Foundation::IInspectable const& sender,
						winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)>
						const _n_callback_unchecked = [this](winrt::Windows::Foundation::IInspectable const& sender,
							winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e) ->
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

				// IsEnabled Get and Set
				bool IsEnabled() override
				{
					if (_ptr_toggle_switch.get())
						return _ptr_toggle_switch.get()->IsEnabled();
					else return true;
				}

				void IsEnabled(bool const& enabled) override
				{
					if (_ptr_toggle_switch.get())
						_ptr_toggle_switch.get()->IsEnabled(enabled);
				}

				// IsChecked Get and Set
				bool IsChecked() override
				{
					if (_ptr_toggle_switch.get())
						return _ptr_toggle_switch.get()->IsOn();
					else return false;
				}

				void IsChecked(bool const& is_checked) override
				{
					if (_ptr_toggle_switch.get())
						_ptr_toggle_switch.get()->IsOn(is_checked);
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
					std::function<void(
						winrt::Windows::Foundation::IInspectable const& sender,
						winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)>
						const _n_callback = [this](winrt::Windows::Foundation::IInspectable const& sender,
							winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e) ->
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

				// Text Get and Set
				std::string Text() override
				{
					if (_ptr_text_box.get())
						return string_cast(
							_ptr_text_box.get()->Text().c_str());
					else return "";
				}

				void Text(std::string const& text) override
				{
					if (_ptr_text_box.get())
						_ptr_text_box.get()->Text(
							wstring_cast(text));
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
					// Create a XAML button
					Controls::TextBox _text_box;

					// Back it up
					_ptr_text_box = std::make_shared<Controls::TextBox>(_text_box);

					// Create a dummy callback
					std::function<void(
						winrt::Windows::Foundation::IInspectable const& sender,
						winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)>
						const _n_callback = [this](winrt::Windows::Foundation::IInspectable const& sender,
							winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e) ->
						void
					{
						if (e.Key() == winrt::Windows::System::VirtualKey::Enter)
							OnEnterKeyDown(this);
					};

					// Set up the click handler to point to the base's one
					_text_box.KeyDown(_n_callback);
				}
			};

			inline HorizontalAlignment horizontalAlignmentConverter(
				Interface::SingleLayoutHorizontalAlignment const& alignment)
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

			inline void AppendGridStarColumn(Controls::Grid& _grid)
			{
				Controls::ColumnDefinition _col;
				_col.Width(GridLengthHelper::FromValueAndType(1, GridUnitType::Star));

				_grid.ColumnDefinitions().Append(_col);
			}

			class AppLayoutRoot final : public Interface::LayoutRoot
			{
			public:
				AppLayoutRoot()
				{
					AppLayoutRoot::Create();
				}

				// Append a One-Row single element
				void AppendSingleElement(
					Interface::Element const& element,
					Interface::SingleLayoutHorizontalAlignment const& alignment) override
				{
					// Switch based on element type: all types
					switch (element.index())
					{
						// TextBlock
					case 0:
					{
						auto const& pElement = static_cast<AppTextBlock*>(
							std::get<Interface::TextBlock*>(element));

						(*pElement).Get().get()->VerticalAlignment(
							VerticalAlignment::Center);

						(*pElement).Get().get()->HorizontalAlignment(
							horizontalAlignmentConverter(alignment));

						_ptr_stack_panel.get()->Children().Append(*(*pElement).Get());

						return;
					}
					// Button
					case 1:
					{
						auto const& pElement = static_cast<AppButton*>(std::get<Interface::Button*>(element));

						(*pElement).Get().get()->VerticalAlignment(
							VerticalAlignment::Center);

						(*pElement).Get().get()->HorizontalAlignment(
							horizontalAlignmentConverter(alignment));

						_ptr_stack_panel.get()->Children().Append(*(*pElement).Get());

						return;
					}
					// NumberBox
					case 2:
					{
						auto const& pElement = static_cast<AppNumberBox*>(
							std::get<Interface::NumberBox*>(element));

						(*pElement).Get().get()->VerticalAlignment(
							VerticalAlignment::Center);

						(*pElement).Get().get()->HorizontalAlignment(
							horizontalAlignmentConverter(alignment));

						_ptr_stack_panel.get()->Children().Append(*(*pElement).Get());

						return;
					}
					// CheckBox
					case 3:
					{
						auto const& pElement = static_cast<AppCheckBox*>(
							std::get<Interface::CheckBox*>(element));

						(*pElement).Get().get()->VerticalAlignment(
							VerticalAlignment::Center);

						(*pElement).Get().get()->HorizontalAlignment(
							horizontalAlignmentConverter(alignment));

						_ptr_stack_panel.get()->Children().Append(*(*pElement).Get());

						return;
					}
					// ToggleSwitch
					case 4:
					{
						auto const& pElement = static_cast<AppToggleSwitch*>(
							std::get<Interface::ToggleSwitch*>(element));

						(*pElement).Get().get()->VerticalAlignment(
							VerticalAlignment::Center);

						(*pElement).Get().get()->HorizontalAlignment(
							horizontalAlignmentConverter(alignment));

						_ptr_stack_panel.get()->Children().Append(*(*pElement).Get());

						return;
					}
					// TextBox
					case 5:
					{
						auto const& pElement = static_cast<AppTextBox*>(
							std::get<Interface::TextBox*>(element));

						(*pElement).Get().get()->VerticalAlignment(
							VerticalAlignment::Center);

						(*pElement).Get().get()->HorizontalAlignment(
							horizontalAlignmentConverter(alignment));

						_ptr_stack_panel.get()->Children().Append(*(*pElement).Get());

						return;
					}
					}
				}

				// Append a One-Row element pair : */* column space
				void AppendElementPair(Interface::Element const& first_element,
					Interface::Element const& second_element) override
				{
					// Set up a placeholder horizontally divided grid
					Controls::Grid _grid;
					_grid.HorizontalAlignment(HorizontalAlignment::Stretch);

					AppendGridStarColumn(_grid);
					AppendGridStarColumn(_grid);

					// Parse and append the first pair element
					{
						// Switch based on element type: all types
						switch (first_element.index())
						{
							// TextBlock
						case 0:
						{
							auto const& pElement = static_cast<AppTextBlock*>(
								std::get<Interface::TextBlock*>(first_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 0);

							break;
						}
						// Button
						case 1:
						{
							auto const& pElement = static_cast<AppButton*>(
								std::get<Interface::Button*>(first_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 0);

							break;
						}
						// NumberBox
						case 2:
						{
							auto const& pElement = static_cast<AppNumberBox*>(
								std::get<Interface::NumberBox*>(first_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 0);

							break;
						}
						// CheckBox
						case 3:
						{
							auto const& pElement = static_cast<AppCheckBox*>(
								std::get<Interface::CheckBox*>(first_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 0);

							break;
						}
						// ToggleSwitch
						case 4:
						{
							auto const& pElement = static_cast<AppToggleSwitch*>(
								std::get<Interface::ToggleSwitch*>(first_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 0);

							break;
						}
						// TextBox
						case 5:
						{
							auto const& pElement = static_cast<AppTextBox*>(
								std::get<Interface::TextBox*>(first_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

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
							auto const& pElement = static_cast<AppTextBlock*>(
								std::get<Interface::TextBlock*>(second_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 1);

							break;
						}
						// Button
						case 1:
						{
							auto const& pElement = static_cast<AppButton*>(
								std::get<Interface::Button*>(second_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 1);

							break;
						}
						// NumberBox
						case 2:
						{
							auto const& pElement = static_cast<AppNumberBox*>(
								std::get<Interface::NumberBox*>(second_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 1);

							break;
						}
						// CheckBox
						case 3:
						{
							auto const& pElement = static_cast<AppCheckBox*>(
								std::get<Interface::CheckBox*>(second_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 1);

							break;
						}
						// ToggleSwitch
						case 4:
						{
							auto const& pElement = static_cast<AppToggleSwitch*>(
								std::get<Interface::ToggleSwitch*>(second_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 1);

							break;
						}
						// TextBox
						case 5:
						{
							auto const& pElement = static_cast<AppTextBox*>(
								std::get<Interface::TextBox*>(second_element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), 1);

							break;
						}
						}
					}

					// Append the created grid to the base stack
					_ptr_stack_panel.get()->Children().Append(_grid);
				}

				// Append a One-Row element pair : */* column space
				void AppendElementVector(std::vector<Interface::Element>
					const& element_vector) override
				{
					// Set up a placeholder horizontally divided grid
					Controls::Grid _grid;
					_grid.HorizontalAlignment(HorizontalAlignment::Stretch);

					// Parse and append the elements
					for (uint32_t i = 0; i < element_vector.size(); i++)
					{
						// Append a column
						AppendGridStarColumn(_grid);

						// Switch based on element type: all types
						switch (auto const& element = element_vector.at(i);
						element.index())
						{
							// TextBlock
						case 0:
						{
							auto const& pElement = static_cast<AppTextBlock*>(
								std::get<Interface::TextBlock*>(element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), i);

							break;
						}
						// Button
						case 1:
						{
							auto const& pElement = static_cast<AppButton*>(
								std::get<Interface::Button*>(element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), i);

							break;
						}
						// NumberBox
						case 2:
						{
							auto const& pElement = static_cast<AppNumberBox*>(
								std::get<Interface::NumberBox*>(element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), i);

							break;
						}
						// CheckBox
						case 3:
						{
							auto const& pElement = static_cast<AppCheckBox*>(
								std::get<Interface::CheckBox*>(element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), i);

							break;
						}
						// ToggleSwitch
						case 4:
						{
							auto const& pElement = static_cast<AppToggleSwitch*>(
								std::get<Interface::ToggleSwitch*>(element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

							_grid.Children().Append(*(*pElement).Get());
							_grid.SetColumn(*(*pElement).Get(), i);

							break;
						}
						// TextBox
						case 5:
						{
							auto const& pElement = static_cast<AppTextBox*>(
								std::get<Interface::TextBox*>(element));

							(*pElement).Get().get()->VerticalAlignment(
								VerticalAlignment::Center);

							(*pElement).Get().get()->HorizontalAlignment(
								HorizontalAlignment::Center);

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
			inline AppTextBlock* CreateAppTextBlock(std::string const& text)
			{
				return new AppTextBlock(text);
			}

			inline Interface::TextBlock* CreateAppTextBlock_Sliced(std::string const& text)
			{
				return new AppTextBlock(text);
			}


			// Create a server-side ui element (not sliced, server-only)
			inline AppButton* CreateAppButton(std::string const& content)
			{
				return new AppButton(content);
			}

			// Create a client-side ui element (sliced)
			inline Interface::Button* CreateAppButton_Sliced(std::string const& content)
			{
				return new AppButton(content);
			}


			// Create a server-side ui element (not sliced, server-only)
			inline AppNumberBox* CreateAppNumberBox(int const& value = 0)
			{
				return new AppNumberBox(value);
			}

			// Create a client-side ui element (sliced)
			inline Interface::NumberBox* CreateAppNumberBox_Sliced(int const& value = 0)
			{
				return new AppNumberBox(value);
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
		}
	}
}
