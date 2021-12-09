#pragma once
#include "DevicesPage.g.h"
#include "TrackingDevicesView.h"
#include "PSMovesView.h"

namespace winrt::KinectToVR::implementation
{
    struct DevicesPage : DevicesPageT<DevicesPage>
    {
        DevicesPage();

        void TrackingDeviceListView_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void RefreshDeviceButton_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::TappedRoutedEventArgs const& e);
    };
}

namespace winrt::KinectToVR::factory_implementation
{
    struct DevicesPage : DevicesPageT<DevicesPage, implementation::DevicesPage>
    {
    };
}
