#pragma once
#include "pch.h"

#include "JointExpander.h"
#include "K2Interfacing.h"
#include "K2Shared.h"

namespace TrackingDevices
{
	// Variant of current devices' layout root pointers
	// Note: the size must be the same as TrackingDevicesVector's
	inline std::vector<k2app::interfacing::AppInterface::AppLayoutRoot*>
	TrackingDevicesLayoutRootsVector;

	// Pointer to the device's constructing function
	using TrackingDeviceBaseFactory = void* (*)(const char* pVersionName, int* pReturnCode);

	// Select proper tracking device in the UI
	inline void updateTrackingDeviceUI(const uint32_t& index)
	{
		if (TrackingDevicesVector.size() < 1) return; // Just give up

		// Get the current tracking device
		auto& trackingDevice = TrackingDevicesVector.at(index);

		std::string deviceName = "[UNKNOWN]"; // Dummy name
		std::wstring device_status = L"Something's wrong!\nE_UKNOWN\nWhat's happened here?"; // Dummy status

		if (trackingDevice.index() == 0)
		{
			// Kinect Basis
			const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

			device_status = device->statusResultWString(device->getStatusResult());
			deviceName = device->getDeviceName();
		}
		else if (trackingDevice.index() == 1)
		{
			// Joints Basis
			const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

			device_status = device->statusResultWString(device->getStatusResult());
			deviceName = device->getDeviceName();
		}

		/* Update the device in general tab */

		// Update the status here
		bool status_ok = device_status.find(L"S_OK") != std::wstring::npos;
		using namespace winrt::Microsoft::UI::Xaml;

		// Check with this one, should be the same for all anyway
		if (k2app::shared::general::errorWhatText.get() != nullptr)
		{
			// Don't show device errors if we've got a server error
			if (!k2app::interfacing::isServerDriverPresent)status_ok = true;

			k2app::shared::general::errorWhatText.get()->Visibility(
				status_ok ? Visibility::Collapsed : Visibility::Visible);
			k2app::shared::general::errorWhatGrid.get()->Visibility(
				status_ok ? Visibility::Collapsed : Visibility::Visible);
			k2app::shared::general::errorButtonsGrid.get()->Visibility(
				status_ok ? Visibility::Collapsed : Visibility::Visible);
			k2app::shared::general::trackingDeviceErrorLabel.get()->Visibility(
				status_ok ? Visibility::Collapsed : Visibility::Visible);

			// Split status and message by \n
			k2app::shared::general::deviceNameLabel.get()->Text(StringToWString(deviceName));
			k2app::shared::general::deviceStatusLabel.get()->Text(split_status(device_status)[0]);
			k2app::shared::general::trackingDeviceErrorLabel.get()->Text(split_status(device_status)[1]);
			k2app::shared::general::errorWhatText.get()->Text(split_status(device_status)[2]);
		}

		/* Update the device in devices tab */

		k2app::shared::devices::smphSignalCurrentUpdate.release();

		/* Update the device in settings tab */

		if (k2app::shared::settings::flipDropDown.get() != nullptr)
		{
			if (trackingDevice.index() == 0)
			{
				// Kinect Basis
				k2app::shared::settings::flipToggle.get()->IsOn(k2app::K2Settings.isFlipEnabled);

				const bool _sup = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>
					(trackingDevice)->isAppOrientationSupported();

				for (auto expander : k2app::shared::settings::jointExpanderVector)
					expander->EnableSoftwareOrientation(_sup);

				k2app::shared::settings::flipToggle.get()->IsEnabled(
					std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported());
				k2app::shared::settings::flipDropDown.get()->IsEnabled(
					std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported());
				k2app::shared::settings::flipDropDownGrid.get()->Opacity(
					k2app::shared::settings::flipToggle.get()->IsEnabled() ? 1 : 0.5);

				settings_set_external_flip_is_enabled();
			}
			else if (trackingDevice.index() == 1)
			{
				// Joints Basis
				k2app::K2Settings.isFlipEnabled = false;

				for (auto expander : k2app::shared::settings::jointExpanderVector)
					expander->EnableSoftwareOrientation(false);

				k2app::shared::settings::flipToggle.get()->IsEnabled(false);
				k2app::shared::settings::flipDropDown.get()->IsEnabled(false);
				k2app::shared::settings::flipDropDownGrid.get()->Opacity(0.5);

				settings_set_external_flip_is_enabled(false);
			}
		}

		// Save settings
		k2app::K2Settings.saveSettings();
	}

	// Select proper tracking device in the UI
	inline void updateOverrideDeviceUI(const uint32_t& index)
	{
		if (TrackingDevicesVector.size() < 1) return; // Just give up

		// Don't show ANYTHING if we 'ven 't selected an override device
		const bool _show = (k2app::K2Settings.overrideDeviceID >= 0) && (TrackingDevicesVector.size() >= 2);
		bool status_ok = false; // Assume failure :/

		std::string deviceName = "[UNKNOWN]"; // Dummy name
		std::wstring device_status = L"E_UKNOWN\nWhat's happened here?"; // Dummy status

		if (_show)
		{
			// Get the current tracking device
			auto& overrideDevice = TrackingDevicesVector.at(index);

			if (overrideDevice.index() == 0)
			{
				// Kinect Basis
				const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(overrideDevice);

				device_status = device->statusResultWString(device->getStatusResult());
				deviceName = device->getDeviceName();
			}
			else if (overrideDevice.index() == 1)
			{
				// Joints Basis
				const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(overrideDevice);

				device_status = device->statusResultWString(device->getStatusResult());
				deviceName = device->getDeviceName();
			}

			status_ok = device_status.find(L"S_OK") != std::wstring::npos;
		}

		/* Update the device in settings tab */

		settings_set_external_flip_is_enabled();

		/* Update the device in general tab */

		// Check with this one, should be the same for all anyway
		if (k2app::shared::general::overrideErrorWhatText.get() != nullptr)
		{
			// Update the status here
			const bool base_status_ok = k2app::shared::general::errorWhatGrid.get()->Visibility() ==
				winrt::Microsoft::UI::Xaml::Visibility::Collapsed;
			using namespace winrt::Microsoft::UI::Xaml;

			// Don't show device errors if we've got a server error or base error OR no o_device
			if (!k2app::interfacing::isServerDriverPresent || !base_status_ok || !_show)status_ok = true;

			// Don't show ANYTHING if we 'ven 't selected an override device
			k2app::shared::general::overrideDeviceNameLabel.get()->Visibility(
				_show ? Visibility::Visible : Visibility::Collapsed);
			k2app::shared::general::overrideDeviceStatusLabel.get()->Visibility(
				_show ? Visibility::Visible : Visibility::Collapsed);

			k2app::shared::general::overrideErrorWhatText.get()->Visibility(
				status_ok ? Visibility::Collapsed : Visibility::Visible);
			k2app::shared::general::overrideErrorWhatGrid.get()->Visibility(
				status_ok ? Visibility::Collapsed : Visibility::Visible);
			k2app::shared::general::overrideErrorButtonsGrid.get()->Visibility(
				status_ok ? Visibility::Collapsed : Visibility::Visible);
			k2app::shared::general::overrideDeviceErrorLabel.get()->Visibility(
				status_ok ? Visibility::Collapsed : Visibility::Visible);

			// Split status and message by \n
			k2app::shared::general::overrideDeviceNameLabel.get()->Text(StringToWString(deviceName));
			k2app::shared::general::overrideDeviceStatusLabel.get()->Text(split_status(device_status)[0]);
			k2app::shared::general::overrideDeviceErrorLabel.get()->Text(split_status(device_status)[1]);
			k2app::shared::general::overrideErrorWhatText.get()->Text(split_status(device_status)[2]);
		}
	}

	inline int32_t devices_override_joint_id(const int32_t& id)
	{
		const auto& _override = getCurrentOverrideDevice_Safe();
		bool _is_kinect = false; // 1: isSet, 2: isKinect

		if (_override.first)
			_is_kinect = _override.second.index() == 0;

		if (
			(id < 0) || // If id is invalid
			(_override.first && !_is_kinect) // If we're using a jointsbasis
		)
			return id;

		if (
			_override.first && _is_kinect // If we're using a kinectbasis
		)
		{
			if (std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(
					_override.second)->getDeviceCharacteristics()
				> ktvr::K2_Character_Basic)
			{
				switch (id)
				{
				case 0:
					return ktvr::Joint_SpineShoulder;
				case 1:
					return ktvr::Joint_ElbowLeft;
				case 2:
					return ktvr::Joint_ElbowRight;
				case 3:
					return ktvr::Joint_SpineWaist;
				case 4:
					return ktvr::Joint_KneeLeft;
				case 5:
					return ktvr::Joint_KneeRight;
				case 6:
					return ktvr::Joint_AnkleLeft;
				case 7:
					return ktvr::Joint_AnkleRight;
				default:
					return -1;
				}
			}
			switch (id)
			{
			case 0:
				return ktvr::Joint_SpineShoulder;
			case 1:
				return ktvr::Joint_SpineWaist;
			case 2:
				return ktvr::Joint_AnkleLeft;
			case 3:
				return ktvr::Joint_AnkleRight;
			default:
				return -1;
			}
		}

		return -1; // Return invalid
	}

	inline int32_t devices_override_joint_id_reverse(const int32_t& id)
	{
		const auto& _override = getCurrentOverrideDevice_Safe();
		bool _is_kinect = false; // 1: isSet, 2: isKinect

		if (_override.first)
			_is_kinect = _override.second.index() == 0;

		if (
			(id < 0) || // If id is invalid
			(_override.first && !_is_kinect) // If we're using a jointsbasis
		)
			return id;

		if (
			_override.first && _is_kinect // If we're using a kinectbasis
		)
		{
			if (std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(
					_override.second)->getDeviceCharacteristics()
				> ktvr::K2_Character_Basic)
			{
				switch (id)
				{
				case ktvr::Joint_SpineShoulder:
					return 0;
				case ktvr::Joint_ElbowLeft:
					return 1;
				case ktvr::Joint_ElbowRight:
					return 2;
				case ktvr::Joint_SpineWaist:
					return 3;
				case ktvr::Joint_KneeLeft:
					return 4;
				case ktvr::Joint_KneeRight:
					return 5;
				case ktvr::Joint_AnkleLeft:
					return 6;
				case ktvr::Joint_AnkleRight:
					return 7;
				default:
					return -1;
				}
			}
			switch (id)
			{
			case ktvr::Joint_SpineShoulder:
				return 0;
			case ktvr::Joint_SpineWaist:
				return 1;
			case ktvr::Joint_AnkleLeft:
				return 2;
			case ktvr::Joint_AnkleRight:
				return 3;
			default:
				return -1;
			}
		}

		return -1; // Return invalid
	}

	inline void devices_check_override_ids(const uint32_t& id)
	{
		// Take down IDs if they're too big
		if (const auto& device_pair = getCurrentOverrideDevice_Safe(id); device_pair.first)
		{
			if (device_pair.second.index() == 1) // If Joints
			{
				// Note: num_joints should never be 0
				const auto num_joints = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>
					(device_pair.second)->getTrackedJoints().size();

				for (auto& tracker : k2app::K2Settings.K2TrackersVector)
					if (tracker.positionOverrideJointID >= num_joints)
						tracker.positionOverrideJointID = 0;

				for (auto& tracker : k2app::K2Settings.K2TrackersVector)
					if (tracker.rotationOverrideJointID >= num_joints)
						tracker.rotationOverrideJointID = 0;
			}
			else if (device_pair.second.index() == 0) // If Kinect
			{
				// Note: switch based on device characteristics
				const auto characteristics = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>
					(device_pair.second)->getDeviceCharacteristics();
				uint32_t num_joints = -1; // To set later

				if (characteristics == ktvr::K2_Character_Full)
					num_joints = 8;
				else if (characteristics == ktvr::K2_Character_Simple)
					num_joints = 8;
				else if (characteristics == ktvr::K2_Character_Basic)
					num_joints = 3;

				for (auto& tracker : k2app::K2Settings.K2TrackersVector)
					if (tracker.positionOverrideJointID >= num_joints)
						tracker.positionOverrideJointID = 0;

				for (auto& tracker : k2app::K2Settings.K2TrackersVector)
					if (tracker.rotationOverrideJointID >= num_joints)
						tracker.rotationOverrideJointID = 0;
			}

			// Save it
			k2app::K2Settings.saveSettings();
		}
	}

	inline void devices_check_base_ids(const uint32_t& id)
	{
		// Take down IDs if they're too big
		if (const auto& device_pair = getCurrentDevice(id);
			device_pair.index() == 1) // If Joints
		{
			// Note: num_joints should never be 0
			const auto num_joints = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>
				(device_pair)->getTrackedJoints().size();

			for (auto& tracker : k2app::K2Settings.K2TrackersVector)
				if (tracker.selectedTrackedJointID >= num_joints)
					tracker.selectedTrackedJointID = 0;

			// Save it
			k2app::K2Settings.saveSettings();
		}
	}

	inline void devices_update_current()
	{
		{
			const auto& trackingDevice = TrackingDevicesVector.at(k2app::K2Settings.trackingDeviceID);

			std::string deviceName = "[UNKNOWN]";

			if (trackingDevice.index() == 0)
			{
				// Kinect Basis
				const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);
				deviceName = device->getDeviceName();
			}
			else if (trackingDevice.index() == 1)
			{
				// Joints Basis
				const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
				deviceName = device->getDeviceName();
			}

			/* Update local statuses */
			k2app::shared::devices::baseDeviceName.get()->Text(StringToWString(deviceName));
			if (k2app::shared::devices::overrideDeviceName.get()->Text() == StringToWString(deviceName))
				k2app::shared::devices::overrideDeviceName.get()->Text(L"No Overrides");
		}
		{
			if (k2app::K2Settings.overrideDeviceID < 0)return;
			const auto& trackingDevice = TrackingDevicesVector.at(k2app::K2Settings.overrideDeviceID);

			std::string deviceName = "[UNKNOWN]";

			if (trackingDevice.index() == 0)
			{
				// Kinect Basis
				const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);
				deviceName = device->getDeviceName();
			}
			else if (trackingDevice.index() == 1)
			{
				// Joints Basis
				const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
				deviceName = device->getDeviceName();
			}

			/* Update local statuses */
			k2app::shared::devices::overrideDeviceName.get()->Text(StringToWString(deviceName));
		}
	}
}
