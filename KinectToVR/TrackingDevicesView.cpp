#include "pch.h"
#include "TrackingDevicesView.h"
#if __has_include("TrackingDevicesView.g.cpp")
#include "TrackingDevicesView.g.cpp"
#endif

namespace winrt::KinectToVR::implementation
{
	TrackingDevicesView::TrackingDevicesView(
		hstring const& DeviceName,
		hstring const& ConnectedStatus,
		hstring const& OnlineStatus) :
		m_DeviceName{DeviceName},
		m_ConnectedStatus{ConnectedStatus},
		m_OnlineStatus{OnlineStatus}
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
	hstring TrackingDevicesView::ConnectedStatus()
	{
		return m_ConnectedStatus;
	}
	hstring TrackingDevicesView::OnlineStatus()
	{
		return m_OnlineStatus;
	}
	bool TrackingDevicesView::Current()
	{
		return m_Current;
	}
	
	void TrackingDevicesView::DeviceID(int32_t const& value)
	{
		if(m_DeviceID != value)
		{
			m_DeviceID = value;
			m_propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"DeviceID" });
		}
	}
	void TrackingDevicesView::DeviceName(hstring const& value)
	{
		if(m_DeviceName != value)
		{
			m_DeviceName = value;
			m_propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"DeviceName" });
		}
	}
	void TrackingDevicesView::ConnectedStatus(hstring const& value)
	{
		if(m_ConnectedStatus != value)
		{
			m_ConnectedStatus = value;
			m_propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"ConnectedStatus" });
		}
	}
	void TrackingDevicesView::OnlineStatus(hstring const& value)
	{
		if(m_OnlineStatus != value)
		{
			m_OnlineStatus = value;
			m_propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"OnlineStatus" });
		}
	}
	void TrackingDevicesView::Current(bool const& value)
	{
		if (m_Current != value)
		{
			m_Current = value;
			m_propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"Current" });
		}
	}

	event_token TrackingDevicesView::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
	{
		return m_propertyChanged.add(handler);
	}
	void TrackingDevicesView::PropertyChanged(event_token const& token)
	{
		m_propertyChanged.remove(token);
	}
}
