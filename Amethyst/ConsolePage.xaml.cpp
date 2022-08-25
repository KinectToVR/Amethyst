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

		LOG(INFO) << "Constructing page with tag: \"console\"...";
	}
}


void Amethyst::implementation::ConsolePage::ConsolePage_Loaded(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	LOG(INFO) << "Ohhhhh, How Sweeet!";
	k2app::interfacing::currentAppState = L"okashi";
}


void Amethyst::implementation::ConsolePage::DevicesCrashButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	exit(-12);
}


void Amethyst::implementation::ConsolePage::OpenVRCrashButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	exit(-11);
}


void Amethyst::implementation::ConsolePage::HRESULTCrashButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	check_hresult(FALSE);
}


void Amethyst::implementation::ConsolePage::DelegateCrashButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	throw hresult_illegal_delegate_assignment();
}


void Amethyst::implementation::ConsolePage::NullCrashButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// This is only a sample, null pointer exceptions
	//      actually happen sometimes, e.g. if an element
	//      in UI is referenced before even constructing it

	int* _pointer = nullptr;
	*_pointer = 420;
}


void Amethyst::implementation::ConsolePage::GuideButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	k2app::shared::main::interfaceBlockerGrid->Opacity(0.35);
	k2app::shared::main::interfaceBlockerGrid->IsHitTestVisible(true);

	k2app::shared::teaching_tips::main::initializerTeachingTip->IsOpen(true);
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


void Amethyst::implementation::ConsolePage::UpdateButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	k2app::shared::main::flyoutHeader->Text(k2app::interfacing::LocalizedResourceWString(
		L"SharedStrings", L"Updates/NewUpdateFound"));

	k2app::shared::main::flyoutFooter->Text(L"Amethyst v{}.{}.{}.{}");
	k2app::shared::main::flyoutContent->Text(L"Forced by the user");
	
	k2app::shared::main::flyoutContent->Margin({0,0,0,12});

	k2app::shared::main::installLaterButton->Visibility(Visibility::Visible);
	k2app::shared::main::installNowButton->Visibility(Visibility::Visible);

	k2app::shared::main::updateIconDot->Opacity(1.0);

	Controls::Primitives::FlyoutShowOptions options;
	options.Placement(Controls::Primitives::FlyoutPlacementMode::RightEdgeAlignedBottom);
	options.ShowMode(Controls::Primitives::FlyoutShowMode::Transient);

	k2app::shared::main::updateFlyout->ShowAt(*k2app::shared::main::helpButton, options);
}
