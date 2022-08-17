#pragma once
#include "DevicesPage.g.h"
#include "TrackingDevicesView.h"

#include "TrackingDevices.h"
#include "K2Shared.h"

#include "JointSelectorRow.h"
#include "JointSelectorExpander.h"

#include "OverrideSelectorRow.h"
#include "OverrideSelectorExpander.h"

namespace winrt::Amethyst::implementation
{
	struct DevicesPage : DevicesPageT<DevicesPage>
	{
		DevicesPage();

		Windows::Foundation::IAsyncAction TrackingDeviceListView_SelectionChanged(
			const Windows::Foundation::IInspectable& sender,
			const Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
			& e);
		Windows::Foundation::IAsyncAction SetAsOverrideButton_Click(const Windows::Foundation::IInspectable& sender,
		                                                            const Microsoft::UI::Xaml::RoutedEventArgs&
		                                                            e);
		Windows::Foundation::IAsyncAction SetAsBaseButton_Click(const Windows::Foundation::IInspectable& sender,
		                                                        const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DisconnectDeviceButton_Click(const Windows::Foundation::IInspectable& sender,
		                                  const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DeselectDeviceButton_Click(const Windows::Foundation::IInspectable& sender,
		                                const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ReconnectDeviceButton_Click(const Microsoft::UI::Xaml::Controls::SplitButton& sender,
		                                 const Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs& args);

		void DismissOverrideTipNoJointsButton_Click(const Windows::Foundation::IInspectable& sender,
		                                            const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DevicesPage_Loaded(const Windows::Foundation::IInspectable& sender,
		                        const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OpenDiscordButton_Click(const Windows::Foundation::IInspectable& sender,
		                             const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OpenDocsButton_Click(const Windows::Foundation::IInspectable& sender,
		                          const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DevicesListTeachingTip_Closed(winrt::Microsoft::UI::Xaml::Controls::TeachingTip const& sender, winrt::Microsoft::UI::Xaml::Controls::TeachingTipClosedEventArgs const& args);
		void DeviceStatusTeachingTip_Closed(winrt::Microsoft::UI::Xaml::Controls::TeachingTip const& sender, winrt::Microsoft::UI::Xaml::Controls::TeachingTipClosedEventArgs const& args);
		Windows::Foundation::IAsyncAction DeviceControlsTeachingTip_Closed(winrt::Microsoft::UI::Xaml::Controls::TeachingTip const& sender, winrt::Microsoft::UI::Xaml::Controls::TeachingTipClosedEventArgs const& args);
		void ButtonFlyout_Opening(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void ButtonFlyout_Closing(winrt::Microsoft::UI::Xaml::Controls::Primitives::FlyoutBase const& sender, winrt::Microsoft::UI::Xaml::Controls::Primitives::FlyoutBaseClosingEventArgs const& args);
	};
}

namespace winrt::Amethyst::factory_implementation
{
	struct DevicesPage : DevicesPageT<DevicesPage, implementation::DevicesPage>
	{
	};
}
