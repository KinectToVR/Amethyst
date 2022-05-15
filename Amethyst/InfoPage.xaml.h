#pragma once

#include "InfoPage.g.h"
#include "K2Shared.h"

namespace winrt::KinectToVR::implementation
{
	struct InfoPage : InfoPageT<InfoPage>
	{
		InfoPage();
		void K2DoubleTapped(const Windows::Foundation::IInspectable& sender,
		                    const winrt::Microsoft::UI::Xaml::Input::DoubleTappedRoutedEventArgs& e);
	};
}

namespace winrt::KinectToVR::factory_implementation
{
	struct InfoPage : InfoPageT<InfoPage, implementation::InfoPage>
	{
	};
}
