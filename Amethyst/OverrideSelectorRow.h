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
	// devices_clear_combo
	inline void ClearComboBox_Safe(const std::shared_ptr<ComboBox>& cbox)
	{
		[&]
		{
			__try
			{
				[&]
				{
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

	// devices_push_combobox
	inline void PushComboBox_Safe(
		const std::shared_ptr<ComboBox>& cbox,
		const hstring& str)
	{
		[&]
		{
			__try
			{
				[&]
				{
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
			                              std::to_wstring(static_cast<int>(ktvr::ITrackerType::Tracker_Chest))),
		                              L" Tracker").c_str());

		if (all.value_or(true))
		{
			PushComboBox_Safe(cbox,
			                  eraseSubStr(k2app::interfacing::LocalizedResourceWString(
				                              L"SharedStrings", L"Joints/Enum/" +
				                              std::to_wstring(static_cast<int>(ktvr::ITrackerType::Tracker_LeftElbow))),
			                              L" Tracker").c_str());
			PushComboBox_Safe(cbox,
			                  eraseSubStr(k2app::interfacing::LocalizedResourceWString(
				                              L"SharedStrings", L"Joints/Enum/" +
				                              std::to_wstring(
					                              static_cast<int>(ktvr::ITrackerType::Tracker_RightElbow))),
			                              L" Tracker").c_str());
		}

		PushComboBox_Safe(cbox,
		                  eraseSubStr(k2app::interfacing::LocalizedResourceWString(
			                              L"SharedStrings", L"Joints/Enum/" +
			                              std::to_wstring(static_cast<int>(ktvr::ITrackerType::Tracker_Waist))),
		                              L" Tracker").c_str());

		if (all.value_or(true))
		{
			PushComboBox_Safe(cbox,
			                  eraseSubStr(k2app::interfacing::LocalizedResourceWString(
				                              L"SharedStrings", L"Joints/Enum/" +
				                              std::to_wstring(static_cast<int>(ktvr::ITrackerType::Tracker_LeftKnee))),
			                              L" Tracker").c_str());
			PushComboBox_Safe(cbox,
			                  eraseSubStr(k2app::interfacing::LocalizedResourceWString(
				                              L"SharedStrings", L"Joints/Enum/" +
				                              std::to_wstring(static_cast<int>(ktvr::ITrackerType::Tracker_RightKnee))),
			                              L" Tracker").c_str());
		}

		PushComboBox_Safe(cbox,
		                  eraseSubStr(k2app::interfacing::LocalizedResourceWString(
			                              L"SharedStrings", L"Joints/Enum/" +
			                              std::to_wstring(static_cast<int>(ktvr::ITrackerType::Tracker_LeftFoot))),
		                              L" Tracker").c_str());
		PushComboBox_Safe(cbox,
		                  eraseSubStr(k2app::interfacing::LocalizedResourceWString(
			                              L"SharedStrings", L"Joints/Enum/" +
			                              std::to_wstring(static_cast<int>(ktvr::ITrackerType::Tracker_RightFoot))),
		                              L" Tracker").c_str());
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
			Helpers::ClearComboBox_Safe(_ptr_tracker_position_combo);
			Helpers::ClearComboBox_Safe(_ptr_tracker_orientation_combo);
		}

		// devices_select_combobox
		void SelectOverrideJoints()
		{
			Helpers::SelectComboBoxItem_Safe(
				_ptr_tracker_position_combo,
				_tracker_pointer->isPositionOverridden
					? _tracker_pointer->positionOverrideJointID
					: -1);
			Helpers::SelectComboBoxItem_Safe(
				_ptr_tracker_orientation_combo,
				_tracker_pointer->isRotationOverridden
					? _tracker_pointer->rotationOverrideJointID
					: -1);
		}

		void UpdateOverrideToggles()
		{
			_ptr_override_position.get()->IsChecked(_tracker_pointer->isPositionOverridden);
			_ptr_override_orientation.get()->IsChecked(_tracker_pointer->isRotationOverridden);
		}

		// devices_push_override_joints
		void UpdateOverrideJoints(const std::optional<bool>& all = std::nullopt)
		{
			Helpers::PushComboBoxJoints_Safe(_ptr_tracker_position_combo, all);
			Helpers::PushComboBoxJoints_Safe(_ptr_tracker_orientation_combo, all);
		}

		// devices_push_override_joints
		void UpdateOverrideJoints(const std::wstring& _string)
		{
			Helpers::PushComboBox_Safe(_ptr_tracker_position_combo, _string.c_str());
			Helpers::PushComboBox_Safe(_ptr_tracker_orientation_combo, _string.c_str());
		}

		std::shared_ptr<Grid> Container() { return _ptr_container; }
		std::shared_ptr<Grid> TitleContainer() { return _ptr_title_container; }

		std::shared_ptr<TextBlock> Title() { return _ptr_title; }

		std::shared_ptr<AppBarButton> Selector() { return _ptr_selector; }

		std::shared_ptr<MenuFlyout> SelectorFlyout() { return _ptr_selector_flyout; }

		std::shared_ptr<ToggleMenuFlyoutItem> OverridePosition() { return _ptr_override_position; }
		std::shared_ptr<ToggleMenuFlyoutItem> OverrideOrientation() { return _ptr_override_orientation; }

		std::shared_ptr<ComboBox> TrackerPositionCombo() { return _ptr_tracker_position_combo; }
		std::shared_ptr<ComboBox> TrackerOrientationCombo() { return _ptr_tracker_orientation_combo; }

		k2app::K2AppTracker* Tracker() { return _tracker_pointer; }

	protected:
		k2app::K2AppTracker* _tracker_pointer;

		// Underlying object shared pointer
		std::shared_ptr<Grid> _ptr_container, _ptr_title_container;

		std::shared_ptr<TextBlock> _ptr_title;

		std::shared_ptr<AppBarButton> _ptr_selector;

		std::shared_ptr<MenuFlyout> _ptr_selector_flyout;

		std::shared_ptr<ToggleMenuFlyoutItem> _ptr_override_position,
		                                      _ptr_override_orientation;

		std::shared_ptr<ComboBox> _ptr_tracker_position_combo,
		                          _ptr_tracker_orientation_combo;

		// Creation: register a host and a callback
		void Create()
		{
			// Create the container grid
			Grid _container;

			AppendGridStarsColumnMinWidthPixels(_container, 3, 80);
			AppendGridStarColumn(_container);
			AppendGridStarsColumnMinWidthPixels(_container, 3, 140);
			AppendGridStarColumn(_container);
			AppendGridStarsColumnMinWidthPixels(_container, 3, 140);

			AppendGridPixelsRow(_container, 50);

			// Create the title text & toggle
			TextBlock _title;
			_title.FontWeight(Windows::UI::Text::FontWeights::SemiBold());

			_title.Text(eraseSubStr(k2app::interfacing::LocalizedResourceWString(
				                        L"SharedStrings", L"Joints/Enum/" +
				                        std::to_wstring(static_cast<int>(_tracker_pointer->tracker))),
			                        L" Tracker"));

			_title.FontSize(14);
			_title.Margin({0, -3, 0, 0});
			_title.HorizontalAlignment(HorizontalAlignment::Center);
			_title.VerticalAlignment(VerticalAlignment::Center);
			_title.HorizontalTextAlignment(TextAlignment::Center);

			AppBarButton _selector;
			_selector.HorizontalAlignment(HorizontalAlignment::Right);
			_selector.VerticalAlignment(VerticalAlignment::Center);
			_selector.Width(35);
			_selector.Height(48);

			MenuFlyout _selector_flyout;
			ToggleMenuFlyoutItem _override_position, _override_orientation;

			_override_position.Text(k2app::interfacing::LocalizedResourceWString(
				L"DevicesPage", L"Titles/Check/Position/Text"));
			_override_orientation.Text(k2app::interfacing::LocalizedResourceWString(
				L"DevicesPage", L"Titles/Check/Orientation/Text"));

			_selector_flyout.Items().Append(_override_position);
			_selector_flyout.Items().Append(_override_orientation);

			_selector.Flyout(_selector_flyout);

			Grid _title_container;
			_title_container.Width(130);
			_title_container.HorizontalAlignment(HorizontalAlignment::Center);
			_title_container.VerticalAlignment(VerticalAlignment::Center);

			AppendGridStarsColumn(_title_container, 2);
			AppendGridStarColumn(_title_container);

			_title_container.Children().Append(_title);
			_title_container.Children().Append(_selector);

			_title_container.SetColumn(_title, 0);
			_title_container.SetColumn(_selector, 1);

			// Push the toggle to the main container
			_container.Children().Append(_title_container);
			_container.SetColumn(_title_container, 0);

			// Set up content combo
			ComboBox _tracker_position_combo, _tracker_orientation_combo;
			_tracker_position_combo.HorizontalAlignment(HorizontalAlignment::Center);
			_tracker_position_combo.VerticalAlignment(VerticalAlignment::Center);
			_tracker_position_combo.Height(45);
			_tracker_position_combo.Width(140);
			_tracker_position_combo.FontSize(15);
			_tracker_position_combo.FontWeight(Windows::UI::Text::FontWeights::SemiBold());
			_tracker_position_combo.PlaceholderText(k2app::interfacing::LocalizedResourceWString(
				L"DevicesPage", L"Placeholders/Overrides/NoOverride/PlaceholderText"));

			_tracker_orientation_combo.HorizontalAlignment(HorizontalAlignment::Center);
			_tracker_orientation_combo.VerticalAlignment(VerticalAlignment::Center);
			_tracker_orientation_combo.Height(45);
			_tracker_orientation_combo.Width(140);
			_tracker_orientation_combo.FontSize(15);
			_tracker_orientation_combo.FontWeight(Windows::UI::Text::FontWeights::SemiBold());
			_tracker_orientation_combo.PlaceholderText(k2app::interfacing::LocalizedResourceWString(
				L"DevicesPage", L"Placeholders/Overrides/NoOverride/PlaceholderText"));

			// Append the selectors to the main container
			_container.Children().Append(_tracker_position_combo);
			_container.Children().Append(_tracker_orientation_combo);

			_container.SetColumn(_tracker_position_combo, 2);
			_container.SetRow(_tracker_position_combo, 0);

			_container.SetColumn(_tracker_orientation_combo, 4);
			_container.SetRow(_tracker_orientation_combo, 0);

			// Back everything up
			_ptr_container = std::make_shared<Grid>(_container);
			_ptr_title_container = std::make_shared<Grid>(_title_container);

			_ptr_title = std::make_shared<TextBlock>(_title);

			_ptr_selector = std::make_shared<AppBarButton>(_selector);

			_ptr_selector_flyout = std::make_shared<MenuFlyout>(_selector_flyout);

			_ptr_override_position = std::make_shared<ToggleMenuFlyoutItem>(_override_position);
			_ptr_override_orientation = std::make_shared<ToggleMenuFlyoutItem>(_override_orientation);

			_ptr_tracker_position_combo = std::make_shared<ComboBox>(_tracker_position_combo);
			_ptr_tracker_orientation_combo = std::make_shared<ComboBox>(_tracker_orientation_combo);

			// Set up some signals
			_ptr_tracker_position_combo->SelectionChanged(
				[this](const Windows::Foundation::IInspectable& sender,
				       const SelectionChangedEventArgs& e) -> void
				{
					if (!k2app::shared::devices::devices_tab_setup_finished)return;
					// Don't even try if we're not set up yet
					if (_tracker_pointer->isPositionOverridden &&
						_ptr_tracker_position_combo.get()->SelectedIndex() >= 0)
						_tracker_pointer->positionOverrideJointID = _ptr_tracker_position_combo.get()->SelectedIndex();

					// If we're using a joints device then also signal the joint
					const auto& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
					if (trackingDevicePair.first && k2app::shared::devices::devices_signal_joints)
						if (trackingDevicePair.second.index() == 1 &&
							k2app::shared::devices::devices_tab_re_setup_finished) // if JointsBasis & Setup Finished
							std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
								signalJoint(_tracker_pointer->positionOverrideJointID);

					// Save settings
					k2app::K2Settings.saveSettings();
				});

			_ptr_tracker_orientation_combo->SelectionChanged(
				[this](const Windows::Foundation::IInspectable& sender,
				       const SelectionChangedEventArgs& e) -> void
				{
					if (!k2app::shared::devices::devices_tab_setup_finished)return;
					// Don't even try if we're not set up yet
					if (_tracker_pointer->isRotationOverridden &&
						_ptr_tracker_orientation_combo.get()->SelectedIndex() >= 0)
						_tracker_pointer->rotationOverrideJointID = _ptr_tracker_orientation_combo.get()->
							SelectedIndex();

					// If we're using a joints device then also signal the joint
					const auto& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
					if (trackingDevicePair.first && k2app::shared::devices::devices_signal_joints)
						if (trackingDevicePair.second.index() == 1 &&
							k2app::shared::devices::devices_tab_re_setup_finished) // if JointsBasis & Setup Finished
							std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
								signalJoint(_tracker_pointer->rotationOverrideJointID);

					// Save settings
					k2app::K2Settings.saveSettings();
				});

			_ptr_override_position->Click(
				[this](const Windows::Foundation::IInspectable& sender,
				       const RoutedEventArgs& e) -> void
				{
					if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
						std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
							TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
					{
						_ptr_override_position.get()->IsChecked(false);
						k2app::shared::devices::noJointsFlyout.get()->ShowAt(*k2app::shared::devices::overridesLabel);
						return; // Don't set up any overrides (yet)
					}

					if (!k2app::shared::devices::devices_tab_setup_finished)return;
					// Don't even try if we're not set up yet
					_tracker_pointer->isPositionOverridden = _ptr_override_position.get()->IsChecked();

					// If we've disabled the override, set the placeholder text
					Helpers::SelectComboBoxItem_Safe(_ptr_tracker_position_combo,
					                                 _ptr_override_position.get()->IsChecked()
						                                 ? _tracker_pointer->positionOverrideJointID
						                                 : -1);

					// Check for errors and disable combos
					k2app::interfacing::devices_check_disabled_joints();

					// Save settings
					k2app::K2Settings.saveSettings();
				});

			_ptr_override_orientation->Click(
				[this](const Windows::Foundation::IInspectable& sender,
				       const RoutedEventArgs& e) -> void
				{
					if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
						std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
							TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
					{
						_ptr_override_orientation.get()->IsChecked(false);
						k2app::shared::devices::noJointsFlyout.get()->ShowAt(*k2app::shared::devices::overridesLabel);
						return; // Don't set up any overrides (yet)
					}

					if (!k2app::shared::devices::devices_tab_setup_finished)return;
					// Don't even try if we're not set up yet
					_tracker_pointer->isRotationOverridden = _ptr_override_orientation.get()->IsChecked();

					// If we've disabled the override, set the placeholder text
					Helpers::SelectComboBoxItem_Safe(_ptr_tracker_orientation_combo,
					                                 _ptr_override_orientation.get()->IsChecked()
						                                 ? _tracker_pointer->rotationOverrideJointID
						                                 : -1);

					// Check for errors and disable combos
					k2app::interfacing::devices_check_disabled_joints();

					// Save settings
					k2app::K2Settings.saveSettings();
				});
		}
	};
}
