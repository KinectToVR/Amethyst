#include "pch.h"
#include "InfoPage.xaml.h"
#if __has_include("InfoPage.g.cpp")
#include "InfoPage.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Amethyst::implementation
{
	InfoPage::InfoPage()
	{
		InitializeComponent();
	}

	void InfoPage::K2DoubleTapped(
		const Windows::Foundation::IInspectable& sender,
		const Input::DoubleTappedRoutedEventArgs& e)
	{
		// Also show a notification when we get to winui 1.1
		k2app::shared::main::consoleItem.get()->Visibility(Visibility::Visible);
	}
}


void Amethyst::implementation::InfoPage::Grid_Loaded(
	const Windows::Foundation::IInspectable& sender,
	const RoutedEventArgs& e)
{
	// The info page was loaded
}
