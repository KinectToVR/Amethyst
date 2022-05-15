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

		Windows::Foundation::IAsyncAction TrackingDeviceListView_SelectionChanged(
			const Windows::Foundation::IInspectable& sender,
			const winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
			& e);
		Windows::Foundation::IAsyncAction SetAsOverrideButton_Click(const Windows::Foundation::IInspectable& sender,
		                                                            const winrt::Microsoft::UI::Xaml::RoutedEventArgs&
		                                                            e);
		Windows::Foundation::IAsyncAction SetAsBaseButton_Click(const Windows::Foundation::IInspectable& sender,
		                                                        const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DisconnectDeviceButton_Click(const Windows::Foundation::IInspectable& sender,
		                                  const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DeselectDeviceButton_Click(const Windows::Foundation::IInspectable& sender,
		                                const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ReconnectDeviceButton_Click(const winrt::Microsoft::UI::Xaml::Controls::SplitButton& sender,
		                                 const winrt::Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs& args);
		void WaistJointOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                          const winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs&
		                                          e);
		void LeftFootJointOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                             const
		                                             winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs&
		                                             e);
		void RightFootJointOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                              const
		                                              winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs&
		                                              e);
		void LeftElbowJointOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                              const
		                                              winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs&
		                                              e);
		void RightElbowJointOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                               const
		                                               winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs&
		                                               e);
		void LeftKneeJointOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                             const
		                                             winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs&
		                                             e);
		void RightKneeJointOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                              const
		                                              winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs&
		                                              e);
		void WaistRotationOverrideOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                     const
		                                                     winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                     & e);
		void WaistPositionOverrideOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                     const
		                                                     winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                     & e);
		void RightFootPositionOverrideOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                         const
		                                                         winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                         & e);
		void RightFootRotationOverrideOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                         const
		                                                         winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                         & e);
		void LeftFootRotationOverrideOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                        const
		                                                        winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                        & e);
		void LeftFootPositionOverrideOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                        const
		                                                        winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                        & e);
		void LeftElbowRotationOverrideOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                         const
		                                                         winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                         & e);
		void LeftElbowPositionOverrideOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                         const
		                                                         winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                         & e);
		void RightElbowRotationOverrideOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                          const
		                                                          winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                          & e);
		void RightElbowPositionOverrideOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                          const
		                                                          winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                          & e);
		void LeftKneeRotationOverrideOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                        const
		                                                        winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                        & e);
		void LeftKneePositionOverrideOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                        const
		                                                        winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                        & e);
		void RightKneeRotationOverrideOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                         const
		                                                         winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                         & e);
		void RightKneePositionOverrideOptionBox_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                                         const
		                                                         winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs
		                                                         & e);
		void OverrideWaistPosition_Click(const Windows::Foundation::IInspectable& sender,
		                                 const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OverrideWaistRotation_Click(const Windows::Foundation::IInspectable& sender,
		                                 const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OverrideLeftFootPosition_Click(const Windows::Foundation::IInspectable& sender,
		                                    const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OverrideLeftFootRotation_Click(const Windows::Foundation::IInspectable& sender,
		                                    const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OverrideRightFootPosition_Click(const Windows::Foundation::IInspectable& sender,
		                                     const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OverrideRightFootRotation_Click(const Windows::Foundation::IInspectable& sender,
		                                     const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OverrideLeftElbowPosition_Click(const Windows::Foundation::IInspectable& sender,
		                                     const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OverrideLeftElbowRotation_Click(const Windows::Foundation::IInspectable& sender,
		                                     const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OverrideRightElbowPosition_Click(const Windows::Foundation::IInspectable& sender,
		                                      const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OverrideRightElbowRotation_Click(const Windows::Foundation::IInspectable& sender,
		                                      const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OverrideLeftKneePosition_Click(const Windows::Foundation::IInspectable& sender,
		                                    const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OverrideLeftKneeRotation_Click(const Windows::Foundation::IInspectable& sender,
		                                    const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OverrideRightKneePosition_Click(const Windows::Foundation::IInspectable& sender,
		                                     const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OverrideRightKneeRotation_Click(const Windows::Foundation::IInspectable& sender,
		                                     const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void WaistPositionOverrideOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                                   const Windows::Foundation::IInspectable& e);
		void WaistRotationOverrideOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                                   const Windows::Foundation::IInspectable& e);
		void LeftFootPositionOverrideOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                                      const Windows::Foundation::IInspectable& e);
		void LeftFootRotationOverrideOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                                      const Windows::Foundation::IInspectable& e);
		void RightFootPositionOverrideOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                                       const Windows::Foundation::IInspectable& e);
		void RightFootRotationOverrideOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                                       const Windows::Foundation::IInspectable& e);
		void LeftElbowPositionOverrideOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                                       const Windows::Foundation::IInspectable& e);
		void LeftElbowRotationOverrideOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                                       const Windows::Foundation::IInspectable& e);
		void RightElbowPositionOverrideOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                                        const Windows::Foundation::IInspectable& e);
		void RightElbowRotationOverrideOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                                        const Windows::Foundation::IInspectable& e);
		void LeftKneePositionOverrideOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                                      const Windows::Foundation::IInspectable& e);
		void LeftKneeRotationOverrideOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                                      const Windows::Foundation::IInspectable& e);
		void RightKneePositionOverrideOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                                       const Windows::Foundation::IInspectable& e);
		void RightKneeRotationOverrideOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                                       const Windows::Foundation::IInspectable& e);
		void WaistJointOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                        const Windows::Foundation::IInspectable& e);
		void LeftFootJointOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                           const Windows::Foundation::IInspectable& e);
		void RightFootJointOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                            const Windows::Foundation::IInspectable& e);
		void LeftElbowJointOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                            const Windows::Foundation::IInspectable& e);
		void RightElbowJointOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                             const Windows::Foundation::IInspectable& e);
		void LeftKneeJointOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                           const Windows::Foundation::IInspectable& e);
		void RightKneeJointOptionBox_DropDownOpened(const Windows::Foundation::IInspectable& sender,
		                                            const Windows::Foundation::IInspectable& e);
		void DismissOverrideTipNoJointsButton_Click(const Windows::Foundation::IInspectable& sender,
		                                            const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DevicesPage_Loaded(const Windows::Foundation::IInspectable& sender,
		                        const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OverridesDropDown_Expanding(const winrt::Microsoft::UI::Xaml::Controls::Expander& sender,
		                                 const winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs& e);
		void OverridesDropDown_1_Expanding(const winrt::Microsoft::UI::Xaml::Controls::Expander& sender,
		                                   const winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs& e);
		void JointBasisDropDown_Expanding(const winrt::Microsoft::UI::Xaml::Controls::Expander& sender,
		                                  const winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs& e);
		void JointBasisDropDown_1_Expanding(const winrt::Microsoft::UI::Xaml::Controls::Expander& sender,
		                                    const winrt::Microsoft::UI::Xaml::Controls::ExpanderExpandingEventArgs& e);
		void OpenDiscordButton_Click(const Windows::Foundation::IInspectable& sender,
		                             const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OpenDocsButton_Click(const Windows::Foundation::IInspectable& sender,
		                          const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void SelectedDeviceSettingsButton_Click(const Windows::Foundation::IInspectable& sender,
		                                        const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
	};
}

namespace winrt::KinectToVR::factory_implementation
{
	struct DevicesPage : DevicesPageT<DevicesPage, implementation::DevicesPage>
	{
	};
}
