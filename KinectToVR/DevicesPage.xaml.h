#pragma once

#include "DevicesPage.g.h"

namespace winrt::KinectToVR::implementation
{
    struct DevicesPage : DevicesPageT<DevicesPage>
    {
        DevicesPage();
    };
}

namespace winrt::KinectToVR::factory_implementation
{
    struct DevicesPage : DevicesPageT<DevicesPage, implementation::DevicesPage>
    {
    };
}
