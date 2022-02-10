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

        void NavView_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void NavView_ItemInvoked(winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs const& args);
        void NavView_BackRequested(winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewBackRequestedEventArgs const& args);
        void ContentFrame_NavigationFailed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Navigation::NavigationFailedEventArgs const& e);

        void NavView_Navigate(
            std::wstring navItemTag,
            Microsoft::UI::Xaml::Media::Animation::NavigationTransitionInfo const& transitionInfo);

        void On_Navigated(
            Windows::Foundation::IInspectable const& /* sender */,
            Windows::UI::Xaml::Navigation::NavigationEventArgs const& args);
        void CoreDispatcher_AcceleratorKeyActivated(
            Windows::UI::Core::CoreDispatcher const& /* sender */,
            Windows::UI::Core::AcceleratorKeyEventArgs const& args);
        void CoreWindow_PointerPressed(
            Windows::UI::Core::CoreWindow const& /* sender */,
            Windows::UI::Core::PointerEventArgs const& args);
        void System_BackRequested(
            Windows::Foundation::IInspectable const& /* sender */,
            Windows::UI::Core::BackRequestedEventArgs const& args);
        bool TryGoBack();

	private:
		// Vector of std::pair holding the Navigation Tag and the relative Navigation Page.
		std::vector<std::pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>> m_pages;

    public:
        Windows::Foundation::IAsyncAction checkUpdates(winrt::Microsoft::UI::Xaml::UIElement const& show_el, bool show = false, DWORD delay_ms = 0);void InstallLaterButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void InstallNowButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ExitButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void MinimizeButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction UpdateButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction UpdateButton_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::KinectToVR::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
