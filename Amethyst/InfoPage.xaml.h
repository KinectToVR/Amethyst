#pragma once

#include "InfoPage.g.h"
#include "K2Shared.h"
#include "K2Interfacing.h"

namespace winrt::Amethyst::implementation
{
	struct InfoPage : InfoPageT<InfoPage>
	{
		InfoPage();
		void K2DoubleTapped(const Windows::Foundation::IInspectable& sender,
		                    const winrt::Microsoft::UI::Xaml::Input::DoubleTappedRoutedEventArgs& e);
		void Grid_Loaded(const winrt::Windows::Foundation::IInspectable& sender,
		                 const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
	};
}

namespace winrt::Amethyst::factory_implementation
{
	struct InfoPage : InfoPageT<InfoPage, implementation::InfoPage>
	{
	};
}
