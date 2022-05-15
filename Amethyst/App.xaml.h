#pragma once

#include "App.xaml.g.h"

namespace winrt::KinectToVR::implementation
{
	struct App : AppT<App>
	{
		App();

		void OnLaunched(const Microsoft::UI::Xaml::LaunchActivatedEventArgs&);

	private:
		winrt::Microsoft::UI::Xaml::Window window{nullptr};
	};
}
