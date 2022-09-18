#pragma once
#include "pch.h"

#include "K2Interfacing.h"
#include "K2Shared.h"

#include "JointExpander.h"
#include "JointSelectorExpander.h"
#include "OverrideSelectorExpander.h"

namespace TrackingDevices
{
	// Vector of current devices' layout root pointers
	// Note: the size must be the same as TrackingDevicesVector's
	inline std::vector<k2app::interfacing::AppInterface::AppLayoutRoot*>
	TrackingDevicesLayoutRootsVector;

	// Pointer to the device's constructing function
	using TrackingDeviceBaseFactory = void* (*)(const char* pVersionName, int* pReturnCode);

	inline void RefreshDevicesMVVMList();

	// Select proper tracking device in the UI
	inline void updateTrackingDeviceUI()
	{
		if (TrackingDevicesVector.size() < 1) return; // Just give up

		// Get the current tracking device
		auto& trackingDevice = TrackingDevicesVector.at(k2app::K2Settings.trackingDeviceID);

		std::wstring deviceName = L"[UNKNOWN]"; // Dummy name
		std::wstring device_status = L"Something's wrong!\nE_UKNOWN\nWhat's happened here?"; // Dummy status
		bool _flip_enabled_backup = k2app::K2Settings.isFlipEnabled;

		if (trackingDevice.index() == 0)
		{
			// Kinect Basis
			const auto device = std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(trackingDevice);

			device_status = device->statusResultWString(device->getStatusResult());
			deviceName = device->getDeviceName();

			// Optionally disable flip (used later, saved later)
			k2app::K2Settings.isFlipEnabled = k2app::K2Settings.isFlipEnabled && device->isFlipSupported();
		}
		else if (trackingDevice.index() == 1)
		{
			// Joints Basis
			const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

			device_status = device->statusResultWString(device->getStatusResult());
			deviceName = device->getDeviceName();

			// Disable flip (used later, saved later)
			k2app::K2Settings.isFlipEnabled = false;
		}

		// Save settings if changed
		if (_flip_enabled_backup != k2app::K2Settings.isFlipEnabled)
			k2app::K2Settings.saveSettings();

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
			k2app::shared::general::deviceNameLabel.get()->Text(deviceName);
			k2app::shared::general::deviceStatusLabel.get()->Text(split_status(device_status)[0]);
			k2app::shared::general::trackingDeviceErrorLabel.get()->Text(split_status(device_status)[1]);
			k2app::shared::general::errorWhatText.get()->Text(split_status(device_status)[2]);
		}

		/* Update the device in devices tab */

		k2app::shared::devices::smphSignalCurrentUpdate.release();

		/* Update the device in settings tab */

		if (k2app::shared::settings::flipDropDown.get() != nullptr)
		{
			// Overwritten a bit earlier
			k2app::shared::settings::flipToggle.get()->IsOn(k2app::K2Settings.isFlipEnabled);

			// Enable/disable mathbased & flip elements
			if (trackingDevice.index() == 0)
			{
				// Kinect Basis
				const bool _sup = std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>
					(trackingDevice)->isAppOrientationSupported();

				for (auto expander : k2app::shared::settings::jointExpanderVector)
					expander->EnableSoftwareOrientation(_sup);

				k2app::shared::settings::flipToggle.get()->IsEnabled(
					std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(trackingDevice)->isFlipSupported());
				k2app::shared::settings::flipDropDown.get()->IsEnabled(k2app::K2Settings.isFlipEnabled &&
					std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(trackingDevice)->isFlipSupported());
				k2app::shared::settings::flipDropDownGrid.get()->Opacity(
					k2app::shared::settings::flipToggle.get()->IsEnabled() ? 1 : 0.5);

				// Hide/Show the flip controls container
				k2app::shared::settings::flipDropDownContainer.get()->Visibility(
					k2app::shared::settings::flipToggle.get()->IsEnabled()
						? Visibility::Visible
						: Visibility::Collapsed);

				settings_set_external_flip_is_enabled();
			}
			else if (trackingDevice.index() == 1)
			{
				// Joints Basis
				for (auto expander : k2app::shared::settings::jointExpanderVector)
					expander->EnableSoftwareOrientation(false);

				k2app::shared::settings::flipToggle.get()->IsEnabled(false);
				k2app::shared::settings::flipDropDown.get()->IsEnabled(false);
				k2app::shared::settings::flipDropDownGrid.get()->Opacity(0.5);

				// Hide the flip controls container
				k2app::shared::settings::flipDropDownContainer.get()->Visibility(Visibility::Collapsed);

				settings_set_external_flip_is_enabled();
			}
		}

		// Refresh the device list MVVM
		RefreshDevicesMVVMList();
	}

	// Select proper tracking device in the UI
	inline void updateOverrideDeviceUI()
	{
		if (TrackingDevicesVector.size() < 1) return; // Just give up

		// Don't show ANYTHING if we 'ven 't selected an override device
		const bool _show = (k2app::K2Settings.overrideDeviceID >= 0) && (TrackingDevicesVector.size() >= 2);
		bool status_ok = false; // Assume failure :/

		std::wstring deviceName = L"[UNKNOWN]"; // Dummy name
		std::wstring device_status = L"E_UKNOWN\nWhat's happened here?"; // Dummy status

		if (_show)
		{
			// Get the current tracking device
			auto& overrideDevice = TrackingDevicesVector.at(k2app::K2Settings.overrideDeviceID);

			if (overrideDevice.index() == 0)
			{
				// Kinect Basis
				const auto device = std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(overrideDevice);

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
			k2app::shared::general::overrideDeviceNameLabel.get()->Text(deviceName);
			k2app::shared::general::overrideDeviceStatusLabel.get()->Text(split_status(device_status)[0]);
			k2app::shared::general::overrideDeviceErrorLabel.get()->Text(split_status(device_status)[1]);
			k2app::shared::general::overrideErrorWhatText.get()->Text(split_status(device_status)[2]);

			if (k2app::interfacing::currentPageTag == L"devices")
			{
				if (k2app::shared::devices::devicesTreeView.get() != nullptr &&
					k2app::shared::devices::selectedTrackingDeviceID ==
					k2app::K2Settings.overrideDeviceID && status_ok)
					k2app::interfacing::currentAppState = L"overrides";
				else
					k2app::interfacing::currentAppState = L"devices";
			}
		}

		// Refresh the device list MVVM
		RefreshDevicesMVVMList();
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
			_override.first && _is_kinect // If we're using a SkeletonBasis
		)
		{
			if (std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(
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
			_override.first && _is_kinect // If we're using a SkeletonBasis
		)
		{
			if (std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(
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

	inline void devices_handle_refresh(const bool& reconnect)
	{
		using namespace k2app::shared::devices;
		using namespace winrt::Microsoft::UI::Xaml;

		// Just give up if not set up yet
		if (jointBasisLabel.get() == nullptr)return;

		devices_signal_joints = false; // Don't signal on status refresh

		auto& trackingDevice = TrackingDevicesVector.at(selectedTrackingDeviceID);

		std::wstring device_status = L"Something's wrong!\nE_UKNOWN\nWhat's happened here?";
		LOG(INFO) << "Now " << (reconnect ? "reconnecting and refreshing" : "refreshing") <<
			" the tracking device at index " << selectedTrackingDeviceID << "...";

		if (trackingDevice.index() == 0)
		{
			// Kinect Basis
			const auto& device = std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(trackingDevice);

			if (reconnect) device->initialize();
			device_status = device->statusResultWString(device->getStatusResult());

			// We've selected a SkeletonBasis device, so this should be hidden
			for (auto& expander : jointSelectorExpanders)
				expander.get()->SetVisibility(Visibility::Collapsed);

			jointBasisLabel.get()->Visibility(Visibility::Collapsed);

			// Set up combos if the device's OK
			if (device_status.find(L"S_OK") != std::wstring::npos)
			{
				// If we're reconnecting an override device, also refresh joints
				if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
				{
					// Clear items
					for (auto& expander : overrideSelectorExpanders)
						expander.get()->ReAppendTrackers();

					// Push the placeholder to all combos
					for (auto& expander : overrideSelectorExpanders)
						expander.get()->PushOverrideJoint(
							k2app::interfacing::LocalizedResourceWString(
								L"DevicesPage", L"Placeholders/Overrides/NoOverride/PlaceholderText"), true);

					// Append all joints to all combos, depend on characteristics
					switch (device->getDeviceCharacteristics())
					{
					case ktvr::K2_Character_Basic:
						{
							for (auto& expander : overrideSelectorExpanders)
								expander.get()->PushOverrideJoints(false);
						}
						break;
					case ktvr::K2_Character_Simple:
						{
							for (auto& expander : overrideSelectorExpanders)
								expander.get()->PushOverrideJoints();
						}
						break;
					case ktvr::K2_Character_Full:
						{
							for (auto& expander : overrideSelectorExpanders)
								expander.get()->PushOverrideJoints();
						}
						break;
					}

					// Try fix override IDs if wrong
					devices_check_override_ids(selectedTrackingDeviceID);

					for (auto& expander : overrideSelectorExpanders)
					{
						// Select the first (or next, if exists) joint
						// Set the placeholder text on disabled combos
						expander.get()->SelectComboItems();

						// Select enabled overrides
						expander.get()->UpdateOverrideToggles();
					}
				}
			}

			// Show / Hide device settings button
			selectedDeviceSettingsHostContainer.get()->Visibility(
				device->isSettingsDaemonSupported()
					? Visibility::Visible
					: Visibility::Collapsed);

			// Append device settings / placeholder layout
			selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
			selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
				device->isSettingsDaemonSupported()
					? *TrackingDevicesLayoutRootsVector.at(
						selectedTrackingDeviceID)->Get()
					: *k2app::interfacing::emptyLayoutRoot->Get());
		}
		else if (trackingDevice.index() == 1)
		{
			// Joints Basis
			const auto& device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

			if (reconnect)device->initialize();
			device_status = device->statusResultWString(device->getStatusResult());

			// We've selected a jointsbasis device, so this should be visible
			//	at least when the device is online
			for (auto& expander : jointSelectorExpanders)
				expander.get()->SetVisibility(
					(device_status.find(L"S_OK") != std::wstring::npos &&
						selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
						? Visibility::Visible
						: Visibility::Collapsed);

			jointBasisLabel.get()->Visibility(
				(device_status.find(L"S_OK") != std::wstring::npos &&
					selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
					? Visibility::Visible
					: Visibility::Collapsed);

			// Set up combos if the device's OK
			if (device_status.find(L"S_OK") != std::wstring::npos)
			{
				// If we're reconnecting a base device, also refresh joints
				if (selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				{
					for (auto& expander : jointSelectorExpanders)
						expander.get()->ReAppendTrackers();
				}
				// If we're reconnecting an override device, also refresh joints
				else if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
				{
					// Clear items
					for (auto& expander : overrideSelectorExpanders)
						expander.get()->ReAppendTrackers();

					// Push the placeholder to all combos
					for (auto& expander : overrideSelectorExpanders)
						expander.get()->PushOverrideJoint(
							k2app::interfacing::LocalizedResourceWString(
								L"DevicesPage", L"Placeholders/Overrides/NoOverride/PlaceholderText"), true);

					// Append all joints to all combos
					for (auto& _joint : device->getTrackedJoints())
						// Push the name to all combos
						for (auto& expander : overrideSelectorExpanders)
							expander.get()->PushOverrideJoint(_joint.getJointName());


					// Try fix override IDs if wrong
					devices_check_override_ids(selectedTrackingDeviceID);

					for (auto& expander : overrideSelectorExpanders)
					{
						// Select the first (or next, if exists) joint
						// Set the placeholder text on disabled combos
						expander.get()->SelectComboItems();

						// Select enabled overrides
						expander.get()->UpdateOverrideToggles();
					}
				}
			}

			// Show / Hide device settings button
			selectedDeviceSettingsHostContainer.get()->Visibility(
				device->isSettingsDaemonSupported()
					? Visibility::Visible
					: Visibility::Collapsed);

			// Append device settings / placeholder layout
			selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
			selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
				device->isSettingsDaemonSupported()
					? *TrackingDevicesLayoutRootsVector.at(
						selectedTrackingDeviceID)->Get()
					: *k2app::interfacing::emptyLayoutRoot->Get());
		}

		// Check if we've disabled any joints from spawning and disable they're mods
		k2app::interfacing::devices_check_disabled_joints();

		/* Update local statuses */

		// Update the status here
		const bool status_ok = device_status.find(L"S_OK") != std::wstring::npos;

		errorWhatText.get()->Visibility(
			status_ok ? Visibility::Collapsed : Visibility::Visible);
		deviceErrorGrid.get()->Visibility(
			status_ok ? Visibility::Collapsed : Visibility::Visible);
		trackingDeviceErrorLabel.get()->Visibility(
			status_ok ? Visibility::Collapsed : Visibility::Visible);

		trackingDeviceChangePanel.get()->Visibility(
			status_ok ? Visibility::Visible : Visibility::Collapsed);

		// Split status and message by \n
		deviceStatusLabel.get()->Text(split_status(device_status)[0]);
		trackingDeviceErrorLabel.get()->Text(split_status(device_status)[1]);
		errorWhatText.get()->Text(split_status(device_status)[2]);

		// Refresh the device list MVVM
		RefreshDevicesMVVMList();

		devices_signal_joints = true; // Change back
	}

	// Refresh the device list
	inline void RefreshDevicesMVVMList()
	{
		// Don't even try if not set up yet
		if (!k2app::shared::devices::devices_mvvm_setup_finished)return;

		LOG(INFO) << "Refreshing the tracking devices' MVVM...";

		// Refresh the device list MVVM
		for (const auto& device : TrackingDevicesVector)
		{
			std::wstring deviceName = L"[UNKNOWN]";
			std::wstring deviceGUID = L"INVALID";
			HRESULT deviceStatus = E_FAIL;

			switch (device.index())
			{
			case 0:
				{
					const auto& pDevice = std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(device);
					deviceName = pDevice->getDeviceName();
					deviceGUID = pDevice->getDeviceGUID();
					deviceStatus = pDevice->getStatusResult();
				}
				break;
			case 1:
				{
					const auto& pDevice = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(device);
					deviceName = pDevice->getDeviceName();
					deviceGUID = pDevice->getDeviceGUID();
					deviceStatus = pDevice->getStatusResult();
				}
				break;
			}

			LOG(INFO) << "Refreshing " << WStringToString(deviceName) <<
				WStringToString(std::format(L" (GUID: \"{}\") ", deviceGUID)) <<
				" tracking devices' list entry...";

			const bool _isBase = deviceGUID_ID_Map[deviceGUID] == k2app::K2Settings.trackingDeviceID,
			           _isOverride = IsAnOverride(deviceGUID);

			deviceMVVM_List.GetAt(deviceGUID_ID_Map[deviceGUID]).
			                IsBase(_isBase);

			deviceMVVM_List.GetAt(deviceGUID_ID_Map[deviceGUID]).
			                IsOverride(_isOverride);

			deviceMVVM_List.GetAt(deviceGUID_ID_Map[deviceGUID]).
			                StatusError((_isBase || _isOverride) && deviceStatus != S_OK);
		}
	}
}
