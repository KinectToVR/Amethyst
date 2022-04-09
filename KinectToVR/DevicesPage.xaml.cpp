#include "pch.h"
#include "DevicesPage.xaml.h"
#if __has_include("DevicesPage.g.cpp")
#include "DevicesPage.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace ::k2app::shared::devices;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.
bool devices_tab_setup_finished = false;

void devices_check_override_ids(uint32_t const& id)
{
	// Take down IDs if they're too big
	if (auto const& device_pair = TrackingDevices::getCurrentOverrideDevice_Safe(id); device_pair.first)
	{
		if (device_pair.second.index() == 1) // If Joints
		{
			// Note: num_joints should never be 0
			const auto num_joints = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>
				(device_pair.second)->getTrackedJoints().size();

			if (k2app::K2Settings.positionOverrideJointID[0] > (num_joints - 1))
				k2app::K2Settings.positionOverrideJointID[0] = 0;
			if (k2app::K2Settings.positionOverrideJointID[1] > (num_joints - 1))
				k2app::K2Settings.positionOverrideJointID[1] = 0;
			if (k2app::K2Settings.positionOverrideJointID[2] > (num_joints - 1))
				k2app::K2Settings.positionOverrideJointID[2] = 0;
			if (k2app::K2Settings.positionOverrideJointID[3] > (num_joints - 1))
				k2app::K2Settings.positionOverrideJointID[3] = 0;
			if (k2app::K2Settings.positionOverrideJointID[4] > (num_joints - 1))
				k2app::K2Settings.positionOverrideJointID[4] = 0;
			if (k2app::K2Settings.positionOverrideJointID[5] > (num_joints - 1))
				k2app::K2Settings.positionOverrideJointID[5] = 0;
			if (k2app::K2Settings.positionOverrideJointID[6] > (num_joints - 1))
				k2app::K2Settings.positionOverrideJointID[6] = 0;

			if (k2app::K2Settings.rotationOverrideJointID[0] > (num_joints - 1))
				k2app::K2Settings.rotationOverrideJointID[0] = 0;
			if (k2app::K2Settings.rotationOverrideJointID[1] > (num_joints - 1))
				k2app::K2Settings.rotationOverrideJointID[1] = 0;
			if (k2app::K2Settings.rotationOverrideJointID[2] > (num_joints - 1))
				k2app::K2Settings.rotationOverrideJointID[2] = 0;
			if (k2app::K2Settings.rotationOverrideJointID[3] > (num_joints - 1))
				k2app::K2Settings.rotationOverrideJointID[3] = 0;
			if (k2app::K2Settings.rotationOverrideJointID[4] > (num_joints - 1))
				k2app::K2Settings.rotationOverrideJointID[4] = 0;
			if (k2app::K2Settings.rotationOverrideJointID[5] > (num_joints - 1))
				k2app::K2Settings.rotationOverrideJointID[5] = 0;
			if (k2app::K2Settings.rotationOverrideJointID[6] > (num_joints - 1))
				k2app::K2Settings.rotationOverrideJointID[6] = 0;
		}
		else if (device_pair.second.index() == 0) // If Kinect
		{
			// Note: switch based on device characteristics
			const auto characteristics = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>
				(device_pair.second)->getDeviceCharacteristics();
			uint32_t num_joints = -1; // To set later

			if (characteristics == ktvr::K2_Character_Full)
				num_joints = 8;
			else if (characteristics == ktvr::K2_Character_Simple)
				num_joints = 8;
			else if (characteristics == ktvr::K2_Character_Basic)
				num_joints = 3;

			if (k2app::K2Settings.positionOverrideJointID[0] > (num_joints - 1))
				k2app::K2Settings.positionOverrideJointID[0] = 0;
			if (k2app::K2Settings.positionOverrideJointID[1] > (num_joints - 1))
				k2app::K2Settings.positionOverrideJointID[1] = 0;
			if (k2app::K2Settings.positionOverrideJointID[2] > (num_joints - 1))
				k2app::K2Settings.positionOverrideJointID[2] = 0;
			if (k2app::K2Settings.positionOverrideJointID[3] > (num_joints - 1))
				k2app::K2Settings.positionOverrideJointID[3] = 0;
			if (k2app::K2Settings.positionOverrideJointID[4] > (num_joints - 1))
				k2app::K2Settings.positionOverrideJointID[4] = 0;
			if (k2app::K2Settings.positionOverrideJointID[5] > (num_joints - 1))
				k2app::K2Settings.positionOverrideJointID[5] = 0;
			if (k2app::K2Settings.positionOverrideJointID[6] > (num_joints - 1))
				k2app::K2Settings.positionOverrideJointID[6] = 0;

			if (k2app::K2Settings.rotationOverrideJointID[0] > (num_joints - 1))
				k2app::K2Settings.rotationOverrideJointID[0] = 0;
			if (k2app::K2Settings.rotationOverrideJointID[1] > (num_joints - 1))
				k2app::K2Settings.rotationOverrideJointID[1] = 0;
			if (k2app::K2Settings.rotationOverrideJointID[2] > (num_joints - 1))
				k2app::K2Settings.rotationOverrideJointID[2] = 0;
			if (k2app::K2Settings.rotationOverrideJointID[3] > (num_joints - 1))
				k2app::K2Settings.rotationOverrideJointID[3] = 0;
			if (k2app::K2Settings.rotationOverrideJointID[4] > (num_joints - 1))
				k2app::K2Settings.rotationOverrideJointID[4] = 0;
			if (k2app::K2Settings.rotationOverrideJointID[5] > (num_joints - 1))
				k2app::K2Settings.rotationOverrideJointID[5] = 0;
			if (k2app::K2Settings.rotationOverrideJointID[6] > (num_joints - 1))
				k2app::K2Settings.rotationOverrideJointID[6] = 0;
		}
	}
}

void devices_check_base_ids(uint32_t const& id)
{
	// Take down IDs if they're too big
	if (auto const& device_pair = TrackingDevices::getCurrentDevice(id);
		device_pair.index() == 1) // If Joints
	{
		// Note: num_joints should never be 0
		const auto num_joints = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>
			(device_pair)->getTrackedJoints().size();

		if (k2app::K2Settings.selectedTrackedJointID[0] > num_joints)
			k2app::K2Settings.selectedTrackedJointID[0] = 0;
		if (k2app::K2Settings.selectedTrackedJointID[1] > num_joints)
			k2app::K2Settings.selectedTrackedJointID[1] = 0;
		if (k2app::K2Settings.selectedTrackedJointID[2] > num_joints)
			k2app::K2Settings.selectedTrackedJointID[2] = 0;
		if (k2app::K2Settings.selectedTrackedJointID[3] > num_joints)
			k2app::K2Settings.selectedTrackedJointID[3] = 0;
		if (k2app::K2Settings.selectedTrackedJointID[4] > num_joints)
			k2app::K2Settings.selectedTrackedJointID[4] = 0;
		if (k2app::K2Settings.selectedTrackedJointID[5] > num_joints)
			k2app::K2Settings.selectedTrackedJointID[5] = 0;
		if (k2app::K2Settings.selectedTrackedJointID[6] > num_joints)
			k2app::K2Settings.selectedTrackedJointID[6] = 0;
	}
}

void devices_update_current()
{
	{
		auto const& trackingDevice = TrackingDevices::TrackingDevicesVector.at(k2app::K2Settings.trackingDeviceID);

		std::string deviceName = "[UNKNOWN]";

		if (trackingDevice.index() == 0)
		{
			// Kinect Basis
			const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);
			deviceName = device->getDeviceName();
		}
		else if (trackingDevice.index() == 1)
		{
			// Joints Basis
			const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
			deviceName = device->getDeviceName();
		}

		/* Update local statuses */
		baseDeviceName.get()->Text(wstring_cast(deviceName));
		if (overrideDeviceName.get()->Text() == wstring_cast(deviceName))
			overrideDeviceName.get()->Text(L"No Overrides");
	}
	{
		if (k2app::K2Settings.overrideDeviceID < 0)return;
		auto const& trackingDevice = TrackingDevices::TrackingDevicesVector.at(k2app::K2Settings.overrideDeviceID);

		std::string deviceName = "[UNKNOWN]";

		if (trackingDevice.index() == 0)
		{
			// Kinect Basis
			const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);
			deviceName = device->getDeviceName();
		}
		else if (trackingDevice.index() == 1)
		{
			// Joints Basis
			const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
			deviceName = device->getDeviceName();
		}

		/* Update local statuses */
		overrideDeviceName.get()->Text(wstring_cast(deviceName));
	}
}

void devices_clear_combo(std::shared_ptr<Controls::ComboBox> const& cbox)
{
	try
	{
		cbox.get()->Items().Clear();
	}
	catch (...)
	{
		LOG(WARNING) << "Couldn't clear a ComboBox. You better call an exorcist.";
	}
}

void devices_push_combobox(
	std::shared_ptr<Controls::ComboBox> const& cbox,
	hstring const& str)
{
	try
	{
		cbox.get()->Items().Append(box_value(str));
	}
	catch (...)
	{
		LOG(WARNING) << "Couldn't push to a ComboBox. You better call an exorcist.";
	}
}

void devices_push_override_joints_combo(
	std::shared_ptr<Controls::ComboBox> const& cbox,
	bool const& all = true)
{
	devices_push_combobox(cbox, L"Chest");

	if (all)
	{
		devices_push_combobox(cbox, L"Left Elbow");
		devices_push_combobox(cbox, L"Right Elbow");
	}

	devices_push_combobox(cbox, L"Waist");

	if (all)
	{
		devices_push_combobox(cbox, L"Left Knee");
		devices_push_combobox(cbox, L"Right Knee");
	}

	devices_push_combobox(cbox, L"Left Foot");
	devices_push_combobox(cbox, L"Right Foot");
}

void devices_push_override_joints(bool const& all = true)
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

void devices_push_override_joints(std::wstring const& _string)
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
	std::shared_ptr<Controls::ComboBox> const& cbox,
	int const& index)
{
	try
	{
		cbox.get()->SelectedIndex(index);
	}
	catch (...)
	{
		LOG(WARNING) << "Couldn't select a ComboBox index. You better call an exorcist.";
	}
}

void devices_select_combobox()
{
	// Waist
	devices_select_combobox_safe(
		waistPositionOverrideOptionBox,
		k2app::K2Settings.isPositionOverriddenJoint[0]
			? k2app::K2Settings.positionOverrideJointID[0]
			: -1);
	devices_select_combobox_safe(
		waistRotationOverrideOptionBox,
		k2app::K2Settings.isRotationOverriddenJoint[0]
			? k2app::K2Settings.rotationOverrideJointID[0]
			: -1);

	// LeftF
	devices_select_combobox_safe(
		leftFootPositionOverrideOptionBox,
		k2app::K2Settings.isPositionOverriddenJoint[1]
			? k2app::K2Settings.positionOverrideJointID[1]
			: -1);
	devices_select_combobox_safe(
		leftFootRotationOverrideOptionBox,
		k2app::K2Settings.isRotationOverriddenJoint[1]
			? k2app::K2Settings.rotationOverrideJointID[1]
			: -1);

	// RightF
	devices_select_combobox_safe(
		rightFootPositionOverrideOptionBox,
		k2app::K2Settings.isPositionOverriddenJoint[2]
			? k2app::K2Settings.positionOverrideJointID[2]
			: -1);
	devices_select_combobox_safe(
		rightFootRotationOverrideOptionBox,
		k2app::K2Settings.isRotationOverriddenJoint[2]
			? k2app::K2Settings.rotationOverrideJointID[2]
			: -1);

	// LeftEL
	devices_select_combobox_safe(
		leftElbowPositionOverrideOptionBox,
		k2app::K2Settings.isPositionOverriddenJoint[3]
			? k2app::K2Settings.positionOverrideJointID[3]
			: -1);
	devices_select_combobox_safe(
		leftElbowRotationOverrideOptionBox,
		k2app::K2Settings.isRotationOverriddenJoint[3]
			? k2app::K2Settings.rotationOverrideJointID[3]
			: -1);

	// RightEL
	devices_select_combobox_safe(
		rightElbowPositionOverrideOptionBox,
		k2app::K2Settings.isPositionOverriddenJoint[4]
			? k2app::K2Settings.positionOverrideJointID[4]
			: -1);
	devices_select_combobox_safe(
		rightElbowRotationOverrideOptionBox,
		k2app::K2Settings.isRotationOverriddenJoint[4]
			? k2app::K2Settings.rotationOverrideJointID[4]
			: -1);

	// LeftK
	devices_select_combobox_safe(
		leftKneePositionOverrideOptionBox,
		k2app::K2Settings.isPositionOverriddenJoint[5]
			? k2app::K2Settings.positionOverrideJointID[5]
			: -1);
	devices_select_combobox_safe(
		leftKneeRotationOverrideOptionBox,
		k2app::K2Settings.isRotationOverriddenJoint[5]
			? k2app::K2Settings.rotationOverrideJointID[5]
			: -1);

	// RightK
	devices_select_combobox_safe(
		rightKneePositionOverrideOptionBox,
		k2app::K2Settings.isPositionOverriddenJoint[6]
			? k2app::K2Settings.positionOverrideJointID[6]
			: -1);
	devices_select_combobox_safe(
		rightKneeRotationOverrideOptionBox,
		k2app::K2Settings.isRotationOverriddenJoint[6]
			? k2app::K2Settings.rotationOverrideJointID[6]
			: -1);
}

namespace winrt::KinectToVR::implementation
{
	DevicesPage::DevicesPage()
	{
		InitializeComponent();

		// Cache needed UI elements
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
		jointBasisControls = std::make_shared<Controls::Grid>(JointBasisControls());
		jointBasisControls_1 = std::make_shared<Controls::Grid>(JointBasisControls_1());
		devicesMainContentGridOuter = std::make_shared<Controls::Grid>(DevicesMainContentGridOuter());
		devicesMainContentGridInner = std::make_shared<Controls::Grid>(DevicesMainContentGridInner());

		jointBasisDropDown = std::make_shared<Controls::Expander>(JointBasisDropDown());
		jointBasisDropDown_1 = std::make_shared<Controls::Expander>(JointBasisDropDown_1());
		overridesDropDown = std::make_shared<Controls::Expander>(OverridesDropDown());
		overridesDropDown_1 = std::make_shared<Controls::Expander>(OverridesDropDown_1());

		devicesListView = std::make_shared<Controls::ListView>(TrackingDeviceListView());

		setAsOverrideButton = std::make_shared<Controls::Button>(SetAsOverrideButton());
		setAsBaseButton = std::make_shared<Controls::Button>(SetAsBaseButton());
		deselectDeviceButton = std::make_shared<Controls::Button>(DeselectDeviceButton());

		waistJointOptionBox = std::make_shared<Controls::ComboBox>(WaistJointOptionBox());
		leftFootJointOptionBox = std::make_shared<Controls::ComboBox>(LeftFootJointOptionBox());
		rightFootJointOptionBox = std::make_shared<Controls::ComboBox>(RightFootJointOptionBox());
		leftElbowJointOptionBox = std::make_shared<Controls::ComboBox>(LeftElbowJointOptionBox());
		rightElbowJointOptionBox = std::make_shared<Controls::ComboBox>(RightElbowJointOptionBox());
		leftKneeJointOptionBox = std::make_shared<Controls::ComboBox>(LeftKneeJointOptionBox());
		rightKneeJointOptionBox = std::make_shared<Controls::ComboBox>(RightKneeJointOptionBox());

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
		jointBasisDropDown = std::make_shared<Controls::Expander>(JointBasisDropDown());
		jointBasisDropDown_1 = std::make_shared<Controls::Expander>(JointBasisDropDown_1());

		devicesMainContentScrollViewer = std::make_shared<Controls::ScrollViewer>(DevicesMainContentScrollViewer());

		devicesOverridesSelectorStackPanelOuter = std::make_shared<Controls::StackPanel>(DevicesOverridesSelectorStackPanelOuter());
		devicesOverridesSelectorStackPanelInner = std::make_shared<Controls::StackPanel>(DevicesOverridesSelectorStackPanelInner());

		devicesJointsBasisSelectorStackPanelOuter = std::make_shared<Controls::StackPanel>(DevicesJointsBasisSelectorStackPanelOuter());
		devicesJointsBasisSelectorStackPanelInner = std::make_shared<Controls::StackPanel>(DevicesJointsBasisSelectorStackPanelInner());

		// Create tracking devices' list
		static auto m_TrackingDevicesViewModels =
			multi_threaded_observable_vector<KinectToVR::TrackingDevicesView>();

		// Watch for insertions
		m_TrackingDevicesViewModels.VectorChanged(
			[&](Windows::Foundation::Collections::IObservableVector<KinectToVR::TrackingDevicesView> const& sender,
			    Windows::Foundation::Collections::IVectorChangedEventArgs const& args)
			{
				// Report a registration and parse
				LOG(INFO) << string_cast(sender.GetAt(sender.Size() - 1).DeviceName().c_str()) <<
					"'s been registered as a tracking device. [UI Node]";

				// Set the current device
				if (sender.Size() > k2app::K2Settings.trackingDeviceID)
					sender.GetAt(k2app::K2Settings.trackingDeviceID).Current(true);

				// Re-set all indexes
				for (uint32_t i = 0; i < sender.Size(); i++)
					sender.GetAt(i).DeviceID(i);
			});

		// Add tracking devices here
		for (auto const& device : TrackingDevices::TrackingDevicesVector)
		{
			std::string deviceName = "[UNKNOWN]";

			switch (device.index())
			{
			case 0:
				{
					auto const& pDevice = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(device);
					deviceName = pDevice->getDeviceName();
				}
				break;
			case 1:
				{
					auto const& pDevice = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(device);
					deviceName = pDevice->getDeviceName();
				}
				break;
			}

			LOG(INFO) << "Appending " << deviceName <<
				" to UI Node's tracking devices' list...";
			m_TrackingDevicesViewModels.Append(
				make<TrackingDevicesView>(wstring_cast(deviceName).c_str()));
		}

		// Register tracking devices' list
		devicesListView.get()->ItemsSource(m_TrackingDevicesViewModels);

		// Set currently tracking device & selected device
		// RadioButton is set on ItemChanged
		devicesListView.get()->SelectedIndex(k2app::K2Settings.trackingDeviceID);

		NavigationCacheMode(Navigation::NavigationCacheMode::Required);
		devices_update_current();
	}
}


Windows::Foundation::IAsyncAction
winrt::KinectToVR::implementation::DevicesPage::TrackingDeviceListView_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)co_return; // Block dummy selects

	selectedTrackingDeviceID = sender.as<Controls::ListView>().SelectedIndex();
	auto const& trackingDevice = TrackingDevices::TrackingDevicesVector.at(selectedTrackingDeviceID);

	std::string deviceName = "[UNKNOWN]";
	std::string device_status = "E_UKNOWN\nWhat's happened here?";

	// Only if override -> select enabled combos
	if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
	{
		overrideWaistPosition.get()->IsChecked(k2app::K2Settings.isPositionOverriddenJoint[0]);
		overrideLeftFootPosition.get()->IsChecked(k2app::K2Settings.isPositionOverriddenJoint[1]);
		overrideRightFootPosition.get()->IsChecked(k2app::K2Settings.isPositionOverriddenJoint[2]);

		overrideWaistRotation.get()->IsChecked(k2app::K2Settings.isRotationOverriddenJoint[0]);
		overrideLeftFootRotation.get()->IsChecked(k2app::K2Settings.isRotationOverriddenJoint[1]);
		overrideRightFootRotation.get()->IsChecked(k2app::K2Settings.isRotationOverriddenJoint[2]);

		overrideLeftElbowPosition.get()->IsChecked(k2app::K2Settings.isPositionOverriddenJoint[3]);
		overrideLeftElbowRotation.get()->IsChecked(k2app::K2Settings.isRotationOverriddenJoint[3]);

		overrideRightElbowPosition.get()->IsChecked(k2app::K2Settings.isPositionOverriddenJoint[4]);
		overrideRightElbowRotation.get()->IsChecked(k2app::K2Settings.isRotationOverriddenJoint[4]);

		overrideLeftKneePosition.get()->IsChecked(k2app::K2Settings.isPositionOverriddenJoint[5]);
		overrideLeftKneeRotation.get()->IsChecked(k2app::K2Settings.isRotationOverriddenJoint[5]);

		overrideRightKneePosition.get()->IsChecked(k2app::K2Settings.isPositionOverriddenJoint[6]);
		overrideRightKneeRotation.get()->IsChecked(k2app::K2Settings.isRotationOverriddenJoint[6]);
	}

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

		deviceNameLabel.get()->Text(wstring_cast(device->getDeviceName()));
		device_status = device->statusResultString(device->getStatusResult());

		deviceName = device->getDeviceName();

		// We've selected a kinectbasis device, so this should be hidden
		jointBasisControls.get()->Visibility(Visibility::Collapsed);
		jointBasisControls_1.get()->Visibility(Visibility::Collapsed);
		jointBasisDropDown.get()->Visibility(Visibility::Collapsed);
		jointBasisDropDown_1.get()->Visibility(Visibility::Collapsed);
		jointBasisLabel.get()->Visibility(Visibility::Collapsed);

		// Set up combos if the device's OK
		if (device_status.find("S_OK") != std::string::npos)
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
				devices_check_override_ids(selectedTrackingDeviceID);

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

		deviceNameLabel.get()->Text(wstring_cast(device->getDeviceName()));
		device_status = device->statusResultString(device->getStatusResult());

		deviceName = device->getDeviceName();

		// We've selected a jointsbasis device, so this should be visible
		//	at least when the device is online
		jointBasisControls.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisControls_1.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisDropDown.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisDropDown_1.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		// Set up combos if the device's OK
		if (device_status.find("S_OK") != std::string::npos)
		{
			// If we're reconnecting a base device, also refresh joints
			if (selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
			{
				// Clear items
				// Waist
				devices_clear_combo(waistJointOptionBox);
				// LeftF
				devices_clear_combo(leftFootJointOptionBox);
				// RightF
				devices_clear_combo(rightFootJointOptionBox);
				// LeftEL
				devices_clear_combo(leftElbowJointOptionBox);
				// RightEL
				devices_clear_combo(rightElbowJointOptionBox);
				// LeftK
				devices_clear_combo(leftKneeJointOptionBox);
				// RightK
				devices_clear_combo(rightKneeJointOptionBox);

				// Append all joints to all combos
				for (auto& _joint : device->getTrackedJoints())
				{
					// Get the name into string
					auto _jointname = _joint.getJointName();

					// Push the name to all combos
					// Waist
					waistJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// LeftF
					leftFootJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// RightF
					rightFootJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// LeftEL
					leftElbowJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// RightEL
					rightElbowJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// LeftK
					leftKneeJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// RightK
					rightKneeJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
				}

				// Check base IDs if wrong
				devices_check_base_ids(selectedTrackingDeviceID);

				// Select the first (or next, if exists) joint
				// Set the placeholder text on disabled combos
				// Waist
				waistJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[0]);
				// LeftF
				leftFootJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[1]);
				// RightF
				rightFootJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[2]);
				// LeftEL
				leftElbowJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[3]);
				// RightEL
				rightElbowJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[4]);
				// LeftEL
				leftKneeJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[5]);
				// RightEL
				rightKneeJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[6]);
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
					devices_push_override_joints(wstring_cast(_jointname));
				}

				// Try fix override IDs if wrong
				devices_check_override_ids(selectedTrackingDeviceID);

				// Select the first (or next, if exists) joint
				// Set the placeholder text on disabled combos
				devices_select_combobox();
			}
		}
	}

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	// Update the status here
	const bool status_ok = device_status.find("S_OK") != std::string::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
	trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
	errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));

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
		winrt::apartment_context ui_thread;
		co_await winrt::resume_background();
		Sleep(10); // 10ms for slower systems
		co_await ui_thread;

		// Re-add the child for it to play our funky transition
		// (Though it's not the same as before...)
		devicesMainContentGridOuter.get()->Children().
			Append(*devicesMainContentGridInner);
	}

	LOG(INFO) << "Changed the currently selected device to " << deviceName;

	// Remove the transition
	winrt::apartment_context ui_thread;
	co_await winrt::resume_background();
	Sleep(100);
	co_await ui_thread;

	devicesMainContentGridInner.get()->Transitions().Clear();
}


void winrt::KinectToVR::implementation::DevicesPage::ReconnectDeviceButton_Click(
	winrt::Microsoft::UI::Xaml::Controls::SplitButton const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs const& args)
{
	auto _index = devicesListView.get()->SelectedIndex();

	auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(_index);
	std::string device_status = "E_UKNOWN\nWhat's happened here?";
	LOG(INFO) << "Now reconnecting the tracking device...";

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		auto const& device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

		device->initialize();
		device_status = device->statusResultString(device->getStatusResult());

		// We've selected a kinectbasis device, so this should be hidden
		jointBasisControls.get()->Visibility(Visibility::Collapsed);
		jointBasisControls_1.get()->Visibility(Visibility::Collapsed);
		jointBasisDropDown.get()->Visibility(Visibility::Collapsed);
		jointBasisDropDown_1.get()->Visibility(Visibility::Collapsed);
		jointBasisLabel.get()->Visibility(Visibility::Collapsed);

		// Set up combos if the device's OK
		if (device_status.find("S_OK") != std::string::npos)
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
				devices_check_override_ids(selectedTrackingDeviceID);

				// Select the first (or next, if exists) joint
				// Set the placeholder text on disabled combos
				devices_select_combobox();
			}
		}
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		auto const& device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

		device->initialize();
		device_status = device->statusResultString(device->getStatusResult());

		// We've selected a jointsbasis device, so this should be visible
		//	at least when the device is online
		jointBasisControls.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisControls_1.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisDropDown.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisDropDown_1.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		// Set up combos if the device's OK
		if (device_status.find("S_OK") != std::string::npos)
		{
			// If we're reconnecting a base device, also refresh joints
			if (selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
			{
				// Clear items
				// Waist
				devices_clear_combo(waistJointOptionBox);
				// LeftF
				devices_clear_combo(leftFootJointOptionBox);
				// RightF
				devices_clear_combo(rightFootJointOptionBox);
				// LeftEL
				devices_clear_combo(leftElbowJointOptionBox);
				// RightEL
				devices_clear_combo(rightElbowJointOptionBox);
				// LeftK
				devices_clear_combo(leftKneeJointOptionBox);
				// RightK
				devices_clear_combo(rightKneeJointOptionBox);

				// Append all joints to all combos
				for (auto& _joint : device->getTrackedJoints())
				{
					// Get the name into string
					auto _jointname = _joint.getJointName();

					// Push the name to all combos
					// Waist
					waistJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// LeftF
					leftFootJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// RightF
					rightFootJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// LeftEL
					leftElbowJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// RightEL
					rightElbowJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// LeftK
					leftKneeJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// RightK
					rightKneeJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
				}

				// Check base IDs if wrong
				devices_check_base_ids(selectedTrackingDeviceID);

				// Select the first (or next, if exists) joint
				// Set the placeholder text on disabled combos
				// Waist
				waistJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[0]);
				// LeftF
				leftFootJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[1]);
				// RightF
				rightFootJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[2]);
				// LeftEL
				leftElbowJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[3]);
				// RightEL
				rightElbowJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[4]);
				// LeftEL
				leftKneeJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[5]);
				// RightEL
				rightKneeJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[6]);
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
					devices_push_override_joints(wstring_cast(_jointname));
				}

				// Try fix override IDs if wrong
				devices_check_override_ids(selectedTrackingDeviceID);

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
	const bool status_ok = device_status.find("S_OK") != std::string::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
	trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
	errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));

	// Update the GeneralPage status
	TrackingDevices::updateTrackingDeviceUI(k2app::K2Settings.trackingDeviceID);
	TrackingDevices::updateOverrideDeviceUI(k2app::K2Settings.overrideDeviceID); // Auto-handles if none
}

// *Nearly* the same as reconnect
void winrt::KinectToVR::implementation::DevicesPage::DisconnectDeviceButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	auto _index = devicesListView.get()->SelectedIndex();

	auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(_index);
	std::string device_status = "E_UKNOWN\nWhat's happened here?";
	LOG(INFO) << "Now disconnecting the tracking device...";

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		auto const& device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

		device->shutdown();
		device_status = device->statusResultString(device->getStatusResult());

		// We've selected a kinectbasis device, so this should be hidden
		jointBasisControls.get()->Visibility(Visibility::Collapsed);
		jointBasisControls_1.get()->Visibility(Visibility::Collapsed);
		jointBasisDropDown.get()->Visibility(Visibility::Collapsed);
		jointBasisDropDown_1.get()->Visibility(Visibility::Collapsed);
		jointBasisLabel.get()->Visibility(Visibility::Collapsed);
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		auto const& device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

		device->shutdown();
		device_status = device->statusResultString(device->getStatusResult());

		// We've selected a jointsbasis device, so this should be visible
		//	at least when the device is online
		jointBasisControls.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisControls_1.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisDropDown.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisDropDown_1.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);
	}

	/* Update local statuses */

	// Update the status here
	const bool status_ok = device_status.find("S_OK") != std::string::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
	trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
	errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));

	// Update the GeneralPage status
	TrackingDevices::updateTrackingDeviceUI(k2app::K2Settings.trackingDeviceID);
	TrackingDevices::updateOverrideDeviceUI(k2app::K2Settings.overrideDeviceID); // Auto-handles if none

	AlternativeConnectionOptionsFlyout().Hide();
}

// Mark override device as -1 -> deselect it
void winrt::KinectToVR::implementation::DevicesPage::DeselectDeviceButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	auto _index = devicesListView.get()->SelectedIndex();

	auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(_index);
	std::string device_status = "E_UKNOWN\nWhat's happened here?";
	LOG(INFO) << "Now deselecting the tracking device...";

	jointBasisControls.get()->Visibility(Visibility::Collapsed);
	jointBasisControls_1.get()->Visibility(Visibility::Collapsed);
	jointBasisDropDown.get()->Visibility(Visibility::Collapsed);
	jointBasisDropDown_1.get()->Visibility(Visibility::Collapsed);
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
		auto const& device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);
		device_status = device->statusResultString(device->getStatusResult());
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		auto const& device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
		device_status = device->statusResultString(device->getStatusResult());
	}

	/* Update local statuses */

	// Update the status here
	const bool status_ok = device_status.find("S_OK") != std::string::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
	trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
	errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));

	// Deselect the device
	k2app::K2Settings.overrideDeviceID = -1; // Only acceptable for an Override
	TrackingDevices::updateOverrideDeviceUI(k2app::K2Settings.overrideDeviceID);

	// Also update in the UI
	overrideDeviceName.get()->Text(L"No Overrides");

	// Save settings
	k2app::K2Settings.saveSettings();

	AlternativeConnectionOptionsFlyout().Hide();
}


Windows::Foundation::IAsyncAction winrt::KinectToVR::implementation::DevicesPage::SetAsOverrideButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	auto const& trackingDevice = TrackingDevices::TrackingDevicesVector.at(selectedTrackingDeviceID);

	std::string device_status = "E_UKNOWN\nWhat's happened here?";
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
		devices_check_override_ids(selectedTrackingDeviceID);

		// Select the first (or next, if exists) joint
		// Set the placeholder text on disabled combos
		devices_select_combobox();

		// Backup the status
		device_status = device->statusResultString(device->getStatusResult());

		// We've selected a kinectbasis device, so this should be hidden
		jointBasisControls.get()->Visibility(Visibility::Collapsed);
		jointBasisControls_1.get()->Visibility(Visibility::Collapsed);
		jointBasisDropDown.get()->Visibility(Visibility::Collapsed);
		jointBasisDropDown_1.get()->Visibility(Visibility::Collapsed);
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
			devices_push_override_joints(wstring_cast(_jointname));
		}

		// Try fix override IDs if wrong
		devices_check_override_ids(selectedTrackingDeviceID);

		// Select the first (or next, if exists) joint
		// Set the placeholder text on disabled combos
		devices_select_combobox();

		// Backup the status
		device_status = device->statusResultString(device->getStatusResult());

		// We've selected an override device, so this should be hidden
		jointBasisControls.get()->Visibility(Visibility::Collapsed);
		jointBasisControls_1.get()->Visibility(Visibility::Collapsed);
		jointBasisDropDown.get()->Visibility(Visibility::Collapsed);
		jointBasisDropDown_1.get()->Visibility(Visibility::Collapsed);
		jointBasisLabel.get()->Visibility(Visibility::Collapsed);
	}

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	/* Update local statuses */
	setAsOverrideButton.get()->IsEnabled(false);
	setAsBaseButton.get()->IsEnabled(true);
	SetDeviceTypeFlyout().Hide(); // Hide the flyout

	overrideDeviceName.get()->Text(wstring_cast(deviceName));

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
	const bool status_ok = device_status.find("S_OK") != std::string::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
	trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
	errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));

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
		winrt::apartment_context ui_thread;
		co_await winrt::resume_background();
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
	winrt::apartment_context ui_thread;
	co_await winrt::resume_background();
	Sleep(100);
	co_await ui_thread;

	devicesOverridesSelectorStackPanelInner.get()->Transitions().Clear();
}


Windows::Foundation::IAsyncAction winrt::KinectToVR::implementation::DevicesPage::SetAsBaseButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	auto const& trackingDevice = TrackingDevices::TrackingDevicesVector.at(selectedTrackingDeviceID);

	std::string device_status = "E_UKNOWN\nWhat's happened here?";
	std::string deviceName = "[UNKNOWN]";

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);
		deviceName = device->getDeviceName();

		device->initialize(); // Init the device as we'll be using it
		device_status = device->statusResultString(device->getStatusResult());

		// We've selected a kinectbasis device, so this should be hidden
		jointBasisControls.get()->Visibility(Visibility::Collapsed);
		jointBasisControls_1.get()->Visibility(Visibility::Collapsed);
		jointBasisDropDown.get()->Visibility(Visibility::Collapsed);
		jointBasisDropDown_1.get()->Visibility(Visibility::Collapsed);
		jointBasisLabel.get()->Visibility(Visibility::Collapsed);
	}
	else if (trackingDevice.index() == 1)
	{
		// Joints Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
		deviceName = device->getDeviceName();
		device->initialize(); // Init the device as we'll be using it

		// Also refresh joints

		// Clear items
		// Waist
		devices_clear_combo(waistJointOptionBox);
		// LeftF
		devices_clear_combo(leftFootJointOptionBox);
		// RightF
		devices_clear_combo(rightFootJointOptionBox);
		// LeftEL
		devices_clear_combo(leftElbowJointOptionBox);
		// RightEL
		devices_clear_combo(rightElbowJointOptionBox);
		// LeftK
		devices_clear_combo(leftKneeJointOptionBox);
		// RightK
		devices_clear_combo(rightKneeJointOptionBox);

		// Append all joints to all combos
		for (auto& _joint : device->getTrackedJoints())
		{
			// Get the name into string
			auto _jointname = _joint.getJointName();

			// Push the name to all combos
			// Waist
			waistJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
			// LeftF
			leftFootJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
			// RightF
			rightFootJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
			// LeftEL
			leftElbowJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
			// RightEL
			rightElbowJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
			// LeftK
			leftKneeJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
			// RightK
			rightKneeJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
		}

		// Check base IDs if wrong
		devices_check_base_ids(selectedTrackingDeviceID);

		// Select the first (or next, if exists) joint
		// Set the placeholder text on disabled combos
		// Waist
		waistJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[0]);
		// LeftF
		leftFootJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[1]);
		// RightF
		rightFootJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[2]);
		// LeftEL
		leftElbowJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[3]);
		// RightEL
		rightElbowJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[4]);
		// LeftEL
		leftKneeJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[5]);
		// RightEL
		rightKneeJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[6]);

		// Update the status
		device_status = device->statusResultString(device->getStatusResult());

		// We've selected a jointsbasis device, so this should be visible
		//	at least when the device is online
		jointBasisControls.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos) ? Visibility::Visible : Visibility::Collapsed);

		jointBasisControls_1.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos) ? Visibility::Visible : Visibility::Collapsed);

		jointBasisDropDown.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos) ? Visibility::Visible : Visibility::Collapsed);

		jointBasisDropDown_1.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos) ? Visibility::Visible : Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos) ? Visibility::Visible : Visibility::Collapsed);
	}

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	/* Update local statuses */
	setAsOverrideButton.get()->IsEnabled(false);
	setAsBaseButton.get()->IsEnabled(false);
	SetDeviceTypeFlyout().Hide(); // Hide the flyout

	baseDeviceName.get()->Text(wstring_cast(deviceName));
	if (overrideDeviceName.get()->Text() == wstring_cast(deviceName))
		overrideDeviceName.get()->Text(L"No Overrides");

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
	const bool status_ok = device_status.find("S_OK") != std::string::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
	trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
	errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));

	k2app::K2Settings.trackingDeviceID = selectedTrackingDeviceID;
	if (k2app::K2Settings.overrideDeviceID == k2app::K2Settings.trackingDeviceID)
		k2app::K2Settings.overrideDeviceID = -1; // Reset the override

	TrackingDevices::updateTrackingDeviceUI(k2app::K2Settings.trackingDeviceID);

	// This is here too cause an override might've became a base... -_-
	TrackingDevices::updateOverrideDeviceUI(k2app::K2Settings.overrideDeviceID); // Auto-handles if none

	// If controls are set to be visible
	if (jointBasisControls.get()->Visibility() == Visibility::Visible) {

		// Remove the only one child of our outer main content grid
		// (What a bestiality it is to do that!!1)
		devicesJointsBasisSelectorStackPanelOuter.get()->Children().RemoveAtEnd();

		Media::Animation::EntranceThemeTransition t;
		t.IsStaggeringEnabled(true);

		devicesJointsBasisSelectorStackPanelInner.get()->Transitions().Append(t);

		// Sleep peacefully pretending that noting happened
		winrt::apartment_context ui_thread;
		co_await winrt::resume_background();
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
	winrt::apartment_context ui_thread;
	co_await winrt::resume_background();
	Sleep(100);
	co_await ui_thread;

	devicesJointsBasisSelectorStackPanelInner.get()->Transitions().Clear();
}

/* For JointBasis device type: joints selector */

void winrt::KinectToVR::implementation::DevicesPage::WaistJointOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (waistJointOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.selectedTrackedJointID[0] = waistJointOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevice = TrackingDevices::getCurrentDevice();
	if (trackingDevice.index() == 1) // if JointsBase
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice)->
			signalJoint(k2app::K2Settings.selectedTrackedJointID[0]);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::LeftFootJointOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (leftFootJointOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.selectedTrackedJointID[1] = leftFootJointOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevice = TrackingDevices::getCurrentDevice();
	if (trackingDevice.index() == 1) // if JointsBase
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice)->
			signalJoint(k2app::K2Settings.selectedTrackedJointID[1]);

	// Save settings
	k2app::K2Settings.saveSettings();
}

void winrt::KinectToVR::implementation::DevicesPage::RightFootJointOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (rightFootJointOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.selectedTrackedJointID[2] = rightFootJointOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevice = TrackingDevices::getCurrentDevice();
	if (trackingDevice.index() == 1) // if JointsBase
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice)->
			signalJoint(k2app::K2Settings.selectedTrackedJointID[2]);

	// Save settings
	k2app::K2Settings.saveSettings();
}

void winrt::KinectToVR::implementation::DevicesPage::LeftElbowJointOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (leftElbowJointOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.selectedTrackedJointID[3] = leftElbowJointOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevice = TrackingDevices::getCurrentDevice();
	if (trackingDevice.index() == 1) // if JointsBase
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice)->
			signalJoint(k2app::K2Settings.selectedTrackedJointID[3]);

	// Save settings
	k2app::K2Settings.saveSettings();
}

void winrt::KinectToVR::implementation::DevicesPage::RightElbowJointOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (rightElbowJointOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.selectedTrackedJointID[4] = rightElbowJointOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevice = TrackingDevices::getCurrentDevice();
	if (trackingDevice.index() == 1) // if JointsBase
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice)->
			signalJoint(k2app::K2Settings.selectedTrackedJointID[4]);

	// Save settings
	k2app::K2Settings.saveSettings();
}

void winrt::KinectToVR::implementation::DevicesPage::LeftKneeJointOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (leftKneeJointOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.selectedTrackedJointID[5] = leftKneeJointOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevice = TrackingDevices::getCurrentDevice();
	if (trackingDevice.index() == 1) // if JointsBase
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice)->
			signalJoint(k2app::K2Settings.selectedTrackedJointID[5]);

	// Save settings
	k2app::K2Settings.saveSettings();
}

void winrt::KinectToVR::implementation::DevicesPage::RightKneeJointOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (rightKneeJointOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.selectedTrackedJointID[6] = rightKneeJointOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevice = TrackingDevices::getCurrentDevice();
	if (trackingDevice.index() == 1) // if JointsBase
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice)->
			signalJoint(k2app::K2Settings.selectedTrackedJointID[6]);

	// Save settings
	k2app::K2Settings.saveSettings();
}

/* For *Override* device type: position & rotation joints selector */

void winrt::KinectToVR::implementation::DevicesPage::WaistPositionOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isPositionOverriddenJoint[0] &&
		waistPositionOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.positionOverrideJointID[0] = waistPositionOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1) // if JointsBase
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.positionOverrideJointID[0]);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::WaistRotationOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isRotationOverriddenJoint[0] &&
		waistRotationOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.rotationOverrideJointID[0] = waistRotationOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1) // if JointsBase
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.rotationOverrideJointID[0]);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::LeftFootPositionOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isPositionOverriddenJoint[1] &&
		leftFootPositionOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.positionOverrideJointID[1] = leftFootPositionOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1) // if JointsBase
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.positionOverrideJointID[1]);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::LeftFootRotationOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isRotationOverriddenJoint[1] &&
		leftFootRotationOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.rotationOverrideJointID[1] = leftFootRotationOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1) // if JointsBase
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.rotationOverrideJointID[1]);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::RightFootPositionOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isPositionOverriddenJoint[2] &&
		rightFootPositionOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.positionOverrideJointID[2] = rightFootPositionOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1) // if JointsBase
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.positionOverrideJointID[2]);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::RightFootRotationOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isRotationOverriddenJoint[2] &&
		rightFootRotationOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.rotationOverrideJointID[2] = rightFootRotationOverrideOptionBox.get()->SelectedIndex();

	//// If we're using a joints device then also signal the joint
	auto const& trackingDevice = TrackingDevices::getCurrentOverrideDevice();
	if (trackingDevice.index() == 1) // if JointsBase
		std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice)->
			signalJoint(k2app::K2Settings.rotationOverrideJointID[2]);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::LeftElbowPositionOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isPositionOverriddenJoint[3] &&
		leftElbowPositionOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.positionOverrideJointID[3] = leftElbowPositionOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1) // if JointsBase
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.positionOverrideJointID[3]);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::LeftElbowRotationOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isRotationOverriddenJoint[3] &&
		leftElbowRotationOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.rotationOverrideJointID[3] = leftElbowRotationOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1) // if JointsBase
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.rotationOverrideJointID[3]);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::RightElbowPositionOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isPositionOverriddenJoint[4] &&
		rightElbowPositionOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.positionOverrideJointID[4] = rightElbowPositionOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1) // if JointsBase
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.positionOverrideJointID[4]);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::RightElbowRotationOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isRotationOverriddenJoint[4] &&
		rightElbowRotationOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.rotationOverrideJointID[4] = rightElbowRotationOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1) // if JointsBase
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.rotationOverrideJointID[4]);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::LeftKneePositionOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isPositionOverriddenJoint[5] &&
		leftKneePositionOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.positionOverrideJointID[5] = leftKneePositionOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1) // if JointsBase
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.positionOverrideJointID[5]);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::LeftKneeRotationOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isRotationOverriddenJoint[5] &&
		leftKneeRotationOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.rotationOverrideJointID[5] = leftKneeRotationOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1) // if JointsBase
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.rotationOverrideJointID[5]);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::RightKneePositionOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isPositionOverriddenJoint[6] &&
		rightKneePositionOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.positionOverrideJointID[6] = rightKneePositionOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1) // if JointsBase
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.positionOverrideJointID[6]);

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::RightKneeRotationOverrideOptionBox_SelectionChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	if (k2app::K2Settings.isRotationOverriddenJoint[6] &&
		rightKneeRotationOverrideOptionBox.get()->SelectedIndex() >= 0)
		k2app::K2Settings.rotationOverrideJointID[6] = rightKneeRotationOverrideOptionBox.get()->SelectedIndex();

	// If we're using a joints device then also signal the joint
	auto const& trackingDevicePair = TrackingDevices::getCurrentOverrideDevice_Safe();
	if (trackingDevicePair.first)
		if (trackingDevicePair.second.index() == 1) // if JointsBase
			std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevicePair.second)->
				signalJoint(k2app::K2Settings.rotationOverrideJointID[6]);

	// Save settings
	k2app::K2Settings.saveSettings();
}

/* For *Override* device type: override elements for joints selector */

void winrt::KinectToVR::implementation::DevicesPage::OverrideWaistPosition_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
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
	k2app::K2Settings.isPositionOverriddenJoint[0] = overrideWaistPosition.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	waistPositionOverrideOptionBox.get()->SelectedIndex(
		overrideWaistPosition.get()->IsChecked() ? k2app::K2Settings.positionOverrideJointID[0] : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideWaistRotation_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
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
	k2app::K2Settings.isRotationOverriddenJoint[0] = overrideWaistRotation.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	waistRotationOverrideOptionBox.get()->SelectedIndex(
		overrideWaistRotation.get()->IsChecked() ? k2app::K2Settings.rotationOverrideJointID[0] : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideLeftFootPosition_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
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
	k2app::K2Settings.isPositionOverriddenJoint[1] = overrideLeftFootPosition.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	leftFootPositionOverrideOptionBox.get()->SelectedIndex(
		overrideLeftFootPosition.get()->IsChecked() ? k2app::K2Settings.positionOverrideJointID[1] : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideLeftFootRotation_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
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
	k2app::K2Settings.isRotationOverriddenJoint[1] = overrideLeftFootRotation.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	leftFootRotationOverrideOptionBox.get()->SelectedIndex(
		overrideLeftFootRotation.get()->IsChecked() ? k2app::K2Settings.rotationOverrideJointID[1] : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideRightFootPosition_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
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
	k2app::K2Settings.isPositionOverriddenJoint[2] = overrideRightFootPosition.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	rightFootPositionOverrideOptionBox.get()->SelectedIndex(
		overrideRightFootPosition.get()->IsChecked() ? k2app::K2Settings.positionOverrideJointID[2] : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideRightFootRotation_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
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
	k2app::K2Settings.isRotationOverriddenJoint[2] = overrideRightFootRotation.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	rightFootRotationOverrideOptionBox.get()->SelectedIndex(
		overrideRightFootRotation.get()->IsChecked() ? k2app::K2Settings.rotationOverrideJointID[2] : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideLeftElbowPosition_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
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
	k2app::K2Settings.isPositionOverriddenJoint[3] = overrideLeftElbowPosition.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	leftElbowPositionOverrideOptionBox.get()->SelectedIndex(
		overrideLeftElbowPosition.get()->IsChecked() ? k2app::K2Settings.positionOverrideJointID[3] : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideLeftElbowRotation_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
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
	k2app::K2Settings.isRotationOverriddenJoint[3] = overrideLeftElbowRotation.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	leftElbowRotationOverrideOptionBox.get()->SelectedIndex(
		overrideLeftElbowRotation.get()->IsChecked() ? k2app::K2Settings.rotationOverrideJointID[3] : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideRightElbowPosition_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
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
	k2app::K2Settings.isPositionOverriddenJoint[4] = overrideRightElbowPosition.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	rightElbowPositionOverrideOptionBox.get()->SelectedIndex(
		overrideRightElbowPosition.get()->IsChecked() ? k2app::K2Settings.positionOverrideJointID[4] : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideRightElbowRotation_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
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
	k2app::K2Settings.isRotationOverriddenJoint[4] = overrideRightElbowRotation.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	rightElbowRotationOverrideOptionBox.get()->SelectedIndex(
		overrideRightElbowRotation.get()->IsChecked() ? k2app::K2Settings.rotationOverrideJointID[4] : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideLeftKneePosition_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
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
	k2app::K2Settings.isPositionOverriddenJoint[5] = overrideLeftKneePosition.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	leftKneePositionOverrideOptionBox.get()->SelectedIndex(
		overrideLeftKneePosition.get()->IsChecked() ? k2app::K2Settings.positionOverrideJointID[5] : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideLeftKneeRotation_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
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
	k2app::K2Settings.isRotationOverriddenJoint[5] = overrideLeftKneeRotation.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	leftKneeRotationOverrideOptionBox.get()->SelectedIndex(
		overrideLeftKneeRotation.get()->IsChecked() ? k2app::K2Settings.rotationOverrideJointID[5] : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideRightKneePosition_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
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
	k2app::K2Settings.isPositionOverriddenJoint[6] = overrideRightKneePosition.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	rightKneePositionOverrideOptionBox.get()->SelectedIndex(
		overrideRightKneePosition.get()->IsChecked() ? k2app::K2Settings.positionOverrideJointID[6] : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


void winrt::KinectToVR::implementation::DevicesPage::OverrideRightKneeRotation_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
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
	k2app::K2Settings.isRotationOverriddenJoint[6] = overrideRightKneeRotation.get()->IsChecked();

	// If we've disabled the override, set the placeholder text
	rightKneeRotationOverrideOptionBox.get()->SelectedIndex(
		overrideRightKneeRotation.get()->IsChecked() ? k2app::K2Settings.rotationOverrideJointID[6] : -1);

	// Check for errors and disable combos
	k2app::interfacing::devices_check_disabled_joints();

	// Save settings
	k2app::K2Settings.saveSettings();
}


/* For comboboxes: update before opening */

void winrt::KinectToVR::implementation::DevicesPage::WaistJointOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::LeftFootJointOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::RightFootJointOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::LeftElbowJointOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::RightElbowJointOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::LeftKneeJointOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::RightKneeJointOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::WaistPositionOverrideOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::WaistRotationOverrideOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::LeftFootPositionOverrideOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::LeftFootRotationOverrideOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::RightFootPositionOverrideOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::RightFootRotationOverrideOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::LeftElbowPositionOverrideOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::LeftElbowRotationOverrideOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::RightElbowPositionOverrideOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::RightElbowRotationOverrideOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::LeftKneePositionOverrideOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::LeftKneeRotationOverrideOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::RightKneePositionOverrideOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::RightKneeRotationOverrideOptionBox_DropDownOpened(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
}


void winrt::KinectToVR::implementation::DevicesPage::DismissOverrideTipNoJointsButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	NoJointsFlyout().Hide();
}


void winrt::KinectToVR::implementation::DevicesPage::DevicesPage_Loaded(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// If it's the first time loading, select the base device
	selectedTrackingDeviceID =
		devices_tab_setup_finished
			? devicesListView.get()->SelectedIndex()
			: k2app::K2Settings.trackingDeviceID;

	// Notify of the setup's end
	devices_tab_setup_finished = true;

	// Run the on-selected routine
	auto const& trackingDevice = TrackingDevices::TrackingDevicesVector.at(selectedTrackingDeviceID);

	std::string deviceName = "[UNKNOWN]";
	std::string device_status = "E_UKNOWN\nWhat's happened here?";

	// Only if override -> select enabled combos
	if (selectedTrackingDeviceID == k2app::K2Settings.overrideDeviceID)
	{
		overrideWaistPosition.get()->IsChecked(k2app::K2Settings.isPositionOverriddenJoint[0]);
		overrideLeftFootPosition.get()->IsChecked(k2app::K2Settings.isPositionOverriddenJoint[1]);
		overrideRightFootPosition.get()->IsChecked(k2app::K2Settings.isPositionOverriddenJoint[2]);

		overrideWaistRotation.get()->IsChecked(k2app::K2Settings.isRotationOverriddenJoint[0]);
		overrideLeftFootRotation.get()->IsChecked(k2app::K2Settings.isRotationOverriddenJoint[1]);
		overrideRightFootRotation.get()->IsChecked(k2app::K2Settings.isRotationOverriddenJoint[2]);

		overrideLeftElbowPosition.get()->IsChecked(k2app::K2Settings.isPositionOverriddenJoint[3]);
		overrideLeftElbowRotation.get()->IsChecked(k2app::K2Settings.isRotationOverriddenJoint[3]);

		overrideRightElbowPosition.get()->IsChecked(k2app::K2Settings.isPositionOverriddenJoint[4]);
		overrideRightElbowRotation.get()->IsChecked(k2app::K2Settings.isRotationOverriddenJoint[4]);

		overrideLeftKneePosition.get()->IsChecked(k2app::K2Settings.isPositionOverriddenJoint[5]);
		overrideLeftKneeRotation.get()->IsChecked(k2app::K2Settings.isRotationOverriddenJoint[5]);

		overrideRightKneePosition.get()->IsChecked(k2app::K2Settings.isPositionOverriddenJoint[6]);
		overrideRightKneeRotation.get()->IsChecked(k2app::K2Settings.isRotationOverriddenJoint[6]);
	}

	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		const auto device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

		deviceNameLabel.get()->Text(wstring_cast(device->getDeviceName()));
		device_status = device->statusResultString(device->getStatusResult());

		deviceName = device->getDeviceName();

		// We've selected a kinectbasis device, so this should be hidden
		jointBasisControls.get()->Visibility(Visibility::Collapsed);
		jointBasisControls_1.get()->Visibility(Visibility::Collapsed);
		jointBasisDropDown.get()->Visibility(Visibility::Collapsed);
		jointBasisDropDown_1.get()->Visibility(Visibility::Collapsed);
		jointBasisLabel.get()->Visibility(Visibility::Collapsed);

		// Set up combos if the device's OK
		if (device_status.find("S_OK") != std::string::npos)
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
				devices_check_override_ids(selectedTrackingDeviceID);

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

		deviceNameLabel.get()->Text(wstring_cast(device->getDeviceName()));
		device_status = device->statusResultString(device->getStatusResult());

		deviceName = device->getDeviceName();

		// We've selected a jointsbasis device, so this should be visible
		//	at least when the device is online
		jointBasisControls.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisLabel.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisControls_1.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisDropDown.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		jointBasisDropDown_1.get()->Visibility(
			(device_status.find("S_OK") != std::string::npos &&
				selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
				? Visibility::Visible
				: Visibility::Collapsed);

		// Set up combos if the device's OK
		if (device_status.find("S_OK") != std::string::npos)
		{
			// If we're reconnecting a base device, also refresh joints
			if (selectedTrackingDeviceID == k2app::K2Settings.trackingDeviceID)
			{
				// Clear items
				// Waist
				devices_clear_combo(waistJointOptionBox);
				// LeftF
				devices_clear_combo(leftFootJointOptionBox);
				// RightF
				devices_clear_combo(rightFootJointOptionBox);
				// LeftEL
				devices_clear_combo(leftElbowJointOptionBox);
				// RightEL
				devices_clear_combo(rightElbowJointOptionBox);
				// LeftK
				devices_clear_combo(leftKneeJointOptionBox);
				// RightK
				devices_clear_combo(rightKneeJointOptionBox);

				// Append all joints to all combos
				for (auto& _joint : device->getTrackedJoints())
				{
					// Get the name into string
					auto _jointname = _joint.getJointName();

					// Push the name to all combos
					// Waist
					waistJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// LeftF
					leftFootJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// RightF
					rightFootJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// LeftEL
					leftElbowJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// RightEL
					rightElbowJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// LeftK
					leftKneeJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
					// RightK
					rightKneeJointOptionBox.get()->Items().Append(box_value(wstring_cast(_jointname)));
				}

				// Check base IDs if wrong
				devices_check_base_ids(selectedTrackingDeviceID);

				// Select the first (or next, if exists) joint
				// Set the placeholder text on disabled combos
				// Waist
				waistJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[0]);
				// LeftF
				leftFootJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[1]);
				// RightF
				rightFootJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[2]);
				// LeftEL
				leftElbowJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[3]);
				// RightEL
				rightElbowJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[4]);
				// LeftEL
				leftKneeJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[5]);
				// RightEL
				rightKneeJointOptionBox.get()->SelectedIndex(k2app::K2Settings.selectedTrackedJointID[6]);
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
					devices_push_override_joints(wstring_cast(_jointname));
				}

				// Try fix override IDs if wrong
				devices_check_override_ids(selectedTrackingDeviceID);

				// Select the first (or next, if exists) joint
				// Set the placeholder text on disabled combos
				devices_select_combobox();
			}
		}
	}

	// Check if we've disabled any joints from spawning and disable they're mods
	k2app::interfacing::devices_check_disabled_joints();

	// Update the status here
	const bool status_ok = device_status.find("S_OK") != std::string::npos;

	errorWhatText.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	deviceErrorGrid.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);
	trackingDeviceErrorLabel.get()->Visibility(
		status_ok ? Visibility::Collapsed : Visibility::Visible);

	trackingDeviceChangePanel.get()->Visibility(
		status_ok ? Visibility::Visible : Visibility::Collapsed);

	// Split status and message by \n
	deviceStatusLabel.get()->Text(wstring_cast(split_status(device_status)[0]));
	trackingDeviceErrorLabel.get()->Text(wstring_cast(split_status(device_status)[1]));
	errorWhatText.get()->Text(wstring_cast(split_status(device_status)[2]));

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
}


void winrt::KinectToVR::implementation::DevicesPage::OverridesDropDown_Expanding(
	winrt::Microsoft::UI::Xaml::Controls::Expander const& sender,
	winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	overridesDropDown_1.get()->IsExpanded(false);
}


void winrt::KinectToVR::implementation::DevicesPage::OverridesDropDown_1_Expanding(
	winrt::Microsoft::UI::Xaml::Controls::Expander const& sender,
	winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	overridesDropDown.get()->IsExpanded(false);
}


void winrt::KinectToVR::implementation::DevicesPage::JointBasisDropDown_Expanding(
	winrt::Microsoft::UI::Xaml::Controls::Expander const& sender,
	winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	jointBasisDropDown_1.get()->IsExpanded(false);
}


void winrt::KinectToVR::implementation::DevicesPage::JointBasisDropDown_1_Expanding(
	winrt::Microsoft::UI::Xaml::Controls::Expander const& sender,
	winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& e)
{
	if (!devices_tab_setup_finished)return; // Don't even try if we're not set up yet
	jointBasisDropDown.get()->IsExpanded(false);
}


void winrt::KinectToVR::implementation::DevicesPage::OpenDiscordButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	ShellExecuteA(0, 0, "https://discord.gg/YBQCRDG", 0, 0, SW_SHOW);
}


void winrt::KinectToVR::implementation::DevicesPage::OpenDocsButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	ShellExecuteA(0, 0, "https://k2vr.tech/docs/", 0, 0, SW_SHOW);
}
