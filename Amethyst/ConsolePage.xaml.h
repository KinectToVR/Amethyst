#pragma once

#include "ConsolePage.g.h"

namespace winrt::Amethyst::implementation
{
	struct ConsolePage : ConsolePageT<ConsolePage>
	{
		ConsolePage();
		void ConsolePage_Loaded(const Windows::Foundation::IInspectable& sender,
		                        const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DevicesCrashButton_Click(const Windows::Foundation::IInspectable& sender,
		                              const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OpenVRCrashButton_Click(const Windows::Foundation::IInspectable& sender,
		                             const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void HRESULTCrashButton_Click(const Windows::Foundation::IInspectable& sender,
		                              const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DelegateCrashButton_Click(const Windows::Foundation::IInspectable& sender,
		                               const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void NullCrashButton_Click(const Windows::Foundation::IInspectable& sender,
		                           const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void GuideButton_Click(const Windows::Foundation::IInspectable& sender,
		                       const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void UpdateButton_Click(const Windows::Foundation::IInspectable& sender,
		                        const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DatePicker_SelectedDateChanged(const Microsoft::UI::Xaml::Controls::DatePicker& sender,
		                                    const
		                                    Microsoft::UI::Xaml::Controls::DatePickerSelectedValueChangedEventArgs
		                                    & args);
	};
}

namespace winrt::Amethyst::factory_implementation
{
	struct ConsolePage : ConsolePageT<ConsolePage, implementation::ConsolePage>
	{
	};
}
