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

		k2app::shared::other::toggleFreezeButton = std::make_shared<
			winrt::Microsoft::UI::Xaml::Controls::Primitives::ToggleButton>(ToggleTrackingButton());
	}
}


void winrt::KinectToVR::implementation::ConsolePage::ConsolePage_Loaded(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	console_setup_finished = true;
	k2app::shared::other::toggleFreezeButton->IsChecked(k2app::interfacing::isTrackingFrozen);
	k2app::shared::other::toggleFreezeButton->Content(k2app::interfacing::isTrackingFrozen
		                                                  ? box_value(L"Unfreeze Tracking")
		                                                  : box_value(L"Freeze Tracking"));
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

void winrt::KinectToVR::implementation::ConsolePage::DevicesCrashButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	exit(-12);
}


void winrt::KinectToVR::implementation::ConsolePage::OpenVRCrashButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	exit(-11);
}


void winrt::KinectToVR::implementation::ConsolePage::HRESULTCrashButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	winrt::check_hresult(FALSE);
}


void winrt::KinectToVR::implementation::ConsolePage::DelegateCrashButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	throw winrt::hresult_illegal_delegate_assignment();
}


void winrt::KinectToVR::implementation::ConsolePage::NullCrashButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// This is only a sample, null pointer exceptions
	//      actually happen sometimes, e.g. if an element
	//      in UI is referenced before even constructing it

	int* _pointer = nullptr;
	*_pointer = 420;
}
