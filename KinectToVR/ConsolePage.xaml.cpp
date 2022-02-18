#include "pch.h"
#include "ConsolePage.xaml.h"

#include "K2Interfacing.h"
#include "K2Settings.h"

#if __has_include("ConsolePage.g.cpp")
#include "ConsolePage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

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


void winrt::KinectToVR::implementation::ConsolePage::ConsolePage_Loaded(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	console_setup_finished = true;
	LOG(INFO) << "Experiments page setup finished.";
}


void winrt::KinectToVR::implementation::ConsolePage::ExpCheckBox_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	if (!console_setup_finished)return;

	ExpScrollViewer().Visibility(Visibility::Visible);
	ThanksScrollViewer().Visibility(Visibility::Collapsed);
}


void winrt::KinectToVR::implementation::ConsolePage::ExpCheckBox_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	if (!console_setup_finished)return;

	ExpScrollViewer().Visibility(Visibility::Collapsed);
	ThanksScrollViewer().Visibility(Visibility::Visible);
}


void winrt::KinectToVR::implementation::ConsolePage::ToggleTrackingButton_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't check if setup's finished since we're gonna emulate a click rather than change the state only
	ToggleTrackingButton().Content(box_value(L"Unfreeze Tracking"));

	// Mark tracking as unfrozen
	k2app::interfacing::isTrackingFrozen = true;
}


void winrt::KinectToVR::implementation::ConsolePage::ToggleTrackingButton_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't check if setup's finished since we're gonna emulate a click rather than change the state only
	ToggleTrackingButton().Content(box_value(L"Freeze Tracking"));

	// Mark trackers as frozen
	k2app::interfacing::isTrackingFrozen = false;
}
