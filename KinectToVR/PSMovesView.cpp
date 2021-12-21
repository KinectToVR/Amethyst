#include "pch.h"
#include "PSMovesView.h"
#if __has_include("PSMovesView.g.cpp")
#include "PSMovesView.g.cpp"
#endif

namespace winrt::KinectToVR::implementation
{
	PSMovesView::PSMovesView(
		int32_t const& DeviceID) :
		m_DeviceID{ DeviceID }
	{
	}

	int32_t PSMovesView::DeviceID()
	{
		return m_DeviceID;
	}

	void PSMovesView::DeviceID(int32_t const& value)
	{
		if (m_DeviceID != value)
		{
			m_DeviceID = value;
			m_propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"DeviceID" });
		}
	}

	event_token PSMovesView::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
	{
		return m_propertyChanged.add(handler);
	}
	void PSMovesView::PropertyChanged(event_token const& token)
	{
		m_propertyChanged.remove(token);
	}
}
