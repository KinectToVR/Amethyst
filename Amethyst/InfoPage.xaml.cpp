#include "pch.h"
#include "InfoPage.xaml.h"
#if __has_include("InfoPage.g.cpp")
#include "InfoPage.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

bool info_loadedOnce = false;

namespace winrt::Amethyst::implementation
{
	InfoPage::InfoPage()
	{
		InitializeComponent();

		k2app::shared::teaching_tips::info::endingTeachingTip =
			std::make_shared<Controls::TeachingTip>(EndingTeachingTip());

		LOG(INFO) << "Constructing page with tag: \"info\"...";

		LOG(INFO) << "Registering a detached binary semaphore reload handler for InfoPage...";
		std::thread([&, this]
		{
			while (true)
			{
				// Wait for a reload signal (blocking)
				k2app::shared::semaphores::semaphore_ReloadPage_InfoPage.acquire();

				// Reload & restart the waiting loop
				if (info_loadedOnce)
					k2app::shared::main::thisDispatcherQueue->TryEnqueue([&, this]
					{
						Page_Loaded_Handler();
					});

				Sleep(100); // Sleep a bit
			}
		}).detach();
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


Windows::Foundation::IAsyncAction Amethyst::implementation::InfoPage::EndingTeachingTip_CloseButtonClick(
	const Controls::TeachingTip& sender,
	const Windows::Foundation::IInspectable& args)
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

	k2app::interfacing::isNUXPending = false;

	// Navigate to the general page
	k2app::shared::main::mainNavigationView->SelectedItem(
		k2app::shared::main::mainNavigationView->MenuItems().GetAt(0));
	k2app::shared::main::NavView_Navigate(L"general", Media::Animation::EntranceNavigationTransitionInfo());

	// We're done
	k2app::K2Settings.firstTimeTourShown = true;
	k2app::K2Settings.saveSettings();
}


void Amethyst::implementation::InfoPage::Page_Loaded(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Execute the handler
	Page_Loaded_Handler();

	// Mark as loaded
	info_loadedOnce = true;
}


void Amethyst::implementation::InfoPage::Page_Loaded_Handler()
{
	AppTitle().Text(
		k2app::interfacing::LocalizedJSONString(L"/InfoPage/AppTitle"));

	AppCaption().Text(
		k2app::interfacing::LocalizedJSONString(L"/InfoPage/AppCaption"));

	Credits_Header().Text(
		k2app::interfacing::LocalizedJSONString(L"/InfoPage/Credits/Header"));

	Credits_MainTeam_Title().Text(
		k2app::interfacing::LocalizedJSONString(L"/InfoPage/Credits/MainTeam/Title"));

	Credits_MainTeam_Roles_Akaya().Text(
		k2app::interfacing::LocalizedJSONString(L"/InfoPage/Credits/MainTeam/Roles/Akaya"));

	Credits_MainTeam_Roles_Ella().Text(
		k2app::interfacing::LocalizedJSONString(L"/InfoPage/Credits/MainTeam/Roles/Ella"));

	Credits_MainTeam_Roles_Hekky().Text(
		k2app::interfacing::LocalizedJSONString(L"/InfoPage/Credits/MainTeam/Roles/Hekky"));

	Credits_MainTeam_Roles_Himbeer().Text(
		k2app::interfacing::LocalizedJSONString(L"/InfoPage/Credits/MainTeam/Roles/Himbeer"));

	Credits_MainTeam_Roles_Artemis().Text(
		k2app::interfacing::LocalizedJSONString(L"/InfoPage/Credits/MainTeam/Roles/Artemis"));

	Credits_MainTeam_Roles_Ollie().Text(
		k2app::interfacing::LocalizedJSONString(L"/InfoPage/Credits/MainTeam/Roles/Ollie"));

	Credits_MainTeam_Roles_Aria().Text(
		k2app::interfacing::LocalizedJSONString(L"/InfoPage/Credits/MainTeam/Roles/Aria"));

	Credits_Translators_Title().Text(
		k2app::interfacing::LocalizedJSONString(L"/InfoPage/Credits/Translators/Title"));

	Credits_Helpers_Title().Text(
		k2app::interfacing::LocalizedJSONString(L"/InfoPage/Credits/Helpers/Title"));

	Credits_Community().Text(
		k2app::interfacing::LocalizedJSONString(L"/InfoPage/Credits/Community"));
}
