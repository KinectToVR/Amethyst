#include "pch.h"
#include "MainWindow.xaml.h"

#include "App.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

// LMAO Eat Dirt Micro&Soft
// Imma just cache object before the fancy UWP delegation reownership
std::shared_ptr<Controls::FontIcon> updateIconDot;

// Helper local variables
bool updateFound = false,
     main_localInitFinished = false;

// Updates checking function
Windows::Foundation::IAsyncAction KinectToVR::implementation::MainWindow::checkUpdates(
	const UIElement& show_el, const bool show, const DWORD delay_ms)
{
	// Attempt only after init
	if (main_localInitFinished)
	{
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
			srand(time(nullptr)); // Generate somewhat random up-to-date or not
			updateFound = rand() & 1;

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
				FlyoutFooter().Text(L"KinectToVR v1.0.1");
				FlyoutContent().Text(L"- OpenSSL Library update\n- Several Kinect V2 fixes");

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
				FlyoutFooter().Text(L"KinectToVR v1.0.0");
				FlyoutContent().Text(L"Please tell us if you have any ideas\nfor the next KinectToVR update.");

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
		options.Placement(Controls::Primitives::FlyoutPlacementMode::Bottom);
		options.ShowMode(Controls::Primitives::FlyoutShowMode::Transient);

		if (updateFound || show)
			UpdateFlyout().ShowAt(show_el, options);
	}
}

namespace winrt::KinectToVR::implementation
{
	MainWindow::MainWindow()
	{
		InitializeComponent();

		// Set up logging
		google::InitGoogleLogging(ktvr::GetK2AppDataLogFileDir("KinectToVR_K2App").c_str());
		// Log everything >=INFO to same file
		google::SetLogDestination(google::GLOG_INFO, ktvr::GetK2AppDataLogFileDir("KinectToVR_K2App").c_str());
		google::SetLogFilenameExtension(".log");

		FLAGS_logbufsecs = 0; //Set max timeout
		FLAGS_minloglevel = google::GLOG_INFO;

		LOG(INFO) << "~~~KinectToVR new logging session begins here!~~~";

		// Read settings
		LOG(INFO) << "Now reading saved settings...";
		k2app::K2Settings.readSettings();

		// Cache needed UI elements
		updateIconDot = std::make_shared<Controls::FontIcon>(UpdateIconDot());

		// Set up
		this->Title(L"KinectToVR");

		this->ExtendsContentIntoTitleBar(true);
		this->SetTitleBar(DragElement());

		m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"general", winrt::xaml_typename<GeneralPage>()));
		m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"settings", winrt::xaml_typename<SettingsPage>()));
		m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"devices", winrt::xaml_typename<DevicesPage>()));
		m_pages.push_back(std::make_pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>
			(L"info", winrt::xaml_typename<InfoPage>()));

		// Scan for tracking devices
		std::thread([&]
			{
				OutputDebugString(L"Searching for tracking devices\n");

				using namespace boost::filesystem;
				using namespace std::string_literals;

				OutputDebugString(L"Current path is: ");
				OutputDebugString(boost::dll::program_location().parent_path().c_str());
				OutputDebugString(L"\n");

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
								OutputDebugString(entry.path().stem().c_str());
								OutputDebugString(L"'s manifest was invalid ヽ(≧□≦)ノ\n");
								continue;
							}

							auto device_name = root.get<std::string>("device_name");
							auto device_type = root.get<std::string>("device_type");

							auto linked_dll_path = root.get<std::string>("linked_dll_path");

							if (root.find("linked_dll_path") == root.not_found())
								linked_dll_path = "none"; // Self-fix or smth?

							OutputDebugString(L"Found tracking device with:\n - name: ");
							OutputDebugString(std::wstring(device_name.begin(), device_name.end()).c_str());
							OutputDebugString(L"\n - type: ");
							OutputDebugString(std::wstring(device_type.begin(), device_type.end()).c_str());
							OutputDebugString(L"\n - linked dll: ");
							OutputDebugString(std::wstring(linked_dll_path.begin(), linked_dll_path.end()).c_str());
							OutputDebugString(L"\n");

							auto deviceDllPath = entry.path() / "bin" / "win64" / ("device_" + device_name + ".dll");

							if (exists(deviceDllPath))
							{
								OutputDebugString(L"Found the device's driver dll, now checking dependencies...\n");

								if (exists(linked_dll_path) || linked_dll_path == "none")
								{
									OutputDebugString(L"Found the device's dependency dll, now loading...\n");

									HINSTANCE hLibraryInstance;
									BOOL fRunTimeLinkSuccess = FALSE;

									// Get a handle to the DLL module.
									hLibraryInstance = LoadLibraryExA(deviceDllPath.string().c_str(), NULL,
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
											OutputDebugString(
												L"Device library loaded, now checking interface...\n");

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
													TrackingDevices::TrackingDevicesVector.push_back(pDevice);

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
													TrackingDevices::TrackingDevicesVector.push_back(pDevice);

													stat = pDevice->statusResultString(
														pDevice->getStatusResult());

													blocks_flip = true; // Always the same for JointsBasis
													supports_math = false; // Always the same for JointsBasis
												}
											}

											switch (returnCode)
											{
											case ktvr::K2InitError_None:
												{
													OutputDebugString(
														L"Interface version OK, now constructing...\n");

													OutputDebugString(
														L"Registered tracking device with:\n - name: ");
													OutputDebugString(
														std::wstring(device_name.begin(), device_name.end()).c_str());
													OutputDebugString(L"\n - type: ");
													OutputDebugString(
														std::wstring(device_type.begin(), device_type.end()).
														c_str());
													OutputDebugString(L"\n - linked dll: ");
													OutputDebugString(
														std::wstring(linked_dll_path.begin(), linked_dll_path.end())
														.c_str());
													OutputDebugString(L"\n - blocks flip: ");
													OutputDebugString(
														std::to_wstring(blocks_flip)
														.c_str());
													OutputDebugString(L"\n - supports math-based orientation: ");
													OutputDebugString(
														std::to_wstring(supports_math)
														.c_str());

													OutputDebugString(L"\nat index ");
													OutputDebugString(std::to_wstring(
														TrackingDevices::TrackingDevicesVector.size() - 1).c_str());
													OutputDebugString(L".\n");

													OutputDebugString(
														L"Device status (should be 'not initialized'): \n[\n");
													OutputDebugString(std::wstring(
														stat.begin(),
														stat.end()).c_str());
													OutputDebugString(L"\n]\n");
												}
												break;
											case ktvr::K2InitError_BadInterface:
												{
													OutputDebugString(
														std::wstring(
															("Device's interface is incompatible with current K2API"s
																+
																ktvr::IK2API_Version +
																", it's probably outdated.\n").
															begin(),
															("Device's interface is incompatible with current K2API"s
																+
																ktvr::IK2API_Version +
																", it's probably outdated.\n").
															end())
														.c_str());
												}
												break;
											case ktvr::K2InitError_Invalid:
												{
													OutputDebugString(
														L"Device either didn't give any return code or it's factory malfunctioned. You can only about it...");
												}
												break;
											}
										}
										else
										{
											OutputDebugString(
												std::wstring(
													("Device's interface is incompatible with current K2API"s
														+
														ktvr::IK2API_Version +
														", it's probably outdated.\n").
													begin(),
													("Device's interface is incompatible with current K2API"s
														+
														ktvr::IK2API_Version +
														", it's probably outdated.\n").
													end())
												.c_str());
										}
									}
									else
										OutputDebugString(
											L"There was an error linking with the device library! (►＿◄)\n");

									// If unable to call the DLL function, use an alternative.
									if (!fRunTimeLinkSuccess)
										OutputDebugString(
											L"There was an error calling the device factory... (⊙_⊙)？\n");
								}
								else
									OutputDebugString(
										L"Device's dependency dll (external linked dll) was not found (┬┬﹏┬┬)\n");
							}
							else
								OutputDebugString(
									L"Device's driver dll (bin/win64/device_[device].dll) was not found o(≧口≦)o\n");
						}
						else
						{
							OutputDebugString(entry.path().stem().c_str());
							OutputDebugString(L"'s manifest was not found :/\n");
						}
					}

					OutputDebugString(
						L"Registration of tracking devices has ended, there are ");
					OutputDebugString(std::to_wstring(TrackingDevices::TrackingDevicesVector.size()).c_str());
					OutputDebugString(L" tracking devices in total.\n");

					// Now select the proper device
					// TODO k2app::interfacing::trackingDeviceID must be read from settings before!
					if (TrackingDevices::TrackingDevicesVector.size() > 0)
					{
						if (TrackingDevices::TrackingDevicesVector.size() <= k2app::interfacing::trackingDeviceID)
							k2app::interfacing::trackingDeviceID = 0; // Select the first one

						// Init the device
						auto const& trackingDevice =
							TrackingDevices::TrackingDevicesVector.at(k2app::interfacing::trackingDeviceID);
						switch (trackingDevice.index())
						{
						case 0:
							// Kinect Basis
							std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->initialize();
							break;
						case 1:
							// Joints Basis
							std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice)->initialize();
							break;
						}

						// Second check and try after 3 seconds
						std::thread([&]
						{
							// Wait a moment
							std::this_thread::sleep_for(std::chrono::seconds(3));

							// Init the device (optionally this time)
							// Base
							{
								switch (auto const& _trackingDevice =
										TrackingDevices::TrackingDevicesVector.at(
											k2app::interfacing::trackingDeviceID);
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
							if (k2app::interfacing::overrideDeviceID > -1 &&
								k2app::interfacing::overrideDeviceID != k2app::interfacing::trackingDeviceID)
							{
								switch (auto const& _trackingDevice =
										TrackingDevices::TrackingDevicesVector.at(
											k2app::interfacing::overrideDeviceID);
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
							else k2app::interfacing::overrideDeviceID = -1; // Set to NONE
						}).detach();

						// Update the UI
						TrackingDevices::updateTrackingDeviceUI(k2app::interfacing::trackingDeviceID);
						//TrackingDevices::updateOverrideDeviceUI(k2app::interfacing::overrideDeviceID); // Not yet
					}
					else // Log and exit, we have nothing to do
					{
						OutputDebugString(L"No proper tracking devices (K2Devices) found :/\n");
						exit(-12); // -12 is for NO_DEVICES
					}
				}
				else // Log and exit, we have nothing to do
				{
					OutputDebugString(L"No tracking devices (K2Devices) found :/\n");
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
	UpdateFlyout().Hide();
}


void KinectToVR::implementation::MainWindow::ExitButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Save and Exit with 0
	exit(0);
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


Windows::Foundation::IAsyncAction KinectToVR::implementation::MainWindow::UpdateButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Check for updates (and show)
	co_await checkUpdates(sender.as<UIElement>(), true);
}


Windows::Foundation::IAsyncAction KinectToVR::implementation::MainWindow::UpdateButton_Loaded(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Check for updates (and show)
	co_await checkUpdates(sender.as<UIElement>(), false, 2000);
}
