#pragma once
#include "pch.h"

#include "K2Interfacing.h"
#include "K2Shared.h"
#include "JointExpander.h"

// Forward declaration
namespace winrt::Microsoft::UI::Xaml::Controls
{
	class JointSelectorRow;
}

inline std::wstring eraseSubStr(std::wstring mainStr, const std::wstring& toErase)
{
	// If found then erase it from string
	if (mainStr.find(toErase) != std::wstring::npos)
		mainStr.erase(mainStr.find(toErase), toErase.length());

	return mainStr;
}

namespace winrt::Microsoft::UI::Xaml::Controls
{
	/*
	 * Joint expander that allows to :
	 *  - select the position filter
	 *	- select the ori tracking option
	 *	- turn joint/joint pair on/off
	 */

	class JointSelectorRow
	{
	public:
		// Note: Signals have to be set up manually
		JointSelectorRow(k2app::K2AppTracker* tracker_pointer)
		{
			_tracker_pointer = tracker_pointer;
			Create();
		}

		void ReAppendJoints()
		{
			// Set up (content caption)'s items
			const auto& trackingDevice = TrackingDevices::getCurrentDevice();

			// JointsBasis
			if (trackingDevice.index() == 1)
			{
				const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

				// Try to clear the combo box
				// (this weird shit is an unwrapper for __try)
				[&, this]
				{
					__try
					{
						[&, this]
						{
							_ptr_tracker_combo->Items().Clear();
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

				// Try to push all available joints in
				// Append all joints to all combos
				// (this weird shit is an unwrapper for __try)
				for (const auto& _joint : device->getTrackedJoints())
					// Push the name to the combo
					[&, this]
					{
						__try
						{
							[&, this]
							{
								_ptr_tracker_combo->Items().Append(
									box_value(_joint.getJointName()));
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

			// Check base IDs if wrong
			TrackingDevices::devices_check_base_ids();
			_ptr_tracker_combo->SelectedIndex(_tracker_pointer->selectedTrackedJointID);

			const bool _overriden_from_other_device =
				(_tracker_pointer->isPositionOverridden || _tracker_pointer->isRotationOverridden) &&
				!_tracker_pointer->overrideGUID.empty() &&
				TrackingDevices::IsAnOverride(_tracker_pointer->overrideGUID);
			
			// Optionally show the overlapping badge
			_ptr_other_badge->Opacity(
				_overriden_from_other_device ? 1.0 : 0.0);

			// Set the badge's tooltip
			using namespace std::string_literals;
			ToolTipService::SetToolTip(
				*_ptr_other_badge,
				_overriden_from_other_device
					? box_value(k2app::interfacing::stringReplaceAll_R(
						k2app::interfacing::LocalizedJSONString(
							L"/DevicesPage/ToolTips/Overrides/Overlapping"),
						L"{0}"s,
						TrackingDevices::getDeviceNameFromGUID(_tracker_pointer->overrideGUID)))
					: nullptr); // Either the default one or none
		}

		std::shared_ptr<Grid> Container() { return _ptr_container; }
		std::shared_ptr<Grid> TrackerContainer() { return _ptr_tracker_container; }

		std::shared_ptr<InfoBadge> Badge() { return _ptr_other_badge; }

		std::shared_ptr<TextBlock> TrackedJointName() { return _ptr_title; }
		std::shared_ptr<ComboBox> TrackerCombo() { return _ptr_tracker_combo; }

		k2app::K2AppTracker* Tracker() { return _tracker_pointer; }

	protected:
		k2app::K2AppTracker* _tracker_pointer;

		// Underlying object shared pointer
		std::shared_ptr<Grid> _ptr_container, _ptr_tracker_container;

		std::shared_ptr<InfoBadge> _ptr_other_badge;

		std::shared_ptr<TextBlock> _ptr_title;
		std::shared_ptr<ComboBox> _ptr_tracker_combo;

		// Creation: register a host and a callback
		void Create()
		{
			// Create the container grid
			Grid _container, _title_container;
			InfoBadge _other_badge;

			AppendGridStarsColumnMinWidthPixels(_container, 3, 80);
			AppendGridStarColumn(_container);
			AppendGridStarsColumnMinWidthPixels(_container, 3, 150);

			AppendGridPixelsRow(_container, 50);

			// Create the title text
			TextBlock _title;
			_title.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());

			_title.Text(eraseSubStr(k2app::interfacing::LocalizedResourceWString(
				                        L"SharedStrings", L"Joints/Enum/" +
				                        std::to_wstring(static_cast<int>(_tracker_pointer->base_tracker))),
			                        L" Tracker"));

			_title.FontSize(14);
			_title.Margin({0, -3, 0, 0});
			_title.HorizontalAlignment(HorizontalAlignment::Center);
			_title.VerticalAlignment(VerticalAlignment::Center);
			_title.HorizontalTextAlignment(TextAlignment::Center);

			// Set up content combo
			ComboBox _tracker_combo;
			_tracker_combo.HorizontalAlignment(HorizontalAlignment::Center);
			_tracker_combo.VerticalAlignment(VerticalAlignment::Center);
			_tracker_combo.Height(45);
			_tracker_combo.MinWidth(150);
			_tracker_combo.FontSize(15);
			_tracker_combo.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
			_tracker_combo.PlaceholderText(k2app::interfacing::LocalizedResourceWString(
				L"DevicesPage", L"Placeholders/Joints/Disabled/PlaceholderText"));

			// Create the overlap badge
			_other_badge.Background(*k2app::shared::main::attentionBrush);
			_other_badge.HorizontalAlignment(HorizontalAlignment::Right);
			_other_badge.VerticalAlignment(VerticalAlignment::Center);

			_other_badge.Margin({ .Right = -12, .Bottom = -10 });
			_other_badge.Width(17);
			_other_badge.Height(17);

			_other_badge.Opacity(0.0);
			_other_badge.OpacityTransition(ScalarTransition());

			FontIconSource _icon;
			_icon.Glyph(L"\uEDB1"); // Exclimation
			_icon.Foreground(
				Application::Current().Resources().TryLookup(
					box_value(L"NoThemeColorSolidColorBrush"
					)).as<Media::SolidColorBrush>());

			_other_badge.IconSource(_icon);

			// Push the title & badge to their container
			_title_container.Children().Append(_title);
			_title_container.Children().Append(_other_badge);

			// Push the title container to the main container
			Grid _inner_title_container;
			_inner_title_container.HorizontalAlignment(HorizontalAlignment::Center);
			_inner_title_container.VerticalAlignment(VerticalAlignment::Center);
			_inner_title_container.Children().Append(_title_container);

			_container.Children().Append(_inner_title_container);
			_container.SetColumn(_inner_title_container, 0);

			// Append all elements to the container
			_container.Children().Append(_tracker_combo);
			_container.SetColumn(_tracker_combo, 2);

			// Back everything up
			_ptr_container = std::make_shared<Grid>(_container);
			_ptr_tracker_container = std::make_shared<Grid>(_title_container);
			_ptr_other_badge = std::make_shared<InfoBadge>(_other_badge);
			_ptr_title = std::make_shared<TextBlock>(_title);
			_ptr_tracker_combo = std::make_shared<ComboBox>(_tracker_combo);

			// Append tracked joints
			ReAppendJoints();

			// Set up some signals
			_ptr_tracker_combo->SelectionChanged(
				[this](const winrt::Windows::Foundation::IInspectable& sender,
				       const SelectionChangedEventArgs& e) -> void
				{
					// Don't even try if we're not set up yet
					if (!k2app::shared::devices::devices_tab_setup_finished ||
						k2app::shared::devices::devices_joints_setup_pending)
						return;

					if (_ptr_tracker_combo->SelectedIndex() >= 0)
						_tracker_pointer->selectedTrackedJointID = _ptr_tracker_combo->SelectedIndex();
					else
						_ptr_tracker_combo->SelectedItem(e.RemovedItems().GetAt(0));

					// If we're using a joints device then also signal the joint
					const auto& trackingDevice = TrackingDevices::getCurrentDevice();
					if (trackingDevice.index() == 1 &&
						k2app::shared::devices::devices_tab_re_setup_finished &&
						k2app::shared::devices::devices_signal_joints)
						// if JointsBasis & Setup Finished
						std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice)->
							signalJoint(_tracker_pointer->selectedTrackedJointID);

					// Save settings
					k2app::interfacing::statusUIRefreshRequested_Urgent = true;
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
		}
	};
}
