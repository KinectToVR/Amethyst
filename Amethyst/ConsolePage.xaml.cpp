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

namespace winrt::KinectToVR::implementation
{
	ConsolePage::ConsolePage()
	{
		InitializeComponent();
	}
}


void KinectToVR::implementation::ConsolePage::ConsolePage_Loaded(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	LOG(INFO) << "Ohhhhh, How Sweeet!";
}


void KinectToVR::implementation::ConsolePage::DevicesCrashButton_Click(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	exit(-12);
}


void KinectToVR::implementation::ConsolePage::OpenVRCrashButton_Click(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	exit(-11);
}


void KinectToVR::implementation::ConsolePage::HRESULTCrashButton_Click(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	check_hresult(FALSE);
}


void KinectToVR::implementation::ConsolePage::DelegateCrashButton_Click(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	throw hresult_illegal_delegate_assignment();
}


void KinectToVR::implementation::ConsolePage::NullCrashButton_Click(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	// This is only a sample, null pointer exceptions
	//      actually happen sometimes, e.g. if an element
	//      in UI is referenced before even constructing it

	int* _pointer = nullptr;
	*_pointer = 420;
}


void KinectToVR::implementation::ConsolePage::DatePicker_SelectedDateChanged(
	const Controls::DatePicker& sender, const Controls::DatePickerSelectedValueChangedEventArgs& args)
{
	if (Windows::Globalization::DateTimeFormatting::DateTimeFormatter(
		L"{year.full}/{month.integer(2)}/{day.integer(2)}"
	).Format(DatePicker().Date()) == L"2022/02/21")
		WhaaatFlyout().ShowAt(sender);
}
