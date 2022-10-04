#pragma once

#include "GeneralPage.g.h"

#include "TrackingDevices.h"
#include "K2Shared.h"
#include "K2Interfacing.h"

#include "OffsetsPivotItem.h"
#include "OffsetsController.h"

namespace winrt::Amethyst::implementation
{
	struct GeneralPage : GeneralPageT<GeneralPage>
	{
		// Helper defines
		bool CalibrationPending = false; // If we're running a calibration
		bool AutoCalibration_StillPending = false; // If calibration panes are still opened

		void sk_line(Microsoft::UI::Xaml::Shapes::Line& line,
		             std::array<ktvr::K2TrackedBaseJoint, 25> joints,
		             const ktvr::ITrackedJointType& from, const ktvr::ITrackedJointType& to);

		void sk_dot(Microsoft::UI::Xaml::Shapes::Ellipse& ellipse,
		            ktvr::K2TrackedBaseJoint joint,
		            const std::pair<bool, bool>& isOverridden);

		// WinUI things
		GeneralPage();

		Windows::Foundation::IAsyncAction ExecuteManualCalibration();
		Windows::Foundation::IAsyncAction ExecuteAutomaticCalibration();

		void AllowNavigation(const bool& allow);

		void OffsetsButton_Click(const Windows::Foundation::IInspectable& sender,
		                         const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void SkeletonToggleButton_Click(const Windows::Foundation::IInspectable& sender,
		                                const Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs& e);
		void ForceRenderCheckBox_Checked(const Windows::Foundation::IInspectable& sender,
		                                 const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ForceRenderCheckBox_Unchecked(const Windows::Foundation::IInspectable& sender,
		                                   const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void SaveOffsetsButton_Click(const Windows::Foundation::IInspectable& sender,
		                             const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DiscardOffsetsButton_Click(const Windows::Foundation::IInspectable& sender,
		                                const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OffsetsView_PaneClosing(const Microsoft::UI::Xaml::Controls::SplitView& sender,
		                             const Microsoft::UI::Xaml::Controls::SplitViewPaneClosingEventArgs& args);
		void CalibrationView_PaneClosing(const Microsoft::UI::Xaml::Controls::SplitView& sender,
		                                 const Microsoft::UI::Xaml::Controls::SplitViewPaneClosingEventArgs&
		                                 args);
		Windows::Foundation::IAsyncAction AutoCalibrationButton_Click(
			const Windows::Foundation::IInspectable& sender,
			const Microsoft::UI::Xaml::RoutedEventArgs& e);
		Windows::Foundation::IAsyncAction ManualCalibrationButton_Click(
			const Windows::Foundation::IInspectable& sender,
			const Microsoft::UI::Xaml::RoutedEventArgs& e);
		Windows::Foundation::IAsyncAction StartAutoCalibrationButton_Click(
			const Windows::Foundation::IInspectable& sender,
			const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DiscardCalibrationButton_Click(const Windows::Foundation::IInspectable& sender,
		                                    const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ToggleTrackersButton_Checked(const Windows::Foundation::IInspectable& sender,
		                                  const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ToggleTrackersButton_Unchecked(const Windows::Foundation::IInspectable& sender,
		                                    const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OpenDiscordButton_Click(const Windows::Foundation::IInspectable& sender,
		                             const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OpenDocsButton_Click(const Windows::Foundation::IInspectable& sender,
		                          const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ServerOpenDocsButton_Click(const Windows::Foundation::IInspectable& sender,
		                                const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void GeneralPage_Loaded(const Windows::Foundation::IInspectable& sender,
		                        const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void GeneralPage_Loaded_Handler();
		void SkeletonDrawingCanvas_Loaded(const Windows::Foundation::IInspectable& sender,
		                                  const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void CalibrationButton_Click(const Windows::Foundation::IInspectable& sender,
									 const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ReRegisterButton_Click(const Windows::Foundation::IInspectable& sender,
		                            const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DismissSetErrorButton_Click(const Windows::Foundation::IInspectable& sender,
		                                 const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ToggleTrackingButton_Click(const Windows::Foundation::IInspectable& sender,
		                                const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void CalibrationDeviceSelectView_PaneClosing(const Microsoft::UI::Xaml::Controls::SplitView& sender,
		                                             const Microsoft::UI::Xaml::Controls::SplitViewPaneClosingEventArgs
		                                             & args);
		void CalibrationModeSelectView_PaneClosing(const Microsoft::UI::Xaml::Controls::SplitView& sender,
		                                           const Microsoft::UI::Xaml::Controls::SplitViewPaneClosingEventArgs
		                                           & args);
		void CalibrationRunningView_PaneClosing(const Microsoft::UI::Xaml::Controls::SplitView& sender,
		                                        const
		                                        Microsoft::UI::Xaml::Controls::SplitViewPaneClosingEventArgs&
		                                        args);
		void CalibrationPointsNumberBox_ValueChanged(const Microsoft::UI::Xaml::Controls::NumberBox& sender,
		                                             const
		                                             Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs
		                                             & args);
		void ToggleTrackersTeachingTip_Closed(const Microsoft::UI::Xaml::Controls::TeachingTip& sender,
		                                      const Windows::Foundation::IInspectable& args);
		Windows::Foundation::IAsyncAction StatusTeachingTip_Closed(
			const Microsoft::UI::Xaml::Controls::TeachingTip& sender,
			const Windows::Foundation::IInspectable& args);
		void CalibrationTeachingTip_Closed(const Microsoft::UI::Xaml::Controls::TeachingTip& sender,
		                                   const Windows::Foundation::IInspectable& args);
		void ToggleButtonFlyout_Opening(const Windows::Foundation::IInspectable& sender,
		                                const Windows::Foundation::IInspectable& e);
		void ToggleButtonFlyout_Closing(const Microsoft::UI::Xaml::Controls::Primitives::FlyoutBase& sender,
		                                const Microsoft::UI::Xaml::Controls::Primitives::FlyoutBaseClosingEventArgs&
		                                args);
		void ToggleTrackersTeachingTip_ActionButtonClick(const Microsoft::UI::Xaml::Controls::TeachingTip& sender,
		                                                 const Windows::Foundation::IInspectable& args);
		void CalibrationTeachingTip_ActionButtonClick(const Microsoft::UI::Xaml::Controls::TeachingTip& sender,
		                                              const Windows::Foundation::IInspectable& args);
		void StatusTeachingTip_ActionButtonClick(const Microsoft::UI::Xaml::Controls::TeachingTip& sender,
		                                         const Windows::Foundation::IInspectable& args);
		void FreezeOnlyLowerToggle_Click(const Windows::Foundation::IInspectable& sender,
		                                 const Microsoft::UI::Xaml::RoutedEventArgs& e);
		Windows::Foundation::IAsyncAction AdditionalDeviceErrorsHyperlink_Tapped(
			const Windows::Foundation::IInspectable& sender,
			const Microsoft::UI::Xaml::Input::TappedRoutedEventArgs& e);
		Windows::Foundation::IAsyncAction TrackingDeviceTreeView_ItemInvoked(
			const Microsoft::UI::Xaml::Controls::TreeView& sender,
			const Microsoft::UI::Xaml::Controls::TreeViewItemInvokedEventArgs& args);
		void NoCalibrationTeachingTip_Closed(
			const Microsoft::UI::Xaml::Controls::TeachingTip& sender, 
			const Microsoft::UI::Xaml::Controls::TeachingTipClosedEventArgs& args);
};
}

namespace winrt::Amethyst::factory_implementation
{
	struct GeneralPage : GeneralPageT<GeneralPage, implementation::GeneralPage>
	{
	};
}
