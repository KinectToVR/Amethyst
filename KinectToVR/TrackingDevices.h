#pragma once
#include "pch.h"

#include "K2Interfacing.h"
#include "K2Shared.h"

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
	inline auto getCurrentDevice(uint32_t const& id)
	{
		// trackingDeviceID is always >= 0 anyway
		return TrackingDevicesVector.at(id);
	}

	// Extract the current device (variant of it)
	inline auto getCurrentOverrideDevice()
	{
		// trackingDeviceID is always >= 0 anyway
		return TrackingDevicesVector.at(k2app::K2Settings.overrideDeviceID);
	}

	// Extract the current device (variant of it)
	inline std::pair<
		bool, std::variant<
			ktvr::K2TrackingDeviceBase_KinectBasis*,
			ktvr::K2TrackingDeviceBase_JointsBasis*>> getCurrentOverrideDevice_Safe()
	{
		bool _exists = k2app::K2Settings.overrideDeviceID >= 0 &&
			TrackingDevicesVector.size() > k2app::K2Settings.overrideDeviceID;

		// Assuming that the caller will test in pair.first is true,
		// we can push the id0 device here as well if pair.first is gonna be false
		uint32_t _deviceID = _exists ? k2app::K2Settings.overrideDeviceID : 0;

		// trackingDeviceID is always >= 0 anyway
		return std::make_pair(_exists,
		                      TrackingDevicesVector.at(_deviceID));
	}

	// Extract the current device (variant of it)
	inline std::pair<
		bool, std::variant<
			ktvr::K2TrackingDeviceBase_KinectBasis*,
			ktvr::K2TrackingDeviceBase_JointsBasis*>> getCurrentOverrideDevice_Safe(uint32_t const& id)
	{
		bool _exists = TrackingDevicesVector.size() > id;

		// Assuming that the caller will test in pair.first is true,
		// we can push the id0 device here as well if pair.first is gonna be false
		uint32_t _deviceID = _exists ? id : 0;

		// trackingDeviceID is always >= 0 anyway
		return std::make_pair(_exists,
		                      TrackingDevicesVector.at(_deviceID));
	}

	// autoCheck->true will force the function to cehck and false will assume unsupported
	inline void settings_set_external_flip_is_enabled(bool autoCheck = true)
	{
		if (k2app::shared::settings::externalFlipCheckBox.get() == nullptr)return;

		if (autoCheck)
		{
			bool isFlipSupported = false;

			auto const& trackingDevice =
				TrackingDevices::getCurrentDevice();

			if (trackingDevice.index() == 0)
				isFlipSupported = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(
					trackingDevice)->isFlipSupported();

			bool isExternalFlipSupported = false;

			auto const& overrideDevice =
				TrackingDevices::getCurrentOverrideDevice_Safe();

			if (overrideDevice.first)
			{
				if (overrideDevice.second.index() == 0)
					isExternalFlipSupported = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(
						overrideDevice.second)->isFlipSupported();

				else if (overrideDevice.second.index() == 1)
					isExternalFlipSupported = true;
			}

			k2app::shared::settings::externalFlipCheckBox.get()->IsEnabled(
				isFlipSupported && isExternalFlipSupported &&
				k2app::K2Settings.isFlipEnabled);
		}
		else
			k2app::shared::settings::externalFlipCheckBox.get()->IsEnabled(false);

		k2app::shared::settings::externalFlipCheckBoxLabel.get()->Opacity(
			k2app::shared::settings::externalFlipCheckBox.get()->IsEnabled() ? 1 : 0.5);

		if (!k2app::shared::settings::externalFlipCheckBox.get()->IsEnabled())
		{
			k2app::shared::settings::externalFlipCheckBox.get()->IsChecked(false);
			k2app::K2Settings.isExternalFlipEnabled = false;
			k2app::K2Settings.saveSettings();
		}
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
		bool status_ok = device_status.find("S_OK") != std::string::npos;
		using namespace winrt::Microsoft::UI::Xaml;

		// Check with this one, should be the same for all anyway
		if (::k2app::shared::general::errorWhatText.get() != nullptr)
		{
			// Don't show device errors if we've got a server error
			if (k2app::interfacing::serverDriverFailure)status_ok = true;

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

		/* Update the device in devices tab */

		::k2app::shared::devices::smphSignalCurrentUpdate.release();

		/* Update the device in settings tab */

		if (::k2app::shared::settings::softwareRotationItem.get() != nullptr)
		{
			if (trackingDevice.index() == 0)
			{
				// Kinect Basis
				::k2app::shared::settings::flipCheckBox.get()->IsChecked(k2app::K2Settings.isFlipEnabled);
				::k2app::shared::settings::softwareRotationItem.get()->IsEnabled(
					std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isAppOrientationSupported());
				::k2app::shared::settings::flipCheckBox.get()->IsEnabled(
					std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported());
				::k2app::shared::settings::flipCheckBoxLabel.get()->Opacity(
					::k2app::shared::settings::flipCheckBox.get()->IsEnabled() ? 1 : 0.5);

				settings_set_external_flip_is_enabled();

				if (!std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isAppOrientationSupported() &&
					(k2app::K2Settings.jointRotationTrackingOption[1] == k2app::k2_SoftwareCalculatedRotation ||
						k2app::K2Settings.jointRotationTrackingOption[2] == k2app::k2_SoftwareCalculatedRotation))
				{
					k2app::K2Settings.jointRotationTrackingOption[1] = k2app::k2_DeviceInferredRotation;
					k2app::K2Settings.jointRotationTrackingOption[2] = k2app::k2_DeviceInferredRotation;

					k2app::shared::settings::feetRotationOptionBox.get()->SelectedIndex(
						k2app::K2Settings.jointRotationTrackingOption[1]); // Feet
				}
			}
			else if (trackingDevice.index() == 1)
			{
				// Joints Basis
				k2app::K2Settings.isFlipEnabled = false;
				::k2app::shared::settings::softwareRotationItem.get()->IsEnabled(false);
				::k2app::shared::settings::flipCheckBox.get()->IsEnabled(false);
				::k2app::shared::settings::flipCheckBoxLabel.get()->Opacity(0.5);

				settings_set_external_flip_is_enabled(false);

				if (k2app::K2Settings.jointRotationTrackingOption[1] == k2app::k2_SoftwareCalculatedRotation ||
					k2app::K2Settings.jointRotationTrackingOption[2] == k2app::k2_SoftwareCalculatedRotation)
				{
					k2app::K2Settings.jointRotationTrackingOption[1] = k2app::k2_DeviceInferredRotation;
					k2app::K2Settings.jointRotationTrackingOption[2] = k2app::k2_DeviceInferredRotation;

					k2app::shared::settings::feetRotationOptionBox.get()->SelectedIndex(
						k2app::K2Settings.jointRotationTrackingOption[1]); // Feet
				}
			}
		}

		// Save settings
		k2app::K2Settings.saveSettings();
	}

	// Select proper tracking device in the UI
	inline void updateOverrideDeviceUI(uint32_t const& index)
	{
		if (TrackingDevicesVector.size() < 1) return; // Just give up

		// Don't show ANYTHING if we 'ven 't selected an override device
		const bool _show = (k2app::K2Settings.overrideDeviceID >= 0) && (TrackingDevicesVector.size() >= 2);
		bool status_ok = false; // Assume failure :/

		std::string deviceName = "[UNKNOWN]"; // Dummy name
		std::string device_status = "E_UKNOWN\nWhat's happened here?"; // Dummy status

		if (_show)
		{
			// Get the current tracking device
			auto& overrideDevice = TrackingDevicesVector.at(index);

			if (overrideDevice.index() == 0)
			{
				// Kinect Basis
				const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(overrideDevice);

				device_status = device->statusResultString(device->getStatusResult());
				deviceName = device->getDeviceName();
			}
			else if (overrideDevice.index() == 1)
			{
				// Joints Basis
				const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(overrideDevice);

				device_status = device->statusResultString(device->getStatusResult());
				deviceName = device->getDeviceName();
			}

			status_ok = device_status.find("S_OK") != std::string::npos;
		}

		/* Update the device in settings tab */

		settings_set_external_flip_is_enabled();

		/* Update the device in general tab */

		// Check with this one, should be the same for all anyway
		if (::k2app::shared::general::overrideErrorWhatText.get() != nullptr)
		{
			// Update the status here
			const bool base_status_ok = ::k2app::shared::general::errorWhatGrid.get()->Visibility() ==
				winrt::Microsoft::UI::Xaml::Visibility::Collapsed;
			using namespace winrt::Microsoft::UI::Xaml;

			// Don't show device errors if we've got a server error or base error OR no o_device
			if (k2app::interfacing::serverDriverFailure || !base_status_ok || !_show)status_ok = true;

			// Don't show ANYTHING if we 'ven 't selected an override device
			::k2app::shared::general::overrideDeviceNameLabel.get()->Visibility(
				_show ? Visibility::Visible : Visibility::Collapsed);
			::k2app::shared::general::overrideDeviceStatusLabel.get()->Visibility(
				_show ? Visibility::Visible : Visibility::Collapsed);

			::k2app::shared::general::overrideErrorWhatText.get()->Visibility(
				status_ok ? Visibility::Collapsed : Visibility::Visible);
			::k2app::shared::general::overrideErrorWhatGrid.get()->Visibility(
				status_ok ? Visibility::Collapsed : Visibility::Visible);
			::k2app::shared::general::overrideErrorButtonsGrid.get()->Visibility(
				status_ok ? Visibility::Collapsed : Visibility::Visible);
			::k2app::shared::general::overrideDeviceErrorLabel.get()->Visibility(
				status_ok ? Visibility::Collapsed : Visibility::Visible);

			// Split status and message by \n
			::k2app::shared::general::overrideDeviceNameLabel.get()->Text(wstring_cast(deviceName));
			::k2app::shared::general::overrideDeviceStatusLabel.get()->Text(
				wstring_cast(split_status(device_status)[0]));
			::k2app::shared::general::overrideDeviceErrorLabel.get()->Text(
				wstring_cast(split_status(device_status)[1]));
			::k2app::shared::general::overrideErrorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));
		}
	}
}
