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
		Windows::Foundation::IAsyncAction executeUpdates();

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
		void InitializerTeachingTip_ActionButtonClick(const Microsoft::UI::Xaml::Controls::TeachingTip& sender,
		                                              const Windows::Foundation::IInspectable& args);
		void InitializerTeachingTip_CloseButtonClick(const Microsoft::UI::Xaml::Controls::TeachingTip& sender,
		                                             const Windows::Foundation::IInspectable& args);
		void HelpButton_Tapped(const Windows::Foundation::IInspectable& sender,
		                       const Microsoft::UI::Xaml::Input::TappedRoutedEventArgs& e);
		void HelpFlyoutDocsButton_Click(const Windows::Foundation::IInspectable& sender,
		                                const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void HelpFlyoutDiscordButton_Click(const Windows::Foundation::IInspectable& sender,
		                                   const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void HelpFlyoutDevButton_Click(const Windows::Foundation::IInspectable& sender,
		                               const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ButtonFlyout_Opening(const Windows::Foundation::IInspectable& sender,
		                          const Windows::Foundation::IInspectable& e);
		void ButtonFlyout_Closing(const Microsoft::UI::Xaml::Controls::Primitives::FlyoutBase& sender,
		                          const Microsoft::UI::Xaml::Controls::Primitives::FlyoutBaseClosingEventArgs& args);
		Windows::Foundation::IAsyncAction HelpFlyoutLicensesButton_Click(
			const Windows::Foundation::IInspectable& sender,
			const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void LicensesFlyout_Closed(const Windows::Foundation::IInspectable& sender,
		                           const Windows::Foundation::IInspectable& e);
		void LicensesFlyout_Opening(const Windows::Foundation::IInspectable& sender,
		                            const Windows::Foundation::IInspectable& e);
		void XMainGrid_Loaded(const Windows::Foundation::IInspectable& sender,
		                      const Microsoft::UI::Xaml::RoutedEventArgs& e);
		Windows::Foundation::IAsyncAction XMainGrid_Loaded_Handler();
	};
}

namespace winrt::Amethyst::factory_implementation
{
	struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
	{
	};
}
