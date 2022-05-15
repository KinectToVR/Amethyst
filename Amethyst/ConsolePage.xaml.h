#pragma once

#include "ConsolePage.g.h"

namespace winrt::KinectToVR::implementation
{
	struct ConsolePage : ConsolePageT<ConsolePage>
	{
		ConsolePage();
		void ExpCheckBox_Checked(const Windows::Foundation::IInspectable& sender,
		                         const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ExpCheckBox_Unchecked(const Windows::Foundation::IInspectable& sender,
		                           const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ConsolePage_Loaded(const Windows::Foundation::IInspectable& sender,
		                        const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DevicesCrashButton_Click(const Windows::Foundation::IInspectable& sender,
		                              const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OpenVRCrashButton_Click(const Windows::Foundation::IInspectable& sender,
		                             const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void HRESULTCrashButton_Click(const Windows::Foundation::IInspectable& sender,
		                              const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DelegateCrashButton_Click(const Windows::Foundation::IInspectable& sender,
		                               const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void NullCrashButton_Click(const Windows::Foundation::IInspectable& sender,
		                           const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ToggleTrackingButton_Click(const winrt::Microsoft::UI::Xaml::Controls::SplitButton& sender,
		                                const winrt::Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs& args);
		void FreezeOnlyLowerCheckBox_Checked(const Windows::Foundation::IInspectable& sender,
		                                     const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void FreezeOnlyLowerCheckBox_Unchecked(const Windows::Foundation::IInspectable& sender,
		                                       const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void LanguageOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                      const Windows::Foundation::IInspectable& e);
	};
}

namespace winrt::KinectToVR::factory_implementation
{
	struct ConsolePage : ConsolePageT<ConsolePage, implementation::ConsolePage>
	{
	};
}
