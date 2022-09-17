#pragma once
#include "pch.h"
#include "K2Settings.h"

#include "DeviceEntryView.h"

#include <codecvt>

inline std::array<std::wstring, 3> split_status(const std::wstring& s)
{
	// If there are 3 strings separated by \n
	return std::array{
		s.substr(0, s.find(L"\n")),
		s.substr(s.find(L"\n") + 1,
		         s.rfind(L"\n") - (s.find(L"\n") + 1)),
		s.substr(s.rfind(L"\n") + 1)
	};
}

namespace k2app::shared
{
	namespace main
	{
		// Navigate the main view (w/ animations)
		void NavView_Navigate(std::wstring navItemTag,
		                      const winrt::Microsoft::UI::Xaml::Media::Animation::NavigationTransitionInfo&
		                      transitionInfo);

		// Vector of std::pair holding the Navigation Tag and the relative Navigation Page.
		inline std::vector<std::pair<std::wstring, winrt::Windows::UI::Xaml::Interop::TypeName>> m_pages;

		// Main Window
		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::NavigationViewItem>
			generalItem,
			settingsItem,
			devicesItem,
			infoItem,
			consoleItem,
			helpButton;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Window> thisWindow;
		inline std::shared_ptr<winrt::Microsoft::UI::Windowing::AppWindow> thisAppWindow;

		inline HWND thisAppWindowID{nullptr};

		inline std::shared_ptr<winrt::Microsoft::UI::Dispatching::DispatcherQueue> thisDispatcherQueue;

		inline std::shared_ptr<winrt::Microsoft::Windows::AppNotifications::AppNotificationManager>
		thisNotificationManager;

		inline std::shared_ptr<winrt::Microsoft::Windows::ApplicationModel::Resources::ResourceManager>
		thisResourceManager;

		inline std::shared_ptr<winrt::Microsoft::Windows::ApplicationModel::Resources::ResourceContext>
		thisResourceContext;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TextBlock>
			appTitleLabel,
			flyoutHeader,
			flyoutFooter,
			flyoutContent;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Grid>
			interfaceBlockerGrid, navigationBlockerGrid;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::NavigationView> mainNavigationView;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Frame> mainContentFrame;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::FontIcon> updateIconDot;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Flyout> updateFlyout;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Button>
			installNowButton,
			installLaterButton;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Media::SolidColorBrush>
			attentionBrush, neutralBrush;

		namespace navigation_items
		{
			/*inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Canvas>
			navViewDevicesButtonIconCanvas;*/

			/*inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Shapes::Path>
				navViewDevicesButtonIcon_Empty,
				navViewDevicesButtonIcon_Solid;*/

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::FontIcon>
				navViewGeneralButtonIcon,
				navViewSettingsButtonIcon,
				navViewDevicesButtonIcon,
				navViewInfoButtonIcon,
				navViewOkashiButtonIcon;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TextBlock>
				navViewGeneralButtonLabel,
				navViewSettingsButtonLabel,
				navViewDevicesButtonLabel,
				navViewInfoButtonLabel,
				navViewOkashiButtonLabel;
		}
	}

	namespace general
	{
		// General Page
		inline bool general_tab_setup_finished = false,
		            pending_offsets_update = false;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Primitives::ToggleButton>
		toggleTrackersButton;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ToggleSplitButton>
		skeletonToggleButton;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::CheckBox>
		forceRenderCheckBox;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::MenuFlyoutItem>
		offsetsButton;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Button>
			calibrationButton,
			reRegisterButton,
			serverOpenDiscordButton;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TextBlock>
			versionLabel,
			deviceNameLabel,
			deviceStatusLabel,
			errorWhatText,
			trackingDeviceErrorLabel,
			overrideDeviceNameLabel,
			overrideDeviceStatusLabel,
			overrideErrorWhatText,
			overrideDeviceErrorLabel,
			serverStatusLabel,
			serverErrorLabel,
			serverErrorWhatText,
			forceRenderText;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Grid>
			offsetsControlHostGrid,
			errorButtonsGrid,
			errorWhatGrid,
			overrideErrorButtonsGrid,
			overrideErrorWhatGrid,
			serverErrorWhatGrid,
			serverErrorButtonsGrid;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Primitives::ToggleButton>
		toggleFreezeButton;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ToggleMenuFlyoutItem>
		freezeOnlyLowerToggle;
	}

	namespace settings
	{
		inline bool settings_localInitFinished = false;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Button> restartButton;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::CheckBox>
			externalFlipCheckBox,
			autoSpawnCheckbox,
			enableSoundsCheckbox,
			autoStartCheckBox,
			checkOverlapsCheckBox;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TextBlock>
			externalFlipCheckBoxLabel,
			setErrorFlyoutText,
			externalFlipStatusLabel;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Grid>
		flipDropDownGrid;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ScrollViewer>
		pageMainScrollViewer;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ToggleSwitch> flipToggle;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Slider> soundsVolumeSlider;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Expander> flipDropDown;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::StackPanel>
			externalFlipStackPanel, jointExpanderHostStackPanel,
			externalFlipStatusStackPanel, flipDropDownContainer;
	}

	namespace devices
	{
		inline bool devices_tab_setup_finished = false, // On-load setup
			devices_tab_re_setup_finished = false, // Other setup
			devices_overrides_setup_pending = false, // Overrides
			devices_signal_joints = true, // Optionally no signal
			devices_mvvm_setup_finished = false; // MVVM setup done?

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TextBlock>
			deviceNameLabel,
			deviceStatusLabel,
			errorWhatText,
			trackingDeviceErrorLabel,
			overridesLabel,
			jointBasisLabel;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Grid>
			deviceErrorGrid,
			trackingDeviceChangePanel,
			devicesMainContentGridOuter,
			devicesMainContentGridInner,
			selectedDeviceSettingsHostContainer;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TreeView> devicesTreeView;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Flyout> noJointsFlyout;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Button>
			setAsOverrideButton,
			setAsBaseButton,
			deselectDeviceButton; // This one's override-only

		inline std::binary_semaphore smphSignalCurrentUpdate{0},
		                             smphSignalStartMain{0};

		inline uint32_t selectedTrackingDeviceID = 0;
		inline std::wstring selectedTrackingDeviceName = L"";

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ScrollViewer> devicesMainContentScrollViewer;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::StackPanel>
			devicesOverridesSelectorStackPanelOuter,
			devicesOverridesSelectorStackPanelInner,

			devicesJointsBasisSelectorStackPanelOuter,
			devicesJointsBasisSelectorStackPanelInner,

			selectedDeviceSettingsRootLayoutPanel,

			jointsBasisExpanderHostStackPanel,
			overridesExpanderHostStackPanel;
	}

	// (Only the first ones are needed)
	namespace teaching_tips
	{
		namespace main
		{
			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TeachingTip>
			initializerTeachingTip;
		}

		namespace general
		{
			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TeachingTip>
				toggleTrackersTeachingTip, statusTeachingTip;
		}

		namespace settings
		{
			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TeachingTip>
				manageTrackersTeachingTip, autoStartTeachingTip;
		}

		namespace devices
		{
			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TeachingTip>
				devicesListTeachingTip, deviceControlsTeachingTip;
		}

		namespace info
		{
			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TeachingTip>
			helpTeachingTip;
		}
	}

	namespace semaphores
	{
		inline std::binary_semaphore
			semaphore_ReloadPage_MainWindow{0},
			semaphore_ReloadPage_GeneralPage{0},
			semaphore_ReloadPage_SettingsPage{0},
			semaphore_ReloadPage_DevicesPage{0},
			semaphore_ReloadPage_InfoPage{0};
	}
}

namespace k2app::interfacing
{
	// Return a language name by code
	// Input: The current (or deduced) language key / en
	// Returns: LANG_NATIVE (LANG_LOCALIZED) / Nihongo (Japanese)
	// https://stackoverflow.com/a/10607146/13934610
	// https://stackoverflow.com/a/51867679/13934610
	inline std::wstring GetLocalizedLanguageName(const std::wstring& language_key)
	{
		try
		{
			// Load the locales.json from Assets/Strings/

			const std::filesystem::path resource_path =
				GetProgramLocation().parent_path() /
				"Assets" / "Strings" / "locales.json";

			// If the specified language doesn't exist somehow, fallback to 'en'
			if (!exists(resource_path))
			{
				LOG(ERROR) << "Could not load language enumeration resources at \"" <<
					WStringToString(resource_path.wstring()) << "\", app interface will be broken!";
				return language_key; // Give up on trying
			}

			// Load the JSON source into buffer
			std::wifstream wif(resource_path);
			wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
			std::wstringstream wss;
			wss << wif.rdbuf();

			// Parse the loaded json
			const auto json_object = winrt::Windows::Data::Json::JsonObject::Parse(wss.str());

			// Check if the resource root is fine
			if (json_object.Size() <= 0)
			{
				LOG(ERROR) << "The current language enumeration resource root is empty!"
					"App interface will be broken!";
				return language_key; // Give up on trying
			}

			// If the language key is the current language, don't split the name
			if (K2Settings.appLanguage == language_key)
				return json_object.GetNamedObject(K2Settings.appLanguage).GetNamedString(K2Settings.appLanguage).
				                   c_str();

			// Else split the same way as in docs
			return (json_object.GetNamedObject(language_key).GetNamedString(language_key) +
				L" (" + json_object.GetNamedObject(K2Settings.appLanguage).GetNamedString(language_key) + L")").c_str();
		}
		catch (const winrt::hresult_error& ex)
		{
			LOG(ERROR) << "JSON error at key: \"" <<
				WStringToString(language_key) << "\"! Message: "
				<< WStringToString(ex.message().c_str());

			// Else return they key alone
			return language_key;
		}
		catch (...)
		{
			LOG(ERROR) << "An exception occurred! "
				"The current language enumeration will be empty! "
				"App interface will be broken!";

			// Else return they key alone
			return language_key;
		}
	}

	// Amethyst language resource trees
	inline winrt::Windows::Data::Json::JsonObject
		m_local_resources, m_english_resources, m_language_enum;

	// Load the current desired resource JSON into app memory
	inline void LoadJSONStringResources(const std::wstring& language_key)
	{
		try
		{
			LOG(INFO) << "Searching for language resources with key \"" <<
				WStringToString(language_key) << "\"...";

			std::filesystem::path resource_path =
				GetProgramLocation().parent_path() /
				"Assets" / "Strings" / (language_key + L".json");

			// If the specified language doesn't exist somehow, fallback to 'en'
			if (!exists(resource_path))
			{
				LOG(WARNING) << "Could not load language resources at \"" <<
					WStringToString(resource_path.wstring()) << "\", falling back to 'en' (en.json)!";

				resource_path = GetProgramLocation().parent_path() /
					"Assets" / "Strings" / "en.json";
			}

			// If failed again, just give up
			if (!exists(resource_path))
			{
				LOG(ERROR) << "Could not load language resources at \"" <<
					WStringToString(resource_path.wstring()) << "\", the app interface will be broken!";
				return; // Just give up
			}

			// If everything's ok, load the resources into the current resource tree

			// Load the JSON source into buffer
			std::wifstream wif(resource_path);
			wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
			std::wstringstream wss;
			wss << wif.rdbuf();

			// Parse the loaded json
			m_local_resources = winrt::Windows::Data::Json::JsonObject::Parse(wss.str());

			// Check if the resource root is fine
			if (m_local_resources.Size() <= 0)
				LOG(ERROR) << "The current resource root is empty! App interface will be broken!";

			else
				LOG(INFO) << "Successfully loaded language resources with key \"" <<
					WStringToString(language_key) << "\"!";
		}
		catch (const winrt::hresult_error& ex)
		{
			LOG(ERROR) << "JSON error at key: \"" <<
				WStringToString(language_key) << "\"! Message: "
				<< WStringToString(ex.message().c_str());
		}
		catch (...)
		{
			LOG(ERROR) <<
				"An exception occurred! The current resource root will be empty! App interface will be broken!";
		}
	}

	// Load the english resource JSON into app memory
	inline void LoadJSONStringResources_English()
	{
		try
		{
			LOG(INFO) << "Searching for shared (English) language resources...";

			const std::filesystem::path resource_path =
				GetProgramLocation().parent_path() /
				"Assets" / "Strings" / "en.json";

			// If the specified language doesn't exist somehow, fallback to 'en'
			if (!exists(resource_path))
			{
				LOG(ERROR) << "Could not load language resources at \"" <<
					WStringToString(resource_path.wstring()) << "\", falling back to the current one! " <<
					"WARNING: The app interface will be broken if not existent!";

				// Override the current english resource tree
				m_english_resources = m_local_resources;
				return; // Give up on trying
			}

			// If everything's ok, load the resources into the current resource tree

			// Load the JSON source into buffer
			std::wifstream wif(resource_path);
			wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
			std::wstringstream wss;
			wss << wif.rdbuf();

			// Parse the loaded json
			m_english_resources = winrt::Windows::Data::Json::JsonObject::Parse(wss.str());

			// Check if the resource root is fine
			if (m_english_resources.Size() <= 0)
				LOG(ERROR) << "The current resource root is empty! App interface will be broken!";

			else
				LOG(INFO) << "Successfully loaded shared (English) language resources!";
		}
		catch (const winrt::hresult_error& ex)
		{
			LOG(ERROR) << "JSON error at key: \"en\"! Message: "
				<< WStringToString(ex.message().c_str());
		}
		catch (...)
		{
			LOG(ERROR) <<
				"An exception occurred! The current resource root will be empty! App interface will be broken!";
		}
	}

	// Get a string from runtime JSON resources, language from settings
	inline std::wstring LocalizedJSONString(const std::wstring& resource_key)
	{
		try
		{
			// Check if the resource root is fine
			if (m_local_resources.Size() <= 0)
			{
				LOG(ERROR) << "The current resource root is empty! App interface will be broken!";
				return resource_key; // Just give up
			}

			return m_local_resources.GetNamedString(resource_key).c_str();
		}
		catch (const winrt::hresult_error& ex)
		{
			LOG(ERROR) << "JSON error at key: \"" <<
				WStringToString(resource_key) << "\"! Message: "
				<< WStringToString(ex.message().c_str());

			// Else return they key alone
			return resource_key;
		}
		catch (...)
		{
			LOG(ERROR) << "An exception occurred! App interface will be broken!";

			// Else return they key alone
			return resource_key;
		}
	}

	// Get a string from runtime JSON resources, specify language
	inline std::wstring LocalizedJSONString_EN(const std::wstring& resource_key)
	{
		try
		{
			// Check if the resource root is fine
			if (m_english_resources.Size() <= 0)
			{
				LOG(ERROR) << "The current EN resource root is empty! App interface will be broken!";
				return resource_key; // Just give up
			}

			return m_english_resources.GetNamedString(resource_key).c_str();
		}
		catch (const winrt::hresult_error& ex)
		{
			LOG(ERROR) << "JSON error at key: \"" <<
				WStringToString(resource_key) << "\"! Message: "
				<< WStringToString(ex.message().c_str());

			// Else return they key alone
			return resource_key;
		}
		catch (...)
		{
			LOG(ERROR) << "An exception occurred! App interface will be broken!";

			// Else return they key alone
			return resource_key;
		}
	}

	// Get a string from resources (crash handler's LangResString)
	inline std::wstring LocalizedResourceWString(const std::wstring& dictionary, const std::wstring& key)
	{
		return LocalizedJSONString(L"/" + dictionary + L"/" + key);
	}
}

namespace TrackingDevices
{
	// Vector of current devices' JSON resource roots & paths
	// Note: the size must be the same as TrackingDevicesVector's
	inline std::vector<std::pair<
		winrt::Windows::Data::Json::JsonObject, std::filesystem::path>>
	TrackingDevicesLocalizationResourcesRootsVector;
}
