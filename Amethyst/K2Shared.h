#pragma once
#include "pch.h"

#include <codecvt>

#include "K2Settings.h"

inline std::array<std::wstring, 3> split_status(const std::wstring& s)
{
	// If there are 3 strings separated by \n
	return std::array<std::wstring, 3>{
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
			consoleItem;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Window> thisAppWindow;

		inline std::shared_ptr<winrt::Microsoft::UI::Dispatching::DispatcherQueue> thisDispatcherQueue;

		inline std::shared_ptr<winrt::Microsoft::Windows::AppNotifications::AppNotificationManager>
		thisNotificationManager;

		inline std::shared_ptr<winrt::Microsoft::Windows::ApplicationModel::Resources::ResourceManager>
		thisResourceManager;

		inline std::shared_ptr<winrt::Microsoft::Windows::ApplicationModel::Resources::ResourceContext>
		thisResourceContext;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TextBlock> appTitleLabel;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Grid>
			interfaceBlockerGrid, navigationBlockerGrid;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::NavigationView> mainNavigationView;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Frame> mainContentFrame;

		namespace navigation_items
		{
			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Canvas>
			navViewDevicesButtonIconCanvas;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Shapes::Path>
				navViewDevicesButtonIcon_Empty,
				navViewDevicesButtonIcon_Solid;

			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::FontIcon>
				navViewGeneralButtonIcon,
				navViewSettingsButtonIcon,
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

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Button>
			calibrationButton,
			reRegisterButton,
			serverOpenDiscordButton,
			offsetsButton;

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

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ToggleSplitButton>
		toggleFreezeButton;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::CheckBox>
		freezeOnlyLowerCheckBox;
	}

	namespace settings
	{
		inline bool settings_localInitFinished = false;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Button> restartButton;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::CheckBox>
			externalFlipCheckBox,
			autoSpawnCheckbox,
			enableSoundsCheckbox,
			autoStartCheckBox;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TextBlock>
			externalFlipCheckBoxLabel,
			setErrorFlyoutText,
			externalFlipStatusLabel;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Grid>
		flipDropDownGrid;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ToggleSwitch> flipToggle;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Slider> soundsVolumeSlider;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Expander> flipDropDown;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::StackPanel>
			externalFlipStackPanel, jointExpanderHostStackPanel,
			externalFlipStatusStackPanel, flipDropDownContainer;
	}

	namespace devices
	{
		inline bool devices_tab_setup_finished = false,
		            devices_tab_re_setup_finished = false,
		            devices_signal_joints = true;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TextBlock>
			deviceNameLabel,
			deviceStatusLabel,
			errorWhatText,
			baseDeviceName,
			overrideDeviceName,
			trackingDeviceErrorLabel,
			overridesLabel,
			jointBasisLabel;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Grid>
			deviceErrorGrid,
			trackingDeviceChangePanel,
			devicesMainContentGridOuter,
			devicesMainContentGridInner,
			selectedDeviceSettingsHostContainer;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::ListView> devicesListView;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Flyout> noJointsFlyout;

		inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::Button>
			setAsOverrideButton,
			setAsBaseButton,
			deselectDeviceButton; // This one's override-only

		inline std::binary_semaphore smphSignalCurrentUpdate{0},
		                             smphSignalStartMain{0};
		inline uint32_t selectedTrackingDeviceID = 0;

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
			toggleTrackersTeachingTip;
		}

		namespace settings
		{
			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TeachingTip>
			manageTrackersTeachingTip;
		}

		namespace devices
		{
			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TeachingTip>
			devicesListTeachingTip;
		}

		namespace info
		{
			inline std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::TeachingTip>
			endingTeachingTip;
		}
	}

	namespace semaphores
	{
		inline std::binary_semaphore
			semaphore_ReloadPage_MainWindow{ 0 },
			semaphore_ReloadPage_GeneralPage{ 0 },
			semaphore_ReloadPage_SettingsPage{ 0 },
			semaphore_ReloadPage_DevicesPage{ 0 },
			semaphore_ReloadPage_InfoPage{ 0 };
	}
}

namespace k2app::interfacing
{
	// Return a language name by code
	// Input: The current (or deduced) language key / en
	// Returns: LANG_NATIVE (LANG_LOCALIZED) / Nihongo (Japanese)
	inline std::wstring GetLocalizedLanguageName(const std::wstring& language_key)
	{
		// Load the locales.json from Assets/Strings/

		const boost::filesystem::path resource_path =
			boost::dll::program_location().parent_path() /
			"Assets" / "Strings" / "locales.json";

		// If the specified language doesn't exist somehow, fallback to 'en'
		if (!exists(resource_path))
		{
			LOG(ERROR) << "Could not load language enumeration resources at \"" <<
				resource_path.string() << "\", app interface will be broken!";
			return language_key; // Give up on trying
		}

		// If everything's ok, load the resources into the current resource tree
		boost::property_tree::wptree w_enum_resources;
		read_json(resource_path.string(), w_enum_resources,
			std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

		// Check if the resource root is fine
		if (w_enum_resources.empty())
		{
			LOG(ERROR) << "The current language enumeration resource root is empty!"
				"App interface will be broken!";
			return language_key; // Give up on trying
		}

		// Check if the language code exists
		if (w_enum_resources.find(language_key) != w_enum_resources.not_found() && // Find the current language
			// Check if the current language's name (local) exists in the property tree
			w_enum_resources.get_optional<std::wstring>(
				language_key + L"." + language_key).has_value() &&
			// Check if the current language's name (self) exists in the property tree
			w_enum_resources.get_optional<std::wstring>(
				K2Settings.appLanguage + L"." + K2Settings.appLanguage).has_value() &&
			// Check if the desired (set) language's name (local) exists in the property tree
			w_enum_resources.get_optional<std::wstring>(
				K2Settings.appLanguage + L"." + language_key).has_value())
		{
			// If everything's fine, compose the return

			// If the language key is the current language, don't split the name
			if (K2Settings.appLanguage == language_key)
				return w_enum_resources.get<std::wstring>(K2Settings.appLanguage + L"." + K2Settings.appLanguage);

			// Else split the same way as in docs
			return w_enum_resources.get<std::wstring>(language_key + L"." + language_key) +
				L" (" + w_enum_resources.get<std::wstring>(K2Settings.appLanguage + L"." + language_key) + L")";
		}

		// Else return they key alone
		return language_key;
	}

	// Amethyst language resource trees
	inline boost::property_tree::wptree
		w_local_resources, w_english_resources, w_language_enum;

	// Load the current desired resource JSON into app memory
	inline void LoadJSONStringResources(const std::wstring& language_key)
	{
		boost::filesystem::path resource_path =
			boost::dll::program_location().parent_path() /
			"Assets" / "Strings" / (language_key + L".json");

		// If the specified language doesn't exist somehow, fallback to 'en'
		if (!exists(resource_path))
		{
			LOG(WARNING) << "Could not load language resources at \"" <<
				resource_path.string() << "\", falling back to 'en' (en.json)!";

			resource_path = boost::dll::program_location().parent_path() /
				"Assets" / "Strings" / "en.json";
		}

		// If failed again, just give up
		if (!exists(resource_path))
		{
			LOG(ERROR) << "Could not load language resources at \"" <<
				resource_path.string() << "\", the app interface will be broken!";
			return; // Just give up
		}

		// If everything's ok, load the resources into the current resource tree
		read_json(resource_path.string(), w_local_resources,
			std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

		// Check if the resource root is fine
		if (w_local_resources.empty())
			LOG(ERROR) << "The current resource root is empty! App interface will be broken!";
	}

	// Load the english resource JSON into app memory
	inline void LoadJSONStringResources_English()
	{
		const boost::filesystem::path resource_path =
			boost::dll::program_location().parent_path() /
			"Assets" / "Strings" / "en.json";

		// If the specified language doesn't exist somehow, fallback to 'en'
		if (!exists(resource_path))
		{
			LOG(ERROR) << "Could not load language resources at \"" <<
				resource_path.string() << "\", falling back to the current one! " <<
				"WARNING: The app interface will be broken if not existent!";

			// Override the current english resource tree
			w_english_resources = w_local_resources;
			return; // Give up on trying
		}

		// If everything's ok, load the resources into the current resource tree
		read_json(resource_path.string(), w_english_resources,
			std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

		// Check if the resource root is fine
		if (w_english_resources.empty())
			LOG(ERROR) << "The current resource root is empty! App interface will be broken!";
	}

	// Get a string from runtime JSON resources, language from settings
	inline std::wstring LocalizedJSONString(const std::wstring& resource_key)
	{
		// Check if the resource root is fine
		if (w_local_resources.empty())
		{
			LOG(ERROR) << "The current resource root is empty! App interface will be broken!";
			return L""; // Just give up
		}

		// Check if the desired key exists
		if (w_local_resources.find(resource_key) == w_local_resources.not_found())
		{
			LOG(ERROR) << "Could not find a resource string with key (narrowed) \"" <<
				WStringToString(resource_key) << "\" in the current resource! App interface will be broken!";
			return L""; // Just give up
		}

		return w_local_resources.get<std::wstring>(resource_key);
	}

	// Get a string from runtime JSON resources, specify language
	inline std::wstring LocalizedJSONString_EN(const std::wstring& resource_key)
	{
		// Check if the resource root is fine
		if (w_english_resources.empty())
		{
			LOG(ERROR) << "The current resource root is empty! App interface will be broken!";
			return L""; // Just give up
		}
		
		// Check if the desired key exists
		if (w_english_resources.find(resource_key) == w_english_resources.not_found())
		{
			LOG(ERROR) << "Could not find a resource string with key (narrowed) \"" <<
				WStringToString(resource_key) << "\" in the current resource! App interface will be broken!";
			return L""; // Just give up
		}

		return w_english_resources.get<std::wstring>(resource_key);
	}

	// Get a string from resources (crash handler's LangResString)
	inline std::wstring LocalizedResourceWString(const std::wstring& dictionary, const std::wstring& key)
	{
		return LocalizedJSONString(L"/" + dictionary + L"/" + key);
	}
}
