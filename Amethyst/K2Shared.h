#pragma once
#include "pch.h"
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
			const winrt::Microsoft::UI::Xaml::Media::Animation::NavigationTransitionInfo& transitionInfo);

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
}

namespace k2app::interfacing
{
	// Get a string from resources (crash handler's LangResString)
	inline std::wstring LocalizedResourceWString(const std::wstring& dictionary, const std::wstring& key)
	{
		winrt::Windows::Globalization::Language language{
			winrt::Windows::System::UserProfile::GlobalizationPreferences::Languages().GetAt(0)
		};

		shared::main::thisResourceContext->QualifierValues().Lookup(L"Language") = language.LanguageTag();

		return shared::main::thisResourceManager.get()->MainResourceMap().GetValue(
			(dictionary + L"/" + key).c_str()).ValueAsString().c_str();
	}

	// Get a string from resources (crash handler's LangResString)
	inline std::wstring LocalizedResourceWString(
		const std::wstring& dictionary, const std::wstring& key, const std::wstring& language)
	{
		shared::main::thisResourceContext->QualifierValues().Lookup(L"Language") = language;

		return shared::main::thisResourceManager.get()->MainResourceMap().GetValue(
			(dictionary + L"/" + key).c_str()).ValueAsString().c_str();
	}
}
