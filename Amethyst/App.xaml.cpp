#include "pch.h"
#include <MddBootstrap.h>

#include "App.xaml.h"
#include "MainWindow.xaml.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace winrt::Microsoft::UI::Xaml;
using namespace Controls;
using namespace Navigation;
using namespace Amethyst;
using namespace implementation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

// https://stackoverflow.com/a/32725606/13934610
std::filesystem::path getLastLogAdded(const std::filesystem::path path)
{
	std::filesystem::file_time_type max{};
	std::filesystem::path last;

	for (std::filesystem::recursive_directory_iterator it(path), directory_iterator; it != directory_iterator; ++it)
		if (is_regular_file(*it) && // If it's a file
			it->path().filename().wstring().find(L"Amethyst") != std::wstring::npos && // If amethyst logs
			it->path().filename().wstring().find(L"VRDriver") == std::wstring::npos && // If not driver log
			it->path().extension() == ".log") // If it's a .log file
		{
			try
			{
				if (auto stamp = last_write_time(*it);
					stamp >= max)
				{
					last = *it;
					max = stamp;
				}
			}
			catch (const std::exception& e)
			{
				std::cerr << "Skipping: " << *it << " (" << e.what() << ")\n";
			}
		}

	return last; // empty if no file matched
}

/// <summary>
/// Initializes the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App()
{
	/* Set up everything before the launch */

	/* Initialize the main app and launch it */

	InitializeComponent();

	Windows::UI::ViewManagement::ApplicationView::PreferredLaunchViewSize(Size(1000, 700));
	Windows::UI::ViewManagement::ApplicationView::PreferredLaunchWindowingMode(
		Windows::UI::ViewManagement::ApplicationViewWindowingMode::PreferredLaunchViewSize);

	// Create an empty file for checking for crashes
	k2app::interfacing::crashFileHandle = CreateFile(
		(k2app::interfacing::GetProgramLocation().parent_path() / ".crash").wstring().c_str(),
		GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, nullptr);

	// If logging was set up by some other thing / assembly,
	// "peacefully" ask it to exit and note that 
	if (google::IsGoogleLoggingInitialized())
	{
		LOG(WARNING) << "Uh-Oh! It appears that google logging was set up previously from this caller.\n " <<
			"Although, it appears GLog likes Amethyst more! (It said that itself, did you know?)\n " <<
			"Logging will be shut down, re-initialized, and forwarded to \"" <<
			WStringToString(ktvr::GetK2AppDataLogFileDir(L"Amethyst", L"Amethyst_")).c_str() << "*.log\"";
		google::ShutdownGoogleLogging();
	}

	// Set up logging : flags
	FLAGS_logbufsecs = 0; // Set max timeout
	FLAGS_minloglevel = google::GLOG_INFO;
	FLAGS_timestamp_in_logfile_name = true;

	// Set up the logging directory
	k2app::interfacing::thisLogDestination =
		ktvr::GetK2AppDataLogFileDir(L"Amethyst", L"Amethyst_");

	// Init logging
	google::InitGoogleLogging(WStringToString(
		k2app::interfacing::thisLogDestination).c_str());

	// Delete logs older than 7 days
	google::EnableLogCleaner(7);

	// Log everything >=INFO to same file
	google::SetLogDestination(google::GLOG_INFO,
	                          WStringToString(k2app::interfacing::thisLogDestination).c_str());

	google::SetLogFilenameExtension(".log");

	// Delete _latest.log as it's not the latest one now
	std::filesystem::remove(ktvr::GetK2AppDataLogFileDir(L"Amethyst", L"_latest.log"));

	// Log the current Amethyst version
	LOG(INFO) << "Amethyst version: " << k2app::interfacing::K2InternalVersion;

	LOG(INFO) << "Running at path: " << WStringToString(
		k2app::interfacing::GetProgramLocation().parent_path().wstring());

	if (const auto deducedLogName = getLastLogAdded(
			ktvr::GetK2AppDataLogFileDir(
				L"Amethyst", L"")).wstring();
		!deducedLogName.empty())
	{
		LOG(INFO) << "The last added Amethyst log appears to be at: \"" <<
			WStringToString(deducedLogName) << "\"!";

		// Overwrite the logging directory
		k2app::interfacing::thisLogDestination = deducedLogName;
	}

	// Read settings
	LOG(INFO) << "Now reading saved settings...";
	k2app::K2Settings.readSettings();

	// Load the language resources
	LOG(INFO) << "Now reading resource settings...";

	// Load app UI strings
	try
	{
		k2app::interfacing::LoadJSONStringResources_English();
		k2app::interfacing::LoadJSONStringResources(k2app::K2Settings.appLanguage);

		// Setup string hot reload watchdog
		std::thread([&, this]
		{
			const std::filesystem::path resource_path =
				k2app::interfacing::GetProgramLocation().parent_path() /
				"Assets" / "Strings";

			// If the specified language doesn't exist somehow, fallback to 'en'
			if (!exists(resource_path))
			{
				LOG(ERROR) << "Could not load language enumeration resources at \"" <<
					WStringToString(resource_path.wstring()) << "\", app interface will be broken!";
				return; // Give up on trying
			}

			HANDLE ChangeHandle = FindFirstChangeNotification(
				resource_path.wstring().c_str(),
				FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);

			while (true)
				if (WaitForSingleObject(ChangeHandle, INFINITE) == WAIT_OBJECT_0)
				{
					// Reload
					k2app::interfacing::LoadJSONStringResources_English();
					k2app::interfacing::LoadJSONStringResources(k2app::K2Settings.appLanguage);

					// Reload plugins' strings
					for (size_t s = 0; s < TrackingDevices::TrackingDevicesLocalizationResourcesRootsVector.size(); s++)
						k2app::interfacing::plugins::plugins_setLocalizationResourcesRoot(
							TrackingDevices::TrackingDevicesLocalizationResourcesRootsVector[s].second, s);

					// Request page reloads
					k2app::shared::semaphores::semaphore_ReloadPage_MainWindow.release();
					k2app::shared::semaphores::semaphore_ReloadPage_GeneralPage.release();
					k2app::shared::semaphores::semaphore_ReloadPage_SettingsPage.release();
					k2app::shared::semaphores::semaphore_ReloadPage_DevicesPage.release();
					k2app::shared::semaphores::semaphore_ReloadPage_InfoPage.release();

					// Wait for the next change
					FindNextChangeNotification(ChangeHandle);
				}
				else return;
		}).detach();
	}
	catch (...)
	{
		LOG(ERROR) << "EXCEPTION READING RESOURCE STRINGS! THE APP INTERFACE WILL BE BROKEN!";
	}

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
	UnhandledException([this](const IInspectable&, const UnhandledExceptionEventArgs& e)
	{
		if (IsDebuggerPresent())
		{
			auto errorMessage = e.Message(); // LOG it?
			__debugbreak();
		}
	});
#endif
}

/// <summary>
/// Invoked when the application is launched normally by the end user.  Other entry points
/// will be used such as when the application is launched to open a specific file.
/// </summary>
/// <param name="e">Details about the launch request and process.</param>
void App::OnLaunched(const LaunchActivatedEventArgs&)
{
	window = make<MainWindow>();
	window.Activate();
}
