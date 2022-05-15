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
		void WaistOnToggle_Checked(const Windows::Foundation::IInspectable& sender,
		                           const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void WaistOnToggle_Unchecked(const Windows::Foundation::IInspectable& sender,
		                             const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void LeftFootOnToggle_Checked(const Windows::Foundation::IInspectable& sender,
		                              const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void LeftFootOnToggle_Unchecked(const Windows::Foundation::IInspectable& sender,
		                                const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void RightFootOnToggle_Checked(const Windows::Foundation::IInspectable& sender,
		                               const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void RightFootOnToggle_Unchecked(const Windows::Foundation::IInspectable& sender,
		                                 const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void LeftElbowOnToggle_Checked(const Windows::Foundation::IInspectable& sender,
		                               const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void LeftElbowOnToggle_Unchecked(const Windows::Foundation::IInspectable& sender,
		                                 const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void RightElbowOnToggle_Checked(const Windows::Foundation::IInspectable& sender,
		                                const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void RightElbowOnToggle_Unchecked(const Windows::Foundation::IInspectable& sender,
		                                  const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void LeftKneeOnToggle_Checked(const Windows::Foundation::IInspectable& sender,
		                              const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void LeftKneeOnToggle_Unchecked(const Windows::Foundation::IInspectable& sender,
		                                const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void RightKneeOnToggle_Checked(const Windows::Foundation::IInspectable& sender,
		                               const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void RightKneeOnToggle_Unchecked(const Windows::Foundation::IInspectable& sender,
		                                 const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ExternalFlipCheckBox_Checked(const Windows::Foundation::IInspectable& sender,
		                                  const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ExternalFlipCheckBox_Unchecked(const Windows::Foundation::IInspectable& sender,
		                                    const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void RestartButton_Click(const Windows::Foundation::IInspectable& sender,
		                         const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ResetButton_Click(const Windows::Foundation::IInspectable& sender,
		                       const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void SettingsPage_Loaded(const Windows::Foundation::IInspectable& sender,
		                         const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void AutoSpawn_Checked(const Windows::Foundation::IInspectable& sender,
		                       const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void AutoSpawn_Unchecked(const Windows::Foundation::IInspectable& sender,
		                         const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void EnableSounds_Checked(const Windows::Foundation::IInspectable& sender,
		                          const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void EnableSounds_Unchecked(const Windows::Foundation::IInspectable& sender,
		                            const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void SoundsVolumeSlider_ValueChanged(const Windows::Foundation::IInspectable& sender,
		                                     const
		                                     winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs
		                                     & e);
		void CalibrateExternalFlipMenuFlyoutItem_Click(const Windows::Foundation::IInspectable& sender,
		                                               const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void WaistDropDown_Expanding(const winrt::Microsoft::UI::Xaml::Controls::Expander& sender,
		                             const winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs& args);
		Windows::Foundation::IAsyncAction WaistTrackerEnabledToggle_Toggled(
			const Windows::Foundation::IInspectable& sender,
			const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void WaistPositionFilterOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                   const
		                                                   winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                   & e);
		void FeetDropDown_Expanding(const winrt::Microsoft::UI::Xaml::Controls::Expander& sender,
		                            const winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs& args);
		Windows::Foundation::IAsyncAction FeetTrackersEnabledToggle_Toggled(
			const Windows::Foundation::IInspectable& sender,
			const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void FeetPositionFilterOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                  const
		                                                  winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                  & e);
		void FeetRotationFilterOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                  const
		                                                  winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                  & e);
		void WaistRotationFilterOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                   const
		                                                   winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                   & e);
		void KneesDropDown_Expanding(const winrt::Microsoft::UI::Xaml::Controls::Expander& sender,
		                             const winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs& args);
		Windows::Foundation::IAsyncAction KneeTrackersEnabledToggle_Toggled(
			const Windows::Foundation::IInspectable& sender,
			const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void KneePositionFilterOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                  const
		                                                  winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                  & e);
		void KneeRotationFilterOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                  const
		                                                  winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                  & e);
		void ElbowsDropDown_Expanding(const winrt::Microsoft::UI::Xaml::Controls::Expander& sender,
		                              const winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs& args);
		Windows::Foundation::IAsyncAction ElbowTrackersEnabledToggle_Toggled(
			const Windows::Foundation::IInspectable& sender,
			const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ElbowsPositionFilterOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                    const
		                                                    winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                    & e);
		void ElbowsRotationFilterOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                    const
		                                                    winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                    & e);
		void FlipDropDown_Expanding(const winrt::Microsoft::UI::Xaml::Controls::Expander& sender,
		                            const winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs& args);
		void FlipToggle_Toggled(const Windows::Foundation::IInspectable& sender,
		                        const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void AutoStartFlyout_Opening(const Windows::Foundation::IInspectable& sender,
		                             const Windows::Foundation::IInspectable& e);
		void AutoStartCheckBox_Checked(const Windows::Foundation::IInspectable& sender,
		                               const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void AutoStartCheckBox_Unchecked(const Windows::Foundation::IInspectable& sender,
		                                 const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ReManifestButton_Click(const winrt::Microsoft::UI::Xaml::Controls::SplitButton& sender,
		                            const winrt::Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs& args);
		void ReRegisterButton_Click(const Windows::Foundation::IInspectable& sender,
		                            const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DismissSetErrorButton_Click(const Windows::Foundation::IInspectable& sender,
		                                 const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
	};
}

namespace winrt::KinectToVR::factory_implementation
{
	struct SettingsPage : SettingsPageT<SettingsPage, implementation::SettingsPage>
	{
	};
}
