#pragma once
#include "PluginEntryView.g.h"

#include "K2Shared.h"
#include "K2Interfacing.h"

namespace winrt::Amethyst::implementation
{
	struct PluginEntryView : PluginEntryViewT<PluginEntryView>
	{
		PluginEntryView() = delete;
		PluginEntryView(
			const hstring& DisplayName,
			const hstring& DeviceGUID,
			const hstring& ErrorText,
			const hstring& Location,
			const bool& IsLoaded,
			const bool& LoadError);

		[[nodiscard]] hstring DisplayName() { return _displayName; }
		[[nodiscard]] hstring DeviceGUID() { return _deviceGUID; }
		[[nodiscard]] hstring ErrorText() { return _errorText; }
		[[nodiscard]] hstring Location() { return _location; }
		[[nodiscard]] bool IsLoaded() const { return _isLoaded; }
		[[nodiscard]] bool LoadError() const { return _loadError; }

		Windows::Foundation::IAsyncAction DisplayName(const hstring& value);
		Windows::Foundation::IAsyncAction DeviceGUID(const hstring& value);
		Windows::Foundation::IAsyncAction ErrorText(const hstring& value);
		Windows::Foundation::IAsyncAction Location(const hstring& value);
		Windows::Foundation::IAsyncAction IsLoaded(const bool& value);
		Windows::Foundation::IAsyncAction LoadError(const bool& value);

		hstring TrimString(const hstring& value, const uint64_t& chars)
		{
			return (value.size() > chars
				        ? std::wstring(value).substr(0, chars - 1) + L"..."
				        : std::wstring(value)).c_str();
		}

		hstring JSONString(const hstring& key)
		{
			return k2app::interfacing::LocalizedJSONString(key.c_str()).c_str();
		}

		bool InvertBoolean(const bool& value)
		{
			return !value;
		}

		Microsoft::UI::Xaml::Input::TappedEventHandler OpenDevicePath()
		{
			// Open the device path
			std::filesystem::path __location = _location.c_str();

			// Repeat until there's no parent or the path is valid
			while (true)
				if (!exists(__location) && __location.has_parent_path())
					__location = __location.parent_path();
				else break;

			// Open the path we've got
			k2app::interfacing::openFolderAndSelectItem(__location);

			// Return a dummy callback handler
			return [&](auto, auto) {};
		}

		[[nodiscard]] event_token PropertyChanged(
			const Microsoft::UI::Xaml::Data::PropertyChangedEventHandler& value);

		void PropertyChanged(const event_token& token);

	private:
		hstring _displayName;
		hstring _deviceGUID;
		hstring _errorText;
		hstring _location;
		bool _isLoaded;
		bool _loadError;

		event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> _propertyChanged;
	};
}

namespace winrt::Amethyst::factory_implementation
{
	struct PluginEntryView : PluginEntryViewT<PluginEntryView, implementation::PluginEntryView>
	{
	};
}
