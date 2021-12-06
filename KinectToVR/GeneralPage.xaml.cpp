#include "pch.h"
#include "GeneralPage.xaml.h"
#if __has_include("GeneralPage.g.cpp")
#include "GeneralPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::KinectToVR::implementation
{
    GeneralPage::GeneralPage()
    {
        InitializeComponent();
    }
}

void winrt::KinectToVR::implementation::GeneralPage::OffsetsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    OffsetsView().IsPaneOpen(true);
}

void winrt::KinectToVR::implementation::GeneralPage::CalibrationButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    AutoCalibrationPane().Visibility(Visibility::Collapsed);
    ManualCalibrationPane().Visibility(Visibility::Collapsed);
    CalibrationSelectionPane().Visibility(Visibility::Visible);

    CalibrationView().IsPaneOpen(true);
}


void winrt::KinectToVR::implementation::GeneralPage::FlipCheckBox_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{

}


void winrt::KinectToVR::implementation::GeneralPage::FlipCheckBox_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{

}


void winrt::KinectToVR::implementation::GeneralPage::SkeletonToggleButton_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{

}


void winrt::KinectToVR::implementation::GeneralPage::SkeletonToggleButton_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{

}


void winrt::KinectToVR::implementation::GeneralPage::SaveOffsetsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    OffsetsView().IsPaneOpen(false);
}


void winrt::KinectToVR::implementation::GeneralPage::DiscardOffsetsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    OffsetsView().IsPaneOpen(false);
}


void winrt::KinectToVR::implementation::GeneralPage::OffsetsView_PaneClosing(winrt::Microsoft::UI::Xaml::Controls::SplitView const& sender, winrt::Microsoft::UI::Xaml::Controls::SplitViewPaneClosingEventArgs const& args)
{
    args.Cancel(true);
}


void winrt::KinectToVR::implementation::GeneralPage::CalibrationView_PaneClosing(winrt::Microsoft::UI::Xaml::Controls::SplitView const& sender, winrt::Microsoft::UI::Xaml::Controls::SplitViewPaneClosingEventArgs const& args)
{
    args.Cancel(true);
}


void winrt::KinectToVR::implementation::GeneralPage::AutoCalibrationButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    AutoCalibrationPane().Visibility(Visibility::Visible);
    ManualCalibrationPane().Visibility(Visibility::Collapsed);
    CalibrationSelectionPane().Visibility(Visibility::Collapsed);

    StartAutoCalibrationButton().IsEnabled(true);
    CalibrationInstructionsLabel().Text(L"Start the calibration");
    CalibrationCountdownLabel().Text(L"~");

    DiscardAutoCalibrationButton().Content(box_value(L"Cancel"));
}


void winrt::KinectToVR::implementation::GeneralPage::ManualCalibrationButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    AutoCalibrationPane().Visibility(Visibility::Collapsed);
    ManualCalibrationPane().Visibility(Visibility::Visible);
    CalibrationSelectionPane().Visibility(Visibility::Collapsed);

    StartManualCalibrationButton().IsEnabled(true);
    DiscardManualCalibrationButton().Content(box_value(L"Cancel"));
}

Windows::Foundation::IAsyncAction winrt::KinectToVR::implementation::GeneralPage::StartAutoCalibrationButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    // Set the [calibration pending] bool
    CalibrationPending = true;
    
    // Disable the start button and change [cancel]'s text
    StartAutoCalibrationButton().IsEnabled(false);
    DiscardAutoCalibrationButton().Content(box_value(L"Abort"));

    // Loop over total 3 points (may change)
    for (int point = 0; point < 3; point++) {
        // Wait for the user to move
        CalibrationInstructionsLabel().Text(L"Move somewhere else");
        for (int i = 3; i >= 0; i--) {
            CalibrationCountdownLabel().Text(std::to_wstring(i));
            if (!CalibrationPending)break; // Check for exiting

            { // Sleep on UI
                winrt::apartment_context ui_thread;
                co_await winrt::resume_background();
                Sleep(1000);
                co_await ui_thread;
            }
            if (!CalibrationPending)break; // Check for exiting
        }

        CalibrationInstructionsLabel().Text(L"Stand still!");
        for (int i = 3; i >= 0; i--) {
            CalibrationCountdownLabel().Text(std::to_wstring(i));
            if (!CalibrationPending)break; // Check for exiting

            // Capture user's position at [1]
            if(i==1)
            {
	            // Capture
            }

            // Wait and eventually break
            { // Sleep on UI
                winrt::apartment_context ui_thread;
                co_await winrt::resume_background();
                Sleep(1000);
                co_await ui_thread;
            }
            if (!CalibrationPending)break; // Check for exiting
        }
        
        // Exit if aborted
        if (!CalibrationPending)break;
    }

	// Notify that we're finished
	CalibrationInstructionsLabel().Text(
        CalibrationPending ? L"Calibration done!" : L"Calibration aborted!");
	CalibrationCountdownLabel().Text(L"~");
    { // Sleep on UI
        winrt::apartment_context ui_thread;
        co_await winrt::resume_background();
        Sleep(2200); // Just right
        co_await ui_thread;
    }

    // Exit the pane
    CalibrationView().IsPaneOpen(false);
}


void winrt::KinectToVR::implementation::GeneralPage::DiscardAutoCalibrationButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    // Just exit
    if(!CalibrationPending) CalibrationView().IsPaneOpen(false);
    // Begin abort
    else CalibrationPending = false;
}


Windows::Foundation::IAsyncAction winrt::KinectToVR::implementation::GeneralPage::StartManualCalibrationButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    // Set the [calibration pending] bool
    CalibrationPending = true;

    // Disable the start button and change [cancel]'s text
    StartManualCalibrationButton().IsEnabled(false);
    DiscardManualCalibrationButton().Content(box_value(L"Abort"));

    // Loop over until finished
    while(true /*!confirm*/) {
        
        // Wait for a mode switch
        while(true /*!modeswap && !confirm*/)
        {
            // TMP
            { // Sleep on UI
                winrt::apartment_context ui_thread;
                co_await winrt::resume_background();
                Sleep(1000);
                co_await ui_thread;
            }
            
            // Exit if aborted
            if (!CalibrationPending)break;
        }

    	// Wait for a mode switch
        while(true /*!modeswap && !confirm*/)
        {
            // TMP
            { // Sleep on UI
                winrt::apartment_context ui_thread;
                co_await winrt::resume_background();
                Sleep(1000);
                co_await ui_thread;
            }

            // Exit if aborted
            if (!CalibrationPending)break;
        }
        
        // Exit if aborted
        if (!CalibrationPending)break;
    }

    { // Sleep on UI
        winrt::apartment_context ui_thread;
        co_await winrt::resume_background();
        Sleep(1000); // Just right
        co_await ui_thread;
    }
    // Exit the pane
    CalibrationView().IsPaneOpen(false);
}


void winrt::KinectToVR::implementation::GeneralPage::DiscardManualCalibrationButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    // Just exit
    if (!CalibrationPending) CalibrationView().IsPaneOpen(false);
    // Begin abort
    else CalibrationPending = false;
}


void winrt::KinectToVR::implementation::GeneralPage::ToggleTrackersButton_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    ToggleTrackersButton().Content(box_value(L"Disconnect Trackers"));
}


void winrt::KinectToVR::implementation::GeneralPage::ToggleTrackersButton_Unchecked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    ToggleTrackersButton().Content(box_value(L"Reconnect Trackers"));
}
