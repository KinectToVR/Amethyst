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

		void sk_line(Microsoft::UI::Xaml::Shapes::Line& line, const std::array<Eigen::Vector3f, 25>& joints,
		             const std::array<ktvr::JointTrackingState, 25>& states,
		             const ktvr::ITrackedJointType& from, const ktvr::ITrackedJointType& to);

		void sk_dot(Microsoft::UI::Xaml::Shapes::Ellipse& ellipse,
		            const Eigen::Vector3f& joint,
		            const ktvr::JointTrackingState& state, const std::pair<bool, bool>& isOverridden);

		// WinUI things
		GeneralPage();

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
		void AutoCalibrationButton_Click(const Windows::Foundation::IInspectable& sender,
		                                 const Microsoft::UI::Xaml::RoutedEventArgs& e);
		Windows::Foundation::IAsyncAction ManualCalibrationButton_Click(const Windows::Foundation::IInspectable& sender,
		                                                                const
		                                                                Microsoft::UI::Xaml::RoutedEventArgs& e);
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
		void BaseCalibration_Click(const Windows::Foundation::IInspectable& sender,
		                           const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OverrideCalibration_Click(const Windows::Foundation::IInspectable& sender,
		                               const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ReRegisterButton_Click(const Windows::Foundation::IInspectable& sender,
		                            const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DismissSetErrorButton_Click(const Windows::Foundation::IInspectable& sender,
		                                 const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ToggleTrackingButton_Click(const Microsoft::UI::Xaml::Controls::SplitButton& sender,
		                                const Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs& args);
		void FreezeOnlyLowerCheckBox_Checked(const Windows::Foundation::IInspectable& sender,
		                                     const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void FreezeOnlyLowerCheckBox_Unchecked(const Windows::Foundation::IInspectable& sender,
		                                       const Microsoft::UI::Xaml::RoutedEventArgs& e);
		void CalibrationSelectView_PaneClosing(const Microsoft::UI::Xaml::Controls::SplitView& sender,
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
		                                      const Microsoft::UI::Xaml::Controls::TeachingTipClosedEventArgs& args);
		Windows::Foundation::IAsyncAction StatusTeachingTip_Closed(
			const Microsoft::UI::Xaml::Controls::TeachingTip& sender,
			const Microsoft::UI::Xaml::Controls::TeachingTipClosedEventArgs& args);
		void CalibrationTeachingTip_Closed(const Microsoft::UI::Xaml::Controls::TeachingTip& sender,
		                                   const Microsoft::UI::Xaml::Controls::TeachingTipClosedEventArgs& args);
		void ToggleButtonFlyout_Opening(const Windows::Foundation::IInspectable& sender,
		                                const Windows::Foundation::IInspectable& e);
		void ToggleButtonFlyout_Closing(const Microsoft::UI::Xaml::Controls::Primitives::FlyoutBase& sender,
		                                const Microsoft::UI::Xaml::Controls::Primitives::FlyoutBaseClosingEventArgs&
		                                args);
	};
}

namespace winrt::Amethyst::factory_implementation
{
	struct GeneralPage : GeneralPageT<GeneralPage, implementation::GeneralPage>
	{
	};
}
