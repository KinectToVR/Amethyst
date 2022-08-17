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

		k2app::shared::teaching_tips::info::endingTeachingTip =
			std::make_shared<Controls::TeachingTip>(EndingTeachingTip());

		LOG(INFO) << "Constructing page with tag: \"info\"...";
	}

	void InfoPage::K2DoubleTapped(
		const Windows::Foundation::IInspectable& sender,
		const Input::DoubleTappedRoutedEventArgs& e)
	{
		k2app::interfacing::ShowToast(
			L"お可愛いこと❢", L"(P.S. Congratulations!)");

		// Play a sound
		playAppSound(k2app::interfacing::sounds::AppSounds::Show);

		k2app::shared::main::consoleItem.get()->Visibility(Visibility::Visible);
	}
}


void Amethyst::implementation::InfoPage::Grid_Loaded(
	const Windows::Foundation::IInspectable& sender,
	const RoutedEventArgs& e)
{
	// The info page was loaded
	LOG(INFO) << "Re/Loading page with tag: \"info\"... (Child)";
	k2app::interfacing::currentAppState = L"info";
}


Windows::Foundation::IAsyncAction winrt::Amethyst::implementation::InfoPage::EndingTeachingTip_CloseButtonClick(
	const winrt::Microsoft::UI::Xaml::Controls::TeachingTip& sender,
	const winrt::Windows::Foundation::IInspectable& args)
{
	// Dismiss the current tip
	EndingTeachingTip().IsOpen(false);

	// Wait a bit
	{
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(200);
		co_await ui_thread;
	}

	// Unblock the interface
	k2app::shared::main::interfaceBlockerGrid->Opacity(0.0);
	k2app::shared::main::interfaceBlockerGrid->IsHitTestVisible(false);

	// Navigate to the general page
	k2app::shared::main::mainNavigationView->SelectedItem(
		k2app::shared::main::mainNavigationView->MenuItems().GetAt(0));
	k2app::shared::main::NavView_Navigate(L"general", Media::Animation::EntranceNavigationTransitionInfo());

	// We're done
	k2app::K2Settings.firstTimeTourShown = true;
	k2app::K2Settings.saveSettings();
}
