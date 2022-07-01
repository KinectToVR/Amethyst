#pragma once
#include "pch.h"

#include "K2Interfacing.h"
#include "K2Shared.h"

// Forward declaration
namespace winrt::Microsoft::UI::Xaml::Controls
{
	class JointExpander;
}

// Extension of the k2/shared namespace
namespace k2app::shared::settings
{
	inline std::vector<std::shared_ptr<
		winrt::Microsoft::UI::Xaml::Controls::JointExpander>> jointExpanderVector;
}

namespace TrackingDevices
{
	// Vector of currently available tracking devices
	// std::variant cause there are 3 possible device types
	inline std::vector<
		std::variant<
			ktvr::K2TrackingDeviceBase_KinectBasis*,
			ktvr::K2TrackingDeviceBase_JointsBasis*>>
	TrackingDevicesVector;

	// Extract the current device (variant of it)
	inline auto getCurrentDevice()
	{
		// trackingDeviceID is always >= 0 anyway
		return TrackingDevicesVector.at(k2app::K2Settings.trackingDeviceID);
	}

	// Extract the current device (variant of it)
	inline auto getCurrentDevice(const uint32_t& id)
	{
		// trackingDeviceID is always >= 0 anyway
		return TrackingDevicesVector.at(id);
	}

	// Extract the current device (variant of it)
	inline auto getCurrentOverrideDevice()
	{
		// trackingDeviceID is always >= 0 anyway
		return TrackingDevicesVector.at(k2app::K2Settings.overrideDeviceID);
	}

	// Extract the current device (variant of it)
	inline std::pair<
		bool, std::variant<
			ktvr::K2TrackingDeviceBase_KinectBasis*,
			ktvr::K2TrackingDeviceBase_JointsBasis*>> getCurrentOverrideDevice_Safe()
	{
		bool _exists = k2app::K2Settings.overrideDeviceID >= 0 &&
			TrackingDevicesVector.size() > k2app::K2Settings.overrideDeviceID;

		// Assuming that the caller will test in pair.first is true,
		// we can push the id0 device here as well if pair.first is gonna be false
		uint32_t _deviceID = _exists ? k2app::K2Settings.overrideDeviceID : 0;

		// trackingDeviceID is always >= 0 anyway
		return std::make_pair(_exists,
		                      TrackingDevicesVector.at(_deviceID));
	}

	// Extract the current device (variant of it)
	inline std::pair<
		bool, std::variant<
			ktvr::K2TrackingDeviceBase_KinectBasis*,
			ktvr::K2TrackingDeviceBase_JointsBasis*>> getCurrentOverrideDevice_Safe(const uint32_t& id)
	{
		bool _exists = TrackingDevicesVector.size() > id;

		// Assuming that the caller will test in pair.first is true,
		// we can push the id0 device here as well if pair.first is gonna be false
		uint32_t _deviceID = _exists ? id : 0;

		// trackingDeviceID is always >= 0 anyway
		return std::make_pair(_exists,
		                      TrackingDevicesVector.at(_deviceID));
	}

	inline bool isExternalFlipSupportable()
	{
		bool isFlipSupported = false;

		/* First check if our tracking device even supports normal flip */

		const auto& trackingDevice =
			getCurrentDevice();

		if (trackingDevice.index() == 0)
			isFlipSupported = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(
				trackingDevice)->isFlipSupported();

		bool isExternalFlipSupported = false, // inapp - overridden/disabled
		     isExternalFlipSupported_Global = false; // global - steamvr

		/* Now check if either waist tracker is overridden or disabled
		 * And then search in OpenVR for a one with waist role */

		const auto& overrideDevice =
			getCurrentOverrideDevice_Safe();

		// If we have an override and if it's actually affecting the waist rotation
		if (overrideDevice.first &&
			k2app::K2Settings.K2TrackersVector.at(0).data.isActive &&
			k2app::K2Settings.K2TrackersVector.at(0).isRotationOverridden)
		{
			// If the override device is a kinect then it HAS NOT TO support flip
			if (overrideDevice.second.index() == 0)
				isExternalFlipSupported = !std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(
					overrideDevice.second)->isFlipSupported();

				// If the override device is a joints then it's always ok
			else if (overrideDevice.second.index() == 1)
				isExternalFlipSupported = true;
		}

		// If still not, then also check if the waist is disabled by any chance
		else if (!k2app::K2Settings.K2TrackersVector.at(0).data.isActive)
			isExternalFlipSupported = true;

		/* Here check if there's a proper waist tracker in steamvr to pull data from */
		if (isExternalFlipSupported)
			isExternalFlipSupported_Global = k2app::interfacing::findVRTracker("waist").first; // .first is [Success?]

		return isExternalFlipSupported_Global;
	}

	// autoCheck->true will force the function to check and false will assume unsupported
	inline void settings_set_external_flip_is_enabled(bool autoCheck = true)
	{
		if (k2app::shared::settings::externalFlipCheckBox.get() == nullptr)return;

		if (autoCheck)
		{
			k2app::shared::settings::externalFlipCheckBox.get()->IsEnabled(
				isExternalFlipSupportable() &&
				k2app::K2Settings.isFlipEnabled);
		}
		else
			k2app::shared::settings::externalFlipCheckBox.get()->IsEnabled(false);

		k2app::shared::settings::externalFlipCheckBoxLabel.get()->Opacity(
			k2app::shared::settings::externalFlipCheckBox.get()->IsEnabled() ? 1 : 0.5);

		if (!k2app::shared::settings::externalFlipCheckBox.get()->IsEnabled())
		{
			k2app::shared::settings::externalFlipCheckBox.get()->IsChecked(false);
			k2app::K2Settings.isExternalFlipEnabled = false;
			k2app::K2Settings.saveSettings();
		}
	}

	inline void settings_trackersConfig_UpdateIsEnabled()
	{
		// Skip if not set up yet
		if (k2app::shared::settings::flipDropDown.get() == nullptr)return;

		// Make expander opacity .5 and collapse it
		// to imitate that it's disabled

		// Flip
		if (!k2app::K2Settings.isFlipEnabled)
		{
			k2app::shared::settings::flipDropDown.get()->IsEnabled(false);
			k2app::shared::settings::flipDropDown.get()->IsExpanded(false);
		}
		else
			k2app::shared::settings::flipDropDown.get()->IsEnabled(true);
	}

	inline void settings_trackersConfigChanged(const bool showToasts = true)
	{
		// Don't react to pre-init signals
		if (!k2app::shared::settings::settings_localInitFinished)return;

		LOG(INFO) << "Trackers configuration has been changed!";
		LOG_IF(INFO, showToasts) << "Any toast won't be shown this time: force-disabled";

		// If this is the first time and happened runtime, also show the notification
		if (k2app::interfacing::K2AppTrackersSpawned)
		{
			if (k2app::shared::settings::restartButton.get() != nullptr && showToasts)
				if (!k2app::shared::settings::restartButton.get()->IsEnabled())
					k2app::interfacing::ShowToast(
						k2app::interfacing::LocalizedResourceWString(L"SharedStrings",
						                                             L"Toasts/TrackersConfigChanged/Title"),
						k2app::interfacing::LocalizedResourceWString(L"SharedStrings",
						                                             L"Toasts/TrackersConfigChanged/Content"));

			// Compare with saved settings and unlock the restart
			if (k2app::shared::settings::restartButton.get() != nullptr)
				k2app::shared::settings::restartButton.get()->IsEnabled(true);
		}

		// Enable/Disable combos
		settings_trackersConfig_UpdateIsEnabled();

		// Enable/Disable ExtFlip
		settings_set_external_flip_is_enabled();

		// Save settings
		k2app::K2Settings.saveSettings();
	}

	inline void devices_check_override_ids(const uint32_t& id)
	{
		// Take down IDs if they're too big
		if (const auto& device_pair = getCurrentOverrideDevice_Safe(id); device_pair.first)
		{
			if (device_pair.second.index() == 1) // If Joints
			{
				// Note: num_joints should never be 0
				const auto num_joints = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>
					(device_pair.second)->getTrackedJoints().size();

				for (auto& tracker : k2app::K2Settings.K2TrackersVector)
					if (tracker.positionOverrideJointID >= num_joints)
						tracker.positionOverrideJointID = 0;

				for (auto& tracker : k2app::K2Settings.K2TrackersVector)
					if (tracker.rotationOverrideJointID >= num_joints)
						tracker.rotationOverrideJointID = 0;
			}
			else if (device_pair.second.index() == 0) // If Kinect
			{
				// Note: switch based on device characteristics
				const auto characteristics = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>
					(device_pair.second)->getDeviceCharacteristics();
				uint32_t num_joints = -1; // To set later

				if (characteristics == ktvr::K2_Character_Full)
					num_joints = 8;
				else if (characteristics == ktvr::K2_Character_Simple)
					num_joints = 8;
				else if (characteristics == ktvr::K2_Character_Basic)
					num_joints = 3;

				for (auto& tracker : k2app::K2Settings.K2TrackersVector)
					if (tracker.positionOverrideJointID >= num_joints)
						tracker.positionOverrideJointID = 0;

				for (auto& tracker : k2app::K2Settings.K2TrackersVector)
					if (tracker.rotationOverrideJointID >= num_joints)
						tracker.rotationOverrideJointID = 0;
			}

			// Save it
			k2app::K2Settings.saveSettings();
		}
	}

	inline void devices_check_base_ids(const uint32_t& id)
	{
		// Take down IDs if they're too big
		if (const auto& device_pair = getCurrentDevice(id);
			device_pair.index() == 1) // If Joints
		{
			// Note: num_joints should never be 0
			const auto num_joints = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>
				(device_pair)->getTrackedJoints().size();

			for (auto& tracker : k2app::K2Settings.K2TrackersVector)
				if (tracker.selectedTrackedJointID >= num_joints)
					tracker.selectedTrackedJointID = 0;

			// Save it
			k2app::K2Settings.saveSettings();
		}
	}
}

namespace winrt::Microsoft::UI::Xaml::Controls
{
	/*
	 * Joint expander that allows to :
	 *  - select the position filter
	 *	- select the ori tracking option
	 *	- turn joint/joint pair on/off
	 */

	class JointExpander
	{
	public:
		// Note: Signals have to be set up manually
		JointExpander(std::vector<k2app::K2AppTracker*> tracker_pointers,
		              std::optional<std::wstring> title = std::nullopt)
		{
			_tracker_pointers = tracker_pointers;
			Create(title);
		}

		std::shared_ptr<Grid> Container() { return _ptr_container; }
		std::shared_ptr<Grid> MainGrid() { return _ptr_main_grid; }
		std::shared_ptr<Grid> Content() { return _ptr_content; }

		std::shared_ptr<ToggleSwitch> JointSwitch() { return _ptr_joint_switch; }
		std::shared_ptr<Expander> MainExpander() { return _ptr_main_expander; }
		std::shared_ptr<ComboBoxItem> SoftwareOrientation() { return _ptr_software_orientation; }

		std::shared_ptr<TextBlock> Title() { return _ptr_title; }
		std::shared_ptr<TextBlock> Position() { return _ptr_position; }
		std::shared_ptr<TextBlock> Orientation() { return _ptr_orientation; }

		std::shared_ptr<ComboBox> PositionCombo() { return _ptr_position_combo; }
		std::shared_ptr<ComboBox> OrientationCombo() { return _ptr_orientation_combo; }

		// Update the toggle switch's state,
		// the rest will be called automagically
		// by the switch's toggle handler
		void UpdateIsActive()
		{
			_ptr_joint_switch.get()->IsOn(
				_tracker_pointers[0]->data.isActive);

			if (!_tracker_pointers[0]->data.isActive)
			{
				_ptr_main_expander->IsEnabled(false);
				_ptr_main_expander->IsExpanded(false);
			}
			else
				_ptr_main_expander->IsEnabled(true);
		}

		// Set mathbased to enable/disable
		// Will work only if the expander is bound to a foot tracker
		void EnableSoftwareOrientation(const bool& enable)
		{
			if (_tracker_pointers[0]->tracker == ktvr::ITrackerType::Tracker_LeftFoot ||
				_tracker_pointers[0]->tracker == ktvr::ITrackerType::Tracker_RightFoot)
			{
				_ptr_software_orientation.get()->IsEnabled(enable);

				// Reset if selected and was turned off
				for (auto tracker_p : _tracker_pointers)
					if (!enable && tracker_p->orientationTrackingOption == k2app::k2_SoftwareCalculatedRotation)
					{
						tracker_p->orientationTrackingOption = k2app::k2_DeviceInferredRotation;
						_ptr_orientation_combo.get()->SelectedIndex(k2app::k2_DeviceInferredRotation);
					}
			}
		}

	protected:
		std::vector<k2app::K2AppTracker*> _tracker_pointers;

		// Underlying object shared pointer
		std::shared_ptr<Grid> _ptr_container,
		                      _ptr_main_grid, _ptr_content;
		std::shared_ptr<ToggleSwitch> _ptr_joint_switch;
		std::shared_ptr<Expander> _ptr_main_expander;
		std::shared_ptr<ComboBoxItem> _ptr_software_orientation;

		std::shared_ptr<TextBlock> _ptr_title,
		                           _ptr_position, _ptr_orientation;
		std::shared_ptr<ComboBox> _ptr_position_combo,
		                          _ptr_orientation_combo;

		// Creation: register a host and a callback
		void Create(std::optional<std::wstring> title = std::nullopt)
		{
			// Create the container grid
			Grid _container;

			// Create the main grid
			Grid _main_grid;
			_main_grid.Margin({20, 15, 60, 0});
			_main_grid.VerticalAlignment(VerticalAlignment::Top);
			_main_grid.SetValue(Canvas::ZIndexProperty(), box_value(1));

			AppendGridStarColumn(_main_grid);
			AppendGridStarColumn(_main_grid);

			// Create the title text
			TextBlock _title;
			_title.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());

			_title.Text(title.value_or(k2app::interfacing::LocalizedResourceWString(
				L"SharedStrings", L"Joints/Enum/" +
				std::to_wstring(static_cast<int>(_tracker_pointers[0]->tracker)))));

			_title.FontSize(14);
			_title.Margin({0, -3, 0, 0});
			_title.HorizontalAlignment(HorizontalAlignment::Left);
			_title.VerticalAlignment(VerticalAlignment::Center);

			_main_grid.Children().Append(_title);
			_main_grid.SetColumn(_title, 0);

			// Create the toggle switch
			ToggleSwitch _joint_switch;
			_joint_switch.Margin({0, 0, -125, 0});
			_joint_switch.OnContent(box_value(L""));
			_joint_switch.OffContent(box_value(L""));
			_joint_switch.HorizontalAlignment(HorizontalAlignment::Right);
			_joint_switch.VerticalAlignment(VerticalAlignment::Center);

			_main_grid.Children().Append(_joint_switch);
			_main_grid.SetColumn(_joint_switch, 1);

			_container.Children().Append(_main_grid);

			// Create and set up the expander content
			Grid _content;
			_content.Width(650);
			_content.HorizontalAlignment(HorizontalAlignment::Stretch);
			_content.VerticalAlignment(VerticalAlignment::Stretch);
			_content.Padding({0, -5, 0, -5});

			AppendGridStarsColumn(_content, 2);
			AppendGridStarsColumn(_content, 5);

			AppendGridPixelsRow(_content, 80);
			AppendGridPixelsRow(_content, 80);

			// Set up content caption / pos
			TextBlock _position;
			_position.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
			_position.Text(k2app::interfacing::LocalizedResourceWString(
				L"SettingsPage", L"Captions/Filters/Position"));
			_position.HorizontalAlignment(HorizontalAlignment::Left);
			_position.VerticalAlignment(VerticalAlignment::Center);
			_position.Margin({0, 0, 20, 0});

			_content.Children().Append(_position);
			_content.SetColumn(_position, 0);
			_content.SetRow(_position, 0);

			// Set up content caption / rot
			TextBlock _orientation;
			_orientation.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
			_orientation.Text(
				k2app::interfacing::LocalizedResourceWString(
					L"SettingsPage", L"Captions/Filters/Orientation"));
			_orientation.HorizontalAlignment(HorizontalAlignment::Left);
			_orientation.VerticalAlignment(VerticalAlignment::Center);
			_orientation.Margin({0, 0, 20, 0});

			_content.Children().Append(_orientation);
			_content.SetColumn(_orientation, 0);
			_content.SetRow(_orientation, 1);

			// Set up content combo / pos
			ComboBox _position_combo;
			_position_combo.HorizontalAlignment(HorizontalAlignment::Right);
			_position_combo.VerticalAlignment(VerticalAlignment::Center);
			_position_combo.Height(65);
			_position_combo.MinWidth(350);
			_position_combo.FontSize(15);
			_position_combo.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());

			// Set up (content caption / pos)'s items
			ComboBoxItem _lerp, _low_pass, _kalman, _pos_off;
			_lerp.Content(box_value(k2app::interfacing::LocalizedResourceWString(
				L"SettingsPage", L"Filters/Position/LERP")));
			_low_pass.Content(box_value(k2app::interfacing::LocalizedResourceWString(
				L"SettingsPage", L"Filters/Position/LowPass")));
			_kalman.Content(box_value(k2app::interfacing::LocalizedResourceWString(
				L"SettingsPage", L"Filters/Position/Kalman")));
			_pos_off.Content(box_value(k2app::interfacing::LocalizedResourceWString(
				L"SettingsPage", L"Filters/Position/Off")));

			_position_combo.Items().Append(_lerp);
			_position_combo.Items().Append(_low_pass);
			_position_combo.Items().Append(_kalman);
			_position_combo.Items().Append(_pos_off);

			_position_combo.SelectedIndex(0);

			_content.Children().Append(_position_combo);
			_content.SetColumn(_position_combo, 1);
			_content.SetRow(_position_combo, 0);

			// Set up content combo / pos
			ComboBox _orientation_combo;
			_orientation_combo.HorizontalAlignment(HorizontalAlignment::Right);
			_orientation_combo.VerticalAlignment(VerticalAlignment::Center);
			_orientation_combo.Height(65);
			_orientation_combo.MinWidth(350);
			_orientation_combo.FontSize(15);
			_orientation_combo.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());

			// Set up (content caption / pos)'s items
			ComboBoxItem _device, _software, _hmd, _off;
			_device.Content(box_value(k2app::interfacing::LocalizedResourceWString(
				L"SettingsPage", L"Filters/Orientation/Device")));
			_software.Content(box_value(k2app::interfacing::LocalizedResourceWString(
				L"SettingsPage", L"Filters/Orientation/MathBased")));
			_hmd.Content(box_value(k2app::interfacing::LocalizedResourceWString(
				L"SettingsPage", L"Filters/Orientation/HMD")));
			_off.Content(box_value(k2app::interfacing::LocalizedResourceWString(
				L"SettingsPage", L"Filters/Orientation/Off")));

			_software.Visibility(Visibility::Collapsed);

			_orientation_combo.Items().Append(_device);
			_orientation_combo.Items().Append(_software);
			_orientation_combo.Items().Append(_hmd);
			_orientation_combo.Items().Append(_off);

			_orientation_combo.SelectedIndex(0);

			_content.Children().Append(_orientation_combo);
			_content.SetColumn(_orientation_combo, 1);
			_content.SetRow(_orientation_combo, 1);

			// Create and set up the expander
			Expander _main_expander;
			_main_expander.ExpandDirection(ExpandDirection::Down);
			_main_expander.HorizontalAlignment(HorizontalAlignment::Stretch);
			_main_expander.VerticalAlignment(VerticalAlignment::Stretch);
			_main_expander.Margin({0, 10, 0, 0});

			_main_expander.Content(_content);
			_container.Children().Append(_main_expander);

			// Back everything up
			_ptr_container = std::make_shared<Grid>(_container);
			_ptr_main_grid = std::make_shared<Grid>(_main_grid);
			_ptr_content = std::make_shared<Grid>(_content);

			_ptr_joint_switch =
				std::make_shared<ToggleSwitch>(_joint_switch);

			_ptr_main_expander =
				std::make_shared<Expander>(_main_expander);

			_ptr_software_orientation =
				std::make_shared<ComboBoxItem>(_software);

			_ptr_title = std::make_shared<TextBlock>(_title);
			_ptr_position = std::make_shared<TextBlock>(_position);
			_ptr_orientation = std::make_shared<TextBlock>(_orientation);

			_ptr_position_combo = std::make_shared<ComboBox>(_position_combo);
			_ptr_orientation_combo = std::make_shared<ComboBox>(_orientation_combo);

			// Set up some signals
			_position_combo.SelectedIndex(_tracker_pointers[0]->positionTrackingFilterOption);
			_orientation_combo.SelectedIndex(_tracker_pointers[0]->orientationTrackingOption);

			_main_expander.Expanding(
				[this](const Expander& sender,
				       const ExpanderExpandingEventArgs& e) -> void
				{
					for (auto& expander : k2app::shared::settings::jointExpanderVector)
						if (expander->MainExpander().get() != nullptr &&
							expander->MainExpander().get() != _ptr_main_expander.get())
							expander->MainExpander().get()->IsExpanded(false);

					for (auto tracker_p : _tracker_pointers)
						tracker_p->data.isActive = _tracker_pointers[0]->data.isActive;

					if (!_tracker_pointers[0]->data.isActive)
					{
						sender.IsEnabled(false);
						sender.IsExpanded(false);
					}
					else
						sender.IsEnabled(true);
				});

			_joint_switch.Toggled(
				[this](const winrt::Windows::Foundation::IInspectable& sender,
				       const RoutedEventArgs& e) -> winrt::Windows::Foundation::IAsyncAction
				{
					// Don't react to pre-init signals
					if (!k2app::shared::settings::settings_localInitFinished)co_return;

					// Make actual changes
					for (auto tracker_p : _tracker_pointers)
					{
						tracker_p->data.isActive = _ptr_joint_switch.get()->IsOn();

						// Do that on UI's background
						apartment_context _ui_thread;
						co_await resume_background();

						// Spawn the tracker (only if the rest is spawned)
						if (k2app::interfacing::K2AppTrackersInitialized)
						{
							// try 3 times cause why not
							for (int i = 0; i < 3; i++)
							{
								// Update status in server
								ktvr::set_tracker_state<false>(
									tracker_p->tracker, tracker_p->data.isActive);
								Sleep(20); // Wait a bit
							}
						}
						co_await _ui_thread;
					}

					if (!_tracker_pointers[0]->data.isActive)
					{
						_ptr_main_expander->IsEnabled(false);
						_ptr_main_expander->IsExpanded(false);
					}
					else
						_ptr_main_expander->IsEnabled(true);

					// Check if we've disabled any joints from spawning and disable their mods
					k2app::interfacing::devices_check_disabled_joints();
					TrackingDevices::settings_trackersConfigChanged();

					// Save settings
					k2app::K2Settings.saveSettings();

					// Request a check for already-added trackers
					LOG(INFO) << "Requesting a check for already-added trackers...";
					k2app::interfacing::alreadyAddedTrackersScanRequested = true;
				});

			_ptr_position_combo->SelectionChanged(
				[this](const winrt::Windows::Foundation::IInspectable& sender,
				       const SelectionChangedEventArgs& e) -> void
				{
					// Don't react to pre-init signals
					if (!k2app::shared::settings::settings_localInitFinished)return;

					for (auto tracker_p : _tracker_pointers)
						tracker_p->positionTrackingFilterOption =
							static_cast<k2app::JointPositionTrackingOption>(_ptr_position_combo.get()->SelectedIndex());

					// Save settings
					k2app::K2Settings.saveSettings();
				});

			_ptr_orientation_combo->SelectionChanged(
				[this](const winrt::Windows::Foundation::IInspectable& sender,
				       const SelectionChangedEventArgs& e) -> void
				{
					// Don't react to pre-init signals
					if (!k2app::shared::settings::settings_localInitFinished)return;

					for (auto tracker_p : _tracker_pointers)
						tracker_p->orientationTrackingOption =
							static_cast<k2app::JointRotationTrackingOption>(_ptr_orientation_combo.get()->
								SelectedIndex());

					// Save settings
					k2app::K2Settings.saveSettings();
				});

			if (_tracker_pointers[0]->tracker == ktvr::ITrackerType::Tracker_LeftFoot ||
				_tracker_pointers[0]->tracker == ktvr::ITrackerType::Tracker_RightFoot)
				_ptr_software_orientation.get()->Visibility(Visibility::Visible);
		}
	};
}
