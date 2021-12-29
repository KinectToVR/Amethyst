#include "pch.h"
#include "TrackingDevicesView.h"
#if __has_include("TrackingDevicesView.g.cpp")
#include "TrackingDevicesView.g.cpp"
#endif

namespace winrt::KinectToVR::implementation
{
	TrackingDevicesView::TrackingDevicesView(
		hstring const& DeviceName) :
		m_DeviceName{DeviceName}
	{
		m_DeviceID = -1;
		m_Current = false;
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

	void TrackingDevicesView::DeviceID(int32_t const& value)
	{
		if (m_DeviceID != value)
		{
			m_DeviceID = value;
			m_propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{L"DeviceID"});
		}
	}

	void TrackingDevicesView::DeviceName(hstring const& value)
	{
		if (m_DeviceName != value)
		{
			m_DeviceName = value;
			m_propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{L"DeviceName"});
		}
	}

	void TrackingDevicesView::Current(bool const& value)
	{
		if (m_Current != value)
		{
			m_Current = value;
			m_propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{L"Current"});
		}
	}
	
	event_token TrackingDevicesView::PropertyChanged(
		Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
	{
		return m_propertyChanged.add(handler);
	}

	void TrackingDevicesView::PropertyChanged(event_token const& token)
	{
		m_propertyChanged.remove(token);
	}
}
