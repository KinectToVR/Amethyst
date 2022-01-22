#include "pch.h"
#include "DevicesPage.xaml.h"
#if __has_include("DevicesPage.g.cpp")
#include "DevicesPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace ::k2app::shared::devices;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.
bool devices_tab_setup_finished = false;

void devices_update_current()
{
	{
		auto const& trackingDevice = TrackingDevices::TrackingDevicesVector.at(k2app::K2Settings.trackingDeviceID);

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
		baseDeviceName.get()->Text(wstring_cast(deviceName));
		if (overrideDeviceName.get()->Text() == wstring_cast(deviceName))
			overrideDeviceName.get()->Text(L"No Overrides");
	}
	{
		if (k2app::K2Settings.overrideDeviceID < 0)return;
		auto const& trackingDevice = TrackingDevices::TrackingDevicesVector.at(k2app::K2Settings.overrideDeviceID);

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
		overrideDeviceName.get()->Text(wstring_cast(deviceName));
	}
}

namespace winrt::KinectToVR::implementation
{
	DevicesPage::DevicesPage()
	{
		InitializeComponent();

		// Cache needed UI elements
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
		overridesControls = std::make_shared<Controls::Grid>(OverridesControls());
		jointBasisControls = std::make_shared<Controls::Grid>(JointBasisControls());

		devicesListView = std::make_shared<Controls::ListView>(TrackingDeviceListView());

		setAsOverrideButton = std::make_shared<Controls::Button>(SetAsOverrideButton());
		setAsBaseButton = std::make_shared<Controls::Button>(SetAsBaseButton());

		waistJointOptionBox = std::make_shared<Controls::ComboBox>(WaistJointOptionBox());
		leftFootJointOptionBox = std::make_shared<Controls::ComboBox>(LeftFootJointOptionBox());
		rightFootJointOptionBox = std::make_shared<Controls::ComboBox>(RightFootJointOptionBox());
		rightFootPositionOverrideOptionBox = std::make_shared<Controls::ComboBox>(RightFootPositionOverrideOptionBox());
		rightFootRotationOverrideOptionBox = std::make_shared<Controls::ComboBox>(RightFootRotationOverrideOptionBox());
		leftFootRotationOverrideOptionBox = std::make_shared<Controls::ComboBox>(LeftFootRotationOverrideOptionBox());
		leftFootPositionOverrideOptionBox = std::make_shared<Controls::ComboBox>(LeftFootPositionOverrideOptionBox());
		waistRotationOverrideOptionBox = std::make_shared<Controls::ComboBox>(WaistRotationOverrideOptionBox());
		waistPositionOverrideOptionBox = std::make_shared<Controls::ComboBox>(WaistPositionOverrideOptionBox());

		overrideWaistPosition = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideWaistPosition());
		overrideWaistRotation = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideWaistRotation());
		overrideLeftFootPosition = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideLeftFootPosition());
		overrideLeftFootRotation = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideLeftFootRotation());
		overrideRightFootPosition = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideRightFootPosition());
		overrideRightFootRotation = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideRightFootRotation());

		// Create tracking devices' list
		static auto m_TrackingDevicesViewModels =
			multi_threaded_observable_vector<KinectToVR::TrackingDevicesView>();

		// Watch for insertions
		m_TrackingDevicesViewModels.VectorChanged(
			[&](Windows::Foundation::Collections::IObservableVector<KinectToVR::TrackingDevicesView> const& sender,
			    Windows::Foundation::Collections::IVectorChangedEventArgs const& args)
			{
				// Report a registration and parse
				LOG(INFO) << string_cast(sender.GetAt(sender.Size() - 1).DeviceName().c_str()) <<
					"'s been registered as a tracking device. [UI Node]";

				// Set the current device
				if (sender.Size() > k2app::K2Settings.trackingDeviceID)
					sender.GetAt(k2app::K2Settings.trackingDeviceID).Current(true);

				// Re-set all indexes
				for (uint32_t i = 0; i < sender.Size(); i++)
					sender.GetAt(i).DeviceID(i);
			});

		// Add tracking devices here
		for (auto const& trackingDevice : TrackingDevices::TrackingDevicesVector)
		{
			std::string deviceName = "[UNKNOWN]";

			switch (trackingDevice.index())
			{
			case 0:
				deviceName = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>
					(trackingDevice)->getDeviceName();
				break;
			case 1:
				deviceName = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>
					(trackingDevice)->getDeviceName();
				break;
			}

			LOG(INFO) << "Appending " << deviceName <<
				" to UI Node's tracking devices' list...";
			m_TrackingDevicesViewModels.Append(
				make<TrackingDevicesView>(wstring_cast(deviceName).c_str()));
		}

		// Set currently tracking device & selected device
		// RadioButton is set on ItemChanged
		TrackingDeviceListView().SelectedIndex(k2app::K2Settings.trackingDeviceID);

		// Register tracking devices' list
		TrackingDeviceListView().ItemsSource(m_TrackingDevicesViewModels);
		NavigationCacheMode(Navigation::NavigationCacheMode::Required);

		std::thread([&, this]() -> Windows::Foundation::IAsyncAction
		{
			/* Update the device in devices tab */

			if (devicesListView.get() != nullptr &&
				m_TrackingDevicesViewModels.Size() > k2app::K2Settings.trackingDeviceID)
			{
				while (true)
				{
					// wait for a signal from the main proc
					// by attempting to decrement the semaphore
					smphSignalCurrentUpdate.acquire();

					// Only when ready
					if (devices_tab_setup_finished)
					{
						DispatcherQueue().TryEnqueue(
							Microsoft::UI::Dispatching::DispatcherQueuePriority::High, [&, this]
							{
								LOG(INFO) << "ID IS " << k2app::K2Settings.trackingDeviceID;
							});
					}
				}
			}

			return Windows::Foundation::IAsyncAction(); // UWU
		}).detach();

		devices_update_current();

		// Notify of the setup's end
		devices_tab_setup_finished = true;
	}
}


void winrt::KinectToVR::implementation::DevicesPage::TrackingDeviceListView_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	selectedTrackingDeviceID = sender.as<Controls::ListView>().SelectedIndex();
	auto const& trackingDevice = TrackingDevices::TrackingDevicesVector.at(selectedTrackingDeviceID);

	std::string deviceName = "[UNKNOWN]";
	std::string device_status = "E_UKNOWN\nWhat's happened here?";

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

		deviceNameLabel.get()->Text(wstring_cast(device->getDeviceName()));
		device_status = device->statusResultString(device->getStatusResult());

		deviceName = device->getDeviceName();

		// We've selected a kinectbasis device, so this should be hidden
		jointBasisControls.get()->Visibility(Visibility::Collapsed);
		jointBasisLabel.get()->Visibility(Visibility::Collapsed);
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

		deviceNameLabel.get()->Text(wstring_cast(device->getDeviceName()));
		device_status = device->statusResultString(device->getStatusResult());

		deviceName = device->getDeviceName();

		// We've selected a jointsbasis device, so this should be visible
		//	at least when the device is online
		jointBasisControls.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);
	}

	// Update the status here
	const bool status_ok = device_status.find("S_OK") != std::string::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
	trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
	errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));

	if (selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
	{
		LOG(INFO) << "Selected a base";
		setAsOverrideButton.get()->IsEnabled(false);
		setAsBaseButton.get()->IsEnabled(false);

		overridesLabel.get()->Visibility(Visibility::Collapsed);
		overridesControls.get()->Visibility(Visibility::Collapsed);
	}
	else if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
	{
		LOG(INFO) << "Selected an override";
		setAsOverrideButton.get()->IsEnabled(false);
		setAsBaseButton.get()->IsEnabled(true);

		overridesLabel.get()->Visibility(Visibility::Visible);
		overridesControls.get()->Visibility(Visibility::Visible);
	}
	else
	{
		LOG(INFO) << "Selected a [none]";
		setAsOverrideButton.get()->IsEnabled(true);
		setAsBaseButton.get()->IsEnabled(true);

		overridesLabel.get()->Visibility(Visibility::Collapsed);
		overridesControls.get()->Visibility(Visibility::Collapsed);
	}

	LOG(INFO) << "Changed the currently selected device to " << deviceName;
}


void winrt::KinectToVR::implementation::DevicesPage::ReconnectDeviceButton_Click(
	winrt::Microsoft::UI::Xaml::Controls::SplitButton const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs const& args)
{
	auto _index = devicesListView->SelectedIndex();

	auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(_index);
	std::string device_status = "E_UKNOWN\nWhat's happened here?";
	LOG(INFO) << "Now reconnecting the tracking device...";

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		auto const& device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

		device->initialize();
		device_status = device->statusResultString(device->getStatusResult());

		// We've selected a kinectbasis device, so this should be hidden
		jointBasisControls.get()->Visibility(Visibility::Collapsed);
		jointBasisLabel.get()->Visibility(Visibility::Collapsed);
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		auto const& device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

		device->initialize();
		device_status = device->statusResultString(device->getStatusResult());

		// We've selected a jointsbasis device, so this should be visible
		//	at least when the device is online
		jointBasisControls.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);
	}

	/* Update local statuses */

	// Update the status here
	const bool status_ok = device_status.find("S_OK") != std::string::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
	trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
	errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));

	// Update the GeneralPage status
	TrackingDevices::updateTrackingDeviceUI(k2app::K2Settings.trackingDeviceID);
}

// *Nearly* the same as reconnect
void winrt::KinectToVR::implementation::DevicesPage::DisconnectDeviceButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	auto _index = devicesListView->SelectedIndex();

	auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(_index);
	std::string device_status = "E_UKNOWN\nWhat's happened here?";
	LOG(INFO) << "Now reconnecting the tracking device...";

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		auto const& device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

		device->shutdown();
		device_status = device->statusResultString(device->getStatusResult());

		// We've selected a kinectbasis device, so this should be hidden
		jointBasisControls.get()->Visibility(Visibility::Collapsed);
		jointBasisLabel.get()->Visibility(Visibility::Collapsed);
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		auto const& device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

		device->shutdown();
		device_status = device->statusResultString(device->getStatusResult());

		// We've selected a jointsbasis device, so this should be visible
		//	at least when the device is online
		jointBasisControls.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);
	}

	/* Update local statuses */

	// Update the status here
	const bool status_ok = device_status.find("S_OK") != std::string::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
	trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
	errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));

	// Update the GeneralPage status
	TrackingDevices::updateTrackingDeviceUI(k2app::K2Settings.trackingDeviceID);
}


void winrt::KinectToVR::implementation::DevicesPage::SetAsOverrideButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	setAsOverrideButton.get()->IsEnabled(false);
	setAsBaseButton.get()->IsEnabled(true);

	auto const& trackingDevice = TrackingDevices::TrackingDevicesVector.at(selectedTrackingDeviceID);

	std::string device_status = "E_UKNOWN\nWhat's happened here?";
	std::string deviceName = "[UNKNOWN]";

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);
		deviceName = device->getDeviceName();

		device->initialize(); // Init the device as we'll be using it
		device_status = device->statusResultString(device->getStatusResult());

		// We've selected a kinectbasis device, so this should be hidden
		jointBasisControls.get()->Visibility(Visibility::Collapsed);
		jointBasisLabel.get()->Visibility(Visibility::Collapsed);
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
		deviceName = device->getDeviceName();

		device->initialize(); // Init the device as we'll be using it
		device_status = device->statusResultString(device->getStatusResult());

		// We've selected an override device, so this should be hidden
		jointBasisControls.get()->Visibility(Visibility::Collapsed);
		jointBasisLabel.get()->Visibility(Visibility::Collapsed);
	}

	/* Update local statuses */
	overrideDeviceName.get()->Text(wstring_cast(deviceName));

	LOG(INFO) << "Changed the current tracking device (Override) to " << deviceName;

	overridesLabel.get()->Visibility(Visibility::Visible);
	overridesControls.get()->Visibility(Visibility::Visible);

	// Register and etc
	/* Update local statuses */

	// Update the status here
	const bool status_ok = device_status.find("S_OK") != std::string::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
	trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
	errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));

	k2app::K2Settings.overrideDeviceID = selectedTrackingDeviceID;
	//TrackingDevices::updateOverrideDeviceUI(k2app::K2Settings.overrideDeviceID); // Not yet TODO
}


void winrt::KinectToVR::implementation::DevicesPage::SetAsBaseButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	setAsOverrideButton.get()->IsEnabled(false);
	setAsBaseButton.get()->IsEnabled(false);

	auto const& trackingDevice = TrackingDevices::TrackingDevicesVector.at(selectedTrackingDeviceID);

	std::string device_status = "E_UKNOWN\nWhat's happened here?";
	std::string deviceName = "[UNKNOWN]";

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);
		deviceName = device->getDeviceName();

		device->initialize(); // Init the device as we'll be using it
		device_status = device->statusResultString(device->getStatusResult());

		// We've selected a kinectbasis device, so this should be hidden
		jointBasisControls.get()->Visibility(Visibility::Collapsed);
		jointBasisLabel.get()->Visibility(Visibility::Collapsed);
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
		deviceName = device->getDeviceName();

		device->initialize(); // Init the device as we'll be using it
		device_status = device->statusResultString(device->getStatusResult());

		// We've selected a jointsbasis device, so this should be visible
		//	at least when the device is online
		jointBasisControls.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos) ? Visibility::Visible : Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos) ? Visibility::Visible : Visibility::Collapsed);
	}

	/* Update local statuses */
	baseDeviceName.get()->Text(wstring_cast(deviceName));
	if (overrideDeviceName.get()->Text() == wstring_cast(deviceName))
		overrideDeviceName.get()->Text(L"No Overrides");

	LOG(INFO) << "Changed the current tracking device (Base) to " << deviceName;

	overridesLabel.get()->Visibility(Visibility::Collapsed);
	overridesControls.get()->Visibility(Visibility::Collapsed);

	// Register and etc
	/* Update local statuses */

	// Update the status here
	const bool status_ok = device_status.find("S_OK") != std::string::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
	trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
	errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));

	k2app::K2Settings.trackingDeviceID = selectedTrackingDeviceID;
	if (k2app::K2Settings.overrideDeviceID == k2app::K2Settings.trackingDeviceID)
		k2app::K2Settings.overrideDeviceID = -1; // Reset the override

	TrackingDevices::updateTrackingDeviceUI(k2app::K2Settings.trackingDeviceID);
}

/* For JointBasis device type: joints selector */

void winrt::KinectToVR::implementation::DevicesPage::WaistJointOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (waistJointOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.selectedTrackedJointID[0] = waistJointOptionBox.get()->SelectedIndex();
}


void winrt::KinectToVR::implementation::DevicesPage::LeftFootJointOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (leftFootJointOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.selectedTrackedJointID[1] = leftFootJointOptionBox.get()->SelectedIndex();
}

void winrt::KinectToVR::implementation::DevicesPage::RightFootJointOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (rightFootJointOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.selectedTrackedJointID[1] = rightFootJointOptionBox.get()->SelectedIndex();
}

/* For *Override* device type: position & rotation joints selector */

void winrt::KinectToVR::implementation::DevicesPage::WaistPositionOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isPositionOverriddenJoint[0] &&
		waistPositionOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.positionOverrideJointID[0] = waistPositionOverrideOptionBox.get()->SelectedIndex();
}


void winrt::KinectToVR::implementation::DevicesPage::WaistRotationOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isRotationOverriddenJoint[0] &&
		waistRotationOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.rotationOverrideJointID[0] = waistRotationOverrideOptionBox.get()->SelectedIndex();
}


void winrt::KinectToVR::implementation::DevicesPage::LeftFootPositionOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isPositionOverriddenJoint[1] &&
		leftFootPositionOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.positionOverrideJointID[1] = leftFootPositionOverrideOptionBox.get()->SelectedIndex();
}


void winrt::KinectToVR::implementation::DevicesPage::LeftFootRotationOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isRotationOverriddenJoint[1] &&
		leftFootRotationOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.rotationOverrideJointID[1] = leftFootRotationOverrideOptionBox.get()->SelectedIndex();
}


void winrt::KinectToVR::implementation::DevicesPage::RightFootPositionOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isPositionOverriddenJoint[2] &&
		rightFootPositionOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.positionOverrideJointID[2] = rightFootPositionOverrideOptionBox.get()->SelectedIndex();
}


void winrt::KinectToVR::implementation::DevicesPage::RightFootRotationOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isRotationOverriddenJoint[2] &&
		rightFootRotationOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.rotationOverrideJointID[2] = rightFootRotationOverrideOptionBox.get()->SelectedIndex();
}

/* For *Override* device type: override elements for joints selector */

void winrt::KinectToVR::implementation::DevicesPage::OverrideWaistPosition_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.isPositionOverriddenJoint[0] = overrideWaistPosition.get()->IsChecked();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideWaistRotation_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.isRotationOverriddenJoint[0] = overrideWaistRotation.get()->IsChecked();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideLeftFootPosition_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.isPositionOverriddenJoint[1] = overrideLeftFootPosition.get()->IsChecked();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideLeftFootRotation_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.isRotationOverriddenJoint[1] = overrideLeftFootRotation.get()->IsChecked();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideRightFootPosition_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.isPositionOverriddenJoint[2] = overrideRightFootPosition.get()->IsChecked();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideRightFootRotation_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.isRotationOverriddenJoint[2] = overrideRightFootRotation.get()->IsChecked();
}
