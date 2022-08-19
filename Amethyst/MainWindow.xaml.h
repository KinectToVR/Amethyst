#pragma once

#include "MainWindow.g.h"

#include "GeneralPage.g.h"
#include "SettingsPage.g.h"
#include "DevicesPage.g.h"
#include "InfoPage.g.h"
#include "ConsolePage.g.h"

#include "TrackingDevices.h"
#include "K2Interfacing.h"
#include "K2Main.h"

#include <openvr.h>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

namespace muxc
{
	using namespace winrt::Microsoft::UI::Xaml::Controls;
}

namespace wuxc
{
	using namespace winrt::Windows::UI::Xaml::Controls;
}

namespace winrt::Amethyst::implementation
{
	struct MainWindow : MainWindowT<MainWindow>
	{
		MainWindow();

		void NavView_Loaded(const Windows::Foundation::IInspectable& sender,
		                    const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void NavView_ItemInvoked(const Microsoft::UI::Xaml::Controls::NavigationView& sender,
		                         const Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs& args);
		void NavView_BackRequested(const Microsoft::UI::Xaml::Controls::NavigationView& sender,
		                           const Microsoft::UI::Xaml::Controls::NavigationViewBackRequestedEventArgs&
		                           args);
		void ContentFrame_NavigationFailed(const Windows::Foundation::IInspectable& sender,
		                                   const Microsoft::UI::Xaml::Navigation::NavigationFailedEventArgs& e);

		void On_Navigated(
			const Windows::Foundation::IInspectable& /* sender */,
			const Windows::UI::Xaml::Navigation::NavigationEventArgs& args);
		void CoreDispatcher_AcceleratorKeyActivated(
			const Windows::UI::Core::CoreDispatcher& /* sender */,
			const Windows::UI::Core::AcceleratorKeyEventArgs& args);
		void CoreWindow_PointerPressed(
			const Windows::UI::Core::CoreWindow& /* sender */,
			const Windows::UI::Core::PointerEventArgs& args);
		void System_BackRequested(
			const Windows::Foundation::IInspectable& /* sender */,
			const Windows::UI::Core::BackRequestedEventArgs& args);
		bool TryGoBack();
		
		Windows::Foundation::IAsyncAction checkUpdates(bool show = false, DWORD delay_ms = 0);
		void InstallLaterButton_Click(
			const Windows::Foundation::IInspectable& sender, const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void InstallNowButton_Click(const Windows::Foundation::IInspectable& sender,
		                            const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ExitButton_Click(const Windows::Foundation::IInspectable& sender,
		                      const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void MinimizeButton_Click(const Windows::Foundation::IInspectable& sender,
		                          const Microsoft::UI::Xaml::RoutedEventArgs& e);
		Windows::Foundation::IAsyncAction UpdateButton_Loaded(const Windows::Foundation::IInspectable& sender,
		                                                      const Microsoft::UI::Xaml::RoutedEventArgs& e);
		Windows::Foundation::IAsyncAction UpdateButton_Tapped(const Windows::Foundation::IInspectable& sender,
		                                                      const
		                                                      Microsoft::UI::Xaml::Input::TappedRoutedEventArgs&
		                                                      e);
		void InitializerTeachingTip_ActionButtonClick(winrt::Microsoft::UI::Xaml::Controls::TeachingTip const& sender, winrt::Windows::Foundation::IInspectable const& args);
		void InitializerTeachingTip_CloseButtonClick(winrt::Microsoft::UI::Xaml::Controls::TeachingTip const& sender, winrt::Windows::Foundation::IInspectable const& args);
		void HelpButton_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::TappedRoutedEventArgs const& e);
		void HelpFlyoutDocsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void HelpFlyoutDiscordButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void HelpFlyoutDevButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void ButtonFlyout_Opening(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void ButtonFlyout_Closing(winrt::Microsoft::UI::Xaml::Controls::Primitives::FlyoutBase const& sender, winrt::Microsoft::UI::Xaml::Controls::Primitives::FlyoutBaseClosingEventArgs const& args);
		winrt::Windows::Foundation::IAsyncAction HelpFlyoutLicensesButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void LicensesFlyout_Closed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void LicensesFlyout_Opening(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void XMainGrid_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
	};
}

namespace winrt::Amethyst::factory_implementation
{
	struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
	{
	};
}
