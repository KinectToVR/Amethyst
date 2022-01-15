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
				overridesLabel;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Grid>
				deviceErrorGrid,
				trackingDeviceChangePanel,
				overridesControls;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ListView> devicesListView;
			
			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Button>
				setAsOverrideButton,
				setAsBaseButton;

			inline std::binary_semaphore smphSignalCurrentUpdate{0};
			inline uint32_t selectedTrackingDeviceID = 0;
		}
	}

	namespace interfacing
	{
		// Current tracking device: 0 is the default
		inline uint32_t trackingDeviceID = 0;
		inline int32_t overrideDeviceID = -1;
	}
}
