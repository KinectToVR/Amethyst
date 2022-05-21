#pragma once

#include "InfoPage.g.h"
#include "K2Shared.h"
#include "K2Interfacing.h"

namespace winrt::KinectToVR::implementation
{
	struct InfoPage : InfoPageT<InfoPage>
	{
		InfoPage();
		void K2DoubleTapped(const Windows::Foundation::IInspectable& sender,
		                    const winrt::Microsoft::UI::Xaml::Input::DoubleTappedRoutedEventArgs& e);
		void Grid_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
	};
}

namespace winrt::KinectToVR::factory_implementation
{
	struct InfoPage : InfoPageT<InfoPage, implementation::InfoPage>
	{
	};
}
