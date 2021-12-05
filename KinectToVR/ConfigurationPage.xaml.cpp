﻿#include "pch.h"
#include "ConfigurationPage.xaml.h"
#if __has_include("ConfigurationPage.g.cpp")
#include "ConfigurationPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::KinectToVR::implementation
{
    ConfigurationPage::ConfigurationPage()
    {
        InitializeComponent();
    }

    int32_t ConfigurationPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void ConfigurationPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void ConfigurationPage::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        myButton().Content(box_value(L"Clicked"));
    }
}
