#pragma once
#include "pch.h"
#include "K2Settings.h"

inline std::wstring wstring_cast(std::string const& s)
{
	return std::wstring(s.begin(), s.end());
}

inline std::string string_cast(std::wstring const& s)
{
	return std::string(s.begin(), s.end());
}

inline std::array<std::string, 3> split_status(std::string const& s)
{
	// If there are 3 strings separated by \n
	return std::array<std::string, 3>{
		s.substr(0, s.find("\n")),
		s.substr(s.find("\n") + 1, s.rfind("\n") - (s.find("\n") + 1)),
		s.substr(s.rfind("\n") + 1)
	};
}

namespace k2app
{
	namespace shared
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
				jointBasisControls_1;

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
		}

		namespace settings
		{
			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Button> restartButton;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ComboBox>
				waistRotationOptionBox,
				feetRotationOptionBox,
				elbowRotationOptionBox,
				kneeRotationOptionBox,
				positionFilterOptionBox,
				positionFilterOptionBox_1;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem> softwareRotationItem;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::CheckBox>
				flipCheckBox,
				externalFlipCheckBox,
				waistOnCheckbox,
				leftFootOnCheckbox,
				rightFootOnCheckbox,
				leftElbowOnCheckBox,
				rightElbowOnCheckBox,
				leftKneeOnCheckBox,
				rightKneeOnCheckBox,
				autoSpawnCheckbox,
				enableSoundsCheckbox;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TextBlock>
				flipCheckBoxLabel,
				externalFlipCheckBoxLabel;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ToggleSwitch>
				waistEnabledToggle,
				leftFootEnabledToggle,
				rightFootEnabledToggle,
				leftElbowEnabledToggle,
				rightElbowEnabledToggle,
				leftKneeEnabledToggle,
				rightKneeEnabledToggle;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Slider> soundsVolumeSlider;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Expander>
				rotationDropDown,
				expRotationDropDown,
				trackersDropDown,
				expTrackersDropDown;
		}

		namespace other
		{
			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Primitives::ToggleButton> toggleFreezeButton;
		}
	}
}
