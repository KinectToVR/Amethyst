#pragma once
#include "pch.h"

#include "K2Interfacing.h"
#include "K2Shared.h"

#include "JointExpander.h"
#include "OffsetsPivotItem.h"

// Forward declaration
namespace winrt::Microsoft::UI::Xaml::Controls
{
	class OffsetsController;
}

// Extension of the k2/shared namespace
namespace k2app::shared::general
{
	inline std::shared_ptr<
		winrt::Microsoft::UI::Xaml::Controls::OffsetsController> offsetsController;
}

namespace winrt::Microsoft::UI::Xaml::Controls
{
	/*
	 * Joint expander that allows to :
	 *  - select the position filter
	 *	- select the ori tracking option
	 *	- turn joint/joint pair on/off
	 */

	class OffsetsController
	{
	public:
		// Note: Signals have to be set up manually
		OffsetsController()
		{
			Create();
		}

		void ReReadOffsets()
		{
			for (const auto& _item : _offsetsPivotItems)
				_item.get()->ReReadOffsets(); // NumberBoxes are guarded inside anyway
		}

		void ReAppendTrackerPivots()
		{
			_offsetsPivotItems.clear();

			// Try to clear the pivots
			// (this weird shit is an unwrapper for __try)
			[&, this]
			{
				__try
				{
					[&, this]
					{
						_ptr_container.get()->Items().Clear();
					}();
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					[&]
					{
						LOG(WARNING) << "Couldn't clear a Pivot. You better call an exorcist.";
					}();
				}
			}();

			for (auto& _tracker : k2app::K2Settings.K2TrackersVector)
				_offsetsPivotItems.push_back(std::make_shared<OffsetsPivotItem>(&_tracker));

			// Append selectors to the UI Node
			// (this weird shit is an unwrapper for __try)
			for (const auto& _item : _offsetsPivotItems)
				[&, this]
				{
					__try
					{
						[&, this]
						{
							_ptr_container.get()->Items().Append(*(_item)->Container());
						}();
					}
					__except (EXCEPTION_EXECUTE_HANDLER)
					{
						[&]
						{
							LOG(WARNING) << "Couldn't append to a Pivot. You better call an exorcist.";
						}();
					}
				}();

			ReReadOffsets();
		}

		std::vector<std::shared_ptr<OffsetsPivotItem>>* OffsetsPivotItems() { return &_offsetsPivotItems; }

		std::shared_ptr<Pivot> Container() { return _ptr_container; }

	protected:
		std::vector<std::shared_ptr<OffsetsPivotItem>> _offsetsPivotItems;
		uint32_t previous_item_index = 0;

		// Underlying object shared pointer
		std::shared_ptr<Pivot> _ptr_container;

		// Creation: register a host and a callback
		void Create()
		{
			// Create the container grid
			Pivot _container;

			// Cache elements
			_ptr_container = std::make_shared<Pivot>(_container);

			_ptr_container->SelectionChanged(
				[&, this](const auto&, const auto&) -> void
				{
					// The last item
					if (_ptr_container->SelectedIndex() == _ptr_container->Items().Size() - 1)
					{
						playAppSound(previous_item_index == 0
							             ? k2app::interfacing::sounds::AppSounds::MovePrevious
							             : k2app::interfacing::sounds::AppSounds::MoveNext);
					}
					// The first item
					else if (_ptr_container->SelectedIndex() == 0)
					{
						playAppSound(previous_item_index == _ptr_container->Items().Size() - 1
							             ? k2app::interfacing::sounds::AppSounds::MoveNext
							             : k2app::interfacing::sounds::AppSounds::MovePrevious);
					}
					// Default
					else
						playAppSound(_ptr_container->SelectedIndex() > previous_item_index
							             ? k2app::interfacing::sounds::AppSounds::MoveNext
							             : k2app::interfacing::sounds::AppSounds::MovePrevious);

					previous_item_index = _ptr_container->SelectedIndex();
				});

			// Push all the trackers & read values
			ReAppendTrackerPivots();
		}
	};
}
