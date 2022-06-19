#include "pch.h"
#include "ConsolePage.xaml.h"

#include "K2Interfacing.h"
#include "K2Settings.h"

#if __has_include("ConsolePage.g.cpp")
#include "ConsolePage.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

bool console_setup_finished = false;

namespace winrt::Amethyst::implementation
{
	ConsolePage::ConsolePage()
	{
		InitializeComponent();
	}
}


void Amethyst::implementation::ConsolePage::ConsolePage_Loaded(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	LOG(INFO) << "Ohhhhh, How Sweeet!";
}


void Amethyst::implementation::ConsolePage::DevicesCrashButton_Click(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	exit(-12);
}


void Amethyst::implementation::ConsolePage::OpenVRCrashButton_Click(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	exit(-11);
}


void Amethyst::implementation::ConsolePage::HRESULTCrashButton_Click(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	check_hresult(FALSE);
}


void Amethyst::implementation::ConsolePage::DelegateCrashButton_Click(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	throw hresult_illegal_delegate_assignment();
}


void Amethyst::implementation::ConsolePage::NullCrashButton_Click(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	// This is only a sample, null pointer exceptions
	//      actually happen sometimes, e.g. if an element
	//      in UI is referenced before even constructing it

	int* _pointer = nullptr;
	*_pointer = 420;
}


void Amethyst::implementation::ConsolePage::DatePicker_SelectedDateChanged(
	const Controls::DatePicker& sender, const Controls::DatePickerSelectedValueChangedEventArgs& args)
{
	if (Windows::Globalization::DateTimeFormatting::DateTimeFormatter(
			L"{year.full}/{month.integer(2)}/{day.integer(2)}"
		).Format(DatePicker().Date()) ==
		Windows::Globalization::DateTimeFormatting::DateTimeFormatter(
			L"{year.full}/{month.integer(2)}/{day.integer(2)}"
		).Format(Windows::Foundation::DateTime(clock::now())))
		WhaaatFlyout().ShowAt(sender);
}
