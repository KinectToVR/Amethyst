#pragma once
#include "pch.h"
#include "K2Shared.h"

inline std::array<std::string, 3> split_status(std::string const& s)
{
	// If there are 3 strings separated by \n
	return std::array<std::string, 3>{
		s.substr(0, s.find("\n")),
		s.substr(s.find("\n") + 1, s.rfind("\n") - (s.find("\n") + 1)),
		s.substr(s.rfind("\n") + 1)
	};
}

namespace TrackingDevices
{
	// Vector of currently available tracking devices
	// std::variant cause there are 3 possible device types
	inline std::vector<
		std::variant<
			ktvr::K2TrackingDeviceBase_KinectBasis*,
			ktvr::K2TrackingDeviceBase_JointsBasis*>>
	TrackingDevicesVector;

	// Pointer to the device's constructing function
	typedef void* (*TrackingDeviceBaseFactory)(const char* pVersionName, int* pReturnCode);

	// Extract the current device (variant of it)
	inline auto getCurrentDevice()
	{
		// trackingDeviceID is always >= 0 anyway
		return TrackingDevicesVector.at(k2app::K2Settings.trackingDeviceID);
	}

	// Extract the current device (variant of it)
	inline auto getCurrentOverrideDevice()
	{
		// trackingDeviceID is always >= 0 anyway
		return TrackingDevicesVector.at(k2app::K2Settings.overrideDeviceID);
	}
	
	// Select proper tracking device in the UI
	inline void updateTrackingDeviceUI(uint32_t const& index)
	{
		if (TrackingDevicesVector.size() < 1) return; // Just give up

		// Get the current tracking device
		auto& trackingDevice = TrackingDevicesVector.at(index);

		std::string deviceName = "[UNKNOWN]"; // Dummy name
		std::string device_status = "E_UKNOWN\nWhat's happened here?"; // Dummy status

		if (trackingDevice.index() == 0)
		{
			// Kinect Basis
			const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

			device_status = device->statusResultString(device->getStatusResult());
			deviceName = device->getDeviceName();
		}
		else if (trackingDevice.index() == 1)
		{
			// Joints Basis
			const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

			device_status = device->statusResultString(device->getStatusResult());
			deviceName = device->getDeviceName();
		}

		/* Update the device in general tab */

		// Update the status here
		const bool status_ok = device_status.find("S_OK") != std::string::npos;
		using namespace winrt::Microsoft::UI::Xaml;

		// Check with this one, should be the same for all anyway
		if (::k2app::shared::general::errorWhatText.get() != nullptr) {

			::k2app::shared::general::errorWhatText.get()->Visibility(
				status_ok ? Visibility::Collapsed : Visibility::Visible);
			::k2app::shared::general::errorWhatGrid.get()->Visibility(
				status_ok ? Visibility::Collapsed : Visibility::Visible);
			::k2app::shared::general::errorButtonsGrid.get()->Visibility(
				status_ok ? Visibility::Collapsed : Visibility::Visible);
			::k2app::shared::general::trackingDeviceErrorLabel.get()->Visibility(
				status_ok ? Visibility::Collapsed : Visibility::Visible);

			// Split status and message by \n
			::k2app::shared::general::deviceNameLabel.get()->Text(wstring_cast(deviceName));
			::k2app::shared::general::deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
			::k2app::shared::general::trackingDeviceErrorLabel.get()->Text(
				wstring_cast(split_status(device_status)[1]));
			::k2app::shared::general::errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));
		}

		// Update the devices tab
		k2app::shared::devices::smphSignalCurrentUpdate.release();
	}
}
