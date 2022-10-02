#include "pch.h"
#include "PluginEntryView.h"
#if __has_include("PluginEntryView.g.cpp")
#include "PluginEntryView.g.cpp"
#endif

namespace winrt::Amethyst::implementation
{
	PluginEntryView::PluginEntryView(
		const hstring& DisplayName,
		const hstring& DeviceGUID,
		const hstring& ErrorText,
		const hstring& Location,
		const bool& IsLoaded,
		const bool& LoadError) :

		_displayName(DisplayName),
		_deviceGUID(DeviceGUID),
		_errorText(ErrorText),
		_location(Location),
		_isLoaded(IsLoaded),
		_loadError(LoadError)
	{
	}

    Windows::Foundation::IAsyncAction PluginEntryView::DisplayName(const hstring& value)
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

    Windows::Foundation::IAsyncAction PluginEntryView::DeviceGUID(const hstring& value)
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

    Windows::Foundation::IAsyncAction PluginEntryView::ErrorText(const hstring& value)
    {
        if (_errorText != value)
        {
            _errorText = value;

            LOG(INFO) << "Changing the apartment thread context to the constructor thread...";
            co_await *k2app::shared::DeviceEntryView::thisEntryContext;

            LOG(INFO) << "Invoking PropertyChanged for ErrorText...";
            _propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"ErrorText"));
        }
    }

    Windows::Foundation::IAsyncAction PluginEntryView::Location(const hstring& value)
    {
        if (_location != value)
        {
            _location = value;

            LOG(INFO) << "Changing the apartment thread context to the constructor thread...";
            co_await *k2app::shared::DeviceEntryView::thisEntryContext;

            LOG(INFO) << "Invoking PropertyChanged for Location...";
            _propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"Location"));
        }
    }

    Windows::Foundation::IAsyncAction PluginEntryView::IsLoaded(const bool& value)
    {
        if (_isLoaded != value)
        {
            _isLoaded = value;

            LOG(INFO) << "Changing the apartment thread context to the constructor thread...";
            co_await *k2app::shared::DeviceEntryView::thisEntryContext;

            // Disable/Enable this plugin
            if (!value)
                k2app::K2Settings.disabledDevicesGuidSet.insert(_deviceGUID.c_str());

            else
                k2app::K2Settings.disabledDevicesGuidSet.erase(_deviceGUID.c_str());

            // Check if the change is valid
            if (!_isLoaded)
            {
                std::set<std::wstring> _loadedDeviceSet;

                // Check which devices are loaded
                if (TrackingDevices::deviceGUID_ID_Map.contains(L"K2VRTEAM-AME1-API1-DVCE-DVCEKINECTV1"))
                    _loadedDeviceSet.insert(L"K2VRTEAM-AME1-API1-DVCE-DVCEKINECTV1");
                if (TrackingDevices::deviceGUID_ID_Map.contains(L"K2VRTEAM-AME1-API1-DVCE-DVCEKINECTV2"))
                    _loadedDeviceSet.insert(L"K2VRTEAM-AME1-API1-DVCE-DVCEKINECTV2");
                if (TrackingDevices::deviceGUID_ID_Map.contains(L"K2VRTEAM-AME1-API1-DVCE-DVCEPSMOVEEX"))
                    _loadedDeviceSet.insert(L"K2VRTEAM-AME1-API1-DVCE-DVCEPSMOVEEX");
                if (TrackingDevices::deviceGUID_ID_Map.contains(L"K2VRTEAM-VEND-API1-DVCE-DVCEOWOTRACK"))
                    _loadedDeviceSet.insert(L"K2VRTEAM-VEND-API1-DVCE-DVCEOWOTRACK");

                // If this device entry happens to be one of these
                if (_loadedDeviceSet.contains(_deviceGUID.c_str()))
                {
                    // Check flag, assume failure
                    bool _one_enabled = false;

	                // Loop over all loaded devices and check which aren't disabled
                    for (const auto& _entry : _loadedDeviceSet)
                        if (!k2app::K2Settings.disabledDevicesGuidSet.contains(_entry))
                            _one_enabled = true; // This device is enabled & loaded

                    // If we've just disabled the last loaded one, re-enable the first
                    if (!_one_enabled)
                    {
                        k2app::K2Settings.disabledDevicesGuidSet.erase(_deviceGUID.c_str());
                        _isLoaded = true; // Re-enable this device
                    }
                }
            }

            // Save settings
            k2app::K2Settings.saveSettings();
            
            // Show the reload tip on any valid changes
            // == cause the upper check would make it different
            // and it's already been assigned at the beginning
            if (_isLoaded == value) 
				k2app::shared::teaching_tips::main::reloadTeachingTip->IsOpen(true);

            LOG(INFO) << "Invoking PropertyChanged for IsLoaded...";
            _propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"IsLoaded"));
        }
    }

    Windows::Foundation::IAsyncAction PluginEntryView::LoadError(const bool& value)
    {
        if (_loadError != value)
        {
            _loadError = value;

            LOG(INFO) << "Changing the apartment thread context to the constructor thread...";
            co_await *k2app::shared::DeviceEntryView::thisEntryContext;

            LOG(INFO) << "Invoking PropertyChanged for LoadError...";
            _propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"LoadError"));
        }
    }
    
	event_token PluginEntryView::PropertyChanged(
		const Microsoft::UI::Xaml::Data::PropertyChangedEventHandler& handler)
	{
		return _propertyChanged.add(handler);
	}

	void PluginEntryView::PropertyChanged(const event_token& token)
	{
		_propertyChanged.remove(token);
	}
}
