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
		                    const Microsoft::UI::Xaml::Input::DoubleTappedRoutedEventArgs& e);
		void Grid_Loaded(const Windows::Foundation::IInspectable& sender,
		                 const Microsoft::UI::Xaml::RoutedEventArgs& e);
		Windows::Foundation::IAsyncAction EndingTeachingTip_CloseButtonClick(
			const winrt::Microsoft::UI::Xaml::Controls::TeachingTip& sender,
			const winrt::Windows::Foundation::IInspectable& args);
	};
}

namespace winrt::Amethyst::factory_implementation
{
	struct InfoPage : InfoPageT<InfoPage, implementation::InfoPage>
	{
	};
}
