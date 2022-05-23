#pragma once
#include "pch.h"
#include "K2Settings.h"

inline std::array<std::wstring, 3> split_status(const std::wstring& s)
{
	// If there are 3 strings separated by \n
	return std::array<std::wstring, 3>{
		s.substr(0, s.find(L"\n")),
		s.substr(s.find(L"\n") + 1, s.rfind(L"\n") - (s.find(L"\n") + 1)),
		s.substr(s.rfind(L"\n") + 1)
	};
}

namespace k2app::shared
{
	namespace main
	{
		// Main Window
		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::NavigationViewItem>
			generalItem,
			settingsItem,
			devicesItem,
			infoItem,
			consoleItem;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Window> thisAppWindow;

		inline std::shared_ptr<winrt::Microsoft::UI::Dispatching::DispatcherQueue> thisDispatcherQueue;

		inline std::shared_ptr<winrt::Microsoft::Windows::AppNotifications::AppNotificationManager>
		thisNotificationManager;

		inline std::shared_ptr<winrt::Microsoft::Windows::ApplicationModel::Resources::ResourceManager>
		thisResourceManager;

		inline std::shared_ptr<winrt::Microsoft::Windows::ApplicationModel::Resources::ResourceContext>
		thisResourceContext;
	}

	namespace general
	{
		// General Page
		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Primitives::ToggleButton>
		toggleTrackersButton;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ToggleSplitButton>
		skeletonToggleButton;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::CheckBox>
		forceRenderCheckBox;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Button>
			calibrationButton,
			offsetsButton;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TextBlock>
			versionLabel,
			deviceNameLabel,
			deviceStatusLabel,
			errorWhatText,
			trackingDeviceErrorLabel,
			overrideDeviceNameLabel,
			overrideDeviceStatusLabel,
			overrideErrorWhatText,
			overrideDeviceErrorLabel,
			serverStatusLabel,
			serverErrorLabel,
			serverErrorWhatText,
			forceRenderText;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Grid>
			errorButtonsGrid,
			errorWhatGrid,
			overrideErrorButtonsGrid,
			overrideErrorWhatGrid,
			serverErrorWhatGrid,
			serverErrorButtonsGrid;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::NumberBox>
			waistRollNumberBox,
			waistYawNumberBox,
			waistPitchNumberBox,
			waistXNumberBox,
			waistYNumberBox,
			waistZNumberBox,

			leftFootRollNumberBox,
			leftFootYawNumberBox,
			leftFootPitchNumberBox,
			leftFootXNumberBox,
			leftFootYNumberBox,
			leftFootZNumberBox,

			rightFootRollNumberBox,
			rightFootYawNumberBox,
			rightFootPitchNumberBox,
			rightFootXNumberBox,
			rightFootYNumberBox,
			rightFootZNumberBox,

			leftElbowRollNumberBox,
			leftElbowYawNumberBox,
			leftElbowPitchNumberBox,
			leftElbowXNumberBox,
			leftElbowYNumberBox,
			leftElbowZNumberBox,

			rightElbowRollNumberBox,
			rightElbowYawNumberBox,
			rightElbowPitchNumberBox,
			rightElbowXNumberBox,
			rightElbowYNumberBox,
			rightElbowZNumberBox,
			leftKneeRollNumberBox,

			leftKneeYawNumberBox,
			leftKneePitchNumberBox,
			leftKneeXNumberBox,
			leftKneeYNumberBox,
			leftKneeZNumberBox,

			rightKneeRollNumberBox,
			rightKneeYawNumberBox,
			rightKneePitchNumberBox,
			rightKneeXNumberBox,
			rightKneeYNumberBox,
			rightKneeZNumberBox;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ToggleSplitButton>
		toggleFreezeButton;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::CheckBox>
		freezeOnlyLowerCheckBox;
	}

	namespace devices
	{
		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TextBlock>
			deviceNameLabel,
			deviceStatusLabel,
			errorWhatText,
			baseDeviceName,
			overrideDeviceName,
			trackingDeviceErrorLabel,
			overridesLabel,
			jointBasisLabel;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Grid>
			deviceErrorGrid,
			trackingDeviceChangePanel,
			overridesControls,
			overridesControls_1,
			jointBasisControls,
			jointBasisControls_1,
			devicesMainContentGridOuter,
			devicesMainContentGridInner;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Expander>
			jointBasisDropDown,
			jointBasisDropDown_1,
			overridesDropDown,
			overridesDropDown_1;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ListView> devicesListView;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Button>
			setAsOverrideButton,
			setAsBaseButton,
			deselectDeviceButton; // This one's override-only

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ComboBox>
			waistJointOptionBox,
			leftFootJointOptionBox,
			rightFootJointOptionBox,
			leftElbowJointOptionBox,
			rightElbowJointOptionBox,
			leftKneeJointOptionBox,
			rightKneeJointOptionBox,
			rightFootPositionOverrideOptionBox,
			rightFootRotationOverrideOptionBox,
			leftFootRotationOverrideOptionBox,
			leftFootPositionOverrideOptionBox,
			waistRotationOverrideOptionBox,
			waistPositionOverrideOptionBox,
			leftElbowPositionOverrideOptionBox,
			leftElbowRotationOverrideOptionBox,
			rightElbowPositionOverrideOptionBox,
			rightElbowRotationOverrideOptionBox,
			leftKneePositionOverrideOptionBox,
			leftKneeRotationOverrideOptionBox,
			rightKneePositionOverrideOptionBox,
			rightKneeRotationOverrideOptionBox;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ToggleMenuFlyoutItem>
			overrideWaistPosition,
			overrideWaistRotation,
			overrideLeftFootPosition,
			overrideLeftFootRotation,
			overrideRightFootPosition,
			overrideRightFootRotation,
			overrideLeftElbowPosition,
			overrideLeftElbowRotation,
			overrideRightElbowPosition,
			overrideRightElbowRotation,
			overrideLeftKneePosition,
			overrideLeftKneeRotation,
			overrideRightKneePosition,
			overrideRightKneeRotation;

		inline std::binary_semaphore smphSignalCurrentUpdate{0},
		                             smphSignalStartMain{0};
		inline uint32_t selectedTrackingDeviceID = 0;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ScrollViewer> devicesMainContentScrollViewer;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::StackPanel>
			devicesOverridesSelectorStackPanelOuter,
			devicesOverridesSelectorStackPanelInner,
			devicesJointsBasisSelectorStackPanelOuter,
			devicesJointsBasisSelectorStackPanelInner,
			selectedDeviceSettingsRootLayoutPanel;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::AppBarButton> selectedDeviceSettingsButton;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Flyout> selectedDeviceSettingsFlyout;
	}

	namespace settings
	{
		inline bool settings_localInitFinished = false;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Button> restartButton;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ComboBox>
			waistPositionFilterOptionBox,
			waistRotationFilterOptionBox,
			feetPositionFilterOptionBox,
			feetRotationFilterOptionBox,
			kneePositionFilterOptionBox,
			kneeRotationFilterOptionBox,
			elbowsPositionFilterOptionBox,
			elbowsRotationFilterOptionBox;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem> softwareRotationItem;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::CheckBox>
			externalFlipCheckBox,
			autoSpawnCheckbox,
			enableSoundsCheckbox,
			autoStartCheckBox;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TextBlock>
			externalFlipCheckBoxLabel,
			setErrorFlyoutText;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Grid>
		flipDropDownGrid;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ToggleSwitch>
		flipToggle;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ToggleSwitch>
			waistTrackerEnabledToggle,
			feetTrackersEnabledToggle,
			kneeTrackersEnabledToggle,
			elbowTrackersEnabledToggle;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Slider> soundsVolumeSlider;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Expander>
			waistDropDown,
			feetDropDown,
			kneesDropDown,
			elbowsDropDown,
			flipDropDown;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::StackPanel>
		externalFlipStackPanel;
	}
}
