#pragma once
#include "TrackingDevicesView.g.h"

namespace winrt::KinectToVR::implementation
{
	struct TrackingDevicesView : TrackingDevicesViewT<TrackingDevicesView>
	{
		TrackingDevicesView() = delete;
		TrackingDevicesView(hstring const& DeviceName,
		                    hstring const& ConnectedStatus,
		                    hstring const& OnlineStatus);

		int32_t DeviceID();
		hstring DeviceName(),
		        ConnectedStatus(),
		        OnlineStatus();
		bool Current();

		void DeviceID(int32_t const& value);
		void DeviceName(hstring const& value);
		void ConnectedStatus(hstring const& value);
		void OnlineStatus(hstring const& value);
		void Current(bool const& value);

		event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value);
		void PropertyChanged(event_token const& token);

	private:
		int32_t m_DeviceID;
		hstring m_DeviceName,
		        m_ConnectedStatus,
		        m_OnlineStatus;
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
