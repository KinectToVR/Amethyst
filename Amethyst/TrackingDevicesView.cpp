#include "pch.h"
#include "TrackingDevicesView.h"
#if __has_include("TrackingDevicesView.g.cpp")
#include "TrackingDevicesView.g.cpp"
#endif

namespace winrt::KinectToVR::implementation
{
	TrackingDevicesView::TrackingDevicesView(
		const hstring& DeviceName) :
		m_DeviceName{DeviceName}
	{
		m_DeviceID = -1;
		m_Current = false;
	}

	TrackingDevicesView::TrackingDevicesView(
		const int32_t& DeviceID,
		const hstring& DeviceName,
		const bool& Current) :
		m_DeviceID{DeviceID},
		m_DeviceName{DeviceName},
		m_Current{Current}
	{
	}

	int32_t TrackingDevicesView::DeviceID()
	{
		return m_DeviceID;
	}

	hstring TrackingDevicesView::DeviceName()
	{
		return m_DeviceName;
	}

	bool TrackingDevicesView::Current()
	{
		return m_Current;
	}

	void TrackingDevicesView::DeviceID(const int32_t& value)
	{
		if (m_DeviceID != value)
		{
			m_DeviceID = value;
			m_propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{L"DeviceID"});
		}
	}

	void TrackingDevicesView::DeviceName(const hstring& value)
	{
		if (m_DeviceName != value)
		{
			m_DeviceName = value;
			m_propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{L"DeviceName"});
		}
	}

	void TrackingDevicesView::Current(const bool& value)
	{
		if (m_Current != value)
		{
			m_Current = value;
			m_propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{L"Current"});
		}
	}

	event_token TrackingDevicesView::PropertyChanged(
		const Windows::UI::Xaml::Data::PropertyChangedEventHandler& handler)
	{
		return m_propertyChanged.add(handler);
	}

	void TrackingDevicesView::PropertyChanged(const event_token& token)
	{
		m_propertyChanged.remove(token);
	}
}
