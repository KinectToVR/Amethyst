#pragma once
#include "CalibrationDeviceEntryView.g.h"

namespace winrt::Amethyst::implementation
{
	struct CalibrationDeviceEntryView : CalibrationDeviceEntryViewT<CalibrationDeviceEntryView>
	{
		CalibrationDeviceEntryView() = delete;
		CalibrationDeviceEntryView(
			const hstring& DeviceGUID,
			const hstring& DisplayName,
			const bool& IsBase,
			const bool& IsOverride,
			const bool& StatusError);

		[[nodiscard]] hstring DeviceGUID();
		[[nodiscard]] hstring DisplayName();
		[[nodiscard]] bool IsBase() const;
		[[nodiscard]] bool IsOverride() const;
		[[nodiscard]] bool StatusError() const;

		Windows::Foundation::IAsyncAction DeviceGUID(const hstring& value);
		Windows::Foundation::IAsyncAction DisplayName(const hstring& value);
		Windows::Foundation::IAsyncAction IsBase(const bool& value);
		Windows::Foundation::IAsyncAction IsOverride(const bool& value);
		Windows::Foundation::IAsyncAction StatusError(const bool& value);

		double BoolToOpacity(const bool& value);
		double ErrorToOpacity(const bool& value);

		[[nodiscard]] event_token PropertyChanged(
			const Microsoft::UI::Xaml::Data::PropertyChangedEventHandler& value);

		void PropertyChanged(const event_token& token);

	private:
		hstring _deviceGUID;
		hstring _displayName;
		bool _isBase = false;
		bool _isOverride = false;
		bool _statusError = false;

		event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;
	};
}

namespace winrt::Amethyst::factory_implementation
{
	struct CalibrationDeviceEntryView : CalibrationDeviceEntryViewT<CalibrationDeviceEntryView, implementation::CalibrationDeviceEntryView>
	{
	};
}

namespace k2app::shared::CalibrationDeviceEntryView
{
	inline winrt::apartment_context* thisEntryContext;
}