#pragma once

#include "InfoPage.g.h"

namespace winrt::KinectToVR::implementation
{
    struct InfoPage : InfoPageT<InfoPage>
    {
        InfoPage();
    };
}

namespace winrt::KinectToVR::factory_implementation
{
    struct InfoPage : InfoPageT<InfoPage, implementation::InfoPage>
    {
    };
}
