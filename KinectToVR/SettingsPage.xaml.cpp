#include "pch.h"
#include "SettingsPage.xaml.h"

#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

// LMAO Eat Dirt Micro&Soft
// Imma just cache object before the fancy UWP delegation reownership
std::shared_ptr<Controls::Button> restartButton;

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
		positionFilterOptionBox =
			std::make_shared<Controls::ComboBox>(PositionFilterOptionBox());

		softwareRotationItem =
			std::make_shared<Controls::ComboBoxItem>(SoftwareRotationItem());

		flipCheckBox = std::make_shared<Controls::CheckBox>(FlipCheckBox());

		flipCheckBoxLabel = std::make_shared<Controls::TextBlock>(FlipCheckBoxLabel());
	}
}

void trackersConfigChanged()
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	// Compare with saved settings and unlock the restart
	restartButton.get()->IsEnabled(true);
}

void winrt::KinectToVR::implementation::SettingsPage::WaistOnToggle_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::WaistOnToggle_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::LeftFootOnToggle_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::LeftFootOnToggle_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::RightFootOnToggle_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::RightFootOnToggle_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	trackersConfigChanged();
}


void winrt::KinectToVR::implementation::SettingsPage::FlipCheckBox_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	// Cache flip to settings and save
	k2app::K2Settings.isFlipEnabled = true; // Checked
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::FlipCheckBox_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't react to pre-init signals
	if (!settings_localInitFinished)return;

	// Cache flip to settings and save
	k2app::K2Settings.isFlipEnabled = false; // Unchecked
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
		k2app::K2Settings.positionFilterOption = k2app::k2_PositionTrackingFilter_LERP;
		break;
	// Lowpass
	case 1:
		k2app::K2Settings.positionFilterOption = k2app::k2_PositionTrackingFilter_Lowpass;
		break;
	// Kalman
	case 2:
		k2app::K2Settings.positionFilterOption = k2app::k2_PositionTrackingFilter_Kalman;
		break;
	// Disable
	case 3:
		k2app::K2Settings.positionFilterOption = k2app::k2_NoPositionTrackingFilter;
		break;
	}

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::WaistEnabledToggle_Toggled(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::LeftFootEnabledToggle_Toggled(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::RightFootEnabledToggle_Toggled(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::SettingsPage::RestartButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
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
		if (system(_cmd.c_str()) < 0)
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
		exit(0);
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

	feetRotationOptionBox.get()->SelectedIndex(
		k2app::K2Settings.jointRotationTrackingOption[1]); // Feet
	positionFilterOptionBox.get()->SelectedIndex(
		k2app::K2Settings.positionFilterOption); // Position

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

	if (auto const& trackingDevice = TrackingDevices::getCurrentDevice(); trackingDevice.index() == 0)
	{
		// Kinect Basis
		softwareRotationItem.get()->IsEnabled(
			std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isAppOrientationSupported());
		flipCheckBox.get()->IsEnabled(
			std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->isFlipSupported());
		flipCheckBoxLabel.get()->Opacity(flipCheckBox.get()->IsEnabled() ? 1 : 0.5);
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		softwareRotationItem.get()->IsEnabled(false);
		flipCheckBox.get()->IsEnabled(false);
		flipCheckBoxLabel.get()->Opacity(0.5);
	}

	// Notify of the setup end
	settings_localInitFinished = true;
}
