#pragma once

#include "DevicesPage.g.h"

namespace winrt::KinectToVR::implementation
{
    struct DevicesPage : DevicesPageT<DevicesPage>
    {
        DevicesPage();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void myButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::KinectToVR::factory_implementation
{
    struct DevicesPage : DevicesPageT<DevicesPage, implementation::DevicesPage>
    {
    };
}
