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

// Extension of the k2/shared namespace
namespace k2app::shared::settings
{
	//inline std::vector<std::shared_ptr<
	//	winrt::Microsoft::UI::Xaml::Controls::JointExpander>> jointExpanderVector;
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
				[&]
				{
					__try
					{
						[&]
						{
							_ptr_tracker_combo.get()->Items().Clear();
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
				for (auto& _joint : device->getTrackedJoints())
					// Push the name to the combo
					[&]
				{
					__try
					{
						[&]
						{
							_ptr_tracker_combo.get()->Items().Append(box_value(StringToWString(_joint.getJointName())));
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
			TrackingDevices::devices_check_base_ids(k2app::K2Settings.trackingDeviceID);
			_ptr_tracker_combo.get()->SelectedIndex(_tracker_pointer->selectedTrackedJointID);
		}

		std::shared_ptr<Grid> Container() { return _ptr_container; }

		std::shared_ptr<TextBlock> TrackedJointName() { return _ptr_title; }
		std::shared_ptr<ComboBox> TrackerCombo() { return _ptr_tracker_combo; }

	protected:
		k2app::K2AppTracker* _tracker_pointer;

		// Underlying object shared pointer
		std::shared_ptr<Grid> _ptr_container;
		std::shared_ptr<TextBlock> _ptr_title;
		std::shared_ptr<ComboBox> _ptr_tracker_combo;

		// Creation: register a host and a callback
		void Create()
		{
			// Create the container grid
			Grid _container;

			AppendGridStarsColumnMinWidthPixels(_container, 3, 80);
			AppendGridStarColumn(_container);
			AppendGridStarsColumnMinWidthPixels(_container, 3, 150);

			AppendGridPixelsRow(_container, 50);

			// Create the title text
			TextBlock _title;
			_title.Width(130);
			_title.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());

			_title.Text(eraseSubStr(k2app::interfacing::LocalizedResourceWString(
				                        L"SharedStrings", L"Joints/Enum/" +
				                        std::to_wstring(static_cast<int>(_tracker_pointer->tracker))), L" Tracker"));

			_title.FontSize(14);
			_title.Margin({0, -3, 0, 0});
			_title.HorizontalAlignment(HorizontalAlignment::Center);
			_title.VerticalAlignment(VerticalAlignment::Center);

			// Set up content combo
			ComboBox _tracker_combo;
			_tracker_combo.HorizontalAlignment(HorizontalAlignment::Center);
			_tracker_combo.VerticalAlignment(VerticalAlignment::Center);
			_tracker_combo.Height(45);
			_tracker_combo.Width(150);
			_tracker_combo.FontSize(15);
			_tracker_combo.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
			_tracker_combo.PlaceholderText(k2app::interfacing::LocalizedResourceWString(
				L"DevicesPage", L"Placeholders/Joints/Disabled/PlaceholderText"));

			// Append tracked joints
			ReAppendJoints();
			
			// Append all elements to the container
			_container.Children().Append(_title);
			_container.Children().Append(_tracker_combo);

			_container.SetColumn(_title, 0);
			_container.SetRow(_title, 2);

			_container.SetColumn(_tracker_combo, 2);
			_container.SetRow(_tracker_combo, 2);
			
			// Back everything up
			_ptr_container = std::make_shared<Grid>(_container);
			_ptr_title = std::make_shared<TextBlock>(_title);
			_ptr_tracker_combo = std::make_shared<ComboBox>(_tracker_combo);

			// Set up some signals
			_ptr_tracker_combo->SelectionChanged(
				[this](const winrt::Windows::Foundation::IInspectable& sender,
				       const Controls::SelectionChangedEventArgs& e) -> void
				{
					if (!k2app::shared::devices::devices_tab_setup_finished)return; // Don't even try if we're not set up yet
					if (_ptr_tracker_combo.get()->SelectedIndex() >= 0)
						_tracker_pointer->selectedTrackedJointID = _ptr_tracker_combo.get()->SelectedIndex();

					// If we're using a joints device then also signal the joint
					const auto& trackingDevice = TrackingDevices::getCurrentDevice();
					if (trackingDevice.index() == 1 && k2app::shared::devices::devices_tab_re_setup_finished) // if JointsBasis & Setup Finished
						std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice)->
						signalJoint(_tracker_pointer->selectedTrackedJointID);

					// Save settings
					k2app::K2Settings.saveSettings();
				});
		}
	};
}