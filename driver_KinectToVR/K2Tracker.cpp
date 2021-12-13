#include "K2Tracker.h"
#include <openvr_driver.h>
#include <string>
#include <thread>
#include <KinectToVR_API.h>

K2Tracker::K2Tracker(ktvr::K2TrackerBase const& tracker_base)
{
	_serial = tracker_base.data.serial;
	_role = tracker_base.data.role;
	_active = tracker_base.data.isActive;

	_pose = { 0 };
	_pose.poseIsValid = true; // Otherwise tracker may disappear
	_pose.result = vr::TrackingResult_Running_OK;
	_pose.deviceIsConnected = tracker_base.data.isActive;

	_pose.qWorldFromDriverRotation.w = 1;
	_pose.qWorldFromDriverRotation.x = 0;
	_pose.qWorldFromDriverRotation.y = 0;
	_pose.qWorldFromDriverRotation.z = 0;
	_pose.qDriverFromHeadRotation.w = 1;
	_pose.qDriverFromHeadRotation.x = 0;
	_pose.qDriverFromHeadRotation.y = 0;
	_pose.qDriverFromHeadRotation.z = 0;

	_pose.vecPosition[0] = 0;
	_pose.vecPosition[1] = 0;
	_pose.vecPosition[2] = 0;
}

std::string K2Tracker::get_serial() const
{
	return _serial;
}

void K2Tracker::update()
{
	if (_index != vr::k_unTrackedDeviceIndexInvalid && _activated) {
		// If _active is false, then disconnect the tracker
		_pose.poseIsValid = _active;
		_pose.deviceIsConnected = _active;
		vr::VRServerDriverHost()->TrackedDevicePoseUpdated(_index, _pose, sizeof _pose);
	}
}

void K2Tracker::set_pose(ktvr::K2PosePacket const& pose)
{
	try {
		// For handling PosePacket's time offset
		std::thread([&, pose]()
			{
				// Wait the specified time
				if (pose.millisFromNow > 0)
					std::this_thread::sleep_for(std::chrono::milliseconds(
						static_cast<int>(pose.millisFromNow)));

				// Just copy the values
				_pose.vecPosition[0] = pose.position.x();
				_pose.vecPosition[1] = pose.position.y();
				_pose.vecPosition[2] = pose.position.z();

				_pose.qRotation.w = pose.orientation.w();
				_pose.qRotation.x = pose.orientation.x();
				_pose.qRotation.y = pose.orientation.y();
				_pose.qRotation.z = pose.orientation.z();

				// Automatically update the tracker when finished
				update(); // called from this
			}).detach();
	}
	catch (...) {
		LOG(ERROR) << "Couldn't set tracker pose. An exception occurred.";
	}
}

void K2Tracker::set_data(ktvr::K2DataPacket const& data)
{
	try {
		// For handling PosePacket's time offset
		std::thread([&, data]()
			{
				// Wait the specified time
				if (data.millisFromNow > 0)
					std::this_thread::sleep_for(std::chrono::milliseconds(
						static_cast<int>(data.millisFromNow)));

				// Not spawning, just pose validity
				_active = data.isActive;

				// Data may only change if the tracker isn't spawned
				if (!_added && !_activated)
				{
					LOG(INFO) << "Updating tracker's role: " << 
						ktvr::ITrackerType_Role_String.at(static_cast<ktvr::ITrackerType>(_role)) << 
						" to: " << 
						ktvr::ITrackerType_Role_String.at(static_cast<ktvr::ITrackerType>(data.role));
					
					_role = data.role;
					if(!data.serial.empty())
					{
						LOG(INFO) << "Updating tracker's serial: " << _serial << " to: " << data.serial;
						_serial = data.serial;
					}
					else LOG(ERROR) << "Couldn't set tracker data. The provided serial was empty.";

					// Spawn automatically
					if (_active)
					{
						// Spawn and set the state
						spawn();
						LOG(INFO) << "Tracker autospawned! Serial: " + get_serial() +
							" Role: " + ktvr::ITrackerType_Role_String.at(static_cast<ktvr::ITrackerType>(_role));
					}
				}
				else LOG(ERROR) << "Couldn't set tracker data. It has been already added.";
			}).detach();
	}
	catch (...) {
		LOG(ERROR) << "Couldn't set tracker data. An exception occurred.";
	}
}

void K2Tracker::set_state(bool state)
{
	_active = state;
}

bool K2Tracker::spawn()
{
	try {
		if (!_added && !_serial.empty())
		{
			// Add device to OpenVR devices list
			vr::VRServerDriverHost()->TrackedDeviceAdded(_serial.c_str(), vr::TrackedDeviceClass_GenericTracker, this);
			_added = true;
			return true;
		}
	}
	catch (...) {}
	return false;
}

vr::TrackedDeviceIndex_t K2Tracker::get_index() const
{
	return _index;
}

void K2Tracker::process_event(const vr::VREvent_t& event)
{
}

vr::EVRInitError K2Tracker::Activate(vr::TrackedDeviceIndex_t index)
{
	// Save the device index
	_index = index;

	// Get the properties handle for our controller
	_props = vr::VRProperties()->TrackedDeviceToPropertyContainer(_index);

	/*
	 * Of course thanks to @SDraw
	 */

	// Set our universe ID
	vr::VRProperties()->SetUint64Property(_props, vr::Prop_CurrentUniverseId_Uint64, 2);

	// Create components
	vr::VRDriverInput()->CreateBooleanComponent(_props, "/input/system/click", &_components._system_click);
	vr::VRDriverInput()->CreateHapticComponent(_props, "/output/haptic", &_components._haptic);

	// Register all properties, dumped by sdraw originally
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_TrackingSystemName_String, "lighthouse");
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_ModelNumber_String, "Vive Tracker Pro MV");
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_SerialNumber_String, _serial.c_str());
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_RenderModelName_String, "{htc}vr_tracker_vive_1_0");

	vr::VRProperties()->SetBoolProperty(_props, vr::Prop_WillDriftInYaw_Bool, false);
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_ManufacturerName_String, "HTC");
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_TrackingFirmwareVersion_String,
		"1541800000 RUNNER-WATCHMAN$runner-watchman@runner-watchman 2018-01-01 FPGA 512(2.56/0/0) BL 0 VRC 1541800000 Radio 1518800000");
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_HardwareRevision_String,
		"product 128 rev 2.5.6 lot 2000/0/0 0");

	vr::VRProperties()->SetStringProperty(_props, vr::Prop_ConnectedWirelessDongle_String, "D0000BE000");
	vr::VRProperties()->SetBoolProperty(_props, vr::Prop_DeviceIsWireless_Bool, true);
	vr::VRProperties()->SetBoolProperty(_props, vr::Prop_DeviceIsCharging_Bool, false);
	vr::VRProperties()->SetFloatProperty(_props, vr::Prop_DeviceBatteryPercentage_Float, 1.f);

	vr::HmdMatrix34_t l_transform = { -1.f, 0.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f };
	vr::VRProperties()->SetProperty(_props, vr::Prop_StatusDisplayTransform_Matrix34, &l_transform,
		sizeof(vr::HmdMatrix34_t),
		vr::k_unHmdMatrix34PropertyTag);

	vr::VRProperties()->SetBoolProperty(_props, vr::Prop_Firmware_UpdateAvailable_Bool, false);
	vr::VRProperties()->SetBoolProperty(_props, vr::Prop_Firmware_ManualUpdate_Bool, false);
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_Firmware_ManualUpdateURL_String,
		"https://developer.valvesoftware.com/wiki/SteamVR/HowTo_Update_Firmware");
	vr::VRProperties()->SetUint64Property(_props, vr::Prop_HardwareRevision_Uint64, 2214720000);
	vr::VRProperties()->SetUint64Property(_props, vr::Prop_FirmwareVersion_Uint64, 1541800000);
	vr::VRProperties()->SetUint64Property(_props, vr::Prop_FPGAVersion_Uint64, 512);
	vr::VRProperties()->SetUint64Property(_props, vr::Prop_VRCVersion_Uint64, 1514800000);
	vr::VRProperties()->SetUint64Property(_props, vr::Prop_RadioVersion_Uint64, 1518800000);
	vr::VRProperties()->SetUint64Property(_props, vr::Prop_DongleVersion_Uint64, 8933539758);

	vr::VRProperties()->SetBoolProperty(_props, vr::Prop_DeviceProvidesBatteryStatus_Bool, true);
	vr::VRProperties()->SetBoolProperty(_props, vr::Prop_DeviceCanPowerOff_Bool, true);
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_Firmware_ProgrammingTarget_String, _serial.c_str());
	vr::VRProperties()->SetInt32Property(_props, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_GenericTracker);
	vr::VRProperties()->SetBoolProperty(_props, vr::Prop_Firmware_ForceUpdateRequired_Bool, false);
	//vr::VRProperties()->SetUint64Property(_props, vr::Prop_ParentDriver_Uint64, 8589934597);
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_ResourceRoot_String, "htc");

	std::string l_registeredType("htc/vive_tracker");
	l_registeredType.append(_serial);
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_RegisteredDeviceType_String, l_registeredType.c_str());
	
	vr::VRProperties()->SetBoolProperty(_props, vr::Prop_Identifiable_Bool, false);
	vr::VRProperties()->SetBoolProperty(_props, vr::Prop_Firmware_RemindUpdate_Bool, false);
	vr::VRProperties()->SetInt32Property(_props, vr::Prop_ControllerRoleHint_Int32, vr::TrackedControllerRole_Invalid);
	vr::VRProperties()->SetInt32Property(_props, vr::Prop_ControllerHandSelectionPriority_Int32, -1);

	vr::VRProperties()->SetStringProperty(_props, vr::Prop_NamedIconPathDeviceOff_String,
		"{htc}/icons/tracker_status_off.png");
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_NamedIconPathDeviceSearching_String,
		"{htc}/icons/tracker_status_searching.gif");
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_NamedIconPathDeviceSearchingAlert_String,
		"{htc}/icons/tracker_status_searching_alert.gif");
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_NamedIconPathDeviceReady_String,
		"{htc}/icons/tracker_status_ready.png");
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_NamedIconPathDeviceReadyAlert_String,
		"{htc}/icons/tracker_status_ready_alert.png");
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_NamedIconPathDeviceNotReady_String,
		"{htc}/icons/tracker_status_error.png");
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_NamedIconPathDeviceStandby_String,
		"{htc}/icons/tracker_status_standby.png");
	vr::VRProperties()->SetStringProperty(_props, vr::Prop_NamedIconPathDeviceAlertLow_String,
		"{htc}/icons/tracker_status_ready_low.png");

	vr::VRProperties()->SetBoolProperty(_props, vr::Prop_HasDisplayComponent_Bool, false);
	vr::VRProperties()->SetBoolProperty(_props, vr::Prop_HasCameraComponent_Bool, false);
	vr::VRProperties()->SetBoolProperty(_props, vr::Prop_HasDriverDirectModeComponent_Bool, false);
	vr::VRProperties()->SetBoolProperty(_props, vr::Prop_HasVirtualDisplayComponent_Bool, false);

	/*Get tracker role*/
	const std::string role_enum_string = ktvr::ITrackerType_String.at(static_cast<ktvr::ITrackerType>(_role));

	/*Update controller type and input path*/
	const std::string input_path =
		"{htc}/input/tracker/" + role_enum_string + "_profile.json";

	vr::VRProperties()->SetStringProperty(_props,
		vr::Prop_InputProfilePath_String, input_path.c_str());
	vr::VRProperties()->SetStringProperty(_props,
		vr::Prop_ControllerType_String, role_enum_string.c_str());

	/*Update tracker's role in menu*/
	std::string l_registeredDevice("/devices/htc/vive_tracker");
	l_registeredDevice.append(_serial);
	
	vr::VRSettings()->SetString(vr::k_pch_Trackers_Section, l_registeredDevice.c_str(),
		ktvr::ITrackerType_Role_String.at(static_cast<ktvr::ITrackerType>(_role)));

	/*Mark tracker as activated*/
	_activated = true;
	
	return vr::VRInitError_None;
}

vr::DriverPose_t K2Tracker::GetPose()
{
	return _pose;
}

ktvr::K2TrackerBase K2Tracker::getTrackerBase() {
	// Copy the data
	_trackerBase.data.serial = _serial;
	_trackerBase.data.role = _role;
	_trackerBase.data.isActive = _active;

	// Copy the position
	_trackerBase.pose.position.x() = _pose.vecPosition[0];
	_trackerBase.pose.position.y() = _pose.vecPosition[1];
	_trackerBase.pose.position.z() = _pose.vecPosition[2];

	// Copy the orientation
	_trackerBase.pose.orientation.w() = _pose.qRotation.w;
	_trackerBase.pose.orientation.x() = _pose.qRotation.x;
	_trackerBase.pose.orientation.y() = _pose.qRotation.y;
	_trackerBase.pose.orientation.z() = _pose.qRotation.z;

	// Return the base object
	return _trackerBase;
}
