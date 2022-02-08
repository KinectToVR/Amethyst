#pragma once

#include "InfoPage.g.h"
#include "K2Shared.h"

namespace winrt::KinectToVR::implementation
{
	struct InfoPage : InfoPageT<InfoPage>
	{
		InfoPage();
		void K2DoubleTapped(winrt::Windows::Foundation::IInspectable const& sender,
		                    winrt::Microsoft::UI::Xaml::Input::DoubleTappedRoutedEventArgs const& e);
	};
}

namespace winrt::KinectToVR::factory_implementation
{
	struct InfoPage : InfoPageT<InfoPage, implementation::InfoPage>
	{
	};
}
