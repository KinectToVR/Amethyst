#pragma once
#include "DevicesPage.g.h"
#include "TrackingDevicesView.h"

#include "TrackingDevices.h"
#include "K2Shared.h"

namespace winrt::KinectToVR::implementation
{
	struct DevicesPage : DevicesPageT<DevicesPage>
	{
		DevicesPage();

		void TrackingDeviceListView_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender,
		                                             winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                             const& e);
		void SetAsOverrideButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void SetAsBaseButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void DisconnectDeviceButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void ReconnectDeviceButton_Click(winrt::Microsoft::UI::Xaml::Controls::SplitButton const& sender, winrt::Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs const& args);
	};
}

namespace winrt::KinectToVR::factory_implementation
{
	struct DevicesPage : DevicesPageT<DevicesPage, implementation::DevicesPage>
	{
	};
}
