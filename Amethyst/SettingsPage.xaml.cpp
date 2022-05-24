#include "pch.h"
#include "SettingsPage.xaml.h"

#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::KinectToVR::implementation
{
	SettingsPage::SettingsPage()
	{
		InitializeComponent();

		// Cache needed UI elements
		using namespace k2app::shared::settings;

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

		externalFlipCheckBox = std::make_shared<Controls::CheckBox>(ExternalFlipCheckBox());
		autoSpawnCheckbox = std::make_shared<Controls::CheckBox>(AutoSpawnCheckBox());
		enableSoundsCheckbox = std::make_shared<Controls::CheckBox>(SoundsEnabledCheckBox());
		autoStartCheckBox = std::make_shared<Controls::CheckBox>(AutoStartCheckBox());

		flipDropDownGrid = std::make_shared<Controls::Grid>(FlipDropDownGrid());
		flipToggle = std::make_shared<Controls::ToggleSwitch>(FlipToggle());

		externalFlipCheckBoxLabel = std::make_shared<Controls::TextBlock>(ExternalFlipCheckBoxLabel());
		setErrorFlyoutText = std::make_shared<Controls::TextBlock>(SetErrorFlyoutText());

		trackerPairEnabledToggles.at(0) = std::make_shared<Controls::ToggleSwitch>(WaistTrackerEnabledToggle());
		trackerPairEnabledToggles.at(1) = std::make_shared<Controls::ToggleSwitch>(FeetTrackersEnabledToggle());
		trackerPairEnabledToggles.at(2) = std::make_shared<Controls::ToggleSwitch>(ElbowTrackersEnabledToggle());
		trackerPairEnabledToggles.at(3) = std::make_shared<Controls::ToggleSwitch>(KneeTrackersEnabledToggle());

		waistDropDown = std::make_shared<Controls::Expander>(WaistDropDown());
		feetDropDown = std::make_shared<Controls::Expander>(FeetDropDown());
		kneesDropDown = std::make_shared<Controls::Expander>(KneesDropDown());
		elbowsDropDown = std::make_shared<Controls::Expander>(ElbowsDropDown());
		flipDropDown = std::make_shared<Controls::Expander>(FlipDropDown());

		soundsVolumeSlider = std::make_shared<Controls::Slider>(SoundsVolumeSlider());

		externalFlipStackPanel = std::make_shared<Controls::StackPanel>(ExternalFlipStackPanel());
	}
}


void KinectToVR::implementation::SettingsPage::ExternalFlipCheckBox_Checked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	// Cache flip to settings and save
	k2app::K2Settings.isExternalFlipEnabled = true; // Checked
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::SettingsPage::ExternalFlipCheckBox_Unchecked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	// Cache flip to settings and save
	k2app::K2Settings.isExternalFlipEnabled = false; // Unchecked
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::SettingsPage::RestartButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	ktvr::request_vr_restart<false>("SteamVR needs to be restarted to enable/disable trackers properly.");
}


void KinectToVR::implementation::SettingsPage::ResetButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Mark trackers as inactive
	k2app::interfacing::K2AppTrackersInitialized = false;
	if (k2app::shared::general::toggleTrackersButton.get() != nullptr)
		k2app::shared::general::toggleTrackersButton->IsChecked(false);

	// Read settings after reset
	k2app::K2Settings = k2app::K2AppSettings(); // Reset settings
	k2app::K2Settings.saveSettings(); // Save empty settings

	/* Restart app */

	// Literals
	using namespace std::string_literals;

	// Get current caller path
	const auto fileName = new CHAR[MAX_PATH + 1];
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
			k2app::interfacing::ShowToast(
				k2app::interfacing::LocalizedResourceWString(L"SharedStrings", L"Toasts/RestartFailed/Title"),
				k2app::interfacing::LocalizedResourceWString(L"SharedStrings", L"Toasts/RestartFailed/Content"));
			return;
		}

		// Mark exiting as true
		k2app::interfacing::isExitingNow = true;

		// Exit the app
		LOG(INFO) << "Configuration has been reset, exiting...";
		Application::Current().Exit();
	}
	else
	{
		LOG(ERROR) << "App will not be restarted due to caller process identification error.";
		k2app::interfacing::ShowToast(
			k2app::interfacing::LocalizedResourceWString(L"SharedStrings", L"Toasts/RestartFailed/Title"),
			k2app::interfacing::LocalizedResourceWString(L"SharedStrings", L"Toasts/RestartFailed/Content"));
	}
}


void KinectToVR::implementation::SettingsPage::SettingsPage_Loaded(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	using namespace k2app::shared::settings;

	// Select saved flip, position and rotation options
	flipToggle.get()->IsOn(k2app::K2Settings.isFlipEnabled);
	externalFlipCheckBox.get()->IsChecked(k2app::K2Settings.isExternalFlipEnabled);

	// Waist (pos)
	waistPositionFilterOptionBox.get()->SelectedIndex(
		k2app::K2Settings.positionTrackingFilterOptions[0]);

	// Feet (pos)
	feetPositionFilterOptionBox.get()->SelectedIndex(
		k2app::K2Settings.positionTrackingFilterOptions[1]);

	// Elbows (pos)
	elbowsPositionFilterOptionBox.get()->SelectedIndex(
		k2app::K2Settings.positionTrackingFilterOptions[3]);

	// Knees (pos)
	kneePositionFilterOptionBox.get()->SelectedIndex(
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

	if (const auto& trackingDevice = TrackingDevices::getCurrentDevice(); trackingDevice.index() == 0)
	{
		// Kinect Basis
		softwareRotationItem.get()->IsEnabled(
			std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isAppOrientationSupported());
		flipToggle.get()->IsEnabled(
			std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported());
		flipDropDown.get()->IsEnabled(
			std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported());
		flipDropDownGrid.get()->Opacity(flipToggle.get()->IsEnabled() ? 1 : 0.5);
		TrackingDevices::settings_set_external_flip_is_enabled();
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		softwareRotationItem.get()->IsEnabled(false);
		flipToggle.get()->IsEnabled(false);
		flipDropDown.get()->IsEnabled(false);
		flipDropDownGrid.get()->Opacity(0.5);
		TrackingDevices::settings_set_external_flip_is_enabled(false);
	}

	// Load the tracker configuration
	trackerPairEnabledToggles.at(0).get()->IsOn(k2app::K2Settings.isJointPairEnabled[0]);
	trackerPairEnabledToggles.at(1).get()->IsOn(k2app::K2Settings.isJointPairEnabled[1]);
	trackerPairEnabledToggles.at(2).get()->IsOn(k2app::K2Settings.isJointPairEnabled[2]);
	trackerPairEnabledToggles.at(3).get()->IsOn(k2app::K2Settings.isJointPairEnabled[3]);

	// Load auto-spawn and sounds config
	autoSpawnCheckbox->IsChecked(k2app::K2Settings.autoSpawnEnabledJoints);
	enableSoundsCheckbox->IsChecked(k2app::K2Settings.enableAppSounds);
	soundsVolumeSlider.get()->Value(k2app::K2Settings.appSoundsVolume);

	// Load tracker settings/enabled
	TrackingDevices::settings_trackersConfig_UpdateIsEnabled();

	// Notify of the setup end
	k2app::shared::settings::settings_localInitFinished = true;
	K2InsightsCLR::LogPageView("Settings");
}

void KinectToVR::implementation::SettingsPage::AutoSpawn_Checked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	k2app::K2Settings.autoSpawnEnabledJoints = true;
	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::SettingsPage::AutoSpawn_Unchecked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	k2app::K2Settings.autoSpawnEnabledJoints = false;
	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::SettingsPage::EnableSounds_Checked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	// Turn sounds on
	k2app::K2Settings.enableAppSounds = true;
	ElementSoundPlayer::State(ElementSoundPlayerState::On);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::SettingsPage::EnableSounds_Unchecked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	// Turn sounds on
	k2app::K2Settings.enableAppSounds = false;
	ElementSoundPlayer::State(ElementSoundPlayerState::Off);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::SettingsPage::SoundsVolumeSlider_ValueChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::Primitives::RangeBaseValueChangedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	// Change sounds level
	k2app::K2Settings.appSoundsVolume = k2app::shared::settings::soundsVolumeSlider.get()->Value();
	ElementSoundPlayer::Volume(std::clamp(
		static_cast<double>(k2app::K2Settings.appSoundsVolume) / 100.0, 0.0, 100.0));

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::SettingsPage::CalibrateExternalFlipMenuFlyoutItem_Click(
	const Windows::Foundation::IInspectable& sender,
	const RoutedEventArgs& e)
{
	k2app::K2Settings.externalFlipCalibrationYaw =
		EigenUtils::QuatToEulers(
			k2app::interfacing::K2TrackersVector.at(0).pose.orientation).y();

	LOG(INFO) << "Captured yaw for external flip: " <<
		radiansToDegrees(k2app::K2Settings.externalFlipCalibrationYaw) << "deg";
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::SettingsPage::WaistDropDown_Expanding(
	const Controls::Expander& sender,
	const Controls::ExpanderExpandingEventArgs& args)
{
	if (!k2app::shared::settings::settings_localInitFinished)return; // Don't even try if we're not set up yet
	TrackingDevices::settings_trackersConfig_UpdateIsEnabled();

	// Close all others if valid
	if (k2app::K2Settings.isJointPairEnabled[0])
	{
		k2app::shared::settings::feetDropDown.get()->IsExpanded(false);
		k2app::shared::settings::elbowsDropDown.get()->IsExpanded(false);
		k2app::shared::settings::kneesDropDown.get()->IsExpanded(false);
	}
}


Windows::Foundation::IAsyncAction KinectToVR::implementation::SettingsPage::WaistTrackerEnabledToggle_Toggled(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)co_return;

	// Mark trackers as inactive, back up the current one
	const bool _trackersInitialized =
		k2app::interfacing::K2AppTrackersInitialized;
	k2app::interfacing::K2AppTrackersInitialized = false;
	{
		// Sleep on UI
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(20);
		co_await ui_thread;
	}

	// Make actual changes
	k2app::K2Settings.isJointPairEnabled[0] = 
		k2app::shared::settings::trackerPairEnabledToggles.at(0).get()->IsOn();

	// Check if we've disabled any joints from spawning and disable their mods
	k2app::interfacing::devices_check_disabled_joints();
	TrackingDevices::settings_trackersConfigChanged();

	// Mark trackers as active (or backup)
	{
		// Sleep on UI
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(20);
		co_await ui_thread;
	}
	k2app::interfacing::K2AppTrackersInitialized = _trackersInitialized;

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::SettingsPage::WaistPositionFilterOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

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


void KinectToVR::implementation::SettingsPage::WaistRotationFilterOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

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


void KinectToVR::implementation::SettingsPage::FeetDropDown_Expanding(
	const Controls::Expander& sender,
	const Controls::ExpanderExpandingEventArgs& args)
{
	if (!k2app::shared::settings::settings_localInitFinished)return; // Don't even try if we're not set up yet
	TrackingDevices::settings_trackersConfig_UpdateIsEnabled();

	// Close all others if valid
	if (k2app::K2Settings.isJointPairEnabled[1])
	{
		k2app::shared::settings::waistDropDown.get()->IsExpanded(false);
		k2app::shared::settings::elbowsDropDown.get()->IsExpanded(false);
		k2app::shared::settings::kneesDropDown.get()->IsExpanded(false);
	}
}


Windows::Foundation::IAsyncAction KinectToVR::implementation::SettingsPage::FeetTrackersEnabledToggle_Toggled(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)co_return;

	// Mark trackers as inactive, back up the current one
	const bool _trackersInitialized =
		k2app::interfacing::K2AppTrackersInitialized;
	k2app::interfacing::K2AppTrackersInitialized = false;
	{
		// Sleep on UI
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(20);
		co_await ui_thread;
	}

	// Make actual changes
	k2app::K2Settings.isJointPairEnabled[1] = 
		k2app::shared::settings::trackerPairEnabledToggles.at(1).get()->IsOn();

	// Check if we've disabled any joints from spawning and disable their mods
	k2app::interfacing::devices_check_disabled_joints();
	TrackingDevices::settings_trackersConfigChanged();

	// Mark trackers as active (or backup)
	{
		// Sleep on UI
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(20);
		co_await ui_thread;
	}
	k2app::interfacing::K2AppTrackersInitialized = _trackersInitialized;

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::SettingsPage::FeetPositionFilterOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

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


void KinectToVR::implementation::SettingsPage::FeetRotationFilterOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

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


void KinectToVR::implementation::SettingsPage::ElbowsDropDown_Expanding(
	const Controls::Expander& sender,
	const Controls::ExpanderExpandingEventArgs& args)
{
	if (!k2app::shared::settings::settings_localInitFinished)return; // Don't even try if we're not set up yet
	TrackingDevices::settings_trackersConfig_UpdateIsEnabled();

	// Close all others if valid
	if (k2app::K2Settings.isJointPairEnabled[2])
	{
		k2app::shared::settings::feetDropDown.get()->IsExpanded(false);
		k2app::shared::settings::waistDropDown.get()->IsExpanded(false);
		k2app::shared::settings::kneesDropDown.get()->IsExpanded(false);
	}
}


Windows::Foundation::IAsyncAction KinectToVR::implementation::SettingsPage::ElbowTrackersEnabledToggle_Toggled(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)co_return;

	// Mark trackers as inactive, back up the current one
	const bool _trackersInitialized =
		k2app::interfacing::K2AppTrackersInitialized;
	k2app::interfacing::K2AppTrackersInitialized = false;
	{
		// Sleep on UI
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(20);
		co_await ui_thread;
	}

	// Make actual changes
	k2app::K2Settings.isJointPairEnabled[2] = 
		k2app::shared::settings::trackerPairEnabledToggles.at(2).get()->IsOn();

	// Check if we've disabled any joints from spawning and disable their mods
	k2app::interfacing::devices_check_disabled_joints();
	TrackingDevices::settings_trackersConfigChanged();

	// Mark trackers as active (or backup)
	{
		// Sleep on UI
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(20);
		co_await ui_thread;
	}
	k2app::interfacing::K2AppTrackersInitialized = _trackersInitialized;

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::SettingsPage::ElbowsPositionFilterOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

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


void KinectToVR::implementation::SettingsPage::ElbowsRotationFilterOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

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


void KinectToVR::implementation::SettingsPage::KneesDropDown_Expanding(
	const Controls::Expander& sender,
	const Controls::ExpanderExpandingEventArgs& args)
{
	if (!k2app::shared::settings::settings_localInitFinished)return; // Don't even try if we're not set up yet
	TrackingDevices::settings_trackersConfig_UpdateIsEnabled();

	// Close all others if valid
	if (k2app::K2Settings.isJointPairEnabled[3])
	{
		k2app::shared::settings::feetDropDown.get()->IsExpanded(false);
		k2app::shared::settings::elbowsDropDown.get()->IsExpanded(false);
		k2app::shared::settings::waistDropDown.get()->IsExpanded(false);
	}
}


Windows::Foundation::IAsyncAction KinectToVR::implementation::SettingsPage::KneeTrackersEnabledToggle_Toggled(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)co_return;

	// Mark trackers as inactive, back up the current one
	const bool _trackersInitialized =
		k2app::interfacing::K2AppTrackersInitialized;
	k2app::interfacing::K2AppTrackersInitialized = false;
	{
		// Sleep on UI
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(20);
		co_await ui_thread;
	}

	// Make actual changes
	k2app::K2Settings.isJointPairEnabled[3] = 
		k2app::shared::settings::trackerPairEnabledToggles.at(3).get()->IsOn();

	// Check if we've disabled any joints from spawning and disable their mods
	k2app::interfacing::devices_check_disabled_joints();
	TrackingDevices::settings_trackersConfigChanged();

	// Mark trackers as active (or backup)
	{
		// Sleep on UI
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(20);
		co_await ui_thread;
	}
	k2app::interfacing::K2AppTrackersInitialized = _trackersInitialized;

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::SettingsPage::KneePositionFilterOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

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


void KinectToVR::implementation::SettingsPage::KneeRotationFilterOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

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


void KinectToVR::implementation::SettingsPage::FlipDropDown_Expanding(
	const winrt::Microsoft::UI::Xaml::Controls::Expander& sender,
	const winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs& args)
{
	if (!k2app::shared::settings::settings_localInitFinished)return; // Don't even try if we're not set up yet

	// Enable/Disable ExtFlip
	TrackingDevices::settings_set_external_flip_is_enabled();
	TrackingDevices::settings_trackersConfig_UpdateIsEnabled();
}


void KinectToVR::implementation::SettingsPage::FlipToggle_Toggled(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	// Don't react to pre-init signals
	if (!k2app::shared::settings::settings_localInitFinished)return;

	// Cache flip to settings and save
	k2app::K2Settings.isFlipEnabled =
		k2app::shared::settings::flipToggle->IsOn(); // Checked?

	TrackingDevices::settings_set_external_flip_is_enabled();
	TrackingDevices::settings_trackersConfig_UpdateIsEnabled();

	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::SettingsPage::AutoStartFlyout_Opening(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
	k2app::shared::settings::autoStartCheckBox->IsChecked(
		vr::VRApplications()->GetApplicationAutoLaunch("KinectToVR.Amethyst"));
}


void KinectToVR::implementation::SettingsPage::AutoStartCheckBox_Checked(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	K2InsightsCLR::LogEvent("Amethyst auto-start enabled");
	k2app::interfacing::installApplicationManifest(); // Just in case

	const auto app_error = vr::VRApplications()->
		SetApplicationAutoLaunch("KinectToVR.Amethyst", true);

	if (app_error != vr::VRApplicationError_None)
		LOG(WARNING) << "Amethyst manifest not installed! Error:  " <<
			vr::VRApplications()->GetApplicationsErrorNameFromEnum(app_error);
}


void KinectToVR::implementation::SettingsPage::AutoStartCheckBox_Unchecked(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	K2InsightsCLR::LogEvent("Amethyst auto-start disabled");
	k2app::interfacing::installApplicationManifest(); // Just in case

	const auto app_error = vr::VRApplications()->
		SetApplicationAutoLaunch("KinectToVR.Amethyst", false);

	if (app_error != vr::VRApplicationError_None)
		LOG(WARNING) << "Amethyst manifest not installed! Error:  " <<
			vr::VRApplications()->GetApplicationsErrorNameFromEnum(app_error);
}


void KinectToVR::implementation::SettingsPage::ReManifestButton_Click(
	const winrt::Microsoft::UI::Xaml::Controls::SplitButton& sender,
	const winrt::Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs& args)
{
	K2InsightsCLR::LogEvent("Amethyst re-manifest");

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
}


void KinectToVR::implementation::SettingsPage::ReRegisterButton_Click(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	K2InsightsCLR::LogEvent("Amethyst Driver re-register");

	if (exists(boost::dll::program_location().parent_path() / "K2CrashHandler" / "K2CrashHandler.exe"))
	{
		std::thread([]
		{
			ShellExecuteA(nullptr, "open",
			              (boost::dll::program_location().parent_path() / "K2CrashHandler" / "K2CrashHandler.exe ")
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
}


void KinectToVR::implementation::SettingsPage::DismissSetErrorButton_Click(
	const Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	SetErrorFlyout().Hide();
}


void winrt::KinectToVR::implementation::SettingsPage::LearnAboutFiltersButton_Click(
	const winrt::Windows::Foundation::IInspectable& sender, const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e)
{
	Controls::Primitives::FlyoutShowOptions options;
	options.Placement(Controls::Primitives::FlyoutPlacementMode::Full);
	options.ShowMode(Controls::Primitives::FlyoutShowMode::Transient);

	LearnAboutFiltersFlyout().ShowAt(LearnAboutFiltersButton(), options);
	DimGrid().Visibility(Visibility::Visible);
}


void winrt::KinectToVR::implementation::SettingsPage::LearnAboutFiltersFlyout_Closed(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
	DimGrid().Visibility(Visibility::Collapsed);
}
