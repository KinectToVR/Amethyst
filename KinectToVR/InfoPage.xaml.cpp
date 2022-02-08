#include "pch.h"
#include "InfoPage.xaml.h"
#if __has_include("InfoPage.g.cpp")
#include "InfoPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::KinectToVR::implementation
{
	InfoPage::InfoPage()
	{
		InitializeComponent();
	}

	void InfoPage::K2DoubleTapped(
		winrt::Windows::Foundation::IInspectable const& sender,
		winrt::Microsoft::UI::Xaml::Input::DoubleTappedRoutedEventArgs const& e)
	{
		// Also show a notification when we get to winui 1.1
		k2app::shared::main::consoleItem.get()->Visibility(Visibility::Visible);
	}
}
