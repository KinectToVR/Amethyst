#pragma once

#include "GeneralPage.g.h"

namespace winrt::App1::implementation
{
    struct GeneralPage : GeneralPageT<GeneralPage>
    {
        GeneralPage();
    };
}

namespace winrt::App1::factory_implementation
{
    struct GeneralPage : GeneralPageT<GeneralPage, implementation::GeneralPage>
    {
    };
}
