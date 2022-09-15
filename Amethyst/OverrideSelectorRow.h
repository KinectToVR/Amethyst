#pragma once
#include "pch.h"

#include "K2Interfacing.h"
#include "K2Shared.h"
#include "JointExpander.h"

// Forward declaration
namespace winrt::Microsoft::UI::Xaml::Controls
{
	class OverrideSelectorRow;
}

// Defined in JointSelectorRow.h
inline std::wstring eraseSubStr(std::wstring mainStr, const std::wstring& toErase);

namespace winrt::Microsoft::UI::Xaml::Controls::Helpers
{
	// devices_push_combobox
	inline void PushComboBox_Safe(
		const std::shared_ptr<ComboBox>& cbox,
		const hstring& str, const bool& secondary = false)
	{
		[&]
		{
			__try
			{
				[&]
				{
					// Split to save resources
					if (secondary)
					{
						// Create a new combobox item
						const auto& item = ComboBoxItem();
						// Set the item's content
						item.Content(box_value(str));

						// Optionally dim
						item.Opacity(0.7);

						// Push the item to the combobox
						cbox.get()->Items().Append(box_value(item));
					}
					else
						// Push the item to the combobox
						cbox.get()->Items().Append(box_value(str));
				}();
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				[&]
				{
					LOG(WARNING) << "Couldn't push to a ComboBox. You better call an exorcist.";
				}();
			}
		}();
	}

	// devices_push_override_joints_combo
	inline void PushComboBoxJoints_Safe(
		const std::shared_ptr<ComboBox>& cbox,
		const std::optional<bool>& all = std::nullopt)
	{
		PushComboBox_Safe(cbox,
		                  eraseSubStr(k2app::interfacing::LocalizedResourceWString(
			                              L"SharedStrings", L"Joints/Enum/" +
			                              std::to_wstring(ktvr::ITrackerType::Tracker_Chest)),
		                              L" Tracker").c_str());

		if (all.value_or(true))
		{
			PushComboBox_Safe(cbox,
			                  eraseSubStr(k2app::interfacing::LocalizedResourceWString(
				                              L"SharedStrings", L"Joints/Enum/" +
				                              std::to_wstring(ktvr::ITrackerType::Tracker_LeftElbow)),
			                              L" Tracker").c_str());
			PushComboBox_Safe(cbox,
			                  eraseSubStr(k2app::interfacing::LocalizedResourceWString(
				                              L"SharedStrings", L"Joints/Enum/" +
				                              std::to_wstring(
					                              ktvr::ITrackerType::Tracker_RightElbow)),
			                              L" Tracker").c_str());
		}

		PushComboBox_Safe(cbox,
		                  eraseSubStr(k2app::interfacing::LocalizedResourceWString(
			                              L"SharedStrings", L"Joints/Enum/" +
			                              std::to_wstring(ktvr::ITrackerType::Tracker_Waist)),
		                              L" Tracker").c_str());

		if (all.value_or(true))
		{
			PushComboBox_Safe(cbox,
			                  eraseSubStr(k2app::interfacing::LocalizedResourceWString(
				                              L"SharedStrings", L"Joints/Enum/" +
				                              std::to_wstring(ktvr::ITrackerType::Tracker_LeftKnee)),
			                              L" Tracker").c_str());
			PushComboBox_Safe(cbox,
			                  eraseSubStr(k2app::interfacing::LocalizedResourceWString(
				                              L"SharedStrings", L"Joints/Enum/" +
				                              std::to_wstring(ktvr::ITrackerType::Tracker_RightKnee)),
			                              L" Tracker").c_str());
		}

		PushComboBox_Safe(cbox,
		                  eraseSubStr(k2app::interfacing::LocalizedResourceWString(
			                              L"SharedStrings", L"Joints/Enum/" +
			                              std::to_wstring(ktvr::ITrackerType::Tracker_LeftFoot)),
		                              L" Tracker").c_str());
		PushComboBox_Safe(cbox,
		                  eraseSubStr(k2app::interfacing::LocalizedResourceWString(
			                              L"SharedStrings", L"Joints/Enum/" +
			                              std::to_wstring(ktvr::ITrackerType::Tracker_RightFoot)),
		                              L" Tracker").c_str());
	}

	// devices_clear_combo
	inline void ClearComboBox_Safe(const std::shared_ptr<ComboBox>& cbox)
	{
		[&]
		{
			__try
			{
				[&]
				{
					// Clear the combobox
					cbox.get()->Items().Clear();
				}();
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				[&]
				{
					LOG(WARNING) << "Couldn't clear a ComboBox. You better call an exorcist.";
				}();
			}
		}();
	}

	// devices_select_combobox_safe
	inline void SelectComboBoxItem_Safe(
		const std::shared_ptr<ComboBox>& cbox,
		const int& index)
	{
		[&]
		{
			__try
			{
				cbox.get()->SelectedIndex(index);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				[&]
				{
					LOG(WARNING) << "Couldn't select a ComboBox index. You better call an exorcist.";
				}();
			}
		}();
	}

	inline void SetComboBoxIsEnabled_Safe(
		const std::shared_ptr<ComboBox>& cbox,
		const bool& enabled)
	{
		[&]
		{
			__try
			{
				cbox.get()->IsEnabled(enabled);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				[&]
				{
					LOG(WARNING) << "Couldn't enable/disable a ComboBox. You better call an exorcist.";
				}();
			}
		}();
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

	class OverrideSelectorRow
	{
	public:
		// Note: Signals have to be set up manually
		OverrideSelectorRow(k2app::K2AppTracker* tracker_pointer)
		{
			_tracker_pointer = tracker_pointer;
			Create();
		}

		// devices_clear_combo
		void ClearOverrideCombos()
		{
			Helpers::ClearComboBox_Safe(_ptr_tracker_combo);
		}

		// devices_select_combobox
		void SelectOverrideJoints()
		{
			Helpers::SelectComboBoxItem_Safe(
				_ptr_tracker_combo,
				_tracker_pointer->data_isActive
					? (_tracker_pointer->isPositionOverridden ||
					   _tracker_pointer->isRotationOverridden
						   ? _tracker_pointer->positionOverrideJointID + 1 // Omit "No Override"
						   : 0) // Display "No Override"
					: -1); // Display "Joint Disabled"
		}

		void UpdateOverrideToggles()
		{
			_ptr_override_position_switch.get()->IsOn(_tracker_pointer->isPositionOverridden);
			_ptr_override_orientation_switch.get()->IsOn(_tracker_pointer->isRotationOverridden);
		}

		// devices_push_override_joints
		void UpdateOverrideJoints(const std::optional<bool>& all = std::nullopt)
		{
			Helpers::PushComboBoxJoints_Safe(_ptr_tracker_combo, all);
		}

		// devices_push_override_joints
		void UpdateOverrideJoints(const std::wstring& _string, const bool& secondary = false)
		{
			Helpers::PushComboBox_Safe(_ptr_tracker_combo, _string.c_str(), secondary);
		}

		std::shared_ptr<Grid> Container() { return _ptr_container; }
		std::shared_ptr<Grid> TitleContainer() { return _ptr_title_container; }

		std::shared_ptr<TextBlock> Title() { return _ptr_title; }

		std::shared_ptr<ToggleSwitch> OverridePositionSwitch() { return _ptr_override_position_switch; }
		std::shared_ptr<ToggleSwitch> OverrideOrientationSwitch() { return _ptr_override_orientation_switch; }

		std::shared_ptr<ComboBox> TrackerCombo() { return _ptr_tracker_combo; }

		k2app::K2AppTracker* Tracker() { return _tracker_pointer; }

	protected:
		k2app::K2AppTracker* _tracker_pointer;

		// Underlying object shared pointer
		std::shared_ptr<Grid> _ptr_container, _ptr_title_container;

		std::shared_ptr<TextBlock> _ptr_title;

		std::shared_ptr<ToggleSwitch> _ptr_override_position_switch,
		                              _ptr_override_orientation_switch;

		std::shared_ptr<ComboBox> _ptr_tracker_combo;

		// Creation: register a host and a callback
		void Create()
		{
			// Create the container grid
			Grid _container;

			AppendGridStarsColumnMinWidthPixels(_container, 3, 80);
			AppendGridStarColumn(_container);
			AppendGridAutoColumnMinWidthPixels(_container, 140);
			AppendGridStarColumn(_container);
			AppendGridStarsColumnMinWidthPixels(_container, 3, 80);
			AppendGridStarColumn(_container);
			AppendGridStarsColumnMinWidthPixels(_container, 3, 80);

			AppendGridPixelsRow(_container, 50);

			// Create the title text
			TextBlock _title;
			_title.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());

			_title.Text(eraseSubStr(k2app::interfacing::LocalizedResourceWString(
				                        L"SharedStrings", L"Joints/Enum/" +
				                        std::to_wstring(_tracker_pointer->base_tracker)),
			                        L" Tracker"));

			_title.FontSize(14);
			_title.Margin({0, -3, 0, 0});
			_title.HorizontalAlignment(HorizontalAlignment::Center);
			_title.VerticalAlignment(VerticalAlignment::Center);
			_title.HorizontalTextAlignment(TextAlignment::Center);

			// Push the title to the main container
			_container.Children().Append(_title);
			_container.SetColumn(_title, 0);

			// Set up content combo
			ComboBox _tracker_combo;
			_tracker_combo.HorizontalAlignment(HorizontalAlignment::Center);
			_tracker_combo.VerticalAlignment(VerticalAlignment::Center);
			_tracker_combo.Height(45);
			_tracker_combo.MinWidth(150);
			_tracker_combo.FontSize(15);
			_tracker_combo.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
			_tracker_combo.PlaceholderText(k2app::interfacing::LocalizedResourceWString(
				L"DevicesPage", L"Placeholders/Overrides/NoOverride/PlaceholderText"));

			// Append the selector to the main container
			_container.Children().Append(_tracker_combo);

			_container.SetColumn(_tracker_combo, 2);
			_container.SetRow(_tracker_combo, 0);

			// Set up toggles
			ToggleSwitch _override_position_switch,
			             _override_orientation_switch;

			// : Position toggle
			_override_position_switch.HorizontalAlignment(HorizontalAlignment::Center);
			_override_position_switch.VerticalAlignment(VerticalAlignment::Center);

			_override_position_switch.Margin({.Right = -107});

			_override_position_switch.OnContent(box_value(L""));
			_override_position_switch.OffContent(box_value(L""));

			// : Orientation toggle
			_override_orientation_switch.HorizontalAlignment(HorizontalAlignment::Center);
			_override_orientation_switch.VerticalAlignment(VerticalAlignment::Center);

			_override_orientation_switch.Margin({.Right = -112});

			_override_orientation_switch.OnContent(box_value(L""));
			_override_orientation_switch.OffContent(box_value(L""));

			// Append the toggles to the main container
			_container.Children().Append(_override_position_switch);
			_container.Children().Append(_override_orientation_switch);

			_container.SetColumn(_override_position_switch, 4);
			_container.SetRow(_override_position_switch, 0);
			_container.SetColumn(_override_orientation_switch, 6);
			_container.SetRow(_override_orientation_switch, 0);

			// Back everything up
			_ptr_container = std::make_shared<Grid>(_container);
			_ptr_title = std::make_shared<TextBlock>(_title);

			_ptr_override_position_switch = std::make_shared<ToggleSwitch>(_override_position_switch);
			_ptr_override_orientation_switch = std::make_shared<ToggleSwitch>(_override_orientation_switch);

			_ptr_tracker_combo = std::make_shared<ComboBox>(_tracker_combo);

			// Set up some signals
			_ptr_tracker_combo->SelectionChanged(
				[this](const winrt::Windows::Foundation::IInspectable& sender,
				       const SelectionChangedEventArgs& e) -> void
				{
					// Don't even try if we're not set up yet
					if (!k2app::shared::devices::devices_tab_setup_finished ||
						k2app::shared::devices::devices_overrides_setup_pending)
						return;
					
					// If the selected joint is valid
					if (_ptr_tracker_combo.get()->SelectedIndex() > 0)
					{
						_tracker_pointer->positionOverrideJointID =
							_ptr_tracker_combo.get()->SelectedIndex() - 1; // minus the "off" item

						_tracker_pointer->rotationOverrideJointID =
							_ptr_tracker_combo.get()->SelectedIndex() - 1; // minus the "off" item

						// If we've disabled the both overrides, re-enable them
						if (!_ptr_override_position_switch.get()->IsOn() &&
							!_ptr_override_orientation_switch.get()->IsOn())
						{
							// Toggle the switches on
							OverridePositionSwitch()->IsOn(true);
							OverrideOrientationSwitch()->IsOn(true);

							// Enable the override switches
							OverridePositionSwitch()->IsEnabled(true);
							OverrideOrientationSwitch()->IsEnabled(true);
						}

						// If we're using a joints device then also signal the joint
						const auto& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
						if (trackingDevicePair.first && k2app::shared::devices::devices_signal_joints)
							if (trackingDevicePair.second.index() == 1 &&
								k2app::shared::devices::devices_tab_re_setup_finished)
								// if JointsBasis & Setup Finished
								std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
									signalJoint(_tracker_pointer->positionOverrideJointID);
					}
					// If the selection is something else (manual)
					else
					{
						// Disable the override
						_ptr_tracker_combo.get()->SelectedIndex(0);

						// Disable the override, should auto-handle
						OverridePositionSwitch()->IsOn(false);
						OverrideOrientationSwitch()->IsOn(false);
					}

					// Save settings
					k2app::K2Settings.saveSettings();
				});

			_ptr_tracker_combo->DropDownOpened([&](auto, auto)
			{
				// Play a sound
				playAppSound(k2app::interfacing::sounds::AppSounds::Show);
			});

			_ptr_tracker_combo->DropDownClosed([&](auto, auto)
			{
				// Play a sound
				playAppSound(k2app::interfacing::sounds::AppSounds::Hide);
			});

			_ptr_override_position_switch->Toggled(
				[this](const winrt::Windows::Foundation::IInspectable& sender,
				       const RoutedEventArgs& e) -> void
				{
					// Don't even try if we're not set up yet
					if (!k2app::shared::devices::devices_tab_setup_finished ||
						k2app::shared::devices::devices_overrides_setup_pending)
						return;
					
					if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
						std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
							TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
					{
						_ptr_override_position_switch.get()->IsOn(false);
						k2app::shared::devices::noJointsFlyout.get()->ShowAt(*k2app::shared::devices::overridesLabel);
						return; // Don't set up any overrides (yet)
					}

					// Play a sound
					playAppSound(_ptr_override_position_switch.get()->IsOn()
						             ? k2app::interfacing::sounds::AppSounds::ToggleOn
						             : k2app::interfacing::sounds::AppSounds::ToggleOff);

					// Change the config
					_tracker_pointer->isPositionOverridden = _ptr_override_position_switch.get()->IsOn();

					// If we've disabled the both overrides, show the "Disabled" text
					if (!_ptr_override_position_switch.get()->IsOn() &&
						!_ptr_override_orientation_switch.get()->IsOn())
						Helpers::SelectComboBoxItem_Safe(_ptr_tracker_combo, 0);

					// Select the first valid item when turning the override on
					if (_ptr_tracker_combo.get()->SelectedIndex() <= 0 &&
						_ptr_override_position_switch.get()->IsOn())
						Helpers::SelectComboBoxItem_Safe(_ptr_tracker_combo, 1);

					// Check for errors and disable combos
					k2app::interfacing::devices_check_disabled_joints();

					// Save settings
					k2app::K2Settings.saveSettings();
				});

			_ptr_override_orientation_switch->Toggled(
				[this](const winrt::Windows::Foundation::IInspectable& sender,
				       const RoutedEventArgs& e) -> void
				{
					// Don't even try if we're not set up yet
					if (!k2app::shared::devices::devices_tab_setup_finished ||
						k2app::shared::devices::devices_overrides_setup_pending)
						return;
					
					if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
						std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
							TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
					{
						_ptr_override_orientation_switch.get()->IsOn(false);
						k2app::shared::devices::noJointsFlyout.get()->ShowAt(*k2app::shared::devices::overridesLabel);
						return; // Don't set up any overrides (yet)
					}

					// Play a sound
					playAppSound(_ptr_override_orientation_switch.get()->IsOn()
						             ? k2app::interfacing::sounds::AppSounds::ToggleOn
						             : k2app::interfacing::sounds::AppSounds::ToggleOff);

					// Change the config
					_tracker_pointer->isRotationOverridden = _ptr_override_orientation_switch.get()->IsOn();

					// If we've disabled the both overrides, show the "Disabled" text
					if (!_ptr_override_position_switch.get()->IsOn() &&
						!_ptr_override_orientation_switch.get()->IsOn())
						Helpers::SelectComboBoxItem_Safe(_ptr_tracker_combo, 0);

					// Select the first valid item when turning the override on
					if (_ptr_tracker_combo.get()->SelectedIndex() <= 0 &&
						_ptr_override_orientation_switch.get()->IsOn())
						Helpers::SelectComboBoxItem_Safe(_ptr_tracker_combo, 1);

					// Check for errors and disable combos
					k2app::interfacing::devices_check_disabled_joints();

					// Save settings
					k2app::K2Settings.saveSettings();
				});
		}
	};
}
