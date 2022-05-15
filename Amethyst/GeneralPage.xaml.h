#pragma once

#include "GeneralPage.g.h"

#include "TrackingDevices.h"
#include "K2Shared.h"
#include "K2Interfacing.h"

namespace winrt::KinectToVR::implementation
{
	struct GeneralPage : GeneralPageT<GeneralPage>
	{
		// Helper defines
		bool CalibrationPending = false;
		void sk_line(Microsoft::UI::Xaml::Shapes::Line& line, const std::array<Eigen::Vector3f, 25>& joints,
		             const std::array<ktvr::JointTrackingState, 25>& states,
		             const ktvr::ITrackedJointType& from, const ktvr::ITrackedJointType& to);

		void sk_dot(Microsoft::UI::Xaml::Shapes::Ellipse& ellipse,
		            const Eigen::Vector3f& joint,
		            const ktvr::JointTrackingState& state, const std::pair<bool, bool>& isOverridden);

		// WinUI things
		GeneralPage();

		void OffsetsButton_Click(const Windows::Foundation::IInspectable& sender,
		                         const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void SkeletonToggleButton_Click(const Windows::Foundation::IInspectable& sender,
		                                const winrt::Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs& e);
		void ForceRenderCheckBox_Checked(const Windows::Foundation::IInspectable& sender,
		                                 const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ForceRenderCheckBox_Unchecked(const Windows::Foundation::IInspectable& sender,
		                                   const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void SaveOffsetsButton_Click(const Windows::Foundation::IInspectable& sender,
		                             const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DiscardOffsetsButton_Click(const Windows::Foundation::IInspectable& sender,
		                                const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OffsetsView_PaneClosing(const winrt::Microsoft::UI::Xaml::Controls::SplitView& sender,
		                             const winrt::Microsoft::UI::Xaml::Controls::SplitViewPaneClosingEventArgs& args);
		void CalibrationView_PaneClosing(const winrt::Microsoft::UI::Xaml::Controls::SplitView& sender,
		                                 const winrt::Microsoft::UI::Xaml::Controls::SplitViewPaneClosingEventArgs&
		                                 args);
		void AutoCalibrationButton_Click(const Windows::Foundation::IInspectable& sender,
		                                 const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ManualCalibrationButton_Click(const Windows::Foundation::IInspectable& sender,
		                                   const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		Windows::Foundation::IAsyncAction StartAutoCalibrationButton_Click(
			const Windows::Foundation::IInspectable& sender,
			const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DiscardAutoCalibrationButton_Click(const Windows::Foundation::IInspectable& sender,
		                                        const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		Windows::Foundation::IAsyncAction StartManualCalibrationButton_Click(
			const Windows::Foundation::IInspectable& sender,
			const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void DiscardManualCalibrationButton_Click(const Windows::Foundation::IInspectable& sender,
		                                          const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ToggleTrackersButton_Checked(const Windows::Foundation::IInspectable& sender,
		                                  const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ToggleTrackersButton_Unchecked(const Windows::Foundation::IInspectable& sender,
		                                    const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OpenDiscordButton_Click(const Windows::Foundation::IInspectable& sender,
		                             const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OpenDocsButton_Click(const Windows::Foundation::IInspectable& sender,
		                          const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void ServerOpenDocsButton_Click(const Windows::Foundation::IInspectable& sender,
		                                const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void GeneralPage_Loaded(const Windows::Foundation::IInspectable& sender,
		                        const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void SkeletonDrawingCanvas_Loaded(const Windows::Foundation::IInspectable& sender,
		                                  const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void CalibrationButton_Click(const Windows::Foundation::IInspectable& sender,
		                             const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void BaseCalibration_Click(const Windows::Foundation::IInspectable& sender,
		                           const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OverrideCalibration_Click(const Windows::Foundation::IInspectable& sender,
		                               const winrt::Microsoft::UI::Xaml::RoutedEventArgs& e);
		void OffsetsFrontendValueChanged(const Windows::Foundation::IInspectable& sender,
		                                 const winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs& e);
	};
}

namespace winrt::KinectToVR::factory_implementation
{
	struct GeneralPage : GeneralPageT<GeneralPage, implementation::GeneralPage>
	{
	};
}
