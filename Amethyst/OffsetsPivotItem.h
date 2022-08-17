#pragma once
#include "pch.h"

#include "K2Interfacing.h"
#include "K2Shared.h"
#include "JointExpander.h"

// Defined in JointSelectorRow.h
inline std::wstring eraseSubStr(std::wstring mainStr, const std::wstring& toErase);

namespace winrt::Microsoft::UI::Xaml::Controls
{
	class OffsetsPivotItem
	{
	public:
		// Note: Signals have to be set up manually
		OffsetsPivotItem(k2app::K2AppTracker* tracker_pointer)
		{
			_tracker_pointer = tracker_pointer;
			Create();
		}

		void ReReadOffsets()
		{
			// Notice that we're gonna change some values
			k2app::shared::general::pending_offsets_update = true;

			// Orientation
			for (uint32_t index = 0; index < 3; index++)
			{
				// ZYX
				std::map<uint32_t, uint32_t> _xyz_zyx_map
					{{0, 2}, {1, 1}, {2, 0}};

				// Update backend offsets with new values DO NOT SAVE
				_ptr_orientation_control_boxes[index].get()->Value(
					static_cast<int>(round(radiansToDegrees(
						_tracker_pointer->orientationOffset(_xyz_zyx_map[index])))));
			}

			// Position
			for (uint32_t index = 0; index < 3; index++)
			{
				// XYZ
				_ptr_position_control_boxes[index].get()->Value(
					static_cast<int>(_tracker_pointer->positionOffset[index] * 100.0));
			}

			// Changes end
			k2app::shared::general::pending_offsets_update = false;
		}

		k2app::K2AppTracker* Tracker() { return _tracker_pointer; }

		std::shared_ptr<PivotItem> Container() { return _ptr_container; }
		std::shared_ptr<Grid> ContainerGrid() { return _ptr_container_grid; }
		std::shared_ptr<TextBlock> OrientationLabel() { return _ptr_orientation_label; }
		std::shared_ptr<TextBlock> PositionLabel() { return _ptr_position_label; }

		std::array<std::shared_ptr<Grid>, 3> OrientationControlGrids() { return _ptr_orientation_control_grids; }
		std::array<std::shared_ptr<TextBlock>, 3> OrientationControlLabels() { return _ptr_orientation_control_labels; }
		std::array<std::shared_ptr<NumberBox>, 3> OrientationControlBoxes() { return _ptr_orientation_control_boxes; }

		std::array<std::shared_ptr<Grid>, 3> PositionControlGrids() { return _ptr_position_control_grids; }
		std::array<std::shared_ptr<TextBlock>, 3> PositionControlLabels() { return _ptr_position_control_labels; }
		std::array<std::shared_ptr<NumberBox>, 3> PositionControlBoxes() { return _ptr_position_control_boxes; }

	protected:
		k2app::K2AppTracker* _tracker_pointer;

		// Underlying object shared pointer
		std::shared_ptr<PivotItem> _ptr_container;
		std::shared_ptr<Grid> _ptr_container_grid;
		std::shared_ptr<TextBlock> _ptr_orientation_label;
		std::shared_ptr<TextBlock> _ptr_position_label;

		std::array<std::shared_ptr<Grid>, 3> _ptr_orientation_control_grids;
		std::array<std::shared_ptr<TextBlock>, 3> _ptr_orientation_control_labels;
		std::array<std::shared_ptr<NumberBox>, 3> _ptr_orientation_control_boxes;

		std::array<std::shared_ptr<Grid>, 3> _ptr_position_control_grids;
		std::array<std::shared_ptr<TextBlock>, 3> _ptr_position_control_labels;
		std::array<std::shared_ptr<NumberBox>, 3> _ptr_position_control_boxes;

		// Creation: register a host and a callback
		void Create()
		{
			// Create the container grid
			Grid _container_grid;

			AppendGridPixelsRow(_container_grid, 20);
			AppendGridPixelsRow(_container_grid, 40);
			AppendGridPixelsRow(_container_grid, 50);
			AppendGridPixelsRow(_container_grid, 20);
			AppendGridPixelsRow(_container_grid, 50);
			AppendGridPixelsRow(_container_grid, 20);
			AppendGridPixelsRow(_container_grid, 50);
			AppendGridPixelsRow(_container_grid, 30);
			AppendGridPixelsRow(_container_grid, 40);
			AppendGridPixelsRow(_container_grid, 50);
			AppendGridPixelsRow(_container_grid, 20);
			AppendGridPixelsRow(_container_grid, 50);
			AppendGridPixelsRow(_container_grid, 20);
			AppendGridPixelsRow(_container_grid, 50);

			// Create and append the titles
			TextBlock _orientation_label, _position_label;

			_orientation_label.FontWeight(winrt::Windows::UI::Text::FontWeights::Medium());
			_orientation_label.Text(k2app::interfacing::LocalizedResourceWString(
				L"DevicesPage", L"Titles/Set/Orientation/Text"));

			_orientation_label.FontSize(20);
			ToolTipService::SetToolTip(_orientation_label,
			                           box_value(k2app::interfacing::LocalizedResourceWString(
				                           L"GeneralPage", L"Captions/Tooltips/Orientation")));

			_position_label.FontWeight(winrt::Windows::UI::Text::FontWeights::Medium());
			_position_label.Text(k2app::interfacing::LocalizedResourceWString(
				L"DevicesPage", L"Titles/Set/Position/Text"));

			_position_label.FontSize(20);
			ToolTipService::SetToolTip(_position_label,
			                           box_value(k2app::interfacing::LocalizedResourceWString(
				                           L"GeneralPage", L"Captions/Tooltips/Position")));

			_container_grid.Children().Append(_orientation_label);
			_container_grid.Children().Append(_position_label);

			_container_grid.SetColumn(_orientation_label, 0);
			_container_grid.SetColumn(_position_label, 0);

			_container_grid.SetRow(_orientation_label, 1);
			_container_grid.SetRow(_position_label, 8);

			// Create and append orientation controls

			std::map<uint32_t, std::wstring> _orientation_labels_map
			{
				{
					0, k2app::interfacing::LocalizedResourceWString(
						L"GeneralPage", L"Captions/Offsets/Orientation/Z")
				},
				{
					1, k2app::interfacing::LocalizedResourceWString(
						L"GeneralPage", L"Captions/Offsets/Orientation/Y")
				},
				{
					2, k2app::interfacing::LocalizedResourceWString(
						L"GeneralPage", L"Captions/Offsets/Orientation/X")
				}
			};

			std::map<uint32_t, std::wstring> _position_labels_map
			{
				{
					0, k2app::interfacing::LocalizedResourceWString(
						L"GeneralPage", L"Captions/Offsets/Position/X")
				},
				{
					1, k2app::interfacing::LocalizedResourceWString(
						L"GeneralPage", L"Captions/Offsets/Position/Y")
				},
				{
					2, k2app::interfacing::LocalizedResourceWString(
						L"GeneralPage", L"Captions/Offsets/Position/Z")
				}
			};

			std::array<Grid, 3> _orientation_control_grids; // Z, Y, X
			std::array<TextBlock, 3> _orientation_control_labels; // Z, Y, X
			std::array<NumberBox, 3> _orientation_control_boxes; // Z, Y, X

			for (uint32_t index = 0; index < 3; index++)
			{
				AppendGridStarColumn(_orientation_control_grids[index]);
				AppendGridPixelsColumn(_orientation_control_grids[index], 270);

				_orientation_control_labels[index].VerticalAlignment(VerticalAlignment::Center);
				_orientation_control_labels[index].HorizontalAlignment(HorizontalAlignment::Left);
				_orientation_control_labels[index].Margin({20, 0, 0, 0});

				_orientation_control_labels[index].FontSize(17);
				_orientation_control_labels[index].FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());

				_orientation_control_labels[index].Text(_orientation_labels_map[index]);

				_orientation_control_grids[index].Children().Append(_orientation_control_labels[index]);
				_orientation_control_grids[index].SetColumn(_orientation_control_labels[index], 0);

				_orientation_control_boxes[index].FontSize(26);
				_orientation_control_boxes[index].FontWeight(winrt::Windows::UI::Text::FontWeights::Normal());

				_orientation_control_boxes[index].Minimum(0);
				_orientation_control_boxes[index].Maximum(360);
				_orientation_control_boxes[index].Value(0);
				_orientation_control_boxes[index].HorizontalContentAlignment(HorizontalAlignment::Stretch);
				_orientation_control_boxes[index].VerticalAlignment(VerticalAlignment::Stretch);
				_orientation_control_boxes[index].SpinButtonPlacementMode(NumberBoxSpinButtonPlacementMode::Inline);
				_orientation_control_boxes[index].SmallChange(1);
				_orientation_control_boxes[index].LargeChange(10);
				_orientation_control_boxes[index].Margin({10, 0, 0, 0});

				_orientation_control_grids[index].Children().Append(_orientation_control_boxes[index]);
				_orientation_control_grids[index].SetColumn(_orientation_control_boxes[index], 1);

				_container_grid.Children().Append(_orientation_control_grids[index]);
				_container_grid.SetColumn(_orientation_control_grids[index], 0);
				_container_grid.SetRow(_orientation_control_grids[index], (index + 1) * 2);
			}

			std::array<Grid, 3> _position_control_grids; // Z, Y, X
			std::array<TextBlock, 3> _position_control_labels; // Z, Y, X
			std::array<NumberBox, 3> _position_control_boxes; // Z, Y, X

			for (uint32_t index = 0; index < 3; index++)
			{
				AppendGridStarColumn(_position_control_grids[index]);
				AppendGridPixelsColumn(_position_control_grids[index], 270);

				_position_control_labels[index].VerticalAlignment(VerticalAlignment::Center);
				_position_control_labels[index].HorizontalAlignment(HorizontalAlignment::Left);
				_position_control_labels[index].Margin({20, 0, 0, 0});

				_position_control_labels[index].FontSize(17);
				_position_control_labels[index].FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());

				_position_control_labels[index].Text(_position_labels_map[index]);

				_position_control_grids[index].Children().Append(_position_control_labels[index]);
				_position_control_grids[index].SetColumn(_position_control_labels[index], 0);

				_position_control_boxes[index].FontSize(26);
				_position_control_boxes[index].FontWeight(winrt::Windows::UI::Text::FontWeights::Normal());

				_position_control_boxes[index].Minimum(-1000);
				_position_control_boxes[index].Maximum(1000);
				_position_control_boxes[index].Value(0);
				_position_control_boxes[index].HorizontalContentAlignment(HorizontalAlignment::Stretch);
				_position_control_boxes[index].VerticalAlignment(VerticalAlignment::Stretch);
				_position_control_boxes[index].SpinButtonPlacementMode(NumberBoxSpinButtonPlacementMode::Inline);
				_position_control_boxes[index].SmallChange(1);
				_position_control_boxes[index].LargeChange(10);
				_position_control_boxes[index].Margin({10, 0, 0, 0});

				_position_control_grids[index].Children().Append(_position_control_boxes[index]);
				_position_control_grids[index].SetColumn(_position_control_boxes[index], 1);

				_container_grid.Children().Append(_position_control_grids[index]);
				_container_grid.SetColumn(_position_control_grids[index], 0);
				_container_grid.SetRow(_position_control_grids[index], (index + 1) * 2 + 7);
			}

			PivotItem _container;
			_container.Content(_container_grid);
			_container.Header(box_value(eraseSubStr(k2app::interfacing::LocalizedResourceWString(
				                                        L"SharedStrings", L"Joints/Enum/" +
				                                        std::to_wstring(static_cast<int>(_tracker_pointer->tracker))),
			                                        L" Tracker")));

			_ptr_container = std::make_shared<PivotItem>(_container);
			_ptr_container_grid = std::make_shared<Grid>(_container_grid);
			_ptr_orientation_label = std::make_shared<TextBlock>(_orientation_label);
			_ptr_position_label = std::make_shared<TextBlock>(_position_label);

			for (uint32_t index = 0; index < 3; index++)
			{
				_ptr_orientation_control_grids[index] = std::make_shared<Grid>(_orientation_control_grids[index]);
				_ptr_orientation_control_labels[index] = std::make_shared<
					TextBlock>(_orientation_control_labels[index]);
				_ptr_orientation_control_boxes[index] = std::make_shared<NumberBox>(_orientation_control_boxes[index]);
				_ptr_position_control_grids[index] = std::make_shared<Grid>(_position_control_grids[index]);
				_ptr_position_control_labels[index] = std::make_shared<TextBlock>(_position_control_labels[index]);
				_ptr_position_control_boxes[index] = std::make_shared<NumberBox>(_position_control_boxes[index]);
			}

			// Orientation
			for (uint32_t index = 0; index < 3; index++)
			{
				// ZYX
				_ptr_orientation_control_boxes[index].get()->ValueChanged(
					[index, this](const winrt::Windows::Foundation::IInspectable& sender,
					              const NumberBoxValueChangedEventArgs& e) -> void
					{
						// Don't react to dummy changes
						if (!k2app::shared::general::general_tab_setup_finished ||
							k2app::shared::general::pending_offsets_update)
							return;

						// Notice that we're gonna change some values
						k2app::shared::general::pending_offsets_update = true;

						// Attempt automatic fixes
						if (isnan(sender.as<NumberBox>().Value()))
							sender.as<NumberBox>().Value(0.0);

						k2app::shared::general::pending_offsets_update = false;

						std::map<uint32_t, uint32_t> _xyz_zyx_map
							{{0, 2}, {1, 1}, {2, 0}};

						// Update backend offsets with new values DO NOT SAVE
						_tracker_pointer->orientationOffset(_xyz_zyx_map[index]) =
							degreesToRadians(sender.as<NumberBox>().Value());

						// Play a sound
						playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);
					});
			}

			// Position
			for (uint32_t index = 0; index < 3; index++)
			{
				// XYZ
				_ptr_position_control_boxes[index].get()->ValueChanged(
					[index, this](const winrt::Windows::Foundation::IInspectable& sender,
					              const NumberBoxValueChangedEventArgs& e) -> void
					{
						// Don't react to dummy changes
						if (!k2app::shared::general::general_tab_setup_finished ||
							k2app::shared::general::pending_offsets_update)
							return;

						// Notice that we're gonna change some values
						k2app::shared::general::pending_offsets_update = true;

						// Attempt automatic fixes
						if (isnan(sender.as<NumberBox>().Value()))
							sender.as<NumberBox>().Value(0.0);

						k2app::shared::general::pending_offsets_update = false;

						// Update backend offsets with new values DO NOT SAVE
						_tracker_pointer->positionOffset[index] =
							sender.as<NumberBox>().Value() / 100.0;

						// Play a sound
						playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);
					});
			}
		}
	};
}
