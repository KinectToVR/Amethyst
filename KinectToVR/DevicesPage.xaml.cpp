#include "pch.h"
#include "DevicesPage.xaml.h"
#if __has_include("DevicesPage.g.cpp")
#include "DevicesPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

std::shared_ptr<Controls::TextBlock>
	deviceNameLabel,
	deviceStatusLabel,
	errorWhatText,
	trackingDeviceErrorLabel;

std::shared_ptr<Controls::Grid>
	errorButtonsGrid,
	errorWhatGrid;

std::shared_ptr<Controls::ListView> devicesListView;

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

		errorButtonsGrid = std::make_shared<Controls::Grid>(ErrorButtonsGrid());
		errorWhatGrid = std::make_shared<Controls::Grid>(ErrorWhatGrid());

		devicesListView = std::make_shared<Controls::ListView>(TrackingDeviceListView());
		
		// Create tracking devices' list
		Windows::Foundation::Collections::IObservableVector<KinectToVR::TrackingDevicesView> m_TrackingDevicesViewModels
		{
			single_threaded_observable_vector<KinectToVR::TrackingDevicesView>()
		};

		// Watch for insertions
		m_TrackingDevicesViewModels.VectorChanged(
			[&](Windows::Foundation::Collections::IObservableVector<KinectToVR::TrackingDevicesView> const& sender,
			    Windows::Foundation::Collections::IVectorChangedEventArgs const& args)
			{
				// Report a registration and parse
				OutputDebugString(sender.GetAt(sender.Size() - 1).DeviceName().c_str());
				OutputDebugString(L"'s been registered as a tracking device. [UI Node]\n");

				// Set the current device
				if (sender.Size() >= 2)
					sender.GetAt(1).Current(true);

				// Re-set all indexes
				for (uint32_t i = 0; i < sender.Size(); i++)
					sender.GetAt(i).DeviceID(i);
			});

		// Add tracking devices here
		std::string device_status = "E_UKNOWN\nWhat's happened here?";

		for (auto const& trackingDevice : TrackingDevices::TrackingDevicesVector)
		{
			std::string deviceName = "[UNKNOWN]";

			if (trackingDevice.index() == 0)
			{
				// Kinect Basis
				const auto device = std::get<ktvr::TrackingDeviceBase_KinectBasis*>(trackingDevice);

				deviceNameLabel.get()->Text(wstring_cast(device->getDeviceName()));
				device_status = device->statusResultString(device->getStatusResult());

				deviceName = device->getDeviceName();
			}
			else if (trackingDevice.index() == 1)
			{
				// Joints Basis
				const auto device = std::get<ktvr::TrackingDeviceBase_JointsBasis*>(trackingDevice);

				deviceNameLabel.get()->Text(wstring_cast(device->getDeviceName()));
				device_status = device->statusResultString(device->getStatusResult());

				deviceName = device->getDeviceName();
			}
			else if (trackingDevice.index() == 2)
			{
				// Override
				const auto device = std::get<ktvr::TrackingDeviceBase_OnlyOverride*>(trackingDevice);

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
			errorWhatText.get()->Visibility(
				status_ok ? Visibility::Collapsed : Visibility::Visible);


			// Split status and message by \n
			deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
			trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
			errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));

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
	}
}


void winrt::KinectToVR::implementation::DevicesPage::TrackingDeviceListView_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	auto const& trackingDevice = TrackingDevices::TrackingDevicesVector.at(
		sender.try_as<Controls::ListView>().SelectedIndex());

	std::string deviceName = "[UNKNOWN]";
	std::string device_status = "E_UKNOWN\nWhat's happened here?";

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto device = std::get<ktvr::TrackingDeviceBase_KinectBasis*>(trackingDevice);

		deviceNameLabel.get()->Text(wstring_cast(device->getDeviceName()));
		device_status = device->statusResultString(device->getStatusResult());

		deviceName = device->getDeviceName();
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto device = std::get<ktvr::TrackingDeviceBase_JointsBasis*>(trackingDevice);

		deviceNameLabel.get()->Text(wstring_cast(device->getDeviceName()));
		device_status = device->statusResultString(device->getStatusResult());

		deviceName = device->getDeviceName();
	}
	else if (trackingDevice.index() == 2)
	{
		// Override
		const auto device = std::get<ktvr::TrackingDeviceBase_OnlyOverride*>(trackingDevice);

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
	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
	trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
	errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));

	OutputDebugString(L"Changed the currently selected device to ");
	OutputDebugString(wstring_cast(deviceName).c_str());
	OutputDebugString(L"\n");

	// Ask for a device change
}


void winrt::KinectToVR::implementation::DevicesPage::ReconnectDeviceButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(devicesListView->SelectedIndex());
	std::string device_status = "E_UKNOWN\nWhat's happened here?";
	OutputDebugString(L"Now reconnecting the tracking device...\n");

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		auto const& device = std::get<ktvr::TrackingDeviceBase_KinectBasis*>(trackingDevice);

		device->shutdown();
		device->initialize();
		device_status = device->statusResultString(device->getStatusResult());
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		auto const& device = std::get<ktvr::TrackingDeviceBase_JointsBasis*>(trackingDevice);

		device->shutdown();
		device->initialize();
		device_status = device->statusResultString(device->getStatusResult());
	}
	else if (trackingDevice.index() == 2)
	{
		// Override
		auto const& device = std::get<ktvr::TrackingDeviceBase_OnlyOverride*>(trackingDevice);

		device->shutdown();
		device->initialize();
		device_status = device->statusResultString(device->getStatusResult());
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
	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
	trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
	errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));
}
