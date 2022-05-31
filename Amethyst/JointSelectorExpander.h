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
	inline std::vector<std::shared_ptr<
		winrt::Microsoft::UI::Xaml::Controls::JointSelectorExpander>> jointSelectorExpanderVector;
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
		JointSelectorExpander(
			std::vector<k2app::K2AppTracker*> tracker_pointers, const uint32_t& type)
		{
			_tracker_pointers = tracker_pointers;
			Create(type);
		}

		std::vector<k2app::K2AppTracker*>* TrackerPointers()
		{
			return &_tracker_pointers;
		}

		void ReAppendTrackers()
		{
			_ptr_container_panel.get()->Children().Clear();
			_ptr_container_panel.get()->Children().Append(*_ptr_header);

			for (const auto& _tracker : _tracker_pointers)
				_ptr_container_panel.get()->Children().Append(
					*std::shared_ptr<Controls::JointSelectorRow>(
						new JointSelectorRow(_tracker))->Container());
		}

		std::shared_ptr<Expander> ContainerExpander() { return _ptr_container_expander; }

		std::shared_ptr<TextBlock> TrackerName() { return _ptr_tracker; }
		std::shared_ptr<TextBlock> TrackedJointName() { return _ptr_tracked_joint; }

		std::shared_ptr<StackPanel> ContainerPanel() { return _ptr_container_panel; }
		std::shared_ptr<Grid> Header() { return _ptr_header; }

	protected:
		std::vector<k2app::K2AppTracker*> _tracker_pointers;

		// Underlying object shared pointer
		std::shared_ptr<Expander> _ptr_container_expander;

		std::shared_ptr<TextBlock> _ptr_tracker;
		std::shared_ptr<TextBlock> _ptr_tracked_joint;

		std::shared_ptr<StackPanel> _ptr_container_panel;
		std::shared_ptr<Grid> _ptr_header;

		// Creation: register a host and a callback
		void Create(uint32_t type = 0)
		{
			// Create the container grid
			Expander _container_expander;

			std::map<uint32_t, std::wstring> _type_title
			{
				{
					0, k2app::interfacing::LocalizedResourceWString(
						L"DevicesPage", L"Titles/Joints/ElbowsAndKnees/Text")
				},
				{
					1, k2app::interfacing::LocalizedResourceWString(
						L"DevicesPage", L"Titles/Joints/WaistAndFeet/Text")
				},
				{
					2, k2app::interfacing::LocalizedResourceWString(
						L"DevicesPage", L"Titles/Joints/Other/Text")
				}
			};

			_container_expander.Header(box_value(_type_title[type]));
			_container_expander.Margin({ 40,17,30,0 });
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

			// Push all the trackers
			ReAppendTrackers();
			_container_expander.Content(box_value(_container_panel));

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
					for (auto& expander : k2app::shared::devices::jointSelectorExpanderVector)
						if (expander->ContainerExpander().get() != nullptr &&
							expander->ContainerExpander().get() != _ptr_container_expander.get())
							expander->ContainerExpander().get()->IsExpanded(false);
				});
		}
	};
}
