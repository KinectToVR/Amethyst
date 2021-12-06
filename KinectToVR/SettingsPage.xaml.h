#pragma once

#include "SettingsPage.g.h"

namespace winrt::KinectToVR::implementation
{
    struct SettingsPage : SettingsPageT<SettingsPage>
    {
        SettingsPage();
    };
}

namespace winrt::KinectToVR::factory_implementation
{
    struct SettingsPage : SettingsPageT<SettingsPage, implementation::SettingsPage>
    {
    };
}
