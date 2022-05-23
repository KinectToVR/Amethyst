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

	// Variant of current devices' layout root pointers
	// Note: the size must be the same as TrackingDevicesVector's
	inline std::vector<k2app::interfacing::AppInterface::AppLayoutRoot*>
	TrackingDevicesLayoutRootsVector;

	// Pointer to the device's constructing function
	using TrackingDeviceBaseFactory = void* (*)(const char* pVersionName, int* pReturnCode);

	// Extract the current device (variant of it)
	inline auto getCurrentDevice()
	{
		// trackingDeviceID is always >= 0 anyway
		return TrackingDevicesVector.at(k2app::K2Settings.trackingDeviceID);
	}

	// Extract the current device (variant of it)
	inline auto getCurrentDevice(const uint32_t& id)
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
			ktvr::K2TrackingDeviceBase_JointsBasis*>> getCurrentOverrideDevice_Safe(const uint32_t& id)
	{
		bool _exists = TrackingDevicesVector.size() > id;

		// Assuming that the caller will test in pair.first is true,
		// we can push the id0 device here as well if pair.first is gonna be false
		uint32_t _deviceID = _exists ? id : 0;

		// trackingDeviceID is always >= 0 anyway
		return std::make_pair(_exists,
		                      TrackingDevicesVector.at(_deviceID));
	}

	inline bool isExternalFlipSupportable()
	{
		bool isFlipSupported = false;

		/* First check if our tracking device even supports normal flip */

		const auto& trackingDevice =
			getCurrentDevice();

		if (trackingDevice.index() == 0)
			isFlipSupported = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(
				trackingDevice)->isFlipSupported();

		bool isExternalFlipSupported = false, // inapp - overridden/disabled
		     isExternalFlipSupported_Global = false; // global - steamvr

		/* Now check if either waist tracker is overridden or disabled
		 * And then search in OpenVR for a one with waist role */

		const auto& overrideDevice =
			getCurrentOverrideDevice_Safe();

		// If we have an override and if it's actually affecting the waist rotation
		if (overrideDevice.first &&
			k2app::K2Settings.isJointPairEnabled[0] &&
			k2app::K2Settings.isRotationOverriddenJoint[0])
		{
			// If the override device is a kinect then it HAS NOT TO support flip
			if (overrideDevice.second.index() == 0)
				isExternalFlipSupported = !std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(
					overrideDevice.second)->isFlipSupported();

				// If the override device is a joints then it's always ok
			else if (overrideDevice.second.index() == 1)
				isExternalFlipSupported = true;
		}

		// If still not, then also check if the waist is disabled by any chance
		else if (!k2app::K2Settings.isJointPairEnabled[0])
			isExternalFlipSupported = true;

		/* Here check if there's a proper waist tracker in steamvr to pull data from */
		if (isExternalFlipSupported)
			isExternalFlipSupported_Global = k2app::interfacing::findVRTracker("waist").first; // .first is [Success?]

		return isExternalFlipSupported_Global;
	}

	// autoCheck->true will force the function to check and false will assume unsupported
	inline void settings_set_external_flip_is_enabled(bool autoCheck = true)
	{
		if (k2app::shared::settings::externalFlipCheckBox.get() == nullptr)return;

		if (autoCheck)
		{
			k2app::shared::settings::externalFlipCheckBox.get()->IsEnabled(
				isExternalFlipSupportable() &&
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

		if (k2app::shared::settings::softwareRotationItem.get() != nullptr)
		{
			if (trackingDevice.index() == 0)
			{
				// Kinect Basis
				k2app::shared::settings::flipToggle.get()->IsOn(k2app::K2Settings.isFlipEnabled);
				k2app::shared::settings::softwareRotationItem.get()->IsEnabled(
					std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isAppOrientationSupported());
				k2app::shared::settings::flipToggle.get()->IsEnabled(
					std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported());
				k2app::shared::settings::flipDropDown.get()->IsEnabled(
					std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported());
				k2app::shared::settings::flipDropDownGrid.get()->Opacity(
					k2app::shared::settings::flipToggle.get()->IsEnabled() ? 1 : 0.5);

				settings_set_external_flip_is_enabled();

				if (!std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isAppOrientationSupported() &&
					(k2app::K2Settings.jointRotationTrackingOption[1] == k2app::k2_SoftwareCalculatedRotation ||
						k2app::K2Settings.jointRotationTrackingOption[2] == k2app::k2_SoftwareCalculatedRotation))
				{
					k2app::K2Settings.jointRotationTrackingOption[1] = k2app::k2_DeviceInferredRotation;
					k2app::K2Settings.jointRotationTrackingOption[2] = k2app::k2_DeviceInferredRotation;

					k2app::shared::settings::feetRotationFilterOptionBox.get()->SelectedIndex(
						k2app::K2Settings.jointRotationTrackingOption[1]); // Feet
				}
			}
			else if (trackingDevice.index() == 1)
			{
				// Joints Basis
				k2app::K2Settings.isFlipEnabled = false;
				k2app::shared::settings::softwareRotationItem.get()->IsEnabled(false);
				k2app::shared::settings::flipToggle.get()->IsEnabled(false);
				k2app::shared::settings::flipDropDown.get()->IsEnabled(false);
				k2app::shared::settings::flipDropDownGrid.get()->Opacity(0.5);

				settings_set_external_flip_is_enabled(false);

				if (k2app::K2Settings.jointRotationTrackingOption[1] == k2app::k2_SoftwareCalculatedRotation ||
					k2app::K2Settings.jointRotationTrackingOption[2] == k2app::k2_SoftwareCalculatedRotation)
				{
					k2app::K2Settings.jointRotationTrackingOption[1] = k2app::k2_DeviceInferredRotation;
					k2app::K2Settings.jointRotationTrackingOption[2] = k2app::k2_DeviceInferredRotation;

					k2app::shared::settings::feetRotationFilterOptionBox.get()->SelectedIndex(
						k2app::K2Settings.jointRotationTrackingOption[1]); // Feet
				}
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

				if (k2app::K2Settings.positionOverrideJointID[0] >= num_joints)
					k2app::K2Settings.positionOverrideJointID[0] = 0;
				if (k2app::K2Settings.positionOverrideJointID[1] >= num_joints)
					k2app::K2Settings.positionOverrideJointID[1] = 0;
				if (k2app::K2Settings.positionOverrideJointID[2] >= num_joints)
					k2app::K2Settings.positionOverrideJointID[2] = 0;
				if (k2app::K2Settings.positionOverrideJointID[3] >= num_joints)
					k2app::K2Settings.positionOverrideJointID[3] = 0;
				if (k2app::K2Settings.positionOverrideJointID[4] >= num_joints)
					k2app::K2Settings.positionOverrideJointID[4] = 0;
				if (k2app::K2Settings.positionOverrideJointID[5] >= num_joints)
					k2app::K2Settings.positionOverrideJointID[5] = 0;
				if (k2app::K2Settings.positionOverrideJointID[6] >= num_joints)
					k2app::K2Settings.positionOverrideJointID[6] = 0;

				if (k2app::K2Settings.rotationOverrideJointID[0] >= num_joints)
					k2app::K2Settings.rotationOverrideJointID[0] = 0;
				if (k2app::K2Settings.rotationOverrideJointID[1] >= num_joints)
					k2app::K2Settings.rotationOverrideJointID[1] = 0;
				if (k2app::K2Settings.rotationOverrideJointID[2] >= num_joints)
					k2app::K2Settings.rotationOverrideJointID[2] = 0;
				if (k2app::K2Settings.rotationOverrideJointID[3] >= num_joints)
					k2app::K2Settings.rotationOverrideJointID[3] = 0;
				if (k2app::K2Settings.rotationOverrideJointID[4] >= num_joints)
					k2app::K2Settings.rotationOverrideJointID[4] = 0;
				if (k2app::K2Settings.rotationOverrideJointID[5] >= num_joints)
					k2app::K2Settings.rotationOverrideJointID[5] = 0;
				if (k2app::K2Settings.rotationOverrideJointID[6] >= num_joints)
					k2app::K2Settings.rotationOverrideJointID[6] = 0;
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

				if (k2app::K2Settings.positionOverrideJointID[0] >= num_joints)
					k2app::K2Settings.positionOverrideJointID[0] = 0;
				if (k2app::K2Settings.positionOverrideJointID[1] >= num_joints)
					k2app::K2Settings.positionOverrideJointID[1] = 0;
				if (k2app::K2Settings.positionOverrideJointID[2] >= num_joints)
					k2app::K2Settings.positionOverrideJointID[2] = 0;
				if (k2app::K2Settings.positionOverrideJointID[3] >= num_joints)
					k2app::K2Settings.positionOverrideJointID[3] = 0;
				if (k2app::K2Settings.positionOverrideJointID[4] >= num_joints)
					k2app::K2Settings.positionOverrideJointID[4] = 0;
				if (k2app::K2Settings.positionOverrideJointID[5] >= num_joints)
					k2app::K2Settings.positionOverrideJointID[5] = 0;
				if (k2app::K2Settings.positionOverrideJointID[6] >= num_joints)
					k2app::K2Settings.positionOverrideJointID[6] = 0;

				if (k2app::K2Settings.rotationOverrideJointID[0] >= num_joints)
					k2app::K2Settings.rotationOverrideJointID[0] = 0;
				if (k2app::K2Settings.rotationOverrideJointID[1] >= num_joints)
					k2app::K2Settings.rotationOverrideJointID[1] = 0;
				if (k2app::K2Settings.rotationOverrideJointID[2] >= num_joints)
					k2app::K2Settings.rotationOverrideJointID[2] = 0;
				if (k2app::K2Settings.rotationOverrideJointID[3] >= num_joints)
					k2app::K2Settings.rotationOverrideJointID[3] = 0;
				if (k2app::K2Settings.rotationOverrideJointID[4] >= num_joints)
					k2app::K2Settings.rotationOverrideJointID[4] = 0;
				if (k2app::K2Settings.rotationOverrideJointID[5] >= num_joints)
					k2app::K2Settings.rotationOverrideJointID[5] = 0;
				if (k2app::K2Settings.rotationOverrideJointID[6] >= num_joints)
					k2app::K2Settings.rotationOverrideJointID[6] = 0;
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

			if (k2app::K2Settings.selectedTrackedJointID[0] >= num_joints)
				k2app::K2Settings.selectedTrackedJointID[0] = 0;
			if (k2app::K2Settings.selectedTrackedJointID[1] >= num_joints)
				k2app::K2Settings.selectedTrackedJointID[1] = 0;
			if (k2app::K2Settings.selectedTrackedJointID[2] >= num_joints)
				k2app::K2Settings.selectedTrackedJointID[2] = 0;
			if (k2app::K2Settings.selectedTrackedJointID[3] >= num_joints)
				k2app::K2Settings.selectedTrackedJointID[3] = 0;
			if (k2app::K2Settings.selectedTrackedJointID[4] >= num_joints)
				k2app::K2Settings.selectedTrackedJointID[4] = 0;
			if (k2app::K2Settings.selectedTrackedJointID[5] >= num_joints)
				k2app::K2Settings.selectedTrackedJointID[5] = 0;
			if (k2app::K2Settings.selectedTrackedJointID[6] >= num_joints)
				k2app::K2Settings.selectedTrackedJointID[6] = 0;

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

	inline void settings_trackersConfig_UpdateIsEnabled()
	{
		// Skip if not set up yet
		if (k2app::shared::settings::flipDropDown.get() == nullptr)return;

		// Make expander opacity .5 and collapse it
		// to imitate that it's disabled

		// Flip
		if (!k2app::K2Settings.isFlipEnabled)
		{
			k2app::shared::settings::flipDropDown.get()->IsEnabled(false);
			k2app::shared::settings::flipDropDown.get()->IsExpanded(false);
		}
		else
			k2app::shared::settings::flipDropDown.get()->IsEnabled(true);

		// Waist
		if (!k2app::K2Settings.isJointPairEnabled[0])
		{
			k2app::shared::settings::waistDropDown.get()->IsEnabled(false);
			k2app::shared::settings::waistDropDown.get()->IsExpanded(false);
		}
		else
			k2app::shared::settings::waistDropDown.get()->IsEnabled(true);

		// Feet
		if (!k2app::K2Settings.isJointPairEnabled[1])
		{
			k2app::shared::settings::feetDropDown.get()->IsEnabled(false);
			k2app::shared::settings::feetDropDown.get()->IsExpanded(false);
		}
		else
			k2app::shared::settings::feetDropDown.get()->IsEnabled(true);

		// Elbows
		if (!k2app::K2Settings.isJointPairEnabled[2])
		{
			k2app::shared::settings::elbowsDropDown.get()->IsEnabled(false);
			k2app::shared::settings::elbowsDropDown.get()->IsExpanded(false);
		}
		else
			k2app::shared::settings::elbowsDropDown.get()->IsEnabled(true);

		// Knees
		if (!k2app::K2Settings.isJointPairEnabled[3])
		{
			k2app::shared::settings::kneesDropDown.get()->IsEnabled(false);
			k2app::shared::settings::kneesDropDown.get()->IsExpanded(false);
		}
		else
			k2app::shared::settings::kneesDropDown.get()->IsEnabled(true);
	}

	inline void settings_trackersConfigChanged(const bool showToasts = true)
	{
		// Don't react to pre-init signals
		if (!k2app::shared::settings::settings_localInitFinished)return;

		// If this is the first time, also show the notification
		if (k2app::shared::settings::restartButton.get() != nullptr && showToasts)
			if (!k2app::shared::settings::restartButton.get()->IsEnabled())
				k2app::interfacing::ShowToast(
					k2app::interfacing::LocalizedResourceWString(L"SharedStrings", L"Toasts/TrackersConfigChanged/Title"),
					k2app::interfacing::LocalizedResourceWString(L"SharedStrings", L"Toasts/TrackersConfigChanged/Content"));

		// If all trackers were turned off then SCREAM
		if (showToasts && std::ranges::all_of(
			k2app::K2Settings.isJointPairEnabled,
			[](const bool& i) { return !i; }
		))
			k2app::interfacing::ShowToast(L"YOU'VE JUST DISABLED ALL TRACKERS",
				L"WHAT SORT OF A TOTAL FUCKING LIFE FAILURE ARE YOU TO DO THAT YOU STUPID BITCH LOSER?!?!");

		// Compare with saved settings and unlock the restart
		if (k2app::shared::settings::restartButton.get() != nullptr)
			k2app::shared::settings::restartButton.get()->IsEnabled(true);

		// Enable/Disable combos
		TrackingDevices::settings_trackersConfig_UpdateIsEnabled();

		// Enable/Disable ExtFlip
		TrackingDevices::settings_set_external_flip_is_enabled();

		// Save settings
		k2app::K2Settings.saveSettings();
	}
}
