#pragma once
#include "pch.h"

#include "K2Interfacing.h"
#include "K2Shared.h"

#include "JointExpander.h"
#include "OverrideSelectorRow.h"

#include "JointSelectorRow.h"
#include "JointSelectorExpander.h"

#define __OVERRIDES_BEGIN_CHANGES k2app::shared::devices::devices_overrides_setup_pending = true;
#define __OVERRIDES_END_CHANGES k2app::shared::devices::devices_overrides_setup_pending = false;

// Forward declaration
namespace winrt::Microsoft::UI::Xaml::Controls
{
	class OverrideSelectorExpander;
}

// Extension of the k2/shared namespace
namespace k2app::shared::devices
{
	inline std::array<std::shared_ptr<winrt::Microsoft::UI::Xaml::Controls::OverrideSelectorExpander>, 3>
	overrideSelectorExpanders;
}

namespace winrt::Microsoft::UI::Xaml::Controls
{
	/*
	 * Joint expander that allows to :
	 *  - select the position filter
	 *	- select the ori tracking option
	 *	- turn joint/joint pair on/off
	 */

	class OverrideSelectorExpander
	{
	public:
		// Note: Signals have to be set up manually
		OverrideSelectorExpander(const uint32_t& type)
		{
			_tracker_pointers = GetTrackerPointerSpan(type);
			_type = type;
			Create(type);
		}

		std::vector<k2app::K2AppTracker*>* TrackerPointers()
		{
			return &_tracker_pointers;
		}

		void EraseComboItems()
		{
			__OVERRIDES_BEGIN_CHANGES

			for (const auto& _row : _overrideSelectorRows)
				_row.get()->ClearOverrideCombos();

			__OVERRIDES_END_CHANGES
		}

		void SelectComboItems()
		{
			__OVERRIDES_BEGIN_CHANGES

			for (const auto& _row : _overrideSelectorRows)
				_row.get()->SelectOverrideJoints();

			__OVERRIDES_END_CHANGES
		}

		void PushOverrideJoints(const std::optional<bool>& all = std::nullopt)
		{
			__OVERRIDES_BEGIN_CHANGES

			for (const auto& _row : _overrideSelectorRows)
				_row.get()->UpdateOverrideJoints(all);

			__OVERRIDES_END_CHANGES
		}

		void PushOverrideJoint(const std::wstring& _string, const bool& secondary = false)
		{
			__OVERRIDES_BEGIN_CHANGES

			for (const auto& _row : _overrideSelectorRows)
				_row.get()->UpdateOverrideJoints(_string, secondary);

			__OVERRIDES_END_CHANGES
		}

		void UpdateOverrideToggles()
		{
			__OVERRIDES_BEGIN_CHANGES

			for (const auto& _row : _overrideSelectorRows)
				_row.get()->UpdateOverrideToggles();

			__OVERRIDES_END_CHANGES
		}

		void UpdateIsEnabled()
		{
			__OVERRIDES_BEGIN_CHANGES

			for (const auto& _row : _overrideSelectorRows)
			{
				_row.get()->TrackerCombo().get()->IsEnabled(
					_row.get()->Tracker()->data_isActive); // Is tracker ON?

				_row.get()->OverridePositionSwitch().get()->IsEnabled(
					_row.get()->Tracker()->data_isActive); // Is tracker ON?

				_row.get()->OverrideOrientationSwitch().get()->IsEnabled(
					_row.get()->Tracker()->data_isActive); // Is tracker ON?

				// Change the placeholder to 'Joint Disabled'
				_row.get()->TrackerCombo().get()->PlaceholderText(
					k2app::interfacing::LocalizedResourceWString(
						L"DevicesPage", L"Placeholders/Joints/Disabled/PlaceholderText"));

				// Optionally show the placeholder
				if (!_row.get()->Tracker()->data_isActive)
				{
					_row.get()->OverridePositionSwitch().get()->IsOn(false);
					_row.get()->OverrideOrientationSwitch().get()->IsOn(false);

					// Show the placeholder
					_row.get()->TrackerCombo().get()->SelectedIndex(-1);
				}
			}

			__OVERRIDES_END_CHANGES
		}

		void ReAppendTrackers()
		{
			__OVERRIDES_BEGIN_CHANGES

			k2app::shared::devices::devices_overrides_setup_pending = true;

			EraseComboItems();

			_overrideSelectorRows.clear();
			_tracker_pointers = GetTrackerPointerSpan(_type);

			// Try to clear the combo box
			// (this weird shit is an unwrapper for __try)
			[&, this]
			{
				__try
				{
					[&, this]
					{
						_ptr_container_panel.get()->Children().Clear();
					}();
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					[&]
					{
						LOG(WARNING) << "Couldn't clear a StackPanel. You better call an exorcist.";
					}();
				}
			}();

			// Append header to the UI Node
			// (this weird shit is an unwrapper for __try)
			[&, this]
			{
				__try
				{
					[&, this]
					{
						_ptr_container_panel.get()->Children().Append(*_ptr_header);
					}();
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					[&]
					{
						LOG(WARNING) << "Couldn't push to a StackPanel. You better call an exorcist.";
					}();
				}
			}();

			for (const auto& _tracker : _tracker_pointers)
				_overrideSelectorRows.push_back(
					std::make_shared<OverrideSelectorRow>(_tracker));

			// Append selectors to the UI Node
			// (this weird shit is an unwrapper for __try)
			for (const auto& _row : _overrideSelectorRows)
				[&, this]
				{
					__try
					{
						[&, this]
						{
							_ptr_container_panel.get()->Children().Append(*_row.get()->Container());
						}();
					}
					__except (EXCEPTION_EXECUTE_HANDLER)
					{
						[&]
						{
							LOG(WARNING) << "Couldn't push to a StackPanel. You better call an exorcist.";
						}();
					}
				}();

			__OVERRIDES_END_CHANGES
		}

		void SetVisibility(const Visibility& visibility)
		{
			_ptr_container_expander->Visibility(
				(_type == 2 && GetTrackerPointerSpan(_type).empty())
					? Visibility::Collapsed
					: visibility);
		}

		std::vector<std::shared_ptr<OverrideSelectorRow>>* OverrideSelectorRows() { return &_overrideSelectorRows; }

		std::shared_ptr<Expander> ContainerExpander() { return _ptr_container_expander; }

		std::shared_ptr<TextBlock> TrackerName() { return _ptr_tracker; }

		std::shared_ptr<TextBlock> OverrideJointName() { return _ptr_override_joint; }
		std::shared_ptr<TextBlock> PositionJointName() { return _ptr_position_joint; }
		std::shared_ptr<TextBlock> OrientationJointName() { return _ptr_orientation_joint; }

		std::shared_ptr<StackPanel> ContainerPanel() { return _ptr_container_panel; }
		std::shared_ptr<Grid> Header() { return _ptr_header; }

	protected:
		std::vector<std::shared_ptr<OverrideSelectorRow>> _overrideSelectorRows;

		std::vector<k2app::K2AppTracker*> _tracker_pointers;

		// Underlying object shared pointer
		std::shared_ptr<Expander> _ptr_container_expander;

		std::shared_ptr<TextBlock> _ptr_tracker, _ptr_override_joint,
		                           _ptr_position_joint, _ptr_orientation_joint;

		std::shared_ptr<StackPanel> _ptr_container_panel;
		std::shared_ptr<Grid> _ptr_header;

		uint32_t _type = 0;

		std::vector<k2app::K2AppTracker*>
		GetTrackerPointerSpan(const uint32_t& type)
		{
			switch (type)
			{
			case 0:
				return {
					&k2app::K2Settings.K2TrackersVector[0],
					&k2app::K2Settings.K2TrackersVector[1],
					&k2app::K2Settings.K2TrackersVector[2]
				};
			case 1:
				return {
					&k2app::K2Settings.K2TrackersVector[3],
					&k2app::K2Settings.K2TrackersVector[4],
					&k2app::K2Settings.K2TrackersVector[5],
					&k2app::K2Settings.K2TrackersVector[6]
				};
			case 2:
				{
					std::vector<k2app::K2AppTracker*> _tracker_p_vector;
					for (uint32_t index = 7; index < k2app::K2Settings.K2TrackersVector.size(); index++)
						_tracker_p_vector.push_back(&k2app::K2Settings.K2TrackersVector[index]);

					return _tracker_p_vector;
				}
			default:
				{
					std::vector<k2app::K2AppTracker*> _tracker_p_vector;
					for (auto& _t : k2app::K2Settings.K2TrackersVector)
						_tracker_p_vector.push_back(&_t);

					return _tracker_p_vector;
				}
			}
		}

		// Creation: register a host and a callback
		void Create(uint32_t type = 0)
		{
			// Create the container grid
			Expander _container_expander;

			std::map<uint32_t, std::wstring> _type_title
			{
				{
					0, k2app::interfacing::LocalizedResourceWString(
						L"DevicesPage", L"Titles/Joints/WaistAndFeet")
				},
				{
					1, k2app::interfacing::LocalizedResourceWString(
						L"DevicesPage", L"Titles/Joints/ElbowsAndKnees")
				},
				{
					2, k2app::interfacing::LocalizedResourceWString(
						L"DevicesPage", L"Titles/Joints/Other")
				}
			};

			TextBlock _e_header;
			_e_header.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
			_e_header.Text(_type_title[type]);

			_container_expander.Header(_e_header);
			_container_expander.Margin({40, 17, 30, 0});
			_container_expander.IsExpanded(false);
			_container_expander.ExpandDirection(ExpandDirection::Down);
			_container_expander.Visibility(Visibility::Collapsed);
			_container_expander.HorizontalContentAlignment(HorizontalAlignment::Stretch);
			_container_expander.HorizontalAlignment(HorizontalAlignment::Stretch);

			Media::Animation::TransitionCollection c_transition_collection;
			c_transition_collection.Append(Media::Animation::RepositionThemeTransition());
			_container_expander.Transitions(c_transition_collection);

			// Create the title grid
			Grid _header;

			_header.HorizontalAlignment(HorizontalAlignment::Stretch);

			AppendGridStarsColumnMinWidthPixels(_header, 3, 80);
			AppendGridStarColumn(_header);
			AppendGridAutoColumnMinWidthPixels(_header, 140);
			AppendGridStarColumn(_header);
			AppendGridStarsColumnMinWidthPixels(_header, 3, 80);
			AppendGridStarColumn(_header);
			AppendGridStarsColumnMinWidthPixels(_header, 3, 80);

			AppendGridPixelsRow(_header, 30);
			AppendGridPixelsRow(_header, 20);

			for (uint32_t i = 0; i < 7; i++)
			{
				MenuFlyoutSeparator _separator;

				_separator.Margin({4, 0, 4, 0});
				_header.Children().Append(_separator);

				_header.SetRow(_separator, 1);
				_header.SetColumn(_separator, i);
			}

			TextBlock _tracker, _override_label,
			          _position_label, _orientation_label;

			_tracker.HorizontalAlignment(HorizontalAlignment::Center);
			_tracker.VerticalAlignment(VerticalAlignment::Center);

			//_tracker.Margin({-20, 0, 0, 0});

			_tracker.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
			_tracker.Text(k2app::interfacing::LocalizedResourceWString(
				L"DevicesPage", L"Titles/Set/Tracker"));

			_header.Children().Append(_tracker);
			_header.SetRow(_tracker, 0);
			_header.SetColumn(_tracker, 0);

			_override_label.HorizontalAlignment(HorizontalAlignment::Center);
			_override_label.VerticalAlignment(VerticalAlignment::Center);

			_override_label.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
			_override_label.Text(k2app::interfacing::LocalizedResourceWString(
				L"DevicesPage", L"Titles/DeviceOverride"));

			_header.Children().Append(_override_label);
			_header.SetRow(_override_label, 0);
			_header.SetColumn(_override_label, 2);

			_position_label.HorizontalAlignment(HorizontalAlignment::Center);
			_position_label.VerticalAlignment(VerticalAlignment::Center);

			_position_label.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
			_position_label.Text(k2app::interfacing::LocalizedResourceWString(
				L"DevicesPage", L"Titles/Set/Position"));

			_header.Children().Append(_position_label);
			_header.SetRow(_position_label, 0);
			_header.SetColumn(_position_label, 4);

			_orientation_label.HorizontalAlignment(HorizontalAlignment::Center);
			_orientation_label.VerticalAlignment(VerticalAlignment::Center);

			_orientation_label.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
			_orientation_label.Text(k2app::interfacing::LocalizedResourceWString(
				L"DevicesPage", L"Titles/Set/Orientation"));

			_header.Children().Append(_orientation_label);
			_header.SetRow(_orientation_label, 0);
			_header.SetColumn(_orientation_label, 6);

			// Set up the main panel
			StackPanel _container_panel;

			_container_panel.Orientation(Orientation::Vertical);
			_container_panel.HorizontalAlignment(HorizontalAlignment::Stretch);

			_container_panel.Children().Append(_header);

			// Back everything up
			_ptr_container_expander = std::make_shared<Expander>(_container_expander);

			_ptr_tracker = std::make_shared<TextBlock>(_tracker);
			_ptr_override_joint = std::make_shared<TextBlock>(_override_label);
			_ptr_position_joint = std::make_shared<TextBlock>(_position_label);
			_ptr_orientation_joint = std::make_shared<TextBlock>(_orientation_label);

			_ptr_container_panel = std::make_shared<StackPanel>(_container_panel);
			_ptr_header = std::make_shared<Grid>(_header);

			// Set up some signals
			_ptr_container_expander.get()->Expanding(
				[this](const Expander& sender,
				       const ExpanderExpandingEventArgs& e) -> void
				{
					for (auto& expander : k2app::shared::devices::overrideSelectorExpanders)
						if (expander->ContainerExpander().get() != nullptr &&
							expander->ContainerExpander().get() != _ptr_container_expander.get())
							expander->ContainerExpander().get()->IsExpanded(false);
				});

			_ptr_container_expander.get()->Expanding([&](const auto&, const auto&)
			{
				// Don't react to pre-init signals
				if (!k2app::shared::settings::settings_localInitFinished)return;

				// Play a sound
				playAppSound(k2app::interfacing::sounds::AppSounds::Show);
			});

			_ptr_container_expander.get()->Collapsed([&](const auto&, const auto&)
			{
				// Don't react to pre-init signals
				if (!k2app::shared::settings::settings_localInitFinished)return;

				// Play a sound
				playAppSound(k2app::interfacing::sounds::AppSounds::Hide);
			});

			// Push all the trackers
			ReAppendTrackers();
			_container_expander.Content(box_value(_container_panel));
		}
	};
}

// Extension of the k2/interfacing namespace
namespace k2app::interfacing
{
	// Check if we've disabled any joints from spawning and disable their mods
	inline void devices_check_disabled_joints()
	{
		using namespace shared::devices;
		using namespace winrt::Microsoft::UI::Xaml::Controls;

		// Ditch this if not loaded yet
		if (jointsBasisExpanderHostStackPanel.get() == nullptr)return;

		__OVERRIDES_BEGIN_CHANGES

		// Optionally fix combos for disabled trackers -> joint selectors for base
		for (auto& expander : jointSelectorExpanders)
			for (std::shared_ptr<JointSelectorRow>& row : *expander.get()->JointSelectorRows())
			{
				Helpers::SetComboBoxIsEnabled_Safe(
					row.get()->TrackerCombo(),
					row.get()->Tracker()->data_isActive);

				if (!row.get()->Tracker()->data_isActive)
					Helpers::SelectComboBoxItem_Safe(
						row.get()->TrackerCombo(), -1); // Placeholder
			}

		// Optionally fix combos for disabled trackers -> joint selectors for override
		for (auto& expander : overrideSelectorExpanders)
			expander.get()->UpdateIsEnabled();

		__OVERRIDES_END_CHANGES
	}
}
