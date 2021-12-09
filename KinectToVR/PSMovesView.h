#pragma once
#include "PSMovesView.g.h"
#include "PSMovesView.h"

namespace winrt::KinectToVR::implementation
{
	struct PSMovesView : PSMovesViewT<PSMovesView>
	{
		PSMovesView() = delete;
		PSMovesView(int32_t const& DeviceID);

		int32_t DeviceID();
		void DeviceID(int32_t const& value);

		event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& value);
		void PropertyChanged(event_token const& token);

	private:
		int32_t m_DeviceID;

		winrt::event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
	};
}


namespace winrt::KinectToVR::factory_implementation
{
	struct PSMovesView : PSMovesViewT<PSMovesView, implementation::PSMovesView>
	{
	};
}
