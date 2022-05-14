#include "pch.h"
#include "MainWindow.xaml.h"

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

// Imma just cache object before the fancy UWP delegation reownership
std::shared_ptr<Controls::FontIcon> updateIconDot;

// Helper local variables
HANDLE hNamedMutex = nullptr;
bool updateFound = false,
     main_localInitFinished = false;

std::atomic_bool checkingUpdatesNow = false;

// Assume we're up to date
std::string K2RemoteVersion =
	k2app::interfacing::K2InternalVersion;

// Exit handler
void h_exit(void);

// Toast struct (json)
struct toast
{
	std::string guid, title, message;
	bool show_always = false;
};

// Updates checking function
Windows::Foundation::IAsyncAction KinectToVR::implementation::MainWindow::checkUpdates(
	const UIElement& show_el, const bool show, const DWORD delay_ms)
{
	// Attempt only after init
	if (main_localInitFinished)
	{
		// Mark as checking
		checkingUpdatesNow = true;

		{
			// Sleep on UI (Non-blocking)
			apartment_context ui_thread;
			co_await resume_background();
			Sleep(delay_ms);
			co_await ui_thread;
		}

		// Don't check if found
		if (!updateFound)
		{
			// Here check for updates (via external bool)
			IconRotation().Begin();

			// Capture the calling context.
			apartment_context ui_thread;
			co_await resume_background();

			// Check for updates
			auto start_time = std::chrono::high_resolution_clock::now();

			// Check now
			updateFound = false;

			// Dummy for holding change logs
			std::vector<std::string> changes_strings_vector;

			// Check for deez updates
			try
			{
				std::ostringstream release_version_os;

				curlpp::options::Url myUrl(
					std::string("https://github.com/KinectToVR/Amethyst-Releases/releases/latest/download/version"));
				curlpp::Easy myRequest;
				myRequest.setOpt(myUrl);
				myRequest.setOpt(cURLpp::Options::FollowLocation(true));

				curlpp::options::WriteStream ws(&release_version_os);
				myRequest.setOpt(ws);
				myRequest.perform();

				// If the read string isn't empty, proceed to checking for updates
				if (std::string read_buffer = release_version_os.str(); !read_buffer.empty())
				{
					LOG(INFO) << "Update-check successful, string:\n" << read_buffer;

					std::stringstream release_version_os_stream;
					release_version_os_stream << read_buffer;

					boost::property_tree::ptree root;
					read_json(release_version_os_stream, root);

					if (root.find("version_string") == root.not_found() ||
						root.find("changes") == root.not_found())
						LOG(ERROR) << "The latest release's manifest was invalid!";

					else
					{
						// Find the version tag (it's a string only to save time, trust me...)
						K2RemoteVersion = root.get<std::string>("version_string");

						/* Now split the gathered string into the version number */

						// Split version strings into integers
						std::vector<std::string> local_version_num, remote_version_num;
						split(local_version_num, k2app::interfacing::K2InternalVersion, boost::is_any_of("."));
						split(remote_version_num, K2RemoteVersion, boost::is_any_of("."));

						// Compare to the current version
						for (uint32_t i = 0; i < 4; i++)
						{
							// Check the version
							if (const auto _ver = boost::lexical_cast<uint32_t>(remote_version_num.at(i));
								_ver > boost::lexical_cast<uint32_t>(local_version_num.at(i)))
								updateFound = true;

								// Not to false-alarm in situations like 1.0.1.8 (local) vs 1.0.1.0 (remote)
							else if (_ver < boost::lexical_cast<uint32_t>(local_version_num.at(i))) break;
						}

						// Cache the changes
						BOOST_FOREACH(boost::property_tree::ptree::value_type & v, root.get_child("changes"))
							changes_strings_vector.push_back(v.second.get_value<std::string>());

						// And maybe log it too
						LOG(INFO) << "Remote version number: " << K2RemoteVersion;
						LOG(INFO) << "Local version number: " << k2app::interfacing::K2InternalVersion;

						// Thanks to this chad: https://stackoverflow.com/a/45123408
						// Now check for push notifications aka toasts

						// !show checks if the update handler was run automatically:
						//       (by design) it's false only when run at the startup
						if (root.find("toasts") != root.not_found() && !show)
						{
							// Scan for all toasts & push them to a table
							boost::multi_index::multi_index_container<
								toast, boost::multi_index::indexed_by<
									boost::multi_index::ordered_unique<
										boost::multi_index::tag<struct by_name>,
										boost::multi_index::member<
											toast, std::string, &toast::guid>>>> toasts;

							for (auto& v : root.get_child("toasts"))
							{
								auto& node = v.second;
								toast tst;

								tst.guid = node.get<std::string>("guid", "");
								tst.title = node.get<std::string>("title", "");
								tst.message = node.get<std::string>("message", "");
								tst.show_always = node.get<bool>("show_always", false);

								if (!toasts.insert(tst).second)
									LOG(INFO) << "Skipping toast with duplicate guid: " << tst.guid << '\n';
							}

							// Iterate over all the found toasts
							for (auto& itr : toasts)
							{
								// Log everything
								LOG(INFO) << "Found a toast with:\n    guid: " << itr.guid <<
									"\n    title: " << itr.title << "\n    message: " << itr.message <<
									"\n    which needs to show " << (itr.show_always ? "always" : "once");

								// Check if this toast hasn't already been shown, and opt show it
								if (itr.show_always ||
									std::ranges::find(k2app::K2Settings.shownToastsGuidVector, itr.guid)
									== k2app::K2Settings.shownToastsGuidVector.end())
								{
									// Log it
									LOG(INFO) << "Showing toast with guid " << itr.guid << " now...";

									// Show the toast (and optionally cache it)
									k2app::interfacing::ShowToast(itr.title, itr.message);

									// If the toast isn't meant to be shown always, cache it
									if (!itr.show_always)
										k2app::K2Settings.shownToastsGuidVector.push_back(itr.guid);
								}
							}
						}
					}
				}
				else
					LOG(ERROR) << "Update failed, string was empty.";
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

			if (updateFound)
			{
				FlyoutHeader().Text(L"New Update Available");
				FlyoutFooter().Text(L"Amethyst v" + wstring_cast(K2RemoteVersion));

				std::string changelog_string;
				for (const auto& str : changes_strings_vector)
					changelog_string += "- " + str + '\n';

				changelog_string.pop_back(); // Remove the last \n
				FlyoutContent().Text(wstring_cast(changelog_string));

				auto thickness = Thickness();
				thickness.Left = 0;
				thickness.Top = 0;
				thickness.Right = 0;
				thickness.Bottom = 12;
				FlyoutContent().Margin(thickness);

				InstallLaterButton().Visibility(Visibility::Visible);
				InstallNowButton().Visibility(Visibility::Visible);

				updateIconDot.get()->Visibility(Visibility::Visible);
			}
			else
			{
				FlyoutHeader().Text(L"You're Up To Date");
				FlyoutFooter().Text(L"Amethyst v" + wstring_cast(k2app::interfacing::K2InternalVersion));
				FlyoutContent().Text(L"Please tell us if you have any ideas\nfor the next Amethyst update.");

				auto thickness = Thickness();
				thickness.Left = 0;
				thickness.Top = 0;
				thickness.Right = 0;
				thickness.Bottom = 0;
				FlyoutContent().Margin(thickness);

				InstallLaterButton().Visibility(Visibility::Collapsed);
				InstallNowButton().Visibility(Visibility::Collapsed);

				updateIconDot.get()->Visibility(Visibility::Collapsed);
			}
		}

		Controls::Primitives::FlyoutShowOptions options;
		options.Placement(Controls::Primitives::FlyoutPlacementMode::RightEdgeAlignedBottom);
		options.ShowMode(Controls::Primitives::FlyoutShowMode::Transient);

		// If an update was found, show it
		// (or if the cheack was manual)
		if (updateFound || show)
			UpdateFlyout().ShowAt(show_el, options);

		// Otherwise, show the rating request
		// (once per 7,9,11... sessions, skip for -1)
		// TODO DISABLE THIS AFTER PREVIEW VERSIONS
		else if (k2app::K2Settings.ratingRemainingSessions >= 0 &&
			k2app::K2Settings.ratingRemainingElapsedSessions >=
			k2app::K2Settings.ratingRemainingSessions)
		{
			k2app::K2Settings.ratingRemainingSessions += 2;
			k2app::K2Settings.ratingRemainingElapsedSessions = 1;
			k2app::K2Settings.saveSettings();

			RatingFlyout().ShowAt(show_el, options); // Show
		}

		// Uncheck
		checkingUpdatesNow = false;
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

namespace winrt::KinectToVR::implementation
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

		// Handle app exits (mica & dispatcher shutdown)
		m_closedRevoker = this->Closed(auto_revoke, [&](auto&&, auto&&)
		{
			if (nullptr != m_micaController)
			{
				m_micaController.Close();
				m_micaController = nullptr;
			}

			if (nullptr != m_dispatcherQueueController)
			{
				m_dispatcherQueueController.ShutdownQueueAsync();
				m_dispatcherQueueController = nullptr;
			}
		});

		// Set up logging
		google::InitGoogleLogging(ktvr::GetK2AppDataLogFileDir("KinectToVR_Amethyst_").c_str());
		// Log everything >=INFO to same file
		google::SetLogDestination(google::GLOG_INFO, ktvr::GetK2AppDataLogFileDir("KinectToVR_Amethyst_").c_str());
		google::SetLogFilenameExtension(".log");

		FLAGS_logbufsecs = 0; //Set max timeout
		FLAGS_minloglevel = google::GLOG_INFO;

		// Cache needed UI elements
		updateIconDot = std::make_shared<Controls::FontIcon>(UpdateIconDot());

		k2app::shared::main::generalItem = std::make_shared<Controls::NavigationViewItem>(GeneralItem());
		k2app::shared::main::settingsItem = std::make_shared<Controls::NavigationViewItem>(SettingsItem());
		k2app::shared::main::devicesItem = std::make_shared<Controls::NavigationViewItem>(DevicesItem());
		k2app::shared::main::infoItem = std::make_shared<Controls::NavigationViewItem>(InfoItem());
		k2app::shared::main::consoleItem = std::make_shared<Controls::NavigationViewItem>(ConsoleItem());

		// Set up
		this->Title(L"Amethyst [Preview]");

		LOG(INFO) << "Extending the window titlebar...";
		this->ExtendsContentIntoTitleBar(true);
		this->SetTitleBar(DragElement());

		LOG(INFO) << "Making the app window available for children views...";
		k2app::shared::main::thisAppWindow = std::make_shared<Window>(this->try_as<Window>());

		LOG(INFO) << "Making the app dispatcher available for children...";
		k2app::shared::main::thisDispatcherQueue =
			std::make_shared<Microsoft::UI::Dispatching::DispatcherQueue>(DispatcherQueue());

		LOG(INFO) << "Creating the default notification manager (may fail on WinAppSDK <1.1)...";
		k2app::shared::main::thisNotificationManager =
			std::make_shared<Microsoft::Windows::AppNotifications::AppNotificationManager>(
				Microsoft::Windows::AppNotifications::AppNotificationManager::Default());

		LOG(INFO) << "Registering the notification manager (may fail on WinAppSDK <1.1)...";
		k2app::shared::main::thisNotificationManager.get()->Register();

		LOG(INFO) << "Pushing control pages to window...";
		m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"general", winrt::xaml_typename<GeneralPage>()));
		m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"settings", winrt::xaml_typename<SettingsPage>()));
		m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"devices", winrt::xaml_typename<DevicesPage>()));
		m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"info", winrt::xaml_typename<InfoPage>()));
		m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"console", winrt::xaml_typename<ConsolePage>()));

		LOG(INFO) << "~~~KinectToVR new logging session begins here!~~~";

		LOG(INFO) << "Registering a named mutex for com_kinecttovr_k2app_amethyst...";

		hNamedMutex = CreateMutexA(nullptr, TRUE, "com_kinecttovr_k2app_amethyst");
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{
			LOG(ERROR) << "Startup failed! The app is already running.";

			if (exists(boost::dll::program_location().parent_path() / "K2CrashHandler" / "K2CrashHandler.exe"))
			{
				std::thread([]
				{
					ShellExecuteA(nullptr, "open",
					              (boost::dll::program_location().parent_path() / "K2CrashHandler" /
						              "K2CrashHandler.exe ")
					              .string().c_str(), "already_running", nullptr, SW_SHOWDEFAULT);
				}).detach();
			}
			else
				LOG(WARNING) << "Crash handler exe (./K2CrashHandler/K2CrashHandler.exe) not found!";

			Sleep(3000);
			exit(0); // Exit peacefully
		}

		// Priority: Register the app exit handler
		LOG(INFO) << "Registering an atexit handler for the app...";
		atexit(h_exit);

		// Priority: Launch the crash handler
		LOG(INFO) << "Starting the crash handler passing the app PID...";

		if (exists(boost::dll::program_location().parent_path() / "K2CrashHandler" / "K2CrashHandler.exe"))
		{
			std::thread([]
			{
				ShellExecuteA(nullptr, "open",
				              (boost::dll::program_location().parent_path() / "K2CrashHandler" / "K2CrashHandler.exe ")
				              .string().c_str(), std::to_string(GetCurrentProcessId()).c_str(), nullptr,
				              SW_SHOWDEFAULT);
			}).detach();
		}
		else
			LOG(WARNING) << "Crash handler exe (./K2CrashHandler/K2CrashHandler.exe) not found!";

		// Priority: Connect to OpenVR
		if (!k2app::interfacing::OpenVRStartup())
		{
			LOG(ERROR) << "Could not connect to OpenVR! The app will be shut down.";
			exit(-11); // OpenVR is critical, so exit
		}

		// Priority: Set up Amethyst as a vr app
		LOG(INFO) << "Installing the vr application manifest...";
		k2app::interfacing::installApplicationManifest();

		// Priority: Set up VR Input Actions
		if (!k2app::interfacing::EVRActionsStartup())
			LOG(ERROR) << "Could not set up VR Input Actions! The app will lack some functionality.";

		// After-Setup Priority: Set up an AI session
		K2InsightsCLR::Initialize();

		// Priority: Set up the K2API & Server
		static std::thread serverStatusThread(k2app::interfacing::K2ServerDriverSetup);
		serverStatusThread.detach();

		// Read settings
		LOG(INFO) << "Now reading saved settings...";
		k2app::K2Settings.readSettings();

		K2InsightsCLR::LogMetric("CalibrationPoints", k2app::K2Settings.calibrationPointsNumber);
		K2InsightsCLR::LogMetric("TrackerPairs", [&, this]()-> uint32_t
		{
			// The default one (W+F) would be 2 pairs
			uint32_t _out_accumulate = 0;
			std::accumulate(
				k2app::K2Settings.isJointPairEnabled.begin(),
				k2app::K2Settings.isJointPairEnabled.end(),
				_out_accumulate);
			return _out_accumulate;
		}());

		// It's been one more, innit?
		k2app::K2Settings.ratingRemainingElapsedSessions += 1;

		// Start the main loop
		std::thread(k2app::main::K2MainLoop).detach();

		// Load sounds config into main
		ElementSoundPlayer::State(k2app::K2Settings.enableAppSounds
			                          ? ElementSoundPlayerState::On
			                          : ElementSoundPlayerState::Off);
		ElementSoundPlayer::Volume(std::clamp(
			static_cast<double>(k2app::K2Settings.appSoundsVolume) / 100.0, 0.0, 100.0));

		// Scan for tracking devices
		std::thread([&]
			{
				LOG(INFO) << "Searching for tracking devices";

				using namespace boost::filesystem;
				using namespace std::string_literals;

				LOG(INFO) << "Current path is: " << boost::dll::program_location().parent_path().string();

				if (exists(boost::dll::program_location().parent_path() / "devices"))
				{
					for (directory_entry& entry : directory_iterator(
						     boost::dll::program_location().parent_path() / "devices"))
					{
						if (exists(entry.path() / "device.k2devicemanifest"))
						{
							boost::property_tree::ptree root;
							read_json((entry.path() / "device.k2devicemanifest").string(), root);

							if (root.find("device_name") == root.not_found() ||
								root.find("device_type") == root.not_found())
							{
								LOG(ERROR) << entry.path().stem().string() << "'s manifest was invalid!";
								continue;
							}

							auto device_name = root.get<std::string>("device_name");
							auto device_type = root.get<std::string>("device_type");

							LOG(INFO) << "Found tracking device with:\n - name: " << device_name;

							auto deviceDllPath = entry.path() / "bin" / "win64" / ("device_" + device_name + ".dll");

							if (exists(deviceDllPath))
							{
								LOG(INFO) << "Found the device's driver dll, now checking dependencies...";

								bool _found = true; // assume success

								// Check for deez dlls
								if (root.find("linked_dll_path") != root.not_found())
								{
									BOOST_FOREACH(boost::property_tree::ptree::value_type & v,
									              root.get_child("linked_dll_path"))
										if (!exists(v.second.get_value<std::string>()))
										{
											_found = false; // Mark as failed
											LOG(ERROR) << "Linked dll not found at path: " << v.second.get_value<
												std::string>();
										}
								}
								// Else continue

								if (_found)
								{
									LOG(INFO) << "Found the device's dependency dll, now loading...";

									HINSTANCE hLibraryInstance;
									BOOL fRunTimeLinkSuccess = FALSE;

									// Get a handle to the DLL module.
									hLibraryInstance = LoadLibraryExA(deviceDllPath.string().c_str(), nullptr,
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
											std::string stat = "E_UNKNOWN";
											bool blocks_flip = false, supports_math = true;

											if (strcmp(device_type.c_str(), "KinectBasis") == 0)
											{
												auto pDevice =
													static_cast<ktvr::K2TrackingDeviceBase_KinectBasis*>(
														(hDeviceFactory)(ktvr::IK2API_Version, &returnCode));

												if (returnCode == ktvr::K2InitError_None)
												{
													LOG(INFO) << "Interface version OK, now constructing...";

													LOG(INFO) << "Overriding device's helper functions...";

													// Push helper functions to the device
													pDevice->getHMDPosition =
														k2app::interfacing::plugins::plugins_getHMDPosition;
													pDevice->getHMDPositionCalibrated =
														k2app::interfacing::plugins::plugins_getHMDPositionCalibrated;
													pDevice->getHMDOrientation =
														k2app::interfacing::plugins::plugins_getHMDOrientation;
													pDevice->getHMDOrientationCalibrated =
														k2app::interfacing::plugins::plugins_getHMDOrientationCalibrated;
													pDevice->getHMDOrientationYaw =
														k2app::interfacing::plugins::plugins_getHMDOrientationYaw;
													pDevice->getHMDOrientationYawCalibrated =
														k2app::interfacing::plugins::plugins_getHMDOrientationYawCalibrated;

													pDevice->getAppJointPoses =
														k2app::interfacing::plugins::plugins_getAppJointPoses;

													pDevice->CreateTextBlock =
														k2app::interfacing::AppInterface::CreateAppTextBlock_Sliced;
													pDevice->CreateButton =
														k2app::interfacing::AppInterface::CreateAppButton_Sliced;
													pDevice->CreateNumberBox =
														k2app::interfacing::AppInterface::CreateAppNumberBox_Sliced;
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

																	// State that everything's fine and he device's loaded
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

																	// State that everything's fine and he device's loaded
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

													stat = pDevice->statusResultString(
														pDevice->getStatusResult());

													blocks_flip = !pDevice->isFlipSupported();
													supports_math = pDevice->isAppOrientationSupported();
												}
											}
											else if (strcmp(device_type.c_str(), "JointsBasis") == 0)
											{
												auto pDevice =
													static_cast<ktvr::K2TrackingDeviceBase_JointsBasis*>(
														(hDeviceFactory)(ktvr::IK2API_Version, &returnCode));

												if (returnCode == ktvr::K2InitError_None)
												{
													LOG(INFO) << "Interface version OK, now constructing...";

													LOG(INFO) << "Overriding device's helper functions...";

													// Push helper functions to the device
													pDevice->getHMDPosition =
														k2app::interfacing::plugins::plugins_getHMDPosition;
													pDevice->getHMDPositionCalibrated =
														k2app::interfacing::plugins::plugins_getHMDPositionCalibrated;
													pDevice->getHMDOrientation =
														k2app::interfacing::plugins::plugins_getHMDOrientation;
													pDevice->getHMDOrientationCalibrated =
														k2app::interfacing::plugins::plugins_getHMDOrientationCalibrated;
													pDevice->getHMDOrientationYaw =
														k2app::interfacing::plugins::plugins_getHMDOrientationYaw;
													pDevice->getHMDOrientationYawCalibrated =
														k2app::interfacing::plugins::plugins_getHMDOrientationYawCalibrated;

													pDevice->getAppJointPoses =
														k2app::interfacing::plugins::plugins_getAppJointPoses;

													pDevice->CreateTextBlock =
														k2app::interfacing::AppInterface::CreateAppTextBlock_Sliced;
													pDevice->CreateButton =
														k2app::interfacing::AppInterface::CreateAppButton_Sliced;
													pDevice->CreateNumberBox =
														k2app::interfacing::AppInterface::CreateAppNumberBox_Sliced;
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

																	// State that everything's fine and he device's loaded
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

																	// State that everything's fine and he device's loaded
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

													stat = pDevice->statusResultString(
														pDevice->getStatusResult());

													blocks_flip = true; // Always the same for JointsBasis
													supports_math = false; // Always the same for JointsBasis
												}
											}
											else if (strcmp(device_type.c_str(), "Spectator") == 0)
											{
												auto pDevice =
													static_cast<ktvr::K2TrackingDeviceBase_Spectator*>(
														(hDeviceFactory)(ktvr::IK2API_Version, &returnCode));

												if (returnCode == ktvr::K2InitError_None)
												{
													LOG(INFO) << "Interface version OK, now constructing...";

													// Push helper functions to the device
													pDevice->getHMDPosition =
														k2app::interfacing::plugins::plugins_getHMDPosition;
													pDevice->getHMDPositionCalibrated =
														k2app::interfacing::plugins::plugins_getHMDPositionCalibrated;
													pDevice->getHMDOrientation =
														k2app::interfacing::plugins::plugins_getHMDOrientation;
													pDevice->getHMDOrientationCalibrated =
														k2app::interfacing::plugins::plugins_getHMDOrientationCalibrated;
													pDevice->getHMDOrientationYaw =
														k2app::interfacing::plugins::plugins_getHMDOrientationYaw;
													pDevice->getHMDOrientationYawCalibrated =
														k2app::interfacing::plugins::plugins_getHMDOrientationYawCalibrated;

													pDevice->getAppJointPoses =
														k2app::interfacing::plugins::plugins_getAppJointPoses;

													// State that everything's fine and he device's loaded
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
													LOG(INFO) << "Registered tracking device with:\n - name: " <<
														device_name << "\n - type: " << device_type <<
														"\n - blocks flip: " << blocks_flip <<
														"\n - supports math-based orientation: " << supports_math <<

														"\nat index " << TrackingDevices::TrackingDevicesVector.size() -
														1;

													LOG(INFO) << "Device status (should be 'not initialized'): \n[\n" <<
														stat << "\n]\n";
												}
												break;
											case ktvr::K2InitError_BadInterface:
												{
													LOG(ERROR) <<
														"Device's interface is incompatible with current K2API"
														<<
														ktvr::IK2API_Version <<
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
											LOG(ERROR) << "Device's interface is incompatible with current K2API" <<
												ktvr::IK2API_Version <<
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
							LOG(ERROR) << entry.path().stem().string() << "'s manifest was not found :/";
						}
					}

					LOG(INFO) << "Registration of tracking devices has ended, there are " <<
						TrackingDevices::TrackingDevicesVector.size() <<
						" tracking devices in total.";

					// Now select the proper device
					// k2app::K2Settings.trackingDeviceID must be read from settings before!
					if (TrackingDevices::TrackingDevicesVector.size() > 0)
					{
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
								if (k2app::K2Settings.jointRotationTrackingOption[1] ==
									k2app::k2_SoftwareCalculatedRotation &&
									!std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->
									isAppOrientationSupported())
									k2app::K2Settings.jointRotationTrackingOption[1] = k2app::k2_DeviceInferredRotation;

								if (k2app::K2Settings.jointRotationTrackingOption[2] ==
									k2app::k2_SoftwareCalculatedRotation &&
									!std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->
									isAppOrientationSupported())
									k2app::K2Settings.jointRotationTrackingOption[2] = k2app::k2_DeviceInferredRotation;

								//Init
								std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->initialize();
							}
							break;
						case 1:
							// Joints Basis
							{
								// Update options for the device
								if (k2app::K2Settings.jointRotationTrackingOption[1] ==
									k2app::k2_SoftwareCalculatedRotation)
									k2app::K2Settings.jointRotationTrackingOption[1] = k2app::k2_DeviceInferredRotation;

								if (k2app::K2Settings.jointRotationTrackingOption[2] ==
									k2app::k2_SoftwareCalculatedRotation)
									k2app::K2Settings.jointRotationTrackingOption[2] = k2app::k2_DeviceInferredRotation;

								k2app::K2Settings.isFlipEnabled = false;

								// Init
								std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice)->initialize();
							}
							break;
						}

						// Check the override device index
						if (k2app::K2Settings.overrideDeviceID >= TrackingDevices::TrackingDevicesVector.size())
						{
							LOG(INFO) << "Previous tracking device ID was too big, it's been reset to [none]";
							k2app::K2Settings.overrideDeviceID = -1; // Select the first one
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
						else k2app::K2Settings.overrideDeviceID = -1; // Set to NONE

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
							else k2app::K2Settings.overrideDeviceID = -1; // Set to NONE
						}).detach();

						// Update the UI
						k2app::shared::main::thisDispatcherQueue.get()->TryEnqueue([&, this]
						{
							TrackingDevices::updateTrackingDeviceUI(k2app::K2Settings.trackingDeviceID);
							TrackingDevices::updateOverrideDeviceUI(k2app::K2Settings.overrideDeviceID);
						});

						std::thread([&]
						{
							// Second check and try after 5 seconds
							std::this_thread::sleep_for(std::chrono::seconds(5));

							// Update the UI
							k2app::shared::main::thisDispatcherQueue.get()->TryEnqueue([&, this]
							{
								TrackingDevices::updateTrackingDeviceUI(k2app::K2Settings.trackingDeviceID);
								TrackingDevices::updateOverrideDeviceUI(k2app::K2Settings.overrideDeviceID);
							});
						}).detach();
						// Auto-handles if none

						// Update the backend extflip value
						if (!TrackingDevices::isExternalFlipSupportable())
						{
							k2app::K2Settings.isExternalFlipEnabled = false;
							k2app::K2Settings.saveSettings();
						}
					}
					else // Log and exit, we have nothing to do
					{
						LOG(ERROR) << "No proper tracking devices (K2Devices) found :/";
						exit(-12); // -12 is for NO_DEVICES
					}
				}
				else // Log and exit, we have nothing to do
				{
					LOG(ERROR) << "No tracking devices (K2Devices) found :/";
					exit(-12);
				}
			}
		).join(); // Now this would be in background but to spare bugs, we're gonna wait

		// Notify of the setup end
		main_localInitFinished = true;
	}
}

void KinectToVR::implementation::MainWindow::NavView_Loaded(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// NavView doesn't load any page by default, so load home page.
	NavView().SelectedItem(NavView().MenuItems().GetAt(0));

	// If navigation occurs on SelectionChanged, then this isn't needed.
	// Because we use ItemInvoked to navigate, we need to call Navigate
	// here to load the home page.
	NavView_Navigate(L"general",
	                 Media::Animation::EntranceNavigationTransitionInfo());

	// Append placeholder text to the dummy layout root
	k2app::interfacing::emptyLayoutRoot =
		new k2app::interfacing::AppInterface::AppLayoutRoot();

	k2app::interfacing::emptyLayoutRoot->AppendSingleElement(
		k2app::interfacing::AppInterface::CreateAppTextBlock_Sliced(
			"In the beginning was the Word."),
		ktvr::Interface::SingleLayoutHorizontalAlignment::Left);

	k2app::interfacing::emptyLayoutRoot->AppendSingleElement(
		k2app::interfacing::AppInterface::CreateAppTextBlock_Sliced(
			"  But this device's settings are something else..."),
		ktvr::Interface::SingleLayoutHorizontalAlignment::Left);
}

void KinectToVR::implementation::MainWindow::NavView_ItemInvoked(
	const Controls::NavigationView& sender,
	const Controls::NavigationViewItemInvokedEventArgs& args)
{
	NavView_Navigate(
		winrt::unbox_value_or<hstring>(
			args.InvokedItemContainer().Tag(), L"").c_str(),
		args.RecommendedNavigationTransitionInfo());
}

void KinectToVR::implementation::MainWindow::NavView_Navigate(
	std::wstring navItemTag,
	const Media::Animation::NavigationTransitionInfo& transitionInfo)
{
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
	Windows::UI::Xaml::Interop::TypeName preNavPageType =
		ContentFrame().CurrentSourcePageType();

	// Navigate only if the selected page isn't currently loaded.
	if (pageTypeName.Name != L"" && preNavPageType.Name != pageTypeName.Name)
	{
		ContentFrame().Navigate(pageTypeName, nullptr, transitionInfo);
	}
}

void KinectToVR::implementation::MainWindow::ContentFrame_NavigationFailed(
	const Windows::Foundation::IInspectable& sender,
	const Navigation::NavigationFailedEventArgs& e)
{
	throw hresult_error(
		E_FAIL, hstring(L"Failed to load Page ") + e.SourcePageType().Name);
}

void KinectToVR::implementation::MainWindow::NavView_BackRequested(
	const Controls::NavigationView& sender,
	const Controls::NavigationViewBackRequestedEventArgs& args)
{
	TryGoBack();
}

void KinectToVR::implementation::MainWindow::CoreDispatcher_AcceleratorKeyActivated(
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

void KinectToVR::implementation::MainWindow::CoreWindow_PointerPressed(
	const Windows::UI::Core::CoreWindow& /* sender */,
	const Windows::UI::Core::PointerEventArgs& args)
{
	// Handle mouse back button.
	if (args.CurrentPoint().Properties().IsXButton1Pressed())
	{
		args.Handled(TryGoBack());
	}
}

void KinectToVR::implementation::MainWindow::System_BackRequested(
	const Windows::Foundation::IInspectable& /* sender */,
	const Windows::UI::Core::BackRequestedEventArgs& args)
{
	if (!args.Handled())
	{
		args.Handled(TryGoBack());
	}
}

bool KinectToVR::implementation::MainWindow::TryGoBack()
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

void KinectToVR::implementation::MainWindow::On_Navigated(
	const Windows::Foundation::IInspectable& /* sender */,
	const Windows::UI::Xaml::Navigation::NavigationEventArgs& args)
{
	NavView().IsBackEnabled(ContentFrame().CanGoBack());

	if (ContentFrame().SourcePageType().Name != L"")
	{
		for (auto&& eachPage : m_pages)
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

void KinectToVR::implementation::MainWindow::InstallLaterButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	UpdateFlyout().Hide();
}

void KinectToVR::implementation::MainWindow::InstallNowButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	ShellExecuteA(nullptr, nullptr, "https://github.com/KinectToVR/Amethyst-Releases/releases/latest", nullptr, nullptr,
	              SW_SHOW);
	UpdateFlyout().Hide();
}


void KinectToVR::implementation::MainWindow::ExitButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	h_exit();
}


void KinectToVR::implementation::MainWindow::MinimizeButton_Click(
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


Windows::Foundation::IAsyncAction KinectToVR::implementation::MainWindow::UpdateButton_Loaded(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Check for updates (and show)
	co_await checkUpdates(sender.as<UIElement>(), false, 2000);
}


Windows::Foundation::IAsyncAction KinectToVR::implementation::MainWindow::UpdateButton_Tapped(
	const Windows::Foundation::IInspectable& sender,
	const Input::TappedRoutedEventArgs& e)
{
	// Check for updates (and show)
	if (!checkingUpdatesNow)
		co_await checkUpdates(sender.as<UIElement>(), true);
	else co_return;
}


void h_exit()
{
	// Mark exiting as true
	k2app::interfacing::isExitingNow = true;

	// Mark trackers as inactive
	k2app::interfacing::K2AppTrackersInitialized = false;

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
					"Shutting down: com_kinecttovr_k2app_amethyst named mutex close failed! The app may misbehave.";
			}();
		}
	}();

	// Wait a moment
	Sleep(1000);
}


// Disable rating prompts button
void winrt::KinectToVR::implementation::MainWindow::HyperlinkButton_Click(
	const winrt::Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	// Disable all rating prompts
	k2app::K2Settings.ratingRemainingSessions = -1;
	k2app::K2Settings.saveSettings();
	RatingFlyout().Hide();
}


// AutoHide after rated, we don't care anymore
Windows::Foundation::IAsyncAction winrt::KinectToVR::implementation::MainWindow::RatingControl_ValueChanged(
	const winrt::Microsoft::UI::Xaml::Controls::RatingControl& sender, const winrt::Windows::Foundation::IInspectable& args)
{
	// Log the rated value
	K2InsightsCLR::LogMetric("Rating", sender.Value());
	BetaRatingControl().IsReadOnly(true);
	
	// Sleep on UI (Non-blocking)
	apartment_context ui_thread;
	co_await resume_background();
	Sleep(300);
	co_await ui_thread;

	RatingFlyout().Hide();
}
