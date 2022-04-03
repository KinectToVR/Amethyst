#include "pch.h"
#include "SettingsPage.xaml.h"

#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

// Helper local variables
bool settings_localInitFinished = false;

namespace winrt::KinectToVR::implementation
{
	SettingsPage::SettingsPage()
	{
		InitializeComponent();

		// Cache needed UI elements
		using namespace ::k2app::shared::settings;

		restartButton = std::make_shared<Controls::Button>(RestartButton());

		waistPositionFilterOptionBox =
			std::make_shared<Controls::ComboBox>(WaistPositionFilterOptionBox());
		waistRotationFilterOptionBox =
			std::make_shared<Controls::ComboBox>(WaistRotationFilterOptionBox());
		feetPositionFilterOptionBox =
			std::make_shared<Controls::ComboBox>(FeetPositionFilterOptionBox());
		feetRotationFilterOptionBox =
			std::make_shared<Controls::ComboBox>(FeetRotationFilterOptionBox());
		kneePositionFilterOptionBox =
			std::make_shared<Controls::ComboBox>(KneePositionFilterOptionBox());
		kneeRotationFilterOptionBox =
			std::make_shared<Controls::ComboBox>(KneeRotationFilterOptionBox());
		elbowsPositionFilterOptionBox =
			std::make_shared<Controls::ComboBox>(ElbowsPositionFilterOptionBox());
		elbowsRotationFilterOptionBox =
			std::make_shared<Controls::ComboBox>(ElbowsRotationFilterOptionBox());

		softwareRotationItem =
			std::make_shared<Controls::ComboBoxItem>(SoftwareRotationItem());

		flipCheckBox = std::make_shared<Controls::CheckBox>(FlipCheckBox());
		externalFlipCheckBox = std::make_shared<Controls::CheckBox>(ExternalFlipCheckBox());
		autoSpawnCheckbox = std::make_shared<Controls::CheckBox>(AutoSpawnCheckBox());
		enableSoundsCheckbox = std::make_shared<Controls::CheckBox>(SoundsEnabledCheckBox());

		flipCheckBoxLabel = std::make_shared<Controls::TextBlock>(FlipCheckBoxLabel());
		externalFlipCheckBoxLabel = std::make_shared<Controls::TextBlock>(ExternalFlipCheckBoxLabel());

		waistTrackerEnabledToggle = std::make_shared<Controls::ToggleSwitch>(WaistTrackerEnabledToggle());
		feetTrackersEnabledToggle = std::make_shared<Controls::ToggleSwitch>(FeetTrackersEnabledToggle());
		kneeTrackersEnabledToggle = std::make_shared<Controls::ToggleSwitch>(KneeTrackersEnabledToggle());
		elbowTrackersEnabledToggle = std::make_shared<Controls::ToggleSwitch>(ElbowTrackersEnabledToggle());

		waistDropDown = std::make_shared<Controls::Expander>(WaistDropDown());
		feetDropDown = std::make_shared<Controls::Expander>(FeetDropDown());
		kneesDropDown = std::make_shared<Controls::Expander>(KneesDropDown());
		elbowsDropDown = std::make_shared<Controls::Expander>(ElbowsDropDown());

		soundsVolumeSlider = std::make_shared<Controls::Slider>(SoundsVolumeSlider());

		externalFlipStackPanel = std::make_shared<Controls::StackPanel>(ExternalFlipStackPanel());
	}
}

void trackersConfig_UpdateIsEnabled()
{
	// Make expander opacity .5 and collapse it
	// to imitate that it's disabled

	// Waist
	if (!k2app::K2Settings.isJointPairEnabled[0])
	{
		k2app::shared::settings::waistDropDown.get()->IsEnabled(false);
		k2app::shared::settings::waistDropDown.get()->IsExpanded(false);
	}
	else
		k2app::shared::settings::waistDropDown.get()->IsEnabled(true);

	// Feet
	if (!k2app::K2Settings.isJointPairEnabled[1])
	{
		k2app::shared::settings::feetDropDown.get()->IsEnabled(false);
		k2app::shared::settings::feetDropDown.get()->IsExpanded(false);
	}
	else
		k2app::shared::settings::feetDropDown.get()->IsEnabled(true);

	// Elbows
	if (!k2app::K2Settings.isJointPairEnabled[2])
	{
		k2app::shared::settings::elbowsDropDown.get()->IsEnabled(false);
		k2app::shared::settings::elbowsDropDown.get()->IsExpanded(false);
	}
	else
		k2app::shared::settings::elbowsDropDown.get()->IsEnabled(true);

	// Knees
	if (!k2app::K2Settings.isJointPairEnabled[3])
	{
		k2app::shared::settings::kneesDropDown.get()->IsEnabled(false);
		k2app::shared::settings::kneesDropDown.get()->IsExpanded(false);
	}
	else
		k2app::shared::settings::kneesDropDown.get()->IsEnabled(true);
}

void trackersConfigChanged()
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	// If this is the first time, also show the notification
	if (!k2app::shared::settings::restartButton.get()->IsEnabled())
		k2app::interfacing::ShowToast("Trackers configuration has changed",
		                              "Restart SteamVR for changes to take effect");

	// If all trackers were turned off then SCREAM
	if (std::ranges::all_of(
		k2app::K2Settings.isJointPairEnabled,
		[](bool const& i) { return !i; }
	))
		k2app::interfacing::ShowToast("YOU'VE JUST DISABLED ALL TRACKERS",
		                              "WHAT SORT OF A TOTAL FUCKING LIFE FAILURE ARE YOU TO DO THAT YOU STUPID BITCH LOSER?!?!");

	// Compare with saved settings and unlock the restart
	k2app::shared::settings::restartButton.get()->IsEnabled(true);

	// Enable/Disable combos
	trackersConfig_UpdateIsEnabled();

	// Enable/Disable ExtFlip
	TrackingDevices::settings_set_external_flip_is_enabled();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::FlipCheckBox_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	// Cache flip to settings and save
	k2app::K2Settings.isFlipEnabled = true; // Checked
	TrackingDevices::settings_set_external_flip_is_enabled();
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::FlipCheckBox_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	// Cache flip to settings and save
	k2app::K2Settings.isFlipEnabled = false; // Unchecked
	TrackingDevices::settings_set_external_flip_is_enabled();
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::ExternalFlipCheckBox_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	// Cache flip to settings and save
	k2app::K2Settings.isExternalFlipEnabled = true; // Checked
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::ExternalFlipCheckBox_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	// Cache flip to settings and save
	k2app::K2Settings.isExternalFlipEnabled = false; // Unchecked
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::RestartButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	ktvr::request_vr_restart<false>("SteamVR needs to be restarted to enable/disable trackers properly.");
}


void winrt::KinectToVR::implementation::SettingsPage::ResetButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Read settings after reset
	k2app::K2Settings = k2app::K2AppSettings(); // Reset settings
	k2app::K2Settings.saveSettings(); // Save empty settings

	/* Restart app */

	// Literals
	using namespace std::string_literals;

	// Get current caller path
	const LPSTR fileName = new CHAR[MAX_PATH + 1];
	const DWORD charsWritten = GetModuleFileNameA(nullptr, fileName, MAX_PATH + 1);

	// If we've found who asked
	if (charsWritten != 0)
	{
		// Compose the restart command: sleep 3 seconds and start the same process
		const std::string _cmd =
			"powershell Start-Process powershell -ArgumentList 'Start-Sleep -Seconds 3; " +
			"Start-Process -WorkingDirectory (Split-Path -Path (Resolve-Path \""s +
			fileName +
			"\")) -filepath \"" +
			fileName +
			"\"' -WindowStyle hidden";

		// Log the caller
		LOG(INFO) << "The current caller process is: "s + fileName;
		LOG(INFO) << "Restart command used: "s + _cmd;


		// Restart the app
		if (WinExec(_cmd.c_str(), SW_HIDE) != NO_ERROR)
		{
			LOG(ERROR) << "App will not be restarted due to new process creation error.";
			k2app::interfacing::ShowToast("We couldn't restart KinectToVR for you!",
			                              "Please try restarting it manually");
			return;
		}

		// Exit the app
		LOG(INFO) << "Configuration has been reset, exiting...";
		k2app::interfacing::ShowToast("KinectToVR restart pending...",
		                              "The app will restart automatically in 3 seconds");
		//exit(0);
		Application::Current().Exit();
	}
	else
	{
		LOG(ERROR) << "App will not be restarted due to caller process identification error.";
		k2app::interfacing::ShowToast("We couldn't restart KinectToVR for you!", "Please try restarting it manually");
	}
}


void winrt::KinectToVR::implementation::SettingsPage::SettingsPage_Loaded(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	using namespace ::k2app::shared::settings;

	// Select saved flip, position and rotation options
	flipCheckBox.get()->IsChecked(k2app::K2Settings.isFlipEnabled);
	externalFlipCheckBox.get()->IsChecked(k2app::K2Settings.isExternalFlipEnabled);

	// Waist (pos)
	waistPositionFilterOptionBox.get()->SelectedIndex(
		k2app::K2Settings.positionTrackingFilterOptions[0]);

	// Feet (pos)
	waistPositionFilterOptionBox.get()->SelectedIndex(
		k2app::K2Settings.positionTrackingFilterOptions[1]);

	// Elbows (pos)
	waistPositionFilterOptionBox.get()->SelectedIndex(
		k2app::K2Settings.positionTrackingFilterOptions[3]);

	// Knees (pos)
	waistPositionFilterOptionBox.get()->SelectedIndex(
		k2app::K2Settings.positionTrackingFilterOptions[5]);

	// Feet
	feetRotationFilterOptionBox.get()->SelectedIndex(
		k2app::K2Settings.jointRotationTrackingOption[1]);

	// Waist
	switch (k2app::K2Settings.jointRotationTrackingOption[0]) // Waist
	{
	default:
		waistRotationFilterOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
	// Device
	case k2app::k2_DeviceInferredRotation:
		waistRotationFilterOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
		break;
	// Software
	case k2app::k2_SoftwareCalculatedRotation: // If somehow...
		waistRotationFilterOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
		break;
	// Headset
	case k2app::k2_FollowHMDRotation:
		waistRotationFilterOptionBox.get()->SelectedIndex(k2app::k2_FollowHMDRotation - 1); // -1 to skip app-based rot
		break;
	// Disable
	case k2app::k2_DisableJointRotation:
		waistRotationFilterOptionBox.get()->SelectedIndex(k2app::k2_DisableJointRotation - 1);
	// -1 to skip app-based rot
		break;
	}

	// Elbows
	switch (k2app::K2Settings.jointRotationTrackingOption[3]) // LElbow
	{
	default:
		elbowsRotationFilterOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
	// Device
	case k2app::k2_DeviceInferredRotation:
		elbowsRotationFilterOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
		break;
	// Software
	case k2app::k2_SoftwareCalculatedRotation: // If somehow...
		elbowsRotationFilterOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
		break;
	// Headset
	case k2app::k2_FollowHMDRotation:
		elbowsRotationFilterOptionBox.get()->SelectedIndex(k2app::k2_FollowHMDRotation - 1); // -1 to skip app-based rot
		break;
	// Disable
	case k2app::k2_DisableJointRotation:
		elbowsRotationFilterOptionBox.get()->SelectedIndex(k2app::k2_DisableJointRotation - 1);
	// -1 to skip app-based rot
		break;
	}

	// Knees
	switch (k2app::K2Settings.jointRotationTrackingOption[5]) // LKnee
	{
	default:
		kneeRotationFilterOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
	// Device
	case k2app::k2_DeviceInferredRotation:
		kneeRotationFilterOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
		break;
	// Software
	case k2app::k2_SoftwareCalculatedRotation: // If somehow...
		kneeRotationFilterOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
		break;
	// Headset
	case k2app::k2_FollowHMDRotation:
		kneeRotationFilterOptionBox.get()->SelectedIndex(k2app::k2_FollowHMDRotation - 1); // -1 to skip app-based rot
		break;
	// Disable
	case k2app::k2_DisableJointRotation:
		kneeRotationFilterOptionBox.get()->SelectedIndex(k2app::k2_DisableJointRotation - 1);
	// -1 to skip app-based rot
		break;
	}

	if (auto const& trackingDevice = TrackingDevices::getCurrentDevice(); trackingDevice.index() == 0)
	{
		// Kinect Basis
		softwareRotationItem.get()->IsEnabled(
			std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isAppOrientationSupported());
		flipCheckBox.get()->IsEnabled(
			std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported());
		flipCheckBoxLabel.get()->Opacity(flipCheckBox.get()->IsEnabled() ? 1 : 0.5);
		TrackingDevices::settings_set_external_flip_is_enabled();
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		softwareRotationItem.get()->IsEnabled(false);
		flipCheckBox.get()->IsEnabled(false);
		flipCheckBoxLabel.get()->Opacity(0.5);
		TrackingDevices::settings_set_external_flip_is_enabled(false);
	}

	// Load the tracker configuration
	waistTrackerEnabledToggle.get()->IsOn(k2app::K2Settings.isJointPairEnabled[0]);
	feetTrackersEnabledToggle.get()->IsOn(k2app::K2Settings.isJointPairEnabled[1]);
	elbowTrackersEnabledToggle.get()->IsOn(k2app::K2Settings.isJointPairEnabled[2]);
	kneeTrackersEnabledToggle.get()->IsOn(k2app::K2Settings.isJointPairEnabled[3]);

	// Load auto-spawn and sounds config
	autoSpawnCheckbox->IsChecked(k2app::K2Settings.autoSpawnEnabledJoints);
	enableSoundsCheckbox->IsChecked(k2app::K2Settings.enableAppSounds);
	soundsVolumeSlider.get()->Value(k2app::K2Settings.appSoundsVolume);

	// Load tracker settings/enabled
	trackersConfig_UpdateIsEnabled();

	// Hide/Show external flip depending on exp_options
	externalFlipStackPanel->Visibility(k2app::shared::main::consoleItem.get()->Visibility());

	// Notify of the setup end
	settings_localInitFinished = true;
}

void winrt::KinectToVR::implementation::SettingsPage::AutoSpawn_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.autoSpawnEnabledJoints = true;
	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::AutoSpawn_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.autoSpawnEnabledJoints = false;
	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::EnableSounds_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	// Turn sounds on
	k2app::K2Settings.enableAppSounds = true;
	ElementSoundPlayer::State(ElementSoundPlayerState::On);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::EnableSounds_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	// Turn sounds on
	k2app::K2Settings.enableAppSounds = false;
	ElementSoundPlayer::State(ElementSoundPlayerState::Off);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::SoundsVolumeSlider_ValueChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	// Change sounds level
	k2app::K2Settings.appSoundsVolume = k2app::shared::settings::soundsVolumeSlider.get()->Value();
	ElementSoundPlayer::Volume(std::clamp(
		double(k2app::K2Settings.appSoundsVolume) / 100.0, 0.0, 100.0));

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::CalibrateExternalFlipMenuFlyoutItem_Click(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	k2app::K2Settings.externalFlipCalibrationYaw =
		EigenUtils::QuatToEulers(
			k2app::interfacing::K2TrackersVector.at(0).pose.orientation).y();

	LOG(INFO) << "Captured yaw for external flip: " <<
		radiansToDegrees(k2app::K2Settings.externalFlipCalibrationYaw) << "deg";
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::WaistDropDown_Expanding(
	winrt::Microsoft::UI::Xaml::Controls::Expander const& sender,
	winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& args)
{
	if (!settings_localInitFinished)return; // Don't even try if we're not set up yet
	trackersConfig_UpdateIsEnabled();

	// Close all others if valid
	if (k2app::K2Settings.isJointPairEnabled[0])
	{
		k2app::shared::settings::feetDropDown.get()->IsExpanded(false);
		k2app::shared::settings::elbowsDropDown.get()->IsExpanded(false);
		k2app::shared::settings::kneesDropDown.get()->IsExpanded(false);
	}
}


void winrt::KinectToVR::implementation::SettingsPage::WaistTrackerEnabledToggle_Toggled(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointPairEnabled[0] = k2app::shared::settings::waistTrackerEnabledToggle.get()->IsOn();

	// Check if we've disabled any joints from spawning and disable their mods
	k2app::interfacing::devices_check_disabled_joints();
	trackersConfigChanged();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::WaistPositionFilterOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	switch (const uint32_t index = k2app::shared::settings::waistPositionFilterOptionBox.get()->SelectedIndex(); index)
	{
	// LERP
	case 0:
		k2app::K2Settings.positionTrackingFilterOptions[0] = k2app::k2_PositionTrackingFilter_LERP;
		break;
	// Lowpass
	case 1:
		k2app::K2Settings.positionTrackingFilterOptions[0] = k2app::k2_PositionTrackingFilter_Lowpass;
		break;
	// Kalman
	case 2:
		k2app::K2Settings.positionTrackingFilterOptions[0] = k2app::k2_PositionTrackingFilter_Kalman;
		break;
	// Disable
	case 3:
		k2app::K2Settings.positionTrackingFilterOptions[0] = k2app::k2_NoPositionTrackingFilter;
		break;
	}

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::WaistRotationFilterOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	switch (const uint32_t index = k2app::shared::settings::waistRotationFilterOptionBox.get()->SelectedIndex(); index)
	{
	// Device
	case 0:
		k2app::K2Settings.jointRotationTrackingOption[0] = k2app::k2_DeviceInferredRotation;
		break;
	// Headset
	case 1:
		k2app::K2Settings.jointRotationTrackingOption[0] = k2app::k2_FollowHMDRotation;
		break;
	// Disable
	case 2:
		k2app::K2Settings.jointRotationTrackingOption[0] = k2app::k2_DisableJointRotation;
		break;
	}

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::FeetDropDown_Expanding(
	winrt::Microsoft::UI::Xaml::Controls::Expander const& sender,
	winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& args)
{
	if (!settings_localInitFinished)return; // Don't even try if we're not set up yet
	trackersConfig_UpdateIsEnabled();

	// Close all others if valid
	if (k2app::K2Settings.isJointPairEnabled[1])
	{
		k2app::shared::settings::waistDropDown.get()->IsExpanded(false);
		k2app::shared::settings::elbowsDropDown.get()->IsExpanded(false);
		k2app::shared::settings::kneesDropDown.get()->IsExpanded(false);
	}
}


void winrt::KinectToVR::implementation::SettingsPage::FeetTrackersEnabledToggle_Toggled(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointPairEnabled[1] = k2app::shared::settings::feetTrackersEnabledToggle.get()->IsOn();

	// Check if we've disabled any joints from spawning and disable their mods
	k2app::interfacing::devices_check_disabled_joints();
	trackersConfigChanged();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::FeetPositionFilterOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	switch (const uint32_t index = k2app::shared::settings::feetPositionFilterOptionBox.get()->SelectedIndex(); index)
	{
	// LERP
	case 0:
		k2app::K2Settings.positionTrackingFilterOptions[1] = k2app::k2_PositionTrackingFilter_LERP;
		k2app::K2Settings.positionTrackingFilterOptions[2] = k2app::k2_PositionTrackingFilter_LERP;
		break;
	// Lowpass
	case 1:
		k2app::K2Settings.positionTrackingFilterOptions[1] = k2app::k2_PositionTrackingFilter_Lowpass;
		k2app::K2Settings.positionTrackingFilterOptions[2] = k2app::k2_PositionTrackingFilter_Lowpass;
		break;
	// Kalman
	case 2:
		k2app::K2Settings.positionTrackingFilterOptions[1] = k2app::k2_PositionTrackingFilter_Kalman;
		k2app::K2Settings.positionTrackingFilterOptions[2] = k2app::k2_PositionTrackingFilter_Kalman;
		break;
	// Disable
	case 3:
		k2app::K2Settings.positionTrackingFilterOptions[1] = k2app::k2_NoPositionTrackingFilter;
		k2app::K2Settings.positionTrackingFilterOptions[2] = k2app::k2_NoPositionTrackingFilter;
		break;
	}

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::FeetRotationFilterOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	switch (const uint32_t index = k2app::shared::settings::feetRotationFilterOptionBox.get()->SelectedIndex(); index)
	{
	// Device
	case 0:
		k2app::K2Settings.jointRotationTrackingOption[1] = k2app::k2_DeviceInferredRotation;
		k2app::K2Settings.jointRotationTrackingOption[2] = k2app::k2_DeviceInferredRotation;
		break;
	// Software
	case 1:
		k2app::K2Settings.jointRotationTrackingOption[1] = k2app::k2_SoftwareCalculatedRotation;
		k2app::K2Settings.jointRotationTrackingOption[2] = k2app::k2_SoftwareCalculatedRotation;
		break;
	// Headset
	case 2:
		k2app::K2Settings.jointRotationTrackingOption[1] = k2app::k2_FollowHMDRotation;
		k2app::K2Settings.jointRotationTrackingOption[2] = k2app::k2_FollowHMDRotation;
		break;
	// Disable
	case 3:
		k2app::K2Settings.jointRotationTrackingOption[1] = k2app::k2_DisableJointRotation;
		k2app::K2Settings.jointRotationTrackingOption[2] = k2app::k2_DisableJointRotation;
		break;
	}

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::ElbowsDropDown_Expanding(
	winrt::Microsoft::UI::Xaml::Controls::Expander const& sender,
	winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& args)
{
	if (!settings_localInitFinished)return; // Don't even try if we're not set up yet
	trackersConfig_UpdateIsEnabled();

	// Close all others if valid
	if (k2app::K2Settings.isJointPairEnabled[2])
	{
		k2app::shared::settings::feetDropDown.get()->IsExpanded(false);
		k2app::shared::settings::waistDropDown.get()->IsExpanded(false);
		k2app::shared::settings::kneesDropDown.get()->IsExpanded(false);
	}
}


void winrt::KinectToVR::implementation::SettingsPage::ElbowTrackersEnabledToggle_Toggled(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointPairEnabled[2] = k2app::shared::settings::elbowTrackersEnabledToggle.get()->IsOn();

	// Check if we've disabled any joints from spawning and disable their mods
	k2app::interfacing::devices_check_disabled_joints();
	trackersConfigChanged();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::ElbowsPositionFilterOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	switch (const uint32_t index = k2app::shared::settings::elbowsPositionFilterOptionBox.get()->SelectedIndex(); index)
	{
	// LERP
	case 0:
		k2app::K2Settings.positionTrackingFilterOptions[3] = k2app::k2_PositionTrackingFilter_LERP;
		k2app::K2Settings.positionTrackingFilterOptions[4] = k2app::k2_PositionTrackingFilter_LERP;
		break;
	// Lowpass
	case 1:
		k2app::K2Settings.positionTrackingFilterOptions[3] = k2app::k2_PositionTrackingFilter_Lowpass;
		k2app::K2Settings.positionTrackingFilterOptions[4] = k2app::k2_PositionTrackingFilter_Lowpass;
		break;
	// Kalman
	case 2:
		k2app::K2Settings.positionTrackingFilterOptions[3] = k2app::k2_PositionTrackingFilter_Kalman;
		k2app::K2Settings.positionTrackingFilterOptions[4] = k2app::k2_PositionTrackingFilter_Kalman;
		break;
	// Disable
	case 3:
		k2app::K2Settings.positionTrackingFilterOptions[3] = k2app::k2_NoPositionTrackingFilter;
		k2app::K2Settings.positionTrackingFilterOptions[4] = k2app::k2_NoPositionTrackingFilter;
		break;
	}

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::ElbowsRotationFilterOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	switch (const uint32_t index = k2app::shared::settings::elbowsRotationFilterOptionBox.get()->SelectedIndex(); index)
	{
	// Device
	case 0:
		k2app::K2Settings.jointRotationTrackingOption[3] = k2app::k2_DeviceInferredRotation;
		k2app::K2Settings.jointRotationTrackingOption[4] = k2app::k2_DeviceInferredRotation;
		break;
	// Headset
	case 1:
		k2app::K2Settings.jointRotationTrackingOption[3] = k2app::k2_FollowHMDRotation;
		k2app::K2Settings.jointRotationTrackingOption[4] = k2app::k2_FollowHMDRotation;
		break;
	// Disable
	case 2:
		k2app::K2Settings.jointRotationTrackingOption[3] = k2app::k2_DisableJointRotation;
		k2app::K2Settings.jointRotationTrackingOption[4] = k2app::k2_DisableJointRotation;
		break;
	}

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::KneesDropDown_Expanding(
	winrt::Microsoft::UI::Xaml::Controls::Expander const& sender,
	winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& args)
{
	if (!settings_localInitFinished)return; // Don't even try if we're not set up yet
	trackersConfig_UpdateIsEnabled();

	// Close all others if valid
	if (k2app::K2Settings.isJointPairEnabled[3])
	{
		k2app::shared::settings::feetDropDown.get()->IsExpanded(false);
		k2app::shared::settings::elbowsDropDown.get()->IsExpanded(false);
		k2app::shared::settings::waistDropDown.get()->IsExpanded(false);
	}
}


void winrt::KinectToVR::implementation::SettingsPage::KneeTrackersEnabledToggle_Toggled(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointPairEnabled[3] = k2app::shared::settings::kneeTrackersEnabledToggle.get()->IsOn();

	// Check if we've disabled any joints from spawning and disable their mods
	k2app::interfacing::devices_check_disabled_joints();
	trackersConfigChanged();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::KneePositionFilterOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	switch (const uint32_t index = k2app::shared::settings::kneePositionFilterOptionBox.get()->SelectedIndex(); index)
	{
	// LERP
	case 0:
		k2app::K2Settings.positionTrackingFilterOptions[5] = k2app::k2_PositionTrackingFilter_LERP;
		k2app::K2Settings.positionTrackingFilterOptions[6] = k2app::k2_PositionTrackingFilter_LERP;
		break;
	// Lowpass
	case 1:
		k2app::K2Settings.positionTrackingFilterOptions[5] = k2app::k2_PositionTrackingFilter_Lowpass;
		k2app::K2Settings.positionTrackingFilterOptions[6] = k2app::k2_PositionTrackingFilter_Lowpass;
		break;
	// Kalman
	case 2:
		k2app::K2Settings.positionTrackingFilterOptions[5] = k2app::k2_PositionTrackingFilter_Kalman;
		k2app::K2Settings.positionTrackingFilterOptions[6] = k2app::k2_PositionTrackingFilter_Kalman;
		break;
	// Disable
	case 3:
		k2app::K2Settings.positionTrackingFilterOptions[5] = k2app::k2_NoPositionTrackingFilter;
		k2app::K2Settings.positionTrackingFilterOptions[6] = k2app::k2_NoPositionTrackingFilter;
		break;
	}

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::KneeRotationFilterOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	switch (const uint32_t index = k2app::shared::settings::kneeRotationFilterOptionBox.get()->SelectedIndex(); index)
	{
	// Device
	case 0:
		k2app::K2Settings.jointRotationTrackingOption[5] = k2app::k2_DeviceInferredRotation;
		k2app::K2Settings.jointRotationTrackingOption[6] = k2app::k2_DeviceInferredRotation;
		break;
	// Headset
	case 1:
		k2app::K2Settings.jointRotationTrackingOption[5] = k2app::k2_FollowHMDRotation;
		k2app::K2Settings.jointRotationTrackingOption[6] = k2app::k2_FollowHMDRotation;
		break;
	// Disable
	case 2:
		k2app::K2Settings.jointRotationTrackingOption[5] = k2app::k2_DisableJointRotation;
		k2app::K2Settings.jointRotationTrackingOption[6] = k2app::k2_DisableJointRotation;
		break;
	}

	// Save settings
	k2app::K2Settings.saveSettings();
}
