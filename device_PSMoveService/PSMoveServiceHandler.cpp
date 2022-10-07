#include "pch.h"
#include "PSMoveServiceHandler.h"

#define M_PI_2 1.57079632679

Eigen::Vector3d PSMSToEigen(PSMVector3f v)
{
	return .01 * Eigen::Vector3d(v.x, v.y, v.z);
}

Eigen::Quaterniond PSMSToEigen(PSMQuatf q)
{
	return {q.w, q.x, q.y, q.z};
}

HRESULT PSMoveServiceHandler::getStatusResult()
{
	Flags_SettingsSupported = initialized && m_deviceUsable; // Just in case

	if (initialized)
		return m_deviceUsable ? S_OK : E_PSMS_NO_JOINTS;

	// If failed
	return E_PSMS_NOT_RUNNING;
}

std::wstring PSMoveServiceHandler::statusResultWString(HRESULT stat)
{
	// Wrap status to string for readability
	if (_loaded)
		switch (stat)
		{
		case S_OK: return requestLocalizedString(L"/Plugins/PSMS/Statuses/Success");
		case E_PSMS_NOT_RUNNING: return requestLocalizedString(L"/Plugins/PSMS/Statuses/NotRunning");
		case E_PSMS_NO_JOINTS: return requestLocalizedString(L"/Plugins/PSMS/Statuses/NoJoints");
		default: return L"Undefined: " + std::to_wstring(stat) +
				L"\nE_UNDEFINED\nSomething weird has happened, though we can't tell what.";
		}

	return L"Undefined: " + std::to_wstring(stat) +
		L"\nE_UNDEFINED\nSomething weird has happened, though we can't tell what.";
}

void PSMoveServiceHandler::initialize()
{
	try
	{
		shutdown(); // Clean up first
		startup(); // Try start up

		initialized = PSM_GetIsConnected();
		Flags_SettingsSupported = initialized && m_deviceUsable;
		m_needsRefresh = true; // Refresh joints

		LOG(INFO) << (initialized ? "PSMoveService init OK." : "PSMoveService is not running.");
	}
	catch (std::exception& e)
	{
		LOG(ERROR) << e.what();
	}
}

std::string PSMResultToString(PSMResult result)
{
	switch (result)
	{
	case PSMResult_Error:
		return "General Error Result";
	case PSMResult_Success:
		return "General Success Result";
	case PSMResult_Timeout:
		return "Requested Timed Out";
	case PSMResult_RequestSent:
		return "Request Successfully Sent";
	case PSMResult_Canceled:
		return "Request Canceled ";
	case PSMResult_NoData:
		return " Request Returned No Data";
	default: return "PSMRESULTTOSTRING INVALID!!!!";
	}
}

void PSMoveServiceHandler::shutdown()
{
	try
	{
		if (controllerList.count > 0)
		{
			for (int i = 0; i < controllerList.count; ++i)
			{
				PSM_StopControllerDataStream(controllerList.controller_id[i], PSM_DEFAULT_TIMEOUT);
				PSM_FreeControllerListener(controllerList.controller_id[i]);
			}
		}
		// No tracker data streams started
		// No HMD data streams started

		initialized = false;
		Flags_SettingsSupported = initialized && m_deviceUsable;

		LOG(INFO) << "PSMoveService attempted shutdown with PSMResult: " <<
			PSMResultToString(PSM_Shutdown());
	}
	catch (std::exception& e)
	{
	}
}

void PSMoveServiceHandler::update()
{
	if (isInitialized())
	{
		rebuildPSMoveLists();

		// Get the controller data for each controller
		if (m_keepRunning)
			processKeyInputs();
	}
}

std::string batteryValueString(PSMBatteryState battery)
{
	switch (battery)
	{
	case PSMBattery_0:
		return "~0%";
	case PSMBattery_20:
		return "~20%";
	case PSMBattery_40:
		return "~40%";
	case PSMBattery_60:
		return "~60%";
	case PSMBattery_80:
		return "~80%";
	case PSMBattery_100:
		return "~100%";
	case PSMBattery_Charging:
		return "Charging...";
	case PSMBattery_Charged:
		return "Charged!";
	default:
		LOG(ERROR) << "INVALID BATTERY VALUE!!!";
		return "INVALID";
	}
}

void PSMoveServiceHandler::signalJoint(uint32_t at)
{
	try
	{
		if (v_controllers.size() > at)
			v_controllers.at(at).flashNow = true;
	}
	catch (const std::exception& e)
	{
	}
}

bool PSMoveServiceHandler::startup()
{
	bool success = true;

	if (PSM_Initialize(PSMOVESERVICE_DEFAULT_ADDRESS, PSMOVESERVICE_DEFAULT_PORT, PSM_DEFAULT_TIMEOUT) ==
		PSMResult_Success)
	{
		LOG(INFO) << "PSMoveConsoleClient::startup() - Initialized client version - " <<
			PSM_GetClientVersionString();
	}
	else
	{
		LOG(INFO) << "PSMoveConsoleClient::startup() - Failed to initialize the client network manager";
		success = false;
	}

	if (success)
	{
		rebuildControllerList();
		LOG(INFO) << "Controller List has been rebuilt.";

		// Register as listener and start stream for each controller
		constexpr unsigned int data_stream_flags =
			PSMStreamFlags_includePositionData |
			PSMStreamFlags_includePhysicsData |
			PSMStreamFlags_includeCalibratedSensorData |
			PSMStreamFlags_includeRawTrackerData;

		if (controllerList.count > 0 || trackerList.count > 0)
		{
			for (int i = 0; i < controllerList.count; ++i)
			{
				LOG(INFO) << "Allocating controller with ID " << i << " into its listener...";

				// In order for the controllers to report more than their IMU stuff, and turn on the light, they all have to go through this
				if (PSM_AllocateControllerListener(controllerList.controller_id[i]) != PSMResult_Success)
					success = false;
				if (PSM_StartControllerDataStream(controllerList.controller_id[i], data_stream_flags,
				                                  PSM_DEFAULT_TIMEOUT) != PSMResult_Success)
					success = false;

				if (success)
					LOG(INFO) << "Allocated controller with ID " << i;
			}

			v_controllers.clear(); // All old controllers must be gone
			for (int i = 0; i < controllerList.count; ++i)
			{
				const auto controller = PSM_GetController(controllerList.controller_id[i]);
				// Check that it's actually a PSMove/Virtual, as there could be dualshock's connected

				if (controller->ControllerType == PSMController_Move ||
					controller->ControllerType == PSMController_Virtual)
				{
					MoveWrapper_PSM wrapper;
					wrapper.controller = controller;
					v_controllers.push_back(wrapper);

					LOG(INFO) << "Pushed controller's ID: " << controller->ControllerID <<
						" wrapper to the controllers' vector.";
				}

				if (controller->ControllerType == PSMController_Move)
					LOG(INFO) << "Controller " << i << " has battery level: " <<
						batteryValueString(v_controllers[i].controller->ControllerState.PSMoveState.BatteryValue);
			}
		}
		else
		{
			LOG(INFO) << "PSMoveConsoleClient::startup() - No controllers found.";
			success = false;
		}
	}

	if (success)
	{
		last_report_fps_timestamp =
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch());
	}

	return success;
}

void PSMoveServiceHandler::rebuildPSMoveLists()
{
	// Unneccessary holdover from inspiration for implementation
	//std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	//std::chrono::milliseconds diff = now - last_report_fps_timestamp;

	// Polls events and updates controller state
	if (PSM_Update() == PSMResult_Success)
	{
		m_keepRunning = true;
	}
	else
	{
		m_keepRunning = false;
	}

	// See if we need to rebuild the controller list
	if (m_keepRunning && (PSM_HasControllerListChanged() || m_needsRefresh))
	{
		m_needsRefresh = false; // Uncheck

		constexpr unsigned int data_stream_flags =
			PSMStreamFlags_includePositionData |
			PSMStreamFlags_includePhysicsData |
			PSMStreamFlags_includeCalibratedSensorData |
			PSMStreamFlags_includeRawTrackerData;

		// Stop all controller streams
		for (int i = 0; i < controllerList.count; ++i)
		{
			PSM_StopControllerDataStream(controllerList.controller_id[i], PSM_DEFAULT_TIMEOUT);
		}

		// Get the current controller list
		rebuildControllerList();

		// Restart the controller streams
		if (controllerList.count > 0)
		{
			for (int i = 0; i < controllerList.count; ++i)
			{
				if (PSM_StartControllerDataStream(controllerList.controller_id[i], data_stream_flags,
				                                  PSM_DEFAULT_TIMEOUT) != PSMResult_Success)
				{
					m_keepRunning = false;
					LOG(ERROR) << "Controller stream " << i << " failed to start!";
				}
			}
			// Rebuild KTVR Controller List for Trackers
			// Here, because of timing issue, where controllers will report as 'None' occasionally when uninitialized properly
		}
		else
		{
			LOG(INFO) << "PSMoveConsoleClient::startup() - No controllers found.";
			m_keepRunning = false;
		}
	}
}

void PSMoveServiceHandler::rebuildControllerList()
{
	memset(&controllerList, 0, sizeof(PSMControllerList)); // Erase internal list
	int timeout = 2000;
	auto result = PSM_GetControllerList(&controllerList, timeout);
	LOG(INFO) << "PSM_GetControllerList returned " << PSMResultToString(result);

	LOG(INFO) << "Found " << controllerList.count << " controllers.";

	// Rebuild the K2App's joint vector too
	trackedJoints.clear();

	for (int cntlr_ix = 0; cntlr_ix < controllerList.count; ++cntlr_ix)
	{
		auto controller_type = L"NONE";

		switch (controllerList.controller_type[cntlr_ix])
		{
		case PSMController_Move:
			controller_type = L"PSMove";
			break;
		case PSMController_Navi:
			controller_type = L"PSNavi";
			break;
		case PSMController_DualShock4:
			controller_type = L"DualShock4";
			break;
		case PSMController_Virtual:
			controller_type = L"Virtual";
			break;
		}

		LOG(INFO) << "Controller ID : " << controllerList.controller_id[cntlr_ix] <<
			" is a " << WStringToString(controller_type);

		using namespace std::literals;
		trackedJoints.push_back(ktvr::K2TrackedJoint( // Add the controller to K2App's vector
			controller_type + L" "s + std::to_wstring(controllerList.controller_id[cntlr_ix])));
	}

	// If there are no real joints, push PSMS_MAX_JOINTS placeholders
	// AND mark the device as unusable (so users won't see the placeholders)
	// This should spare us at least one crash condition （￣︶￣）↗　
	if (trackedJoints.empty())
	{
		m_deviceUsable = false;
		for (size_t i = 0; i < PSMOVESERVICE_MAX_CONTROLLER_COUNT; i++)
			trackedJoints.push_back(ktvr::K2TrackedJoint(L"INVALID " + std::to_wstring(i)));
	}
	else m_deviceUsable = true;
}

void PSMoveServiceHandler::processKeyInputs()
{
	if (controllerList.count < 1 || v_controllers.empty()) { return; }

	for (MoveWrapper_PSM& wrapper : v_controllers)
	{
		if (wrapper.controller->ControllerType != PSMController_Move &&
			wrapper.controller->ControllerType != PSMController_Virtual)
			continue; // Skip other types cuz no input otr whatever

		const auto& controller = wrapper.controller->ControllerState.PSMoveState;
		const bool bStartRealignHMDTriggered = (controller.StartButton == PSMButtonState_PRESSED
			|| controller.StartButton == PSMButtonState_DOWN) && (controller.SelectButton == PSMButtonState_PRESSED
			|| controller.SelectButton == PSMButtonState_DOWN);
		
		// Recenter (capture)
		if (controller.SelectButton == PSMButtonState_PRESSED)
			wrapper.orientationOffset = PSMSToEigen(controller.Pose.Orientation);

		// Update joint's position
		trackedJoints.at(wrapper.controller->ControllerID).
		              update(
			              // Pose : [pos, ori]
			              PSMSToEigen(controller.Pose.Position),
			              wrapper.orientationOffset.inverse() * PSMSToEigen(controller.Pose.Orientation),

			              // Physics : [v, a, ang_v, ang_a] : [cm]->[m] (1/100)
			              PSMSToEigen(controller.PhysicsData.LinearVelocityCmPerSec) * 0.01,
			              PSMSToEigen(controller.PhysicsData.LinearAccelerationCmPerSecSqr) * 0.01,
			              PSMSToEigen(controller.PhysicsData.AngularVelocityRadPerSec) * 0.01,
			              PSMSToEigen(controller.PhysicsData.AngularAccelerationRadPerSecSqr) * 0.01,

			              // Recenter (offset)
			              ktvr::State_Tracked);

		// Optionally signal the joint
		if (wrapper.flashNow)
		{
			// Vibrate the controller to signal selection
			std::thread([&, this](const uint32_t id)
			            {
				            PSM_SetControllerRumble(id, PSMControllerRumbleChannel_All, 0.7);
				            std::this_thread::sleep_for(std::chrono::milliseconds(100));
				            PSM_SetControllerRumble(id, PSMControllerRumbleChannel_All, 0.0);
			            },
			            wrapper.controller->ControllerID).detach();
			wrapper.flashNow = false; // Reset
		}

		if (bStartRealignHMDTriggered)
		{
			PSMVector3f controllerBallPointedUpEuler =
				{static_cast<float>(M_PI_2), 0.0f, 0.0f};
			PSMQuatf controllerBallPointedUpQuat =
				PSM_QuatfCreateFromAngles(&controllerBallPointedUpEuler);
			PSM_ResetControllerOrientationAsync(
				wrapper.controller->ControllerID, &controllerBallPointedUpQuat, nullptr);
		}
	}
}
