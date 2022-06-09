#pragma once

#include "SettingsPage.g.h"
#include "TrackingDevices.h"
#include "JointExpander.h"
#include "LocalizedServerStatuses.h"

namespace winrt::KinectToVR::implementation
{
	struct SettingsPage : SettingsPageT<SettingsPage>
	{
		SettingsPage();

		void ExternalFlipCheckBox_Checked(const Windows::Foundation::IInspectable& sender,
		                                  const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ExternalFlipCheckBox_Unchecked(const Windows::Foundation::IInspectable& sender,
		                                    const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void RestartButton_Click(const Windows::Foundation::IInspectable& sender,
		                         const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		winrt::Windows::Foundation::IAsyncAction ResetButton_Click(
			const Windows::Foundation::IInspectable& sender,
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
		void LearnAboutFiltersButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
		                                   winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void LearnAboutFiltersFlyout_Closed(winrt::Windows::Foundation::IInspectable const& sender,
		                                    winrt::Windows::Foundation::IInspectable const& e);
		void TrackerConfigButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
		                               winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void CheckOverlapsCheckBox_Checked(winrt::Windows::Foundation::IInspectable const& sender,
		                                   winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void CheckOverlapsCheckBox_Unchecked(winrt::Windows::Foundation::IInspectable const& sender,
		                                     winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
	};
}

namespace winrt::KinectToVR::factory_implementation
{
	struct SettingsPage : SettingsPageT<SettingsPage, implementation::SettingsPage>
	{
	};
}
