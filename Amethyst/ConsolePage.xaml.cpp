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

		k2app::shared::other::toggleFreezeButton = std::make_shared<
			winrt::Microsoft::UI::Xaml::Controls::ToggleSplitButton>(ToggleTrackingButton());

		k2app::shared::other::freezeOnlyLowerCheckBox = std::make_shared<
			winrt::Microsoft::UI::Xaml::Controls::CheckBox>(FreezeOnlyLowerCheckBox());
	}
}


void KinectToVR::implementation::ConsolePage::ConsolePage_Loaded(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	console_setup_finished = true;
	k2app::shared::other::toggleFreezeButton.get()->IsChecked(k2app::interfacing::isTrackingFrozen);
	k2app::shared::other::toggleFreezeButton.get()->Content(k2app::interfacing::isTrackingFrozen
		                                                        ? box_value(L"Unfreeze")
		                                                        : box_value(L"Freeze"));
	k2app::shared::other::freezeOnlyLowerCheckBox->IsChecked(k2app::K2Settings.freezeLowerOnly);
	LOG(INFO) << "Experiments page setup finished.";

	K2InsightsCLR::LogEvent("Okashi Tab Displayed");
}


void KinectToVR::implementation::ConsolePage::ExpCheckBox_Checked(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	if (!console_setup_finished)return;

	ExpScrollViewer().Visibility(Visibility::Visible);
	ThanksScrollViewer().Visibility(Visibility::Collapsed);
}


void KinectToVR::implementation::ConsolePage::ExpCheckBox_Unchecked(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	if (!console_setup_finished)return;

	ExpScrollViewer().Visibility(Visibility::Collapsed);
	ThanksScrollViewer().Visibility(Visibility::Visible);
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


void KinectToVR::implementation::ConsolePage::ToggleTrackingButton_Click(
	const winrt::Microsoft::UI::Xaml::Controls::SplitButton& sender,
	const winrt::Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs& args)
{
	k2app::interfacing::isTrackingFrozen = !k2app::interfacing::isTrackingFrozen;

	k2app::shared::other::toggleFreezeButton.get()->IsChecked(k2app::interfacing::isTrackingFrozen);
	k2app::shared::other::toggleFreezeButton.get()->Content(k2app::interfacing::isTrackingFrozen
		                                                        ? box_value(L"Unfreeze")
		                                                        : box_value(L"Freeze"));
}


void KinectToVR::implementation::ConsolePage::FreezeOnlyLowerCheckBox_Checked(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	k2app::K2Settings.freezeLowerOnly = true;
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::ConsolePage::FreezeOnlyLowerCheckBox_Unchecked(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	k2app::K2Settings.freezeLowerOnly = false;
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::ConsolePage::LanguageOptionBox_DropDownOpened(
	const Windows::Foundation::IInspectable& sender,
	const Windows::Foundation::IInspectable& e)
{
	ShellExecuteA(nullptr, nullptr, "https://www.google.com/search?q=learn+english", nullptr, nullptr, SW_SHOW);
}
