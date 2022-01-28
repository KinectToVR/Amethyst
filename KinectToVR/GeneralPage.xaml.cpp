#include "pch.h"
#include "GeneralPage.xaml.h"
#if __has_include("GeneralPage.g.cpp")
#include "GeneralPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.
bool show_skeleton_current = true,
     show_skeleton_previous = true,
     general_tab_setup_finished = false,
     pending_offsets_update = false;

namespace winrt::KinectToVR::implementation
{
	GeneralPage::GeneralPage()
	{
		InitializeComponent();

		// Cache needed UI elements
		using namespace ::k2app::shared::general;

		toggleTrackersButton = std::make_shared<Controls::Primitives::ToggleButton>(ToggleTrackersButton());

		calibrationButton = std::make_shared<Controls::Button>(CalibrationButton());
		offsetsButton = std::make_shared<Controls::Button>(OffsetsButton());

		deviceNameLabel = std::make_shared<Controls::TextBlock>(SelectedDeviceNameLabel());
		deviceStatusLabel = std::make_shared<Controls::TextBlock>(TrackingDeviceStatusLabel());
		errorWhatText = std::make_shared<Controls::TextBlock>(ErrorWhatText());
		trackingDeviceErrorLabel = std::make_shared<Controls::TextBlock>(TrackingDeviceErrorLabel());
		serverStatusLabel = std::make_shared<Controls::TextBlock>(ServerStatusLabel());
		serverErrorLabel = std::make_shared<Controls::TextBlock>(ServerErrorLabel());
		serverErrorWhatText = std::make_shared<Controls::TextBlock>(ServerErrorWhatText());

		errorButtonsGrid = std::make_shared<Controls::Grid>(ErrorButtonsGrid());
		errorWhatGrid = std::make_shared<Controls::Grid>(ErrorWhatGrid());
		serverErrorWhatGrid = std::make_shared<Controls::Grid>(ServerErrorWhatGrid());
		serverErrorButtonsGrid = std::make_shared<Controls::Grid>(ServerErrorButtonsGrid());

		waistRollNumberBox = std::make_shared<Controls::NumberBox>(WaistRollNumberBox());
		waistYawNumberBox = std::make_shared<Controls::NumberBox>(WaistYawNumberBox());
		waistPitchNumberBox = std::make_shared<Controls::NumberBox>(WaistPitchNumberBox());
		waistXNumberBox = std::make_shared<Controls::NumberBox>(WaistXNumberBox());
		waistYNumberBox = std::make_shared<Controls::NumberBox>(WaistYNumberBox());
		waistZNumberBox = std::make_shared<Controls::NumberBox>(WaistZNumberBox());

		leftFootRollNumberBox = std::make_shared<Controls::NumberBox>(LeftFootRollNumberBox());
		leftFootYawNumberBox = std::make_shared<Controls::NumberBox>(LeftFootYawNumberBox());
		leftFootPitchNumberBox = std::make_shared<Controls::NumberBox>(LeftFootPitchNumberBox());
		leftFootXNumberBox = std::make_shared<Controls::NumberBox>(LeftFootXNumberBox());
		leftFootYNumberBox = std::make_shared<Controls::NumberBox>(LeftFootYNumberBox());
		leftFootZNumberBox = std::make_shared<Controls::NumberBox>(LeftFootZNumberBox());

		rightFootRollNumberBox = std::make_shared<Controls::NumberBox>(RightFootRollNumberBox());
		rightFootYawNumberBox = std::make_shared<Controls::NumberBox>(RightFootYawNumberBox());
		rightFootPitchNumberBox = std::make_shared<Controls::NumberBox>(RightFootPitchNumberBox());
		rightFootXNumberBox = std::make_shared<Controls::NumberBox>(RightFootXNumberBox());
		rightFootYNumberBox = std::make_shared<Controls::NumberBox>(RightFootYNumberBox());
		rightFootZNumberBox = std::make_shared<Controls::NumberBox>(RightFootZNumberBox());
	}
}

void winrt::KinectToVR::implementation::GeneralPage::OffsetsButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Push saved offsets' by reading them from settings
	k2app::K2Settings.readSettings();

	// Notice that we're gonna change some values
	pending_offsets_update = true;

	k2app::shared::general::waistXNumberBox.get()->Value(
		int(k2app::K2Settings.positionJointsOffsets[0].x() * 100.0));
	k2app::shared::general::waistYNumberBox.get()->Value(
		int(k2app::K2Settings.positionJointsOffsets[0].y() * 100.0));
	k2app::shared::general::waistZNumberBox.get()->Value(
		int(k2app::K2Settings.positionJointsOffsets[0].z() * 100.0));

	k2app::shared::general::leftFootXNumberBox.get()->Value(
		int(k2app::K2Settings.positionJointsOffsets[1].x() * 100.0));
	k2app::shared::general::leftFootYNumberBox.get()->Value(
		int(k2app::K2Settings.positionJointsOffsets[1].y() * 100.0));
	k2app::shared::general::leftFootZNumberBox.get()->Value(
		int(k2app::K2Settings.positionJointsOffsets[1].z() * 100.0));

	k2app::shared::general::rightFootXNumberBox.get()->Value(
		int(k2app::K2Settings.positionJointsOffsets[2].x() * 100.0));
	k2app::shared::general::rightFootYNumberBox.get()->Value(
		int(k2app::K2Settings.positionJointsOffsets[2].y() * 100.0));
	k2app::shared::general::rightFootZNumberBox.get()->Value(
		int(k2app::K2Settings.positionJointsOffsets[2].z() * 100.0));

	k2app::shared::general::waistPitchNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[0].x())));
	k2app::shared::general::waistYawNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[0].y())));
	k2app::shared::general::waistRollNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[0].z())));

	k2app::shared::general::leftFootPitchNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[1].x())));
	k2app::shared::general::leftFootYawNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[1].y())));
	k2app::shared::general::leftFootRollNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[1].z())));

	k2app::shared::general::rightFootPitchNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[2].x())));
	k2app::shared::general::rightFootYawNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[2].y())));
	k2app::shared::general::rightFootRollNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[2].z())));

	// Notice that we're finished
	pending_offsets_update = false;

	// Open the pane now
	OffsetsView().IsPaneOpen(true);
}


void winrt::KinectToVR::implementation::GeneralPage::SkeletonToggleButton_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	show_skeleton_current = true;
}


void winrt::KinectToVR::implementation::GeneralPage::SkeletonToggleButton_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	show_skeleton_current = false;
}


void winrt::KinectToVR::implementation::GeneralPage::SaveOffsetsButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	OffsetsView().IsPaneOpen(false);

	// Save backend offsets' values to settings/file
	// (they are already captured by OffsetsFrontendValueChanged(...))
	k2app::K2Settings.saveSettings();
}

void winrt::KinectToVR::implementation::GeneralPage::DiscardOffsetsButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Discard backend offsets' values by re-reading them from settings
	k2app::K2Settings.readSettings();

	// Notice that we're gonna change some values
	pending_offsets_update = true;

	k2app::shared::general::waistXNumberBox.get()->Value(int(k2app::K2Settings.positionJointsOffsets[0].x() * 100.0));
	k2app::shared::general::waistYNumberBox.get()->Value(int(k2app::K2Settings.positionJointsOffsets[0].y() * 100.0));
	k2app::shared::general::waistZNumberBox.get()->Value(int(k2app::K2Settings.positionJointsOffsets[0].z() * 100.0));

	k2app::shared::general::leftFootXNumberBox.get()->
	                                           Value(int(k2app::K2Settings.positionJointsOffsets[1].x() * 100.0));
	k2app::shared::general::leftFootYNumberBox.get()->
	                                           Value(int(k2app::K2Settings.positionJointsOffsets[1].y() * 100.0));
	k2app::shared::general::leftFootZNumberBox.get()->
	                                           Value(int(k2app::K2Settings.positionJointsOffsets[1].z() * 100.0));

	k2app::shared::general::rightFootXNumberBox.get()->Value(
		int(k2app::K2Settings.positionJointsOffsets[2].x() * 100.0));
	k2app::shared::general::rightFootYNumberBox.get()->Value(
		int(k2app::K2Settings.positionJointsOffsets[2].y() * 100.0));
	k2app::shared::general::rightFootZNumberBox.get()->Value(
		int(k2app::K2Settings.positionJointsOffsets[2].z() * 100.0));

	k2app::shared::general::waistPitchNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[0].x())));
	k2app::shared::general::waistYawNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[0].y())));
	k2app::shared::general::waistRollNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[0].z())));

	k2app::shared::general::leftFootPitchNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[1].x())));
	k2app::shared::general::leftFootYawNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[1].y())));
	k2app::shared::general::leftFootRollNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[1].z())));

	k2app::shared::general::rightFootPitchNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[2].x())));
	k2app::shared::general::rightFootYawNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[2].y())));
	k2app::shared::general::rightFootRollNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[2].z())));

	// Notice that we're finished
	pending_offsets_update = false;

	// Close the pane now
	OffsetsView().IsPaneOpen(false);
}


void winrt::KinectToVR::implementation::GeneralPage::OffsetsFrontendValueChanged(
	winrt::Windows::Foundation::IInspectable const& sender,
	winrt::Microsoft::UI::Xaml::Controls::NumberBoxValueChangedEventArgs const& e)
{
	if (!general_tab_setup_finished || pending_offsets_update)return; // Don't react to dummy changes

	// Notice that we're gonna change some values
	pending_offsets_update = true;

	// Attempt automatic nan fix after clicking x
	if (isnan(k2app::shared::general::waistXNumberBox.get()->Value()))
		k2app::shared::general::waistXNumberBox.get()->Value(0.0);
	if (isnan(k2app::shared::general::waistYNumberBox.get()->Value()))
		k2app::shared::general::waistYNumberBox.get()->Value(0.0);
	if (isnan(k2app::shared::general::waistZNumberBox.get()->Value()))
		k2app::shared::general::waistZNumberBox.get()->Value(0.0);

	if (isnan(k2app::shared::general::leftFootXNumberBox.get()->Value()))
		k2app::shared::general::leftFootXNumberBox.get()->Value(0.0);
	if (isnan(k2app::shared::general::leftFootYNumberBox.get()->Value()))
		k2app::shared::general::leftFootYNumberBox.get()->Value(0.0);
	if (isnan(k2app::shared::general::leftFootZNumberBox.get()->Value()))
		k2app::shared::general::leftFootZNumberBox.get()->Value(0.0);

	if (isnan(k2app::shared::general::rightFootXNumberBox.get()->Value()))
		k2app::shared::general::rightFootXNumberBox.get()->Value(0.0);
	if (isnan(k2app::shared::general::rightFootYNumberBox.get()->Value()))
		k2app::shared::general::rightFootYNumberBox.get()->Value(0.0);
	if (isnan(k2app::shared::general::rightFootZNumberBox.get()->Value()))
		k2app::shared::general::rightFootZNumberBox.get()->Value(0.0);

	if (isnan(k2app::shared::general::waistPitchNumberBox.get()->Value()))
		k2app::shared::general::waistPitchNumberBox.get()->Value(0.0);
	if (isnan(k2app::shared::general::waistYawNumberBox.get()->Value()))
		k2app::shared::general::waistYawNumberBox.get()->Value(0.0);
	if (isnan(k2app::shared::general::waistRollNumberBox.get()->Value()))
		k2app::shared::general::waistRollNumberBox.get()->Value(0.0);

	if (isnan(k2app::shared::general::leftFootPitchNumberBox.get()->Value()))
		k2app::shared::general::leftFootPitchNumberBox.get()->Value(0.0);
	if (isnan(k2app::shared::general::leftFootYawNumberBox.get()->Value()))
		k2app::shared::general::leftFootYawNumberBox.get()->Value(0.0);
	if (isnan(k2app::shared::general::leftFootRollNumberBox.get()->Value()))
		k2app::shared::general::leftFootRollNumberBox.get()->Value(0.0);

	if (isnan(k2app::shared::general::rightFootPitchNumberBox.get()->Value()))
		k2app::shared::general::rightFootPitchNumberBox.get()->Value(0.0);
	if (isnan(k2app::shared::general::rightFootYawNumberBox.get()->Value()))
		k2app::shared::general::rightFootYawNumberBox.get()->Value(0.0);
	if (isnan(k2app::shared::general::rightFootRollNumberBox.get()->Value()))
		k2app::shared::general::rightFootRollNumberBox.get()->Value(0.0);

	// Notice that we're finished
	pending_offsets_update = false;

	// Update backend offsets with new values BUT NOT SAVE
	k2app::K2Settings.positionJointsOffsets[0].x() = double(k2app::shared::general::waistXNumberBox.get()->Value()) /
		100.0;
	k2app::K2Settings.positionJointsOffsets[0].y() = double(k2app::shared::general::waistYNumberBox.get()->Value()) /
		100.0;
	k2app::K2Settings.positionJointsOffsets[0].z() = double(k2app::shared::general::waistZNumberBox.get()->Value()) /
		100.0;

	k2app::K2Settings.positionJointsOffsets[1].x() = double(k2app::shared::general::leftFootXNumberBox.get()->Value()) /
		100.0;
	k2app::K2Settings.positionJointsOffsets[1].y() = double(k2app::shared::general::leftFootYNumberBox.get()->Value()) /
		100.0;
	k2app::K2Settings.positionJointsOffsets[1].z() = double(k2app::shared::general::leftFootZNumberBox.get()->Value()) /
		100.0;

	k2app::K2Settings.positionJointsOffsets[2].x() = double(k2app::shared::general::rightFootXNumberBox.get()->Value())
		/ 100.0;
	k2app::K2Settings.positionJointsOffsets[2].y() = double(k2app::shared::general::rightFootYNumberBox.get()->Value())
		/ 100.0;
	k2app::K2Settings.positionJointsOffsets[2].z() = double(k2app::shared::general::rightFootZNumberBox.get()->Value())
		/ 100.0;

	k2app::K2Settings.rotationJointsOffsets[0].x() = degreesToRadians(
		k2app::shared::general::waistPitchNumberBox.get()->Value());
	k2app::K2Settings.rotationJointsOffsets[0].y() = degreesToRadians(
		k2app::shared::general::waistYawNumberBox.get()->Value());
	k2app::K2Settings.rotationJointsOffsets[0].z() = degreesToRadians(
		k2app::shared::general::waistRollNumberBox.get()->Value());

	k2app::K2Settings.rotationJointsOffsets[1].x() = degreesToRadians(
		k2app::shared::general::leftFootPitchNumberBox.get()->Value());
	k2app::K2Settings.rotationJointsOffsets[1].y() = degreesToRadians(
		k2app::shared::general::leftFootYawNumberBox.get()->Value());
	k2app::K2Settings.rotationJointsOffsets[1].z() = degreesToRadians(
		k2app::shared::general::leftFootRollNumberBox.get()->Value());

	k2app::K2Settings.rotationJointsOffsets[2].x() = degreesToRadians(
		k2app::shared::general::rightFootPitchNumberBox.get()->Value());
	k2app::K2Settings.rotationJointsOffsets[2].y() = degreesToRadians(
		k2app::shared::general::rightFootYawNumberBox.get()->Value());
	k2app::K2Settings.rotationJointsOffsets[2].z() = degreesToRadians(
		k2app::shared::general::rightFootRollNumberBox.get()->Value());
}


void winrt::KinectToVR::implementation::GeneralPage::OffsetsView_PaneClosing(
	winrt::Microsoft::UI::Xaml::Controls::SplitView const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SplitViewPaneClosingEventArgs const& args)
{
	args.Cancel(true);
}


void winrt::KinectToVR::implementation::GeneralPage::CalibrationView_PaneClosing(
	winrt::Microsoft::UI::Xaml::Controls::SplitView const& sender,
	winrt::Microsoft::UI::Xaml::Controls::SplitViewPaneClosingEventArgs const& args)
{
	args.Cancel(true);
}


void winrt::KinectToVR::implementation::GeneralPage::AutoCalibrationButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	AutoCalibrationPane().Visibility(Visibility::Visible);
	ManualCalibrationPane().Visibility(Visibility::Collapsed);
	CalibrationSelectionPane().Visibility(Visibility::Collapsed);

	StartAutoCalibrationButton().IsEnabled(true);
	CalibrationInstructionsLabel().Text(L"Start the calibration");
	CalibrationCountdownLabel().Text(L"~");

	DiscardAutoCalibrationButton().Content(box_value(L"Cancel"));
}


void winrt::KinectToVR::implementation::GeneralPage::ManualCalibrationButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	AutoCalibrationPane().Visibility(Visibility::Collapsed);
	ManualCalibrationPane().Visibility(Visibility::Visible);
	CalibrationSelectionPane().Visibility(Visibility::Collapsed);

	StartManualCalibrationButton().IsEnabled(true);
	DiscardManualCalibrationButton().Content(box_value(L"Cancel"));
}

Windows::Foundation::IAsyncAction winrt::KinectToVR::implementation::GeneralPage::StartAutoCalibrationButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Set the [calibration pending] bool
	CalibrationPending = true;

	// Disable the start button and change [cancel]'s text
	StartAutoCalibrationButton().IsEnabled(false);
	DiscardAutoCalibrationButton().Content(box_value(L"Abort"));

	// Loop over total 3 points (may change)
	for (int point = 0; point < 3; point++)
	{
		// Wait for the user to move
		CalibrationInstructionsLabel().Text(L"Move somewhere else");
		for (int i = 3; i >= 0; i--)
		{
			CalibrationCountdownLabel().Text(std::to_wstring(i));
			if (!CalibrationPending)break; // Check for exiting

			{
				// Sleep on UI
				winrt::apartment_context ui_thread;
				co_await winrt::resume_background();
				Sleep(1000);
				co_await ui_thread;
			}
			if (!CalibrationPending)break; // Check for exiting
		}

		CalibrationInstructionsLabel().Text(L"Stand still!");
		for (int i = 3; i >= 0; i--)
		{
			CalibrationCountdownLabel().Text(std::to_wstring(i));
			if (!CalibrationPending)break; // Check for exiting

			// Capture user's position at [1]
			if (i == 1)
			{
				// Capture
			}

			// Wait and eventually break
			{
				// Sleep on UI
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
	{
		// Sleep on UI
		winrt::apartment_context ui_thread;
		co_await winrt::resume_background();
		Sleep(2200); // Just right
		co_await ui_thread;
	}

	// Exit the pane
	CalibrationView().IsPaneOpen(false);

	show_skeleton_current = show_skeleton_previous; // Change to whatever
	SkeletonToggleButton().IsChecked(show_skeleton_previous); // Change to whatever
}


void winrt::KinectToVR::implementation::GeneralPage::DiscardAutoCalibrationButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Just exit
	if (!CalibrationPending)
	{
		CalibrationView().IsPaneOpen(false);

		show_skeleton_current = show_skeleton_previous; // Change to whatever
		SkeletonToggleButton().IsChecked(show_skeleton_previous); // Change to whatever
	}
	// Begin abort
	else CalibrationPending = false;
}


Windows::Foundation::IAsyncAction winrt::KinectToVR::implementation::GeneralPage::StartManualCalibrationButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Set the [calibration pending] bool
	CalibrationPending = true;

	// Disable the start button and change [cancel]'s text
	StartManualCalibrationButton().IsEnabled(false);
	DiscardManualCalibrationButton().Content(box_value(L"Abort"));

	// Loop over until finished
	while (true /*!confirm*/)
	{
		// Wait for a mode switch
		while (true /*!modeswap && !confirm*/)
		{
			// TMP
			{
				// Sleep on UI
				winrt::apartment_context ui_thread;
				co_await winrt::resume_background();
				Sleep(1000);
				co_await ui_thread;
			}

			// Exit if aborted
			if (!CalibrationPending)break;
		}

		// Wait for a mode switch
		while (true /*!modeswap && !confirm*/)
		{
			// TMP
			{
				// Sleep on UI
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

	{
		// Sleep on UI
		winrt::apartment_context ui_thread;
		co_await winrt::resume_background();
		Sleep(1000); // Just right
		co_await ui_thread;
	}
	// Exit the pane
	CalibrationView().IsPaneOpen(false);

	show_skeleton_current = show_skeleton_previous; // Change to whatever
	SkeletonToggleButton().IsChecked(show_skeleton_previous); // Change to whatever
}


void winrt::KinectToVR::implementation::GeneralPage::DiscardManualCalibrationButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Just exit
	if (!CalibrationPending)
	{
		CalibrationView().IsPaneOpen(false);

		show_skeleton_current = show_skeleton_previous; // Change to whatever
		SkeletonToggleButton().IsChecked(show_skeleton_previous); // Change to whatever
	}
	// Begin abort
	else CalibrationPending = false;
}


void winrt::KinectToVR::implementation::GeneralPage::ToggleTrackersButton_Checked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't check if setup's finished since we're gonna emulate a click rather than change the state only
	ToggleTrackersButton().Content(box_value(L"Disconnect Trackers"));

	// Optionally spawn trackers
	if (!k2app::interfacing::K2AppTrackersSpawned)
	{
		if (!k2app::interfacing::SpawnDefaultEnabledTrackers()) // Mark as spawned
		{
			k2app::interfacing::serverDriverFailure = true; // WAAAAAAA
			k2app::interfacing::K2ServerDriverSetup(); // Refresh
			k2app::interfacing::ShowToast("We couldn't spawn trackers automatically!",
			                              "A server failure occurred and body trackers couldn't be spawned");
		}

		// Update things
		k2app::interfacing::UpdateServerStatusUI();
	}

	// Mark trackers as active
	k2app::interfacing::K2AppTrackersInitialized = true;
}


void winrt::KinectToVR::implementation::GeneralPage::ToggleTrackersButton_Unchecked(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Don't check if setup's finished since we're gonna emulate a click rather than change the state only
	ToggleTrackersButton().Content(box_value(L"Reconnect Trackers"));

	// Mark trackers as active
	k2app::interfacing::K2AppTrackersInitialized = false;
}


void winrt::KinectToVR::implementation::GeneralPage::OpenDiscordButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
}


void winrt::KinectToVR::implementation::GeneralPage::OpenDocsButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
}


void winrt::KinectToVR::implementation::GeneralPage::ServerOpenDocsButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
}


void winrt::KinectToVR::implementation::GeneralPage::GeneralPage_Loaded(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// Try auto-spawning trackers if stated so
	if (!general_tab_setup_finished && // If first-time
		k2app::interfacing::isServerDriverPresent && // If the driver's ok
		k2app::K2Settings.autoSpawnEnabledJoints) // If autospawn
	{
		if (k2app::interfacing::SpawnDefaultEnabledTrackers()) // Mark as spawned
			k2app::shared::general::toggleTrackersButton->IsChecked(true);

		// Cry about it
		else
		{
			k2app::interfacing::serverDriverFailure = true; // WAAAAAAA
			k2app::interfacing::K2ServerDriverSetup(); // Refresh
			k2app::interfacing::ShowToast("We couldn't spawn trackers automatically!",
			                              "A server failure occurred and body trackers couldn't be spawned");
		}
	}

	// Update things
	k2app::interfacing::UpdateServerStatusUI();
	TrackingDevices::updateTrackingDeviceUI(k2app::K2Settings.trackingDeviceID);

	// Notice that we're gonna change some values
	pending_offsets_update = true;

	// Load values into number boxes
	k2app::shared::general::waistXNumberBox.get()->Value(int(k2app::K2Settings.positionJointsOffsets[0].x() * 100.0));
	k2app::shared::general::waistYNumberBox.get()->Value(int(k2app::K2Settings.positionJointsOffsets[0].y() * 100.0));
	k2app::shared::general::waistZNumberBox.get()->Value(int(k2app::K2Settings.positionJointsOffsets[0].z() * 100.0));

	k2app::shared::general::leftFootXNumberBox.get()->
	                                           Value(int(k2app::K2Settings.positionJointsOffsets[1].x() * 100.0));
	k2app::shared::general::leftFootYNumberBox.get()->
	                                           Value(int(k2app::K2Settings.positionJointsOffsets[1].y() * 100.0));
	k2app::shared::general::leftFootZNumberBox.get()->
	                                           Value(int(k2app::K2Settings.positionJointsOffsets[1].z() * 100.0));

	k2app::shared::general::rightFootXNumberBox.get()->Value(
		int(k2app::K2Settings.positionJointsOffsets[2].x() * 100.0));
	k2app::shared::general::rightFootYNumberBox.get()->Value(
		int(k2app::K2Settings.positionJointsOffsets[2].y() * 100.0));
	k2app::shared::general::rightFootZNumberBox.get()->Value(
		int(k2app::K2Settings.positionJointsOffsets[2].z() * 100.0));

	k2app::shared::general::waistPitchNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[0].x())));
	k2app::shared::general::waistYawNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[0].y())));
	k2app::shared::general::waistRollNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[0].z())));

	k2app::shared::general::leftFootPitchNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[1].x())));
	k2app::shared::general::leftFootYawNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[1].y())));
	k2app::shared::general::leftFootRollNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[1].z())));

	k2app::shared::general::rightFootPitchNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[2].x())));
	k2app::shared::general::rightFootYawNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[2].y())));
	k2app::shared::general::rightFootRollNumberBox.get()->Value(
		(int)round(radiansToDegrees(k2app::K2Settings.rotationJointsOffsets[2].z())));

	// Notice that we're finished
	pending_offsets_update = false;

	// Notify of the setup's end
	general_tab_setup_finished = true;
}


void winrt::KinectToVR::implementation::GeneralPage::sk_line(
	Shapes::Line& line,
	std::array<Eigen::Vector3f, 25> const& joints,
	std::array<ktvr::JointTrackingState, 25> const& states,
	ktvr::ITrackedJointType const& from,
	ktvr::ITrackedJointType const& to)
{
	constexpr double s_mat_width_default = 700,
	                 s_mat_height_default = 600;

	double s_mat_width = SkeletonDrawingCanvas().ActualWidth(),
	       s_mat_height = SkeletonDrawingCanvas().ActualHeight();

	// Eventually fix sizes
	if (s_mat_height < 1)s_mat_height = s_mat_height_default;
	if (s_mat_height < 1)s_mat_width = s_mat_width_default;

	// Where to scale by 1.0 in perspective
	constexpr double s_normal_distance = 3;

	// Compose perspective constants, make it 70%
	const double s_from_multiply = .7 * (s_normal_distance / (joints[from].z() > 0. ? joints[from].z() : 3.)),
	             s_to_multiply = .7 * (s_normal_distance / (joints[to].z() > 0. ? joints[to].z() : 3.));

	auto a = Media::AcrylicBrush();
	auto ui = Windows::UI::ViewManagement::UISettings();

	line.StrokeThickness(5);
	a.TintColor(ui.GetColorValue(Windows::UI::ViewManagement::UIColorType::Accent));

	if (states[from] != ktvr::State_Tracked ||
		states[to] != ktvr::State_Tracked)
		line.Stroke(a);
	else
		line.Stroke(Media::SolidColorBrush(Windows::UI::Colors::White()));

	// Select the smaller scale to preserve somewhat uniform skeleton scaling
	const double s_scale_w = s_mat_width / s_mat_width_default,
	             s_scale_h = s_mat_height / s_mat_height_default;

	line.X1(joints[from].x() * 300. * std::min(s_scale_w, s_scale_h) * s_from_multiply + s_mat_width / 2.);
	line.Y1(joints[from].y() * -300. * std::min(s_scale_w, s_scale_h) * s_from_multiply + s_mat_height / 3.);

	line.X2(joints[to].x() * 300. * std::min(s_scale_w, s_scale_h) * s_to_multiply + s_mat_width / 2.);
	line.Y2(joints[to].y() * -300. * std::min(s_scale_w, s_scale_h) * s_to_multiply + s_mat_height / 3.);
}


void winrt::KinectToVR::implementation::GeneralPage::SkeletonDrawingCanvas_Loaded(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	static auto boneLines = std::array<Shapes::Line, 24>();

	SkeletonDrawingCanvas().Children().Clear();
	for (auto& l : boneLines)
	{
		l = Shapes::Line();
		SkeletonDrawingCanvas().Children().Append(l);
	}

	auto timer = DispatcherTimer();
	timer.Interval(std::chrono::milliseconds(33));

	timer.Tick([&, this](IInspectable const& sender, IInspectable const& e)
	{
		if (!show_skeleton_current)
		{
			// Hide the UI, only show that viewing is disabled
			SkeletonDrawingCanvas().Visibility(Visibility::Collapsed);
			NotTrackedNotice().Visibility(Visibility::Collapsed);

			SkeletonHiddenNotice().Visibility(Visibility::Visible);
			return; // Nothing more to do anyway
		}

		SkeletonHiddenNotice().Visibility(Visibility::Collapsed); // Else hide
		auto const& trackingDevice = TrackingDevices::getCurrentDevice();

		switch (trackingDevice.index())
		{
		case 0:
			{
				auto const& device = std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice);

				const auto joints = device->getJointPositions();
				const auto states = device->getTrackingStates();

				if (device->isSkeletonTracked())
				{
					// Don't waste cpu & ram, ok?

					// Show the UI
					SkeletonDrawingCanvas().Visibility(Visibility::Visible);
					NotTrackedNotice().Visibility(Visibility::Collapsed);

					if (device->getDeviceCharacteristics() == ktvr::K2_Character_Full)
					{
						// Draw the skeleton with from-to lines
						// Head
						sk_line(boneLines[0], joints, states, ktvr::Joint_Head, ktvr::Joint_Neck);
						sk_line(boneLines[1], joints, states, ktvr::Joint_Neck, ktvr::Joint_SpineShoulder);

						// Upper left limb
						sk_line(boneLines[2], joints, states, ktvr::Joint_SpineShoulder, ktvr::Joint_ShoulderLeft);
						sk_line(boneLines[3], joints, states, ktvr::Joint_ShoulderLeft, ktvr::Joint_ElbowLeft);
						sk_line(boneLines[4], joints, states, ktvr::Joint_ElbowLeft, ktvr::Joint_WristLeft);
						sk_line(boneLines[5], joints, states, ktvr::Joint_WristLeft, ktvr::Joint_HandLeft);
						sk_line(boneLines[6], joints, states, ktvr::Joint_HandLeft, ktvr::Joint_HandTipLeft);
						sk_line(boneLines[7], joints, states, ktvr::Joint_HandLeft, ktvr::Joint_ThumbLeft);

						// Upper right limb
						sk_line(boneLines[8], joints, states, ktvr::Joint_SpineShoulder, ktvr::Joint_ShoulderRight);
						sk_line(boneLines[9], joints, states, ktvr::Joint_ShoulderRight, ktvr::Joint_ElbowRight);
						sk_line(boneLines[10], joints, states, ktvr::Joint_ElbowRight, ktvr::Joint_WristRight);
						sk_line(boneLines[11], joints, states, ktvr::Joint_WristRight, ktvr::Joint_HandRight);
						sk_line(boneLines[12], joints, states, ktvr::Joint_HandRight, ktvr::Joint_HandTipRight);
						sk_line(boneLines[13], joints, states, ktvr::Joint_HandRight, ktvr::Joint_ThumbRight);

						// Spine
						sk_line(boneLines[14], joints, states, ktvr::Joint_SpineShoulder, ktvr::Joint_SpineMiddle);
						sk_line(boneLines[15], joints, states, ktvr::Joint_SpineMiddle, ktvr::Joint_SpineWaist);

						// Lower left limb
						sk_line(boneLines[16], joints, states, ktvr::Joint_SpineWaist, ktvr::Joint_HipLeft);
						sk_line(boneLines[17], joints, states, ktvr::Joint_HipLeft, ktvr::Joint_KneeLeft);
						sk_line(boneLines[18], joints, states, ktvr::Joint_KneeLeft, ktvr::Joint_AnkleLeft);
						sk_line(boneLines[19], joints, states, ktvr::Joint_AnkleLeft, ktvr::Joint_FootLeft);

						// Lower right limb
						sk_line(boneLines[20], joints, states, ktvr::Joint_SpineWaist, ktvr::Joint_HipRight);
						sk_line(boneLines[21], joints, states, ktvr::Joint_HipRight, ktvr::Joint_KneeRight);
						sk_line(boneLines[22], joints, states, ktvr::Joint_KneeRight, ktvr::Joint_AnkleRight);
						sk_line(boneLines[23], joints, states, ktvr::Joint_AnkleRight, ktvr::Joint_FootRight);
					}
					else if (device->getDeviceCharacteristics() == ktvr::K2_Character_Simple)
					{
						// Draw the skeleton with from-to lines
						// Head
						sk_line(boneLines[0], joints, states, ktvr::Joint_Head, ktvr::Joint_SpineWaist);

						// Empty lines
						boneLines[1] = Shapes::Line();
						boneLines[2] = Shapes::Line();
						boneLines[3] = Shapes::Line();
						boneLines[4] = Shapes::Line();
						boneLines[5] = Shapes::Line();
						boneLines[6] = Shapes::Line();
						boneLines[7] = Shapes::Line();
						boneLines[8] = Shapes::Line();
						boneLines[9] = Shapes::Line();
						boneLines[10] = Shapes::Line();
						boneLines[11] = Shapes::Line();
						boneLines[12] = Shapes::Line();
						boneLines[13] = Shapes::Line();
						boneLines[14] = Shapes::Line();
						boneLines[15] = Shapes::Line();

						// Lower left limb
						sk_line(boneLines[16], joints, states, ktvr::Joint_SpineWaist, ktvr::Joint_HipLeft);
						sk_line(boneLines[17], joints, states, ktvr::Joint_HipLeft, ktvr::Joint_KneeLeft);
						sk_line(boneLines[18], joints, states, ktvr::Joint_KneeLeft, ktvr::Joint_AnkleLeft);
						sk_line(boneLines[19], joints, states, ktvr::Joint_AnkleLeft, ktvr::Joint_FootLeft);

						// Lower right limb
						sk_line(boneLines[20], joints, states, ktvr::Joint_SpineWaist, ktvr::Joint_HipRight);
						sk_line(boneLines[21], joints, states, ktvr::Joint_HipRight, ktvr::Joint_KneeRight);
						sk_line(boneLines[22], joints, states, ktvr::Joint_KneeRight, ktvr::Joint_AnkleRight);
						sk_line(boneLines[23], joints, states, ktvr::Joint_AnkleRight, ktvr::Joint_FootRight);
					}
					else if (device->getDeviceCharacteristics() == ktvr::K2_Character_Basic)
					{
						// Draw the skeleton with from-to lines
						// Head
						sk_line(boneLines[0], joints, states, ktvr::Joint_Head, ktvr::Joint_SpineWaist);

						// Empty lines
						boneLines[1] = Shapes::Line();
						boneLines[2] = Shapes::Line();
						boneLines[3] = Shapes::Line();
						boneLines[4] = Shapes::Line();
						boneLines[5] = Shapes::Line();
						boneLines[6] = Shapes::Line();
						boneLines[7] = Shapes::Line();
						boneLines[8] = Shapes::Line();
						boneLines[9] = Shapes::Line();
						boneLines[10] = Shapes::Line();
						boneLines[11] = Shapes::Line();
						boneLines[12] = Shapes::Line();
						boneLines[13] = Shapes::Line();
						boneLines[14] = Shapes::Line();
						boneLines[15] = Shapes::Line();

						// Lower left limb
						sk_line(boneLines[16], joints, states, ktvr::Joint_SpineWaist, ktvr::Joint_AnkleLeft);

						// Empty lines
						boneLines[17] = Shapes::Line();
						boneLines[18] = Shapes::Line();
						boneLines[19] = Shapes::Line();

						// Lower right limb
						sk_line(boneLines[20], joints, states, ktvr::Joint_SpineWaist, ktvr::Joint_AnkleRight);

						// Empty lines
						boneLines[21] = Shapes::Line();
						boneLines[22] = Shapes::Line();
						boneLines[23] = Shapes::Line();
					}
				}
				else
				{
					SkeletonDrawingCanvas().Visibility(Visibility::Collapsed);
					NotTrackedNotice().Visibility(Visibility::Visible);
				}
			}
			break;
		//case 1: // TODO: Other device types
		//	{
		//		auto const& device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);

		//		const auto joints = device->getTrackedJoints();

		//		if (device->isSkeletonTracked())
		//		{
		//			// Don't waste cpu & ram, ok?

		//			// Show the UI
		//			SkeletonDrawingCanvas().Visibility(Visibility::Visible);
		//			NotTrackedNotice().Visibility(Visibility::Collapsed);

		//			{
		//				// Draw the skeleton with from-to lines
		//				// Head
		//				sk_line(boneLines[0], joints, states, ktvr::Joint_Head, ktvr::Joint_SpineWaist);

		//				// Empty lines
		//				boneLines[1] = Shapes::Line();
		//				boneLines[2] = Shapes::Line();
		//				boneLines[3] = Shapes::Line();
		//				boneLines[4] = Shapes::Line();
		//				boneLines[5] = Shapes::Line();
		//				boneLines[6] = Shapes::Line();
		//				boneLines[7] = Shapes::Line();
		//				boneLines[8] = Shapes::Line();
		//				boneLines[9] = Shapes::Line();
		//				boneLines[10] = Shapes::Line();
		//				boneLines[11] = Shapes::Line();
		//				boneLines[12] = Shapes::Line();
		//				boneLines[13] = Shapes::Line();
		//				boneLines[14] = Shapes::Line();
		//				boneLines[15] = Shapes::Line();

		//				// Lower left limb
		//				sk_line(boneLines[16], joints, states, ktvr::Joint_SpineWaist, ktvr::Joint_AnkleLeft);

		//				// Empty lines
		//				boneLines[17] = Shapes::Line();
		//				boneLines[18] = Shapes::Line();
		//				boneLines[19] = Shapes::Line();

		//				// Lower right limb
		//				sk_line(boneLines[20], joints, states, ktvr::Joint_SpineWaist, ktvr::Joint_AnkleRight);

		//				// Empty lines
		//				boneLines[21] = Shapes::Line();
		//				boneLines[22] = Shapes::Line();
		//				boneLines[23] = Shapes::Line();
		//			}
		//		}
		//		else
		//		{
		//			SkeletonDrawingCanvas().Visibility(Visibility::Collapsed);
		//			NotTrackedNotice().Visibility(Visibility::Visible);
		//		}
		//	}
		//	break;
		}
	});

	timer.Start();
}


void winrt::KinectToVR::implementation::GeneralPage::CalibrationButton_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	// If no overrides
	if (k2app::K2Settings.overrideDeviceID < 0)
	{
		AutoCalibrationPane().Visibility(Visibility::Collapsed);
		ManualCalibrationPane().Visibility(Visibility::Collapsed);
		CalibrationSelectionPane().Visibility(Visibility::Visible);

		CalibrationView().IsPaneOpen(true);

		show_skeleton_previous = show_skeleton_current; // Back up
		show_skeleton_current = true; // Change to show
		SkeletonToggleButton().IsChecked(true); // Change to show
	}
	else
	{
		ChooseDeviceFlyout().ShowAt(CalibrationButton());

		// Assume no head position providers
		AutoCalibrationButton().IsEnabled(false);
	}
}


void winrt::KinectToVR::implementation::GeneralPage::BaseCalibration_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	ChooseDeviceFlyout().Hide();

	AutoCalibrationPane().Visibility(Visibility::Collapsed);
	ManualCalibrationPane().Visibility(Visibility::Collapsed);
	CalibrationSelectionPane().Visibility(Visibility::Visible);

	CalibrationView().IsPaneOpen(true);

	show_skeleton_previous = show_skeleton_current; // Back up
	show_skeleton_current = true; // Change to show
	SkeletonToggleButton().IsChecked(true); // Change to show

	// Eventually enable the auto calibration
	auto const& trackingDevice = TrackingDevices::TrackingDevicesVector.at(k2app::K2Settings.trackingDeviceID);
	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		if (std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->getDeviceCharacteristics() ==
			ktvr::K2_Character_Full)
			AutoCalibrationButton().IsEnabled(true);
	}
}


void winrt::KinectToVR::implementation::GeneralPage::OverrideCalibration_Click(
	winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
	ChooseDeviceFlyout().Hide();

	AutoCalibrationPane().Visibility(Visibility::Collapsed);
	ManualCalibrationPane().Visibility(Visibility::Collapsed);
	CalibrationSelectionPane().Visibility(Visibility::Visible);

	CalibrationView().IsPaneOpen(true);

	show_skeleton_previous = show_skeleton_current; // Back up
	show_skeleton_current = true; // Change to show
	SkeletonToggleButton().IsChecked(true); // Change to show

	// Eventually enable the auto calibration
	auto const& trackingDevice = TrackingDevices::TrackingDevicesVector.at(k2app::K2Settings.overrideDeviceID);
	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		if (std::get<ktvr::K2TrackingDeviceBase_KinectBasis*>(trackingDevice)->getDeviceCharacteristics() ==
			ktvr::K2_Character_Full)
			AutoCalibrationButton().IsEnabled(true);
	}
}
