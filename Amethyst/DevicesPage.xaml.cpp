#include "pch.h"
#include "DevicesPage.xaml.h"
#if __has_include("DevicesPage.g.cpp")
#include "DevicesPage.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace k2app::shared::devices;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

void devices_clear_combo(const std::shared_ptr<Controls::ComboBox>& cbox)
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

void devices_push_combobox(
	const std::shared_ptr<Controls::ComboBox>& cbox,
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


void devices_push_override_joints_combo(
	const std::shared_ptr<Controls::ComboBox>& cbox,
	const bool& all = true)
{
	devices_push_combobox(cbox, 
		eraseSubStr(k2app::interfacing::LocalizedResourceWString(
		L"SharedStrings", L"Joints/Enum/" +
		std::to_wstring(static_cast<int>(ktvr::ITrackerType::Tracker_Chest))), L" Tracker").c_str());

	if (all)
	{
		devices_push_combobox(cbox,
			eraseSubStr(k2app::interfacing::LocalizedResourceWString(
				L"SharedStrings", L"Joints/Enum/" +
				std::to_wstring(static_cast<int>(ktvr::ITrackerType::Tracker_LeftElbow))), L" Tracker").c_str());
		devices_push_combobox(cbox,
			eraseSubStr(k2app::interfacing::LocalizedResourceWString(
				L"SharedStrings", L"Joints/Enum/" +
				std::to_wstring(static_cast<int>(ktvr::ITrackerType::Tracker_RightElbow))), L" Tracker").c_str());
	}

	devices_push_combobox(cbox,
		eraseSubStr(k2app::interfacing::LocalizedResourceWString(
			L"SharedStrings", L"Joints/Enum/" +
			std::to_wstring(static_cast<int>(ktvr::ITrackerType::Tracker_Waist))), L" Tracker").c_str());

	if (all)
	{
		devices_push_combobox(cbox,
			eraseSubStr(k2app::interfacing::LocalizedResourceWString(
				L"SharedStrings", L"Joints/Enum/" +
				std::to_wstring(static_cast<int>(ktvr::ITrackerType::Tracker_LeftKnee))), L" Tracker").c_str());
		devices_push_combobox(cbox,
			eraseSubStr(k2app::interfacing::LocalizedResourceWString(
				L"SharedStrings", L"Joints/Enum/" +
				std::to_wstring(static_cast<int>(ktvr::ITrackerType::Tracker_RightKnee))), L" Tracker").c_str());
	}

	devices_push_combobox(cbox,
		eraseSubStr(k2app::interfacing::LocalizedResourceWString(
			L"SharedStrings", L"Joints/Enum/" +
			std::to_wstring(static_cast<int>(ktvr::ITrackerType::Tracker_LeftFoot))), L" Tracker").c_str());
	devices_push_combobox(cbox,
		eraseSubStr(k2app::interfacing::LocalizedResourceWString(
			L"SharedStrings", L"Joints/Enum/" +
			std::to_wstring(static_cast<int>(ktvr::ITrackerType::Tracker_RightFoot))), L" Tracker").c_str());
}

void devices_push_override_joints(const bool& all = true)
{
	// Waist
	devices_push_override_joints_combo(waistPositionOverrideOptionBox, all);
	devices_push_override_joints_combo(waistRotationOverrideOptionBox, all);

	// LFoot
	devices_push_override_joints_combo(leftFootPositionOverrideOptionBox, all);
	devices_push_override_joints_combo(leftFootRotationOverrideOptionBox, all);

	// RFoot
	devices_push_override_joints_combo(rightFootPositionOverrideOptionBox, all);
	devices_push_override_joints_combo(rightFootRotationOverrideOptionBox, all);

	// LElbow
	devices_push_override_joints_combo(leftElbowPositionOverrideOptionBox, all);
	devices_push_override_joints_combo(leftElbowRotationOverrideOptionBox, all);

	// RElbow
	devices_push_override_joints_combo(rightElbowPositionOverrideOptionBox, all);
	devices_push_override_joints_combo(rightElbowRotationOverrideOptionBox, all);

	// LKnee
	devices_push_override_joints_combo(leftKneePositionOverrideOptionBox, all);
	devices_push_override_joints_combo(leftKneeRotationOverrideOptionBox, all);

	// RKnee
	devices_push_override_joints_combo(rightKneePositionOverrideOptionBox, all);
	devices_push_override_joints_combo(rightKneeRotationOverrideOptionBox, all);
}

void devices_push_override_joints(const std::wstring& _string)
{
	// Convert
	const hstring string = _string.c_str();

	// Waist
	devices_push_combobox(waistPositionOverrideOptionBox, string);
	devices_push_combobox(waistRotationOverrideOptionBox, string);

	// LFoot
	devices_push_combobox(leftFootPositionOverrideOptionBox, string);
	devices_push_combobox(leftFootRotationOverrideOptionBox, string);

	// RFoot
	devices_push_combobox(rightFootPositionOverrideOptionBox, string);
	devices_push_combobox(rightFootRotationOverrideOptionBox, string);

	// LElbow
	devices_push_combobox(leftElbowPositionOverrideOptionBox, string);
	devices_push_combobox(leftElbowRotationOverrideOptionBox, string);

	// RElbow
	devices_push_combobox(rightElbowPositionOverrideOptionBox, string);
	devices_push_combobox(rightElbowRotationOverrideOptionBox, string);

	// LKnee
	devices_push_combobox(leftKneePositionOverrideOptionBox, string);
	devices_push_combobox(leftKneeRotationOverrideOptionBox, string);

	// RKnee
	devices_push_combobox(rightKneePositionOverrideOptionBox, string);
	devices_push_combobox(rightKneeRotationOverrideOptionBox, string);
}

void devices_select_combobox_safe(
	const std::shared_ptr<Controls::ComboBox>& cbox,
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

void devices_select_combobox()
{
	// Waist
	devices_select_combobox_safe(
		waistPositionOverrideOptionBox,
		k2app::K2Settings.K2TrackersVector[0].isPositionOverridden
			? k2app::K2Settings.K2TrackersVector[0].positionOverrideJointID
			: -1);
	devices_select_combobox_safe(
		waistRotationOverrideOptionBox,
		k2app::K2Settings.K2TrackersVector[0].isRotationOverridden
			? k2app::K2Settings.K2TrackersVector[0].rotationOverrideJointID
			: -1);

	// LeftF
	devices_select_combobox_safe(
		leftFootPositionOverrideOptionBox,
		k2app::K2Settings.K2TrackersVector[1].isPositionOverridden
			? k2app::K2Settings.K2TrackersVector[2].positionOverrideJointID
			: -1);
	devices_select_combobox_safe(
		leftFootRotationOverrideOptionBox,
		k2app::K2Settings.K2TrackersVector[1].isRotationOverridden
			? k2app::K2Settings.K2TrackersVector[1].rotationOverrideJointID
			: -1);

	// RightF
	devices_select_combobox_safe(
		rightFootPositionOverrideOptionBox,
		k2app::K2Settings.K2TrackersVector[2].isPositionOverridden
			? k2app::K2Settings.K2TrackersVector[2].positionOverrideJointID
			: -1);
	devices_select_combobox_safe(
		rightFootRotationOverrideOptionBox,
		k2app::K2Settings.K2TrackersVector[2].isRotationOverridden
			? k2app::K2Settings.K2TrackersVector[2].rotationOverrideJointID
			: -1);

	// LeftEL
	devices_select_combobox_safe(
		leftElbowPositionOverrideOptionBox,
		k2app::K2Settings.K2TrackersVector[3].isPositionOverridden
			? k2app::K2Settings.K2TrackersVector[3].positionOverrideJointID
			: -1);
	devices_select_combobox_safe(
		leftElbowRotationOverrideOptionBox,
		k2app::K2Settings.K2TrackersVector[3].isRotationOverridden
			? k2app::K2Settings.K2TrackersVector[3].rotationOverrideJointID
			: -1);

	// RightEL
	devices_select_combobox_safe(
		rightElbowPositionOverrideOptionBox,
		k2app::K2Settings.K2TrackersVector[4].isPositionOverridden
			? k2app::K2Settings.K2TrackersVector[4].positionOverrideJointID
			: -1);
	devices_select_combobox_safe(
		rightElbowRotationOverrideOptionBox,
		k2app::K2Settings.K2TrackersVector[4].isRotationOverridden
			? k2app::K2Settings.K2TrackersVector[4].rotationOverrideJointID
			: -1);

	// LeftK
	devices_select_combobox_safe(
		leftKneePositionOverrideOptionBox,
		k2app::K2Settings.K2TrackersVector[5].isPositionOverridden
			? k2app::K2Settings.K2TrackersVector[5].positionOverrideJointID
			: -1);
	devices_select_combobox_safe(
		leftKneeRotationOverrideOptionBox,
		k2app::K2Settings.K2TrackersVector[5].isRotationOverridden
			? k2app::K2Settings.K2TrackersVector[5].rotationOverrideJointID
			: -1);

	// RightK
	devices_select_combobox_safe(
		rightKneePositionOverrideOptionBox,
		k2app::K2Settings.K2TrackersVector[6].isPositionOverridden
			? k2app::K2Settings.K2TrackersVector[6].positionOverrideJointID
			: -1);
	devices_select_combobox_safe(
		rightKneeRotationOverrideOptionBox,
		k2app::K2Settings.K2TrackersVector[6].isRotationOverridden
			? k2app::K2Settings.K2TrackersVector[6].rotationOverrideJointID
			: -1);
}

namespace winrt::KinectToVR::implementation
{
	DevicesPage::DevicesPage()
	{
		InitializeComponent();

		// Cache needed UI elements
		jointsBasisExpanderHostStackPanel = std::make_shared<Controls::StackPanel>(JointsBasisExpanderHostStackPanel());

		deviceNameLabel = std::make_shared<Controls::TextBlock>(SelectedDeviceNameLabel());
		deviceStatusLabel = std::make_shared<Controls::TextBlock>(TrackingDeviceStatusLabel());
		errorWhatText = std::make_shared<Controls::TextBlock>(ErrorWhatText());
		trackingDeviceErrorLabel = std::make_shared<Controls::TextBlock>(TrackingDeviceErrorLabel());
		baseDeviceName = std::make_shared<Controls::TextBlock>(BaseDeviceName());
		overrideDeviceName = std::make_shared<Controls::TextBlock>(OverrideDeviceName());
		overridesLabel = std::make_shared<Controls::TextBlock>(OverridesLabel());
		jointBasisLabel = std::make_shared<Controls::TextBlock>(JointBasisLabel());

		deviceErrorGrid = std::make_shared<Controls::Grid>(DeviceErrorGrid());
		trackingDeviceChangePanel = std::make_shared<Controls::Grid>(TrackingDeviceChangePanel());
		overridesControls = std::make_shared<Controls::Grid>(OverridesControls());
		overridesControls_1 = std::make_shared<Controls::Grid>(OverridesControls_1());
		devicesMainContentGridOuter = std::make_shared<Controls::Grid>(DevicesMainContentGridOuter());
		devicesMainContentGridInner = std::make_shared<Controls::Grid>(DevicesMainContentGridInner());
		
		overridesDropDown = std::make_shared<Controls::Expander>(OverridesDropDown());
		overridesDropDown_1 = std::make_shared<Controls::Expander>(OverridesDropDown_1());

		devicesListView = std::make_shared<Controls::ListView>(TrackingDeviceListView());

		setAsOverrideButton = std::make_shared<Controls::Button>(SetAsOverrideButton());
		setAsBaseButton = std::make_shared<Controls::Button>(SetAsBaseButton());
		deselectDeviceButton = std::make_shared<Controls::Button>(DeselectDeviceButton());

		waistRotationOverrideOptionBox = std::make_shared<Controls::ComboBox>(WaistRotationOverrideOptionBox());
		waistPositionOverrideOptionBox = std::make_shared<Controls::ComboBox>(WaistPositionOverrideOptionBox());
		leftFootRotationOverrideOptionBox = std::make_shared<Controls::ComboBox>(LeftFootRotationOverrideOptionBox());
		leftFootPositionOverrideOptionBox = std::make_shared<Controls::ComboBox>(LeftFootPositionOverrideOptionBox());
		rightFootPositionOverrideOptionBox = std::make_shared<Controls::ComboBox>(RightFootPositionOverrideOptionBox());
		rightFootRotationOverrideOptionBox = std::make_shared<Controls::ComboBox>(RightFootRotationOverrideOptionBox());
		leftElbowPositionOverrideOptionBox = std::make_shared<Controls::ComboBox>(LeftElbowPositionOverrideOptionBox());
		leftElbowRotationOverrideOptionBox = std::make_shared<Controls::ComboBox>(LeftElbowRotationOverrideOptionBox());
		rightElbowPositionOverrideOptionBox = std::make_shared<Controls::ComboBox>(
			RightElbowPositionOverrideOptionBox());
		rightElbowRotationOverrideOptionBox = std::make_shared<Controls::ComboBox>(
			RightElbowRotationOverrideOptionBox());
		leftKneePositionOverrideOptionBox = std::make_shared<Controls::ComboBox>(LeftKneePositionOverrideOptionBox());
		leftKneeRotationOverrideOptionBox = std::make_shared<Controls::ComboBox>(LeftKneeRotationOverrideOptionBox());
		rightKneePositionOverrideOptionBox = std::make_shared<Controls::ComboBox>(RightKneePositionOverrideOptionBox());
		rightKneeRotationOverrideOptionBox = std::make_shared<Controls::ComboBox>(RightKneeRotationOverrideOptionBox());

		overrideWaistPosition = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideWaistPosition());
		overrideWaistRotation = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideWaistRotation());
		overrideLeftFootPosition = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideLeftFootPosition());
		overrideLeftFootRotation = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideLeftFootRotation());
		overrideRightFootPosition = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideRightFootPosition());
		overrideRightFootRotation = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideRightFootRotation());
		overrideLeftElbowPosition = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideLeftElbowPosition());
		overrideLeftElbowRotation = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideLeftElbowRotation());
		overrideRightElbowPosition = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideRightElbowPosition());
		overrideRightElbowRotation = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideRightElbowRotation());
		overrideLeftKneePosition = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideLeftKneePosition());
		overrideLeftKneeRotation = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideLeftKneeRotation());
		overrideRightKneePosition = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideRightKneePosition());
		overrideRightKneeRotation = std::make_shared<Controls::ToggleMenuFlyoutItem>(OverrideRightKneeRotation());

		overridesDropDown = std::make_shared<Controls::Expander>(OverridesDropDown());
		overridesDropDown_1 = std::make_shared<Controls::Expander>(OverridesDropDown_1());

		devicesMainContentScrollViewer = std::make_shared<Controls::ScrollViewer>(DevicesMainContentScrollViewer());

		devicesOverridesSelectorStackPanelOuter = std::make_shared<Controls::StackPanel>(
			DevicesOverridesSelectorStackPanelOuter());
		devicesOverridesSelectorStackPanelInner = std::make_shared<Controls::StackPanel>(
			DevicesOverridesSelectorStackPanelInner());

		devicesJointsBasisSelectorStackPanelOuter = std::make_shared<Controls::StackPanel>(
			DevicesJointsBasisSelectorStackPanelOuter());
		devicesJointsBasisSelectorStackPanelInner = std::make_shared<Controls::StackPanel>(
			DevicesJointsBasisSelectorStackPanelInner());

		selectedDeviceSettingsRootLayoutPanel = std::make_shared<Controls::StackPanel>(
			SelectedDeviceSettingsRootLayoutPanel());

		selectedDeviceSettingsButton = std::make_shared<Controls::AppBarButton>(SelectedDeviceSettingsButton());

		selectedDeviceSettingsFlyout = std::make_shared<Controls::Flyout>(SelectedDeviceSettingsFlyout());

		// Create tracking devices' list
		static auto m_TrackingDevicesViewModels =
			multi_threaded_observable_vector<KinectToVR::TrackingDevicesView>();

		// Watch for insertions
		m_TrackingDevicesViewModels.VectorChanged(
			[&](const Windows::Foundation::Collections::IObservableVector<KinectToVR::TrackingDevicesView>& sender,
			    const Windows::Foundation::Collections::IVectorChangedEventArgs& args)
			{
				// Report a registration and parse
				LOG(INFO) << WStringToString(sender.GetAt(sender.Size() - 1).DeviceName().c_str()) <<
					"'s been registered as a tracking device. [UI Node]";

				// Set the current device
				if (sender.Size() > k2app::K2Settings.trackingDeviceID)
					sender.GetAt(k2app::K2Settings.trackingDeviceID).Current(true);

				// Re-set all indexes
				for (uint32_t i = 0; i < sender.Size(); i++)
					sender.GetAt(i).DeviceID(i);
			});

		// Add tracking devices here
		for (const auto& device : TrackingDevices::TrackingDevicesVector)
		{
			std::string deviceName = "[UNKNOWN]";

			switch (device.index())
			{
			case 0:
				{
					const auto& pDevice = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(device);
					deviceName = pDevice->getDeviceName();
				}
				break;
			case 1:
				{
					const auto& pDevice = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(device);
					deviceName = pDevice->getDeviceName();
				}
				break;
			}

			LOG(INFO) << "Appending " << deviceName <<
				" to UI Node's tracking devices' list...";
			m_TrackingDevicesViewModels.Append(
				make<TrackingDevicesView>(StringToWString(deviceName).c_str()));
		}

		// Register tracking devices' list
		devicesListView.get()->ItemsSource(m_TrackingDevicesViewModels);

		// Set currently tracking device & selected device
		// RadioButton is set on ItemChanged
		devicesListView.get()->SelectedIndex(k2app::K2Settings.trackingDeviceID);

		// Set joint expanders up

		// Type 0: WF
		jointSelectorExpanders[0] = std::move(std::shared_ptr<Controls::JointSelectorExpander>(
			new Controls::JointSelectorExpander({ &k2app::K2Settings.K2TrackersVector[0], 
				&k2app::K2Settings.K2TrackersVector[1], &k2app::K2Settings.K2TrackersVector[2] }, 0)));

		// Type 1: EK
		jointSelectorExpanders[1] = std::move(std::shared_ptr<Controls::JointSelectorExpander>(
			new Controls::JointSelectorExpander({ &k2app::K2Settings.K2TrackersVector[3], 
				&k2app::K2Settings.K2TrackersVector[4], &k2app::K2Settings.K2TrackersVector[5],
				&k2app::K2Settings.K2TrackersVector[6] }, 1)));

		// Type 2: OTHER
		std::vector<k2app::K2AppTracker*> _tracker_p_vector;
		for (uint32_t index = 7; index < k2app::K2Settings.K2TrackersVector.size(); index++)
			_tracker_p_vector.push_back(&k2app::K2Settings.K2TrackersVector[index]);

		jointSelectorExpanders[2] = std::move(std::shared_ptr<Controls::JointSelectorExpander>(
			new Controls::JointSelectorExpander(_tracker_p_vector, 2)));

		NavigationCacheMode(Navigation::NavigationCacheMode::Required);
		TrackingDevices::devices_update_current();
	}
}


Windows::Foundation::IAsyncAction
KinectToVR::implementation::DevicesPage::TrackingDeviceListView_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	if (!devices_tab_setup_finished)co_return; // Block dummy selects

	selectedTrackingDeviceID = sender.as<Controls::ListView>().SelectedIndex();
	const auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(selectedTrackingDeviceID);

	std::string deviceName = "[UNKNOWN]";
	std::wstring device_status = L"Something's wrong!\nE_UKNOWN\nWhat's happened here?";

	// Only if override -> select enabled combos
	if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
	{
		overrideWaistPosition.get()->IsChecked(k2app::K2Settings.K2TrackersVector[0].isPositionOverridden);
		overrideLeftFootPosition.get()->IsChecked(k2app::K2Settings.K2TrackersVector[1].isPositionOverridden);
		overrideRightFootPosition.get()->IsChecked(k2app::K2Settings.K2TrackersVector[2].isPositionOverridden);

		overrideWaistRotation.get()->IsChecked(k2app::K2Settings.K2TrackersVector[0].isRotationOverridden);
		overrideLeftFootRotation.get()->IsChecked(k2app::K2Settings.K2TrackersVector[1].isRotationOverridden);
		overrideRightFootRotation.get()->IsChecked(k2app::K2Settings.K2TrackersVector[2].isRotationOverridden);

		overrideLeftElbowPosition.get()->IsChecked(k2app::K2Settings.K2TrackersVector[3].isPositionOverridden);
		overrideLeftElbowRotation.get()->IsChecked(k2app::K2Settings.K2TrackersVector[3].isRotationOverridden);

		overrideRightElbowPosition.get()->IsChecked(k2app::K2Settings.K2TrackersVector[4].isPositionOverridden);
		overrideRightElbowRotation.get()->IsChecked(k2app::K2Settings.K2TrackersVector[4].isRotationOverridden);

		overrideLeftKneePosition.get()->IsChecked(k2app::K2Settings.K2TrackersVector[5].isPositionOverridden);
		overrideLeftKneeRotation.get()->IsChecked(k2app::K2Settings.K2TrackersVector[5].isRotationOverridden);

		overrideRightKneePosition.get()->IsChecked(k2app::K2Settings.K2TrackersVector[6].isPositionOverridden);
		overrideRightKneeRotation.get()->IsChecked(k2app::K2Settings.K2TrackersVector[6].isRotationOverridden);
	}

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

		deviceNameLabel.get()->Text(StringToWString(device->getDeviceName()));
		device_status = device->statusResultWString(device->getStatusResult());

		deviceName = device->getDeviceName();

		// Show / Hide device settings button
		selectedDeviceSettingsButton.get()->Visibility(
			device->isSettingsDaemonSupported()
				? Visibility::Visible
				: Visibility::Collapsed);

		// Append device settings / placeholder layout
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
			device->isSettingsDaemonSupported()
				? *TrackingDevices::TrackingDevicesLayoutRootsVector.at(
					selectedTrackingDeviceID)->Get()
				: *k2app::interfacing::emptyLayoutRoot->Get());

		// We've selected a kinectbasis device, so this should be hidden
		for(auto& expander: jointSelectorExpanders)
			expander.get()->ContainerExpander().get()->Visibility(Visibility::Collapsed);
		
		jointBasisLabel.get()->Visibility(Visibility::Collapsed);

		// Set up combos if the device's OK
		if (device_status.find(L"S_OK") != std::wstring::npos)
		{
			// If we're reconnecting an override device, also refresh joints
			if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
			{
				// Clear items
				// // Waist
				devices_clear_combo(waistPositionOverrideOptionBox);
				devices_clear_combo(waistRotationOverrideOptionBox);

				// LeftF
				devices_clear_combo(leftFootPositionOverrideOptionBox);
				devices_clear_combo(leftFootRotationOverrideOptionBox);

				// RightF
				devices_clear_combo(rightFootPositionOverrideOptionBox);
				devices_clear_combo(rightFootRotationOverrideOptionBox);

				// LeftEL
				devices_clear_combo(leftElbowPositionOverrideOptionBox);
				devices_clear_combo(leftElbowRotationOverrideOptionBox);

				// RightEL
				devices_clear_combo(rightElbowPositionOverrideOptionBox);
				devices_clear_combo(rightElbowRotationOverrideOptionBox);

				// LeftK
				devices_clear_combo(leftKneePositionOverrideOptionBox);
				devices_clear_combo(leftKneeRotationOverrideOptionBox);

				// RightK
				devices_clear_combo(rightKneePositionOverrideOptionBox);
				devices_clear_combo(rightKneeRotationOverrideOptionBox);

				// Append all joints to all combos, depend on characteristics
				switch (device->getDeviceCharacteristics())
				{
				case ktvr::K2_Character_Basic:
					{
						devices_push_override_joints(false);
					}
					break;
				case ktvr::K2_Character_Simple:
					{
						devices_push_override_joints();
					}
					break;
				case ktvr::K2_Character_Full:
					{
						devices_push_override_joints();
					}
					break;
				}

				// Try fix override IDs if wrong
				TrackingDevices::devices_check_override_ids(selectedTrackingDeviceID);

				// Select the first (or next, if exists) joint
				// Set the placeholder text on disabled combos
				devices_select_combobox();
			}
		}
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

		deviceNameLabel.get()->Text(StringToWString(device->getDeviceName()));
		device_status = device->statusResultWString(device->getStatusResult());

		deviceName = device->getDeviceName();

		// Show / Hide device settings button
		selectedDeviceSettingsButton.get()->Visibility(
			device->isSettingsDaemonSupported()
				? Visibility::Visible
				: Visibility::Collapsed);

		// Append device settings / placeholder layout
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
			device->isSettingsDaemonSupported()
				? *TrackingDevices::TrackingDevicesLayoutRootsVector.at(
					selectedTrackingDeviceID)->Get()
				: *k2app::interfacing::emptyLayoutRoot->Get());

		// We've selected a jointsbasis device, so this should be visible
		//	at least when the device is online
		for (auto& expander : jointSelectorExpanders)
			expander.get()->ContainerExpander().get()->Visibility(
				(device_status.find(L"S_OK") != std::wstring::npos &&
					selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);
		
		jointBasisLabel.get()->Visibility(
			(device_status.find(L"S_OK") != std::wstring::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);
		
		// Set up combos if the device's OK
		if (device_status.find(L"S_OK") != std::wstring::npos)
		{
			// If we're reconnecting a base device, also refresh joints
			if (selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
			{
				for (auto& expander : jointSelectorExpanders)
					expander.get()->ReAppendTrackers();
			}
			// If we're reconnecting an override device, also refresh joints
			else if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
			{
				// Clear items
				// Waist
				devices_clear_combo(waistPositionOverrideOptionBox);
				devices_clear_combo(waistRotationOverrideOptionBox);

				// LeftF
				devices_clear_combo(leftFootPositionOverrideOptionBox);
				devices_clear_combo(leftFootRotationOverrideOptionBox);

				// RightF
				devices_clear_combo(rightFootPositionOverrideOptionBox);
				devices_clear_combo(rightFootRotationOverrideOptionBox);

				// LeftEL
				devices_clear_combo(leftElbowPositionOverrideOptionBox);
				devices_clear_combo(leftElbowRotationOverrideOptionBox);

				// RightEL
				devices_clear_combo(rightElbowPositionOverrideOptionBox);
				devices_clear_combo(rightElbowRotationOverrideOptionBox);

				// LeftK
				devices_clear_combo(leftKneePositionOverrideOptionBox);
				devices_clear_combo(leftKneeRotationOverrideOptionBox);

				// RightK
				devices_clear_combo(rightKneePositionOverrideOptionBox);
				devices_clear_combo(rightKneeRotationOverrideOptionBox);

				// Append all joints to all combos
				for (auto& _joint : device->getTrackedJoints())
				{
					// Get the name into string
					auto _jointname = _joint.getJointName();

					// Push the name to all combos
					devices_push_override_joints(StringToWString(_jointname));
				}

				// Try fix override IDs if wrong
				TrackingDevices::devices_check_override_ids(selectedTrackingDeviceID);

				// Select the first (or next, if exists) joint
				// Set the placeholder text on disabled combos
				devices_select_combobox();
			}
		}
	}

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	// Update the status here
	const bool status_ok = device_status.find(L"S_OK") != std::wstring::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(split_status(device_status)[0]);
	trackingDeviceErrorLabel.get()->Text(split_status(device_status)[1]);
	errorWhatText.get()->Text(split_status(device_status)[2]);

	if (selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
	{
		LOG(INFO) << "Selected a base";
		setAsOverrideButton.get()->IsEnabled(false);
		setAsBaseButton.get()->IsEnabled(false);

		overridesLabel.get()->Visibility(Visibility::Collapsed);
		overridesControls.get()->Visibility(Visibility::Collapsed);
		overridesControls_1.get()->Visibility(Visibility::Collapsed);
		overridesDropDown.get()->Visibility(Visibility::Collapsed);
		overridesDropDown_1.get()->Visibility(Visibility::Collapsed);
		deselectDeviceButton.get()->Visibility(Visibility::Collapsed);
	}
	else if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
	{
		LOG(INFO) << "Selected an override";
		setAsOverrideButton.get()->IsEnabled(false);
		setAsBaseButton.get()->IsEnabled(true);

		overridesLabel.get()->Visibility(Visibility::Visible);
		overridesControls.get()->Visibility(Visibility::Visible);
		overridesControls_1.get()->Visibility(Visibility::Visible);
		overridesDropDown.get()->Visibility(Visibility::Visible);
		overridesDropDown_1.get()->Visibility(Visibility::Visible);
		deselectDeviceButton.get()->Visibility(Visibility::Visible);
	}
	else
	{
		LOG(INFO) << "Selected a [none]";
		setAsOverrideButton.get()->IsEnabled(true);
		setAsBaseButton.get()->IsEnabled(true);

		overridesLabel.get()->Visibility(Visibility::Collapsed);
		overridesControls.get()->Visibility(Visibility::Collapsed);
		overridesControls_1.get()->Visibility(Visibility::Collapsed);
		overridesDropDown.get()->Visibility(Visibility::Collapsed);
		overridesDropDown_1.get()->Visibility(Visibility::Collapsed);
		deselectDeviceButton.get()->Visibility(Visibility::Collapsed);
	}

	{
		// Remove the only one child of our outer main content grid
		// (What a bestiality it is to do that!!1)
		devicesMainContentGridOuter.get()->Children().RemoveAtEnd();

		Media::Animation::EntranceThemeTransition t;
		t.IsStaggeringEnabled(true);

		devicesMainContentGridInner.get()->Transitions().Append(t);

		// Sleep peacefully pretending that noting happened
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(10); // 10ms for slower systems
		co_await ui_thread;

		// Re-add the child for it to play our funky transition
		// (Though it's not the same as before...)
		devicesMainContentGridOuter.get()->Children().
		                            Append(*devicesMainContentGridInner);
	}

	LOG(INFO) << "Changed the currently selected device to " << deviceName;

	// Remove the transition
	apartment_context ui_thread;
	co_await resume_background();
	Sleep(100);
	co_await ui_thread;

	devicesMainContentGridInner.get()->Transitions().Clear();
}


void KinectToVR::implementation::DevicesPage::ReconnectDeviceButton_Click(
	const Controls::SplitButton& sender,
	const Controls::SplitButtonClickEventArgs& args)
{
	auto _index = devicesListView.get()->SelectedIndex();

	auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(_index);
	std::wstring device_status = L"Something's wrong!\nE_UKNOWN\nWhat's happened here?";
	LOG(INFO) << "Now reconnecting the tracking device...";

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto& device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

		device->initialize();
		device_status = device->statusResultWString(device->getStatusResult());

		// We've selected a kinectbasis device, so this should be hidden
		for (auto& expander : jointSelectorExpanders)
			expander.get()->ContainerExpander().get()->Visibility(Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(Visibility::Collapsed);

		// Set up combos if the device's OK
		if (device_status.find(L"S_OK") != std::wstring::npos)
		{
			// If we're reconnecting an override device, also refresh joints
			if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
			{
				// Clear items
				// Waist
				devices_clear_combo(waistPositionOverrideOptionBox);
				devices_clear_combo(waistRotationOverrideOptionBox);

				// LeftF
				devices_clear_combo(leftFootPositionOverrideOptionBox);
				devices_clear_combo(leftFootRotationOverrideOptionBox);

				// RightF
				devices_clear_combo(rightFootPositionOverrideOptionBox);
				devices_clear_combo(rightFootRotationOverrideOptionBox);

				// LeftEL
				devices_clear_combo(leftElbowPositionOverrideOptionBox);
				devices_clear_combo(leftElbowRotationOverrideOptionBox);

				// RightEL
				devices_clear_combo(rightElbowPositionOverrideOptionBox);
				devices_clear_combo(rightElbowRotationOverrideOptionBox);

				// LeftK
				devices_clear_combo(leftKneePositionOverrideOptionBox);
				devices_clear_combo(leftKneeRotationOverrideOptionBox);

				// RightK
				devices_clear_combo(rightKneePositionOverrideOptionBox);
				devices_clear_combo(rightKneeRotationOverrideOptionBox);

				// Append all joints to all combos, depend on characteristics
				switch (device->getDeviceCharacteristics())
				{
				case ktvr::K2_Character_Basic:
					{
						devices_push_override_joints(false);
					}
					break;
				case ktvr::K2_Character_Simple:
					{
						devices_push_override_joints();
					}
					break;
				case ktvr::K2_Character_Full:
					{
						devices_push_override_joints();
					}
					break;
				}

				// Try fix override IDs if wrong
				TrackingDevices::devices_check_override_ids(selectedTrackingDeviceID);

				// Select the first (or next, if exists) joint
				// Set the placeholder text on disabled combos
				devices_select_combobox();
			}
		}
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto& device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

		device->initialize();
		device_status = device->statusResultWString(device->getStatusResult());

		// We've selected a jointsbasis device, so this should be visible
		//	at least when the device is online
		for (auto& expander : jointSelectorExpanders)
			expander.get()->ContainerExpander().get()->Visibility(
				(device_status.find(L"S_OK") != std::wstring::npos &&
					selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);
		
		jointBasisLabel.get()->Visibility(
			(device_status.find(L"S_OK") != std::wstring::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);
		
		// Set up combos if the device's OK
		if (device_status.find(L"S_OK") != std::wstring::npos)
		{
			// If we're reconnecting a base device, also refresh joints
			if (selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
			{
				for (auto& expander : jointSelectorExpanders)
					expander.get()->ReAppendTrackers();
			}
			// If we're reconnecting an override device, also refresh joints
			else if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
			{
				// Clear items
				// Waist
				devices_clear_combo(waistPositionOverrideOptionBox);
				devices_clear_combo(waistRotationOverrideOptionBox);

				// LeftF
				devices_clear_combo(leftFootPositionOverrideOptionBox);
				devices_clear_combo(leftFootRotationOverrideOptionBox);

				// RightF
				devices_clear_combo(rightFootPositionOverrideOptionBox);
				devices_clear_combo(rightFootRotationOverrideOptionBox);

				// LeftEL
				devices_clear_combo(leftElbowPositionOverrideOptionBox);
				devices_clear_combo(leftElbowRotationOverrideOptionBox);

				// RightEL
				devices_clear_combo(rightElbowPositionOverrideOptionBox);
				devices_clear_combo(rightElbowRotationOverrideOptionBox);

				// LeftK
				devices_clear_combo(leftKneePositionOverrideOptionBox);
				devices_clear_combo(leftKneeRotationOverrideOptionBox);

				// RightK
				devices_clear_combo(rightKneePositionOverrideOptionBox);
				devices_clear_combo(rightKneeRotationOverrideOptionBox);

				// Append all joints to all combos
				for (auto& _joint : device->getTrackedJoints())
				{
					// Get the name into string
					auto _jointname = _joint.getJointName();

					// Push the name to all combos
					devices_push_override_joints(StringToWString(_jointname));
				}

				// Try fix override IDs if wrong
				TrackingDevices::devices_check_override_ids(selectedTrackingDeviceID);

				// Select the first (or next, if exists) joint
				// Set the placeholder text on disabled combos
				devices_select_combobox();
			}
		}
	}

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	/* Update local statuses */

	// Update the status here
	const bool status_ok = device_status.find(L"S_OK") != std::wstring::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(split_status(device_status)[0]);
	trackingDeviceErrorLabel.get()->Text(split_status(device_status)[1]);
	errorWhatText.get()->Text(split_status(device_status)[2]);

	// Update the GeneralPage status
	TrackingDevices::updateTrackingDeviceUI(k2app::K2Settings.trackingDeviceID);
	TrackingDevices::updateOverrideDeviceUI(k2app::K2Settings.overrideDeviceID); // Auto-handles if none
}

// *Nearly* the same as reconnect
void KinectToVR::implementation::DevicesPage::DisconnectDeviceButton_Click(
	const Windows::Foundation::IInspectable& sender,
	const RoutedEventArgs& e)
{
	auto _index = devicesListView.get()->SelectedIndex();

	auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(_index);
	std::wstring device_status = L"Something's wrong!\nE_UKNOWN\nWhat's happened here?";
	LOG(INFO) << "Now disconnecting the tracking device...";

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto& device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

		device->shutdown();
		device_status = device->statusResultWString(device->getStatusResult());

		// We've selected a kinectbasis device, so this should be hidden
		for (auto& expander : jointSelectorExpanders)
			expander.get()->ContainerExpander().get()->Visibility(Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(Visibility::Collapsed);
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto& device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

		device->shutdown();
		device_status = device->statusResultWString(device->getStatusResult());

		// We've selected a jointsbasis device, so this should be visible
		//	at least when the device is online
		for (auto& expander : jointSelectorExpanders)
			expander.get()->ContainerExpander().get()->Visibility(
				(device_status.find(L"S_OK") != std::wstring::npos &&
					selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);
		
		jointBasisLabel.get()->Visibility(
			(device_status.find(L"S_OK") != std::wstring::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);
	}

	/* Update local statuses */

	// Update the status here
	const bool status_ok = device_status.find(L"S_OK") != std::wstring::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(split_status(device_status)[0]);
	trackingDeviceErrorLabel.get()->Text(split_status(device_status)[1]);
	errorWhatText.get()->Text(split_status(device_status)[2]);

	// Update the GeneralPage status
	TrackingDevices::updateTrackingDeviceUI(k2app::K2Settings.trackingDeviceID);
	TrackingDevices::updateOverrideDeviceUI(k2app::K2Settings.overrideDeviceID); // Auto-handles if none

	AlternativeConnectionOptionsFlyout().Hide();
}

// Mark override device as -1 -> deselect it
void KinectToVR::implementation::DevicesPage::DeselectDeviceButton_Click(
	const Windows::Foundation::IInspectable& sender,
	const RoutedEventArgs& e)
{
	auto _index = devicesListView.get()->SelectedIndex();

	auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(_index);
	std::wstring device_status = L"Something's wrong!\nE_UKNOWN\nWhat's happened here?";
	LOG(INFO) << "Now deselecting the tracking device...";

	for (auto& expander : jointSelectorExpanders)
		expander.get()->ContainerExpander().get()->Visibility(Visibility::Collapsed);

	jointBasisLabel.get()->Visibility(Visibility::Collapsed);

	setAsOverrideButton.get()->IsEnabled(true);
	setAsBaseButton.get()->IsEnabled(true);

	overridesLabel.get()->Visibility(Visibility::Collapsed);
	overridesControls.get()->Visibility(Visibility::Collapsed);
	overridesControls_1.get()->Visibility(Visibility::Collapsed);
	overridesDropDown.get()->Visibility(Visibility::Collapsed);
	overridesDropDown_1.get()->Visibility(Visibility::Collapsed);
	deselectDeviceButton.get()->Visibility(Visibility::Collapsed);

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto& device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);
		device_status = device->statusResultWString(device->getStatusResult());
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto& device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
		device_status = device->statusResultWString(device->getStatusResult());
	}

	/* Update local statuses */

	// Update the status here
	const bool status_ok = device_status.find(L"S_OK") != std::wstring::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(split_status(device_status)[0]);
	trackingDeviceErrorLabel.get()->Text(split_status(device_status)[1]);
	errorWhatText.get()->Text(split_status(device_status)[2]);

	// Deselect the device
	k2app::K2Settings.overrideDeviceID = -1; // Only acceptable for an Override
	TrackingDevices::updateOverrideDeviceUI(k2app::K2Settings.overrideDeviceID);

	// Also update in the UI
	overrideDeviceName.get()->Text(
		k2app::interfacing::LocalizedResourceWString(L"DevicesPage", L"Titles/NoOverrides/Text"));

	// Save settings
	k2app::K2Settings.saveSettings();

	AlternativeConnectionOptionsFlyout().Hide();
}


Windows::Foundation::IAsyncAction KinectToVR::implementation::DevicesPage::SetAsOverrideButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	const auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(selectedTrackingDeviceID);

	std::wstring device_status = L"Something's wrong!\nE_UKNOWN\nWhat's happened here?";
	std::string deviceName = "[UNKNOWN]";

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);
		deviceName = device->getDeviceName();
		device->initialize(); // Init the device as we'll be using it

		// Also refresh joints

		// Clear items
		// Waist
		devices_clear_combo(waistPositionOverrideOptionBox);
		devices_clear_combo(waistRotationOverrideOptionBox);

		// LeftF
		devices_clear_combo(leftFootPositionOverrideOptionBox);
		devices_clear_combo(leftFootRotationOverrideOptionBox);

		// RightF
		devices_clear_combo(rightFootPositionOverrideOptionBox);
		devices_clear_combo(rightFootRotationOverrideOptionBox);

		// LeftEL
		devices_clear_combo(leftElbowPositionOverrideOptionBox);
		devices_clear_combo(leftElbowRotationOverrideOptionBox);

		// RightEL
		devices_clear_combo(rightElbowPositionOverrideOptionBox);
		devices_clear_combo(rightElbowRotationOverrideOptionBox);

		// LeftK
		devices_clear_combo(leftKneePositionOverrideOptionBox);
		devices_clear_combo(leftKneeRotationOverrideOptionBox);

		// RightK
		devices_clear_combo(rightKneePositionOverrideOptionBox);
		devices_clear_combo(rightKneeRotationOverrideOptionBox);

		// Append all joints to all combos, depend on characteristics
		switch (device->getDeviceCharacteristics())
		{
		case ktvr::K2_Character_Basic:
			{
				devices_push_override_joints(false);
			}
			break;
		case ktvr::K2_Character_Simple:
			{
				devices_push_override_joints();
			}
			break;
		case ktvr::K2_Character_Full:
			{
				devices_push_override_joints();
			}
			break;
		}

		// Try fix override IDs if wrong
		TrackingDevices::devices_check_override_ids(selectedTrackingDeviceID);

		// Select the first (or next, if exists) joint
		// Set the placeholder text on disabled combos
		devices_select_combobox();

		// Backup the status
		device_status = device->statusResultWString(device->getStatusResult());

		// We've selected a kinectbasis device, so this should be hidden
		for (auto& expander : jointSelectorExpanders)
			expander.get()->ContainerExpander().get()->Visibility(Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(Visibility::Collapsed);
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
		deviceName = device->getDeviceName();
		device->initialize(); // Init the device as we'll be using it

		// Abort if there are no joints
		if (device->getTrackedJoints().empty())
		{
			NoJointsFlyout().ShowAt(SelectedDeviceNameLabel());
			co_return; // Don't set up any overrides (yet)
		}

		// Also refresh joints

		// Clear items
		// Waist
		devices_clear_combo(waistPositionOverrideOptionBox);
		devices_clear_combo(waistRotationOverrideOptionBox);

		// LeftF
		devices_clear_combo(leftFootPositionOverrideOptionBox);
		devices_clear_combo(leftFootRotationOverrideOptionBox);

		// RightF
		devices_clear_combo(rightFootPositionOverrideOptionBox);
		devices_clear_combo(rightFootRotationOverrideOptionBox);

		// LeftEL
		devices_clear_combo(leftElbowPositionOverrideOptionBox);
		devices_clear_combo(leftElbowRotationOverrideOptionBox);

		// RightEL
		devices_clear_combo(rightElbowPositionOverrideOptionBox);
		devices_clear_combo(rightElbowRotationOverrideOptionBox);

		// LeftK
		devices_clear_combo(leftKneePositionOverrideOptionBox);
		devices_clear_combo(leftKneeRotationOverrideOptionBox);

		// RightK
		devices_clear_combo(rightKneePositionOverrideOptionBox);
		devices_clear_combo(rightKneeRotationOverrideOptionBox);

		// Append all joints to all combos
		for (auto& _joint : device->getTrackedJoints())
		{
			// Get the name into string
			auto _jointname = _joint.getJointName();

			// Push the name to all combos
			devices_push_override_joints(StringToWString(_jointname));
		}

		// Try fix override IDs if wrong
		TrackingDevices::devices_check_override_ids(selectedTrackingDeviceID);

		// Select the first (or next, if exists) joint
		// Set the placeholder text on disabled combos
		devices_select_combobox();

		// Backup the status
		device_status = device->statusResultWString(device->getStatusResult());

		// We've selected an override device, so this should be hidden
		for (auto& expander : jointSelectorExpanders)
			expander.get()->ContainerExpander().get()->Visibility(Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(Visibility::Collapsed);
	}

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	/* Update local statuses */
	setAsOverrideButton.get()->IsEnabled(false);
	setAsBaseButton.get()->IsEnabled(true);
	SetDeviceTypeFlyout().Hide(); // Hide the flyout

	overrideDeviceName.get()->Text(StringToWString(deviceName));

	LOG(INFO) << "Changed the current tracking device (Override) to " << deviceName;

	overridesLabel.get()->Visibility(Visibility::Visible);
	overridesControls.get()->Visibility(Visibility::Visible);
	overridesControls_1.get()->Visibility(Visibility::Visible);
	overridesDropDown.get()->Visibility(Visibility::Visible);
	overridesDropDown_1.get()->Visibility(Visibility::Visible);
	deselectDeviceButton.get()->Visibility(Visibility::Visible);

	// Register and etc
	/* Update local statuses */

	// Update the status here
	const bool status_ok = device_status.find(L"S_OK") != std::wstring::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(split_status(device_status)[0]);
	trackingDeviceErrorLabel.get()->Text(split_status(device_status)[1]);
	errorWhatText.get()->Text(split_status(device_status)[2]);

	k2app::K2Settings.overrideDeviceID = selectedTrackingDeviceID;
	TrackingDevices::updateOverrideDeviceUI(k2app::K2Settings.overrideDeviceID);

	{
		// Remove the only one child of our outer main content grid
		// (What a bestiality it is to do that!!1)
		devicesOverridesSelectorStackPanelOuter.get()->Children().RemoveAtEnd();

		Media::Animation::EntranceThemeTransition t;
		t.IsStaggeringEnabled(true);

		devicesOverridesSelectorStackPanelInner.get()->Transitions().Append(t);

		// Sleep peacefully pretending that noting happened
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(10); // 10ms for slower systems
		co_await ui_thread;

		// Re-add the child for it to play our funky transition
		// (Though it's not the same as before...)
		devicesOverridesSelectorStackPanelOuter.get()->Children().
		                                        Append(*devicesOverridesSelectorStackPanelInner);
	}

	// Save settings
	k2app::K2Settings.saveSettings();

	// Remove the transition
	apartment_context ui_thread;
	co_await resume_background();
	Sleep(100);
	co_await ui_thread;

	devicesOverridesSelectorStackPanelInner.get()->Transitions().Clear();
}


Windows::Foundation::IAsyncAction KinectToVR::implementation::DevicesPage::SetAsBaseButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	const auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(selectedTrackingDeviceID);

	std::wstring device_status = L"Something's wrong!\nE_UKNOWN\nWhat's happened here?";
	std::string deviceName = "[UNKNOWN]";

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);
		deviceName = device->getDeviceName();

		device->initialize(); // Init the device as we'll be using it
		device_status = device->statusResultWString(device->getStatusResult());

		// We've selected a kinectbasis device, so this should be hidden
		for (auto& expander : jointSelectorExpanders)
			expander.get()->ContainerExpander().get()->Visibility(Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(Visibility::Collapsed);
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
		deviceName = device->getDeviceName();
		device->initialize(); // Init the device as we'll be using it

		// Abort if there are no joints
		if (device->getTrackedJoints().empty())
		{
			NoJointsFlyout().ShowAt(SelectedDeviceNameLabel());
			co_return; // Don't set up any overrides (yet)
		}

		// Also refresh joints
		for (auto& expander : jointSelectorExpanders)
			expander.get()->ReAppendTrackers();
		
		// Update the status
		device_status = device->statusResultWString(device->getStatusResult());
		jointBasisLabel.get()->Visibility(
			(device_status.find(L"S_OK") != std::wstring::npos) ? Visibility::Visible : Visibility::Collapsed);
	}

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	/* Update local statuses */
	setAsOverrideButton.get()->IsEnabled(false);
	setAsBaseButton.get()->IsEnabled(false);
	SetDeviceTypeFlyout().Hide(); // Hide the flyout

	baseDeviceName.get()->Text(StringToWString(deviceName));
	if (overrideDeviceName.get()->Text() == StringToWString(deviceName))
		overrideDeviceName.get()->Text(
			k2app::interfacing::LocalizedResourceWString(L"DevicesPage", L"Titles/NoOverrides/Text"));

	LOG(INFO) << "Changed the current tracking device (Base) to " << deviceName;

	overridesLabel.get()->Visibility(Visibility::Collapsed);
	overridesControls.get()->Visibility(Visibility::Collapsed);
	overridesControls_1.get()->Visibility(Visibility::Collapsed);
	overridesDropDown.get()->Visibility(Visibility::Collapsed);
	overridesDropDown_1.get()->Visibility(Visibility::Collapsed);
	deselectDeviceButton.get()->Visibility(Visibility::Collapsed);

	// Register and etc
	/* Update local statuses */

	// Update the status here
	const bool status_ok = device_status.find(L"S_OK") != std::wstring::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(split_status(device_status)[0]);
	trackingDeviceErrorLabel.get()->Text(split_status(device_status)[1]);
	errorWhatText.get()->Text(split_status(device_status)[2]);

	k2app::K2Settings.trackingDeviceID = selectedTrackingDeviceID;
	if (k2app::K2Settings.overrideDeviceID == k2app::K2Settings.trackingDeviceID)
		k2app::K2Settings.overrideDeviceID = -1; // Reset the override

	TrackingDevices::updateTrackingDeviceUI(k2app::K2Settings.trackingDeviceID);

	// This is here too cause an override might've became a base... -_-
	TrackingDevices::updateOverrideDeviceUI(k2app::K2Settings.overrideDeviceID); // Auto-handles if none

	// If controls are set to be visible
	if (jointSelectorExpanders[0].get()->ContainerExpander().get()->Visibility() == Visibility::Visible)
	{
		// Remove the only one child of our outer main content grid
		// (What a bestiality it is to do that!!1)
		devicesJointsBasisSelectorStackPanelOuter.get()->Children().RemoveAtEnd();

		Media::Animation::EntranceThemeTransition t;
		t.IsStaggeringEnabled(true);

		devicesJointsBasisSelectorStackPanelInner.get()->Transitions().Append(t);

		// Sleep peacefully pretending that noting happened
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(10); // 10ms for slower systems
		co_await ui_thread;

		// Re-add the child for it to play our funky transition
		// (Though it's not the same as before...)
		devicesJointsBasisSelectorStackPanelOuter.get()->Children().
		                                          Append(*devicesJointsBasisSelectorStackPanelInner);
	}

	// Save settings
	k2app::K2Settings.saveSettings();

	// Remove the transition
	apartment_context ui_thread;
	co_await resume_background();
	Sleep(100);
	co_await ui_thread;

	devicesJointsBasisSelectorStackPanelInner.get()->Transitions().Clear();
}

/* For *Override* device type: position & rotation joints selector */

void KinectToVR::implementation::DevicesPage::WaistPositionOverrideOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.K2TrackersVector[0].isPositionOverridden &&
		waistPositionOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.K2TrackersVector[0].positionOverrideJointID = waistPositionOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	const auto& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1 && devices_tab_re_setup_finished) // if JointsBasis & Setup Finished
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.K2TrackersVector[0].positionOverrideJointID);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::WaistRotationOverrideOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.K2TrackersVector[0].isRotationOverridden &&
		waistRotationOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.K2TrackersVector[0].rotationOverrideJointID = waistRotationOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	const auto& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1 && devices_tab_re_setup_finished) // if JointsBasis & Setup Finished
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.K2TrackersVector[0].rotationOverrideJointID);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::LeftFootPositionOverrideOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.K2TrackersVector[1].isPositionOverridden &&
		leftFootPositionOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.K2TrackersVector[1].positionOverrideJointID = leftFootPositionOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	const auto& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1 && devices_tab_re_setup_finished) // if JointsBasis & Setup Finished
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.K2TrackersVector[1].positionOverrideJointID);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::LeftFootRotationOverrideOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.K2TrackersVector[1].isRotationOverridden &&
		leftFootRotationOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.K2TrackersVector[1].rotationOverrideJointID = leftFootRotationOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	const auto& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1 && devices_tab_re_setup_finished) // if JointsBasis & Setup Finished
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.K2TrackersVector[1].rotationOverrideJointID);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::RightFootPositionOverrideOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.K2TrackersVector[2].isPositionOverridden &&
		rightFootPositionOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.K2TrackersVector[2].positionOverrideJointID = rightFootPositionOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	const auto& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1 && devices_tab_re_setup_finished) // if JointsBasis & Setup Finished
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.K2TrackersVector[2].positionOverrideJointID);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::RightFootRotationOverrideOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.K2TrackersVector[2].isRotationOverridden &&
		rightFootRotationOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.K2TrackersVector[2].rotationOverrideJointID = rightFootRotationOverrideOptionBox.get()->SelectedIndex();

	//// If we're using a joints device then also signal the joint
	const auto& trackingDevice = TrackingDevices::getCurrentOverrideDevice();
	if (trackingDevice.index() == 1 && devices_tab_re_setup_finished) // if JointsBasis & Setup Finished
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice)->
			signalJoint(k2app::K2Settings.K2TrackersVector[2].rotationOverrideJointID);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::LeftElbowPositionOverrideOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.K2TrackersVector[3].isPositionOverridden &&
		leftElbowPositionOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.K2TrackersVector[3].positionOverrideJointID = leftElbowPositionOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	const auto& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1 && devices_tab_re_setup_finished) // if JointsBasis & Setup Finished
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.K2TrackersVector[3].positionOverrideJointID);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::LeftElbowRotationOverrideOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.K2TrackersVector[3].isRotationOverridden &&
		leftElbowRotationOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.K2TrackersVector[3].rotationOverrideJointID = leftElbowRotationOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	const auto& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1 && devices_tab_re_setup_finished) // if JointsBasis & Setup Finished
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.K2TrackersVector[3].rotationOverrideJointID);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::RightElbowPositionOverrideOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.K2TrackersVector[4].isPositionOverridden &&
		rightElbowPositionOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.K2TrackersVector[4].positionOverrideJointID = rightElbowPositionOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	const auto& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1 && devices_tab_re_setup_finished) // if JointsBasis & Setup Finished
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.K2TrackersVector[4].positionOverrideJointID);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::RightElbowRotationOverrideOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.K2TrackersVector[4].isRotationOverridden &&
		rightElbowRotationOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.K2TrackersVector[4].rotationOverrideJointID = rightElbowRotationOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	const auto& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1 && devices_tab_re_setup_finished) // if JointsBasis & Setup Finished
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.K2TrackersVector[4].rotationOverrideJointID);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::LeftKneePositionOverrideOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.K2TrackersVector[5].isPositionOverridden &&
		leftKneePositionOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.K2TrackersVector[5].positionOverrideJointID = leftKneePositionOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	const auto& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1 && devices_tab_re_setup_finished) // if JointsBasis & Setup Finished
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.K2TrackersVector[5].positionOverrideJointID);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::LeftKneeRotationOverrideOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.K2TrackersVector[5].isRotationOverridden &&
		leftKneeRotationOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.K2TrackersVector[5].rotationOverrideJointID = leftKneeRotationOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	const auto& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1 && devices_tab_re_setup_finished) // if JointsBasis & Setup Finished
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.K2TrackersVector[5].rotationOverrideJointID);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::RightKneePositionOverrideOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.K2TrackersVector[6].isPositionOverridden &&
		rightKneePositionOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.K2TrackersVector[6].positionOverrideJointID = rightKneePositionOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	const auto& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1 && devices_tab_re_setup_finished) // if JointsBasis & Setup Finished
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.K2TrackersVector[6].positionOverrideJointID);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::RightKneeRotationOverrideOptionBox_SelectionChanged(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SelectionChangedEventArgs& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.K2TrackersVector[6].isRotationOverridden &&
		rightKneeRotationOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.K2TrackersVector[6].rotationOverrideJointID = rightKneeRotationOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	const auto& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1 && devices_tab_re_setup_finished) // if JointsBasis & Setup Finished
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.K2TrackersVector[6].rotationOverrideJointID);

	// Save settings
	k2app::K2Settings.saveSettings();
}

/* For *Override* device type: override elements for joints selector */

void KinectToVR::implementation::DevicesPage::OverrideWaistPosition_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
			TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
	{
		OverrideWaistPosition().IsChecked(false);
		NoJointsFlyout().ShowAt(OverridesLabel());
		return; // Don't set up any overrides (yet)
	}

	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.K2TrackersVector[0].isPositionOverridden = overrideWaistPosition.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	waistPositionOverrideOptionBox.get()->SelectedIndex(
		overrideWaistPosition.get()->IsChecked() ? k2app::K2Settings.K2TrackersVector[0].positionOverrideJointID : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::OverrideWaistRotation_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
			TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
	{
		OverrideWaistRotation().IsChecked(false);
		NoJointsFlyout().ShowAt(OverridesLabel());
		return; // Don't set up any overrides (yet)
	}

	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.K2TrackersVector[0].isRotationOverridden = overrideWaistRotation.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	waistRotationOverrideOptionBox.get()->SelectedIndex(
		overrideWaistRotation.get()->IsChecked() ? k2app::K2Settings.K2TrackersVector[0].rotationOverrideJointID : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::OverrideLeftFootPosition_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
			TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
	{
		OverrideLeftFootPosition().IsChecked(false);
		NoJointsFlyout().ShowAt(OverridesLabel());
		return; // Don't set up any overrides (yet)
	}

	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.K2TrackersVector[1].isPositionOverridden = overrideLeftFootPosition.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	leftFootPositionOverrideOptionBox.get()->SelectedIndex(
		overrideLeftFootPosition.get()->IsChecked() ? k2app::K2Settings.K2TrackersVector[1].positionOverrideJointID : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::OverrideLeftFootRotation_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
			TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
	{
		OverrideLeftFootRotation().IsChecked(false);
		NoJointsFlyout().ShowAt(OverridesLabel());
		return; // Don't set up any overrides (yet)
	}

	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.K2TrackersVector[1].isRotationOverridden = overrideLeftFootRotation.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	leftFootRotationOverrideOptionBox.get()->SelectedIndex(
		overrideLeftFootRotation.get()->IsChecked() ? k2app::K2Settings.K2TrackersVector[1].rotationOverrideJointID : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::OverrideRightFootPosition_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
			TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
	{
		OverrideRightFootPosition().IsChecked(false);
		NoJointsFlyout().ShowAt(OverridesLabel());
		return; // Don't set up any overrides (yet)
	}

	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.K2TrackersVector[2].isPositionOverridden = overrideRightFootPosition.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	rightFootPositionOverrideOptionBox.get()->SelectedIndex(
		overrideRightFootPosition.get()->IsChecked() ? k2app::K2Settings.K2TrackersVector[2].positionOverrideJointID : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::OverrideRightFootRotation_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
			TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
	{
		OverrideRightFootRotation().IsChecked(false);
		NoJointsFlyout().ShowAt(OverridesLabel());
		return; // Don't set up any overrides (yet)
	}

	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.K2TrackersVector[2].isRotationOverridden = overrideRightFootRotation.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	rightFootRotationOverrideOptionBox.get()->SelectedIndex(
		overrideRightFootRotation.get()->IsChecked() ? k2app::K2Settings.K2TrackersVector[2].rotationOverrideJointID : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::OverrideLeftElbowPosition_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
			TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
	{
		OverrideLeftElbowPosition().IsChecked(false);
		NoJointsFlyout().ShowAt(OverridesLabel());
		return; // Don't set up any overrides (yet)
	}

	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.K2TrackersVector[3].isPositionOverridden = overrideLeftElbowPosition.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	leftElbowPositionOverrideOptionBox.get()->SelectedIndex(
		overrideLeftElbowPosition.get()->IsChecked() ? k2app::K2Settings.K2TrackersVector[3].positionOverrideJointID : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::OverrideLeftElbowRotation_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
			TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
	{
		OverrideLeftElbowRotation().IsChecked(false);
		NoJointsFlyout().ShowAt(OverridesLabel());
		return; // Don't set up any overrides (yet)
	}

	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.K2TrackersVector[3].isRotationOverridden = overrideLeftElbowRotation.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	leftElbowRotationOverrideOptionBox.get()->SelectedIndex(
		overrideLeftElbowRotation.get()->IsChecked() ? k2app::K2Settings.K2TrackersVector[3].rotationOverrideJointID : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::OverrideRightElbowPosition_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
			TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
	{
		OverrideRightElbowPosition().IsChecked(false);
		NoJointsFlyout().ShowAt(OverridesLabel());
		return; // Don't set up any overrides (yet)
	}

	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.K2TrackersVector[4].isPositionOverridden = overrideRightElbowPosition.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	rightElbowPositionOverrideOptionBox.get()->SelectedIndex(
		overrideRightElbowPosition.get()->IsChecked() ? k2app::K2Settings.K2TrackersVector[4].positionOverrideJointID : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::OverrideRightElbowRotation_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
			TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
	{
		OverrideRightElbowRotation().IsChecked(false);
		NoJointsFlyout().ShowAt(OverridesLabel());
		return; // Don't set up any overrides (yet)
	}

	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.K2TrackersVector[4].isRotationOverridden = overrideRightElbowRotation.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	rightElbowRotationOverrideOptionBox.get()->SelectedIndex(
		overrideRightElbowRotation.get()->IsChecked() ? k2app::K2Settings.K2TrackersVector[4].rotationOverrideJointID : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::OverrideLeftKneePosition_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
			TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
	{
		OverrideLeftKneePosition().IsChecked(false);
		NoJointsFlyout().ShowAt(OverridesLabel());
		return; // Don't set up any overrides (yet)
	}

	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.K2TrackersVector[5].isPositionOverridden = overrideLeftKneePosition.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	leftKneePositionOverrideOptionBox.get()->SelectedIndex(
		overrideLeftKneePosition.get()->IsChecked() ? k2app::K2Settings.K2TrackersVector[5].positionOverrideJointID : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::OverrideLeftKneeRotation_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
			TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
	{
		OverrideLeftKneeRotation().IsChecked(false);
		NoJointsFlyout().ShowAt(OverridesLabel());
		return; // Don't set up any overrides (yet)
	}

	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.K2TrackersVector[5].isRotationOverridden = overrideLeftKneeRotation.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	leftKneeRotationOverrideOptionBox.get()->SelectedIndex(
		overrideLeftKneeRotation.get()->IsChecked() ? k2app::K2Settings.K2TrackersVector[5].rotationOverrideJointID : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::OverrideRightKneePosition_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
			TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
	{
		OverrideRightKneePosition().IsChecked(false);
		NoJointsFlyout().ShowAt(OverridesLabel());
		return; // Don't set up any overrides (yet)
	}

	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.K2TrackersVector[6].isPositionOverridden = overrideRightKneePosition.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	rightKneePositionOverrideOptionBox.get()->SelectedIndex(
		overrideRightKneePosition.get()->IsChecked() ? k2app::K2Settings.K2TrackersVector[6].positionOverrideJointID : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void KinectToVR::implementation::DevicesPage::OverrideRightKneeRotation_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (TrackingDevices::getCurrentOverrideDevice().index() == 1 &&
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(
			TrackingDevices::getCurrentOverrideDevice())->getTrackedJoints().empty())
	{
		OverrideRightKneeRotation().IsChecked(false);
		NoJointsFlyout().ShowAt(OverridesLabel());
		return; // Don't set up any overrides (yet)
	}

	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	k2app::K2Settings.K2TrackersVector[6].isRotationOverridden = overrideRightKneeRotation.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	rightKneeRotationOverrideOptionBox.get()->SelectedIndex(
		overrideRightKneeRotation.get()->IsChecked() ? k2app::K2Settings.K2TrackersVector[6].rotationOverrideJointID : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


/* For comboboxes: update before opening */


void KinectToVR::implementation::DevicesPage::WaistPositionOverrideOptionBox_DropDownOpened(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
}


void KinectToVR::implementation::DevicesPage::WaistRotationOverrideOptionBox_DropDownOpened(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
}


void KinectToVR::implementation::DevicesPage::LeftFootPositionOverrideOptionBox_DropDownOpened(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
}


void KinectToVR::implementation::DevicesPage::LeftFootRotationOverrideOptionBox_DropDownOpened(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
}


void KinectToVR::implementation::DevicesPage::RightFootPositionOverrideOptionBox_DropDownOpened(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
}


void KinectToVR::implementation::DevicesPage::RightFootRotationOverrideOptionBox_DropDownOpened(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
}


void KinectToVR::implementation::DevicesPage::LeftElbowPositionOverrideOptionBox_DropDownOpened(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
}


void KinectToVR::implementation::DevicesPage::LeftElbowRotationOverrideOptionBox_DropDownOpened(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
}


void KinectToVR::implementation::DevicesPage::RightElbowPositionOverrideOptionBox_DropDownOpened(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
}


void KinectToVR::implementation::DevicesPage::RightElbowRotationOverrideOptionBox_DropDownOpened(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
}


void KinectToVR::implementation::DevicesPage::LeftKneePositionOverrideOptionBox_DropDownOpened(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
}


void KinectToVR::implementation::DevicesPage::LeftKneeRotationOverrideOptionBox_DropDownOpened(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
}


void KinectToVR::implementation::DevicesPage::RightKneePositionOverrideOptionBox_DropDownOpened(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
}


void KinectToVR::implementation::DevicesPage::RightKneeRotationOverrideOptionBox_DropDownOpened(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
}


void KinectToVR::implementation::DevicesPage::DismissOverrideTipNoJointsButton_Click(
	const Windows::Foundation::IInspectable& sender,
	const RoutedEventArgs& e)
{
	NoJointsFlyout().Hide();
}


void KinectToVR::implementation::DevicesPage::DevicesPage_Loaded(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Reset
	devices_tab_re_setup_finished = false;

	// If it's the first time loading, select the base device
	selectedTrackingDeviceID =
		devices_tab_setup_finished
			? devicesListView.get()->SelectedIndex()
			: k2app::K2Settings.trackingDeviceID;

	// Notify of the setup's end
	devices_tab_setup_finished = true;

	// Run the on-selected routine
	const auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(selectedTrackingDeviceID);

	std::string deviceName = "[UNKNOWN]";
	std::wstring device_status = L"Something's wrong!\nE_UNKNOWN\nWhat's happened here?";

	// Only if override -> select enabled combos
	if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
	{
		overrideWaistPosition.get()->IsChecked(k2app::K2Settings.K2TrackersVector[0].isPositionOverridden);
		overrideLeftFootPosition.get()->IsChecked(k2app::K2Settings.K2TrackersVector[1].isPositionOverridden);
		overrideRightFootPosition.get()->IsChecked(k2app::K2Settings.K2TrackersVector[2].isPositionOverridden);

		overrideWaistRotation.get()->IsChecked(k2app::K2Settings.K2TrackersVector[0].isRotationOverridden);
		overrideLeftFootRotation.get()->IsChecked(k2app::K2Settings.K2TrackersVector[1].isRotationOverridden);
		overrideRightFootRotation.get()->IsChecked(k2app::K2Settings.K2TrackersVector[2].isRotationOverridden);

		overrideLeftElbowPosition.get()->IsChecked(k2app::K2Settings.K2TrackersVector[3].isPositionOverridden);
		overrideLeftElbowRotation.get()->IsChecked(k2app::K2Settings.K2TrackersVector[3].isRotationOverridden);

		overrideRightElbowPosition.get()->IsChecked(k2app::K2Settings.K2TrackersVector[4].isPositionOverridden);
		overrideRightElbowRotation.get()->IsChecked(k2app::K2Settings.K2TrackersVector[4].isRotationOverridden);

		overrideLeftKneePosition.get()->IsChecked(k2app::K2Settings.K2TrackersVector[5].isPositionOverridden);
		overrideLeftKneeRotation.get()->IsChecked(k2app::K2Settings.K2TrackersVector[5].isRotationOverridden);

		overrideRightKneePosition.get()->IsChecked(k2app::K2Settings.K2TrackersVector[6].isPositionOverridden);
		overrideRightKneeRotation.get()->IsChecked(k2app::K2Settings.K2TrackersVector[6].isRotationOverridden);
	}

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

		deviceNameLabel.get()->Text(StringToWString(device->getDeviceName()));
		device_status = device->statusResultWString(device->getStatusResult());

		deviceName = device->getDeviceName();

		// Show / Hide device settings button
		selectedDeviceSettingsButton.get()->Visibility(
			device->isSettingsDaemonSupported()
				? Visibility::Visible
				: Visibility::Collapsed);

		// Append device settings / placeholder layout
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
			device->isSettingsDaemonSupported()
				? *TrackingDevices::TrackingDevicesLayoutRootsVector.at(
					selectedTrackingDeviceID)->Get()
				: *k2app::interfacing::emptyLayoutRoot->Get());

		// We've selected a kinectbasis device, so this should be hidden
		for (auto& expander : jointSelectorExpanders)
			expander.get()->ContainerExpander().get()->Visibility(Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(Visibility::Collapsed);

		// Set up combos if the device's OK
		if (device_status.find(L"S_OK") != std::wstring::npos)
		{
			// If we're reconnecting an override device, also refresh joints
			if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
			{
				// Clear items
				// Waist
				devices_clear_combo(waistPositionOverrideOptionBox);
				devices_clear_combo(waistRotationOverrideOptionBox);

				// LeftF
				devices_clear_combo(leftFootPositionOverrideOptionBox);
				devices_clear_combo(leftFootRotationOverrideOptionBox);

				// RightF
				devices_clear_combo(rightFootPositionOverrideOptionBox);
				devices_clear_combo(rightFootRotationOverrideOptionBox);

				// LeftEL
				devices_clear_combo(leftElbowPositionOverrideOptionBox);
				devices_clear_combo(leftElbowRotationOverrideOptionBox);

				// RightEL
				devices_clear_combo(rightElbowPositionOverrideOptionBox);
				devices_clear_combo(rightElbowRotationOverrideOptionBox);

				// LeftK
				devices_clear_combo(leftKneePositionOverrideOptionBox);
				devices_clear_combo(leftKneeRotationOverrideOptionBox);

				// RightK
				devices_clear_combo(rightKneePositionOverrideOptionBox);
				devices_clear_combo(rightKneeRotationOverrideOptionBox);

				// Append all joints to all combos, depend on characteristics
				switch (device->getDeviceCharacteristics())
				{
				case ktvr::K2_Character_Basic:
					{
						devices_push_override_joints(false);
					}
					break;
				case ktvr::K2_Character_Simple:
					{
						devices_push_override_joints();
					}
					break;
				case ktvr::K2_Character_Full:
					{
						devices_push_override_joints();
					}
					break;
				}

				// Try fix override IDs if wrong
				TrackingDevices::devices_check_override_ids(selectedTrackingDeviceID);

				// Select the first (or next, if exists) joint
				// Set the placeholder text on disabled combos
				devices_select_combobox();
			}
		}
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

		deviceNameLabel.get()->Text(StringToWString(device->getDeviceName()));
		device_status = device->statusResultWString(device->getStatusResult());

		deviceName = device->getDeviceName();

		// Show / Hide device settings button
		selectedDeviceSettingsButton.get()->Visibility(
			device->isSettingsDaemonSupported()
				? Visibility::Visible
				: Visibility::Collapsed);

		// Append device settings / placeholder layout
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Clear();
		selectedDeviceSettingsRootLayoutPanel.get()->Children().Append(
			device->isSettingsDaemonSupported()
				? *TrackingDevices::TrackingDevicesLayoutRootsVector.at(
					selectedTrackingDeviceID)->Get()
				: *k2app::interfacing::emptyLayoutRoot->Get());

		// We've selected a jointsbasis device, so this should be visible
		//	at least when the device is online
		for (auto& expander : jointSelectorExpanders)
			expander.get()->ContainerExpander().get()->Visibility(
				(device_status.find(L"S_OK") != std::wstring::npos &&
					selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);
		
		jointBasisLabel.get()->Visibility(
			(device_status.find(L"S_OK") != std::wstring::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);
		
		// Set up combos if the device's OK
		if (device_status.find(L"S_OK") != std::wstring::npos)
		{
			// If we're reconnecting a base device, also refresh joints
			if (selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
			{
				for (auto& expander : jointSelectorExpanders)
					expander.get()->ReAppendTrackers();
			}
			// If we're reconnecting an override device, also refresh joints
			else if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
			{
				// Clear items
				// Waist
				devices_clear_combo(waistPositionOverrideOptionBox);
				devices_clear_combo(waistRotationOverrideOptionBox);

				// LeftF
				devices_clear_combo(leftFootPositionOverrideOptionBox);
				devices_clear_combo(leftFootRotationOverrideOptionBox);

				// RightF
				devices_clear_combo(rightFootPositionOverrideOptionBox);
				devices_clear_combo(rightFootRotationOverrideOptionBox);

				// LeftEL
				devices_clear_combo(leftElbowPositionOverrideOptionBox);
				devices_clear_combo(leftElbowRotationOverrideOptionBox);

				// RightEL
				devices_clear_combo(rightElbowPositionOverrideOptionBox);
				devices_clear_combo(rightElbowRotationOverrideOptionBox);

				// LeftK
				devices_clear_combo(leftKneePositionOverrideOptionBox);
				devices_clear_combo(leftKneeRotationOverrideOptionBox);

				// RightK
				devices_clear_combo(rightKneePositionOverrideOptionBox);
				devices_clear_combo(rightKneeRotationOverrideOptionBox);

				// Append all joints to all combos
				for (auto& _joint : device->getTrackedJoints())
				{
					// Get the name into string
					auto _jointname = _joint.getJointName();

					// Push the name to all combos
					devices_push_override_joints(StringToWString(_jointname));
				}

				// Try fix override IDs if wrong
				TrackingDevices::devices_check_override_ids(selectedTrackingDeviceID);

				// Select the first (or next, if exists) joint
				// Set the placeholder text on disabled combos
				devices_select_combobox();
			}
		}
	}

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	// Update the status here
	const bool status_ok = device_status.find(L"S_OK") != std::wstring::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(split_status(device_status)[0]);
	trackingDeviceErrorLabel.get()->Text(split_status(device_status)[1]);
	errorWhatText.get()->Text(split_status(device_status)[2]);

	if (selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
	{
		LOG(INFO) << "Selected a base";
		setAsOverrideButton.get()->IsEnabled(false);
		setAsBaseButton.get()->IsEnabled(false);

		overridesLabel.get()->Visibility(Visibility::Collapsed);
		overridesControls.get()->Visibility(Visibility::Collapsed);
		overridesControls_1.get()->Visibility(Visibility::Collapsed);
		overridesDropDown.get()->Visibility(Visibility::Collapsed);
		overridesDropDown_1.get()->Visibility(Visibility::Collapsed);
		deselectDeviceButton.get()->Visibility(Visibility::Collapsed);
	}
	else if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
	{
		LOG(INFO) << "Selected an override";
		setAsOverrideButton.get()->IsEnabled(false);
		setAsBaseButton.get()->IsEnabled(true);

		overridesLabel.get()->Visibility(Visibility::Visible);
		overridesControls.get()->Visibility(Visibility::Visible);
		overridesControls_1.get()->Visibility(Visibility::Visible);
		overridesDropDown.get()->Visibility(Visibility::Visible);
		overridesDropDown_1.get()->Visibility(Visibility::Visible);
		deselectDeviceButton.get()->Visibility(Visibility::Visible);
	}
	else
	{
		LOG(INFO) << "Selected a [none]";
		setAsOverrideButton.get()->IsEnabled(true);
		setAsBaseButton.get()->IsEnabled(true);

		overridesLabel.get()->Visibility(Visibility::Collapsed);
		overridesControls.get()->Visibility(Visibility::Collapsed);
		overridesControls_1.get()->Visibility(Visibility::Collapsed);
		overridesDropDown.get()->Visibility(Visibility::Collapsed);
		overridesDropDown_1.get()->Visibility(Visibility::Collapsed);
		deselectDeviceButton.get()->Visibility(Visibility::Collapsed);
	}

	LOG(INFO) << "Changed the currently selected device to " << deviceName;

	// Now we're good
	devices_tab_re_setup_finished = true;
}


void KinectToVR::implementation::DevicesPage::OverridesDropDown_Expanding(
	const Controls::Expander& sender,
	const Controls::ExpanderExpandingEventArgs& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	overridesDropDown_1.get()->IsExpanded(false);
}


void KinectToVR::implementation::DevicesPage::OverridesDropDown_1_Expanding(
	const Controls::Expander& sender,
	const Controls::ExpanderExpandingEventArgs& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	overridesDropDown.get()->IsExpanded(false);
}


void KinectToVR::implementation::DevicesPage::OpenDiscordButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	ShellExecuteA(nullptr, nullptr, "https://discord.gg/YBQCRDG", nullptr, nullptr, SW_SHOW);
}


void KinectToVR::implementation::DevicesPage::OpenDocsButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	ShellExecuteA(nullptr, nullptr, "https://k2vr.tech/docs/", nullptr, nullptr, SW_SHOW);
}


void KinectToVR::implementation::DevicesPage::SelectedDeviceSettingsButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	selectedDeviceSettingsFlyout.get()->ShowAt(sender.as<FrameworkElement>());
}
