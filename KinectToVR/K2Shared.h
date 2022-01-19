#pragma once
#include "pch.h"

namespace k2app
{
	namespace shared
	{
		namespace general
		{
			// General Page
			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TextBlock>
				deviceNameLabel,
				deviceStatusLabel,
				errorWhatText,
				trackingDeviceErrorLabel;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Grid>
				errorButtonsGrid,
				errorWhatGrid;
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
	}

	namespace interfacing
	{
		// Current tracking device: 0 is the default
		inline uint32_t trackingDeviceID = 0;
		inline int32_t overrideDeviceID = -1;

		// Joint tracking device selected joints: 0s are the defaults
		// On the first time refresh the joints are assigned like W0 L1 R2
		inline std::array<int32_t, 3> // W,L,R
			selectedTrackedJointID = { 0, 0, 0 };

		// Current override joints: W,L,R and 0 is the default for waist
		// Override joints may overlap, -1 is for combo disabled and 'None' displayed
		inline std::array<int32_t, 3>
			positionOverrideJointID = {0, -1, -1},
			rotationOverrideJointID = {0, -1, -1};

		// Current override joints: W,L,R and true is the default for waist
		inline std::array<bool, 3>
			isPositionOverriddenJoint = { true, false, false },
			isRotationOverriddenJoint = { true, false, false };

	}
}
