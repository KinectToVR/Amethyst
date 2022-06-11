#pragma once

#include "ConsolePage.g.h"

namespace winrt::Amethyst::implementation
{
	struct ConsolePage : ConsolePageT<ConsolePage>
	{
		ConsolePage();
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
		void DatePicker_SelectedDateChanged(winrt::Microsoft::UI::Xaml::Controls::DatePicker const& sender, winrt::Microsoft::UI::Xaml::Controls::DatePickerSelectedValueChangedEventArgs const& args);
	};
}

namespace winrt::Amethyst::factory_implementation
{
	struct ConsolePage : ConsolePageT<ConsolePage, implementation::ConsolePage>
	{
	};
}
