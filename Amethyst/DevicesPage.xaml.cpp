#include "pch.h"
#include "DevicesPage.xaml.h"
#if __has_include("DevicesPage.g.cpp")
#include "DevicesPage.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace k2app::shared::devices;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

bool devices_loadedOnce = false;

namespace winrt::Amethyst::implementation
{
	DevicesPage::DevicesPage()
	{
		InitializeComponent();

		LOG(INFO) << "Constructing page with tag: \"devices\"...";

		// Cache needed UI elements
		jointsBasisExpanderHostStackPanel = std::make_shared<Controls::StackPanel>(JointsBasisExpanderHostStackPanel());
		overridesExpanderHostStackPanel = std::make_shared<Controls::StackPanel>(OverridesExpanderHostStackPanel());

		deviceNameLabel = std::make_shared<Controls::TextBlock>(SelectedDeviceNameLabel());
		deviceStatusLabel = std::make_shared<Controls::TextBlock>(TrackingDeviceStatusLabel());
		errorWhatText = std::make_shared<Controls::TextBlock>(ErrorWhatText());
		trackingDeviceErrorLabel = std::make_shared<Controls::TextBlock>(TrackingDeviceErrorLabel());
		baseDeviceName = std::make_shared<Controls::TextBlock>(BaseDeviceName());
		overrideDeviceName = std::make_shared<Controls::TextBlock>(OverrideDeviceName());
		overridesLabel = std::make_shared<Controls::TextBlock>(OverridesLabel());
		jointBasisLabel = std::make_shared<Controls::TextBlock>(JointBasisLabel());

		deviceErrorGrid = std::make_shared<Controls::Grid>(DeviceErrorGrid());
		trackingDeviceChangePanel = std::make_shared<Controls::Grid>(TrackingDeviceChangePanel());
		devicesMainContentGridOuter = std::make_shared<Controls::Grid>(DevicesMainContentGridOuter());
		devicesMainContentGridInner = std::make_shared<Controls::Grid>(DevicesMainContentGridInner());
		selectedDeviceSettingsHostContainer = std::make_shared<Controls::Grid>(SelectedDeviceSettingsHostContainer());

		devicesListView = std::make_shared<Controls::ListView>(TrackingDeviceListView());
		noJointsFlyout = std::make_shared<Controls::Flyout>(NoJointsFlyout());

		k2app::shared::teaching_tips::devices::devicesListTeachingTip =
			std::make_shared<Controls::TeachingTip>(DevicesListTeachingTip());
		k2app::shared::teaching_tips::devices::deviceControlsTeachingTip =
			std::make_shared<Controls::TeachingTip>(DeviceControlsTeachingTip());

		setAsOverrideButton = std::make_shared<Controls::Button>(SetAsOverrideButton());
		setAsBaseButton = std::make_shared<Controls::Button>(SetAsBaseButton());
		deselectDeviceButton = std::make_shared<Controls::Button>(DeselectDeviceButton());

		devicesMainContentScrollViewer = std::make_shared<Controls::ScrollViewer>(DevicesMainContentScrollViewer());

		devicesOverridesSelectorStackPanelOuter = std::make_shared<Controls::StackPanel>(
			DevicesOverridesSelectorStackPanelOuter());
		devicesOverridesSelectorStackPanelInner = std::make_shared<Controls::StackPanel>(
			DevicesOverridesSelectorStackPanelInner());

		devicesJointsBasisSelectorStackPanelOuter = std::make_shared<Controls::StackPanel>(
			DevicesJointsBasisSelectorStackPanelOuter());
		devicesJointsBasisSelectorStackPanelInner = std::make_shared<Controls::StackPanel>(
			DevicesJointsBasisSelectorStackPanelInner());

		selectedDeviceSettingsRootLayoutPanel = std::make_shared<Controls::StackPanel>(
			SelectedDeviceSettingsRootLayoutPanel());

		// Create tracking devices' list
		static auto m_TrackingDevicesViewModels =
			multi_threaded_observable_vector<Amethyst::TrackingDevicesView>();

		// Watch for insertions
		m_TrackingDevicesViewModels.VectorChanged(
			[&](const Windows::Foundation::Collections::IObservableVector<Amethyst::TrackingDevicesView>& sender,
			    const Windows::Foundation::Collections::IVectorChangedEventArgs& args)
			{
				// Report a registration and parse
				LOG(INFO) << WStringToString(sender.GetAt(sender.Size() - 1).DeviceName().c_str()) <<
					"'s been registered as a tracking device. [UI Node]";

				// Set the current device
				if (sender.Size() > k2app::K2Settings.trackingDeviceID)
					sender.GetAt(k2app::K2Settings.trackingDeviceID).Current(true);

				// Re-set all indexes
				for (uint32_t i = 0; i < sender.Size(); i++)
					sender.GetAt(i).DeviceID(i);
			});

		// Add tracking devices here
		for (const auto& device : TrackingDevices::TrackingDevicesVector)
		{
			std::string deviceName = "[UNKNOWN]";

			switch (device.index())
			{
			case 0:
				{
					const auto& pDevice = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(device);
					deviceName = pDevice->getDeviceName();
				}
				break;
			case 1:
				{
					const auto& pDevice = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(device);
					deviceName = pDevice->getDeviceName();
				}
				break;
			}

			LOG(INFO) << "Appending " << deviceName <<
				" to UI Node's tracking devices' list...";
			m_TrackingDevicesViewModels.Append(
				make<TrackingDevicesView>(StringToWString(deviceName).c_str()));
		}

		// Register tracking devices' list
		devicesListView.get()->ItemsSource(m_TrackingDevicesViewModels);

		// Set currently tracking device & selected device
		// RadioButton is set on ItemChanged
		devicesListView.get()->SelectedIndex(k2app::K2Settings.trackingDeviceID);

		// Set joint expanders up

		// Type 0: WF
		jointSelectorExpanders[0] = std::move(std::make_shared<Controls::JointSelectorExpander>(0));

		// Type 1: EK
		jointSelectorExpanders[1] = std::move(std::make_shared<Controls::JointSelectorExpander>(1));

		// Type 2: OTHER
		jointSelectorExpanders[2] = std::move(std::make_shared<Controls::JointSelectorExpander>(2));

		for (auto& expander : jointSelectorExpanders)
			jointsBasisExpanderHostStackPanel->Children().Append(*expander->ContainerExpander());

		// Set override expanders up

		// Type 0: WF
		overrideSelectorExpanders[0] = std::move(std::make_shared<Controls::OverrideSelectorExpander>(0));

		// Type 1: EK
		overrideSelectorExpanders[1] = std::move(std::make_shared<Controls::OverrideSelectorExpander>(1));

		// Type 2: OTHER
		overrideSelectorExpanders[2] = std::move(std::make_shared<Controls::OverrideSelectorExpander>(2));

		for (auto& expander : overrideSelectorExpanders)
			overridesExpanderHostStackPanel->Children().Append(*expander->ContainerExpander());

		NavigationCacheMode(Navigation::NavigationCacheMode::Required);
		TrackingDevices::devices_update_current();

		LOG(INFO) << "Registering a detached binary semaphore reload handler for DevicesPage...";
		std::thread([&, this]
		{
			while (true)
			{
				// Wait for a reload signal (blocking)
				k2app::shared::semaphores::semaphore_ReloadPage_DevicesPage.acquire();

				// Reload & restart the waiting loop
				if (devices_loadedOnce)
					k2app::shared::main::thisDispatcherQueue->TryEnqueue([&, this]
					{
						LOG(INFO) << "Rebuilding joint selector expanders... this may take a while...";

						// Set joint expanders up

						// Type 0: WF
						jointSelectorExpanders[0] = std::move(std::make_shared<Controls::JointSelectorExpander>(0));

						// Type 1: EK
						jointSelectorExpanders[1] = std::move(std::make_shared<Controls::JointSelectorExpander>(1));

						// Type 2: OTHER
						jointSelectorExpanders[2] = std::move(std::make_shared<Controls::JointSelectorExpander>(2));

						jointsBasisExpanderHostStackPanel->Children().Clear();

						for (auto& expander : jointSelectorExpanders)
							jointsBasisExpanderHostStackPanel->Children().Append(*expander->ContainerExpander());

						LOG(INFO) << "Rebuilding override selector expanders... this may take a while...";

						// Set override expanders up

						// Type 0: WF
						overrideSelectorExpanders[0] = std::move(
							std::make_shared<Controls::OverrideSelectorExpander>(0));

						// Type 1: EK
						overrideSelectorExpanders[1] = std::move(
							std::make_shared<Controls::OverrideSelectorExpander>(1));

						// Type 2: OTHER
						overrideSelectorExpanders[2] = std::move(
							std::make_shared<Controls::OverrideSelectorExpander>(2));

						overridesExpanderHostStackPanel->Children().Clear();

						for (auto& expander : overrideSelectorExpanders)
							overridesExpanderHostStackPanel->Children().Append(*expander->ContainerExpander());

						TrackingDevices::devices_update_current();

						DevicesPage_Loaded_Handler();
					});

				Sleep(100); // Sleep a bit
			}
		}).detach();
	}
}


Windows::Foundation::IAsyncAction
Amethyst::implementation::DevicesPage::TrackingDeviceListView_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	if (!devices_tab_setup_finished)co_return; // Block dummy selects
	devices_signal_joints = false; // Don't signal on device selection

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);

	selectedTrackingDeviceID = sender.as<Controls::ListView>().SelectedIndex();
	const auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(selectedTrackingDeviceID);

	std::string deviceName = "[UNKNOWN]";
	std::wstring device_status = L"Something's wrong!\nE_UKNOWN\nWhat's happened here?";

	// Collapse all joint expanders
	for (auto& expander : jointSelectorExpanders)
		expander.get()->ContainerExpander().get()->IsExpanded(false);

	// Collapse all override expanders
	for (auto& expander : overrideSelectorExpanders)
		expander.get()->ContainerExpander().get()->IsExpanded(false);

	// Only if override -> select enabled combos
	if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
		for (auto& expander : overrideSelectorExpanders)
			expander.get()->UpdateOverrideToggles();

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

		deviceNameLabel.get()->Text(StringToWString(device->getDeviceName()));
		device_status = device->statusResultWString(device->getStatusResult());

		deviceName = device->getDeviceName();

		// Show / Hide device settings button
		selectedDeviceSettingsHostContainer.get()->Visibility(
			device->isSettingsDaemonSupported()
				? Visibility::Visible
				: Visibility::Collapsed);

		// Append device settings / placeholder layout
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
			device->isSettingsDaemonSupported()
				? *TrackingDevices::TrackingDevicesLayoutRootsVector.at(
					selectedTrackingDeviceID)->Get()
				: *k2app::interfacing::emptyLayoutRoot->Get());

		// We've selected a kinectbasis device, so this should be hidden
		for (auto& expander : jointSelectorExpanders)
			expander.get()->SetVisibility(Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(Visibility::Collapsed);

		// For all override devices
		{
			for (auto& expander : overrideSelectorExpanders)
				expander.get()->SetVisibility(
					(device_status.find(L"S_OK") != std::wstring::npos &&
						selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
						? Visibility::Visible
						: Visibility::Collapsed);

			overridesLabel.get()->Visibility(
				(device_status.find(L"S_OK") != std::wstring::npos &&
					selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
					? Visibility::Visible
					: Visibility::Collapsed);
		}

		// Set up combos if the device's OK
		if (device_status.find(L"S_OK") != std::wstring::npos)
		{
			// If we're reconnecting an override device, also refresh joints
			if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
			{
				// Clear items
				for (auto& expander : overrideSelectorExpanders)
					expander.get()->ReAppendTrackers();

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
				TrackingDevices::devices_check_override_ids(selectedTrackingDeviceID);

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
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

		deviceNameLabel.get()->Text(StringToWString(device->getDeviceName()));
		device_status = device->statusResultWString(device->getStatusResult());

		deviceName = device->getDeviceName();

		// Show / Hide device settings button
		selectedDeviceSettingsHostContainer.get()->Visibility(
			device->isSettingsDaemonSupported()
				? Visibility::Visible
				: Visibility::Collapsed);

		// Append device settings / placeholder layout
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
			device->isSettingsDaemonSupported()
				? *TrackingDevices::TrackingDevicesLayoutRootsVector.at(
					selectedTrackingDeviceID)->Get()
				: *k2app::interfacing::emptyLayoutRoot->Get());

		// We've selected a jointsbasis device, so this should be visible
		//	at least when the device is online

		// For base joints devices
		{
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
		}

		// For all override devices
		{
			for (auto& expander : overrideSelectorExpanders)
				expander.get()->SetVisibility(
					(device_status.find(L"S_OK") != std::wstring::npos &&
						selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
						? Visibility::Visible
						: Visibility::Collapsed);

			overridesLabel.get()->Visibility(
				(device_status.find(L"S_OK") != std::wstring::npos &&
					selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
					? Visibility::Visible
					: Visibility::Collapsed);
		}

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

				// Append all joints to all combos
				for (auto& _joint : device->getTrackedJoints())
				{
					// Get the name into string
					auto _jointname = _joint.getJointName();

					// Push the name to all combos
					for (auto& expander : overrideSelectorExpanders)
						expander.get()->PushOverrideJoint(StringToWString(_jointname));
				}

				// Try fix override IDs if wrong
				TrackingDevices::devices_check_override_ids(selectedTrackingDeviceID);

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
	}

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

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

	if (selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
	{
		LOG(INFO) << "Selected a base";
		setAsOverrideButton.get()->IsEnabled(false);
		setAsBaseButton.get()->IsEnabled(false);

		deselectDeviceButton.get()->Visibility(Visibility::Collapsed);
	}
	else if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
	{
		LOG(INFO) << "Selected an override";
		setAsOverrideButton.get()->IsEnabled(false);
		setAsBaseButton.get()->IsEnabled(true);

		deselectDeviceButton.get()->Visibility(Visibility::Visible);

		if (status_ok)
			k2app::interfacing::currentAppState = L"overrides";
	}
	else
	{
		LOG(INFO) << "Selected a [none]";
		setAsOverrideButton.get()->IsEnabled(true);
		setAsBaseButton.get()->IsEnabled(true);

		deselectDeviceButton.get()->Visibility(Visibility::Collapsed);
	}

	{
		// Remove the only one child of our outer main content grid
		// (What a bestiality it is to do that!!1)
		devicesMainContentGridOuter.get()->Children().RemoveAtEnd();

		Media::Animation::EntranceThemeTransition t;
		t.IsStaggeringEnabled(true);

		devicesMainContentGridInner.get()->Transitions().Append(t);

		// Sleep peacefully pretending that noting happened
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(10); // 10ms for slower systems
		co_await ui_thread;

		// Re-add the child for it to play our funky transition
		// (Though it's not the same as before...)
		devicesMainContentGridOuter.get()->Children().
		                            Append(*devicesMainContentGridInner);
	}

	devices_signal_joints = true; // Change back
	LOG(INFO) << "Changed the currently selected device to " << deviceName;

	// Remove the transition
	apartment_context ui_thread;
	co_await resume_background();
	Sleep(100);
	co_await ui_thread;

	devicesMainContentGridInner.get()->Transitions().Clear();
}


void Amethyst::implementation::DevicesPage::ReconnectDeviceButton_Click(
	const Controls::SplitButton& sender,
	const Controls::SplitButtonClickEventArgs& args)
{
	TrackingDevices::devices_handle_refresh(true);

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);

	// Update the GeneralPage status
	TrackingDevices::updateTrackingDeviceUI();
	TrackingDevices::updateOverrideDeviceUI(); // Auto-handles if none
}

// *Nearly* the same as reconnect
void Amethyst::implementation::DevicesPage::DisconnectDeviceButton_Click(
	const Windows::Foundation::IInspectable& sender,
	const RoutedEventArgs& e)
{
	auto _index = devicesListView.get()->SelectedIndex();

	auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(_index);
	std::wstring device_status = L"Something's wrong!\nE_UKNOWN\nWhat's happened here?";
	LOG(INFO) << "Now disconnecting the tracking device...";

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto& device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

		device->shutdown();
		device_status = device->statusResultWString(device->getStatusResult());

		// We've selected a kinectbasis device, so this should be hidden
		for (auto& expander : jointSelectorExpanders)
			expander.get()->SetVisibility(Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(Visibility::Collapsed);

		// For all override devices
		{
			for (auto& expander : overrideSelectorExpanders)
				expander.get()->SetVisibility(
					(device_status.find(L"S_OK") != std::wstring::npos &&
						selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
						? Visibility::Visible
						: Visibility::Collapsed);

			overridesLabel.get()->Visibility(
				(device_status.find(L"S_OK") != std::wstring::npos &&
					selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
					? Visibility::Visible
					: Visibility::Collapsed);
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
				? *TrackingDevices::TrackingDevicesLayoutRootsVector.at(
					selectedTrackingDeviceID)->Get()
				: *k2app::interfacing::emptyLayoutRoot->Get());
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto& device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

		device->shutdown();
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

		// For all override devices
		{
			for (auto& expander : overrideSelectorExpanders)
				expander.get()->SetVisibility(
					(device_status.find(L"S_OK") != std::wstring::npos &&
						selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
						? Visibility::Visible
						: Visibility::Collapsed);

			overridesLabel.get()->Visibility(
				(device_status.find(L"S_OK") != std::wstring::npos &&
					selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
					? Visibility::Visible
					: Visibility::Collapsed);
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
				? *TrackingDevices::TrackingDevicesLayoutRootsVector.at(
					selectedTrackingDeviceID)->Get()
				: *k2app::interfacing::emptyLayoutRoot->Get());
	}

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

	// Update the GeneralPage status
	TrackingDevices::updateTrackingDeviceUI();
	TrackingDevices::updateOverrideDeviceUI(); // Auto-handles if none

	AlternativeConnectionOptionsFlyout().Hide();
}

// Mark override device as -1 -> deselect it
void Amethyst::implementation::DevicesPage::DeselectDeviceButton_Click(
	const Windows::Foundation::IInspectable& sender,
	const RoutedEventArgs& e)
{
	auto _index = devicesListView.get()->SelectedIndex();

	auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(_index);
	std::wstring device_status = L"Something's wrong!\nE_UKNOWN\nWhat's happened here?";
	LOG(INFO) << "Now deselecting the tracking device...";

	for (auto& expander : jointSelectorExpanders)
		expander.get()->SetVisibility(Visibility::Collapsed);

	jointBasisLabel.get()->Visibility(Visibility::Collapsed);

	setAsOverrideButton.get()->IsEnabled(true);
	setAsBaseButton.get()->IsEnabled(true);

	overridesLabel.get()->Visibility(Visibility::Collapsed);
	deselectDeviceButton.get()->Visibility(Visibility::Collapsed);

	for (auto& expander : overrideSelectorExpanders)
		expander.get()->SetVisibility(Visibility::Collapsed);

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto& device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);
		device_status = device->statusResultWString(device->getStatusResult());

		// Show / Hide device settings button
		selectedDeviceSettingsHostContainer.get()->Visibility(
			device->isSettingsDaemonSupported()
				? Visibility::Visible
				: Visibility::Collapsed);

		// Append device settings / placeholder layout
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
			device->isSettingsDaemonSupported()
				? *TrackingDevices::TrackingDevicesLayoutRootsVector.at(
					selectedTrackingDeviceID)->Get()
				: *k2app::interfacing::emptyLayoutRoot->Get());
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto& device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
		device_status = device->statusResultWString(device->getStatusResult());

		// Show / Hide device settings button
		selectedDeviceSettingsHostContainer.get()->Visibility(
			device->isSettingsDaemonSupported()
				? Visibility::Visible
				: Visibility::Collapsed);

		// Append device settings / placeholder layout
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
			device->isSettingsDaemonSupported()
				? *TrackingDevices::TrackingDevicesLayoutRootsVector.at(
					selectedTrackingDeviceID)->Get()
				: *k2app::interfacing::emptyLayoutRoot->Get());
	}

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

	// Deselect the device
	k2app::K2Settings.overrideDeviceID = -1; // Only acceptable for an Override
	TrackingDevices::updateOverrideDeviceUI();

	// Also update in the UI
	overrideDeviceName.get()->Text(
		k2app::interfacing::LocalizedResourceWString(L"DevicesPage", L"Titles/NoOverrides"));

	// Save settings
	k2app::K2Settings.saveSettings();

	AlternativeConnectionOptionsFlyout().Hide();
}


Windows::Foundation::IAsyncAction Amethyst::implementation::DevicesPage::SetAsOverrideButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	const auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(selectedTrackingDeviceID);

	std::wstring device_status = L"Something's wrong!\nE_UKNOWN\nWhat's happened here?";
	std::string deviceName = "[UNKNOWN]";

	if (trackingDevice.index() == 0)
	{
		k2app::K2Settings.overrideDeviceID = selectedTrackingDeviceID;

		// Kinect Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);
		deviceName = device->getDeviceName();
		device->initialize(); // Init the device as we'll be using it

		// Also refresh joints

		// Clear items
		for (auto& expander : overrideSelectorExpanders)
			expander.get()->ReAppendTrackers();

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
		TrackingDevices::devices_check_override_ids(selectedTrackingDeviceID);

		for (auto& expander : overrideSelectorExpanders)
		{
			// Select the first (or next, if exists) joint
			// Set the placeholder text on disabled combos
			expander.get()->SelectComboItems();

			// Select enabled overrides
			expander.get()->UpdateOverrideToggles();
		}

		// Backup the status
		device_status = device->statusResultWString(device->getStatusResult());

		// We've selected a kinectbasis device, so this should be hidden
		for (auto& expander : jointSelectorExpanders)
			expander.get()->SetVisibility(Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(Visibility::Collapsed);

		// Show / Hide device settings button
		selectedDeviceSettingsHostContainer.get()->Visibility(
			device->isSettingsDaemonSupported()
				? Visibility::Visible
				: Visibility::Collapsed);

		// Append device settings / placeholder layout
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
			device->isSettingsDaemonSupported()
				? *TrackingDevices::TrackingDevicesLayoutRootsVector.at(
					selectedTrackingDeviceID)->Get()
				: *k2app::interfacing::emptyLayoutRoot->Get());
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
		deviceName = device->getDeviceName();
		device->initialize(); // Init the device as we'll be using it

		// Abort if there are no joints
		if (device->getTrackedJoints().empty())
		{
			NoJointsFlyout().ShowAt(SelectedDeviceNameLabel());
			co_return; // Don't set up any overrides (yet)
		}

		k2app::K2Settings.overrideDeviceID = selectedTrackingDeviceID;

		// Also refresh joints

		// Clear items
		for (auto& expander : overrideSelectorExpanders)
			expander.get()->ReAppendTrackers();

		// Append all joints to all combos
		for (auto& _joint : device->getTrackedJoints())
		{
			// Get the name into string
			auto _jointname = _joint.getJointName();

			// Push the name to all combos
			for (auto& expander : overrideSelectorExpanders)
				expander.get()->PushOverrideJoint(StringToWString(_jointname));
		}

		// Try fix override IDs if wrong
		TrackingDevices::devices_check_override_ids(selectedTrackingDeviceID);

		for (auto& expander : overrideSelectorExpanders)
		{
			// Select the first (or next, if exists) joint
			// Set the placeholder text on disabled combos
			expander.get()->SelectComboItems();

			// Select enabled overrides
			expander.get()->UpdateOverrideToggles();
		}

		// Backup the status
		device_status = device->statusResultWString(device->getStatusResult());

		// We've selected an override device, so this should be hidden
		for (auto& expander : jointSelectorExpanders)
			expander.get()->SetVisibility(Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(Visibility::Collapsed);

		// Show / Hide device settings button
		selectedDeviceSettingsHostContainer.get()->Visibility(
			device->isSettingsDaemonSupported()
				? Visibility::Visible
				: Visibility::Collapsed);

		// Append device settings / placeholder layout
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
			device->isSettingsDaemonSupported()
				? *TrackingDevices::TrackingDevicesLayoutRootsVector.at(
					selectedTrackingDeviceID)->Get()
				: *k2app::interfacing::emptyLayoutRoot->Get());
	}

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	/* Update local statuses */
	setAsOverrideButton.get()->IsEnabled(false);
	setAsBaseButton.get()->IsEnabled(true);
	SetDeviceTypeFlyout().Hide(); // Hide the flyout

	overrideDeviceName.get()->Text(StringToWString(deviceName));

	LOG(INFO) << "Changed the current tracking device (Override) to " << deviceName;

	overridesLabel.get()->Visibility(Visibility::Visible);
	deselectDeviceButton.get()->Visibility(Visibility::Visible);

	for (auto& expander : overrideSelectorExpanders)
		expander.get()->SetVisibility(Visibility::Visible);

	// Register and etc
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

	TrackingDevices::updateOverrideDeviceUI();

	{
		// Remove the only one child of our outer main content grid
		// (What a bestiality it is to do that!!1)
		devicesOverridesSelectorStackPanelOuter.get()->Children().RemoveAtEnd();

		Media::Animation::EntranceThemeTransition t;
		t.IsStaggeringEnabled(true);

		devicesOverridesSelectorStackPanelInner.get()->Transitions().Append(t);

		// Sleep peacefully pretending that noting happened
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(10); // 10ms for slower systems
		co_await ui_thread;

		// Re-add the child for it to play our funky transition
		// (Though it's not the same as before...)
		devicesOverridesSelectorStackPanelOuter.get()->Children().
		                                        Append(*devicesOverridesSelectorStackPanelInner);
	}

	// Save settings
	k2app::K2Settings.saveSettings();

	// Remove the transition
	apartment_context ui_thread;
	co_await resume_background();
	Sleep(100);
	co_await ui_thread;

	devicesOverridesSelectorStackPanelInner.get()->Transitions().Clear();
}


Windows::Foundation::IAsyncAction Amethyst::implementation::DevicesPage::SetAsBaseButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	const auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(selectedTrackingDeviceID);

	std::wstring device_status = L"Something's wrong!\nE_UKNOWN\nWhat's happened here?";
	std::string deviceName = "[UNKNOWN]";

	if (trackingDevice.index() == 0)
	{
		k2app::K2Settings.trackingDeviceID = selectedTrackingDeviceID;
		if (k2app::K2Settings.overrideDeviceID == k2app::K2Settings.trackingDeviceID)
			k2app::K2Settings.overrideDeviceID = -1; // Reset the override

		// Kinect Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);
		deviceName = device->getDeviceName();

		device->initialize(); // Init the device as we'll be using it
		device_status = device->statusResultWString(device->getStatusResult());

		// We've selected a kinectbasis device, so this should be hidden
		for (auto& expander : jointSelectorExpanders)
			expander.get()->SetVisibility(Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(Visibility::Collapsed);

		// Show / Hide device settings button
		selectedDeviceSettingsHostContainer.get()->Visibility(
			device->isSettingsDaemonSupported()
				? Visibility::Visible
				: Visibility::Collapsed);

		// Append device settings / placeholder layout
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
			device->isSettingsDaemonSupported()
				? *TrackingDevices::TrackingDevicesLayoutRootsVector.at(
					selectedTrackingDeviceID)->Get()
				: *k2app::interfacing::emptyLayoutRoot->Get());
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
		deviceName = device->getDeviceName();
		device->initialize(); // Init the device as we'll be using it

		// Abort if there are no joints
		if (device->getTrackedJoints().empty())
		{
			NoJointsFlyout().ShowAt(SelectedDeviceNameLabel());
			co_return; // Don't set up any overrides (yet)
		}

		k2app::K2Settings.trackingDeviceID = selectedTrackingDeviceID;
		if (k2app::K2Settings.overrideDeviceID == k2app::K2Settings.trackingDeviceID)
			k2app::K2Settings.overrideDeviceID = -1; // Reset the override

		// Also refresh joints
		for (auto& expander : jointSelectorExpanders)
		{
			expander.get()->ReAppendTrackers(); // Refresh trackers/joints
			expander.get()->SetVisibility(Visibility::Visible); // Set as visible
		}

		// Update the status
		device_status = device->statusResultWString(device->getStatusResult());
		jointBasisLabel.get()->Visibility(
			(device_status.find(L"S_OK") != std::wstring::npos) ? Visibility::Visible : Visibility::Collapsed);

		// Show / Hide device settings button
		selectedDeviceSettingsHostContainer.get()->Visibility(
			device->isSettingsDaemonSupported()
				? Visibility::Visible
				: Visibility::Collapsed);

		// Append device settings / placeholder layout
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
			device->isSettingsDaemonSupported()
				? *TrackingDevices::TrackingDevicesLayoutRootsVector.at(
					selectedTrackingDeviceID)->Get()
				: *k2app::interfacing::emptyLayoutRoot->Get());
	}

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	/* Update local statuses */
	setAsOverrideButton.get()->IsEnabled(false);
	setAsBaseButton.get()->IsEnabled(false);
	SetDeviceTypeFlyout().Hide(); // Hide the flyout

	baseDeviceName.get()->Text(StringToWString(deviceName));
	if (overrideDeviceName.get()->Text() == StringToWString(deviceName))
		overrideDeviceName.get()->Text(
			k2app::interfacing::LocalizedResourceWString(L"DevicesPage", L"Titles/NoOverrides"));

	LOG(INFO) << "Changed the current tracking device (Base) to " << deviceName;

	overridesLabel.get()->Visibility(Visibility::Collapsed);
	deselectDeviceButton.get()->Visibility(Visibility::Collapsed);

	for (auto& expander : overrideSelectorExpanders)
		expander.get()->SetVisibility(Visibility::Collapsed);

	// Register and etc
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

	TrackingDevices::updateTrackingDeviceUI();

	// This is here too cause an override might've became a base... -_-
	TrackingDevices::updateOverrideDeviceUI(); // Auto-handles if none

	// If controls are set to be visible
	if (jointSelectorExpanders[0].get()->ContainerExpander().get()->Visibility() == Visibility::Visible)
	{
		// Remove the only one child of our outer main content grid
		// (What a bestiality it is to do that!!1)
		devicesJointsBasisSelectorStackPanelOuter.get()->Children().RemoveAtEnd();

		Media::Animation::EntranceThemeTransition t;
		t.IsStaggeringEnabled(true);

		devicesJointsBasisSelectorStackPanelInner.get()->Transitions().Append(t);

		// Sleep peacefully pretending that noting happened
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(10); // 10ms for slower systems
		co_await ui_thread;

		// Re-add the child for it to play our funky transition
		// (Though it's not the same as before...)
		devicesJointsBasisSelectorStackPanelOuter.get()->Children().
		                                          Append(*devicesJointsBasisSelectorStackPanelInner);
	}

	// Save settings
	k2app::K2Settings.saveSettings();

	// Remove the transition
	apartment_context ui_thread;
	co_await resume_background();
	Sleep(100);
	co_await ui_thread;

	devicesJointsBasisSelectorStackPanelInner.get()->Transitions().Clear();
}


void Amethyst::implementation::DevicesPage::DismissOverrideTipNoJointsButton_Click(
	const Windows::Foundation::IInspectable& sender,
	const RoutedEventArgs& e)
{
	NoJointsFlyout().Hide();
}


void Amethyst::implementation::DevicesPage::DevicesPage_Loaded(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	LOG(INFO) << "Re/Loading page with tag: \"devices\"...";
	k2app::interfacing::currentAppState = L"devices";

	// Execute the handler
	DevicesPage_Loaded_Handler();

	// Mark as loaded
	devices_loadedOnce = true;
}


void Amethyst::implementation::DevicesPage::DevicesPage_Loaded_Handler()
{
	// Load strings (must be the first thing we're doing)

	Titles_Devices().Text(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Titles/Devices"));

	Titles_CurrentDevice().Text(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Titles/CurrentDevice"));

	Titles_DeviceBase().Text(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Titles/DeviceBase"));

	Titles_DeviceOverride().Text(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Titles/DeviceOverride"));

	OverrideDeviceName().Text(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Titles/NoOverrides"));

	Titles_DeviceStatus().Text(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Titles/DeviceStatus"));

	ReconnectDeviceButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Buttons/Reconnect")));

	DisconnectDeviceButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Buttons/Disconnect")));

	DeselectDeviceButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Buttons/Deselect")));

	Controls::ToolTipService::SetToolTip(
		DeselectDeviceButton(), box_value(
			k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Buttons/Deselect/ToolTip")));

	OpenDocsButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Buttons/ViewDocs")));

	OpenDiscordButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Buttons/JoinDiscord")));

	Titles_SetAsDevice().Text(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Titles/SetAsDevice"));

	SetThisDeviceAsButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Buttons/SetAs/Header")));

	Controls::ToolTipService::SetToolTip(
		SetThisDeviceAsButton(), box_value(
			k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Buttons/SetAs/Header/ToolTip")));

	SetAsBaseButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Buttons/SetAs/Base")));

	Controls::ToolTipService::SetToolTip(
		SetAsBaseButton(), box_value(
			k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Buttons/SetAs/Base/ToolTip")));

	SetAsOverrideButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Buttons/SetAs/Override")));

	Controls::ToolTipService::SetToolTip(
		SetAsOverrideButton(), box_value(
			k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Buttons/SetAs/Override/ToolTip")));

	DismissOverrideTipNoJointsButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Buttons/NoJoints/Dismiss")));

	Titles_DeviceHasNoJoints().Text(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Titles/DeviceHasNoJoints"));

	OverridesLabel().Text(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Titles/Overrides/Header"));

	JointBasisLabel().Text(
		k2app::interfacing::LocalizedJSONString(L"/DevicesPage/Titles/Joints/Assign"));

	DevicesListTeachingTip().Title(
		k2app::interfacing::LocalizedJSONString(L"/NUX/Tip8/Title"));
	DevicesListTeachingTip().Subtitle(
		k2app::interfacing::LocalizedJSONString(L"/NUX/Tip8/Content"));
	DevicesListTeachingTip().CloseButtonContent(
		box_value(k2app::interfacing::LocalizedJSONString(L"/NUX/Next")));
	DevicesListTeachingTip().ActionButtonContent(
		box_value(k2app::interfacing::LocalizedJSONString(L"/NUX/Prev")));

	DeviceStatusTeachingTip().Title(
		k2app::interfacing::LocalizedJSONString(L"/NUX/Tip9/Title"));
	DeviceStatusTeachingTip().Subtitle(
		k2app::interfacing::LocalizedJSONString(L"/NUX/Tip9/Content"));
	DeviceStatusTeachingTip().CloseButtonContent(
		box_value(k2app::interfacing::LocalizedJSONString(L"/NUX/Next")));
	DeviceStatusTeachingTip().ActionButtonContent(
		box_value(k2app::interfacing::LocalizedJSONString(L"/NUX/Prev")));

	DeviceControlsTeachingTip().Title(
		k2app::interfacing::LocalizedJSONString(L"/NUX/Tip10/Title"));
	DeviceControlsTeachingTip().Subtitle(
		k2app::interfacing::LocalizedJSONString(L"/NUX/Tip10/Content"));
	DeviceControlsTeachingTip().CloseButtonContent(
		box_value(k2app::interfacing::LocalizedJSONString(L"/NUX/Next")));
	DeviceControlsTeachingTip().ActionButtonContent(
		box_value(k2app::interfacing::LocalizedJSONString(L"/NUX/Prev")));

	// Reset
	devices_tab_re_setup_finished = false;

	// If it's the first time loading, select the base device
	selectedTrackingDeviceID =
		devices_tab_setup_finished
			? devicesListView.get()->SelectedIndex()
			: k2app::K2Settings.trackingDeviceID;

	// Notify of the setup's end
	devices_tab_setup_finished = true;

	// Run the on-selected routine
	const auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(selectedTrackingDeviceID);

	std::string deviceName = "[UNKNOWN]";
	std::wstring device_status = L"Something's wrong!\nE_UNKNOWN\nWhat's happened here?";

	// Only if override -> select enabled combos
	if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
		for (auto& expander : overrideSelectorExpanders)
			expander.get()->UpdateOverrideToggles();

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

		deviceNameLabel.get()->Text(StringToWString(device->getDeviceName()));
		device_status = device->statusResultWString(device->getStatusResult());

		deviceName = device->getDeviceName();

		// Show / Hide device settings button
		selectedDeviceSettingsHostContainer.get()->Visibility(
			device->isSettingsDaemonSupported()
				? Visibility::Visible
				: Visibility::Collapsed);

		// Append device settings / placeholder layout
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
			device->isSettingsDaemonSupported()
				? *TrackingDevices::TrackingDevicesLayoutRootsVector.at(
					selectedTrackingDeviceID)->Get()
				: *k2app::interfacing::emptyLayoutRoot->Get());

		// We've selected a kinectbasis device, so this should be hidden
		for (auto& expander : jointSelectorExpanders)
			expander.get()->SetVisibility(Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(Visibility::Collapsed);

		// For all override devices
		{
			for (auto& expander : overrideSelectorExpanders)
				expander.get()->SetVisibility(
					(device_status.find(L"S_OK") != std::wstring::npos &&
						selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
						? Visibility::Visible
						: Visibility::Collapsed);

			overridesLabel.get()->Visibility(
				(device_status.find(L"S_OK") != std::wstring::npos &&
					selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
					? Visibility::Visible
					: Visibility::Collapsed);
		}

		// Set up combos if the device's OK
		if (device_status.find(L"S_OK") != std::wstring::npos)
		{
			// If we're reconnecting an override device, also refresh joints
			if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
			{
				// Clear items
				for (auto& expander : overrideSelectorExpanders)
					expander.get()->ReAppendTrackers();

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
				TrackingDevices::devices_check_override_ids(selectedTrackingDeviceID);

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
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

		deviceNameLabel.get()->Text(StringToWString(device->getDeviceName()));
		device_status = device->statusResultWString(device->getStatusResult());

		deviceName = device->getDeviceName();

		// Show / Hide device settings button
		selectedDeviceSettingsHostContainer.get()->Visibility(
			device->isSettingsDaemonSupported()
				? Visibility::Visible
				: Visibility::Collapsed);

		// Append device settings / placeholder layout
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
			device->isSettingsDaemonSupported()
				? *TrackingDevices::TrackingDevicesLayoutRootsVector.at(
					selectedTrackingDeviceID)->Get()
				: *k2app::interfacing::emptyLayoutRoot->Get());

		// We've selected a jointsbasis device, so this should be visible
		//	at least when the device is online

		// For base joints devices
		{
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
		}

		// For all override devices
		{
			for (auto& expander : overrideSelectorExpanders)
				expander.get()->SetVisibility(
					(device_status.find(L"S_OK") != std::wstring::npos &&
						selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
						? Visibility::Visible
						: Visibility::Collapsed);

			overridesLabel.get()->Visibility(
				(device_status.find(L"S_OK") != std::wstring::npos &&
					selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
					? Visibility::Visible
					: Visibility::Collapsed);
		}

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

				// Append all joints to all combos
				for (auto& _joint : device->getTrackedJoints())
				{
					// Get the name into string
					auto _jointname = _joint.getJointName();

					// Push the name to all combos
					for (auto& expander : overrideSelectorExpanders)
						expander.get()->PushOverrideJoint(StringToWString(_jointname));
				}

				// Try fix override IDs if wrong
				TrackingDevices::devices_check_override_ids(selectedTrackingDeviceID);

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
	}

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

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

	if (selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
	{
		LOG(INFO) << "Selected a base";
		setAsOverrideButton.get()->IsEnabled(false);
		setAsBaseButton.get()->IsEnabled(false);

		deselectDeviceButton.get()->Visibility(Visibility::Collapsed);
	}
	else if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
	{
		LOG(INFO) << "Selected an override";
		setAsOverrideButton.get()->IsEnabled(false);
		setAsBaseButton.get()->IsEnabled(true);

		deselectDeviceButton.get()->Visibility(Visibility::Visible);
	}
	else
	{
		LOG(INFO) << "Selected a [none]";
		setAsOverrideButton.get()->IsEnabled(true);
		setAsBaseButton.get()->IsEnabled(true);

		deselectDeviceButton.get()->Visibility(Visibility::Collapsed);
	}

	LOG(INFO) << "Changed the currently selected device to " << deviceName;

	// Now we're good
	devices_tab_re_setup_finished = true;
}


void Amethyst::implementation::DevicesPage::OpenDiscordButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);

	ShellExecuteA(nullptr, nullptr, "https://discord.gg/YBQCRDG", nullptr, nullptr, SW_SHOW);
}


void Amethyst::implementation::DevicesPage::OpenDocsButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);

	const auto _device_error_string = TrackingDeviceErrorLabel().Text();
	const auto _device_name = SelectedDeviceNameLabel().Text();

	if (_device_name == L"Xbox 360 Kinect")
	{
		if (_device_error_string == L"E_NUI_NOTPOWERED")
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/360/troubleshooting/notpowered/",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);

		else if (_device_error_string == L"E_NUI_NOTREADY")
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/360/troubleshooting/notready/",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);

		else if (_device_error_string == L"E_NUI_NOTGENUINE")
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/360/troubleshooting/notgenuine/",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);

		else if (_device_error_string == L"E_NUI_INSUFFICIENTBANDWIDTH")
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/360/troubleshooting/insufficientbandwidth",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);

		else
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/app/help/",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);
	}

	else if (_device_name == L"Xbox One Kinect")
	{
		if (_device_error_string == L"E_NOTAVAILABLE")
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/one/troubleshooting",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);

		else
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/app/help/",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);
	}

	else if (_device_name == L"PSMove Service")
	{
		if (_device_error_string == L"E_PSMS_NOT_RUNNING")
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/psmove/troubleshooting",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);

		else if (_device_error_string == L"E_PSMS_NO_JOINTS")
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/psmove/troubleshooting",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);
		else
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/app/help/",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);
	}

	else
		ShellExecuteW(nullptr, nullptr,
		              std::format(L"https://docs.k2vr.tech/{}/app/help/", k2app::interfacing::docsLanguageCode).c_str(),
		              nullptr, nullptr, SW_SHOW);
}


void Amethyst::implementation::DevicesPage::DevicesListTeachingTip_Closed(
	const Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args)
{
	DeviceStatusTeachingTip().TailVisibility(Controls::TeachingTipTailVisibility::Collapsed);
	DeviceStatusTeachingTip().IsOpen(true);
}


void Amethyst::implementation::DevicesPage::DeviceStatusTeachingTip_Closed(
	const Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args)
{
	DeviceControlsTeachingTip().TailVisibility(Controls::TeachingTipTailVisibility::Collapsed);
	DeviceControlsTeachingTip().IsOpen(true);
}


Windows::Foundation::IAsyncAction Amethyst::implementation::DevicesPage::DeviceControlsTeachingTip_Closed(
	const Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args)
{
	// Wait a bit
	{
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(200);
		co_await ui_thread;
	}

	// Navigate to the settings page
	k2app::shared::main::mainNavigationView->SelectedItem(
		k2app::shared::main::mainNavigationView->MenuItems().GetAt(3));
	k2app::shared::main::NavView_Navigate(L"info", Media::Animation::EntranceNavigationTransitionInfo());

	// Wait a bit
	{
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(500);
		co_await ui_thread;
	}

	// Show the next tip
	k2app::shared::teaching_tips::info::helpTeachingTip->TailVisibility(
		Controls::TeachingTipTailVisibility::Collapsed);
	k2app::shared::teaching_tips::info::helpTeachingTip->IsOpen(true);
}


void Amethyst::implementation::DevicesPage::ButtonFlyout_Opening(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Show);
}


void Amethyst::implementation::DevicesPage::ButtonFlyout_Closing(
	const Controls::Primitives::FlyoutBase& sender,
	const Controls::Primitives::FlyoutBaseClosingEventArgs& args)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Hide);
}


Windows::Foundation::IAsyncAction Amethyst::implementation::DevicesPage::DevicesListTeachingTip_ActionButtonClick(
	const Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args)
{
	// Close the current tip
	DevicesListTeachingTip().IsOpen(false);

	// Wait a bit
	{
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(400);
		co_await ui_thread;
	}

	// Reset the next page layout (if ever changed)
	if (k2app::shared::settings::pageMainScrollViewer)
		k2app::shared::settings::pageMainScrollViewer->ScrollToVerticalOffset(0);

	// Navigate to the settings page
	k2app::shared::main::mainNavigationView->SelectedItem(
		k2app::shared::main::mainNavigationView->MenuItems().GetAt(1));
	k2app::shared::main::NavView_Navigate(L"settings", Media::Animation::EntranceNavigationTransitionInfo());

	// Wait a bit
	{
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(500);
		co_await ui_thread;
	}

	// Show the next tip
	k2app::shared::teaching_tips::settings::autoStartTeachingTip->TailVisibility(
		Controls::TeachingTipTailVisibility::Collapsed);
	k2app::shared::teaching_tips::settings::autoStartTeachingTip->IsOpen(true);
}


void Amethyst::implementation::DevicesPage::DeviceStatusTeachingTip_ActionButtonClick(
	const Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args)
{
	// Close the current tip
	DeviceStatusTeachingTip().IsOpen(false);

	// Show the previous one
	DevicesListTeachingTip().TailVisibility(
		Controls::TeachingTipTailVisibility::Collapsed);
	DevicesListTeachingTip().IsOpen(true);
}


void Amethyst::implementation::DevicesPage::DeviceControlsTeachingTip_ActionButtonClick(
	const Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args)
{
	// Close the current tip
	DeviceControlsTeachingTip().IsOpen(false);

	// Show the previous one
	DeviceStatusTeachingTip().TailVisibility(
		Controls::TeachingTipTailVisibility::Collapsed);
	DeviceStatusTeachingTip().IsOpen(true);
}
