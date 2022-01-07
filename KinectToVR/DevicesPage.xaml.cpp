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
		auto const& trackingDevice = TrackingDevices::TrackingDevicesVector.at(k2app::interfacing::trackingDeviceID);

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
		if (k2app::interfacing::overrideDeviceID < 0)return;
		auto const& trackingDevice = TrackingDevices::TrackingDevicesVector.at(k2app::interfacing::overrideDeviceID);

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

		errorButtonsGrid = std::make_shared<Controls::Grid>(ErrorButtonsGrid());
		errorWhatGrid = std::make_shared<Controls::Grid>(ErrorWhatGrid());

		devicesListView = std::make_shared<Controls::ListView>(TrackingDeviceListView());

		trackingDeviceChangePanel = std::make_shared<Controls::StackPanel>(TrackingDeviceChangePanel());

		setAsOverrideButton = std::make_shared<Controls::Button>(SetAsOverrideButton());
		setAsBaseButton = std::make_shared<Controls::Button>(SetAsBaseButton());

		// Create tracking devices' list
		static auto m_TrackingDevicesViewModels =
			multi_threaded_observable_vector<KinectToVR::TrackingDevicesView>();

		// Watch for insertions
		m_TrackingDevicesViewModels.VectorChanged(
			[&](Windows::Foundation::Collections::IObservableVector<KinectToVR::TrackingDevicesView> const& sender,
			    Windows::Foundation::Collections::IVectorChangedEventArgs const& args)
			{
				// Report a registration and parse
				OutputDebugString(sender.GetAt(sender.Size() - 1).DeviceName().c_str());
				OutputDebugString(L"'s been registered as a tracking device. [UI Node]\n");

				// Set the current device
				if (sender.Size() > k2app::interfacing::trackingDeviceID)
					sender.GetAt(k2app::interfacing::trackingDeviceID).Current(true);

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

			OutputDebugString(L"Appending ");
			OutputDebugString(wstring_cast(deviceName).c_str());
			OutputDebugString(L" to UI Node's tracking devices' list...\n");
			m_TrackingDevicesViewModels.Append(
				make<TrackingDevicesView>(wstring_cast(deviceName).c_str()));
		}

		// Set currently tracking device & selected device
		// RadioButton is set on ItemChanged
		TrackingDeviceListView().SelectedIndex(0);

		// Register tracking devices' list
		TrackingDeviceListView().ItemsSource(m_TrackingDevicesViewModels);
		NavigationCacheMode(Navigation::NavigationCacheMode::Required);

		std::thread([&, this]() -> Windows::Foundation::IAsyncAction
		{
			/* Update the device in devices tab */

			if (devicesListView.get() != nullptr &&
				m_TrackingDevicesViewModels.Size() > k2app::interfacing::trackingDeviceID)
			{
				while (true)
				{
					// wait for a signal from the main proc
					// by attempting to decrement the semaphore
					smphSignalCurrentUpdate.acquire();

					// Only when ready
					if (devices_tab_setup_finished)
					{
						DispatcherQueue().TryEnqueue(Microsoft::UI::Dispatching::DispatcherQueuePriority::High,
						                             [&, this]
						                             {
							                             OutputDebugString(L"ID IS ");
							                             OutputDebugString(
								                             std::to_wstring(k2app::interfacing::trackingDeviceID).
								                             c_str());
							                             OutputDebugString(L"\n");
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
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

		deviceNameLabel.get()->Text(wstring_cast(device->getDeviceName()));
		device_status = device->statusResultString(device->getStatusResult());

		deviceName = device->getDeviceName();
	}

	// Update the status here
	const bool status_ok = device_status.find("S_OK") != std::string::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	errorWhatGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	errorButtonsGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
	trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
	errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));

	if (selectedTrackingDeviceID == k2app::interfacing::trackingDeviceID)
	{
		OutputDebugString(L"Selected a base\n");
		setAsOverrideButton.get()->IsEnabled(false);
		setAsBaseButton.get()->IsEnabled(false);
	}
	else if (selectedTrackingDeviceID == k2app::interfacing::overrideDeviceID)
	{
		OutputDebugString(L"Selected an override\n");
		setAsOverrideButton.get()->IsEnabled(false);
		setAsBaseButton.get()->IsEnabled(true);
	}
	else
	{
		OutputDebugString(L"Selected a [none]\n");
		setAsOverrideButton.get()->IsEnabled(true);
		setAsBaseButton.get()->IsEnabled(true);
	}

	OutputDebugString(L"Changed the currently selected device to ");
	OutputDebugString(wstring_cast(deviceName).c_str());
	OutputDebugString(L"\n");

	// Ask for a device change
}


void winrt::KinectToVR::implementation::DevicesPage::ReconnectDeviceButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	auto _index = devicesListView->SelectedIndex();

	auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(_index);
	std::string device_status = "E_UKNOWN\nWhat's happened here?";
	OutputDebugString(L"Now reconnecting the tracking device...\n");

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		auto const& device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

		device->initialize();
		device_status = device->statusResultString(device->getStatusResult());
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		auto const& device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

		device->initialize();
		device_status = device->statusResultString(device->getStatusResult());
	}

	/* Update local statuses */

	// Update the status here
	const bool status_ok = device_status.find("S_OK") != std::string::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	errorWhatGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	errorButtonsGrid.get()->Visibility(
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
	TrackingDevices::updateTrackingDeviceUI(k2app::interfacing::trackingDeviceID);
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
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
		deviceName = device->getDeviceName();

		device->initialize(); // Init the device as we'll be using it
		device_status = device->statusResultString(device->getStatusResult());
	}

	/* Update local statuses */
	overrideDeviceName.get()->Text(wstring_cast(deviceName));

	OutputDebugString(L"Changed the current tracking device (Override) to ");
	OutputDebugString(wstring_cast(deviceName).c_str());
	OutputDebugString(L"\n");

	// Register and etc
	/* Update local statuses */

	// Update the status here
	const bool status_ok = device_status.find("S_OK") != std::string::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	errorWhatGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	errorButtonsGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
	trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
	errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));
	
	k2app::interfacing::overrideDeviceID = selectedTrackingDeviceID;
	//TrackingDevices::updateOverrideDeviceUI(k2app::interfacing::overrideDeviceID); // Not yet
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
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
		deviceName = device->getDeviceName();

		device->initialize(); // Init the device as we'll be using it
		device_status = device->statusResultString(device->getStatusResult());
	}

	/* Update local statuses */
	baseDeviceName.get()->Text(wstring_cast(deviceName));
	if (overrideDeviceName.get()->Text() == wstring_cast(deviceName))
		overrideDeviceName.get()->Text(L"No Overrides");

	OutputDebugString(L"Changed the current tracking device (Base) to ");
	OutputDebugString(wstring_cast(deviceName).c_str());
	OutputDebugString(L"\n");

	// Register and etc
	/* Update local statuses */

	// Update the status here
	const bool status_ok = device_status.find("S_OK") != std::string::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	errorWhatGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	errorButtonsGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
	trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
	errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));
	
	k2app::interfacing::trackingDeviceID = selectedTrackingDeviceID;
	if (k2app::interfacing::overrideDeviceID == k2app::interfacing::trackingDeviceID)
		k2app::interfacing::overrideDeviceID = -1; // Reset the override

	TrackingDevices::updateTrackingDeviceUI(k2app::interfacing::trackingDeviceID);
}
