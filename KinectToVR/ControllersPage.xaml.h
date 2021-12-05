#pragma once

#include "ControllersPage.g.h"

namespace winrt::KinectToVR::implementation
{
    struct ControllersPage : ControllersPageT<ControllersPage>
    {
        ControllersPage();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void myButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::KinectToVR::factory_implementation
{
    struct ControllersPage : ControllersPageT<ControllersPage, implementation::ControllersPage>
    {
    };
}
