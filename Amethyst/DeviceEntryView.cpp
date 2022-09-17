#include "pch.h"
#include "DeviceEntryView.h"
#if __has_include("DeviceEntryView.g.cpp")
#include "DeviceEntryView.g.cpp"
#endif

namespace winrt::Amethyst::implementation
{
	DeviceEntryView::DeviceEntryView(
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

	hstring DeviceEntryView::DeviceGUID()
	{
		return _deviceGUID;
	}

	hstring DeviceEntryView::DisplayName()
	{
		return _displayName;
	}

	bool DeviceEntryView::IsBase() const
	{
		return _isBase;
	}

	bool DeviceEntryView::IsOverride() const
	{
		return _isOverride;
	}

	bool DeviceEntryView::StatusError() const
	{
		return _statusError;
	}

	Windows::Foundation::IAsyncAction DeviceEntryView::DeviceGUID(const hstring& value)
	{
		if (_deviceGUID != value)
		{
			_deviceGUID = value;

			LOG(INFO) << "Changing the apartment thread context to the constructor thread...";
			co_await *k2app::shared::DeviceEntryView::thisEntryContext;

			LOG(INFO) << "Invoking PropertyChanged for DeviceGUID...";
			_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"DeviceGUID"));
		}
	}

	Windows::Foundation::IAsyncAction DeviceEntryView::DisplayName(const hstring& value)
	{
		if (_displayName != value)
		{
			_displayName = value;

			LOG(INFO) << "Changing the apartment thread context to the constructor thread...";
			co_await *k2app::shared::DeviceEntryView::thisEntryContext;

			LOG(INFO) << "Invoking PropertyChanged for DisplayName...";
			_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"DisplayName"));
		}
	}

	Windows::Foundation::IAsyncAction DeviceEntryView::IsBase(const bool& value)
	{
		if (_isBase != value)
		{
			_isBase = value;

			LOG(INFO) << "Changing the apartment thread context to the constructor thread...";
			co_await *k2app::shared::DeviceEntryView::thisEntryContext;

			LOG(INFO) << "Invoking PropertyChanged for IsBase...";
			_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"IsBase"));
		}
	}

	Windows::Foundation::IAsyncAction DeviceEntryView::IsOverride(const bool& value)
	{
		if (_isOverride != value)
		{
			_isOverride = value;

			LOG(INFO) << "Changing the apartment thread context to the constructor thread...";
			co_await *k2app::shared::DeviceEntryView::thisEntryContext;

			LOG(INFO) << "Invoking PropertyChanged for IsOverride...";
			_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"IsOverride"));
		}
	}

	Windows::Foundation::IAsyncAction DeviceEntryView::StatusError(const bool& value)
	{
		if (_statusError != value)
		{
			_statusError = value;

			LOG(INFO) << "Changing the apartment thread context to the constructor thread...";
			co_await *k2app::shared::DeviceEntryView::thisEntryContext;
			
			LOG(INFO) << "Invoking PropertyChanged for StatusError...";
			_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"StatusError"));
		}
	}

	event_token DeviceEntryView::PropertyChanged(
		const Microsoft::UI::Xaml::Data::PropertyChangedEventHandler& handler)
	{
		return _propertyChanged.add(handler);
	}

	void DeviceEntryView::PropertyChanged(const event_token& token)
	{
		_propertyChanged.remove(token);
	}
}
