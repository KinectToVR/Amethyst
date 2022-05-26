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

namespace muxc
{
	using namespace winrt::Microsoft::UI::Xaml::Controls;
};

namespace wuxc
{
	using namespace winrt::Windows::UI::Xaml::Controls;
};

namespace winrt::KinectToVR::implementation
{
	struct MainWindow : MainWindowT<MainWindow>
	{
		MainWindow();

		void NavView_Loaded(const Windows::Foundation::IInspectable& sender,
		                    const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void NavView_ItemInvoked(const winrt::Microsoft::UI::Xaml::Controls::NavigationView& sender,
		                         const winrt::Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs& args);
		void NavView_BackRequested(const winrt::Microsoft::UI::Xaml::Controls::NavigationView& sender,
		                           const winrt::Microsoft::UI::Xaml::Controls::NavigationViewBackRequestedEventArgs&
		                           args);
		void ContentFrame_NavigationFailed(const Windows::Foundation::IInspectable& sender,
		                                   const winrt::Microsoft::UI::Xaml::Navigation::NavigationFailedEventArgs& e);

		void NavView_Navigate(
			std::wstring navItemTag,
			const Microsoft::UI::Xaml::Media::Animation::NavigationTransitionInfo& transitionInfo);

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

	private:
		// Vector of std::pair holding the Navigation Tag and the relative Navigation Page.
		std::vector<std::pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>> m_pages;

	public:
		Windows::Foundation::IAsyncAction checkUpdates(const winrt::Microsoft::UI::Xaml::UIElement& show_el,
		                                               bool show = false, DWORD delay_ms = 0);
		void InstallLaterButton_Click(
			const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void InstallNowButton_Click(const Windows::Foundation::IInspectable& sender,
		                            const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ExitButton_Click(const Windows::Foundation::IInspectable& sender,
		                      const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void MinimizeButton_Click(const Windows::Foundation::IInspectable& sender,
		                          const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		Windows::Foundation::IAsyncAction UpdateButton_Loaded(const Windows::Foundation::IInspectable& sender,
		                                                      const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		Windows::Foundation::IAsyncAction UpdateButton_Tapped(const Windows::Foundation::IInspectable& sender,
		                                                      const
		                                                      winrt::Microsoft::UI::Xaml::Input::TappedRoutedEventArgs&
		                                                      e);
	};
}

namespace winrt::KinectToVR::factory_implementation
{
	struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
	{
	};
}
