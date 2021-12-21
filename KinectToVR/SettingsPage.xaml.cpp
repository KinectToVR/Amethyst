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
		restartButton = std::make_shared<Controls::Button>(RestartButton());

		// Notify of the setup end
		settings_localInitFinished = true;
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
}


void winrt::KinectToVR::implementation::SettingsPage::FlipCheckBox_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
}


void winrt::KinectToVR::implementation::SettingsPage::WaistRotationOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
}


void winrt::KinectToVR::implementation::SettingsPage::FeetRotationOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
}


void winrt::KinectToVR::implementation::SettingsPage::PositionFilterOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
}


void winrt::KinectToVR::implementation::SettingsPage::WaistEnabledToggle_Toggled(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
}


void winrt::KinectToVR::implementation::SettingsPage::LeftFootEnabledToggle_Toggled(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
}


void winrt::KinectToVR::implementation::SettingsPage::RightFootEnabledToggle_Toggled(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
}


void winrt::KinectToVR::implementation::SettingsPage::RestartButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
}
