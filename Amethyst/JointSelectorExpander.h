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
						LOG(WARNING) << "Couldn't push to a ComboBox. You better call an exorcist.";
					}();
				}
			}();

			for (const auto& _tracker : _tracker_pointers)
				_jointSelectorRows.push_back(
					std::make_shared<JointSelectorRow>(_tracker));

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
							LOG(WARNING) << "Couldn't push to a ComboBox. You better call an exorcist.";
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
				[this](const Expander& sender,
				       const ExpanderExpandingEventArgs& e) -> void
				{
					for (auto& expander : k2app::shared::devices::jointSelectorExpanders)
						if (expander.get()->ContainerExpander().get() != nullptr &&
							expander.get()->ContainerExpander().get() != _ptr_container_expander.get())
							expander.get()->ContainerExpander().get()->IsExpanded(false);
				});

			// Push all the trackers
			ReAppendTrackers();
			_container_expander.Content(box_value(_container_panel));
		}
	};
}
