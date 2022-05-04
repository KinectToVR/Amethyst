#pragma once

#include "SettingsPage.g.h"
#include "K2Shared.h"
#include "TrackingDevices.h"
#include "K2Interfacing.h"

namespace winrt::KinectToVR::implementation
{
    struct SettingsPage : SettingsPageT<SettingsPage>
    {
        SettingsPage();
        void WaistOnToggle_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void WaistOnToggle_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void LeftFootOnToggle_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void LeftFootOnToggle_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RightFootOnToggle_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RightFootOnToggle_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void LeftElbowOnToggle_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void LeftElbowOnToggle_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RightElbowOnToggle_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RightElbowOnToggle_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    	void LeftKneeOnToggle_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void LeftKneeOnToggle_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RightKneeOnToggle_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RightKneeOnToggle_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ExternalFlipCheckBox_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ExternalFlipCheckBox_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RestartButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ResetButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SettingsPage_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void AutoSpawn_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void AutoSpawn_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void EnableSounds_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void EnableSounds_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SoundsVolumeSlider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);
        void CalibrateExternalFlipMenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void WaistDropDown_Expanding(winrt::Microsoft::UI::Xaml::Controls::Expander const& sender, winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& args);
        Windows::Foundation::IAsyncAction WaistTrackerEnabledToggle_Toggled(
	        winrt::Windows::Foundation::IInspectable const& sender,
	        winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void WaistPositionFilterOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void FeetDropDown_Expanding(winrt::Microsoft::UI::Xaml::Controls::Expander const& sender, winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& args);
        Windows::Foundation::IAsyncAction FeetTrackersEnabledToggle_Toggled(
	        winrt::Windows::Foundation::IInspectable const& sender,
	        winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void FeetPositionFilterOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void FeetRotationFilterOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void WaistRotationFilterOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void KneesDropDown_Expanding(winrt::Microsoft::UI::Xaml::Controls::Expander const& sender, winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& args);
        Windows::Foundation::IAsyncAction KneeTrackersEnabledToggle_Toggled(
	        winrt::Windows::Foundation::IInspectable const& sender,
	        winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void KneePositionFilterOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void KneeRotationFilterOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void ElbowsDropDown_Expanding(winrt::Microsoft::UI::Xaml::Controls::Expander const& sender, winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& args);
        Windows::Foundation::IAsyncAction ElbowTrackersEnabledToggle_Toggled(
	        winrt::Windows::Foundation::IInspectable const& sender,
	        winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ElbowsPositionFilterOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void ElbowsRotationFilterOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void FlipDropDown_Expanding(winrt::Microsoft::UI::Xaml::Controls::Expander const& sender, winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& args);
        void FlipToggle_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::KinectToVR::factory_implementation
{
    struct SettingsPage : SettingsPageT<SettingsPage, implementation::SettingsPage>
    {
    };
}
