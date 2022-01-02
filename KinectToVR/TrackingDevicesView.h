#pragma once
#include "TrackingDevicesView.g.h"

namespace winrt::KinectToVR::implementation
{
	struct TrackingDevicesView : TrackingDevicesViewT<TrackingDevicesView>
	{
		TrackingDevicesView() = delete;
		TrackingDevicesView(hstring const& DeviceName);
		TrackingDevicesView(int32_t const& DeviceID, hstring const& DeviceName, bool const& Current);

		int32_t DeviceID();
		hstring DeviceName();
		bool Current();

		void DeviceID(int32_t const& value);
		void DeviceName(hstring const& value);
		void Current(bool const& value);

		event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value);
		void PropertyChanged(event_token const& token);

	private:
		int32_t m_DeviceID;
		hstring m_DeviceName;
		bool m_Current;

		winrt::event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
	};
}


namespace winrt::KinectToVR::factory_implementation
{
	struct TrackingDevicesView : TrackingDevicesViewT<TrackingDevicesView, implementation::TrackingDevicesView>
	{
	};
}
