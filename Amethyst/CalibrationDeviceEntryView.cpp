#include "pch.h"
#include "CalibrationDeviceEntryView.h"
#if __has_include("CalibrationDeviceEntryView.g.cpp")
#include "CalibrationDeviceEntryView.g.cpp"
#endif

namespace winrt::Amethyst::implementation
{
	CalibrationDeviceEntryView::CalibrationDeviceEntryView(
		const hstring& DeviceGUID,
		const hstring& DisplayName,
		const bool& IsBase,
		const bool& IsOverride,
		const bool& StatusError) :

		_deviceGUID(DeviceGUID),
		_displayName(DisplayName),
		_isBase(IsBase),
		_isOverride(IsOverride),
		_statusError(StatusError)
	{
	}

	hstring CalibrationDeviceEntryView::DeviceGUID()
	{
		return _deviceGUID;
	}

	hstring CalibrationDeviceEntryView::DisplayName()
	{
		return _displayName;
	}

	bool CalibrationDeviceEntryView::IsBase() const
	{
		return _isBase;
	}

	bool CalibrationDeviceEntryView::IsOverride() const
	{
		return _isOverride;
	}

	bool CalibrationDeviceEntryView::StatusError() const
	{
		return _statusError;
	}

	Windows::Foundation::IAsyncAction CalibrationDeviceEntryView::DeviceGUID(const hstring& value)
	{
		if (_deviceGUID != value)
		{
			_deviceGUID = value;

			LOG(INFO) << "Changing the apartment thread context to the constructor thread...";
			co_await *k2app::shared::CalibrationDeviceEntryView::thisEntryContext;

			LOG(INFO) << "Invoking PropertyChanged for DeviceGUID...";
			_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"DeviceGUID"));
		}
	}

	Windows::Foundation::IAsyncAction CalibrationDeviceEntryView::DisplayName(const hstring& value)
	{
		if (_displayName != value)
		{
			_displayName = value;

			LOG(INFO) << "Changing the apartment thread context to the constructor thread...";
			co_await *k2app::shared::CalibrationDeviceEntryView::thisEntryContext;

			LOG(INFO) << "Invoking PropertyChanged for DisplayName...";
			_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"DisplayName"));
		}
	}

	Windows::Foundation::IAsyncAction CalibrationDeviceEntryView::IsBase(const bool& value)
	{
		if (_isBase != value)
		{
			_isBase = value;

			LOG(INFO) << "Changing the apartment thread context to the constructor thread...";
			co_await *k2app::shared::CalibrationDeviceEntryView::thisEntryContext;

			LOG(INFO) << "Invoking PropertyChanged for IsBase...";
			_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"IsBase"));
		}
	}

	Windows::Foundation::IAsyncAction CalibrationDeviceEntryView::IsOverride(const bool& value)
	{
		if (_isOverride != value)
		{
			_isOverride = value;

			LOG(INFO) << "Changing the apartment thread context to the constructor thread...";
			co_await *k2app::shared::CalibrationDeviceEntryView::thisEntryContext;

			LOG(INFO) << "Invoking PropertyChanged for IsOverride...";
			_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"IsOverride"));
		}
	}

	Windows::Foundation::IAsyncAction CalibrationDeviceEntryView::StatusError(const bool& value)
	{
		if (_statusError != value)
		{
			_statusError = value;

			LOG(INFO) << "Changing the apartment thread context to the constructor thread...";
			co_await *k2app::shared::CalibrationDeviceEntryView::thisEntryContext;
			
			LOG(INFO) << "Invoking PropertyChanged for StatusError...";
			_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"StatusError"));
		}
	}

	double CalibrationDeviceEntryView::BoolToOpacity(const bool& value)
	{
		return value;
	}

	event_token CalibrationDeviceEntryView::PropertyChanged(
		const Microsoft::UI::Xaml::Data::PropertyChangedEventHandler& handler)
	{
		return _propertyChanged.add(handler);
	}

	void CalibrationDeviceEntryView::PropertyChanged(const event_token& token)
	{
		_propertyChanged.remove(token);
	}
}
