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
		LOG(WARNING) << "Uh-Oh! It appears that google logging was set up previously from this caller.\n" <<
			"Although, it appears GLog likes Amethyst more! (It said that itself, did you know?)\n" <<
			"Logging will be shut down, re-initialized, and forwarded to \"" <<
			ktvr::GetK2AppDataLogFileDir("Amethyst_").c_str() << "*.log\"";
		google::ShutdownGoogleLogging();
	}

	// Set up logging : flags
	FLAGS_logbufsecs = 0; //Set max timeout
	FLAGS_minloglevel = google::GLOG_INFO;
	FLAGS_timestamp_in_logfile_name = false;

	// Set up logging
	k2app::interfacing::thisLogDestination =
		ktvr::GetK2AppDataLogFileDir("Amethyst_") + k2app::interfacing::GetLogTimestamp();

	google::InitGoogleLogging(k2app::interfacing::thisLogDestination.c_str());

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
