#pragma once

#include "ConsolePage.g.h"

namespace winrt::KinectToVR::implementation
{
    struct ConsolePage : ConsolePageT<ConsolePage>
    {
        ConsolePage();
    };
}

namespace winrt::KinectToVR::factory_implementation
{
    struct ConsolePage : ConsolePageT<ConsolePage, implementation::ConsolePage>
    {
    };
}
