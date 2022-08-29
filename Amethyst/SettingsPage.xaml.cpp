#include "pch.h"
#include "SettingsPage.xaml.h"

#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

bool settings_loadedOnce = false;

void settings_safe_clear(const std::shared_ptr<Controls::StackPanel>& panel)
{
	[&]
	{
		__try
		{
			[&]
			{
				panel.get()->Children().Clear();
			}();
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			[&]
			{
				LOG(WARNING) << "Couldn't clear a StackPanel. You better call an exorcist.";
			}();
		}
	}();
}

namespace winrt::Amethyst::implementation
{
	SettingsPage::SettingsPage()
	{
		InitializeComponent();

		LOG(INFO) << "Constructing page with tag: \"settings\"...";

		// Cache needed UI elements
		using namespace k2app::shared::settings;

		LOG(INFO) << "Appending settings' page elements to the shared context";

		k2app::shared::teaching_tips::settings::manageTrackersTeachingTip =
			std::make_shared<Controls::TeachingTip>(ManageTrackersTeachingTip());

		restartButton = std::make_shared<Controls::Button>(RestartButton());

		externalFlipCheckBox = std::make_shared<Controls::CheckBox>(ExternalFlipCheckBox());
		autoSpawnCheckbox = std::make_shared<Controls::CheckBox>(AutoSpawnCheckBox());
		enableSoundsCheckbox = std::make_shared<Controls::CheckBox>(SoundsEnabledCheckBox());
		autoStartCheckBox = std::make_shared<Controls::CheckBox>(AutoStartCheckBox());
		checkOverlapsCheckBox = std::make_shared<Controls::CheckBox>(CheckOverlapsCheckBox());

		flipDropDownGrid = std::make_shared<Controls::Grid>(FlipDropDownGrid());

		pageMainScrollViewer = std::make_shared<Controls::ScrollViewer>(PageMainScrollViewer());

		flipDropDownContainer = std::make_shared<Controls::StackPanel>(FlipDropDownContainer());
		jointExpanderHostStackPanel = std::make_shared<Controls::StackPanel>(JointExpanderHostStackPanel());
		externalFlipStatusStackPanel = std::make_shared<Controls::StackPanel>(ExtFlipStatusStackPanel());

		flipToggle = std::make_shared<Controls::ToggleSwitch>(FlipToggle());

		externalFlipCheckBoxLabel = std::make_shared<Controls::TextBlock>(ExternalFlipCheckBoxLabel());
		setErrorFlyoutText = std::make_shared<Controls::TextBlock>(SetErrorFlyoutText());
		externalFlipStatusLabel = std::make_shared<Controls::TextBlock>(ExtFlipStatusLabel());

		flipDropDown = std::make_shared<Controls::Expander>(FlipDropDown());
		soundsVolumeSlider = std::make_shared<Controls::Slider>(SoundsVolumeSlider());
		externalFlipStackPanel = std::make_shared<Controls::StackPanel>(ExternalFlipStackPanel());

		LOG(INFO) << "Rebuilding joint expanders... this may take a while...";

		jointExpanderVector.clear();
		jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(std::vector{
			&k2app::K2Settings.K2TrackersVector[0]
		})));

		if (k2app::K2Settings.useTrackerPairs)
		{
			LOG(INFO) << "UseTrackerPairs is set to true: Appending the default expanders as pairs...";

			jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(
				std::vector{&k2app::K2Settings.K2TrackersVector[1], &k2app::K2Settings.K2TrackersVector[2]},
				k2app::interfacing::LocalizedResourceWString(
					L"SharedStrings", L"Joints/Pairs/Feet"))));

			jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(
				std::vector{&k2app::K2Settings.K2TrackersVector[3], &k2app::K2Settings.K2TrackersVector[4]},
				k2app::interfacing::LocalizedResourceWString(
					L"SharedStrings", L"Joints/Pairs/Elbows"))));

			jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(
				std::vector{&k2app::K2Settings.K2TrackersVector[5], &k2app::K2Settings.K2TrackersVector[6]},
				k2app::interfacing::LocalizedResourceWString(
					L"SharedStrings", L"Joints/Pairs/Knees"))));
		}

		LOG(INFO) << "Appending additional expanders (if they exist)...";

		// k2app::K2Settings.useTrackerPairs ? 7 : 1 means that if pairs have
		// already been appended, we'll start after them, and if not -
		// - we'll append them as individual tracker/joint expanders

		for (uint32_t index = (k2app::K2Settings.useTrackerPairs ? 7 : 1);
		     index < k2app::K2Settings.K2TrackersVector.size(); index++)
			jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(std::vector{
				&k2app::K2Settings.K2TrackersVector[index]
			})));

		LOG(INFO) << "Clearing the appended expanders (UI Node)";
		settings_safe_clear(jointExpanderHostStackPanel);

		LOG(INFO) << "Appending the new expanders to the UI Node";

		int _expander_number = (k2app::K2Settings.useTrackerPairs ? 1 : 2); // For separators
		for (auto& expander : jointExpanderVector)
		{
			// Append the expander
			jointExpanderHostStackPanel->Children().Append(*expander->Container());

			// Append the separator (optionally)
			if (_expander_number >= 2 && jointExpanderVector.back() != expander)
			{
				auto separator = Controls::MenuFlyoutSeparator();
				separator.Margin({10, 10, 10, 0});

				Media::Animation::TransitionCollection c_transition_collection;
				c_transition_collection.Append(Media::Animation::RepositionThemeTransition());
				separator.Transitions(c_transition_collection);

				jointExpanderHostStackPanel->Children().Append(separator);
				_expander_number = 1;
			}
			else _expander_number++;
		}

		LOG(INFO) << "Registering a detached binary semaphore reload handler for SettingsPage...";
		std::thread([&, this]
		{
			while (true)
			{
				// Wait for a reload signal (blocking)
				k2app::shared::semaphores::semaphore_ReloadPage_SettingsPage.acquire();

				// Reload & restart the waiting loop
				if (settings_loadedOnce)
					k2app::shared::main::thisDispatcherQueue->TryEnqueue([&, this]
					{
						SettingsPage_Loaded_Handler();

						settings_localInitFinished = false;

						LOG(INFO) << "Rebuilding joint expanders... this may take a while...";

						jointExpanderVector.clear();
						jointExpanderVector.push_back(std::move(
							std::make_shared<Controls::JointExpander>(std::vector{
								&k2app::K2Settings.K2TrackersVector[0]
							})));

						if (k2app::K2Settings.useTrackerPairs)
						{
							LOG(INFO) << "UseTrackerPairs is set to true: Appending the default expanders as pairs...";

							jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(
								std::vector{
									&k2app::K2Settings.K2TrackersVector[1], &k2app::K2Settings.K2TrackersVector[2]
								},
								k2app::interfacing::LocalizedResourceWString(
									L"SharedStrings", L"Joints/Pairs/Feet"))));

							jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(
								std::vector{
									&k2app::K2Settings.K2TrackersVector[3], &k2app::K2Settings.K2TrackersVector[4]
								},
								k2app::interfacing::LocalizedResourceWString(
									L"SharedStrings", L"Joints/Pairs/Elbows"))));

							jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(
								std::vector{
									&k2app::K2Settings.K2TrackersVector[5], &k2app::K2Settings.K2TrackersVector[6]
								},
								k2app::interfacing::LocalizedResourceWString(
									L"SharedStrings", L"Joints/Pairs/Knees"))));
						}

						LOG(INFO) << "Appending additional expanders (if they exist)...";

						// k2app::K2Settings.useTrackerPairs ? 7 : 1 means that if pairs have
						// already been appended, we'll start after them, and if not -
						// - we'll append them as individual tracker/joint expanders

						for (uint32_t ind = (k2app::K2Settings.useTrackerPairs ? 7 : 1);
						     ind < k2app::K2Settings.K2TrackersVector.size(); ind++)
							jointExpanderVector.push_back(std::move(
								std::make_shared<Controls::JointExpander>(std::vector{
									&k2app::K2Settings.K2TrackersVector[ind]
								})));

						LOG(INFO) << "Clearing the appended expanders (UI Node)";
						settings_safe_clear(jointExpanderHostStackPanel);

						LOG(INFO) << "Appending the new expanders to the UI Node";

						int _expander_number = (k2app::K2Settings.useTrackerPairs ? 1 : 2); // For separators
						for (auto expander : jointExpanderVector)
						{
							// Append the expander
							jointExpanderHostStackPanel->Children().Append(*expander->Container());

							// Append the separator (optionally)
							if (_expander_number >= 2 && jointExpanderVector.back() != expander)
							{
								auto separator = Controls::MenuFlyoutSeparator();
								separator.Margin({10, 10, 10, 0});

								Media::Animation::TransitionCollection c_transition_collection;
								c_transition_collection.Append(Media::Animation::RepositionThemeTransition());
								separator.Transitions(c_transition_collection);

								jointExpanderHostStackPanel->Children().Append(separator);
								_expander_number = 1;
							}
							else _expander_number++;
						}

						if (const auto& trackingDevice = TrackingDevices::getCurrentDevice();
							trackingDevice.index() == 0)
						{
							// Kinect Basis
							const bool _sup = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->
								isAppOrientationSupported();

							for (auto expander : jointExpanderVector)
								expander->EnableSoftwareOrientation(_sup);

							flipToggle.get()->IsEnabled(
								std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported());
							flipDropDown.get()->IsEnabled(k2app::K2Settings.isFlipEnabled &&
								std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported());
							flipDropDownGrid.get()->Opacity(flipToggle.get()->IsEnabled() ? 1 : 0.5);

							// Hide/Show the flip controls container
							flipDropDownContainer.get()->Visibility(
								flipToggle.get()->IsEnabled()
									? Visibility::Visible
									: Visibility::Collapsed);

							TrackingDevices::settings_set_external_flip_is_enabled();
						}
						else if (trackingDevice.index() == 1)
						{
							// Joints Basis
							for (auto expander : jointExpanderVector)
								expander->EnableSoftwareOrientation(false);

							flipToggle.get()->IsEnabled(false);
							flipDropDown.get()->IsEnabled(false);
							flipDropDownGrid.get()->Opacity(0.5);

							// Hide the flip controls container
							flipDropDownContainer.get()->Visibility(Visibility::Collapsed);

							TrackingDevices::settings_set_external_flip_is_enabled();
						}

						// Load the tracker configuration
						for (auto expander : jointExpanderVector)
							expander->UpdateIsActive();

						// Enable/Disable combos
						TrackingDevices::settings_trackersConfig_UpdateIsEnabled();

						// Enable/Disable ExtFlip
						TrackingDevices::settings_set_external_flip_is_enabled();

						// Notify of the setup end
						settings_localInitFinished = true;
						k2app::K2Settings.saveSettings();
						k2app::K2Settings.readSettings(); // Calls config check
					});

				Sleep(100); // Sleep a bit
			}
		}).detach();
	}
}


void Amethyst::implementation::SettingsPage::ExternalFlipCheckBox_Checked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	// Cache flip to settings and save
	k2app::K2Settings.isExternalFlipEnabled = true; // Checked
	TrackingDevices::settings_set_external_flip_is_enabled(); // Parse

	k2app::K2Settings.saveSettings();

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::ToggleOn);
}


void Amethyst::implementation::SettingsPage::ExternalFlipCheckBox_Unchecked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	// Cache flip to settings and save
	k2app::K2Settings.isExternalFlipEnabled = false; // Unchecked
	TrackingDevices::settings_set_external_flip_is_enabled(); // Parse

	k2app::K2Settings.saveSettings();

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::ToggleOff);
}


void Amethyst::implementation::SettingsPage::RestartButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	ktvr::request_vr_restart<false>("SteamVR needs to be restarted to enable/disable trackers properly.");

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);
}

Windows::Foundation::IAsyncAction
Amethyst::implementation::SettingsPage::ResetButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	LOG(INFO) << "Reset has been invoked: turning trackers off...";

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);

	// Mark trackers as inactive
	k2app::interfacing::K2AppTrackersInitialized = false;
	if (k2app::shared::general::toggleTrackersButton.get() != nullptr)
		k2app::shared::general::toggleTrackersButton->IsChecked(false);

	LOG(INFO) << "Reset has been invoked: clearing app settings...";

	// Mark exiting as true
	k2app::interfacing::isExitingNow = true;

	{
		// Sleep a bit
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(50);
		co_await ui_thread;
	}

	// Read settings after reset
	k2app::K2Settings = k2app::K2AppSettings(); // Reset settings
	k2app::K2Settings.saveSettings(); // Save empty settings
	k2app::K2Settings.readSettings(); // Reload empty settings

	/* Restart app */

	// Literals
	using namespace std::string_literals;

	// Get current caller path
	const auto fileName = new CHAR[MAX_PATH + 1];
	const DWORD charsWritten = GetModuleFileNameA(nullptr, fileName, MAX_PATH + 1);

	LOG(INFO) << "Reset invoked: trying to restart the app...";

	// If we've found who asked
	if (charsWritten != 0)
	{
		// Log the caller
		LOG(INFO) << "The current caller process is: "s + fileName;

		// Exit the app
		LOG(INFO) << "Configuration has been reset, exiting in 500ms...";

		// Don't execute the exit routine
		k2app::interfacing::isExitHandled = true;

		// Handle a typical app exit
		apartment_context ui_thread;
		co_await resume_background();
		k2app::interfacing::handle_app_exit(500);
		co_await ui_thread;

		// Restart and exit with code 0
		CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		ShellExecuteA(nullptr, nullptr, fileName, nullptr, nullptr, SW_SHOWDEFAULT);

		exit(0);
	}
	LOG(ERROR) << "App will not be restarted due to caller process identification error.";
	k2app::interfacing::ShowToast(
		k2app::interfacing::LocalizedResourceWString(L"SharedStrings", L"Toasts/RestartFailed/Title"),
		k2app::interfacing::LocalizedResourceWString(L"SharedStrings", L"Toasts/RestartFailed"));

	k2app::interfacing::ShowVRToast(
		k2app::interfacing::LocalizedJSONString_EN(L"/SharedStrings/Toasts/RestartFailed/Title"),
		k2app::interfacing::LocalizedJSONString_EN(L"/SharedStrings/Toasts/RestartFailed"));
}


std::vector<std::wstring> _language_codes_enum;

void Amethyst::implementation::SettingsPage::SettingsPage_Loaded(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	LOG(INFO) << "Re/Loading page with tag: \"settings\"...";
	k2app::interfacing::currentAppState = L"settings";

	// Execute the handler
	SettingsPage_Loaded_Handler();

	// Mark as loaded
	settings_loadedOnce = true;
}

void Amethyst::implementation::SettingsPage::SettingsPage_Loaded_Handler()
{
	// Load strings (must be the first thing we're doing)

	Titles_Application().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Titles/Application"));

	Labels_DisplayLanguage().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Labels/DisplayLanguage"));

	Labels_AppTheme().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Labels/AppTheme"));

	Captions_Sounds().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Captions/Sounds"));

	Titles_SkeletonFlip().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Titles/SkeletonFlip"));

	Captions_SkeletonFlip().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Captions/SkeletonFlip"));

	Controls::ToolTipService::SetToolTip(
		FlipToggle(), box_value(
			k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Elements/FlipToggle/ToolTip")));

	ExternalFlipCheckBoxLabel().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Titles/ExtFlip"));

	Controls::ToolTipService::SetToolTip(
		ExtFlipAppBarButton(), box_value(
			k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Elements/ExtFlipCalibration/ToolTip")));

	Captions_FaceTheKinect().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Captions/FaceTheKinect"));

	Captions_CalibrateExtFlip().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Captions/CalibrateExtFlip"));

	Captions_Device_Status().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/Device/Status"));

	Captions_ExtFlip().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Captions/ExtFlip"));

	Titles_TrackerConfig_Header().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Titles/TrackerConfig/Header"));

	LearnAboutFiltersButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Titles/TrackerConfig/Learn")));

	Titles_TrackerConfig().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Titles/TrackerConfig"));

	Captions_TrackerConfig().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/TrackerConfig"));

	Captions_TrackerConfigNote().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/TrackerConfigNote"));

	Titles_FilterSettings().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Titles/FilterSettings"));

	Captions_Filters_Names_LERP().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/Filters/Names/LERP"));

	Captions_Filters_Explanations_LERP().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/Filters/Explanations/LERP"));

	Captions_Filters_Names_LowPass().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/Filters/Names/LowPass"));

	Captions_Filters_Explanations_LowPass().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/Filters/Explanations/LowPass"));

	Captions_Filters_Names_EKF().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/Filters/Names/EKF"));

	Captions_Filters_Explanations_EKF().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/Filters/Explanations/EKF"));

	Captions_Filters_Names_None().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/Filters/Names/None"));

	Captions_Filters_Explanations_None().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/Filters/Explanations/None"));

	Titles_RotationSettings().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Titles/RotationSettings"));

	Captions_Orientation_Introduction().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/Orientation/Introduction"));

	Captions_Orientation_Names_Default().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/Orientation/Names/Default"));

	Captions_Orientation_Explanations_Default().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/Orientation/Explanations/Default"));

	Captions_Orientation_Names_MathBased().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/Orientation/Names/MathBased"));

	Captions_Orientation_Explanations_MathBased().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/Orientation/Explanations/MathBased"));

	Captions_Orientation_Names_HMD().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/Orientation/Names/HMD"));

	Captions_Orientation_Explanations_HMD().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/Orientation/Explanations/HMD"));

	Captions_Orientation_Names_None().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/Orientation/Names/None"));

	Captions_Orientation_Explanations_None().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/Orientation/Explanations/None"));

	Titles_ManageTrackers().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Titles/ManageTrackers"));

	Captions_ManageTrackers().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsLearn/Captions/ManageTrackers"));

	RestartButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Buttons/RestartSteamVR")));

	Captions_TrackersRestart_Line1().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Captions/TrackersRestart/Line1"));

	Captions_TrackersRestart_Line2().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Captions/TrackersRestart/Line2"));

	Captions_TrackersAutoSpawn().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Captions/TrackersAutoSpawn"));

	Captions_TrackersAutoCheck().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Captions/TrackersAutoCheck"));

	Titles_Troubleshooting().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Titles/Troubleshooting"));

	ResetButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Buttons/Reset")));

	ReRegisterButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Buttons/ReRegister")));

	ViewLogsButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Buttons/ViewLogs")));

	ReManifestButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Buttons/ReManifest")));

	Captions_AutoStart().Text(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Captions/AutoStart"));

	DismissSetErrorButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Buttons/Error/Dismiss")));

	using namespace k2app::shared::settings;

	// Notify of the setup end
	settings_localInitFinished = false;
	CheckOverlapsCheckBox().IsChecked(k2app::K2Settings.checkForOverlappingTrackers);

	// Clear available languages' list
	LanguageOptionBox().Items().Clear();

	// Push all the found languages
	if (exists(k2app::interfacing::GetProgramLocation().parent_path() / "Assets" / "Strings"))
		for (auto entry : std::filesystem::directory_iterator(
			     k2app::interfacing::GetProgramLocation().parent_path() / "Assets" / "Strings"))
		{
			if (entry.path().stem().wstring() == L"locales")continue;

			_language_codes_enum.push_back(entry.path().stem().wstring());

			LanguageOptionBox().Items().Append(box_value(
				k2app::interfacing::GetLocalizedLanguageName(entry.path().stem().wstring())));

			if (entry.path().stem().wstring() == k2app::K2Settings.appLanguage)
				LanguageOptionBox().SelectedIndex(LanguageOptionBox().Items().Size() - 1);
		}

	// Clear available themes' list
	AppThemeOptionBox().Items().Clear();

	// Push all the available themes
	AppThemeOptionBox().Items().Append(box_value(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Themes/System")));
	AppThemeOptionBox().Items().Append(box_value(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Themes/Dark")));
	AppThemeOptionBox().Items().Append(box_value(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Themes/White")));

	// Select the current theme
	AppThemeOptionBox().SelectedIndex(k2app::K2Settings.appTheme);

	// Optionally show the foreign language grid
	if (!exists(k2app::interfacing::GetProgramLocation().parent_path() /
		"Assets" / "Strings" / (std::wstring(
			Windows::Globalization::Language(
				Windows::System::UserProfile::
				GlobalizationPreferences::Languages()
				.GetAt(0)).LanguageTag()).substr(0, 2) + L".json")))
		ForeignLangGrid().Visibility(Visibility::Visible);

	// Select saved flip, position and rotation options
	flipToggle.get()->IsOn(k2app::K2Settings.isFlipEnabled);
	externalFlipCheckBox.get()->IsChecked(k2app::K2Settings.isExternalFlipEnabled);

	if (const auto& trackingDevice = TrackingDevices::getCurrentDevice();
		trackingDevice.index() == 0)
	{
		// Kinect Basis
		const bool _sup = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->
			isAppOrientationSupported();

		for (auto expander : jointExpanderVector)
			expander->EnableSoftwareOrientation(_sup);

		flipToggle.get()->IsEnabled(
			std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported());
		flipDropDown.get()->IsEnabled(k2app::K2Settings.isFlipEnabled &&
			std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported());
		flipDropDownGrid.get()->Opacity(flipToggle.get()->IsEnabled() ? 1 : 0.5);

		// Hide/Show the flip controls container
		flipDropDownContainer.get()->Visibility(
			flipToggle.get()->IsEnabled()
				? Visibility::Visible
				: Visibility::Collapsed);

		TrackingDevices::settings_set_external_flip_is_enabled();
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		for (auto expander : jointExpanderVector)
			expander->EnableSoftwareOrientation(false);

		flipToggle.get()->IsEnabled(false);
		flipDropDown.get()->IsEnabled(false);
		flipDropDownGrid.get()->Opacity(0.5);

		// Hide the flip controls container
		flipDropDownContainer.get()->Visibility(Visibility::Collapsed);

		TrackingDevices::settings_set_external_flip_is_enabled();
	}

	// Load the tracker configuration
	for (auto expander : jointExpanderVector)
		expander->UpdateIsActive();

	// Load auto-spawn and sounds config
	autoSpawnCheckbox->IsChecked(k2app::K2Settings.autoSpawnEnabledJoints);
	enableSoundsCheckbox->IsChecked(k2app::K2Settings.enableAppSounds);
	soundsVolumeSlider.get()->Value(k2app::K2Settings.appSoundsVolume);

	// Enable/Disable Ext/Flip
	TrackingDevices::settings_set_external_flip_is_enabled();

	// Load tracker settings/enabled
	TrackingDevices::settings_trackersConfig_UpdateIsEnabled();

	// Notify of the setup end
	settings_localInitFinished = true;
}

void Amethyst::implementation::SettingsPage::AutoSpawn_Checked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	k2app::K2Settings.autoSpawnEnabledJoints = true;

	// Save settings
	k2app::K2Settings.saveSettings();

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::ToggleOn);
}


void Amethyst::implementation::SettingsPage::AutoSpawn_Unchecked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	k2app::K2Settings.autoSpawnEnabledJoints = false;
	// Save settings
	k2app::K2Settings.saveSettings();

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::ToggleOff);
}


void Amethyst::implementation::SettingsPage::EnableSounds_Checked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	// Turn sounds on
	k2app::K2Settings.enableAppSounds = true;

	// Save settings
	k2app::K2Settings.saveSettings();

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::ToggleOn);
}


void Amethyst::implementation::SettingsPage::EnableSounds_Unchecked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::ToggleOff);

	// Turn sounds on
	k2app::K2Settings.enableAppSounds = false;

	// Save settings
	k2app::K2Settings.saveSettings();
}


void Amethyst::implementation::SettingsPage::SoundsVolumeSlider_ValueChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::Primitives::RangeBaseValueChangedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	// Change sounds level
	k2app::K2Settings.appSoundsVolume =
		k2app::shared::settings::soundsVolumeSlider.get()->Value();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void Amethyst::implementation::SettingsPage::CalibrateExternalFlipMenuFlyoutItem_Click(
	const Windows::Foundation::IInspectable& sender,
	const RoutedEventArgs& e)
{
	// Get current yaw angle

	// If the extflip is from Amethyst
	if (k2app::K2Settings.K2TrackersVector[0].isRotationOverridden)
	{
		k2app::K2Settings.externalFlipCalibrationYaw =
			EigenUtils::RotationProjectedYaw( // Overriden tracker
				k2app::interfacing::vrPlayspaceOrientationQuaternion.inverse() * // VR space offset
				k2app::K2Settings.K2TrackersVector[0].pose.orientation); // Raw orientation
	}
	// If it's from an external tracker
	else
	{
		k2app::K2Settings.externalFlipCalibrationYaw =
			EigenUtils::RotationProjectedYaw( // External tracker
				k2app::interfacing::getVRTrackerPoseCalibrated("waist").second);
	}

	LOG(INFO) << "Captured yaw for external flip: " <<
		radiansToDegrees(k2app::K2Settings.externalFlipCalibrationYaw) << "rad";
	k2app::K2Settings.saveSettings();
}


void Amethyst::implementation::SettingsPage::FlipDropDown_Expanding(
	const Controls::Expander& sender,
	const Controls::ExpanderExpandingEventArgs& args)
{
	if (!k2app::shared::settings::settings_localInitFinished)return; // Don't even try if we're not set up yet

	// Enable/Disable ExtFlip
	TrackingDevices::settings_set_external_flip_is_enabled();
	TrackingDevices::settings_trackersConfig_UpdateIsEnabled();

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Show);
}


void Amethyst::implementation::SettingsPage::FlipToggle_Toggled(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	// Cache flip to settings and save
	k2app::K2Settings.isFlipEnabled =
		k2app::shared::settings::flipToggle->IsOn(); // Checked?

	TrackingDevices::settings_set_external_flip_is_enabled();
	TrackingDevices::settings_trackersConfig_UpdateIsEnabled();

	// Optionally show the binding teaching tip
	if (!k2app::K2Settings.teachingTipShown_Flip &&
		k2app::interfacing::currentPageTag == L"settings")
	{
		auto _header =
			k2app::interfacing::LocalizedResourceWString(
				L"SettingsPage", L"Tips/FlipToggle/Header");

		// Change the tip depending on the currently connected controllers
		char _controller_model[1024];
		vr::VRSystem()->GetStringTrackedDeviceProperty(
			vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(
				vr::ETrackedControllerRole::TrackedControllerRole_LeftHand),
			vr::ETrackedDeviceProperty::Prop_ModelNumber_String,
			_controller_model, std::size(_controller_model));

		// For the ""s operator
		using namespace std::string_literals;

		if (k2app::interfacing::findStringIC(_controller_model, "knuckles") ||
			k2app::interfacing::findStringIC(_controller_model, "index"))
			k2app::interfacing::stringReplaceAll(_header, L"{0}"s,
			                                     k2app::interfacing::LocalizedResourceWString(
				                                     L"SettingsPage",
				                                     L"Tips/FlipToggle/Buttons/Index"));

		else if (k2app::interfacing::findStringIC(_controller_model, "vive"))
			k2app::interfacing::stringReplaceAll(_header, L"{0}"s,
			                                     k2app::interfacing::LocalizedResourceWString(
				                                     L"SettingsPage",
				                                     L"Tips/FlipToggle/Buttons/VIVE"));

		else if (k2app::interfacing::findStringIC(_controller_model, "mr"))
			k2app::interfacing::stringReplaceAll(_header, L"{0}"s,
			                                     k2app::interfacing::LocalizedResourceWString(
				                                     L"SettingsPage",
				                                     L"Tips/FlipToggle/Buttons/WMR"));

		else
			k2app::interfacing::stringReplaceAll(_header, L"{0}"s,
			                                     k2app::interfacing::LocalizedResourceWString(
				                                     L"SettingsPage",
				                                     L"Tips/FlipToggle/Buttons/Oculus"));

		ToggleFlipTeachingTip().Title(_header.c_str());
		ToggleFlipTeachingTip().Subtitle(
			k2app::interfacing::LocalizedResourceWString(
				L"SettingsPage",
				L"Tips/FlipToggle/Footer").c_str());

		ToggleFlipTeachingTip().TailVisibility(Controls::TeachingTipTailVisibility::Collapsed);
		ToggleFlipTeachingTip().IsOpen(true);

		k2app::K2Settings.teachingTipShown_Flip = true;
		k2app::K2Settings.saveSettings();
	}

	k2app::K2Settings.saveSettings();

	// Play a sound
	playAppSound(k2app::K2Settings.isFlipEnabled
		             ? k2app::interfacing::sounds::AppSounds::ToggleOn
		             : k2app::interfacing::sounds::AppSounds::ToggleOff);
}


void Amethyst::implementation::SettingsPage::AutoStartFlyout_Opening(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
	k2app::shared::settings::autoStartCheckBox->IsChecked(
		vr::VRApplications()->GetApplicationAutoLaunch("KinectToVR.Amethyst"));

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Show);
}


void Amethyst::implementation::SettingsPage::AutoStartCheckBox_Checked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	k2app::interfacing::installApplicationManifest(); // Just in case

	const auto app_error = vr::VRApplications()->
		SetApplicationAutoLaunch("KinectToVR.Amethyst", true);

	if (app_error != vr::VRApplicationError_None)
		LOG(WARNING) << "Amethyst manifest not installed! Error:  " <<
			vr::VRApplications()->GetApplicationsErrorNameFromEnum(app_error);

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::ToggleOn);
}


void Amethyst::implementation::SettingsPage::AutoStartCheckBox_Unchecked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	k2app::interfacing::installApplicationManifest(); // Just in case

	const auto app_error = vr::VRApplications()->
		SetApplicationAutoLaunch("KinectToVR.Amethyst", false);

	if (app_error != vr::VRApplicationError_None)
		LOG(WARNING) << "Amethyst manifest not installed! Error:  " <<
			vr::VRApplications()->GetApplicationsErrorNameFromEnum(app_error);

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::ToggleOff);
}


void Amethyst::implementation::SettingsPage::ReManifestButton_Click(
	const Controls::SplitButton& sender,
	const Controls::SplitButtonClickEventArgs& args)
{
	switch (k2app::interfacing::installApplicationManifest())
	{
	// Not found failure
	case 0:
		{
			k2app::shared::settings::setErrorFlyoutText->Text(
				k2app::interfacing::LocalizedResourceWString(L"SettingsPage", L"ReManifest/Error/NotFound"));

			Controls::Primitives::FlyoutShowOptions _opt;
			_opt.Placement(Controls::Primitives::FlyoutPlacementMode::RightEdgeAlignedBottom);
			SetErrorFlyout().ShowAt(ReManifestButton(), _opt);
			break;
		}
	// Generic success
	case 1:
		break;
	// SteamVR failure
	case 2:
		{
			k2app::shared::settings::setErrorFlyoutText->Text(
				k2app::interfacing::LocalizedResourceWString(L"SettingsPage", L"ReManifest/Error/Other"));

			Controls::Primitives::FlyoutShowOptions _opt;
			_opt.Placement(Controls::Primitives::FlyoutPlacementMode::RightEdgeAlignedBottom);
			SetErrorFlyout().ShowAt(ReManifestButton(), _opt);
			break;
		}
	}

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);
}


void Amethyst::implementation::SettingsPage::ReRegisterButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (exists(k2app::interfacing::GetProgramLocation().parent_path() / "K2CrashHandler" / "K2CrashHandler.exe"))
	{
		std::thread([]
		{
			ShellExecuteA(nullptr, "open",
			              (k2app::interfacing::GetProgramLocation().parent_path() / "K2CrashHandler" /
				              "K2CrashHandler.exe ")
			              .string().c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
		}).detach();
	}
	else
	{
		LOG(WARNING) << "Crash handler exe (./K2CrashHandler/K2CrashHandler.exe) not found!";

		k2app::shared::settings::setErrorFlyoutText->Text(
			k2app::interfacing::LocalizedResourceWString(L"SettingsPage", L"ReRegister/Error/NotFound"));

		Controls::Primitives::FlyoutShowOptions _opt;
		_opt.Placement(Controls::Primitives::FlyoutPlacementMode::RightEdgeAlignedBottom);
		SetErrorFlyout().ShowAt(ReRegisterButton(), _opt);
	}

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);
}


void Amethyst::implementation::SettingsPage::DismissSetErrorButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	SetErrorFlyout().Hide();
}


void Amethyst::implementation::SettingsPage::LearnAboutFiltersButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	DimGrid().Opacity(0.35);
	DimGrid().IsHitTestVisible(true);

	Controls::Primitives::FlyoutShowOptions options;
	options.Placement(Controls::Primitives::FlyoutPlacementMode::Full);
	options.ShowMode(Controls::Primitives::FlyoutShowMode::Transient);

	LearnAboutFiltersFlyout().ShowAt(LearnAboutFiltersButton(), options);
}


void Amethyst::implementation::SettingsPage::LearnAboutFiltersFlyout_Closed(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
	DimGrid().Opacity(0.0);
	DimGrid().IsHitTestVisible(false);
}


// Global scope to spare creating new ones each click
std::optional<Controls::MenuFlyout> settings_trackerConfigFlyout = std::nullopt;

void Amethyst::implementation::SettingsPage::TrackerConfigButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	settings_trackerConfigFlyout = Controls::MenuFlyout();

	enum i_tracker_list
	{
		Tracker_Waist,
		Tracker_LeftFoot,
		Tracker_RightFoot,
		Tracker_LeftElbow,
		Tracker_RightElbow,
		Tracker_LeftKnee,
		Tracker_RightKnee,

		Tracker_Chest,
		Tracker_LeftShoulder,
		Tracker_RightShoulder,
		Tracker_Handed,
		Tracker_Camera,
		Tracker_Keyboard
	};

	std::map<i_tracker_list, ktvr::ITrackerType> tracker_map
	{
		{Tracker_Handed, ktvr::ITrackerType::Tracker_Handed},
		{Tracker_LeftFoot, ktvr::ITrackerType::Tracker_LeftFoot},
		{Tracker_RightFoot, ktvr::ITrackerType::Tracker_RightFoot},
		{Tracker_LeftShoulder, ktvr::ITrackerType::Tracker_LeftShoulder},
		{Tracker_RightShoulder, ktvr::ITrackerType::Tracker_RightShoulder},
		{Tracker_LeftElbow, ktvr::ITrackerType::Tracker_LeftElbow},
		{Tracker_RightElbow, ktvr::ITrackerType::Tracker_RightElbow},
		{Tracker_LeftKnee, ktvr::ITrackerType::Tracker_LeftKnee},
		{Tracker_RightKnee, ktvr::ITrackerType::Tracker_RightKnee},
		{Tracker_Waist, ktvr::ITrackerType::Tracker_Waist},
		{Tracker_Chest, ktvr::ITrackerType::Tracker_Chest},
		{Tracker_Camera, ktvr::ITrackerType::Tracker_Camera},
		{Tracker_Keyboard, ktvr::ITrackerType::Tracker_Keyboard}
	};

	for (uint32_t index = Tracker_Chest;
	     index <= static_cast<int>(Tracker_Keyboard); index++)
	{
		// Back the current tracker's role up
		ktvr::ITrackerType current_tracker =
			tracker_map[static_cast<i_tracker_list>(index)];

		auto menuTrackerToggleItem = Controls::ToggleMenuFlyoutItem();

		menuTrackerToggleItem.Text(
			k2app::interfacing::LocalizedResourceWString(
				L"SharedStrings", L"Joints/Enum/" +
				std::to_wstring(static_cast<int>(current_tracker))));

		bool isEnabled = (index >= static_cast<int>(
			     Tracker_Chest)),
		     isChecked = (index < static_cast<int>(
			     Tracker_Chest));

		for (const auto& tracker : k2app::K2Settings.K2TrackersVector)
			if (tracker.tracker == tracker_map[static_cast<i_tracker_list>(index)])
				isChecked = true; // Tracker is enabled

		menuTrackerToggleItem.IsEnabled(isEnabled);
		menuTrackerToggleItem.IsChecked(isChecked);

		menuTrackerToggleItem.Click(
			[&, index, tracker_map, current_tracker, this]
		(const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
		-> Windows::Foundation::IAsyncAction
			{
				// Notify of the setup end
				k2app::shared::settings::settings_localInitFinished = false;

				// Create a new tracker / Remove the unchecked one
				if (sender.as<Controls::ToggleMenuFlyoutItem>().IsChecked())
				{
					// If not checked, add a new tracker
					k2app::K2Settings.K2TrackersVector.push_back(k2app::K2AppTracker());

					// Set the newly created tracker up
					k2app::K2Settings.K2TrackersVector.back().tracker = current_tracker;
					k2app::K2Settings.K2TrackersVector.back().data.serial =
						k2app::ITrackerType_Role_Serial[k2app::K2Settings.K2TrackersVector.back().tracker];
				}
				else
				// If the tracker was unchecked
					for (uint32_t _t = 0; _t < k2app::K2Settings.K2TrackersVector.size(); _t++)
						if (k2app::K2Settings.K2TrackersVector[_t].tracker == current_tracker)
						{
							// Cache the tracker's state
							const bool trackerState = k2app::K2Settings.K2TrackersVector.at(_t).data.isActive;

							// Make actual changes
							if (trackerState && k2app::interfacing::K2AppTrackersInitialized)
								ktvr::set_tracker_state<false>(
									k2app::K2Settings.K2TrackersVector.at(_t).tracker, false);

							// Sleep on UI's background
							apartment_context _ui_thread;
							co_await resume_background();
							Sleep(20);
							co_await _ui_thread;

							k2app::K2Settings.K2TrackersVector.erase(
								k2app::K2Settings.K2TrackersVector.begin() + _t);

							// Check if we've disabled any joints from spawning and disable their mods
							k2app::interfacing::devices_check_disabled_joints();

							// If the tracker was on and then removed
							if (trackerState)
							{
								// Boiler
								k2app::shared::settings::settings_localInitFinished = true;

								// Show the notifications and rebuild
								TrackingDevices::settings_trackersConfigChanged();

								// Boiler end
								k2app::shared::settings::settings_localInitFinished = false;
							}

							// Save settings
							k2app::K2Settings.saveSettings();
						}

				// Rebuild the joint expander stack
				using namespace k2app::shared::settings;

				LOG(INFO) << "Rebuilding joint expanders... this may take a while...";
				jointExpanderHostStackPanel->Transitions().Append(Media::Animation::ContentThemeTransition());

				jointExpanderVector.clear();
				jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(std::vector{
					&k2app::K2Settings.K2TrackersVector[0]
				})));

				if (k2app::K2Settings.useTrackerPairs)
				{
					LOG(INFO) << "UseTrackerPairs is set to true: Appending the default expanders as pairs...";

					jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(
						std::vector{&k2app::K2Settings.K2TrackersVector[1], &k2app::K2Settings.K2TrackersVector[2]},
						k2app::interfacing::LocalizedResourceWString(
							L"SharedStrings", L"Joints/Pairs/Feet"))));

					jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(
						std::vector{&k2app::K2Settings.K2TrackersVector[3], &k2app::K2Settings.K2TrackersVector[4]},
						k2app::interfacing::LocalizedResourceWString(
							L"SharedStrings", L"Joints/Pairs/Elbows"))));

					jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(
						std::vector{&k2app::K2Settings.K2TrackersVector[5], &k2app::K2Settings.K2TrackersVector[6]},
						k2app::interfacing::LocalizedResourceWString(
							L"SharedStrings", L"Joints/Pairs/Knees"))));
				}

				LOG(INFO) << "Appending additional expanders (if they exist)...";

				// k2app::K2Settings.useTrackerPairs ? 7 : 1 means that if pairs have
				// already been appended, we'll start after them, and if not -
				// - we'll append them as individual tracker/joint expanders

				for (uint32_t ind = (k2app::K2Settings.useTrackerPairs ? 7 : 1);
				     ind < k2app::K2Settings.K2TrackersVector.size(); ind++)
					jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(std::vector{
						&k2app::K2Settings.K2TrackersVector[ind]
					})));

				LOG(INFO) << "Clearing the appended expanders (UI Node)";
				settings_safe_clear(jointExpanderHostStackPanel);

				LOG(INFO) << "Appending the new expanders to the UI Node";

				int _expander_number = (k2app::K2Settings.useTrackerPairs ? 1 : 2); // For separators
				for (auto expander : jointExpanderVector)
				{
					// Append the expander
					jointExpanderHostStackPanel->Children().Append(*expander->Container());

					// Append the separator (optionally)
					if (_expander_number >= 2 && jointExpanderVector.back() != expander)
					{
						auto separator = Controls::MenuFlyoutSeparator();
						separator.Margin({10, 10, 10, 0});

						Media::Animation::TransitionCollection c_transition_collection;
						c_transition_collection.Append(Media::Animation::RepositionThemeTransition());
						separator.Transitions(c_transition_collection);

						jointExpanderHostStackPanel->Children().Append(separator);
						_expander_number = 1;
					}
					else _expander_number++;
				}

				if (const auto& trackingDevice = TrackingDevices::getCurrentDevice();
					trackingDevice.index() == 0)
				{
					// Kinect Basis
					const bool _sup = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->
						isAppOrientationSupported();

					for (auto expander : jointExpanderVector)
						expander->EnableSoftwareOrientation(_sup);

					flipToggle.get()->IsEnabled(
						std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported());
					flipDropDown.get()->IsEnabled(k2app::K2Settings.isFlipEnabled &&
						std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported());
					flipDropDownGrid.get()->Opacity(flipToggle.get()->IsEnabled() ? 1 : 0.5);

					// Hide/Show the flip controls container
					flipDropDownContainer.get()->Visibility(
						flipToggle.get()->IsEnabled()
							? Visibility::Visible
							: Visibility::Collapsed);

					TrackingDevices::settings_set_external_flip_is_enabled();
				}
				else if (trackingDevice.index() == 1)
				{
					// Joints Basis
					for (auto expander : jointExpanderVector)
						expander->EnableSoftwareOrientation(false);

					flipToggle.get()->IsEnabled(false);
					flipDropDown.get()->IsEnabled(false);
					flipDropDownGrid.get()->Opacity(0.5);

					// Hide the flip controls container
					flipDropDownContainer.get()->Visibility(Visibility::Collapsed);

					TrackingDevices::settings_set_external_flip_is_enabled();
				}

				// Load the tracker configuration
				for (auto expander : jointExpanderVector)
					expander->UpdateIsActive();

				// Enable/Disable combos
				TrackingDevices::settings_trackersConfig_UpdateIsEnabled();

				// Enable/Disable ExtFlip
				TrackingDevices::settings_set_external_flip_is_enabled();

				// Notify of the setup end
				settings_localInitFinished = true;
				k2app::K2Settings.saveSettings();

				{
					// Sleep on UI
					apartment_context ui_thread;
					co_await resume_background();
					Sleep(50);
					co_await ui_thread;
				}
				jointExpanderHostStackPanel->Transitions().RemoveAtEnd();

				// Check if any trackers are enabled
				// No std::ranges today...
				bool _find_result = false;
				for (const auto& tracker : k2app::K2Settings.K2TrackersVector)
					if (tracker.data.isActive)_find_result = true;

				// No trackers are enabled, force-enable the waist tracker
				if (!_find_result)
				{
					LOG(WARNING) << "All trackers have been disabled, force-enabling the waist tracker!";

					// Enable the wiast tracker (no need to worry about the dispatcher, we're already inside)
					jointExpanderVector.front()->JointSwitch().get()->IsOn(true);

					// Save settings
					k2app::K2Settings.saveSettings();
				}

				// Request a check for already-added trackers
				LOG(INFO) << "Requesting a check for already-added trackers...";
				k2app::interfacing::alreadyAddedTrackersScanRequested = true;
				co_return;
			});

		// Append the item
		settings_trackerConfigFlyout.value().Items().Append(menuTrackerToggleItem);
	}

	auto menuPairsToggleItem = Controls::ToggleMenuFlyoutItem();
	menuPairsToggleItem.Text(
		k2app::interfacing::LocalizedResourceWString(
			L"SettingsPage", L"Captions/TrackerPairs"));

	menuPairsToggleItem.IsChecked(k2app::K2Settings.useTrackerPairs);
	menuPairsToggleItem.Click(
		[&, this](const Windows::Foundation::IInspectable& sender,
		          const RoutedEventArgs& e) -> Windows::Foundation::IAsyncAction
		{
			// Notify of the setup end
			k2app::shared::settings::settings_localInitFinished = false;

			k2app::K2Settings.useTrackerPairs = sender.as<Controls::ToggleMenuFlyoutItem>().IsChecked();

			// Rebuild the joint expander stack
			using namespace k2app::shared::settings;

			LOG(INFO) << "Rebuilding joint expanders... this may take a while...";
			jointExpanderHostStackPanel->Transitions().Append(Media::Animation::ContentThemeTransition());

			jointExpanderVector.clear();
			jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(std::vector{
				&k2app::K2Settings.K2TrackersVector[0]
			})));

			if (k2app::K2Settings.useTrackerPairs)
			{
				LOG(INFO) << "UseTrackerPairs is set to true: Appending the default expanders as pairs...";

				jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(
					std::vector{&k2app::K2Settings.K2TrackersVector[1], &k2app::K2Settings.K2TrackersVector[2]},
					k2app::interfacing::LocalizedResourceWString(
						L"SharedStrings", L"Joints/Pairs/Feet"))));

				jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(
					std::vector{&k2app::K2Settings.K2TrackersVector[3], &k2app::K2Settings.K2TrackersVector[4]},
					k2app::interfacing::LocalizedResourceWString(
						L"SharedStrings", L"Joints/Pairs/Elbows"))));

				jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(
					std::vector{&k2app::K2Settings.K2TrackersVector[5], &k2app::K2Settings.K2TrackersVector[6]},
					k2app::interfacing::LocalizedResourceWString(
						L"SharedStrings", L"Joints/Pairs/Knees"))));
			}

			LOG(INFO) << "Appending additional expanders (if they exist)...";

			// k2app::K2Settings.useTrackerPairs ? 7 : 1 means that if pairs have
			// already been appended, we'll start after them, and if not -
			// - we'll append them as individual tracker/joint expanders

			for (uint32_t ind = (k2app::K2Settings.useTrackerPairs ? 7 : 1);
			     ind < k2app::K2Settings.K2TrackersVector.size(); ind++)
				jointExpanderVector.push_back(std::move(std::make_shared<Controls::JointExpander>(std::vector{
					&k2app::K2Settings.K2TrackersVector[ind]
				})));

			LOG(INFO) << "Clearing the appended expanders (UI Node)";
			settings_safe_clear(jointExpanderHostStackPanel);

			LOG(INFO) << "Appending the new expanders to the UI Node";

			int _expander_number = (k2app::K2Settings.useTrackerPairs ? 1 : 2); // For separators
			for (auto expander : jointExpanderVector)
			{
				// Append the expander
				jointExpanderHostStackPanel->Children().Append(*expander->Container());

				// Append the separator (optionally)
				if (_expander_number >= 2 && jointExpanderVector.back() != expander)
				{
					auto separator = Controls::MenuFlyoutSeparator();
					separator.Margin({10, 10, 10, 0});

					Media::Animation::TransitionCollection c_transition_collection;
					c_transition_collection.Append(Media::Animation::RepositionThemeTransition());
					separator.Transitions(c_transition_collection);

					jointExpanderHostStackPanel->Children().Append(separator);
					_expander_number = 1;
				}
				else _expander_number++;
			}

			if (const auto& trackingDevice = TrackingDevices::getCurrentDevice();
				trackingDevice.index() == 0)
			{
				// Kinect Basis
				const bool _sup = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->
					isAppOrientationSupported();

				for (auto expander : jointExpanderVector)
					expander->EnableSoftwareOrientation(_sup);

				flipToggle.get()->IsEnabled(
					std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported());
				flipDropDown.get()->IsEnabled(k2app::K2Settings.isFlipEnabled &&
					std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported());
				flipDropDownGrid.get()->Opacity(flipToggle.get()->IsEnabled() ? 1 : 0.5);

				// Hide/Show the flip controls container
				flipDropDownContainer.get()->Visibility(
					flipToggle.get()->IsEnabled()
						? Visibility::Visible
						: Visibility::Collapsed);

				TrackingDevices::settings_set_external_flip_is_enabled();
			}
			else if (trackingDevice.index() == 1)
			{
				// Joints Basis
				for (auto expander : jointExpanderVector)
					expander->EnableSoftwareOrientation(false);

				flipToggle.get()->IsEnabled(false);
				flipDropDown.get()->IsEnabled(false);
				flipDropDownGrid.get()->Opacity(0.5);

				// Hide the flip controls container
				flipDropDownContainer.get()->Visibility(Visibility::Collapsed);

				TrackingDevices::settings_set_external_flip_is_enabled();
			}

			// Load the tracker configuration
			for (auto expander : jointExpanderVector)
				expander->UpdateIsActive();

			// Enable/Disable combos
			TrackingDevices::settings_trackersConfig_UpdateIsEnabled();

			// Enable/Disable ExtFlip
			TrackingDevices::settings_set_external_flip_is_enabled();

			// Notify of the setup end
			settings_localInitFinished = true;
			k2app::K2Settings.saveSettings();
			k2app::K2Settings.readSettings(); // Calls config check

			{
				// Sleep on UI
				apartment_context ui_thread;
				co_await resume_background();
				Sleep(100);
				co_await ui_thread;
			}
			jointExpanderHostStackPanel->Transitions().RemoveAtEnd();
			co_return;
		});

	// Append the item
	settings_trackerConfigFlyout.value().Items().Append(Controls::MenuFlyoutSeparator());
	settings_trackerConfigFlyout.value().Items().Append(menuPairsToggleItem);

	settings_trackerConfigFlyout.value().Placement(Controls::Primitives::FlyoutPlacementMode::LeftEdgeAlignedBottom);
	settings_trackerConfigFlyout.value().ShowMode(Controls::Primitives::FlyoutShowMode::Transient);
	settings_trackerConfigFlyout.value().ShowAt(TrackerConfigButton());

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Show);

	settings_trackerConfigFlyout.value().Closing([&](auto, auto)
	{
		// Play a sound
		playAppSound(k2app::interfacing::sounds::AppSounds::Hide);
	});
}


void Amethyst::implementation::SettingsPage::CheckOverlapsCheckBox_Checked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	k2app::K2Settings.checkForOverlappingTrackers = true;
	k2app::K2Settings.saveSettings();

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::ToggleOn);
}


void Amethyst::implementation::SettingsPage::CheckOverlapsCheckBox_Unchecked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	k2app::K2Settings.checkForOverlappingTrackers = false;
	k2app::K2Settings.saveSettings();

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::ToggleOff);
}


void Amethyst::implementation::SettingsPage::ViewLogsButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	k2app::interfacing::openFolderAndSelectItem(
		k2app::interfacing::thisLogDestination);

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);
}


void Amethyst::implementation::SettingsPage::ManageTrackersTeachingTip_Closed(
	const Controls::TeachingTip& sender,
	const Controls::TeachingTipClosedEventArgs& args)
{
	AddTrackersTeachingTip().TailVisibility(Controls::TeachingTipTailVisibility::Collapsed);
	AddTrackersTeachingTip().IsOpen(true);
}


void Amethyst::implementation::SettingsPage::AddTrackersTeachingTip_Closed(
	const Controls::TeachingTip& sender,
	const Controls::TeachingTipClosedEventArgs& args)
{
	LearnAboutFiltersTeachingTip().TailVisibility(Controls::TeachingTipTailVisibility::Collapsed);
	LearnAboutFiltersTeachingTip().IsOpen(true);
}


Windows::Foundation::IAsyncAction Amethyst::implementation::SettingsPage::LearnAboutFiltersTeachingTip_Closed(
	const Controls::TeachingTip& sender,
	const Controls::TeachingTipClosedEventArgs& args)
{
	PageMainScrollViewer().UpdateLayout();
	PageMainScrollViewer().ChangeView(nullptr,
	                                  PageMainScrollViewer().ExtentHeight(), nullptr);

	apartment_context ui_thread;
	co_await resume_background();
	Sleep(500);
	co_await ui_thread;

	AutoStartTeachingTip().TailVisibility(Controls::TeachingTipTailVisibility::Collapsed);
	AutoStartTeachingTip().IsOpen(true);
}


Windows::Foundation::IAsyncAction Amethyst::implementation::SettingsPage::AutoStartTeachingTip_Closed(
	const Controls::TeachingTip& sender,
	const Controls::TeachingTipClosedEventArgs& args)
{
	// Wait a bit
	{
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(200);
		co_await ui_thread;
	}

	// Navigate to the devices page
	k2app::shared::main::mainNavigationView->SelectedItem(
		k2app::shared::main::mainNavigationView->MenuItems().GetAt(2));
	k2app::shared::main::NavView_Navigate(L"devices", Media::Animation::EntranceNavigationTransitionInfo());

	// Wait a bit
	{
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(500);
		co_await ui_thread;
	}

	// Reset the previous page layout
	PageMainScrollViewer().ScrollToVerticalOffset(0);

	// Show the next tip
	k2app::shared::teaching_tips::devices::devicesListTeachingTip->TailVisibility(
		Controls::TeachingTipTailVisibility::Collapsed);
	k2app::shared::teaching_tips::devices::devicesListTeachingTip->IsOpen(true);
}


void Amethyst::implementation::SettingsPage::ButtonFlyout_Opening(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Show);
}


void Amethyst::implementation::SettingsPage::ButtonFlyout_Closing(
	const Controls::Primitives::FlyoutBase& sender,
	const Controls::Primitives::FlyoutBaseClosingEventArgs& args)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Hide);
}


void Amethyst::implementation::SettingsPage::FlipDropDown_Collapsed
(const Controls::Expander& sender,
 const Controls::ExpanderCollapsedEventArgs& args)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Hide);
}


Windows::Foundation::IAsyncAction
Amethyst::implementation::SettingsPage::LanguageOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)co_return;

	if (LanguageOptionBox().SelectedIndex() < 0)
		LanguageOptionBox().SelectedItem(e.RemovedItems().GetAt(0));

	// Overwrite the current language code
	k2app::K2Settings.appLanguage = _language_codes_enum[LanguageOptionBox().SelectedIndex()];

	// Save made changes
	k2app::K2Settings.saveSettings();

	// Reload
	k2app::interfacing::LoadJSONStringResources_English();
	k2app::interfacing::LoadJSONStringResources(k2app::K2Settings.appLanguage);

	// Request page reloads
	k2app::shared::semaphores::semaphore_ReloadPage_MainWindow.release();
	k2app::shared::semaphores::semaphore_ReloadPage_GeneralPage.release();
	k2app::shared::semaphores::semaphore_ReloadPage_SettingsPage.release();
	k2app::shared::semaphores::semaphore_ReloadPage_DevicesPage.release();
	k2app::shared::semaphores::semaphore_ReloadPage_InfoPage.release();
}


Windows::Foundation::IAsyncAction
Amethyst::implementation::SettingsPage::AppThemeOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)co_return;

	if (AppThemeOptionBox().SelectedIndex() < 0)
		AppThemeOptionBox().SelectedItem(e.RemovedItems().GetAt(0));

	// Overwrite the current theme
	k2app::K2Settings.appTheme = AppThemeOptionBox().SelectedIndex();

	switch (k2app::K2Settings.appTheme)
	{
	case 2:
		{
			k2app::shared::main::mainNavigationView->XamlRoot()
			                                       .Content().as<Controls::Grid>()
			                                       .RequestedTheme(ElementTheme::Light);
			break;
		}
	case 1:
		{
			k2app::shared::main::mainNavigationView->XamlRoot()
			                                       .Content().as<Controls::Grid>()
			                                       .RequestedTheme(ElementTheme::Dark);
			break;
		}
	case 0:
	default:
		{
			k2app::shared::main::mainNavigationView->XamlRoot()
			                                       .Content().as<Controls::Grid>()
			                                       .RequestedTheme(ElementTheme::Default);
			break;
		}
	}

	// Save made changes
	k2app::K2Settings.saveSettings();
}


void Amethyst::implementation::SettingsPage::OptionBox_DropDownOpened(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Show);
}


void Amethyst::implementation::SettingsPage::OptionBox_DropDownClosed(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Hide);
}
