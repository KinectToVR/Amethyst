#pragma once
#include "DevicesPage.g.h"
#include "TrackingDevicesView.h"

#include "TrackingDevices.h"
#include "K2Shared.h"

#include "JointSelectorRow.h"
#include "JointSelectorExpander.h"

#include "OverrideSelectorRow.h"
#include "OverrideSelectorExpander.h"

namespace winrt::KinectToVR::implementation
{
	struct DevicesPage : DevicesPageT<DevicesPage>
	{
		DevicesPage();

		Windows::Foundation::IAsyncAction TrackingDeviceListView_SelectionChanged(
			const Windows::Foundation::IInspectable& sender,
			const winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
			& e);
		Windows::Foundation::IAsyncAction SetAsOverrideButton_Click(const Windows::Foundation::IInspectable& sender,
		                                                            const winrt::Microsoft::UI::Xaml::RoutedEventArgs&
		                                                            e);
		Windows::Foundation::IAsyncAction SetAsBaseButton_Click(const Windows::Foundation::IInspectable& sender,
		                                                        const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DisconnectDeviceButton_Click(const Windows::Foundation::IInspectable& sender,
		                                  const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DeselectDeviceButton_Click(const Windows::Foundation::IInspectable& sender,
		                                const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ReconnectDeviceButton_Click(const winrt::Microsoft::UI::Xaml::Controls::SplitButton& sender,
		                                 const winrt::Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs& args);
		
		void DismissOverrideTipNoJointsButton_Click(const Windows::Foundation::IInspectable& sender,
		                                            const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DevicesPage_Loaded(const Windows::Foundation::IInspectable& sender,
		                        const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OpenDiscordButton_Click(const Windows::Foundation::IInspectable& sender,
		                             const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OpenDocsButton_Click(const Windows::Foundation::IInspectable& sender,
		                          const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void SelectedDeviceSettingsButton_Click(const Windows::Foundation::IInspectable& sender,
		                                        const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
	};
}

namespace winrt::KinectToVR::factory_implementation
{
	struct DevicesPage : DevicesPageT<DevicesPage, implementation::DevicesPage>
	{
	};
}
