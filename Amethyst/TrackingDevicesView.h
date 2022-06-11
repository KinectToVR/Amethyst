#pragma once
#include "TrackingDevicesView.g.h"

namespace winrt::Amethyst::implementation
{
	struct TrackingDevicesView : TrackingDevicesViewT<TrackingDevicesView>
	{
		TrackingDevicesView() = delete;
		TrackingDevicesView(const hstring& DeviceName);
		TrackingDevicesView(const int32_t& DeviceID, const hstring& DeviceName, const bool& Current);

		int32_t DeviceID();
		hstring DeviceName();
		bool Current();

		void DeviceID(const int32_t& value);
		void DeviceName(const hstring& value);
		void Current(const bool& value);

		event_token PropertyChanged(const Windows::UI::Xaml::Data::PropertyChangedEventHandler& value);
		void PropertyChanged(const event_token& token);

	private:
		int32_t m_DeviceID;
		hstring m_DeviceName;
		bool m_Current;

		event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
	};
}


namespace winrt::Amethyst::factory_implementation
{
	struct TrackingDevicesView : TrackingDevicesViewT<TrackingDevicesView, implementation::TrackingDevicesView>
	{
	};
}
