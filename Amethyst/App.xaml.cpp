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

	// If logging was set up by some other thing / assembly,
	// "peacefully" ask it to exit and note that 
	if (google::IsGoogleLoggingInitialized())
	{
		LOG(WARNING) << "Uh-Oh! It appears that google logging was set up previously from this caller.\n " <<
			"Although, it appears GLog likes Amethyst more! (It said that itself, did you know?)\n " <<
			"Logging will be shut down, re-initialized, and forwarded to \"" <<
			ktvr::GetK2AppDataLogFileDir("Amethyst_").c_str() << "*.log\"";
		google::ShutdownGoogleLogging();
	}

	// Set up logging : flags
	FLAGS_logbufsecs = 0; // Set max timeout
	FLAGS_minloglevel = google::GLOG_INFO;
	FLAGS_timestamp_in_logfile_name = true;

	// Set up the logging directory
	k2app::interfacing::thisLogDestination = ktvr::GetK2AppDataLogFileDir("Amethyst_");

	// Init logging
	google::InitGoogleLogging(k2app::interfacing::thisLogDestination.c_str());

	// Delete logs older than 7 days
	google::EnableLogCleaner(7);
	
	// Log everything >=INFO to same file
	google::SetLogDestination(google::GLOG_INFO, k2app::interfacing::thisLogDestination.c_str());
	google::SetLogFilenameExtension(".log");

	// Log the current Amethyst version
	LOG(INFO) << "Amethyst version: " << k2app::interfacing::K2InternalVersion;

	// Read settings
	LOG(INFO) << "Now reading saved settings...";
	k2app::K2Settings.readSettings();

	// Load the language resources
	LOG(INFO) << "Now reading resource settings...";

	try
	{
		k2app::interfacing::LoadJSONStringResources_English();
		k2app::interfacing::LoadJSONStringResources(k2app::K2Settings.appLanguage);

		std::thread([&, this]
		{
			const boost::filesystem::path resource_path =
				boost::dll::program_location().parent_path() /
				"Assets" / "Strings";

			// If the specified language doesn't exist somehow, fallback to 'en'
			if (!exists(resource_path))
			{
				LOG(ERROR) << "Could not load language enumeration resources at \"" <<
					resource_path.string() << "\", app interface will be broken!";
				return; // Give up on trying
			}

			HANDLE ChangeHandle = FindFirstChangeNotification(
				resource_path.wstring().c_str(),
				FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);

			while (true)
			{
				if (WaitForSingleObject(ChangeHandle, INFINITE) == WAIT_OBJECT_0)
				{
					// Reload
					k2app::interfacing::LoadJSONStringResources_English();
					k2app::interfacing::LoadJSONStringResources(k2app::K2Settings.appLanguage);

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
			}
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
