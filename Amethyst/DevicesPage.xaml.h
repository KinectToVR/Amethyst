#pragma once
#include "DevicesPage.g.h"

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

		Windows::Foundation::IAsyncAction ReloadSelectedDevice(const bool& _manual = false);

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
		void DevicesPage_Loaded_Handler();
		void OpenDiscordButton_Click(const Windows::Foundation::IInspectable& sender,
		                             const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OpenDocsButton_Click(const Windows::Foundation::IInspectable& sender,
		                          const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DevicesListTeachingTip_Closed(
			const Microsoft::UI::Xaml::Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args);
		void DeviceStatusTeachingTip_Closed(
			const Microsoft::UI::Xaml::Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args);
		Windows::Foundation::IAsyncAction DeviceControlsTeachingTip_Closed(
			const Microsoft::UI::Xaml::Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args);
		void ButtonFlyout_Opening(const Windows::Foundation::IInspectable& sender,
		                          const Windows::Foundation::IInspectable& e);
		void ButtonFlyout_Closing(const Microsoft::UI::Xaml::Controls::Primitives::FlyoutBase& sender,
		                          const Microsoft::UI::Xaml::Controls::Primitives::FlyoutBaseClosingEventArgs& args);
		Windows::Foundation::IAsyncAction DevicesListTeachingTip_ActionButtonClick(
			const Microsoft::UI::Xaml::Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args);
		void DeviceStatusTeachingTip_ActionButtonClick(
			const Microsoft::UI::Xaml::Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args);
		void DeviceControlsTeachingTip_ActionButtonClick(
			const Microsoft::UI::Xaml::Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args);
		Windows::Foundation::IAsyncAction TrackingDeviceTreeView_ItemInvoked(
			winrt::Microsoft::UI::Xaml::Controls::TreeView const& sender, winrt::Microsoft::UI::Xaml::Controls::TreeViewItemInvokedEventArgs const& args);
	};
}

namespace winrt::Amethyst::factory_implementation
{
	struct DevicesPage : DevicesPageT<DevicesPage, implementation::DevicesPage>
	{
	};
}
