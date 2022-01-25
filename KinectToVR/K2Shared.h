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

namespace k2app
{
	namespace shared
	{
		namespace general
		{
			// General Page
			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Primitives::ToggleButton>
				toggleTrackersButton;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TextBlock>
				deviceNameLabel,
				deviceStatusLabel,
				errorWhatText,
				trackingDeviceErrorLabel;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Grid>
				errorButtonsGrid,
				errorWhatGrid;

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
				rightFootZNumberBox;
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
				jointBasisControls;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ListView> devicesListView;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Button>
				setAsOverrideButton,
				setAsBaseButton;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ComboBox>
				waistJointOptionBox,
				leftFootJointOptionBox,
				rightFootJointOptionBox,
				rightFootPositionOverrideOptionBox,
				rightFootRotationOverrideOptionBox,
				leftFootRotationOverrideOptionBox,
				leftFootPositionOverrideOptionBox,
				waistRotationOverrideOptionBox,
				waistPositionOverrideOptionBox;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ToggleMenuFlyoutItem>
				overrideWaistPosition,
				overrideWaistRotation,
				overrideLeftFootPosition,
				overrideLeftFootRotation,
				overrideRightFootPosition,
				overrideRightFootRotation;

			inline std::binary_semaphore smphSignalCurrentUpdate{0};
			inline uint32_t selectedTrackingDeviceID = 0;
		}

		namespace settings
		{
			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Button> restartButton;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ComboBox>
				waistRotationOptionBox,
				feetRotationOptionBox,
				positionFilterOptionBox;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem> softwareRotationItem;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::CheckBox>
				flipCheckBox,
				waistOnCheckbox,
				leftFootOnCheckbox,
				rightFootOnCheckbox,
				autoSpawnCheckbox,
				enableSoundsCheckbox;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TextBlock> flipCheckBoxLabel;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ToggleSwitch>
				waistEnabledToggle,
				leftFootEnabledToggle,
				rightFootEnabledToggle;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Slider> soundsVolumeSlider;
		}
	}
}
