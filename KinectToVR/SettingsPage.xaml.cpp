#include "pch.h"
#include "SettingsPage.xaml.h"

#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

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

		waistRotationOptionBox =
			std::make_shared<Controls::ComboBox>(WaistRotationOptionBox());
		feetRotationOptionBox =
			std::make_shared<Controls::ComboBox>(FeetRotationOptionBox());
		elbowRotationOptionBox =
			std::make_shared<Controls::ComboBox>(ElbowRotationOptionBox());
		kneeRotationOptionBox =
			std::make_shared<Controls::ComboBox>(KneeRotationOptionBox());
		positionFilterOptionBox =
			std::make_shared<Controls::ComboBox>(PositionFilterOptionBox());
		positionFilterOptionBox_1 =
			std::make_shared<Controls::ComboBox>(PositionFilterOptionBox_1());

		softwareRotationItem =
			std::make_shared<Controls::ComboBoxItem>(SoftwareRotationItem());

		flipCheckBox = std::make_shared<Controls::CheckBox>(FlipCheckBox());
		externalFlipCheckBox = std::make_shared<Controls::CheckBox>(ExternalFlipCheckBox());
		waistOnCheckbox = std::make_shared<Controls::CheckBox>(WaistOnToggle());
		leftFootOnCheckbox = std::make_shared<Controls::CheckBox>(LeftFootOnToggle());
		rightFootOnCheckbox = std::make_shared<Controls::CheckBox>(RightFootOnToggle());
		leftElbowOnCheckBox = std::make_shared<Controls::CheckBox>(LeftElbowOnToggle());
		rightElbowOnCheckBox = std::make_shared<Controls::CheckBox>(RightElbowOnToggle());
		leftKneeOnCheckBox = std::make_shared<Controls::CheckBox>(LeftKneeOnToggle());
		rightKneeOnCheckBox = std::make_shared<Controls::CheckBox>(RightKneeOnToggle());
		autoSpawnCheckbox = std::make_shared<Controls::CheckBox>(AutoSpawnCheckBox());
		enableSoundsCheckbox = std::make_shared<Controls::CheckBox>(SoundsEnabledCheckBox());

		flipCheckBoxLabel = std::make_shared<Controls::TextBlock>(FlipCheckBoxLabel());
		externalFlipCheckBoxLabel = std::make_shared<Controls::TextBlock>(ExternalFlipCheckBoxLabel());

		waistEnabledToggle = std::make_shared<Controls::ToggleSwitch>(WaistEnabledToggle());
		leftFootEnabledToggle = std::make_shared<Controls::ToggleSwitch>(LeftFootEnabledToggle());
		rightFootEnabledToggle = std::make_shared<Controls::ToggleSwitch>(RightFootEnabledToggle());
		leftElbowEnabledToggle = std::make_shared<Controls::ToggleSwitch>(LeftElbowEnabledToggle());
		rightElbowEnabledToggle = std::make_shared<Controls::ToggleSwitch>(RightElbowEnabledToggle());
		leftKneeEnabledToggle = std::make_shared<Controls::ToggleSwitch>(LeftKneeEnabledToggle());
		rightKneeEnabledToggle = std::make_shared<Controls::ToggleSwitch>(RightKneeEnabledToggle());

		rotationDropDown = std::make_shared<Controls::Expander>(RotationDropDown());
		expRotationDropDown = std::make_shared<Controls::Expander>(ExpRotationDropDown());
		trackersDropDown = std::make_shared<Controls::Expander>(TrackersDropDown());
		expTrackersDropDown = std::make_shared<Controls::Expander>(ExpTrackersDropDown());

		soundsVolumeSlider = std::make_shared<Controls::Slider>(SoundsVolumeSlider());

		externalFlipStackPanel = std::make_shared<Controls::StackPanel>(ExternalFlipStackPanel());
	}
}

void trackersConfig_UpdateIsEnabled()
{
	// Also turn off basic trackers pose filter if they are off
	k2app::shared::settings::
		positionFilterOptionBox.get()->IsEnabled(
			k2app::K2Settings.isJointEnabled[0] ||
			k2app::K2Settings.isJointEnabled[1] ||
			k2app::K2Settings.isJointEnabled[2]);

	// Also turn off extra trackers pose filter if they are off
	k2app::shared::settings::
		positionFilterOptionBox_1.get()->IsEnabled(
			k2app::K2Settings.isJointEnabled[3] ||
			k2app::K2Settings.isJointEnabled[4] ||
			k2app::K2Settings.isJointEnabled[5] ||
			k2app::K2Settings.isJointEnabled[6]);

	// Also turn off waist rot combo if waist is turned off
	k2app::shared::settings::
		waistRotationOptionBox.get()->IsEnabled(
			k2app::K2Settings.isJointEnabled[0]);

	// Also turn off feet rot combo if both feet are turned off
	k2app::shared::settings::
		feetRotationOptionBox.get()->IsEnabled(
			k2app::K2Settings.isJointEnabled[1] ||
			k2app::K2Settings.isJointEnabled[2]);

	// Also turn off elbow rot combo if both elbows are turned off
	k2app::shared::settings::
		elbowRotationOptionBox.get()->IsEnabled(
			k2app::K2Settings.isJointEnabled[3] ||
			k2app::K2Settings.isJointEnabled[4]);

	// Also turn off knee rot combo if both knees are turned off
	k2app::shared::settings::
		kneeRotationOptionBox.get()->IsEnabled(
			k2app::K2Settings.isJointEnabled[5] ||
			k2app::K2Settings.isJointEnabled[6]);

	// Turn off the base expander if base joints are off
	k2app::shared::settings::
		rotationDropDown.get()->IsEnabled(
			k2app::K2Settings.isJointEnabled[0] ||
			k2app::K2Settings.isJointEnabled[1] ||
			k2app::K2Settings.isJointEnabled[2]);

	// Also collapse it if so
	if(!k2app::shared::settings::
		rotationDropDown.get()->IsEnabled())
		k2app::shared::settings::
		rotationDropDown.get()->IsExpanded(false);

	// Turn off the extra expander if extra joints are off
	k2app::shared::settings::
		expRotationDropDown.get()->IsEnabled(
			k2app::K2Settings.isJointEnabled[3] ||
			k2app::K2Settings.isJointEnabled[4] ||
			k2app::K2Settings.isJointEnabled[5] ||
			k2app::K2Settings.isJointEnabled[6]);

	// Also collapse it if so
	if (!k2app::shared::settings::
		expRotationDropDown.get()->IsEnabled())
		k2app::shared::settings::
		expRotationDropDown.get()->IsExpanded(false);
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
	if (!k2app::K2Settings.isJointEnabled[0] &&
		!k2app::K2Settings.isJointEnabled[1] &&
		!k2app::K2Settings.isJointEnabled[2])
		k2app::interfacing::ShowToast("YOU'VE JUST DISABLED ALL TRACKERS",
		                              "WHAT SORT OF A TOTAL FUCKING LIFE FAILURE ARE YOU THAT YOU DID THIS YOU STUPID BITCH");

	// Compare with saved settings and unlock the restart
	k2app::shared::settings::restartButton.get()->IsEnabled(true);

	// Enable/Disable combos
	trackersConfig_UpdateIsEnabled();

	// Enable/Disable ExtFlip
	TrackingDevices::settings_set_external_flip_is_enabled();

	// Save settings
	k2app::K2Settings.saveSettings();
}

void winrt::KinectToVR::implementation::SettingsPage::WaistOnToggle_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointEnabled[0] = true;
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::WaistOnToggle_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointEnabled[0] = false;
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::LeftFootOnToggle_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointEnabled[1] = true;
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::LeftFootOnToggle_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointEnabled[1] = false;
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::RightFootOnToggle_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointEnabled[2] = true;
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::RightFootOnToggle_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointEnabled[2] = false;
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::LeftElbowOnToggle_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointEnabled[3] = true;
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::LeftElbowOnToggle_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointEnabled[3] = false;
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::RightElbowOnToggle_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointEnabled[4] = true;
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::RightElbowOnToggle_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointEnabled[4] = false;
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::LeftKneeOnToggle_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointEnabled[5] = true;
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::LeftKneeOnToggle_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointEnabled[5] = false;
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::RightKneeOnToggle_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointEnabled[6] = true;
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::RightKneeOnToggle_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointEnabled[6] = false;
	trackersConfigChanged();
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


void winrt::KinectToVR::implementation::SettingsPage::WaistRotationOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	switch (const uint32_t index = k2app::shared::settings::waistRotationOptionBox.get()->SelectedIndex(); index)
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


void winrt::KinectToVR::implementation::SettingsPage::FeetRotationOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	switch (const uint32_t index = k2app::shared::settings::feetRotationOptionBox.get()->SelectedIndex(); index)
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


void winrt::KinectToVR::implementation::SettingsPage::ElbowRotationOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	switch (const uint32_t index = k2app::shared::settings::elbowRotationOptionBox.get()->SelectedIndex(); index)
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


void winrt::KinectToVR::implementation::SettingsPage::KneeRotationOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	switch (const uint32_t index = k2app::shared::settings::kneeRotationOptionBox.get()->SelectedIndex(); index)
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


void winrt::KinectToVR::implementation::SettingsPage::PositionFilterOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	switch (const uint32_t index = k2app::shared::settings::positionFilterOptionBox.get()->SelectedIndex(); index)
	{
	// LERP
	case 0:
		k2app::K2Settings.positionFilterOption_basic = k2app::k2_PositionTrackingFilter_LERP;
		break;
	// Lowpass
	case 1:
		k2app::K2Settings.positionFilterOption_basic = k2app::k2_PositionTrackingFilter_Lowpass;
		break;
	// Kalman
	case 2:
		k2app::K2Settings.positionFilterOption_basic = k2app::k2_PositionTrackingFilter_Kalman;
		break;
	// Disable
	case 3:
		k2app::K2Settings.positionFilterOption_basic = k2app::k2_NoPositionTrackingFilter;
		break;
	}

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::PositionFilterOptionBox_1_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	switch (const uint32_t index = k2app::shared::settings::positionFilterOptionBox_1.get()->SelectedIndex(); index)
	{
	// LERP
	case 0:
		k2app::K2Settings.positionFilterOption_ext = k2app::k2_PositionTrackingFilter_LERP;
		break;
	// Lowpass
	case 1:
		k2app::K2Settings.positionFilterOption_ext = k2app::k2_PositionTrackingFilter_Lowpass;
		break;
	// Kalman
	case 2:
		k2app::K2Settings.positionFilterOption_ext = k2app::k2_PositionTrackingFilter_Kalman;
		break;
	// Disable
	case 3:
		k2app::K2Settings.positionFilterOption_ext = k2app::k2_NoPositionTrackingFilter;
		break;
	}

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::WaistEnabledToggle_Toggled(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointTurnedOn[0] = WaistEnabledToggle().IsOn();

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::LeftFootEnabledToggle_Toggled(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointTurnedOn[1] = LeftFootEnabledToggle().IsOn();

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::RightFootEnabledToggle_Toggled(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointTurnedOn[2] = RightFootEnabledToggle().IsOn();

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::LeftElbowEnabledToggle_Toggled(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointTurnedOn[3] = LeftElbowEnabledToggle().IsOn();

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::RightElbowEnabledToggle_Toggled(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointTurnedOn[4] = RightElbowEnabledToggle().IsOn();

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::LeftKneeEnabledToggle_Toggled(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointTurnedOn[5] = LeftKneeEnabledToggle().IsOn();

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::RightKneeEnabledToggle_Toggled(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	k2app::K2Settings.isJointTurnedOn[6] = RightKneeEnabledToggle().IsOn();

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
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

	feetRotationOptionBox.get()->SelectedIndex(
		k2app::K2Settings.jointRotationTrackingOption[1]); // Feet
	positionFilterOptionBox.get()->SelectedIndex(
		k2app::K2Settings.positionFilterOption_basic); // Position
	positionFilterOptionBox_1.get()->SelectedIndex(
		k2app::K2Settings.positionFilterOption_ext); // Position

	// Waist
	switch (k2app::K2Settings.jointRotationTrackingOption[0]) // Waist
	{
	default:
		waistRotationOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
	// Device
	case k2app::k2_DeviceInferredRotation:
		waistRotationOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
		break;
	// Software
	case k2app::k2_SoftwareCalculatedRotation: // If somehow...
		waistRotationOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
		break;
	// Headset
	case k2app::k2_FollowHMDRotation:
		waistRotationOptionBox.get()->SelectedIndex(k2app::k2_FollowHMDRotation - 1); // -1 to skip app-based rot
		break;
	// Disable
	case k2app::k2_DisableJointRotation:
		waistRotationOptionBox.get()->SelectedIndex(k2app::k2_DisableJointRotation - 1); // -1 to skip app-based rot
		break;
	}

	// Elbows
	switch (k2app::K2Settings.jointRotationTrackingOption[3]) // LElbow
	{
	default:
		elbowRotationOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
	// Device
	case k2app::k2_DeviceInferredRotation:
		elbowRotationOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
		break;
	// Software
	case k2app::k2_SoftwareCalculatedRotation: // If somehow...
		elbowRotationOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
		break;
	// Headset
	case k2app::k2_FollowHMDRotation:
		elbowRotationOptionBox.get()->SelectedIndex(k2app::k2_FollowHMDRotation - 1); // -1 to skip app-based rot
		break;
	// Disable
	case k2app::k2_DisableJointRotation:
		elbowRotationOptionBox.get()->SelectedIndex(k2app::k2_DisableJointRotation - 1); // -1 to skip app-based rot
		break;
	}

	// Knees
	switch (k2app::K2Settings.jointRotationTrackingOption[5]) // LKnee
	{
	default:
		kneeRotationOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
	// Device
	case k2app::k2_DeviceInferredRotation:
		kneeRotationOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
		break;
	// Software
	case k2app::k2_SoftwareCalculatedRotation: // If somehow...
		kneeRotationOptionBox.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
		break;
	// Headset
	case k2app::k2_FollowHMDRotation:
		kneeRotationOptionBox.get()->SelectedIndex(k2app::k2_FollowHMDRotation - 1); // -1 to skip app-based rot
		break;
	// Disable
	case k2app::k2_DisableJointRotation:
		kneeRotationOptionBox.get()->SelectedIndex(k2app::k2_DisableJointRotation - 1); // -1 to skip app-based rot
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
	waistEnabledToggle.get()->IsOn(k2app::K2Settings.isJointTurnedOn[0]);
	leftFootEnabledToggle.get()->IsOn(k2app::K2Settings.isJointTurnedOn[1]);
	rightFootEnabledToggle.get()->IsOn(k2app::K2Settings.isJointTurnedOn[2]);
	leftElbowEnabledToggle.get()->IsOn(k2app::K2Settings.isJointTurnedOn[3]);
	rightElbowEnabledToggle.get()->IsOn(k2app::K2Settings.isJointTurnedOn[4]);
	leftKneeEnabledToggle.get()->IsOn(k2app::K2Settings.isJointTurnedOn[5]);
	rightKneeEnabledToggle.get()->IsOn(k2app::K2Settings.isJointTurnedOn[6]);

	waistOnCheckbox.get()->IsChecked(k2app::K2Settings.isJointEnabled[0]);
	leftFootOnCheckbox.get()->IsChecked(k2app::K2Settings.isJointEnabled[1]);
	rightFootOnCheckbox.get()->IsChecked(k2app::K2Settings.isJointEnabled[2]);
	leftElbowOnCheckBox.get()->IsChecked(k2app::K2Settings.isJointEnabled[3]);
	rightElbowOnCheckBox.get()->IsChecked(k2app::K2Settings.isJointEnabled[4]);
	leftKneeOnCheckBox.get()->IsChecked(k2app::K2Settings.isJointEnabled[5]);
	rightKneeOnCheckBox.get()->IsChecked(k2app::K2Settings.isJointEnabled[6]);

	// Also turn off feet rot combo if both feet are turned off
	feetRotationOptionBox.get()->IsEnabled(
		k2app::K2Settings.isJointEnabled[1] ||
		k2app::K2Settings.isJointEnabled[2]);

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

void winrt::KinectToVR::implementation::SettingsPage::RotationDropDown_Expanding(
	winrt::Microsoft::UI::Xaml::Controls::Expander const& sender,
	winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& e)
{
	if (!settings_localInitFinished)return; // Don't even try if we're not set up yet
	k2app::shared::settings::expRotationDropDown.get()->IsExpanded(false);
}


void winrt::KinectToVR::implementation::SettingsPage::ExpRotationDropDown_Expanding(
	winrt::Microsoft::UI::Xaml::Controls::Expander const& sender,
	winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& e)
{
	if (!settings_localInitFinished)return; // Don't even try if we're not set up yet
	k2app::shared::settings::rotationDropDown.get()->IsExpanded(false);
}


void winrt::KinectToVR::implementation::SettingsPage::TrackersDropDown_Expanding(
	winrt::Microsoft::UI::Xaml::Controls::Expander const& sender,
	winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& e)
{
	if (!settings_localInitFinished)return; // Don't even try if we're not set up yet
	k2app::shared::settings::expTrackersDropDown.get()->IsExpanded(false);
}


void winrt::KinectToVR::implementation::SettingsPage::ExpTrackersDropDown_Expanding(
	winrt::Microsoft::UI::Xaml::Controls::Expander const& sender,
	winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& e)
{
	if (!settings_localInitFinished)return; // Don't even try if we're not set up yet
	k2app::shared::settings::trackersDropDown.get()->IsExpanded(false);
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
