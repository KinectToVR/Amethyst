#include "pch.h"
#include "SettingsPage.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::KinectToVR::implementation
{
    SettingsPage::SettingsPage()
    {
        InitializeComponent();
    }
}


void winrt::KinectToVR::implementation::SettingsPage::WaistOnToggleButton_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    WaistOnToggleButton().Content(box_value(L"Turn Off"));
}


void winrt::KinectToVR::implementation::SettingsPage::WaistOnToggleButton_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    WaistOnToggleButton().Content(box_value(L"Turn On"));
}


void winrt::KinectToVR::implementation::SettingsPage::WaistEnabledToggleButton_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    WaistEnabledToggleButton().Content(box_value(L"Disable"));
}


void winrt::KinectToVR::implementation::SettingsPage::WaistEnabledToggleButton_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    WaistEnabledToggleButton().Content(box_value(L"Enable"));
}


void winrt::KinectToVR::implementation::SettingsPage::LeftFootOnToggleButton_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    LeftFootOnToggleButton().Content(box_value(L"Turn Off"));
}


void winrt::KinectToVR::implementation::SettingsPage::LeftFootOnToggleButton_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    LeftFootOnToggleButton().Content(box_value(L"Turn On"));
}


void winrt::KinectToVR::implementation::SettingsPage::LeftFootEnabledToggleButton_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    LeftFootEnabledToggleButton().Content(box_value(L"Disable"));
}


void winrt::KinectToVR::implementation::SettingsPage::LeftFootEnabledToggleButton_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    LeftFootEnabledToggleButton().Content(box_value(L"Enable"));
}


void winrt::KinectToVR::implementation::SettingsPage::RightFootOnToggleButton_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    RightFootOnToggleButton().Content(box_value(L"Turn Off"));
}


void winrt::KinectToVR::implementation::SettingsPage::RightFootOnToggleButton_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    RightFootOnToggleButton().Content(box_value(L"Turn On"));
}


void winrt::KinectToVR::implementation::SettingsPage::RightFootEnabledToggleButton_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    RightFootEnabledToggleButton().Content(box_value(L"Disable"));
}


void winrt::KinectToVR::implementation::SettingsPage::RightFootEnabledToggleButton_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    RightFootEnabledToggleButton().Content(box_value(L"Enable"));
}


void winrt::KinectToVR::implementation::SettingsPage::SoundsEnabledToggleButton_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    SoundsEnabledToggleButton().Content(box_value(L"Disable Sounds"));
}


void winrt::KinectToVR::implementation::SettingsPage::SoundsEnabledToggleButton_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    SoundsEnabledToggleButton().Content(box_value(L"Enable Sounds"));
}


void winrt::KinectToVR::implementation::SettingsPage::TrackingFreezeToggleButton_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    TrackingFreezeToggleButton().Content(box_value(L"Freeze Tracking"));
}


void winrt::KinectToVR::implementation::SettingsPage::TrackingFreezeToggleButton_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    TrackingFreezeToggleButton().Content(box_value(L"Resume Tracking"));
}


void winrt::KinectToVR::implementation::SettingsPage::ResetConfigButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    // Reset?
}


void winrt::KinectToVR::implementation::SettingsPage::SoundsVolumeSlider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e)
{
    // Capture?
}


void winrt::KinectToVR::implementation::SettingsPage::AutoSpawnCheckBox_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{

}


void winrt::KinectToVR::implementation::SettingsPage::AutoSpawnCheckBox_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{

}
