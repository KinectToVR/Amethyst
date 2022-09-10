#include "pch.h"
#include "MainWindow.xaml.h"

#include <codecvt>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Windows.System.h>
#include <dispatcherqueue.h>
#include <numeric>

#include "App.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

// Helper local variables
HANDLE hNamedMutex = nullptr;
bool main_localInitFinished = false,
     main_loadedOnce = false;

// Notifications stuff
event_token notification_token;

// Assume we're up to date
std::string K2RemoteVersion =
	k2app::interfacing::K2InternalVersion;

// Toast struct (json)
struct toast
{
	std::wstring guid, title, message;
	bool show_always = false;
};

// Updating function
Windows::Foundation::IAsyncAction Amethyst::implementation::MainWindow::executeUpdates()
{
	//k2app::interfacing::updatingNow = true;

	//UpdatePendingFlyout().Hide();

	//// Setup the flyout contents: action & footer
	///*UpdatePendingFlyoutHeader().Text(k2app::interfacing::LocalizedResourceWString(
	//	L"SharedStrings", L"Updates/UpToDate"));
	//UpdatePendingFlyoutFooter().Text(L"Amethyst v" + StringToWString(k2app::interfacing::K2InternalVersion));*/

	//// Hide the new-update icon dot
	//UpdateIconDot().Opacity(0.0);

	//// Sleep on UI (Non-blocking)
	//{
	//	apartment_context ui_thread;
	//	co_await resume_background();
	//	Sleep(500);
	//	co_await ui_thread;
	//}

	//// Mark the update footer as active
	//UpdateIconGrid().Translation({0, 0, 0});
	//UpdateIconText().Opacity(0.0);
	//UpdateIcon().Foreground(*k2app::shared::main::attentionBrush);
	//IconRotation().Begin();

	//// Show the updater progress flyout
	//Controls::Primitives::FlyoutShowOptions options;
	//options.Placement(Controls::Primitives::FlyoutPlacementMode::RightEdgeAlignedBottom);
	//options.ShowMode(Controls::Primitives::FlyoutShowMode::Transient);

	//if (!k2app::interfacing::isNUXPending)
	//	UpdatePendingFlyout().ShowAt(HelpButton(), options);

	//// Success? ...or nah?
	//bool m_update_error = false;

	//// Reset the progressbar
	//auto UpdatePendingFlyoutProgressBar = Controls::ProgressBar();
	//UpdatePendingFlyoutProgressBar.IsIndeterminate(false);
	//UpdatePendingFlyoutProgressBar.HorizontalAlignment(
	//	HorizontalAlignment::Stretch);

	//UpdatePendingFlyoutMainStack().Children().Append(
	//	UpdatePendingFlyoutProgressBar);

	///* Simulate some work being done */
	//for (uint32_t i = 0; i <= 100; i++)
	//{
	//	UpdatePendingFlyoutProgressBar.Value(i);
	//	UpdatePendingFlyoutStatusContent().Text(
	//		L"Downlodign - " + std::to_wstring(i) + L"%");

	//	// Sleep on UI (Non-blocking)
	//	{
	//		apartment_context ui_thread;
	//		co_await resume_background();
	//		Sleep(150);
	//		co_await ui_thread;
	//	}

	//	// Simulate an error
	//	if (i > 60)
	//	{
	//		m_update_error = true;
	//		break;
	//	}
	//}

	//// Mark the update footer as inactive
	//IconRotation().Stop();
	//UpdateIcon().Foreground(*k2app::shared::main::neutralBrush);
	//UpdateIconGrid().Translation({0, -8, 0});
	//UpdateIconText().Opacity(1.0);

	//// Check the file result and the DL result
	//if (!m_update_error)
	//{
	//	// Execute the update
	//	ShellExecuteA(nullptr, nullptr,
	//	              "https://github.com/KinectToVR/Amethyst-Releases/releases/latest",
	//	              nullptr, nullptr, SW_SHOW);
	//}
	//else
	//{
	//	// Play a sound
	//	playAppSound(k2app::interfacing::sounds::AppSounds::Error);

	//	UpdatePendingFlyoutProgressBar.ShowError(true);
	//	UpdatePendingFlyoutStatusContent().Text(L"Erraa downlodign!");

	//	// Sleep on UI (Non-blocking)
	//	{
	//		apartment_context ui_thread;
	//		co_await resume_background();
	//		Sleep(3200);
	//		co_await ui_thread;
	//	}
	//}

	//// Hide the flyout
	//UpdatePendingFlyout().Hide();

	//// Sleep on UI (Non-blocking)
	//{
	//	apartment_context ui_thread;
	//	co_await resume_background();
	//	Sleep(500);
	//	co_await ui_thread;
	//}

	//// Don't give up yet
	//k2app::interfacing::updatingNow = false;
	//if (k2app::interfacing::updateFound)
	//	UpdateIconDot().Opacity(1.0);

	//// Remove the progressbar
	//UpdatePendingFlyoutMainStack().Children().RemoveAtEnd();


	// Execute the update
	ShellExecuteA(nullptr, nullptr,
	              "https://github.com/KinectToVR/Amethyst-Releases/releases/latest",
	              nullptr, nullptr, SW_SHOW);

	co_return;
}

// Updates checking function
Windows::Foundation::IAsyncAction Amethyst::implementation::MainWindow::checkUpdates(
	const bool show, const DWORD delay_ms)
{
	// Attempt only after init
	if (main_localInitFinished)
	{
		// Check if we're midway updating
		if (k2app::interfacing::updatingNow)
		{
			// Show the updater progress flyout
			Controls::Primitives::FlyoutShowOptions options;
			options.Placement(Controls::Primitives::FlyoutPlacementMode::RightEdgeAlignedBottom);
			options.ShowMode(Controls::Primitives::FlyoutShowMode::Transient);

			if (!k2app::interfacing::isNUXPending)
				UpdatePendingFlyout().ShowAt(HelpButton(), options);

			co_return; // Don't proceed further
		}

		// Mark as checking
		k2app::interfacing::checkingUpdatesNow = true;

		{
			// Sleep on UI (Non-blocking)
			apartment_context ui_thread;
			co_await resume_background();
			Sleep(delay_ms);
			co_await ui_thread;
		}

		// Don't check if found
		if (!k2app::interfacing::updateFound)
		{
			// Mark the update footer as active
			{
				UpdateIconGrid().Translation({0, 0, 0});
				UpdateIconText().Opacity(0.0);

				UpdateIcon().Foreground(*k2app::shared::main::attentionBrush);
			}

			// Here check for updates (via external bool)
			IconRotation().Begin();

			// Capture the calling context.
			apartment_context ui_thread;
			co_await resume_background();

			// Check for updates
			auto start_time = std::chrono::high_resolution_clock::now();

			// Check now
			k2app::interfacing::updateFound = false;

			// Dummy for holding change logs
			std::vector<std::wstring> changes_strings_vector;

			// Check for deez updates
			try
			{
				auto client = Windows::Web::Http::HttpClient();

				std::wstring get_release_version,
				             get_docs_languages;

				LOG(INFO) << "Checking for updates... [GET]";

				// Release
				try
				{
					const auto& response = co_await client.GetAsync(
						Windows::Foundation::Uri(
							L"https://github.com/KinectToVR/Amethyst-Releases/releases/latest/download/version"));

					get_release_version = co_await response.Content().ReadAsStringAsync();
				}
				catch (const hresult_error& ex)
				{
					LOG(ERROR) << "Error getting the release info! Message: "
						<< WStringToString(ex.message().c_str());
				}

				LOG(INFO) << "Checking available languages... [GET]";

				// Language
				try
				{
					const auto& response = co_await client.GetAsync(
						Windows::Foundation::Uri(L"https://docs.k2vr.tech/shared/locales.json"));

					get_docs_languages = co_await response.Content().ReadAsStringAsync();
				}
				catch (const hresult_error& ex)
				{
					LOG(ERROR) << "Error getting the language info! Message: "
						<< WStringToString(ex.message().c_str());
				}

				// If the read string isn't empty, proceed to checking for updates
				if (!get_release_version.empty())
				{
					LOG(INFO) << "Update-check successful, string:\n" << WStringToString(get_release_version);

					// Parse the loaded json
					const auto json_root = Windows::Data::Json::JsonObject::Parse(get_release_version);

					if (!json_root.HasKey(L"version_string") || !json_root.HasKey(L"changes"))
						LOG(ERROR) << "The latest release's manifest was invalid!";

					else
					{
						// Find the version tag (it's a string only to save time, trust me...)
						K2RemoteVersion = WStringToString(json_root.GetNamedString(L"version_string").c_str());

						LOG(INFO) << "Local version string: " << k2app::interfacing::K2InternalVersion;
						LOG(INFO) << "Remote version string: " << K2RemoteVersion;

						/* Now split the gathered string into the version number */

						// Split version strings into integers
						std::vector<std::string> local_version_num, remote_version_num;

						using namespace std::string_literals;
						local_version_num = k2app::interfacing::splitStringByDelimiter(
							k2app::interfacing::K2InternalVersion, "."s);
						remote_version_num = k2app::interfacing::splitStringByDelimiter(K2RemoteVersion, "."s);

						// Compare to the current version
						for (uint32_t i = 0; i < 4; i++)
						{
							// Check the version
							if (const auto _ver = std::stoi(remote_version_num.at(i));
								_ver > std::stoi(local_version_num.at(i)))
								k2app::interfacing::updateFound = true;

								// Not to false-alarm in situations like 1.0.1.0 (local) vs 1.0.0.1 (remote)
							else if (_ver < std::stoi(local_version_num.at(i))) break;
						}

						// Cache the changes
						for (auto change_entry : json_root.GetNamedArray(L"changes"))
							changes_strings_vector.push_back(change_entry.GetString().c_str());

						// Thanks to this chad: https://stackoverflow.com/a/45123408
						// Now check for push notifications aka toasts

						// !show checks if the update handler was run automatically:
						//       (by design) it's false only when run at the startup
						if (json_root.HasKey(L"toasts") && !show)
						{
							// Scan for all toasts & push them to a table
							std::map<std::wstring, toast> toasts;

							for (auto v : json_root.GetNamedArray(L"toasts"))
							{
								auto node = v.GetObject();
								toast tst;

								tst.guid = node.GetNamedString(L"guid", L"");
								tst.title = node.GetNamedString(L"title", L"");
								tst.message = node.GetNamedString(L"message", L"");
								tst.show_always = node.GetNamedBoolean(L"show_always", false);

								// Add or replace (won't allow duplicates)
								toasts.insert_or_assign(tst.guid, tst);
							}

							// Iterate over all the found toasts
							for (auto& itr : toasts)
							{
								// Log everything
								LOG(INFO) << "Found a toast with:\n    guid: " << WStringToString(itr.first) <<
									"\n    title: " << WStringToString(itr.second.title) << "\n    message: " <<
									WStringToString(itr.second.message) << "\n    which needs to show " <<
									(itr.second.show_always ? "always" : "once");

								// Check if this toast hasn't already been shown, and opt show it
								if (itr.second.show_always ||
									std::ranges::find(k2app::K2Settings.shownToastsGuidVector, itr.first)
									== k2app::K2Settings.shownToastsGuidVector.end())
								{
									// Log it
									LOG(INFO) << "Showing toast with guid " << WStringToString(itr.first) << " now...";

									// Show the toast (and optionally cache it)
									k2app::interfacing::ShowToast(itr.second.title, itr.second.message);

									k2app::interfacing::ShowVRToast(itr.second.title, itr.second.message);

									// If the toast isn't meant to be shown always, cache it
									if (!itr.second.show_always)
										k2app::K2Settings.shownToastsGuidVector.push_back(itr.first);
								}
							}
						}
					}
				}
				else
					LOG(ERROR) << "Update-check failed, the string was empty.";

				if (!get_docs_languages.empty())
				{
					// Parse the loaded json
					const auto json_root = Windows::Data::Json::JsonObject::Parse(get_docs_languages);

					// Check if the resource root is fine & the language code exists
					k2app::interfacing::docsLanguageCode = k2app::K2Settings.appLanguage;

					if (json_root.Size() <= 0 || !json_root.HasKey(k2app::interfacing::docsLanguageCode))
					{
						LOG(INFO) << "Docs do not contain a language with code \"" <<
							WStringToString(k2app::interfacing::docsLanguageCode) <<
							"\", falling back to \"en\" (English)!";

						k2app::interfacing::docsLanguageCode = L"en";
					}
				}
				else
					LOG(ERROR) << "Language-check failed, the string was empty.";
			}
			catch (const hresult_error& ex)
			{
				LOG(ERROR) << "Update failed, an exception occurred."
					<< " Message: " << WStringToString(ex.message().c_str());
			}
			catch (...)
			{
				LOG(ERROR) << "Update failed, an exception occurred.";
			}

			// Limit time to (min) 1s
			if (auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now() - start_time).count(); duration <= 1000.f)
				std::this_thread::sleep_for(std::chrono::milliseconds(1000 - duration));

			// Resume to UI and stop the animation
			co_await ui_thread;
			IconRotation().Stop();

			// Mark the update footer as inactive
			{
				UpdateIcon().Foreground(*k2app::shared::main::neutralBrush);

				UpdateIconGrid().Translation({0, -8, 0});
				UpdateIconText().Opacity(1.0);
			}

			if (k2app::interfacing::updateFound)
			{
				FlyoutHeader().Text(k2app::interfacing::LocalizedResourceWString(
					L"SharedStrings", L"Updates/NewUpdateFound"));
				FlyoutFooter().Text(L"Amethyst v" + StringToWString(K2RemoteVersion));

				std::wstring changelog_string;
				for (const auto& str : changes_strings_vector)
					changelog_string += L"- " + str + L'\n';

				if (changelog_string.length() > 0)
					changelog_string.pop_back(); // Remove the last \n
				FlyoutContent().Text(changelog_string);

				FlyoutContent().Margin({0, 0, 0, 12});

				InstallLaterButton().Visibility(Visibility::Visible);
				InstallNowButton().Visibility(Visibility::Visible);

				UpdateIconDot().Opacity(1.0);
			}
			else
			{
				FlyoutHeader().Text(k2app::interfacing::LocalizedResourceWString(
					L"SharedStrings", L"Updates/UpToDate"));
				FlyoutFooter().Text(L"Amethyst v" + StringToWString(k2app::interfacing::K2InternalVersion));
				FlyoutContent().Text(k2app::interfacing::LocalizedResourceWString(
					L"SharedStrings", L"Updates/Suggestions"));

				FlyoutContent().Margin({0, 0, 0, 0});

				InstallLaterButton().Visibility(Visibility::Collapsed);
				InstallNowButton().Visibility(Visibility::Collapsed);

				UpdateIconDot().Opacity(0.0);
			}
		}

		Controls::Primitives::FlyoutShowOptions options;
		options.Placement(Controls::Primitives::FlyoutPlacementMode::RightEdgeAlignedBottom);
		options.ShowMode(Controls::Primitives::FlyoutShowMode::Transient);

		// If an update was found, show it
		// (or if the cheack was manual)
		if ((k2app::interfacing::updateFound || show) && !k2app::interfacing::isNUXPending)
			UpdateFlyout().ShowAt(HelpButton(), options);

		// Uncheck
		k2app::interfacing::checkingUpdatesNow = false;
	}
}

Windows::System::DispatcherQueueController CreateSystemDispatcherQueueController()
{
	DispatcherQueueOptions options
	{
		sizeof(DispatcherQueueOptions),
		DQTYPE_THREAD_CURRENT,
		DQTAT_COM_NONE
	};

	ABI::Windows::System::IDispatcherQueueController* ptr{nullptr};
	check_hresult(CreateDispatcherQueueController(options, &ptr));
	return {ptr, take_ownership_from_abi};
}

winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropTheme ConvertToSystemBackdropTheme(
	const ElementTheme& theme)
{
	switch (theme)
	{
	case ElementTheme::Dark:
		return winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropTheme::Dark;
	case ElementTheme::Light:
		return winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropTheme::Light;
	default:
		return winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropTheme::Default;
	}
}

namespace winrt::Amethyst::implementation
{
	Microsoft::UI::Composition::SystemBackdrops::SystemBackdropConfiguration m_configuration{nullptr};
	Microsoft::UI::Composition::SystemBackdrops::MicaController m_micaController{nullptr};
	Window::Activated_revoker m_activatedRevoker;
	Window::Closed_revoker m_closedRevoker;
	Microsoft::UI::Xaml::FrameworkElement::ActualThemeChanged_revoker m_themeChangedRevoker;
	Microsoft::UI::Xaml::FrameworkElement m_rootElement{nullptr};
	Windows::System::DispatcherQueueController m_dispatcherQueueController{nullptr};

	MainWindow::MainWindow()
	{
		InitializeComponent();

		// Set up mica controllers
		if (Microsoft::UI::Composition::SystemBackdrops::MicaController::IsSupported())
		{
			// Log it!
			LOG(INFO) << "You're using Windows 11, great! Amethyst will look more chad";

			// We ensure that there is a Windows.System.DispatcherQueue on the current thread.
			// Always check if one already exists before attempting to create a new one.
			if (nullptr == Windows::System::DispatcherQueue::GetForCurrentThread() &&
				nullptr == m_dispatcherQueueController)
			{
				m_dispatcherQueueController = CreateSystemDispatcherQueueController();
			}

			// Setup the SystemBackdropConfiguration object.
			{
				m_configuration = Microsoft::UI::Composition::SystemBackdrops::SystemBackdropConfiguration();

				// Activation state.
				m_activatedRevoker = this->Activated(
					auto_revoke,
					[&](auto&&, const Microsoft::UI::Xaml::WindowActivatedEventArgs& args)
					{
						m_configuration.IsInputActive(
							WindowActivationState::Deactivated
							!= args.WindowActivationState());
					});

				// Initial state.
				m_configuration.IsInputActive(true);

				// Application theme.
				m_rootElement = this->Content().try_as<Microsoft::UI::Xaml::FrameworkElement>();
				if (nullptr != m_rootElement)
				{
					m_themeChangedRevoker =
						m_rootElement.ActualThemeChanged(auto_revoke,
						                                 [&](auto&&, auto&&)
						                                 {
							                                 m_configuration.Theme(
								                                 ConvertToSystemBackdropTheme(
									                                 m_rootElement.ActualTheme()));
						                                 });

					// Initial state.
					m_configuration.Theme(
						ConvertToSystemBackdropTheme(m_rootElement.ActualTheme()));
				}
			}

			// Setup Mica on the current Window.
			m_micaController = Microsoft::UI::Composition::SystemBackdrops::MicaController();
			m_micaController.SetSystemBackdropConfiguration(m_configuration);
			m_micaController.AddSystemBackdropTarget(
				this->try_as<Microsoft::UI::Composition::ICompositionSupportsSystemBackdrop>());

			// Change the window background to support mica
			XMainGrid().Background(Media::SolidColorBrush(Microsoft::UI::Colors::Transparent()));
		}
		else
		{
			// No Mica support.
			LOG(INFO) << "You're using Windows 10, bruh... Amethyst won't have cool Mica effects";
		}

		// Cache needed UI elements
		k2app::shared::teaching_tips::main::initializerTeachingTip =
			std::make_shared<Controls::TeachingTip>(InitializerTeachingTip());

		k2app::shared::main::mainNavigationView = std::make_shared<Controls::NavigationView>(NavView());

		k2app::shared::main::appTitleLabel = std::make_shared<Controls::TextBlock>(AppTitleLabel());
		k2app::shared::main::flyoutHeader = std::make_shared<Controls::TextBlock>(FlyoutHeader());
		k2app::shared::main::flyoutFooter = std::make_shared<Controls::TextBlock>(FlyoutFooter());
		k2app::shared::main::flyoutContent = std::make_shared<Controls::TextBlock>(FlyoutContent());

		k2app::shared::main::interfaceBlockerGrid = std::make_shared<Controls::Grid>(InterfaceBlockerGrid());
		k2app::shared::main::navigationBlockerGrid = std::make_shared<Controls::Grid>(NavigationBlockerGrid());

		k2app::shared::main::mainContentFrame = std::make_shared<Controls::Frame>(ContentFrame());

		k2app::shared::main::updateIconDot = std::make_shared<Controls::FontIcon>(UpdateIconDot());

		k2app::shared::main::updateFlyout = std::make_shared<Controls::Flyout>(UpdateFlyout());

		k2app::shared::main::installNowButton = std::make_shared<Controls::Button>(InstallNowButton());
		k2app::shared::main::installLaterButton = std::make_shared<Controls::Button>(InstallLaterButton());

		k2app::shared::main::generalItem = std::make_shared<Controls::NavigationViewItem>(GeneralItem());
		k2app::shared::main::settingsItem = std::make_shared<Controls::NavigationViewItem>(SettingsItem());
		k2app::shared::main::devicesItem = std::make_shared<Controls::NavigationViewItem>(DevicesItem());
		k2app::shared::main::infoItem = std::make_shared<Controls::NavigationViewItem>(InfoItem());
		k2app::shared::main::consoleItem = std::make_shared<Controls::NavigationViewItem>(ConsoleItem());
		k2app::shared::main::helpButton = std::make_shared<Controls::NavigationViewItem>(HelpButton());

		/*k2app::shared::main::navigation_items::navViewDevicesButtonIconCanvas =
		    std::make_shared<Controls::Canvas>(NavViewDevicesButtonIconCanvas());

		k2app::shared::main::navigation_items::navViewDevicesButtonIcon_Empty =
			std::make_shared<Shapes::Path>(NavViewDevicesButtonIcon_Empty());
		k2app::shared::main::navigation_items::navViewDevicesButtonIcon_Solid =
			std::make_shared<Shapes::Path>(NavViewDevicesButtonIcon_Solid());*/

		k2app::shared::main::navigation_items::navViewGeneralButtonIcon = std::make_shared<Controls::FontIcon>(
			NavViewGeneralButtonIcon());
		k2app::shared::main::navigation_items::navViewSettingsButtonIcon = std::make_shared<Controls::FontIcon>(
			NavViewSettingsButtonIcon());
		k2app::shared::main::navigation_items::navViewDevicesButtonIcon = std::make_shared<Controls::FontIcon>(
			NavViewDevicesButtonIcon());
		k2app::shared::main::navigation_items::navViewInfoButtonIcon = std::make_shared<Controls::FontIcon>(
			NavViewInfoButtonIcon());
		k2app::shared::main::navigation_items::navViewOkashiButtonIcon = std::make_shared<Controls::FontIcon>(
			NavViewOkashiButtonIcon());

		k2app::shared::main::navigation_items::navViewGeneralButtonLabel = std::make_shared<Controls::TextBlock>(
			NavViewGeneralButtonLabel());
		k2app::shared::main::navigation_items::navViewSettingsButtonLabel = std::make_shared<Controls::TextBlock>(
			NavViewSettingsButtonLabel());
		k2app::shared::main::navigation_items::navViewDevicesButtonLabel = std::make_shared<Controls::TextBlock>(
			NavViewDevicesButtonLabel());
		k2app::shared::main::navigation_items::navViewInfoButtonLabel = std::make_shared<Controls::TextBlock>(
			NavViewInfoButtonLabel());
		k2app::shared::main::navigation_items::navViewOkashiButtonLabel = std::make_shared<Controls::TextBlock>(
			NavViewOkashiButtonLabel());

		// Set up
		this->Title(L"Amethyst");

		LOG(INFO) << "Extending the window titlebar...";
		this->ExtendsContentIntoTitleBar(true);
		this->SetTitleBar(DragElement());

		// Set titlebar/taskview icon
		LOG(INFO) << "Setting the App Window icon...";
		this->try_as<IWindowNative>()->get_WindowHandle(
			&k2app::shared::main::thisAppWindowID);

		Microsoft::UI::Windowing::AppWindow::GetFromWindowId(
			Microsoft::UI::GetWindowIdFromWindow(k2app::shared::main::thisAppWindowID)).SetIcon(
			(k2app::interfacing::GetProgramLocation().parent_path() / "Assets" / "ktvr.ico").c_str());

		LOG(INFO) << "Making the app window available for children views...";
		k2app::shared::main::thisAppWindow = std::make_shared<Window>(this->try_as<Window>());

		LOG(INFO) << "Making the app dispatcher available for children...";
		k2app::shared::main::thisDispatcherQueue =
			std::make_shared<Microsoft::UI::Dispatching::DispatcherQueue>(DispatcherQueue());

		{
			using namespace Microsoft::Windows::AppNotifications;
			using namespace Microsoft::Windows::AppLifecycle;

			LOG(INFO) << "Registering for NotificationInvoked WinRT event...";

			// To ensure all Notification handling happens in this process instance, register for
			// NotificationInvoked before calling Register(). Without this a new process will
			// be launched to handle the notification.
			notification_token = AppNotificationManager::Default()
				.NotificationInvoked([&, this](const auto&,
				                               const AppNotificationActivatedEventArgs& notificationActivatedEventArgs)
				{
					k2app::interfacing::ProcessToastArguments(notificationActivatedEventArgs);
				});

			LOG(INFO) << "Creating the default notification manager...";
			k2app::shared::main::thisNotificationManager =
				std::make_shared<AppNotificationManager>(AppNotificationManager::Default());

			LOG(INFO) << "Registering the notification manager...";
			k2app::shared::main::thisNotificationManager.get()->Register();
		}

		LOG(INFO) << "Creating and registering the default resource manager...";
		k2app::shared::main::thisResourceManager =
			std::make_shared<Microsoft::Windows::ApplicationModel::Resources::ResourceManager>(
				Microsoft::Windows::ApplicationModel::Resources::ResourceManager(L"resources.pri"));

		LOG(INFO) << "Creating and registering the default resource context...";
		k2app::shared::main::thisResourceContext =
			std::make_shared<Microsoft::Windows::ApplicationModel::Resources::ResourceContext>(
				k2app::shared::main::thisResourceManager.get()->CreateResourceContext());

		LOG(INFO) << "Pushing control pages to window...";
		k2app::shared::main::m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"general", winrt::xaml_typename<GeneralPage>()));
		k2app::shared::main::m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"settings", winrt::xaml_typename<SettingsPage>()));
		k2app::shared::main::m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"devices", winrt::xaml_typename<DevicesPage>()));
		k2app::shared::main::m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"info", winrt::xaml_typename<InfoPage>()));
		k2app::shared::main::m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"console", winrt::xaml_typename<ConsolePage>()));

		LOG(INFO) << "Registering a detached binary semaphore reload handler for MainWindow...";
		std::thread([&, this]
		{
			while (true)
			{
				// Wait for a reload signal (blocking)
				k2app::shared::semaphores::semaphore_ReloadPage_MainWindow.acquire();

				// Reload & restart the waiting loop
				if (main_loadedOnce)
					k2app::shared::main::thisDispatcherQueue->TryEnqueue([&, this]
					{
						XMainGrid_Loaded_Handler();

						// Rebuild devices' settings
						// (Trick the device into rebuilding its interface)
						for (size_t index = 0; index < TrackingDevices::TrackingDevicesVector.size(); index++)
						{
							LOG(INFO) << "Rebuilding device[" << index <<
								"]'s layout root...";

							const auto pLayoutRoot = new
								k2app::interfacing::AppInterface::AppLayoutRoot();

							switch (const auto& device =
								TrackingDevices::TrackingDevicesVector.at(
									index); device.index())
							{
							case 0:
								{
									const auto& pDevice =
										std::get<
											ktvr::K2TrackingDeviceBase_KinectBasis*>(
											device);

									// Register the layout
									pDevice->layoutRoot = dynamic_cast<
											ktvr::Interface::LayoutRoot*>
										(pLayoutRoot);

									// State that everything's fine and the device's loaded
									// Note: the dispatcher is starting AFTER device setup
									pDevice->onLoad();
								}
								break;
							case 1:
								{
									const auto& pDevice =
										std::get<
											ktvr::K2TrackingDeviceBase_JointsBasis*>(
											device);

									// Register the layout
									pDevice->layoutRoot = dynamic_cast<
											ktvr::Interface::LayoutRoot*>
										(pLayoutRoot);

									// State that everything's fine and the device's loaded
									// Note: the dispatcher is starting AFTER device setup
									pDevice->onLoad();
								}
								break;
							}

							LOG(INFO) <<
								"Appending the device[" << index <<
								"]'s layout root to the global registry...";

							// Push the device's layout root to pointers' vector
							TrackingDevices::TrackingDevicesLayoutRootsVector.
								at(index) = pLayoutRoot;
						}
					});

				Sleep(100); // Sleep a bit
			}
		}).detach();

		LOG(INFO) << "~~~Amethyst new logging session begins here!~~~";

		LOG(INFO) << "Registering a named mutex for com_kinecttovr_amethyst...";

		hNamedMutex = CreateMutexA(nullptr, TRUE, "com_kinecttovr_amethyst");
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{
			LOG(ERROR) << "Startup failed! The app is already running.";

			if (exists(
				k2app::interfacing::GetProgramLocation().parent_path() / "K2CrashHandler" / "K2CrashHandler.exe"))
			{
				std::thread([]
				{
					ShellExecute(nullptr, L"open",
					             (k2app::interfacing::GetProgramLocation().parent_path() /
						             L"K2CrashHandler" / L"K2CrashHandler.exe ")
					             .wstring().c_str(), L"already_running", nullptr, SW_SHOWDEFAULT);
				}).detach();
			}
			else
				LOG(WARNING) << "Crash handler exe (./K2CrashHandler/K2CrashHandler.exe) not found!";

			Sleep(3000);
			exit(0); // Exit peacefully
		}

		// Priority: Register the app exit handler
		LOG(INFO) << "Registering an exit handler for the app window...";
		this->Closed([&](const IInspectable& window, const WindowEventArgs& e)
		-> Windows::Foundation::IAsyncAction
			{
				// Handled(true) means Cancel()
				// and Handled(false) means Continue()
				// -> Block exiting until we're done
				e.Handled(true);

				// Handle all the exit actions (if needed)
				// Show the close tip (if not shown yet)
				if (!k2app::interfacing::isExitHandled &&
					!k2app::K2Settings.firstShutdownTipShown)
				{
					ShutdownTeachingTip().IsOpen(true);

					k2app::K2Settings.firstShutdownTipShown = true;
					k2app::K2Settings.saveSettings(); // Save settings

					co_return;
				}

				if (k2app::interfacing::updateOnClosed)
					co_await executeUpdates();

				if (!k2app::interfacing::isExitHandled)
				{
					// Shut down the mica controller
					if (nullptr != m_micaController)
					{
						m_micaController.Close();
						m_micaController = nullptr;
					}

					// Shut down the dispatcher
					if (nullptr != m_dispatcherQueueController)
					{
						m_dispatcherQueueController.ShutdownQueueAsync();
						m_dispatcherQueueController = nullptr;
					}

					// Handle the exit actions
					k2app::interfacing::handle_app_exit_n();
				}

				// Cleanup event handler
				k2app::shared::main::thisNotificationManager.get()->NotificationInvoked(notification_token);

				// Call Unregister() before exiting main so that subsequent invocations will launch a new process
				k2app::shared::main::thisNotificationManager.get()->Unregister();

				// Flush the log cleaner
				google::FlushLogFiles(google::GLOG_INFO);

				// Finally allow exits
				e.Handled(false);
			});

		// Priority: Launch the crash handler
		LOG(INFO) << "Starting the crash handler passing the app PID...";

		if (exists(k2app::interfacing::GetProgramLocation().parent_path() / "K2CrashHandler" / "K2CrashHandler.exe"))
		{
			std::thread([]
			{
				ShellExecute(nullptr, L"open",
				             (k2app::interfacing::GetProgramLocation().parent_path() /
					             L"K2CrashHandler" / L"K2CrashHandler.exe ")
				             .wstring().c_str(),
				             (std::to_wstring(GetCurrentProcessId()) +
					             L" \"" + k2app::interfacing::thisLogDestination + L"\"").c_str(),
				             nullptr,
				             SW_SHOWDEFAULT);
			}).detach();
		}
		else
			LOG(WARNING) << "Crash handler exe (./K2CrashHandler/K2CrashHandler.exe) not found!";

		// Priority: Connect to OpenVR
		if (!k2app::interfacing::OpenVRStartup())
		{
			LOG(ERROR) << "Could not connect to OpenVR! The app will be shut down.";
			k2app::interfacing::_fail(-11); // OpenVR is critical, so exit
		}

		// Priority: Set up Amethyst as a vr app
		LOG(INFO) << "Installing the vr application manifest...";
		k2app::interfacing::installApplicationManifest();

		// Priority: Set up VR Input Actions
		if (!k2app::interfacing::EVRActionsStartup())
			LOG(ERROR) << "Could not set up VR Input Actions! The app will lack some functionality.";

		// Priority: Set up the K2API & Server
		static std::thread serverStatusThread(k2app::interfacing::K2ServerDriverSetup);
		serverStatusThread.detach();

		// Start the main loop
		std::thread(k2app::main::K2MainLoop).detach();

		// Disable internal sounds
		ElementSoundPlayer::State(ElementSoundPlayerState::Off);

		// Scan for tracking devices
		std::thread([&]
			{
				LOG(INFO) << "Searching for tracking devices...";
				LOG(INFO) << "Current path is: " << WStringToString(
					k2app::interfacing::GetProgramLocation().parent_path().wstring());

				if (exists(k2app::interfacing::GetProgramLocation().parent_path() / "devices"))
				{
					for (auto entry : std::filesystem::directory_iterator(
						     k2app::interfacing::GetProgramLocation().parent_path() / "devices"))
					{
						if (exists(entry.path() / "device.k2devicemanifest"))
						{
							// Load the JSON source into buffer
							std::wifstream wif(entry.path() / L"device.k2devicemanifest");
							wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
							std::wstringstream wss;
							wss << wif.rdbuf();

							// Parse the loaded json
							const auto json_root = Windows::Data::Json::JsonObject::Parse(wss.str());

							if (!json_root.HasKey(L"device_name") || !json_root.HasKey(L"device_type"))
							{
								LOG(ERROR) << WStringToString(entry.path().stem().wstring()) <<
									"'s manifest was invalid!";
								continue;
							}

							std::wstring device_name = json_root.GetNamedString(L"device_name").c_str(),
							             device_type = json_root.GetNamedString(L"device_type").c_str();

							LOG(INFO) << "Found tracking device with:\n - name: " << WStringToString(device_name);

							auto deviceDllPath = entry.path() / L"bin" / L"win64" /
								(L"device_" + device_name + L".dll");

							if (exists(deviceDllPath))
							{
								LOG(INFO) << "Found the device's driver dll, now checking dependencies...";

								bool _found = true; // assume success

								// Check for deez dlls
								if (json_root.HasKey(L"linked_dll_path"))
									for (auto dll_entry : json_root.GetNamedArray(L"linked_dll_path"))
										if (!std::filesystem::exists(dll_entry.GetString().c_str()))
										{
											_found = false; // Mark as failed
											LOG(ERROR) << "Linked dll not found at path: " <<
												WStringToString(dll_entry.GetString().c_str());
										}

								// Else continue
								if (_found)
								{
									LOG(INFO) << "Found the device's dependency dll, now loading...";

									HINSTANCE hLibraryInstance;
									BOOL fRunTimeLinkSuccess = FALSE;

									// Get a handle to the DLL module.
									hLibraryInstance = LoadLibraryExW(deviceDllPath.wstring().c_str(), nullptr,
									                                  LOAD_LIBRARY_SEARCH_DEFAULT_DIRS |
									                                  // Add device's folder to dll search path
									                                  LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR |
									                                  LOAD_LIBRARY_SEARCH_SYSTEM32);

									if (hLibraryInstance != nullptr)
									{
										auto hDeviceFactory = (TrackingDevices::TrackingDeviceBaseFactory)
											GetProcAddress(hLibraryInstance, "TrackingDeviceBaseFactory");

										// If the function address is valid, call the function.
										if (nullptr != hDeviceFactory)
										{
											fRunTimeLinkSuccess = TRUE;
											LOG(INFO) << "Device library loaded, now checking interface...";

											int returnCode = ktvr::K2InitError_Invalid;
											std::wstring stat = L"Something's wrong!\nE_UKNOWN\nWhat's happened here?";
											std::string _name = "E_UNKNOWN"; // Placeholder
											bool blocks_flip = false, supports_math = true;

											if (wcscmp(device_type.c_str(), L"KinectBasis") == 0)
											{
												auto pDevice =
													static_cast<ktvr::K2TrackingDeviceBase_KinectBasis*>(
														(hDeviceFactory)(ktvr::IAME_API_Version, &returnCode));

												if (returnCode == ktvr::K2InitError_None)
												{
													LOG(INFO) << "Interface version OK, now constructing...";

													LOG(INFO) << "Overriding device's helper functions...";

													// Push helper functions to the device
													pDevice->getHMDPose =
														k2app::interfacing::plugins::plugins_getHMDPose;
													pDevice->getHMDPoseCalibrated =
														k2app::interfacing::plugins::plugins_getHMDPoseCalibrated;
													pDevice->getHMDOrientationYaw =
														k2app::interfacing::plugins::plugins_getHMDOrientationYaw;
													pDevice->getHMDOrientationYawCalibrated =
														k2app::interfacing::plugins::plugins_getHMDOrientationYawCalibrated;

													pDevice->getLeftControllerPose =
														k2app::interfacing::plugins::plugins_getLeftControllerPose;
													pDevice->getLeftControllerPoseCalibrated =
														k2app::interfacing::plugins::plugins_getLeftControllerPoseCalibrated;
													pDevice->getRightControllerPose =
														k2app::interfacing::plugins::plugins_getRightControllerPose;
													pDevice->getRightControllerPoseCalibrated =
														k2app::interfacing::plugins::plugins_getRightControllerPoseCalibrated;

													pDevice->getAppJointPoses =
														k2app::interfacing::plugins::plugins_getAppJointPoses;

													pDevice->requestLanguageCode =
														k2app::interfacing::plugins::plugins_requestLanguageCode;
													pDevice->requestLocalizedString =
														k2app::interfacing::plugins::plugins_requestLocalizedString;

													pDevice->requestStatusUIRefresh =
														k2app::interfacing::plugins::plugins_requestStatusUIRefresh;

													pDevice->CreateTextBlock =
														k2app::interfacing::AppInterface::CreateAppTextBlock_Sliced;
													pDevice->CreateButton =
														k2app::interfacing::AppInterface::CreateAppButton_Sliced;
													pDevice->CreateNumberBox =
														k2app::interfacing::AppInterface::CreateAppNumberBox_Sliced;
													pDevice->CreateComboBox =
														k2app::interfacing::AppInterface::CreateAppComboBox_Sliced;
													pDevice->CreateCheckBox =
														k2app::interfacing::AppInterface::CreateAppCheckBox_Sliced;
													pDevice->CreateToggleSwitch =
														k2app::interfacing::AppInterface::CreateAppToggleSwitch_Sliced;
													pDevice->CreateTextBox =
														k2app::interfacing::AppInterface::CreateAppTextBox_Sliced;
													pDevice->CreateProgressRing =
														k2app::interfacing::AppInterface::CreateAppProgressRing_Sliced;
													pDevice->CreateProgressBar =
														k2app::interfacing::AppInterface::CreateAppProgressBar_Sliced;

													LOG(INFO) << "Appending the device to the global registry...";

													// Push the device to pointers' vector
													TrackingDevices::TrackingDevicesVector.push_back(pDevice);

													// Cache the name
													_name = pDevice->getDeviceName();

													// Create a layout root for the device and override
													const uint32_t _last_device_index =
														TrackingDevices::TrackingDevicesVector.size() - 1;
													k2app::shared::main::thisDispatcherQueue.get()->TryEnqueue(
														[_last_device_index, this]
														{
															LOG(INFO) << "Registering device[" << _last_device_index <<
																"]'s layout root...";

															const auto pLayoutRoot = new
																k2app::interfacing::AppInterface::AppLayoutRoot();

															switch (const auto& device =
																TrackingDevices::TrackingDevicesVector.at(
																	_last_device_index); device.index())
															{
															case 0:
																{
																	const auto& pDevice =
																		std::get<
																			ktvr::K2TrackingDeviceBase_KinectBasis*>(
																			device);

																	// Register the layout
																	pDevice->layoutRoot = dynamic_cast<
																			ktvr::Interface::LayoutRoot*>
																		(pLayoutRoot);
																	
																	// State that everything's fine and the device's loaded
																	// Note: the dispatcher is starting AFTER device setup
																	pDevice->onLoad();
																}
																break;
															case 1:
																{
																	const auto& pDevice =
																		std::get<
																			ktvr::K2TrackingDeviceBase_JointsBasis*>(
																			device);

																	// Register the layout
																	pDevice->layoutRoot = dynamic_cast<
																			ktvr::Interface::LayoutRoot*>
																		(pLayoutRoot);
																	
																	// State that everything's fine and the device's loaded
																	// Note: the dispatcher is starting AFTER device setup
																	pDevice->onLoad();
																}
																break;
															}

															LOG(INFO) <<
																"Appending the device[" << _last_device_index <<
																"]'s layout root to the global registry...";

															// Push the device's layout root to pointers' vector
															TrackingDevices::TrackingDevicesLayoutRootsVector.
																push_back(pLayoutRoot);
														});

													stat = pDevice->statusResultWString(
														pDevice->getStatusResult());

													blocks_flip = !pDevice->isFlipSupported();
													supports_math = pDevice->isAppOrientationSupported();
												}
											}
											else if (wcscmp(device_type.c_str(), L"JointsBasis") == 0)
											{
												auto pDevice =
													static_cast<ktvr::K2TrackingDeviceBase_JointsBasis*>(
														(hDeviceFactory)(ktvr::IAME_API_Version, &returnCode));

												if (returnCode == ktvr::K2InitError_None)
												{
													LOG(INFO) << "Interface version OK, now constructing...";

													LOG(INFO) << "Overriding device's helper functions...";

													// Push helper functions to the device
													pDevice->getHMDPose =
														k2app::interfacing::plugins::plugins_getHMDPose;
													pDevice->getHMDPoseCalibrated =
														k2app::interfacing::plugins::plugins_getHMDPoseCalibrated;
													pDevice->getHMDOrientationYaw =
														k2app::interfacing::plugins::plugins_getHMDOrientationYaw;
													pDevice->getHMDOrientationYawCalibrated =
														k2app::interfacing::plugins::plugins_getHMDOrientationYawCalibrated;

													pDevice->getLeftControllerPose =
														k2app::interfacing::plugins::plugins_getLeftControllerPose;
													pDevice->getLeftControllerPoseCalibrated =
														k2app::interfacing::plugins::plugins_getLeftControllerPoseCalibrated;
													pDevice->getRightControllerPose =
														k2app::interfacing::plugins::plugins_getRightControllerPose;
													pDevice->getRightControllerPoseCalibrated =
														k2app::interfacing::plugins::plugins_getRightControllerPoseCalibrated;

													pDevice->getAppJointPoses =
														k2app::interfacing::plugins::plugins_getAppJointPoses;

													pDevice->requestLanguageCode =
														k2app::interfacing::plugins::plugins_requestLanguageCode;
													pDevice->requestLocalizedString =
														k2app::interfacing::plugins::plugins_requestLocalizedString;

													pDevice->requestStatusUIRefresh =
														k2app::interfacing::plugins::plugins_requestStatusUIRefresh;

													pDevice->CreateTextBlock =
														k2app::interfacing::AppInterface::CreateAppTextBlock_Sliced;
													pDevice->CreateButton =
														k2app::interfacing::AppInterface::CreateAppButton_Sliced;
													pDevice->CreateNumberBox =
														k2app::interfacing::AppInterface::CreateAppNumberBox_Sliced;
													pDevice->CreateComboBox =
														k2app::interfacing::AppInterface::CreateAppComboBox_Sliced;
													pDevice->CreateCheckBox =
														k2app::interfacing::AppInterface::CreateAppCheckBox_Sliced;
													pDevice->CreateToggleSwitch =
														k2app::interfacing::AppInterface::CreateAppToggleSwitch_Sliced;
													pDevice->CreateTextBox =
														k2app::interfacing::AppInterface::CreateAppTextBox_Sliced;
													pDevice->CreateProgressRing =
														k2app::interfacing::AppInterface::CreateAppProgressRing_Sliced;
													pDevice->CreateProgressBar =
														k2app::interfacing::AppInterface::CreateAppProgressBar_Sliced;

													LOG(INFO) << "Appending the device to the global registry...";

													// Push the device to pointers' vector
													TrackingDevices::TrackingDevicesVector.push_back(pDevice);

													// Cache the name
													_name = pDevice->getDeviceName();

													// Create a layout root for the device and override
													const uint32_t _last_device_index =
														TrackingDevices::TrackingDevicesVector.size() - 1;
													k2app::shared::main::thisDispatcherQueue.get()->TryEnqueue(
														[_last_device_index, this]
														{
															LOG(INFO) << "Registering device[" << _last_device_index <<
																"]'s layout root...";

															const auto pLayoutRoot = new
																k2app::interfacing::AppInterface::AppLayoutRoot();

															switch (const auto& device =
																TrackingDevices::TrackingDevicesVector.at(
																	_last_device_index); device.index())
															{
															case 0:
																{
																	const auto& pDevice =
																		std::get<
																			ktvr::K2TrackingDeviceBase_KinectBasis*>(
																			device);

																	// Register the layout
																	pDevice->layoutRoot = dynamic_cast<
																			ktvr::Interface::LayoutRoot*>
																		(pLayoutRoot);
																	
																	// State that everything's fine and the device's loaded
																	// Note: the dispatcher is starting AFTER device setup
																	pDevice->onLoad();
																}
																break;
															case 1:
																{
																	const auto& pDevice =
																		std::get<
																			ktvr::K2TrackingDeviceBase_JointsBasis*>(
																			device);

																	// Register the layout
																	pDevice->layoutRoot = dynamic_cast<
																			ktvr::Interface::LayoutRoot*>
																		(pLayoutRoot);
																	
																	// State that everything's fine and the device's loaded
																	// Note: the dispatcher is starting AFTER device setup
																	pDevice->onLoad();
																}
																break;
															}

															LOG(INFO) <<
																"Appending the device[" << _last_device_index <<
																"]'s layout root to the global registry...";

															// Push the device's layout root to pointers' vector
															TrackingDevices::TrackingDevicesLayoutRootsVector.
																push_back(pLayoutRoot);
														});

													stat = pDevice->statusResultWString(
														pDevice->getStatusResult());

													blocks_flip = true; // Always the same for JointsBasis
													supports_math = false; // Always the same for JointsBasis
												}
											}
											else if (wcscmp(device_type.c_str(), L"Spectator") == 0)
											{
												auto pDevice =
													static_cast<ktvr::K2TrackingDeviceBase_Spectator*>(
														(hDeviceFactory)(ktvr::IAME_API_Version, &returnCode));

												if (returnCode == ktvr::K2InitError_None)
												{
													LOG(INFO) << "Interface version OK, now constructing...";

													// Push helper functions to the device
													pDevice->getHMDPose =
														k2app::interfacing::plugins::plugins_getHMDPose;
													pDevice->getHMDPoseCalibrated =
														k2app::interfacing::plugins::plugins_getHMDPoseCalibrated;
													pDevice->getHMDOrientationYaw =
														k2app::interfacing::plugins::plugins_getHMDOrientationYaw;
													pDevice->getHMDOrientationYawCalibrated =
														k2app::interfacing::plugins::plugins_getHMDOrientationYawCalibrated;

													pDevice->getLeftControllerPose =
														k2app::interfacing::plugins::plugins_getLeftControllerPose;
													pDevice->getLeftControllerPoseCalibrated =
														k2app::interfacing::plugins::plugins_getLeftControllerPoseCalibrated;
													pDevice->getRightControllerPose =
														k2app::interfacing::plugins::plugins_getRightControllerPose;
													pDevice->getRightControllerPoseCalibrated =
														k2app::interfacing::plugins::plugins_getRightControllerPoseCalibrated;

													pDevice->getAppJointPoses =
														k2app::interfacing::plugins::plugins_getAppJointPoses;

													pDevice->requestLanguageCode =
														k2app::interfacing::plugins::plugins_requestLanguageCode;
													pDevice->requestLocalizedString =
														k2app::interfacing::plugins::plugins_requestLocalizedString;

													// State that everything's fine and the device's loaded
													// Note: the dispatcher is starting AFTER device setup
													pDevice->onLoad();

													LOG(INFO) << "A Spectator device's been added successfully!";
													continue; // Don't do any more jobs
												}
											}

											switch (returnCode)
											{
											case ktvr::K2InitError_None:
												{
													LOG(INFO) << "Registered tracking device with:"
														"\n - name: " << WStringToString(device_name) <<
														"\n - type: " << WStringToString(device_type) <<
														"\n - blocks flip: " << blocks_flip <<
														"\n - supports math-based orientation: " << supports_math <<

														"\nat index " <<
														TrackingDevices::TrackingDevicesVector.size() - 1;

													LOG(INFO) << "Device status (should be 'not initialized'): \n[\n" <<
														WStringToString(stat) << "\n]\n";

													// Switch check the device name
													if (_name == k2app::K2Settings.trackingDeviceName)
													{
														LOG(INFO) << "This device is the main device!";
														k2app::K2Settings.trackingDeviceID =
															TrackingDevices::TrackingDevicesVector.size() - 1;
													}
													else if (_name == k2app::K2Settings.overrideDeviceName)
													{
														LOG(INFO) << "This device is an override device!";
														k2app::K2Settings.overrideDeviceID =
															TrackingDevices::TrackingDevicesVector.size() - 1;
													}
												}
												break;
											case ktvr::K2InitError_BadInterface:
												{
													LOG(ERROR) <<
														"Device's interface is incompatible with current Amethyst API "
														<<
														ktvr::IAME_API_Version <<
														", it's probably outdated.";
												}
												break;
											case ktvr::K2InitError_Invalid:
												{
													LOG(ERROR) <<
														"Device either didn't give any return code or it's factory malfunctioned. You can only cry about it...";
												}
												break;
											}
										}
										else
										{
											LOG(ERROR) <<
												"Device's interface is incompatible with current Amethyst API " <<
												ktvr::IAME_API_Version <<
												", it's probably outdated.";
										}
									}
									else
										LOG(ERROR) << "There was an error linking with the device library!";

									// If unable to call the DLL function, use an alternative.
									if (!fRunTimeLinkSuccess)
										LOG(ERROR) << "There was an error calling the device factory...";
								}
								else
									LOG(ERROR) << "Device's dependency dll (external linked dll) was not found!";
							}
							else
								LOG(ERROR) << "Device's driver dll (bin/win64/device_[device].dll) was not found!";
						}
						else
						{
							LOG(ERROR) << WStringToString(entry.path().stem().wstring()) <<
								"'s manifest was not found :/";
						}
					}

					LOG(INFO) << "Registration of tracking devices has ended, there are " <<
						TrackingDevices::TrackingDevicesVector.size() <<
						" tracking devices in total.";

					// Now select the proper device
					// k2app::K2Settings.trackingDeviceName must be read from settings before!
					if (TrackingDevices::TrackingDevicesVector.size() > 0)
					{
						// Loop over all the devices and select the saved one
						// : Has been done at loading!

						// Check the base device index
						if (k2app::K2Settings.trackingDeviceID >= TrackingDevices::TrackingDevicesVector.size())
						{
							LOG(INFO) << "Previous tracking device ID was too big, it's been reset to 0";
							k2app::K2Settings.trackingDeviceID = 0; // Select the first one
						}

						// Init the device (base)
						const auto& trackingDevice =
							TrackingDevices::TrackingDevicesVector.at(k2app::K2Settings.trackingDeviceID);
						switch (trackingDevice.index())
						{
						case 0:
							// Kinect Basis
							{
								// Update options for the device
								if (k2app::K2Settings.K2TrackersVector[1].orientationTrackingOption ==
									k2app::k2_SoftwareCalculatedRotation &&
									!std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->
									isAppOrientationSupported())
									k2app::K2Settings.K2TrackersVector[1].orientationTrackingOption =
										k2app::k2_DeviceInferredRotation;

								if (k2app::K2Settings.K2TrackersVector[2].orientationTrackingOption ==
									k2app::k2_SoftwareCalculatedRotation &&
									!std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->
									isAppOrientationSupported())
									k2app::K2Settings.K2TrackersVector[2].orientationTrackingOption =
										k2app::k2_DeviceInferredRotation;

								// Init
								std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->initialize();

								// Backup the name
								k2app::K2Settings.trackingDeviceName = 
									std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->getDeviceName();
							}
							break;
						case 1:
							// Joints Basis
							{
								// Update options for the device
								if (k2app::K2Settings.K2TrackersVector[1].orientationTrackingOption ==
									k2app::k2_SoftwareCalculatedRotation)
									k2app::K2Settings.K2TrackersVector[1].orientationTrackingOption =
										k2app::k2_DeviceInferredRotation;

								if (k2app::K2Settings.K2TrackersVector[2].orientationTrackingOption ==
									k2app::k2_SoftwareCalculatedRotation)
									k2app::K2Settings.K2TrackersVector[2].orientationTrackingOption =
										k2app::k2_DeviceInferredRotation;

								k2app::K2Settings.isFlipEnabled = false;

								// Init
								std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice)->initialize();

								// Backup the name
								k2app::K2Settings.trackingDeviceName =
									std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice)->getDeviceName();
							}
							break;
						}

						// Check the override device index
						if (k2app::K2Settings.overrideDeviceID >= TrackingDevices::TrackingDevicesVector.size())
						{
							LOG(INFO) << "Previous tracking device ID was too big, it's been reset to [none]";
							k2app::K2Settings.overrideDeviceID = -1; // Select [none]
							k2app::K2Settings.overrideDeviceName = "";
						}

						// Init the device (override, optionally)
						if (k2app::K2Settings.overrideDeviceID > -1 &&
							k2app::K2Settings.overrideDeviceID != k2app::K2Settings.trackingDeviceID)
						{
							const auto& overrideDevice =
								TrackingDevices::TrackingDevicesVector.at(k2app::K2Settings.overrideDeviceID);
							switch (overrideDevice.index())
							{
							case 0:
								// Kinect Basis
								std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(overrideDevice)->initialize();
								break;
							case 1:
								// Joints Basis
								std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(overrideDevice)->initialize();
								break;
							}
						}
						else 
						{
							k2app::K2Settings.overrideDeviceID = -1; // Set to NONE
							k2app::K2Settings.overrideDeviceName = "";
						}

						// Second check and try after 3 seconds
						std::thread([&]
						{
							// Wait a moment
							std::this_thread::sleep_for(std::chrono::seconds(3));

							// Init the device (optionally this time)
							// Base
							{
								switch (const auto& _trackingDevice =
										TrackingDevices::TrackingDevicesVector.at(
											k2app::K2Settings.trackingDeviceID);
									_trackingDevice.index())
								{
								case 0:
									// Kinect Basis
									if (!std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_trackingDevice)->
										isInitialized())
										std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_trackingDevice)->
											initialize();
									break;
								case 1:
									// Joints Basis
									if (!std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_trackingDevice)->
										isInitialized())
										std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_trackingDevice)->
											initialize();
									break;
								}
							}
							// Override
							if (k2app::K2Settings.overrideDeviceID > -1 &&
								k2app::K2Settings.overrideDeviceID != k2app::K2Settings.trackingDeviceID)
							{
								switch (const auto& _trackingDevice =
										TrackingDevices::TrackingDevicesVector.at(
											k2app::K2Settings.overrideDeviceID);
									_trackingDevice.index())
								{
								case 0:
									// Kinect Basis
									if (!std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_trackingDevice)->
										isInitialized())
										std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(_trackingDevice)->
											initialize();
									break;
								case 1:
									// Joints Basis
									if (!std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_trackingDevice)->
										isInitialized())
										std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(_trackingDevice)->
											initialize();
									break;
								}
							}
							else 
							{
								k2app::K2Settings.overrideDeviceID = -1; // Set to NONE
								k2app::K2Settings.overrideDeviceName = "";
							}
						}).detach();

						// Update the UI
						k2app::shared::main::thisDispatcherQueue.get()->TryEnqueue([&, this]
						{
							TrackingDevices::updateTrackingDeviceUI();
							TrackingDevices::updateOverrideDeviceUI();
						});
					}
					else // Log and exit, we have nothing to do
					{
						LOG(ERROR) << "No proper tracking devices (K2Devices) found :/";
						k2app::interfacing::_fail(-12); // -12 is for NO_DEVICES
					}
				}
				else // Log and exit, we have nothing to do
				{
					LOG(ERROR) << "No tracking devices (K2Devices) found :/";
					k2app::interfacing::_fail(-12);
				}
			}
		).join(); // Now this would be in background but to spare bugs, we're gonna wait

		// Notify of the setup end
		main_localInitFinished = true;
	}
}

void Amethyst::implementation::MainWindow::NavView_Loaded(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	k2app::interfacing::actualTheme = NavView().ActualTheme();

	k2app::shared::main::attentionBrush =
		k2app::interfacing::actualTheme == ElementTheme::Dark
			? std::make_shared<Media::SolidColorBrush>(
				Application::Current().Resources().TryLookup(
					box_value(L"AttentionBrush_Dark"
					)).as<Media::SolidColorBrush>())
			: std::make_shared<Media::SolidColorBrush>(
				Application::Current().Resources().TryLookup(
					box_value(L"AttentionBrush_Light"
					)).as<Media::SolidColorBrush>());

	k2app::shared::main::neutralBrush =
		k2app::interfacing::actualTheme == ElementTheme::Dark
			? std::make_shared<Media::SolidColorBrush>(
				Application::Current().Resources().TryLookup(
					box_value(L"NeutralBrush_Dark"
					)).as<Media::SolidColorBrush>())
			: std::make_shared<Media::SolidColorBrush>(
				Application::Current().Resources().TryLookup(
					box_value(L"NeutralBrush_Light"
					)).as<Media::SolidColorBrush>());

	// NavView doesn't load any page by default, so load home page.
	NavView().SelectedItem(NavView().MenuItems().GetAt(0));

	// If navigation occurs on SelectionChanged, then this isn't needed.
	// Because we use ItemInvoked to navigate, we need to call Navigate
	// here to load the home page.
	k2app::shared::main::NavView_Navigate(
		L"general", Media::Animation::EntranceNavigationTransitionInfo());

	// Append placeholder text to the dummy layout root
	k2app::interfacing::emptyLayoutRoot =
		new k2app::interfacing::AppInterface::AppLayoutRoot();

	k2app::interfacing::emptyLayoutRoot->AppendSingleElement(
		k2app::interfacing::AppInterface::CreateAppTextBlock_Sliced(
			L"In the beginning was the Word."),
		ktvr::Interface::SingleLayoutHorizontalAlignment::Left);

	k2app::interfacing::emptyLayoutRoot->AppendSingleElement(
		k2app::interfacing::AppInterface::CreateAppTextBlock_Sliced(
			L"  But this device's settings are something else..."),
		ktvr::Interface::SingleLayoutHorizontalAlignment::Left);
}

void Amethyst::implementation::MainWindow::NavView_ItemInvoked(
	const Controls::NavigationView& sender,
	const Controls::NavigationViewItemInvokedEventArgs& args)
{
	k2app::shared::main::NavView_Navigate(
		winrt::unbox_value_or<hstring>(
			args.InvokedItemContainer().Tag(), L"").c_str(),
		args.RecommendedNavigationTransitionInfo());
}

void k2app::shared::main::NavView_Navigate(std::wstring navItemTag,
                                           const Media::Animation::NavigationTransitionInfo& transitionInfo)
{
	LOG(INFO) << "Navigation requested! Page tag: " << WStringToString(navItemTag);

	Windows::UI::Xaml::Interop::TypeName pageTypeName;
	for (auto&& eachPage : m_pages)
	{
		if (eachPage.first == navItemTag)
		{
			pageTypeName = eachPage.second;
			break;
		}
	}

	// Get the page type before navigation so you can prevent duplicate
	// entries in the backstack.
	Windows::UI::Xaml::Interop::TypeName prevNavPageType =
		mainContentFrame->CurrentSourcePageType();

	// Navigate only if the selected page isn't currently loaded.
	if (pageTypeName.Name != L"" && prevNavPageType.Name != pageTypeName.Name)
	{
		playAppSound(interfacing::sounds::AppSounds::Invoke);

		// Switch bring back the current navview item to the base state
		if (!prevNavPageType.Name.empty())
		{
			if (prevNavPageType.Name == L"Amethyst.GeneralPage")
			{
				navigation_items::navViewGeneralButtonIcon->Translation({0, -8, 0});
				navigation_items::navViewGeneralButtonLabel->Opacity(1.0);

				navigation_items::navViewGeneralButtonIcon->Foreground(*neutralBrush);
				navigation_items::navViewGeneralButtonIcon->Glyph(L"\uE80F");
			}
			else if (prevNavPageType.Name == L"Amethyst.SettingsPage")
			{
				navigation_items::navViewSettingsButtonIcon->Translation({0, -8, 0});
				navigation_items::navViewSettingsButtonLabel->Opacity(1.0);

				navigation_items::navViewSettingsButtonIcon->Foreground(*neutralBrush);
				navigation_items::navViewSettingsButtonIcon->Glyph(L"\uE713");
			}
			else if (prevNavPageType.Name == L"Amethyst.DevicesPage")
			{
				navigation_items::navViewDevicesButtonIcon->Translation({0, -8, 0});
				navigation_items::navViewDevicesButtonLabel->Opacity(1.0);

				navigation_items::navViewDevicesButtonIcon->Foreground(*neutralBrush);

				navigation_items::navViewDevicesButtonIcon->Glyph(L"\uF158");
				navigation_items::navViewDevicesButtonIcon->FontSize(20);
			}
			else if (prevNavPageType.Name == L"Amethyst.InfoPage")
			{
				navigation_items::navViewInfoButtonIcon->Translation({0, -8, 0});
				navigation_items::navViewInfoButtonLabel->Opacity(1.0);

				navigation_items::navViewInfoButtonIcon->Foreground(*neutralBrush);
				navigation_items::navViewInfoButtonIcon->Glyph(L"\uE946");
			}
			else if (prevNavPageType.Name == L"Amethyst.ConsolePage")
			{
				navigation_items::navViewOkashiButtonIcon->Translation({0, -8, 0});
				navigation_items::navViewOkashiButtonLabel->Opacity(1.0);

				navigation_items::navViewOkashiButtonIcon->Foreground(*neutralBrush);
				navigation_items::navViewOkashiButtonIcon->Glyph(L"\uEB51");
			}
		}

		// Switch the next navview item to the active state
		if (!pageTypeName.Name.empty())
		{
			if (pageTypeName.Name == L"Amethyst.GeneralPage")
			{
				navigation_items::navViewGeneralButtonIcon->Glyph(L"\uEA8A");
				navigation_items::navViewGeneralButtonIcon->Foreground(*attentionBrush);

				navigation_items::navViewGeneralButtonLabel->Opacity(0.0);
				navigation_items::navViewGeneralButtonIcon->Translation({0, 0, 0});
			}
			else if (pageTypeName.Name == L"Amethyst.SettingsPage")
			{
				navigation_items::navViewSettingsButtonIcon->Glyph(L"\uF8B0");
				navigation_items::navViewSettingsButtonIcon->Foreground(*attentionBrush);

				navigation_items::navViewSettingsButtonLabel->Opacity(0.0);
				navigation_items::navViewSettingsButtonIcon->Translation({0, 0, 0});
			}
			else if (pageTypeName.Name == L"Amethyst.DevicesPage")
			{
				navigation_items::navViewDevicesButtonLabel->Opacity(0.0);
				navigation_items::navViewDevicesButtonIcon->Translation({0, 0, 0});

				navigation_items::navViewDevicesButtonIcon->Foreground(*attentionBrush);

				navigation_items::navViewDevicesButtonIcon->Glyph(L"\uEBD2");
				navigation_items::navViewDevicesButtonIcon->FontSize(23);
			}
			else if (pageTypeName.Name == L"Amethyst.InfoPage")
			{
				navigation_items::navViewInfoButtonIcon->Glyph(L"\uF167");
				navigation_items::navViewInfoButtonIcon->Foreground(*attentionBrush);

				navigation_items::navViewInfoButtonLabel->Opacity(0.0);
				navigation_items::navViewInfoButtonIcon->Translation({0, 0, 0});
			}
			else if (pageTypeName.Name == L"Amethyst.ConsolePage")
			{
				navigation_items::navViewOkashiButtonIcon->Glyph(L"\uEB52");
				navigation_items::navViewOkashiButtonIcon->Foreground(*attentionBrush);

				navigation_items::navViewOkashiButtonLabel->Opacity(0.0);
				navigation_items::navViewOkashiButtonIcon->Translation({0, 0, 0});
			}
		}

		interfacing::currentPageTag = navItemTag; // Cache the current page tag
		interfacing::currentPageClass = pageTypeName.Name; // Cache the current page tag

		mainContentFrame->Navigate(pageTypeName, nullptr, transitionInfo);
	}
}

void Amethyst::implementation::MainWindow::ContentFrame_NavigationFailed(
	const Windows::Foundation::IInspectable& sender,
	const Navigation::NavigationFailedEventArgs& e)
{
	throw hresult_error(
		E_FAIL, hstring(L"Failed to load Page ") + e.SourcePageType().Name);
}

void Amethyst::implementation::MainWindow::NavView_BackRequested(
	const Controls::NavigationView& sender,
	const Controls::NavigationViewBackRequestedEventArgs& args)
{
	TryGoBack();
}

void Amethyst::implementation::MainWindow::CoreDispatcher_AcceleratorKeyActivated(
	const Windows::UI::Core::CoreDispatcher& /* sender */,
	const Windows::UI::Core::AcceleratorKeyEventArgs& args)
{
	// When Alt+Left are pressed navigate back
	if (args.EventType() == Windows::UI::Core::CoreAcceleratorKeyEventType::SystemKeyDown
		&& args.VirtualKey() == Windows::System::VirtualKey::Left
		&& args.KeyStatus().IsMenuKeyDown
		&& !args.Handled())
	{
		args.Handled(TryGoBack());
	}
}

void Amethyst::implementation::MainWindow::CoreWindow_PointerPressed(
	const Windows::UI::Core::CoreWindow& /* sender */,
	const Windows::UI::Core::PointerEventArgs& args)
{
	// Handle mouse back button.
	if (args.CurrentPoint().Properties().IsXButton1Pressed())
	{
		args.Handled(TryGoBack());
	}
}

void Amethyst::implementation::MainWindow::System_BackRequested(
	const Windows::Foundation::IInspectable& /* sender */,
	const Windows::UI::Core::BackRequestedEventArgs& args)
{
	if (!args.Handled())
	{
		args.Handled(TryGoBack());
	}
}

bool Amethyst::implementation::MainWindow::TryGoBack()
{
	if (!ContentFrame().CanGoBack())
		return false;
	// Don't go back if the nav pane is overlayed.
	if (NavView().IsPaneOpen() &&
		(NavView().DisplayMode() == muxc::NavigationViewDisplayMode::Compact ||
			NavView().DisplayMode() == muxc::NavigationViewDisplayMode::Minimal))
		return false;
	ContentFrame().GoBack();
	return true;
}

void Amethyst::implementation::MainWindow::On_Navigated(
	const Windows::Foundation::IInspectable& /* sender */,
	const Windows::UI::Xaml::Navigation::NavigationEventArgs& args)
{
	NavView().IsBackEnabled(ContentFrame().CanGoBack());

	if (ContentFrame().SourcePageType().Name != L"")
	{
		for (auto&& eachPage : k2app::shared::main::m_pages)
		{
			if (eachPage.second.Name == args.SourcePageType().Name)
			{
				for (auto&& eachMenuItem : NavView().MenuItems())
				{
					auto navigationViewItem =
						eachMenuItem.try_as<muxc::NavigationViewItem>();
					{
						if (navigationViewItem)
						{
							auto hstringValue =
								winrt::unbox_value_or<hstring>(
									navigationViewItem.Tag(), L"");
							if (hstringValue == eachPage.first)
							{
								NavView().SelectedItem(navigationViewItem);
								NavView().Header(navigationViewItem.Content());
							}
						}
					}
				}
				break;
			}
		}
	}
}

Windows::UI::Xaml::Controls::Primitives::Popup GetPopup()
{
	auto popups =
		Windows::UI::Xaml::Media::VisualTreeHelper::GetOpenPopups(Windows::UI::Xaml::Window::Current());
	if (popups.Size() > 0)
		return popups.GetAt(0);
	return nullptr;
}

void Amethyst::implementation::MainWindow::InstallLaterButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	k2app::interfacing::updateOnClosed = true;

	UpdateFlyout().Hide();
}

void Amethyst::implementation::MainWindow::InstallNowButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	executeUpdates();

	UpdateFlyout().Hide();
}


void Amethyst::implementation::MainWindow::ExitButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	k2app::interfacing::handle_app_exit();
}


void Amethyst::implementation::MainWindow::MinimizeButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Minimize with win+down
	INPUT inputs[4] = {};
	ZeroMemory(inputs, sizeof(inputs));

	inputs[0].type = INPUT_KEYBOARD;
	inputs[0].ki.wVk = VK_LWIN;

	inputs[1].type = INPUT_KEYBOARD;
	inputs[1].ki.wVk = VK_DOWN;

	inputs[2].type = INPUT_KEYBOARD;
	inputs[2].ki.wVk = VK_DOWN;
	inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

	inputs[3].type = INPUT_KEYBOARD;
	inputs[3].ki.wVk = VK_LWIN;
	inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

	SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}


Windows::Foundation::IAsyncAction Amethyst::implementation::MainWindow::UpdateButton_Loaded(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Show the startup tour teachingtip
	//if (!k2app::K2Settings.firstTimeTourShown) // TODO ENABLE SOMEDAY
	//{
	//	k2app::shared::main::interfaceBlockerGrid->Opacity(0.35);
	//	k2app::shared::main::interfaceBlockerGrid->IsHitTestVisible(true);

	//	k2app::shared::teaching_tips::main::initializerTeachingTip->IsOpen(true);

	//  k2app::interfacing::isNUXPending = true;
	//}

	// Check for updates (and show)
	co_await checkUpdates(false, 2000);
}


Windows::Foundation::IAsyncAction Amethyst::implementation::MainWindow::UpdateButton_Tapped(
	const Windows::Foundation::IInspectable& sender,
	const Input::TappedRoutedEventArgs& e)
{
	// Check for updates (and show)
	if (!k2app::interfacing::checkingUpdatesNow)
	{
		playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);
		co_await checkUpdates(true);
	}
	else co_return;
}


void k2app::interfacing::handle_app_exit(const uint32_t& p_sleep_millis)
{
	// Mark exiting as true
	isExitingNow = true;
	LOG(INFO) << "AppWindow.Closing handler called, starting the shutdown routine...";

	// Mark trackers as inactive
	K2AppTrackersInitialized = false;

	// Wait a moment & exit
	LOG(INFO) << "Shutdown actions completed, " <<
		"disconnecting devices and exiting in " << p_sleep_millis << "ms...";
	Sleep(p_sleep_millis); // Sleep a bit for a proper server disconnect

	// Close the multi-process mutex
	[&]
	{
		__try
		{
			ReleaseMutex(hNamedMutex); // Explicitly release mutex
			CloseHandle(hNamedMutex); // Close handle before terminating
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			[&]
			{
				LOG(INFO) <<
					"Shutting down: com_kinecttovr_amethyst named mutex close failed! The app may misbehave.";
			}();
		}
	}();

	// We've (mostly) done what we had to
	isExitHandled = true;

	// Disconnect all tracking devices and don't care about any errors
	[&]
	{
		try
		{
			for (auto& trackingDevice : TrackingDevices::TrackingDevicesVector)
			{
				LOG(INFO) << "Now disconnecting the tracking device...";

				if (trackingDevice.index() == 0)
				{
					// Kinect Basis
					const auto& device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);
					device->shutdown();
				}
				else if (trackingDevice.index() == 1)
				{
					// Joints Basis
					const auto& device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
					device->shutdown();
				}
			}
		}
		catch (...)
		{
		}
	}();
}


void Amethyst::implementation::MainWindow::InitializerTeachingTip_ActionButtonClick(
	const Controls::TeachingTip& sender,
	const Windows::Foundation::IInspectable& args)
{
	// Dismiss the current tip
	k2app::shared::teaching_tips::main::initializerTeachingTip->IsOpen(false);

	// Just dismiss the tip
	k2app::shared::main::interfaceBlockerGrid->Opacity(0.0);
	k2app::shared::main::interfaceBlockerGrid->IsHitTestVisible(false);

	k2app::interfacing::isNUXPending = false;
}


void Amethyst::implementation::MainWindow::InitializerTeachingTip_CloseButtonClick(
	const Controls::TeachingTip& sender,
	const Windows::Foundation::IInspectable& args)
{
	// Dismiss the current tip
	k2app::shared::teaching_tips::main::initializerTeachingTip->IsOpen(false);

	// Navigate to the general page
	k2app::shared::main::mainNavigationView->
		SelectedItem(k2app::shared::main::mainNavigationView->MenuItems().GetAt(0));
	k2app::shared::main::NavView_Navigate(L"general", Media::Animation::EntranceNavigationTransitionInfo());

	// Show the next tip (general page)
	k2app::shared::teaching_tips::general::toggleTrackersTeachingTip->TailVisibility(
		Controls::TeachingTipTailVisibility::Collapsed);
	k2app::shared::teaching_tips::general::toggleTrackersTeachingTip->IsOpen(true);
}


void Amethyst::implementation::MainWindow::HelpButton_Tapped(
	const Windows::Foundation::IInspectable& sender,
	const Input::TappedRoutedEventArgs& e)
{
	// Change the docs button's text

	// General Page
	if (k2app::interfacing::currentAppState == L"general")
	{
		HelpFlyoutDocsButton().Text(
			k2app::interfacing::LocalizedResourceWString(
				L"SharedStrings",
				L"Buttons/Help/Docs/GeneralPage/Overview"));
	}
	else if (k2app::interfacing::currentAppState == L"calibration")
	{
		HelpFlyoutDocsButton().Text(
			k2app::interfacing::LocalizedResourceWString(
				L"SharedStrings",
				L"Buttons/Help/Docs/GeneralPage/Calibration/Main"));
	}
	else if (k2app::interfacing::currentAppState == L"calibration_auto")
	{
		HelpFlyoutDocsButton().Text(
			k2app::interfacing::LocalizedResourceWString(
				L"SharedStrings",
				L"Buttons/Help/Docs/GeneralPage/Calibration/Automatic"));
	}
	else if (k2app::interfacing::currentAppState == L"calibration_manual")
	{
		HelpFlyoutDocsButton().Text(
			k2app::interfacing::LocalizedResourceWString(
				L"SharedStrings",
				L"Buttons/Help/Docs/GeneralPage/Calibration/Manual"));
	}
	else if (k2app::interfacing::currentAppState == L"offsets")
	{
		HelpFlyoutDocsButton().Text(
			k2app::interfacing::LocalizedResourceWString(
				L"SharedStrings",
				L"Buttons/Help/Docs/GeneralPage/Offsets"));
	}

	// Settings Page
	else if (k2app::interfacing::currentAppState == L"settings")
	{
		HelpFlyoutDocsButton().Text(
			k2app::interfacing::LocalizedResourceWString(
				L"SharedStrings",
				L"Buttons/Help/Docs/SettingsPage/Overview"));
	}

	// Devices Page
	else if (k2app::interfacing::currentAppState == L"devices")
	{
		HelpFlyoutDocsButton().Text(
			k2app::interfacing::LocalizedResourceWString(
				L"SharedStrings",
				L"Buttons/Help/Docs/DevicesPage/Overview"));
	}
	else if (k2app::interfacing::currentAppState == L"overrides")
	{
		HelpFlyoutDocsButton().Text(
			k2app::interfacing::LocalizedResourceWString(
				L"SharedStrings",
				L"Buttons/Help/Docs/DevicesPage/Overrides"));
	}

	// Info Page
	else if (k2app::interfacing::currentAppState == L"info")
	{
		HelpFlyoutDocsButton().Text(
			k2app::interfacing::LocalizedResourceWString(
				L"SharedStrings",
				L"Buttons/Help/Docs/InfoPage/OpenCollective"));
	}

	// Okashi Page
	else if (k2app::interfacing::currentAppState == L"okashi")
	{
		HelpFlyoutDocsButton().Text(L"\u2753\u2754\u2753\u2754\u2753\u2754");
	}

	// Show the help flyout
	Controls::Primitives::FlyoutShowOptions options;
	options.Placement(Controls::Primitives::FlyoutPlacementMode::RightEdgeAlignedBottom);
	options.ShowMode(Controls::Primitives::FlyoutShowMode::Transient);

	HelpFlyout().ShowAt(HelpButton(), options);
}


void Amethyst::implementation::MainWindow::HelpFlyoutDocsButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// General Page
	if (k2app::interfacing::currentAppState == L"general")
	{
		ShellExecuteW(nullptr, nullptr,
		              std::format(L"https://docs.k2vr.tech/{}/", k2app::K2Settings.appLanguage).c_str(),
		              nullptr, nullptr, SW_SHOW);
	}
	else if (k2app::interfacing::currentAppState == L"calibration")
	{
		ShellExecuteW(nullptr, nullptr,
		              std::format(L"https://docs.k2vr.tech/{}/calibration/", k2app::K2Settings.appLanguage).c_str(),
		              nullptr, nullptr, SW_SHOW);
	}
	else if (k2app::interfacing::currentAppState == L"calibration_auto")
	{
		ShellExecuteW(nullptr, nullptr,
		              std::format(L"https://docs.k2vr.tech/{}/calibration/#3", k2app::K2Settings.appLanguage).c_str(),
		              nullptr, nullptr, SW_SHOW);
	}
	else if (k2app::interfacing::currentAppState == L"calibration_manual")
	{
		ShellExecuteW(nullptr, nullptr,
		              std::format(L"https://docs.k2vr.tech/{}/calibration/#6", k2app::K2Settings.appLanguage).c_str(),
		              nullptr, nullptr, SW_SHOW);
	}
	else if (k2app::interfacing::currentAppState == L"offsets")
	{
		ShellExecuteW(nullptr, nullptr,
		              std::format(L"https://docs.k2vr.tech/{}/", k2app::K2Settings.appLanguage).c_str(),
		              nullptr, nullptr, SW_SHOW);
	}

	// Settings Page
	else if (k2app::interfacing::currentAppState == L"settings")
	{
		ShellExecuteW(nullptr, nullptr,
		              std::format(L"https://docs.k2vr.tech/{}/", k2app::K2Settings.appLanguage).c_str(),
		              nullptr, nullptr, SW_SHOW);
	}

	// Devices Page
	else if (k2app::interfacing::currentAppState == L"devices")
	{
		ShellExecuteW(nullptr, nullptr,
		              std::format(L"https://docs.k2vr.tech/{}/", k2app::K2Settings.appLanguage).c_str(),
		              nullptr, nullptr, SW_SHOW);
	}
	else if (k2app::interfacing::currentAppState == L"overrides")
	{
		ShellExecuteW(nullptr, nullptr,
		              std::format(L"https://docs.k2vr.tech/{}/overrides/", k2app::K2Settings.appLanguage).c_str(),
		              nullptr, nullptr, SW_SHOW);
	}

	// Info Page
	else if (k2app::interfacing::currentAppState == L"info")
	{
		ShellExecuteA(nullptr, nullptr,
		              "https://opencollective.com/k2vr",
		              nullptr, nullptr, SW_SHOW);
	}

	// Okashi Page
	else if (k2app::interfacing::currentAppState == L"okashi")
	{
		// Navigate to the general page
		k2app::shared::main::mainNavigationView->
			SelectedItem(k2app::shared::main::mainNavigationView->MenuItems().GetAt(0));
		k2app::shared::main::NavView_Navigate(L"general", Media::Animation::EntranceNavigationTransitionInfo());

		// Hide the okashi tab
		k2app::shared::main::consoleItem.get()->Visibility(Visibility::Collapsed);
	}
}


void Amethyst::implementation::MainWindow::HelpFlyoutDiscordButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	ShellExecuteA(nullptr, nullptr,
	              "https://discord.gg/YBQCRDG",
	              nullptr, nullptr, SW_SHOW);
}


void Amethyst::implementation::MainWindow::HelpFlyoutDevButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	ShellExecuteA(nullptr, nullptr,
	              "https://github.com/KinectToVR/K2TrackingDevice-Samples/blob/main/DEVICES.md",
	              nullptr, nullptr, SW_SHOW);
}


void Amethyst::implementation::MainWindow::ButtonFlyout_Opening(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Show);
}


void Amethyst::implementation::MainWindow::ButtonFlyout_Closing(
	const Controls::Primitives::FlyoutBase& sender,
	const Controls::Primitives::FlyoutBaseClosingEventArgs& args)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Hide);
}


Windows::Foundation::IAsyncAction Amethyst::implementation::MainWindow::HelpFlyoutLicensesButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	apartment_context ui_thread;
	co_await resume_background();
	Sleep(500);
	co_await ui_thread;

	Controls::Primitives::FlyoutShowOptions options;
	options.Placement(Controls::Primitives::FlyoutPlacementMode::Full);
	options.ShowMode(Controls::Primitives::FlyoutShowMode::Transient);

	LicensesFlyout().ShowAt(XMainGrid());
}


void Amethyst::implementation::MainWindow::LicensesFlyout_Closed(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
	k2app::shared::main::interfaceBlockerGrid->Opacity(0.0);
	k2app::shared::main::interfaceBlockerGrid->IsHitTestVisible(false);

	k2app::interfacing::isNUXPending = false;
}


void Amethyst::implementation::MainWindow::LicensesFlyout_Opening(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
	k2app::shared::main::interfaceBlockerGrid->Opacity(0.35);
	k2app::shared::main::interfaceBlockerGrid->IsHitTestVisible(true);

	k2app::interfacing::isNUXPending = true;

	// Load the license text
	if (exists(k2app::interfacing::GetProgramLocation().parent_path() / "Assets" / "Licenses.txt") &&
		is_regular_file(k2app::interfacing::GetProgramLocation().parent_path() / "Assets" / "Licenses.txt"))
	{
		std::wifstream fileHandler(
			(k2app::interfacing::GetProgramLocation().parent_path() / "Assets" / "Licenses.txt").wstring());

		// Change the reader's locale
		fileHandler.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

		// If the file handler's OK, proceed
		if (fileHandler.is_open())
		{
			std::wstringstream wss; // Read the licenses file into shared buffer
			wss << fileHandler.rdbuf(); // And populate the buffer now

			LicensesText().Text(wss.str()); // Replace the placeholder text
		}
	}

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Show);
}


void Amethyst::implementation::MainWindow::XMainGrid_Loaded(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Load theme config
	switch (k2app::K2Settings.appTheme)
	{
	case 2:
		{
			k2app::shared::main::mainNavigationView->
				XamlRoot()
				.Content().as<Controls::Grid>()
				.RequestedTheme(ElementTheme::Light);
			break;
		}
	case 1:
		{
			k2app::shared::main::mainNavigationView->
				XamlRoot()
				.Content().as<Controls::Grid>()
				.RequestedTheme(ElementTheme::Dark);
			break;
		}
	case 0:
	default:
		{
			k2app::shared::main::mainNavigationView->
				XamlRoot()
				.Content().as<Controls::Grid>()
				.RequestedTheme(ElementTheme::Default);
			break;
		}
	}

	// Execute the handler
	XMainGrid_Loaded_Handler();

	// Register a theme watchdog
	NavView().XamlRoot()
	         .Content().as<Controls::Grid>()
	         .ActualThemeChanged([&, this](auto, auto)
	         {
		         k2app::interfacing::actualTheme = NavView().ActualTheme();

		         k2app::shared::main::attentionBrush =
			         k2app::interfacing::actualTheme == ElementTheme::Dark
				         ? std::make_shared<Media::SolidColorBrush>(
					         Application::Current().Resources().TryLookup(
						         box_value(L"AttentionBrush_Dark"
						         )).as<Media::SolidColorBrush>())
				         : std::make_shared<Media::SolidColorBrush>(
					         Application::Current().Resources().TryLookup(
						         box_value(L"AttentionBrush_Light"
						         )).as<Media::SolidColorBrush>());

		         k2app::shared::main::neutralBrush =
			         k2app::interfacing::actualTheme == ElementTheme::Dark
				         ? std::make_shared<Media::SolidColorBrush>(
					         Application::Current().Resources().TryLookup(
						         box_value(L"NeutralBrush_Dark"
						         )).as<Media::SolidColorBrush>())
				         : std::make_shared<Media::SolidColorBrush>(
					         Application::Current().Resources().TryLookup(
						         box_value(L"NeutralBrush_Light"
						         )).as<Media::SolidColorBrush>());

		         // Overwrite the titlebar (decorations) color
		         if (Application::Current().Resources().HasKey(
			         box_value(L"WindowCaptionForeground")))
			         Application::Current().Resources().TryRemove(
				         box_value(L"WindowCaptionForeground"));

		         Application::Current().Resources().Insert(
			         box_value(L"WindowCaptionForeground"),
			         box_value(k2app::interfacing::actualTheme == ElementTheme::Dark
				                   ? Windows::UI::Colors::White()
				                   : Windows::UI::Colors::Black())
		         );

		         // Trigger a titlebar repaint
		         if (k2app::shared::main::thisAppWindowID == GetActiveWindow())
		         {
			         SendMessage(k2app::shared::main::thisAppWindowID, WM_ACTIVATE, WA_INACTIVE, 0);
			         SendMessage(k2app::shared::main::thisAppWindowID, WM_ACTIVATE, WA_ACTIVE, 0);
		         }
		         else
		         {
			         SendMessage(k2app::shared::main::thisAppWindowID, WM_ACTIVATE, WA_ACTIVE, 0);
			         SendMessage(k2app::shared::main::thisAppWindowID, WM_ACTIVATE, WA_INACTIVE, 0);
		         }

		         // Request page reloads
		         k2app::shared::semaphores::semaphore_ReloadPage_MainWindow.release();
		         k2app::shared::semaphores::semaphore_ReloadPage_GeneralPage.release();
		         k2app::shared::semaphores::semaphore_ReloadPage_SettingsPage.release();
		         k2app::shared::semaphores::semaphore_ReloadPage_DevicesPage.release();
		         k2app::shared::semaphores::semaphore_ReloadPage_InfoPage.release();
	         });

	// Mark as loaded
	main_loadedOnce = true;
}


Windows::Foundation::IAsyncAction Amethyst::implementation::MainWindow::XMainGrid_Loaded_Handler()
{
	NavViewGeneralButtonLabel().Text(
		k2app::interfacing::LocalizedJSONString(L"/SharedStrings/Buttons/General"));

	NavViewSettingsButtonLabel().Text(
		k2app::interfacing::LocalizedJSONString(L"/SharedStrings/Buttons/Settings"));

	NavViewDevicesButtonLabel().Text(
		k2app::interfacing::LocalizedJSONString(L"/SharedStrings/Buttons/Devices"));

	NavViewInfoButtonLabel().Text(
		k2app::interfacing::LocalizedJSONString(L"/SharedStrings/Buttons/Info"));

	UpdateIconText().Text(
		k2app::interfacing::LocalizedJSONString(L"/SharedStrings/Buttons/Updates/Header"));

	HelpIconText().Text(
		k2app::interfacing::LocalizedJSONString(L"/SharedStrings/Buttons/Help/Header"));

	InstallLaterButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/SharedStrings/Buttons/Updates/Skip")));

	InstallNowButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/SharedStrings/Buttons/Updates/Install")));

	HelpFlyoutDiscordButton().Text(
		k2app::interfacing::LocalizedJSONString(L"/SharedStrings/Buttons/Help/Discord"));

	HelpFlyoutDevButton().Text(
		k2app::interfacing::LocalizedJSONString(L"/SharedStrings/Buttons/Help/Developers"));

	HelpFlyoutLicensesButton().Text(
		k2app::interfacing::LocalizedJSONString(L"/SharedStrings/Buttons/Help/Licenses"));

	ShutdownTeachingTip().Title(
		k2app::interfacing::LocalizedJSONString(L"/NUX/Tip0/Title"));
	ShutdownTeachingTip().Subtitle(
		k2app::interfacing::LocalizedJSONString(L"/NUX/Tip0/Content"));

	InitializerTeachingTip().Title(
		k2app::interfacing::LocalizedJSONString(L"/NUX/Tip1/Title"));
	InitializerTeachingTip().Subtitle(
		k2app::interfacing::LocalizedJSONString(L"/NUX/Tip1/Content"));
	InitializerTeachingTip().CloseButtonContent(
		box_value(k2app::interfacing::LocalizedJSONString(L"/NUX/Next")));
	InitializerTeachingTip().ActionButtonContent(
		box_value(k2app::interfacing::LocalizedJSONString(L"/NUX/Skip")));

	using namespace k2app;
	using namespace shared::main;

	if (interfacing::currentPageClass == L"Amethyst.GeneralPage")
	{
		navigation_items::navViewGeneralButtonIcon->Glyph(L"\uEA8A");
		navigation_items::navViewGeneralButtonIcon->Foreground(*attentionBrush);

		navigation_items::navViewGeneralButtonLabel->Opacity(0.0);
		navigation_items::navViewGeneralButtonIcon->Translation({0, 0, 0});
	}
	else
	{
		navigation_items::navViewGeneralButtonIcon->Translation({0, -8, 0});
		navigation_items::navViewGeneralButtonLabel->Opacity(1.0);

		navigation_items::navViewGeneralButtonIcon->Foreground(*neutralBrush);
		navigation_items::navViewGeneralButtonIcon->Glyph(L"\uE80F");
	}

	if (interfacing::currentPageClass == L"Amethyst.SettingsPage")
	{
		navigation_items::navViewSettingsButtonIcon->Glyph(L"\uF8B0");
		navigation_items::navViewSettingsButtonIcon->Foreground(*attentionBrush);

		navigation_items::navViewSettingsButtonLabel->Opacity(0.0);
		navigation_items::navViewSettingsButtonIcon->Translation({0, 0, 0});
	}
	else
	{
		navigation_items::navViewSettingsButtonIcon->Translation({0, -8, 0});
		navigation_items::navViewSettingsButtonLabel->Opacity(1.0);

		navigation_items::navViewSettingsButtonIcon->Foreground(*neutralBrush);
		navigation_items::navViewSettingsButtonIcon->Glyph(L"\uE713");
	}

	if (interfacing::currentPageClass == L"Amethyst.DevicesPage")
	{
		navigation_items::navViewDevicesButtonLabel->Opacity(0.0);
		navigation_items::navViewDevicesButtonIcon->Translation({0, 0, 0});

		navigation_items::navViewDevicesButtonIcon->Foreground(*attentionBrush);

		navigation_items::navViewDevicesButtonIcon->Glyph(L"\uEBD2");
		navigation_items::navViewDevicesButtonIcon->FontSize(23);
	}
	else
	{
		navigation_items::navViewDevicesButtonIcon->Translation({0, -8, 0});
		navigation_items::navViewDevicesButtonLabel->Opacity(1.0);

		navigation_items::navViewDevicesButtonIcon->Foreground(*neutralBrush);

		navigation_items::navViewDevicesButtonIcon->Glyph(L"\uF158");
		navigation_items::navViewDevicesButtonIcon->FontSize(20);
	}

	if (interfacing::currentPageClass == L"Amethyst.InfoPage")
	{
		navigation_items::navViewInfoButtonIcon->Glyph(L"\uF167");
		navigation_items::navViewInfoButtonIcon->Foreground(*attentionBrush);

		navigation_items::navViewInfoButtonLabel->Opacity(0.0);
		navigation_items::navViewInfoButtonIcon->Translation({0, 0, 0});
	}
	else
	{
		navigation_items::navViewInfoButtonIcon->Translation({0, -8, 0});
		navigation_items::navViewInfoButtonLabel->Opacity(1.0);

		navigation_items::navViewInfoButtonIcon->Foreground(*neutralBrush);
		navigation_items::navViewInfoButtonIcon->Glyph(L"\uE946");
	}

	if (interfacing::currentPageClass == L"Amethyst.ConsolePage")
	{
		navigation_items::navViewOkashiButtonIcon->Glyph(L"\uEB52");
		navigation_items::navViewOkashiButtonIcon->Foreground(*attentionBrush);

		navigation_items::navViewOkashiButtonLabel->Opacity(0.0);
		navigation_items::navViewOkashiButtonIcon->Translation({0, 0, 0});
	}
	else
	{
		navigation_items::navViewOkashiButtonIcon->Translation({0, -8, 0});
		navigation_items::navViewOkashiButtonLabel->Opacity(1.0);

		navigation_items::navViewOkashiButtonIcon->Foreground(*neutralBrush);
		navigation_items::navViewOkashiButtonIcon->Glyph(L"\uEB51");
	}

	UpdateIcon().Foreground(
		interfacing::checkingUpdatesNow
			? *attentionBrush
			: *neutralBrush);

	const ElementTheme oppositeTheme =
		interfacing::actualTheme == ElementTheme::Dark
			? ElementTheme::Light
			: ElementTheme::Dark;

	{
		// Sleep a bit
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(30);
		co_await ui_thread;
	}
	HelpButton().RequestedTheme(oppositeTheme);
	NavViewGeneralButtonLabel().RequestedTheme(oppositeTheme);
	NavViewSettingsButtonLabel().RequestedTheme(oppositeTheme);
	NavViewDevicesButtonLabel().RequestedTheme(oppositeTheme);
	NavViewInfoButtonLabel().RequestedTheme(oppositeTheme);
	NavViewOkashiButtonLabel().RequestedTheme(oppositeTheme);
	UpdateIconText().RequestedTheme(oppositeTheme);
	PreviewBadgeLabel().RequestedTheme(oppositeTheme);

	{
		// Sleep a bit
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(30);
		co_await ui_thread;
	}
	HelpButton().RequestedTheme(interfacing::actualTheme);
	NavViewGeneralButtonLabel().RequestedTheme(interfacing::actualTheme);
	NavViewSettingsButtonLabel().RequestedTheme(interfacing::actualTheme);
	NavViewDevicesButtonLabel().RequestedTheme(interfacing::actualTheme);
	NavViewInfoButtonLabel().RequestedTheme(interfacing::actualTheme);
	NavViewOkashiButtonLabel().RequestedTheme(interfacing::actualTheme);
	UpdateIconText().RequestedTheme(interfacing::actualTheme);
	PreviewBadgeLabel().RequestedTheme(interfacing::actualTheme);
}
