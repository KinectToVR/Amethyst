#pragma once
#include "pch.h"

#include "K2Interfacing.h"
#include "K2Shared.h"
#include "JointExpander.h"
#include "JointSelectorRow.h"

// Forward declaration
namespace winrt::Microsoft::UI::Xaml::Controls
{
	class JointSelectorExpander;
}

// Extension of the k2/shared namespace
namespace k2app::shared::devices
{
	inline std::array<std::shared_ptr<
		                  winrt::Microsoft::UI::Xaml::Controls::JointSelectorExpander>, 3> jointSelectorExpanders;
}

namespace winrt::Microsoft::UI::Xaml::Controls
{
	/*
	 * Joint expander that allows to :
	 *  - select the position filter
	 *	- select the ori tracking option
	 *	- turn joint/joint pair on/off
	 */

	class JointSelectorExpander
	{
	public:
		// Note: Signals have to be set up manually
		JointSelectorExpander(const uint32_t& type)
		{
			_tracker_pointers = GetTrackerPointerSpan(type);
			_type = type;
			Create(type);
		}

		std::vector<k2app::K2AppTracker*>* TrackerPointers()
		{
			return &_tracker_pointers;
		}

		void ReAppendTrackers()
		{
			_jointSelectorRows.clear();
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
						LOG(WARNING) << "Couldn't clear a ComboBox. You better call an exorcist.";
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
						LOG(WARNING) << "Couldn't clear a ComboBox. You better call an exorcist.";
					}();
				}
			}();

			for (const auto& _tracker : _tracker_pointers)
				_jointSelectorRows.push_back(
					std::shared_ptr<JointSelectorRow>(
						new JointSelectorRow(_tracker)));

			// Append selectors to the UI Node
			// (this weird shit is an unwrapper for __try)
			for (const auto& _row : _jointSelectorRows)
				[&, this]
				{
					__try
					{
						[&, this]
						{
							_ptr_container_panel.get()->Children().Append(*_row->Container());
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

		void SetVisibility(const Visibility& visibility)
		{
			_ptr_container_expander->Visibility(
				(_type == 2 && GetTrackerPointerSpan(_type).empty())
					? Visibility::Collapsed
					: visibility);
		}

		std::vector<std::shared_ptr<JointSelectorRow>>* JointSelectorRows() { return &_jointSelectorRows; }

		std::shared_ptr<Expander> ContainerExpander() { return _ptr_container_expander; }

		std::shared_ptr<TextBlock> TrackerName() { return _ptr_tracker; }
		std::shared_ptr<TextBlock> TrackedJointName() { return _ptr_tracked_joint; }

		std::shared_ptr<StackPanel> ContainerPanel() { return _ptr_container_panel; }
		std::shared_ptr<Grid> Header() { return _ptr_header; }

	protected:
		std::vector<std::shared_ptr<JointSelectorRow>> _jointSelectorRows;

		std::vector<k2app::K2AppTracker*> _tracker_pointers;

		// Underlying object shared pointer
		std::shared_ptr<Expander> _ptr_container_expander;

		std::shared_ptr<TextBlock> _ptr_tracker;
		std::shared_ptr<TextBlock> _ptr_tracked_joint;

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
						L"DevicesPage", L"Titles/Joints/WaistAndFeet/Text")
				},
				{
					1, k2app::interfacing::LocalizedResourceWString(
						L"DevicesPage", L"Titles/Joints/ElbowsAndKnees/Text")
				},
				{
					2, k2app::interfacing::LocalizedResourceWString(
						L"DevicesPage", L"Titles/Joints/Other/Text")
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

			// Create the title grid
			Grid _header;

			_header.HorizontalAlignment(HorizontalAlignment::Stretch);

			AppendGridStarsColumnMinWidthPixels(_header, 3, 80);
			AppendGridStarColumn(_header);
			AppendGridStarsColumnMinWidthPixels(_header, 3, 150);

			AppendGridPixelsRow(_header, 30);
			AppendGridPixelsRow(_header, 20);

			for (uint32_t i = 0; i < 3; i++)
			{
				MenuFlyoutSeparator _separator;

				_separator.Margin({4, 0, 4, 0});
				_header.Children().Append(_separator);

				_header.SetRow(_separator, 1);
				_header.SetColumn(_separator, i);
			}

			TextBlock _tracker, _tracked_joint;

			_tracker.HorizontalAlignment(HorizontalAlignment::Center);
			_tracker.VerticalAlignment(VerticalAlignment::Center);

			_tracker.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
			_tracker.Text(k2app::interfacing::LocalizedResourceWString(
				L"DevicesPage", L"Titles/Set/Tracker/Text"));

			_header.Children().Append(_tracker);
			_header.SetRow(_tracker, 0);
			_header.SetColumn(_tracker, 0);

			_tracked_joint.HorizontalAlignment(HorizontalAlignment::Center);
			_tracked_joint.VerticalAlignment(VerticalAlignment::Center);

			_tracked_joint.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
			_tracked_joint.Text(k2app::interfacing::LocalizedResourceWString(
				L"DevicesPage", L"Titles/Set/TrackedJoint/Text"));

			_header.Children().Append(_tracked_joint);
			_header.SetRow(_tracked_joint, 0);
			_header.SetColumn(_tracked_joint, 2);

			// Set up the main panel
			StackPanel _container_panel;

			_container_panel.Orientation(Orientation::Vertical);
			_container_panel.HorizontalAlignment(HorizontalAlignment::Stretch);

			_container_panel.Children().Append(_header);

			// Back everything up
			_ptr_container_expander = std::make_shared<Expander>(_container_expander);

			_ptr_tracker = std::make_shared<TextBlock>(_tracker);
			_ptr_tracked_joint = std::make_shared<TextBlock>(_tracked_joint);

			_ptr_container_panel = std::make_shared<StackPanel>(_container_panel);
			_ptr_header = std::make_shared<Grid>(_header);

			// Set up some signals
			_ptr_container_expander.get()->Expanding(
				[this](const Controls::Expander& sender,
				       const Controls::ExpanderExpandingEventArgs& e) -> void
				{
					for (auto& expander : k2app::shared::devices::jointSelectorExpanders)
						if (expander->ContainerExpander().get() != nullptr &&
							expander->ContainerExpander().get() != _ptr_container_expander.get())
							expander->ContainerExpander().get()->IsExpanded(false);
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

		// Optionally fix combos for disabled trackers -> joint selectors for base
		for (auto& expander : jointSelectorExpanders)
			for (std::shared_ptr<JointSelectorRow>& row : *expander->JointSelectorRows())
			{
				row.get()->TrackerCombo()->IsEnabled(row.get()->Tracker()->data.isActive);
				if (!row.get()->Tracker()->data.isActive)
					row.get()->TrackerCombo()->SelectedIndex(-1); // Placeholder
			}

		// Optionally fix combos for disabled trackers -> joint selectors for override
		waistPositionOverrideOptionBox.get()->IsEnabled(
			K2Settings.K2TrackersVector[0].data.isActive && K2Settings.K2TrackersVector[0].isPositionOverridden);
		waistRotationOverrideOptionBox.get()->IsEnabled(
			K2Settings.K2TrackersVector[0].data.isActive && K2Settings.K2TrackersVector[0].isRotationOverridden);

		overrideWaistPosition.get()->IsEnabled(K2Settings.K2TrackersVector[0].data.isActive);
		overrideWaistRotation.get()->IsEnabled(K2Settings.K2TrackersVector[0].data.isActive);

		if (!K2Settings.K2TrackersVector[0].data.isActive)
		{
			overrideWaistPosition.get()->IsChecked(false);
			overrideWaistRotation.get()->IsChecked(false);

			waistPositionOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
			waistRotationOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
		}

		leftFootPositionOverrideOptionBox.get()->IsEnabled(
			K2Settings.K2TrackersVector[1].data.isActive && K2Settings.K2TrackersVector[1].isPositionOverridden);
		leftFootRotationOverrideOptionBox.get()->IsEnabled(
			K2Settings.K2TrackersVector[1].data.isActive && K2Settings.K2TrackersVector[1].isRotationOverridden);

		overrideLeftFootPosition.get()->IsEnabled(K2Settings.K2TrackersVector[1].data.isActive);
		overrideLeftFootRotation.get()->IsEnabled(K2Settings.K2TrackersVector[1].data.isActive);

		if (!K2Settings.K2TrackersVector[1].data.isActive)
		{
			overrideLeftFootPosition.get()->IsChecked(false);
			overrideLeftFootRotation.get()->IsChecked(false);

			leftFootPositionOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
			leftFootRotationOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
		}

		rightFootPositionOverrideOptionBox.get()->IsEnabled(
			K2Settings.K2TrackersVector[2].data.isActive && K2Settings.K2TrackersVector[2].isPositionOverridden);
		rightFootRotationOverrideOptionBox.get()->IsEnabled(
			K2Settings.K2TrackersVector[2].data.isActive && K2Settings.K2TrackersVector[2].isRotationOverridden);

		overrideRightFootPosition.get()->IsEnabled(K2Settings.K2TrackersVector[2].data.isActive);
		overrideRightFootRotation.get()->IsEnabled(K2Settings.K2TrackersVector[2].data.isActive);

		if (!K2Settings.K2TrackersVector[2].data.isActive)
		{
			overrideRightFootPosition.get()->IsChecked(false);
			overrideRightFootRotation.get()->IsChecked(false);

			rightFootPositionOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
			rightFootRotationOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
		}

		leftElbowPositionOverrideOptionBox.get()->IsEnabled(
			K2Settings.K2TrackersVector[3].data.isActive && K2Settings.K2TrackersVector[3].isPositionOverridden);
		leftElbowRotationOverrideOptionBox.get()->IsEnabled(
			K2Settings.K2TrackersVector[3].data.isActive && K2Settings.K2TrackersVector[3].isRotationOverridden);

		overrideLeftElbowPosition.get()->IsEnabled(K2Settings.K2TrackersVector[3].data.isActive);
		overrideLeftElbowRotation.get()->IsEnabled(K2Settings.K2TrackersVector[3].data.isActive);

		if (!K2Settings.K2TrackersVector[3].data.isActive)
		{
			overrideLeftElbowPosition.get()->IsChecked(false);
			overrideLeftElbowRotation.get()->IsChecked(false);

			leftElbowPositionOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
			leftElbowRotationOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
		}

		rightElbowPositionOverrideOptionBox.get()->IsEnabled(
			K2Settings.K2TrackersVector[4].data.isActive && K2Settings.K2TrackersVector[4].isPositionOverridden);
		rightElbowRotationOverrideOptionBox.get()->IsEnabled(
			K2Settings.K2TrackersVector[4].data.isActive && K2Settings.K2TrackersVector[4].isRotationOverridden);

		overrideRightElbowPosition.get()->IsEnabled(K2Settings.K2TrackersVector[4].data.isActive);
		overrideRightElbowRotation.get()->IsEnabled(K2Settings.K2TrackersVector[4].data.isActive);

		if (!K2Settings.K2TrackersVector[4].data.isActive)
		{
			overrideRightElbowPosition.get()->IsChecked(false);
			overrideRightElbowRotation.get()->IsChecked(false);

			rightElbowPositionOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
			rightElbowRotationOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
		}

		leftKneePositionOverrideOptionBox.get()->IsEnabled(
			K2Settings.K2TrackersVector[5].data.isActive && K2Settings.K2TrackersVector[5].isPositionOverridden);
		leftKneeRotationOverrideOptionBox.get()->IsEnabled(
			K2Settings.K2TrackersVector[5].data.isActive && K2Settings.K2TrackersVector[5].isRotationOverridden);

		overrideLeftKneePosition.get()->IsEnabled(K2Settings.K2TrackersVector[5].data.isActive);
		overrideLeftKneeRotation.get()->IsEnabled(K2Settings.K2TrackersVector[5].data.isActive);

		if (!K2Settings.K2TrackersVector[5].data.isActive)
		{
			overrideLeftKneePosition.get()->IsChecked(false);
			overrideLeftKneeRotation.get()->IsChecked(false);

			leftKneePositionOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
			leftKneeRotationOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
		}

		rightKneePositionOverrideOptionBox.get()->IsEnabled(
			K2Settings.K2TrackersVector[6].data.isActive && K2Settings.K2TrackersVector[6].isPositionOverridden);
		rightKneeRotationOverrideOptionBox.get()->IsEnabled(
			K2Settings.K2TrackersVector[6].data.isActive && K2Settings.K2TrackersVector[6].isRotationOverridden);

		overrideRightKneePosition.get()->IsEnabled(K2Settings.K2TrackersVector[6].data.isActive);
		overrideRightKneeRotation.get()->IsEnabled(K2Settings.K2TrackersVector[6].data.isActive);

		if (!K2Settings.K2TrackersVector[6].data.isActive)
		{
			overrideRightKneePosition.get()->IsChecked(false);
			overrideRightKneeRotation.get()->IsChecked(false);

			rightKneePositionOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
			rightKneeRotationOverrideOptionBox.get()->SelectedIndex(-1); // Show the placeholder
		}
	}
}
