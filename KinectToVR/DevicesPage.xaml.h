#pragma once
#include "DevicesPage.g.h"
#include "TrackingDevicesView.h"

#include "TrackingDevices.h"
#include "K2Shared.h"

namespace winrt::KinectToVR::implementation
{
	struct DevicesPage : DevicesPageT<DevicesPage>
	{
		DevicesPage();

		void TrackingDeviceListView_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender,
		                                             winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                             const& e);
		void SetAsOverrideButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void SetAsBaseButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void DisconnectDeviceButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void DeselectDeviceButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void ReconnectDeviceButton_Click(winrt::Microsoft::UI::Xaml::Controls::SplitButton const& sender, winrt::Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs const& args);
		void WaistJointOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void LeftFootJointOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void RightFootJointOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void LeftElbowJointOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void RightElbowJointOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void LeftKneeJointOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void RightKneeJointOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void WaistRotationOverrideOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void WaistPositionOverrideOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void RightFootPositionOverrideOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void RightFootRotationOverrideOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void LeftFootRotationOverrideOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void LeftFootPositionOverrideOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void LeftElbowRotationOverrideOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void LeftElbowPositionOverrideOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void RightElbowRotationOverrideOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void RightElbowPositionOverrideOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void LeftKneeRotationOverrideOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void LeftKneePositionOverrideOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void RightKneeRotationOverrideOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void RightKneePositionOverrideOptionBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void OverrideWaistPosition_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void OverrideWaistRotation_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void OverrideLeftFootPosition_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void OverrideLeftFootRotation_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void OverrideRightFootPosition_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void OverrideRightFootRotation_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void OverrideLeftElbowPosition_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void OverrideLeftElbowRotation_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void OverrideRightElbowPosition_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void OverrideRightElbowRotation_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void OverrideLeftKneePosition_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void OverrideLeftKneeRotation_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void OverrideRightKneePosition_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void OverrideRightKneeRotation_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void WaistPositionOverrideOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void WaistRotationOverrideOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void LeftFootPositionOverrideOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void LeftFootRotationOverrideOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void RightFootPositionOverrideOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void RightFootRotationOverrideOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void LeftElbowPositionOverrideOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void LeftElbowRotationOverrideOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void RightElbowPositionOverrideOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void RightElbowRotationOverrideOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void LeftKneePositionOverrideOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void LeftKneeRotationOverrideOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void RightKneePositionOverrideOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void RightKneeRotationOverrideOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void WaistJointOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void LeftFootJointOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void RightFootJointOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void LeftElbowJointOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void RightElbowJointOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void LeftKneeJointOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void RightKneeJointOptionBox_DropDownOpened(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
		void DismissOverrideTipNoJointsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void DevicesPage_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void OverridesDropDown_Expanding(winrt::Microsoft::UI::Xaml::Controls::Expander const& sender, winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& e);
		void OverridesDropDown_1_Expanding(winrt::Microsoft::UI::Xaml::Controls::Expander const& sender, winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& e);
		void JointBasisDropDown_Expanding(winrt::Microsoft::UI::Xaml::Controls::Expander const& sender, winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& e);
		void JointBasisDropDown_1_Expanding(winrt::Microsoft::UI::Xaml::Controls::Expander const& sender, winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs const& e);
	};
}

namespace winrt::KinectToVR::factory_implementation
{
	struct DevicesPage : DevicesPageT<DevicesPage, implementation::DevicesPage>
	{
	};
}
