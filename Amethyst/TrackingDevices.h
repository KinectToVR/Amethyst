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
	inline void updateTrackingDevicesUI()
	{
		if (TrackingDevicesVector.size() < 1) return; // Just give up

		// Get the current tracking device
		const auto& trackingDevice = TrackingDevicesVector.at(k2app::K2Settings.trackingDeviceGUIDPair.second);

		std::wstring deviceName = L"[UNKNOWN]"; // Dummy name
		HRESULT device_status = E_FAIL;
		std::wstring device_status_string = L"Something's wrong!\nE_UKNOWN\nWhat's happened here?"; // Dummy status
		const bool _flip_enabled_backup = k2app::K2Settings.isFlipEnabled;

		if (trackingDevice.index() == 0)
		{
			// Kinect Basis
			const auto device = std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(trackingDevice);

			device_status = device->getStatusResult();
			device_status_string = device->statusResultWString(device_status);
			deviceName = device->getDeviceName();

			// Optionally disable flip (used later, saved later)
			k2app::K2Settings.isFlipEnabled = k2app::K2Settings.isFlipEnabled && device->isFlipSupported();
		}
		else if (trackingDevice.index() == 1)
		{
			// Joints Basis
			const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

			device_status = device->getStatusResult();
			device_status_string = device->statusResultWString(device_status);
			deviceName = device->getDeviceName();

			// Disable flip (used later, saved later)
			k2app::K2Settings.isFlipEnabled = false;
		}

		// Save settings if changed
		if (_flip_enabled_backup != k2app::K2Settings.isFlipEnabled)
			k2app::K2Settings.saveSettings();

		/* Update the device in general tab */

		// Update the status here
		bool base_status_ok = device_status == S_OK;
		bool override_status_ok = true; // Assume success :)
		std::pair<std::wstring, int32_t> devices_navigate_index{L"", -1};

		// Loop over all the override indexes and check them
		for (auto& [_override_guid, _override_id] : k2app::K2Settings.overrideDeviceGUIDsMap)
			// Check the device status
			if (deviceGUID_ID_Map.contains(_override_guid))
			{
				const auto& overrideDevice = TrackingDevicesVector.at(_override_id);
				switch (overrideDevice.index())
				{
				case 0:
					// Kinect Basis
					if (std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(overrideDevice)->getStatusResult() != S_OK)
					{
						override_status_ok = false; // Failure!

						// Overwrite the select index if not changed yet
						if (devices_navigate_index.second < 0)
							devices_navigate_index = {
								std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(overrideDevice)->getDeviceGUID(),
								_override_id
							};
					}
					break;
				case 1:
					// Joints Basis
					if (std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(overrideDevice)->getStatusResult() != S_OK)
					{
						override_status_ok = false; // Failure!

						// Overwrite the select index if not changed yet
						if (devices_navigate_index.second < 0)
							devices_navigate_index = {
								std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(overrideDevice)->getDeviceGUID(),
								_override_id
							};
					}
					break;
				}
			}

		k2app::shared::general::additionalDeviceErrorsHyperlink_TappedEvent =
			[&, devices_navigate_index]()-> winrt::Windows::Foundation::IAsyncAction
			{
				// Play a sound
				playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);

				// Navigate to the devices page
				k2app::shared::main::mainNavigationView->SelectedItem(
					k2app::shared::main::mainNavigationView->MenuItems().GetAt(2));
				k2app::shared::main::NavView_Navigate(
					L"devices", winrt::Microsoft::UI::Xaml::Media::Animation::EntranceNavigationTransitionInfo());

				// Wait a bit
				{
					winrt::apartment_context ui_thread;
					co_await winrt::resume_background();
					Sleep(500);
					co_await ui_thread;
				}

				// Should already be init-ed after 500ms, but check anyway
				if (k2app::shared::devices::devicesTreeView.get() != nullptr
					&& devices_navigate_index.second >= 0)
				{
					k2app::shared::devices::devicesTreeView.get()->SelectedNode(
						k2app::shared::devices::devicesTreeView.get()->RootNodes().
						                                        GetAt(devices_navigate_index.second));

					// Backup
					const bool _anim = devices_navigate_index.first ==
						k2app::shared::devices::selectedTrackingDeviceGUIDPair.first;

					// Overwrite the selected device
					k2app::shared::devices::selectedTrackingDeviceGUIDPair = devices_navigate_index;

					// Reload the tracking device UI (no animations if unchanged)
					k2app::shared::devices::ReloadSelectedDevice(_anim);

					// Backup
					k2app::shared::devices::previousSelectedTrackingDeviceGUIDPair =
						k2app::shared::devices::selectedTrackingDeviceGUIDPair;
				}
			};

		using namespace winrt::Microsoft::UI::Xaml;

		// Check with this one, should be the same for all anyway
		if (k2app::shared::general::errorWhatText.get() != nullptr)
		{
			// Don't show device errors if we've got a server error
			if (!k2app::interfacing::isServerDriverPresent)
				base_status_ok = true; // Skip if the server's failed

			// If the server driver's OK
			k2app::shared::general::errorWhatText.get()->Visibility(
				base_status_ok ? Visibility::Collapsed : Visibility::Visible);
			k2app::shared::general::errorWhatGrid.get()->Visibility(
				base_status_ok ? Visibility::Collapsed : Visibility::Visible);
			k2app::shared::general::errorButtonsGrid.get()->Visibility(
				base_status_ok ? Visibility::Collapsed : Visibility::Visible);
			k2app::shared::general::trackingDeviceErrorLabel.get()->Visibility(
				base_status_ok ? Visibility::Collapsed : Visibility::Visible);

			// Split status and message by \n
			k2app::shared::general::deviceNameLabel.get()->Text(deviceName);
			k2app::shared::general::deviceStatusLabel.get()->Text(split_status(device_status_string)[0]);
			k2app::shared::general::trackingDeviceErrorLabel.get()->Text(split_status(device_status_string)[1]);
			k2app::shared::general::errorWhatText.get()->Text(split_status(device_status_string)[2]);

			// TODO DIM THE CALIBRATION BUTTON IF NOT OKAY
		}

		// Check with this one, should be the same for all anyway
		if (k2app::shared::general::additionalDeviceErrorsHyperlink.get() != nullptr)
		{
			// Show the 'additional errors' button if there are errors
			k2app::shared::general::additionalDeviceErrorsHyperlink.get()->Visibility(
				// Skip if the server driver's not present
				(override_status_ok || !k2app::interfacing::isServerDriverPresent)
					? Visibility::Collapsed
					: Visibility::Visible);

			// Set the app state (optionally)
			if (k2app::interfacing::currentPageTag == L"devices")
			{
				if (k2app::shared::devices::devicesTreeView.get() != nullptr && override_status_ok &&
					IsAnOverride(k2app::shared::devices::selectedTrackingDeviceGUIDPair.first))
					k2app::interfacing::currentAppState = L"overrides";
				else
					k2app::interfacing::currentAppState = L"devices";
			}
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

				for (const auto& expander : k2app::shared::settings::jointExpanderVector)
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
			}
			else if (trackingDevice.index() == 1)
			{
				// Joints Basis
				for (const auto& expander : k2app::shared::settings::jointExpanderVector)
					expander->EnableSoftwareOrientation(false);

				k2app::shared::settings::flipToggle.get()->IsEnabled(false);
				k2app::shared::settings::flipDropDown.get()->IsEnabled(false);
				k2app::shared::settings::flipDropDownGrid.get()->Opacity(0.5);

				// Hide the flip controls container
				k2app::shared::settings::flipDropDownContainer.get()->Visibility(Visibility::Collapsed);
			}
			// Update extflip
			settings_set_external_flip_is_enabled();
		}

		// Refresh the device list MVVM
		RefreshDevicesMVVMList();
	}

	inline int32_t devices_override_joint_id(
		const uint32_t& override_id, const int32_t& joint_id)
	{
		const auto& _override = TrackingDevicesVector[override_id];

		if (_override.index() != 0)return joint_id;

		if (std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(
				_override)->getDeviceCharacteristics()
			> ktvr::K2_Character_Basic)
		{
			switch (joint_id)
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
		switch (joint_id)
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

	inline int32_t devices_override_joint_id_reverse(
		const uint32_t& override_id, const int32_t& joint_id)
	{
		const auto& _override = TrackingDevicesVector[override_id];

		if (_override.index() != 0)return joint_id;

		if (std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(
				_override)->getDeviceCharacteristics()
			> ktvr::K2_Character_Basic)
		{
			switch (joint_id)
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
		switch (joint_id)
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

	inline void devices_handle_refresh(const bool& reconnect)
	{
		using namespace winrt::Microsoft::UI::Xaml;

		// Just give up if not set up yet
		if (k2app::shared::devices::jointBasisLabel.get() == nullptr)return;

		k2app::shared::devices::devices_signal_joints = false; // Don't signal on status refresh

		auto& trackingDevice =
			TrackingDevicesVector.at(k2app::shared::devices::selectedTrackingDeviceGUIDPair.second);

		std::wstring device_status = L"Something's wrong!\nE_UKNOWN\nWhat's happened here?";
		LOG(INFO) << "Now " << (reconnect ? "reconnecting and refreshing" : "refreshing") <<
			" the tracking device at index " << k2app::shared::devices::selectedTrackingDeviceGUIDPair.second << "...";

		if (trackingDevice.index() == 0)
		{
			// Kinect Basis
			const auto& device =
				std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(trackingDevice);

			if (reconnect) device->initialize();
			device_status = device->statusResultWString(device->getStatusResult());

			// Update the device name
			k2app::shared::devices::deviceNameLabel.get()->Text(device->getDeviceName());

			// We've selected a SkeletonBasis device, so this should be hidden
			for (auto& expander : k2app::shared::devices::jointSelectorExpanders)
				expander.get()->SetVisibility(Visibility::Collapsed);

			k2app::shared::devices::jointBasisLabel.get()->Visibility(Visibility::Collapsed);

			// Show / Hide the override expanders
			for (auto& expander :
			     k2app::shared::devices::overrideSelectorExpanders)
				// Collapse on fails & non-overrides
				expander.get()->SetVisibility(
					device_status.find(L"S_OK") != std::wstring::npos &&
					IsAnOverride(device->getDeviceGUID())
						? Visibility::Visible
						: Visibility::Collapsed);

			// Collapse on fails & non-overrides
			k2app::shared::devices::overridesLabel.get()->Visibility(
				device_status.find(L"S_OK") != std::wstring::npos &&
				IsAnOverride(device->getDeviceGUID())
					? Visibility::Visible
					: Visibility::Collapsed);

			// Set up combos if the device's OK
			if (device_status.find(L"S_OK") != std::wstring::npos)
			{
				// If we're reconnecting an override device, also refresh joints
				if (IsAnOverride(k2app::shared::devices::selectedTrackingDeviceGUIDPair.first))
				{
					// Clear items
					for (auto& expander : k2app::shared::devices::overrideSelectorExpanders)
						expander.get()->ReAppendTrackers();

					// Push the placeholder to all combos
					for (auto& expander : k2app::shared::devices::overrideSelectorExpanders)
						expander.get()->PushOverrideJoint(
							k2app::interfacing::LocalizedResourceWString(
								L"DevicesPage", L"Placeholders/Overrides/NoOverride/PlaceholderText"), true);

					// Append all joints to all combos, depend on characteristics
					switch (device->getDeviceCharacteristics())
					{
					case ktvr::K2_Character_Basic:
						{
							for (auto& expander : k2app::shared::devices::overrideSelectorExpanders)
								expander.get()->PushOverrideJoints(false);
						}
						break;
					case ktvr::K2_Character_Simple:
						{
							for (auto& expander : k2app::shared::devices::overrideSelectorExpanders)
								expander.get()->PushOverrideJoints();
						}
						break;
					case ktvr::K2_Character_Full:
						{
							for (auto& expander : k2app::shared::devices::overrideSelectorExpanders)
								expander.get()->PushOverrideJoints();
						}
						break;
					}

					// Try fix override IDs if wrong
					devices_check_override_ids(k2app::shared::devices::selectedTrackingDeviceGUIDPair.second);

					for (auto& expander : k2app::shared::devices::overrideSelectorExpanders)
					{
						// Select the first (or next, if exists) joint
						// Set the placeholder text on disabled combos
						expander.get()->SelectComboItems();

						// Select enabled overrides
						expander.get()->UpdateOverrideToggles();
					}
				}
			}

			// Show / Hide device settings
			k2app::shared::devices::selectedDeviceSettingsHostContainer.get()->Visibility(
				device->isSettingsDaemonSupported()
					? Visibility::Visible
					: Visibility::Collapsed);

			// Append device settings / placeholder layout
			k2app::shared::devices::selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
			k2app::shared::devices::selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
				device->isSettingsDaemonSupported()
					? *TrackingDevicesLayoutRootsVector.at(
						k2app::shared::devices::selectedTrackingDeviceGUIDPair.second)->Get()
					: *k2app::interfacing::emptyLayoutRoot->Get());
		}
		else if (trackingDevice.index() == 1)
		{
			// Joints Basis
			const auto& device =
				std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

			if (reconnect) device->initialize();
			device_status = device->statusResultWString(device->getStatusResult());

			// Update the device name
			k2app::shared::devices::deviceNameLabel.get()->Text(device->getDeviceName());

			// We've selected a jointsbasis device, so this should be visible
			//	at least when the device is online
			for (auto& expander : k2app::shared::devices::jointSelectorExpanders)
				expander.get()->SetVisibility(
					(device_status.find(L"S_OK") != std::wstring::npos &&
						IsABase(k2app::shared::devices::selectedTrackingDeviceGUIDPair.first))
						? Visibility::Visible
						: Visibility::Collapsed);

			k2app::shared::devices::jointBasisLabel.get()->Visibility(
				(device_status.find(L"S_OK") != std::wstring::npos &&
					IsABase(k2app::shared::devices::selectedTrackingDeviceGUIDPair.first))
					? Visibility::Visible
					: Visibility::Collapsed);

			// Show / Hide the override expanders
			for (auto& expander :
			     k2app::shared::devices::overrideSelectorExpanders)
				// Collapse on fails & non-overrides
				expander.get()->SetVisibility(
					device_status.find(L"S_OK") != std::wstring::npos &&
					IsAnOverride(device->getDeviceGUID())
						? Visibility::Visible
						: Visibility::Collapsed);

			// Collapse on fails & non-overrides
			k2app::shared::devices::overridesLabel.get()->Visibility(
				device_status.find(L"S_OK") != std::wstring::npos &&
				IsAnOverride(device->getDeviceGUID())
					? Visibility::Visible
					: Visibility::Collapsed);

			// Set up combos if the device's OK
			if (device_status.find(L"S_OK") != std::wstring::npos)
			{
				// If we're reconnecting a base device, also refresh joints
				if (IsABase(k2app::shared::devices::selectedTrackingDeviceGUIDPair.first))
				{
					for (auto& expander : k2app::shared::devices::jointSelectorExpanders)
						expander.get()->ReAppendTrackers();
				}

				// If we're reconnecting an override device, also refresh joints
				else if (IsAnOverride(k2app::shared::devices::selectedTrackingDeviceGUIDPair.first))
				{
					// Clear items
					for (auto& expander : k2app::shared::devices::overrideSelectorExpanders)
						expander.get()->ReAppendTrackers();

					// Push the placeholder to all combos
					for (auto& expander : k2app::shared::devices::overrideSelectorExpanders)
						expander.get()->PushOverrideJoint(
							k2app::interfacing::LocalizedResourceWString(
								L"DevicesPage", L"Placeholders/Overrides/NoOverride/PlaceholderText"), true);

					// Append all joints to all combos
					for (const auto& _joint : device->getTrackedJoints())
						// Push the name to all combos
						for (auto& expander : k2app::shared::devices::overrideSelectorExpanders)
							expander.get()->PushOverrideJoint(_joint.getJointName());


					// Try fix override IDs if wrong
					devices_check_override_ids(k2app::shared::devices::selectedTrackingDeviceGUIDPair.second);

					for (auto& expander : k2app::shared::devices::overrideSelectorExpanders)
					{
						// Select the first (or next, if exists) joint
						// Set the placeholder text on disabled combos
						expander.get()->SelectComboItems();

						// Select enabled overrides
						expander.get()->UpdateOverrideToggles();
					}
				}
			}

			// Show / Hide device settings
			k2app::shared::devices::selectedDeviceSettingsHostContainer.get()->Visibility(
				device->isSettingsDaemonSupported()
					? Visibility::Visible
					: Visibility::Collapsed);

			// Append device settings / placeholder layout
			k2app::shared::devices::selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
			k2app::shared::devices::selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
				device->isSettingsDaemonSupported()
					? *TrackingDevicesLayoutRootsVector.at(
						k2app::shared::devices::selectedTrackingDeviceGUIDPair.second)->Get()
					: *k2app::interfacing::emptyLayoutRoot->Get());
		}

		// Check if we've disabled any joints from spawning and disable they're mods
		k2app::interfacing::devices_check_disabled_joints();

		/* Update local statuses */

		// Update the status here
		const bool status_ok = device_status.find(L"S_OK") != std::wstring::npos;

		k2app::shared::devices::errorWhatText.get()->Visibility(
			status_ok ? Visibility::Collapsed : Visibility::Visible);
		k2app::shared::devices::deviceErrorGrid.get()->Visibility(
			status_ok ? Visibility::Collapsed : Visibility::Visible);
		k2app::shared::devices::trackingDeviceErrorLabel.get()->Visibility(
			status_ok ? Visibility::Collapsed : Visibility::Visible);

		k2app::shared::devices::trackingDeviceChangePanel.get()->Visibility(
			status_ok ? Visibility::Visible : Visibility::Collapsed);

		// Split status and message by \n
		k2app::shared::devices::deviceStatusLabel.get()->Text(split_status(device_status)[0]);
		k2app::shared::devices::trackingDeviceErrorLabel.get()->Text(split_status(device_status)[1]);
		k2app::shared::devices::errorWhatText.get()->Text(split_status(device_status)[2]);

		// Refresh the device list MVVM
		RefreshDevicesMVVMList();

		k2app::shared::devices::devices_signal_joints = true; // Change back
	}

	// Refresh the device list
	inline void RefreshDevicesMVVMList()
	{
		// Don't even try if not set up yet (or not on the right page)
		if (!k2app::shared::devices::devices_mvvm_setup_finished
			|| k2app::interfacing::currentPageTag != L"devices")
			return;

#ifdef _DEBUG
		LOG(INFO) << "Refreshing the tracking devices' MVVM...";
#endif

		// Refresh the device list MVVM
		for (const auto& device : TrackingDevicesVector)
		{
			std::wstring deviceGUID = L"INVALID";
			HRESULT deviceStatus = E_FAIL;

			switch (device.index())
			{
			case 0:
				{
					const auto& pDevice = std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(device);
					deviceGUID = pDevice->getDeviceGUID();
					deviceStatus = pDevice->getStatusResult();
				}
				break;
			case 1:
				{
					const auto& pDevice = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(device);
					deviceGUID = pDevice->getDeviceGUID();
					deviceStatus = pDevice->getStatusResult();
				}
				break;
			}

#ifdef _DEBUG
			LOG(INFO) << WStringToString(std::format(L"Refreshing GUID: \"{}\" ", deviceGUID)) <<
				" tracking devices' list entry...";
#endif

			const bool _isBase = IsABase(deviceGUID),
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
