#include "pch.h"
#include "GeneralPage.xaml.h"
#if __has_include("GeneralPage.g.cpp")
#include "GeneralPage.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace k2app::shared::general;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.
bool show_skeleton_previous = true,
     general_loadedOnce = false;

enum class general_calibrating_device
{
	K2_BaseDevice,
	K2_OverrideDevice
} general_current_calibrating_device;

void skeleton_visibility_set_ui(const bool& v)
{
	if (!general_tab_setup_finished)return; // Don't even care if we're not set up yet
	skeletonToggleButton.get()->IsChecked(v);
	skeletonToggleButton.get()->Content(box_value(v
		                                              ? box_value(
			                                              k2app::interfacing::LocalizedResourceWString(
				                                              L"GeneralPage",
				                                              L"Buttons/Skeleton/Hide"))
		                                              : box_value(
			                                              k2app::interfacing::LocalizedResourceWString(
				                                              L"GeneralPage",
				                                              L"Buttons/Skeleton/Show"))));

	forceRenderCheckBox.get()->IsEnabled(v);
	forceRenderText.get()->Opacity(v ? 1.0 : 0.5);
}

void skeleton_force_set_ui(const bool& v)
{
	if (!general_tab_setup_finished)return; // Don't even care if we're not set up yet
	forceRenderCheckBox.get()->IsChecked(v);
}

std::wstring points_format(std::wstring fmt,
                           const int32_t& p1, const int32_t& p2)
{
	using namespace std::string_literals;
	k2app::interfacing::stringReplaceAll(fmt, L"{1}"s, std::to_wstring(p1));
	k2app::interfacing::stringReplaceAll(fmt, L"{2}"s, std::to_wstring(p2));

	return fmt;
}

void Amethyst::implementation::GeneralPage::AllowNavigation(const bool& allow)
{
	k2app::shared::main::navigationBlockerGrid->IsHitTestVisible(!allow);
	InterfaceBlockerGrid().IsHitTestVisible(!allow);

	k2app::shared::main::generalItem->SelectsOnInvoked(allow);
	k2app::shared::main::settingsItem->SelectsOnInvoked(allow);
	k2app::shared::main::devicesItem->SelectsOnInvoked(allow);
	k2app::shared::main::infoItem->SelectsOnInvoked(allow);
	k2app::shared::main::consoleItem->SelectsOnInvoked(allow);
}

namespace winrt::Amethyst::implementation
{
	GeneralPage::GeneralPage()
	{
		InitializeComponent();

		LOG(INFO) << "Constructing page with tag: \"general\"...";

		// Cache needed UI elements
		using namespace k2app::shared::general;

		toggleTrackersButton = std::make_shared<Controls::Primitives::ToggleButton>(ToggleTrackersButton());

		skeletonToggleButton = std::make_shared<Controls::ToggleSplitButton>(SkeletonToggleButton());

		forceRenderCheckBox = std::make_shared<Controls::CheckBox>(ForceRenderCheckBox());

		calibrationButton = std::make_shared<Controls::Button>(CalibrationButton());
		reRegisterButton = std::make_shared<Controls::Button>(ReRegisterButton());
		serverOpenDiscordButton = std::make_shared<Controls::Button>(ServerOpenDiscordButton());

		versionLabel = std::make_shared<Controls::TextBlock>(VersionLabel());
		deviceNameLabel = std::make_shared<Controls::TextBlock>(SelectedDeviceNameLabel());
		deviceStatusLabel = std::make_shared<Controls::TextBlock>(TrackingDeviceStatusLabel());
		errorWhatText = std::make_shared<Controls::TextBlock>(ErrorWhatText());
		trackingDeviceErrorLabel = std::make_shared<Controls::TextBlock>(TrackingDeviceErrorLabel());
		overrideDeviceNameLabel = std::make_shared<Controls::TextBlock>(SelectedOverrideDeviceNameLabel());
		overrideDeviceStatusLabel = std::make_shared<Controls::TextBlock>(OverrideDeviceStatusLabel());
		overrideErrorWhatText = std::make_shared<Controls::TextBlock>(OverrideErrorWhatText());
		overrideDeviceErrorLabel = std::make_shared<Controls::TextBlock>(OverrideTrackingDeviceErrorLabel());
		serverStatusLabel = std::make_shared<Controls::TextBlock>(ServerStatusLabel());
		serverErrorLabel = std::make_shared<Controls::TextBlock>(ServerErrorLabel());
		serverErrorWhatText = std::make_shared<Controls::TextBlock>(ServerErrorWhatText());
		forceRenderText = std::make_shared<Controls::TextBlock>(ForceRenderText());

		offsetsControlHostGrid = std::make_shared<Controls::Grid>(OffsetsControlHostGrid());
		errorButtonsGrid = std::make_shared<Controls::Grid>(ErrorButtonsGrid());
		errorWhatGrid = std::make_shared<Controls::Grid>(ErrorWhatGrid());
		overrideErrorButtonsGrid = std::make_shared<Controls::Grid>(OverrideErrorButtonsGrid());
		overrideErrorWhatGrid = std::make_shared<Controls::Grid>(OverrideErrorWhatGrid());
		serverErrorWhatGrid = std::make_shared<Controls::Grid>(ServerErrorWhatGrid());
		serverErrorButtonsGrid = std::make_shared<Controls::Grid>(ServerErrorButtonsGrid());

		toggleFreezeButton = std::make_shared<Controls::Primitives::ToggleButton>(ToggleTrackingButton());

		offsetsButton = std::make_shared<Controls::MenuFlyoutItem>(OffsetsButton());
		freezeOnlyLowerToggle = std::make_shared<Controls::ToggleMenuFlyoutItem>(FreezeOnlyLowerToggle());

		k2app::shared::teaching_tips::general::toggleTrackersTeachingTip =
			std::make_shared<Controls::TeachingTip>(ToggleTrackersTeachingTip());
		k2app::shared::teaching_tips::general::statusTeachingTip =
			std::make_shared<Controls::TeachingTip>(StatusTeachingTip());

		// Create and push the offsets controller
		offsetsController = std::move(std::make_shared<Controls::OffsetsController>());

		offsetsControlHostGrid.get()->Children().Append(*offsetsController->Container());
		offsetsControlHostGrid.get()->SetRow(*offsetsController->Container(), 0);

		offsetsController->ReAppendTrackerPivots();

		// Set the media source (absolute)
		CalibrationPreviewMediaElement().Source(
			Windows::Media::Core::MediaSource::CreateFromUri(
				Windows::Foundation::Uri(
					k2app::interfacing::GetProgramLocation().parent_path().wstring()
					+ L"\\Assets\\CalibrationDirections.mp4")));

		LOG(INFO) << "Registering a detached binary semaphore reload handler for GeneralPage...";
		std::thread([&, this]
		{
			while (true)
			{
				// Wait for a reload signal (blocking)
				k2app::shared::semaphores::semaphore_ReloadPage_GeneralPage.acquire();

				// Reload & restart the waiting loop
				if (general_loadedOnce)
					k2app::shared::main::thisDispatcherQueue->TryEnqueue([&, this]
					{
						GeneralPage_Loaded_Handler();
					});

				Sleep(100); // Sleep a bit
			}
		}).detach();
	}
}

void Amethyst::implementation::GeneralPage::OffsetsButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Push saved offsets' by reading them from settings
	k2app::K2Settings.readSettings();

	// Notice that we're gonna change some values
	pending_offsets_update = true;

	offsetsController->ReAppendTrackerPivots();

	// Notice that we're finished
	pending_offsets_update = false;

	// Open the pane now
	OffsetsView().DisplayMode(Controls::SplitViewDisplayMode::Inline);
	OffsetsView().IsPaneOpen(true);

	AllowNavigation(false);

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Show);

	k2app::interfacing::currentAppState = L"offsets";
}


void Amethyst::implementation::GeneralPage::SkeletonToggleButton_Click(
	const Windows::Foundation::IInspectable& sender,
	const Controls::SplitButtonClickEventArgs& e)
{
	if (!general_tab_setup_finished)return; // Don't even care if we're not set up yet
	k2app::K2Settings.skeletonPreviewEnabled = skeletonToggleButton.get()->IsChecked();

	skeletonToggleButton.get()->Content(
		k2app::K2Settings.skeletonPreviewEnabled
			? box_value(k2app::interfacing::LocalizedResourceWString(
				L"GeneralPage", L"Buttons/Skeleton/Hide"))
			: box_value(k2app::interfacing::LocalizedResourceWString(
				L"GeneralPage", L"Buttons/Skeleton/Show")));

	// Play a sound
	playAppSound(k2app::K2Settings.skeletonPreviewEnabled
		             ? k2app::interfacing::sounds::AppSounds::ToggleOn
		             : k2app::interfacing::sounds::AppSounds::ToggleOff);

	forceRenderCheckBox.get()->IsEnabled(
		skeletonToggleButton.get()->IsChecked());
	forceRenderText.get()->Opacity(
		skeletonToggleButton.get()->IsChecked() ? 1.0 : 0.5);

	k2app::K2Settings.saveSettings();
}

void Amethyst::implementation::GeneralPage::ForceRenderCheckBox_Checked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (!general_tab_setup_finished)return; // Don't even care if we're not set up yet
	k2app::K2Settings.forceSkeletonPreview = true;
	skeleton_force_set_ui(k2app::K2Settings.forceSkeletonPreview);
	k2app::K2Settings.saveSettings();

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::ToggleOn);
}

void Amethyst::implementation::GeneralPage::ForceRenderCheckBox_Unchecked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (!general_tab_setup_finished)return; // Don't even care if we're not set up yet
	k2app::K2Settings.forceSkeletonPreview = false;
	skeleton_force_set_ui(k2app::K2Settings.forceSkeletonPreview);
	k2app::K2Settings.saveSettings();

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::ToggleOff);
}

void Amethyst::implementation::GeneralPage::SaveOffsetsButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	OffsetsView().DisplayMode(Controls::SplitViewDisplayMode::Overlay);
	OffsetsView().IsPaneOpen(false);

	AllowNavigation(true);

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Hide);

	k2app::interfacing::currentAppState = L"general";

	// Save backend offsets' values to settings/file
	// (they are already captured by OffsetsFrontendValueChanged(...))
	k2app::K2Settings.saveSettings();
}

void Amethyst::implementation::GeneralPage::DiscardOffsetsButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Discard backend offsets' values by re-reading them from settings
	k2app::K2Settings.readSettings();

	// Notice that we're gonna change some values
	pending_offsets_update = true;

	offsetsController->ReReadOffsets();

	// Notice that we're finished
	pending_offsets_update = false;

	// Close the pane now
	OffsetsView().DisplayMode(Controls::SplitViewDisplayMode::Overlay);
	OffsetsView().IsPaneOpen(false);

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::GoBack);

	AllowNavigation(true);
	k2app::interfacing::currentAppState = L"general";
}


void Amethyst::implementation::GeneralPage::OffsetsView_PaneClosing(
	const Controls::SplitView& sender,
	const Controls::SplitViewPaneClosingEventArgs& args)
{
	args.Cancel(true);
}


void Amethyst::implementation::GeneralPage::CalibrationView_PaneClosing(
	const Controls::SplitView& sender,
	const Controls::SplitViewPaneClosingEventArgs& args)
{
	args.Cancel(true);
}


void Amethyst::implementation::GeneralPage::AutoCalibrationButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Setup the calibration image : reset and stop
	CalibrationPreviewMediaElement().MediaPlayer().Position(Windows::Foundation::TimeSpan::zero());
	CalibrationPreviewMediaElement().MediaPlayer().Pause();

	AutoCalibrationPane().Visibility(Visibility::Visible);
	ManualCalibrationPane().Visibility(Visibility::Collapsed);

	CalibrationRunningView().DisplayMode(Controls::SplitViewDisplayMode::Inline);
	CalibrationRunningView().IsPaneOpen(true);

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Show);

	k2app::interfacing::currentAppState = L"calibration_auto";

	CalibrationPointsNumberBox().IsEnabled(true);
	CalibrationPointsNumberBox().Value(k2app::K2Settings.calibrationPointsNumber);

	CalibrationInstructionsLabel().Text(
		k2app::interfacing::LocalizedResourceWString(
			L"GeneralPage", L"Calibration/Captions/Start"));
	CalibrationCountdownLabel().Text(L"~");
	DiscardAutoCalibrationButton().Content(box_value(
		k2app::interfacing::LocalizedResourceWString(
			L"GeneralPage", L"Buttons/Cancel")));

	NoSkeletonTextNotice().Text(k2app::interfacing::LocalizedResourceWString(
		L"GeneralPage", L"Captions/Preview/NoSkeletonTextCalibrating"));
}


Windows::Foundation::IAsyncAction Amethyst::implementation::GeneralPage::StartAutoCalibrationButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Setup the calibration image : start
	CalibrationPreviewMediaElement().MediaPlayer().Play();

	// Set the [calibration pending] bool
	CalibrationPending = true;
	AutoCalibration_StillPending = true;

	// Play a nice sound - starting
	playAppSound(k2app::interfacing::sounds::AppSounds::CalibrationStart);

	// Disable the start button and change [cancel]'s text
	StartAutoCalibrationButton().IsEnabled(false);
	CalibrationPointsNumberBox().IsEnabled(false);

	DiscardAutoCalibrationButton().Content(box_value(
		k2app::interfacing::LocalizedResourceWString(
			L"GeneralPage", L"Buttons/Abort")));

	// Ref current matrices to helper pointers
	Eigen::Matrix<double, 3, 3>* calibrationRotation = // Rotation
		general_current_calibrating_device == general_calibrating_device::K2_BaseDevice
			? &k2app::K2Settings.calibrationRotationMatrices.first
			: &k2app::K2Settings.calibrationRotationMatrices.second;
	Eigen::Matrix<double, 1, 3>* calibrationTranslation = // Translation
		general_current_calibrating_device == general_calibrating_device::K2_BaseDevice
			? &k2app::K2Settings.calibrationTranslationVectors.first
			: &k2app::K2Settings.calibrationTranslationVectors.second;
	Eigen::Vector3d* calibrationOrigin = // Origin
		general_current_calibrating_device == general_calibrating_device::K2_BaseDevice
			? &k2app::K2Settings.calibrationOrigins.first
			: &k2app::K2Settings.calibrationOrigins.second;
	double* calibrationYaw = // Yaw
		general_current_calibrating_device == general_calibrating_device::K2_BaseDevice
			? &k2app::K2Settings.calibrationYaws.first
			: &k2app::K2Settings.calibrationYaws.second;
	bool* isMatrixCalibrated = // Are we calibrated?
		general_current_calibrating_device == general_calibrating_device::K2_BaseDevice
			? &k2app::K2Settings.isMatrixCalibrated.first
			: &k2app::K2Settings.isMatrixCalibrated.second;
	bool* autoCalibration = // Which calibration method did we use
		general_current_calibrating_device == general_calibrating_device::K2_BaseDevice
			? &k2app::K2Settings.autoCalibration.first
			: &k2app::K2Settings.autoCalibration.second;

	// Mark what are we doing
	*autoCalibration = true;

	// Mark as calibrated for no preview
	*isMatrixCalibrated = false;

	// Reset the origin
	*calibrationOrigin = Eigen::Vector3d(0, 0, 0);

	// Setup helper variables
	std::vector<Eigen::Vector3d> vrHMDPositions, kinectHeadPositions;

	// Sleep on UI
	apartment_context ui_thread;
	co_await resume_background();
	Sleep(1000);
	co_await ui_thread;

	// Loop over total 3 points (by default)
	for (int point = 0; point < k2app::K2Settings.calibrationPointsNumber; point++)
	{
		// Setup helper variables - inside each point
		Eigen::Vector3d vrHMDPosition;

		// Wait for the user to move
		CalibrationInstructionsLabel().Text(
			points_format(k2app::interfacing::LocalizedResourceWString(
				              L"GeneralPage", L"Calibration/Captions/Move"),
			              point + 1, k2app::K2Settings.calibrationPointsNumber));

		for (int i = 3; i >= 0; i--)
		{
			if (!CalibrationPending)break; // Check for exiting

			// Update the countdown label
			CalibrationCountdownLabel().Text(std::to_wstring(i));

			// Exit if aborted
			if (!CalibrationPending)break;

			// Play a nice sound - tick / move
			if (i > 0) // Don't play the last one!
				playAppSound(k2app::interfacing::sounds::AppSounds::CalibrationTick);

			{
				// Sleep on UI
				apartment_context ui_thread;
				co_await resume_background();
				Sleep(1000);
				co_await ui_thread;
			}
			if (!CalibrationPending)break; // Check for exiting
		}

		CalibrationInstructionsLabel().Text(
			points_format(k2app::interfacing::LocalizedResourceWString(
				              L"GeneralPage", L"Calibration/Captions/Stand"),
			              point + 1, k2app::K2Settings.calibrationPointsNumber));

		for (int i = 3; i >= 0; i--)
		{
			CalibrationCountdownLabel().Text(std::to_wstring(i));
			if (!CalibrationPending)break; // Check for exiting

			// Play a nice sound - tick / stand
			if (i > 0) // Don't play the last one!
				playAppSound(k2app::interfacing::sounds::AppSounds::CalibrationTick);

			// Capture user's position at t_end-1
			if (i == 1)
			{
				// Capture positions
				vrHMDPosition = k2app::interfacing::plugins::plugins_getHMDPositionCalibrated().cast<double>();

				vrHMDPositions.push_back(vrHMDPosition);
				kinectHeadPositions.push_back(
					(general_current_calibrating_device == general_calibrating_device::K2_BaseDevice
						 ? k2app::interfacing::kinectHeadPosition.first
						 : k2app::interfacing::kinectHeadPosition.second
					).cast<double>());
			}
			else if (i == 0)
				CalibrationInstructionsLabel().Text(
					k2app::interfacing::LocalizedResourceWString(
						L"GeneralPage", L"Calibration/Captions/Captured"));

			// Wait and eventually break
			{
				// Sleep on UI
				apartment_context ui_thread;
				co_await resume_background();
				Sleep(1000);
				co_await ui_thread;
			}
			if (!CalibrationPending)break; // Check for exiting
		}

		// Exit if aborted
		if (!CalibrationPending)break;

		// Play a nice sound - tick / captured
		playAppSound(k2app::interfacing::sounds::AppSounds::CalibrationPointCaptured);

		// Sleep on UI
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(1000);
		co_await ui_thread;

		// Exit if aborted
		if (!CalibrationPending)break;
	}

	// Do the actual calibration after capturing points
	if (CalibrationPending)
	{
		Eigen::Matrix<double, 3, Eigen::Dynamic>
			vrPoints(3, k2app::K2Settings.calibrationPointsNumber),
			kinectPoints(3, k2app::K2Settings.calibrationPointsNumber);

		for (uint32_t i_point = 0; i_point < k2app::K2Settings.calibrationPointsNumber; i_point++)
		{
			vrPoints(0, i_point) = vrHMDPositions.at(i_point).x();
			vrPoints(1, i_point) = vrHMDPositions.at(i_point).y();
			vrPoints(2, i_point) = vrHMDPositions.at(i_point).z();

			kinectPoints(0, i_point) = kinectHeadPositions.at(i_point).x();
			kinectPoints(1, i_point) = kinectHeadPositions.at(i_point).y();
			kinectPoints(2, i_point) = kinectHeadPositions.at(i_point).z();
			if (!CalibrationPending) break;
		}

		EigenUtils::PointSet A = kinectPoints, B = vrPoints;
		const auto [return_Rotation, return_Translation] =
			EigenUtils::rigid_transform_3D(A, B); // MVP Korejan

		LOG(INFO) <<
			"Head points\n" << A <<
			"\nSteamvr points\n" << B <<
			"\nTranslation\n" << return_Translation <<
			"\nRotation\n" << return_Rotation;

		EigenUtils::PointSet B2 = (return_Rotation * A).colwise() + return_Translation;

		EigenUtils::PointSet err = B2 - B;
		err = err.cwiseProduct(err);

		std::cout <<
			"Original points\n" << B <<
			"\nMy result\n" << B2;

		*calibrationRotation = return_Rotation;
		*calibrationTranslation = return_Translation;

		LOG(INFO) << "Retrieved playspace rotation [eulers, radians]: ";
		LOG(INFO) << return_Rotation.eulerAngles(0, 1, 2);

		Eigen::Vector3d projected_Rotation =
			EigenUtils::RotationProjectedEulerAngles(return_Rotation);

		LOG(INFO) << "Retrieved/Projected playspace rotation [eulers, radians]: ";
		LOG(INFO) << projected_Rotation;

		*calibrationYaw = projected_Rotation.y(); // Note: radians
		*calibrationOrigin = Eigen::Vector3d(0, 0, 0);

		*isMatrixCalibrated = true;

		// Settings will be saved below
	}

	// Reset by re-reading the settings if aborted
	if (!CalibrationPending)
	{
		*isMatrixCalibrated = false;
		k2app::K2Settings.readSettings();

		playAppSound(k2app::interfacing::sounds::AppSounds::CalibrationAborted);
	}
	// Else save I guess
	else
	{
		k2app::K2Settings.saveSettings();
		playAppSound(k2app::interfacing::sounds::AppSounds::CalibrationComplete);
	}

	// Notify that we're finished
	CalibrationCountdownLabel().Text(L"~");
	CalibrationInstructionsLabel().Text(CalibrationPending
		                                    ? k2app::interfacing::LocalizedResourceWString(
			                                    L"GeneralPage", L"Calibration/Captions/Done")
		                                    : k2app::interfacing::LocalizedResourceWString(
			                                    L"GeneralPage", L"Calibration/Captions/Aborted"));

	{
		// Sleep on UI
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(2200); // Just right
		co_await ui_thread;
	}

	// Exit the pane
	CalibrationSelectView().DisplayMode(Controls::SplitViewDisplayMode::Overlay);
	CalibrationSelectView().IsPaneOpen(false);

	CalibrationRunningView().DisplayMode(Controls::SplitViewDisplayMode::Overlay);
	CalibrationRunningView().IsPaneOpen(false);

	AllowNavigation(true);
	k2app::interfacing::currentAppState = L"general";

	NoSkeletonTextNotice().Text(k2app::interfacing::LocalizedResourceWString(
		L"GeneralPage", L"Captions/Preview/NoSkeletonText"));

	CalibrationPending = false; // We're finished
	AutoCalibration_StillPending = false;

	k2app::K2Settings.skeletonPreviewEnabled = show_skeleton_previous; // Change to whatever
	skeleton_visibility_set_ui(show_skeleton_previous); // Change to whatever
}


void Amethyst::implementation::GeneralPage::DiscardCalibrationButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Just exit
	if (!CalibrationPending && !AutoCalibration_StillPending)
	{
		CalibrationSelectView().DisplayMode(Controls::SplitViewDisplayMode::Overlay);
		CalibrationSelectView().IsPaneOpen(false);

		CalibrationRunningView().DisplayMode(Controls::SplitViewDisplayMode::Overlay);
		CalibrationRunningView().IsPaneOpen(false);

		AllowNavigation(true);

		// Play a sound
		playAppSound(k2app::interfacing::sounds::AppSounds::Hide);

		k2app::interfacing::currentAppState = L"general";

		NoSkeletonTextNotice().Text(k2app::interfacing::LocalizedResourceWString(
			L"GeneralPage", L"Captions/Preview/NoSkeletonText"));

		k2app::K2Settings.skeletonPreviewEnabled = show_skeleton_previous; // Change to whatever
		skeleton_visibility_set_ui(show_skeleton_previous); // Change to whatever
	}
	// Begin abort
	else CalibrationPending = false;
}


Windows::Foundation::IAsyncAction Amethyst::implementation::GeneralPage::ManualCalibrationButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Swap trigger/grip if we're on index or vive
	char _controller_model[1024];
	vr::VRSystem()->GetStringTrackedDeviceProperty(
		vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(
			vr::ETrackedControllerRole::TrackedControllerRole_LeftHand),
		vr::ETrackedDeviceProperty::Prop_ModelNumber_String, _controller_model, std::size(_controller_model));

	// Set up as default (just in case)
	LabelFineTuneVive().Visibility(Visibility::Collapsed);
	LabelFineTuneNormal().Visibility(Visibility::Visible);

	// Swap (optionally)
	if (k2app::interfacing::findStringIC(_controller_model, "knuckles") ||
		k2app::interfacing::findStringIC(_controller_model, "index") ||
		k2app::interfacing::findStringIC(_controller_model, "vive"))
	{
		LabelFineTuneVive().Visibility(Visibility::Visible);
		LabelFineTuneNormal().Visibility(Visibility::Collapsed);
	}

	// Set up panels
	AutoCalibrationPane().Visibility(Visibility::Collapsed);
	ManualCalibrationPane().Visibility(Visibility::Visible);

	CalibrationRunningView().DisplayMode(Controls::SplitViewDisplayMode::Inline);
	CalibrationRunningView().IsPaneOpen(true);

	k2app::interfacing::currentAppState = L"calibration_manual";

	// Set the [calibration pending] bool
	CalibrationPending = true;

	// Play a nice sound - starting
	playAppSound(k2app::interfacing::sounds::AppSounds::CalibrationStart);

	// Ref current matrices to helper pointers
	Eigen::Matrix<double, 3, 3>* calibrationRotation = // Rotation
		general_current_calibrating_device == general_calibrating_device::K2_BaseDevice
			? &k2app::K2Settings.calibrationRotationMatrices.first
			: &k2app::K2Settings.calibrationRotationMatrices.second;
	Eigen::Matrix<double, 1, 3>* calibrationTranslation = // Translation
		general_current_calibrating_device == general_calibrating_device::K2_BaseDevice
			? &k2app::K2Settings.calibrationTranslationVectors.first
			: &k2app::K2Settings.calibrationTranslationVectors.second;
	Eigen::Vector3d* calibrationOrigin = // Origin
		general_current_calibrating_device == general_calibrating_device::K2_BaseDevice
			? &k2app::K2Settings.calibrationOrigins.first
			: &k2app::K2Settings.calibrationOrigins.second;
	double* calibrationYaw = // Yaw
		general_current_calibrating_device == general_calibrating_device::K2_BaseDevice
			? &k2app::K2Settings.calibrationYaws.first
			: &k2app::K2Settings.calibrationYaws.second;
	bool* isMatrixCalibrated = // Are we calibrated?
		general_current_calibrating_device == general_calibrating_device::K2_BaseDevice
			? &k2app::K2Settings.isMatrixCalibrated.first
			: &k2app::K2Settings.isMatrixCalibrated.second;
	bool* autoCalibration = // Which calibration method did we use
		general_current_calibrating_device == general_calibrating_device::K2_BaseDevice
			? &k2app::K2Settings.autoCalibration.first
			: &k2app::K2Settings.autoCalibration.second;

	// Mark what are we doing
	*autoCalibration = false;

	// Mark as calibrated for the preview
	*isMatrixCalibrated = true;

	// Set up (a lot of) helper variables
	bool calibration_first_time = true;

	Eigen::AngleAxisd rollAngle(0.f, Eigen::Vector3d::UnitZ());
	Eigen::AngleAxisd yawAngle(0.f, Eigen::Vector3d::UnitY());
	Eigen::AngleAxisd pitchAngle(0.f, Eigen::Vector3d::UnitX());
	Eigen::Quaterniond q = rollAngle * yawAngle * pitchAngle;

	Eigen::Matrix3d rotationMatrix = q.matrix();
	double temp_yaw = 0, temp_pitch = 0;

	// Copy the empty matrices to settings
	*calibrationRotation = rotationMatrix;

	// Loop over until finished
	while (!k2app::interfacing::calibration_confirm)
	{
		// Wait for a mode switch
		while (!k2app::interfacing::calibration_modeSwap && !k2app::interfacing::calibration_confirm)
		{
			const double _multiplexer = k2app::interfacing::calibration_fineTune ? .0015 : .015;

			// Compute the translation delta for the current calibration frame
			Eigen::Vector3d _currentCalibrationTranslation_new{
				k2app::interfacing::calibration_joystick_positions[0][0], // Left X
				k2app::interfacing::calibration_joystick_positions[1][1], // Right Y
				-k2app::interfacing::calibration_joystick_positions[0][1] // Left Y (inv)
			};

			// Apply the multiplexer
			_currentCalibrationTranslation_new = _currentCalibrationTranslation_new * _multiplexer;

			// Un-rotate the translation (sometimes broken due to SteamVR playspace)
			_currentCalibrationTranslation_new =
				k2app::interfacing::vrPlayspaceOrientationQuaternion.cast<double>().inverse() *
				_currentCalibrationTranslation_new;

			// Apply to the global base
			(*calibrationTranslation)(0) += _currentCalibrationTranslation_new(0);
			(*calibrationTranslation)(1) += _currentCalibrationTranslation_new(1);
			(*calibrationTranslation)(2) += _currentCalibrationTranslation_new(2);

			// Sleep on UI
			apartment_context ui_thread;
			co_await resume_background();
			Sleep(5);
			co_await ui_thread;

			// Exit if aborted
			if (!CalibrationPending)break;
		}

		// Play mode swap sound
		if (CalibrationPending && !k2app::interfacing::calibration_confirm)
			playAppSound(k2app::interfacing::sounds::AppSounds::CalibrationPointCaptured);

		// Set up the calibration origin
		if (calibration_first_time)
			*calibrationOrigin = (
				general_current_calibrating_device == general_calibrating_device::K2_BaseDevice
					? k2app::interfacing::kinectWaistPosition.first
					: k2app::interfacing::kinectWaistPosition.second
			).cast<double>();

		// Cache the calibration first_time
		calibration_first_time = false;

		// Sleep on UI -> wait for mode switch
		{
			apartment_context ui_thread;
			co_await resume_background();
			Sleep(300);
			co_await ui_thread;
		}

		// Wait for a mode switch
		while (!k2app::interfacing::calibration_modeSwap && !k2app::interfacing::calibration_confirm)
		{
			const double _multiplexer = k2app::interfacing::calibration_fineTune ? 0.1 : 1.0;

			temp_yaw +=
				(k2app::interfacing::calibration_joystick_positions[0][0] * 3.14159265358979323846 / 280.) *
				_multiplexer; // Left X
			temp_pitch +=
				(k2app::interfacing::calibration_joystick_positions[1][1] * 3.14159265358979323846 / 280.) *
				_multiplexer; // Right Y

			Eigen::AngleAxisd rollAngle(0.f, Eigen::Vector3d::UnitZ());
			Eigen::AngleAxisd yawAngle(temp_yaw, Eigen::Vector3d::UnitY());
			Eigen::AngleAxisd pitchAngle(temp_pitch, Eigen::Vector3d::UnitX());
			Eigen::Quaterniond q = rollAngle * yawAngle * pitchAngle;

			Eigen::Matrix3d rotationMatrix = q.matrix();
			*calibrationRotation = rotationMatrix;

			*calibrationYaw = temp_yaw; // Note: radians

			// Sleep on UI
			apartment_context ui_thread;
			co_await resume_background();
			Sleep(5);
			co_await ui_thread;

			// Exit if aborted
			if (!CalibrationPending)break;
		}

		// Sleep on UI -> wait for mode switch
		{
			apartment_context ui_thread;
			co_await resume_background();
			Sleep(300);
			co_await ui_thread;
		}

		// Play mode swap sound
		if (CalibrationPending && !k2app::interfacing::calibration_confirm)
			playAppSound(k2app::interfacing::sounds::AppSounds::CalibrationPointCaptured);

		// Exit if aborted
		if (!CalibrationPending)break;
	}

	// Reset by re-reading the settings if aborted
	if (!CalibrationPending)
	{
		*isMatrixCalibrated = false;
		k2app::K2Settings.readSettings();

		playAppSound(k2app::interfacing::sounds::AppSounds::CalibrationAborted);
	}
	// Else save I guess
	else
	{
		k2app::K2Settings.saveSettings();
		playAppSound(k2app::interfacing::sounds::AppSounds::CalibrationComplete);

		// Sleep on UI
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(1000); // Just right
		co_await ui_thread;
	}

	// Exit the pane and reset
	CalibrationSelectView().DisplayMode(Controls::SplitViewDisplayMode::Overlay);
	CalibrationSelectView().IsPaneOpen(false);

	CalibrationRunningView().DisplayMode(Controls::SplitViewDisplayMode::Overlay);
	CalibrationRunningView().IsPaneOpen(false);

	AllowNavigation(true);
	k2app::interfacing::currentAppState = L"general";

	k2app::interfacing::calibration_confirm = false;

	CalibrationPending = false; // We're finished
	k2app::K2Settings.skeletonPreviewEnabled = show_skeleton_previous; // Change to whatever
	skeleton_visibility_set_ui(show_skeleton_previous); // Change to whatever
}


void Amethyst::implementation::GeneralPage::ToggleTrackersButton_Checked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't check if setup's finished since we're gonna emulate a click rather than change the state only
	ToggleTrackersButton().Content(box_value(
		k2app::interfacing::LocalizedResourceWString(
			L"GeneralPage", L"Buttons/TrackersToggle/Disconnect")));

	// Optionally spawn trackers
	if (!k2app::interfacing::K2AppTrackersSpawned)
	{
		if (!k2app::interfacing::SpawnEnabledTrackers()) // Mark as spawned
		{
			k2app::interfacing::serverDriverFailure = true; // WAAAAAAA
			k2app::interfacing::K2ServerDriverSetup(); // Refresh
			k2app::interfacing::ShowToast(
				k2app::interfacing::LocalizedResourceWString(L"SharedStrings", L"Toasts/AutoSpawnFailed/Title"),
				k2app::interfacing::LocalizedResourceWString(L"SharedStrings", L"Toasts/AutoSpawnFailed"),
				true); // High priority - it's probably a server failure

			k2app::interfacing::ShowVRToast(
				k2app::interfacing::LocalizedJSONString(L"/SharedStrings/Toasts/AutoSpawnFailed/Title"),
				k2app::interfacing::LocalizedJSONString(L"/SharedStrings/Toasts/AutoSpawnFailed"));
		}

		// Update things
		k2app::interfacing::UpdateServerStatusUI();
	}

	// Give up if failed
	if (k2app::interfacing::serverDriverFailure)return;

	// Mark trackers as active
	k2app::interfacing::K2AppTrackersInitialized = true;

	// Request a check for already-added trackers
	k2app::interfacing::alreadyAddedTrackersScanRequested = true;

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::TrackersConnected);

	// Show additional controls
	CalibrationButton().IsEnabled(true);
}


void Amethyst::implementation::GeneralPage::ToggleTrackersButton_Unchecked(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Don't check if setup's finished since we're gonna emulate a click rather than change the state only
	ToggleTrackersButton().Content(box_value(
		k2app::interfacing::LocalizedResourceWString(
			L"GeneralPage", L"Buttons/TrackersToggle/Reconnect")));

	// Mark trackers as inactive
	k2app::interfacing::K2AppTrackersInitialized = false;

	// Request a check for already-added trackers
	k2app::interfacing::alreadyAddedTrackersScanRequested = true;

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::TrackersDisconnected);

	// Hide additional controls
	CalibrationButton().IsEnabled(false);
}


void Amethyst::implementation::GeneralPage::OpenDiscordButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);

	ShellExecuteA(nullptr, nullptr, "https://discord.gg/YBQCRDG", nullptr, nullptr, SW_SHOW);
}


void Amethyst::implementation::GeneralPage::OpenDocsButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);

	const auto _device_error_string = ErrorWhatGrid().Visibility() == Visibility::Collapsed
		                                  ? TrackingDeviceErrorLabel().Text()
		                                  : OverrideTrackingDeviceErrorLabel().Text();

	const auto _device_name = ErrorWhatGrid().Visibility() == Visibility::Collapsed
		                          ? SelectedDeviceNameLabel().Text()
		                          : SelectedOverrideDeviceNameLabel().Text();

	if (_device_name == L"Xbox 360 Kinect")
	{
		if (_device_error_string == L"E_NUI_NOTPOWERED")
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/360/troubleshooting/notpowered/",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);

		else if (_device_error_string == L"E_NUI_NOTREADY")
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/360/troubleshooting/notready/",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);

		else if (_device_error_string == L"E_NUI_NOTGENUINE")
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/360/troubleshooting/notgenuine/",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);

		else if (_device_error_string == L"E_NUI_INSUFFICIENTBANDWIDTH")
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/360/troubleshooting/insufficientbandwidth",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);

		else
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/app/help/",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);
	}

	else if (_device_name == L"Xbox One Kinect")
	{
		if (_device_error_string == L"E_NOTAVAILABLE")
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/one/troubleshooting",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);

		else
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/app/help/",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);
	}

	else if (_device_name == L"PSMove Service")
	{
		if (_device_error_string == L"E_PSMS_NOT_RUNNING")
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/psmove/troubleshooting",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);

		else if (_device_error_string == L"E_PSMS_NO_JOINTS")
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/psmove/troubleshooting",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);

		else
			ShellExecuteW(nullptr, nullptr,
			              std::format(L"https://docs.k2vr.tech/{}/app/help/",
			                          k2app::interfacing::docsLanguageCode).c_str(),
			              nullptr, nullptr, SW_SHOW);
	}

	else
		ShellExecuteW(nullptr, nullptr,
		              std::format(L"https://docs.k2vr.tech/{}/app/help/", k2app::interfacing::docsLanguageCode).c_str(),
		              nullptr, nullptr, SW_SHOW);
}


void Amethyst::implementation::GeneralPage::ServerOpenDocsButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);

	const auto _server_status_code = ServerErrorLabel().Text();

	if (_server_status_code == L"E_EXCEPTION_WHILE_CHECKING")
		ShellExecuteW(nullptr, nullptr,
		              std::format(L"https://docs.k2vr.tech/{}/app/steamvr-driver-codes/#2",
		                          k2app::interfacing::docsLanguageCode).c_str(),
		              nullptr, nullptr, SW_SHOW);

	else if (_server_status_code == L"E_CONNECTION_ERROR")
		ShellExecuteW(nullptr, nullptr,
		              std::format(L"https://docs.k2vr.tech/{}/app/steamvr-driver-codes/#3",
		                          k2app::interfacing::docsLanguageCode).c_str(),
		              nullptr, nullptr, SW_SHOW);

	else
		ShellExecuteW(nullptr, nullptr,
		              std::format(L"https://docs.k2vr.tech/{}/app/steamvr-driver-codes/#5",
		                          k2app::interfacing::docsLanguageCode).c_str(),
		              nullptr, nullptr, SW_SHOW);
}


void Amethyst::implementation::GeneralPage::GeneralPage_Loaded(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	LOG(INFO) << "Re/Loading page with tag: \"general\"...";
	k2app::interfacing::currentAppState = L"general";

	// Execute the handler
	GeneralPage_Loaded_Handler();

	// Mark as loaded
	general_loadedOnce = true;
}


void Amethyst::implementation::GeneralPage::GeneralPage_Loaded_Handler()
{
	// Load strings (must be the first thing we're doing)

	SaveOffsetsButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Buttons/Save")));

	DiscardOffsetsButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Buttons/Discard")));

	Calibration_Titles_Choose().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Titles/Choose"));

	Calibration_Captions_Recommended().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Captions/Recommended"));

	Calibration_Titles_Automatic().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Titles/Automatic"));

	Calibration_Captions_Automatic().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Captions/Automatic"));

	Calibration_Titles_Manual().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Titles/Manual"));

	Calibration_Captions_Manual().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Captions/Manual"));

	CancelCalibrationButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Buttons/Cancel")));

	Calibration_Headers_Automatic().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Headers/Automatic"));

	CalibrationInstructionsLabel().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Captions/Start"));

	StartAutoCalibrationButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Buttons/Calibration/Begin")));

	Calibration_Titles_Points().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Titles/Points"));

	Calibration_Captions_Points().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Captions/Points"));

	DiscardAutoCalibrationButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Buttons/Cancel")));

	Calibration_Headers_Manual().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Headers/Manual"));

	Calibration_Labels_Brief_Left().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Labels/Brief/Left"));

	Calibration_LabelsWhat_Brief_Left().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/LabelsWhat/Brief/Left"));

	Calibration_Labels_Brief_Right().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Labels/Brief/Right"));

	Calibration_LabelsWhat_Brief_Right().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/LabelsWhat/Brief/Right"));

	Calibration_Labels_Grips_Left().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Labels/Grips/Left"));

	Calibration_Labels_Triggers_Left().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Labels/Triggers/Left"));

	Calibration_LabelsWhat_Grips_Left().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/LabelsWhat/Grips/Left"));

	Calibration_Labels_Grips_Right().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Labels/Grips/Right"));

	Calibration_Labels_Grips_Right().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Labels/Grips/Right"));

	Calibration_LabelsWhat_Grips_Right().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/LabelsWhat/Grips/Right"));

	Calibration_Labels_Triggers_Both().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Labels/Triggers/Both"));

	Calibration_LabelsWhat_Triggers().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/LabelsWhat/Triggers"));

	Calibration_Labels_InputsNotice().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Calibration/Labels/InputsNotice"));

	DiscardCalibrationButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Buttons/Cancel")));

	Titles_Trackers().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Titles/Trackers"));

	CalibrationButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Buttons/Calibration/Begin")));

	BaseCalibration().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Buttons/Calibration/Base")));

	OverrideCalibration().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Buttons/Calibration/Override")));

	Captions_CalibrateOverrides().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/CalibrateOverrides"));

	OffsetsButton().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Buttons/Offsets"));

	ToggleTrackersButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Buttons/TrackersToggle/Connect")));

	Titles_Device().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Titles/Device"));

	Captions_Device_Name().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/Device/Name"));

	Captions_Device_Status().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/Device/Status"));

	OpenDiscordButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Buttons/Help/Discord")));

	OpenDocsButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Buttons/Help/Docs")));

	Captions_OverrideDevice_Name().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/OverrideDevice/Name"));

	Titles_Status().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Titles/Status"));

	Captions_DriverStatus_Label().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/DriverStatus/Label"));

	ServerStatusLabel().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/DriverStatus"));

	ReRegisterButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Buttons/ReRegister")));

	Captions_VersionText().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/VersionText"));

	Captions_Preview_NoSkeleton().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/Preview/NoSkeleton"));

	NoSkeletonTextNotice().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/Preview/NoSkeletonText"));

	Captions_Preview_Disabled().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/Preview/Disabled"));

	Captions_Preview_DisabledText().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/Preview/DisabledText"));

	Captions_Preview_NoFocus().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/Preview/NoFocus"));

	Captions_Preview_NoFocusText().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/Preview/NoFocusText"));

	Captions_Preview_NoDashboard().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/Preview/NoDashboard"));

	Captions_Preview_NoDashboardText().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/Preview/NoDashboardText"));

	Labels_Tracking().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Labels/Tracking"));

	Labels_Inferred().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Labels/Inferred"));

	SkeletonToggleButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Buttons/Skeleton/Hide")));

	ForceRenderText().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/Preview/Force"));

	ToggleTrackingButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Buttons/Skeleton/Freeze")));

	FreezeOnlyLowerToggle().Text(
		k2app::interfacing::LocalizedJSONString(L"/GeneralPage/Captions/FreezeLowerOnly"));

	DismissSetErrorButton().Content(box_value(
		k2app::interfacing::LocalizedJSONString(L"/SettingsPage/Buttons/Error/Dismiss")));

	ToggleTrackersTeachingTip().Title(
		k2app::interfacing::LocalizedJSONString(L"/NUX/Tip2/Title"));
	ToggleTrackersTeachingTip().Subtitle(
		k2app::interfacing::LocalizedJSONString(L"/NUX/Tip2/Content"));
	ToggleTrackersTeachingTip().CloseButtonContent(
		box_value(k2app::interfacing::LocalizedJSONString(L"/NUX/Next")));
	ToggleTrackersTeachingTip().ActionButtonContent(
		box_value(k2app::interfacing::LocalizedJSONString(L"/NUX/Prev")));

	CalibrationTeachingTip().Title(
		k2app::interfacing::LocalizedJSONString(L"/NUX/Tip3/Title"));
	CalibrationTeachingTip().Subtitle(
		k2app::interfacing::LocalizedJSONString(L"/NUX/Tip3/Content"));
	CalibrationTeachingTip().CloseButtonContent(
		box_value(k2app::interfacing::LocalizedJSONString(L"/NUX/Next")));
	CalibrationTeachingTip().ActionButtonContent(
		box_value(k2app::interfacing::LocalizedJSONString(L"/NUX/Prev")));

	StatusTeachingTip().Title(
		k2app::interfacing::LocalizedJSONString(L"/NUX/Tip4/Title"));
	StatusTeachingTip().Subtitle(
		k2app::interfacing::LocalizedJSONString(L"/NUX/Tip4/Content"));
	StatusTeachingTip().CloseButtonContent(
		box_value(k2app::interfacing::LocalizedJSONString(L"/NUX/Next")));
	StatusTeachingTip().ActionButtonContent(
		box_value(k2app::interfacing::LocalizedJSONString(L"/NUX/Prev")));

	// Start the main loop since we're done with basic setup
	k2app::shared::devices::smphSignalStartMain.release();

	// Update the internal version
	if (versionLabel.get() != nullptr)
		versionLabel.get()->Text(
			L"v" + StringToWString(k2app::interfacing::K2InternalVersion));

	// Try auto-spawning trackers if stated so
	if (!general_tab_setup_finished && // If first-time
		k2app::interfacing::isServerDriverPresent && // If the driver's ok
		k2app::K2Settings.autoSpawnEnabledJoints) // If autospawn
	{
		if (k2app::interfacing::SpawnEnabledTrackers())
		{
			// Mark as spawned
			toggleTrackersButton->IsChecked(true);
			CalibrationButton().IsEnabled(true);
		}

		// Cry about it
		else
		{
			k2app::interfacing::serverDriverFailure = true; // WAAAAAAA
			k2app::interfacing::K2ServerDriverSetup(); // Refresh
			k2app::interfacing::ShowToast(
				k2app::interfacing::LocalizedResourceWString(L"SharedStrings", L"Toasts/AutoSpawnFailed/Title"),
				k2app::interfacing::LocalizedResourceWString(L"SharedStrings", L"Toasts/AutoSpawnFailed"),
				true); // High priority - it's probably a server failure

			k2app::interfacing::ShowVRToast(
				k2app::interfacing::LocalizedJSONString(L"/SharedStrings/Toasts/AutoSpawnFailed/Title"),
				k2app::interfacing::LocalizedJSONString(L"/SharedStrings/Toasts/AutoSpawnFailed"));
		}
	}

	// Refresh the server status
	k2app::interfacing::K2ServerDriverRefresh();

	// Update things
	k2app::interfacing::UpdateServerStatusUI();
	TrackingDevices::updateTrackingDeviceUI();
	TrackingDevices::updateOverrideDeviceUI();

	// Notice that we're gonna change some values
	pending_offsets_update = true;

	// Load values into number boxes
	offsetsController->ReAppendTrackerPivots();

	// Notice that we're finished
	pending_offsets_update = false;

	// Notify of the setup's end
	general_tab_setup_finished = true;

	// Setup the preview button
	skeleton_visibility_set_ui(k2app::K2Settings.skeletonPreviewEnabled);
	skeleton_force_set_ui(k2app::K2Settings.forceSkeletonPreview);

	// Setup the freeze button
	toggleFreezeButton.get()->IsChecked(k2app::interfacing::isTrackingFrozen);
	toggleFreezeButton.get()->Content(k2app::interfacing::isTrackingFrozen
		                                  ? box_value(
			                                  k2app::interfacing::LocalizedResourceWString(
				                                  L"GeneralPage",
				                                  L"Buttons/Skeleton/Unfreeze"))
		                                  : box_value(
			                                  k2app::interfacing::LocalizedResourceWString(
				                                  L"GeneralPage",
				                                  L"Buttons/Skeleton/Freeze")));

	// Set up the co/re/disconnect button
	if (!k2app::interfacing::K2AppTrackersSpawned)
	{
		toggleTrackersButton.get()->Content(box_value(
			k2app::interfacing::LocalizedResourceWString(
				L"GeneralPage", L"Buttons/TrackersToggle/Connect")));

		toggleTrackersButton.get()->IsChecked(false);
	}
	else
	{
		toggleTrackersButton.get()->IsChecked(k2app::interfacing::K2AppTrackersInitialized);
		toggleTrackersButton.get()->Content(k2app::interfacing::K2AppTrackersInitialized
			                                    ? box_value(
				                                    k2app::interfacing::LocalizedResourceWString(
					                                    L"GeneralPage",
					                                    L"Buttons/TrackersToggle/Disconnect"))
			                                    : box_value(
				                                    k2app::interfacing::LocalizedResourceWString(
					                                    L"GeneralPage",
					                                    L"Buttons/TrackersToggle/Reconnect")));
	}

	freezeOnlyLowerToggle->IsChecked(k2app::K2Settings.freezeLowerOnly);
}


void Amethyst::implementation::GeneralPage::sk_line(
	Shapes::Line& line,
	const std::array<Eigen::Vector3f, 25>& joints,
	const std::array<ktvr::ITrackedJointState, 25>& states,
	const ktvr::ITrackedJointType& from,
	const ktvr::ITrackedJointType& to)
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

	line.StrokeThickness(5);

	if (states[from] != ktvr::State_Tracked ||
		states[to] != ktvr::State_Tracked)
		line.Stroke(*k2app::shared::main::attentionBrush);
	else
		line.Stroke(Media::SolidColorBrush(Windows::UI::Colors::White()));

	// Select the smaller scale to preserve somewhat uniform skeleton scaling
	const double s_scale_w = s_mat_width / s_mat_width_default,
	             s_scale_h = s_mat_height / s_mat_height_default;

	line.X1(joints[from].x() * 300. * std::min(s_scale_w, s_scale_h) * s_from_multiply + s_mat_width / 2.);
	line.Y1(joints[from].y() * -300. * std::min(s_scale_w, s_scale_h) * s_from_multiply + s_mat_height / 3.);

	line.X2(joints[to].x() * 300. * std::min(s_scale_w, s_scale_h) * s_to_multiply + s_mat_width / 2.);
	line.Y2(joints[to].y() * -300. * std::min(s_scale_w, s_scale_h) * s_to_multiply + s_mat_height / 3.);

	line.Visibility(Visibility::Visible);
}


// the tuple goes like <position, rotation>
void Amethyst::implementation::GeneralPage::sk_dot(
	Shapes::Ellipse& ellipse,
	const Eigen::Vector3f& joint,
	const ktvr::ITrackedJointState& state,
	const std::pair<bool, bool>& isOverridden)
{
	constexpr double s_mat_width_default = 700,
	                 s_mat_height_default = 600;

	constexpr double s_ellipse_wh = 12, s_ellipse_stroke = 2;

	double s_mat_width = SkeletonDrawingCanvas().ActualWidth(),
	       s_mat_height = SkeletonDrawingCanvas().ActualHeight();

	// Eventually fix sizes
	if (s_mat_height < 1)s_mat_height = s_mat_height_default;
	if (s_mat_height < 1)s_mat_width = s_mat_width_default;

	// Where to scale by 1.0 in perspective
	constexpr double s_normal_distance = 3;

	// Compose perspective constants, make it 70%
	const double s_multiply = .7 * (s_normal_distance / (joint.z() > 0. ? joint.z() : 3.));

	auto a = Media::AcrylicBrush();
	auto ui = Windows::UI::ViewManagement::UISettings();

	ellipse.StrokeThickness(s_ellipse_stroke);
	ellipse.Width(s_ellipse_wh);
	ellipse.Height(s_ellipse_wh);

	a.TintColor(ui.GetColorValue(Windows::UI::ViewManagement::UIColorType::Accent));

	if (state != ktvr::State_Tracked)
	{
		ellipse.Stroke(a);
		ellipse.Fill(a);
	}
	else
	{
		ellipse.Stroke(Media::SolidColorBrush(Windows::UI::Colors::White()));
		ellipse.Fill(Media::SolidColorBrush(Windows::UI::Colors::White()));
	}

	// Change the stroke based on overrides
	if (isOverridden.first && isOverridden.second) // Both
		ellipse.Stroke(Media::SolidColorBrush(Windows::UI::Colors::BlueViolet()));
	if (isOverridden.first && !isOverridden.second) // Rotation
		ellipse.Stroke(Media::SolidColorBrush(Windows::UI::Colors::DarkOliveGreen()));
	if (!isOverridden.first && isOverridden.second) // Position
		ellipse.Stroke(Media::SolidColorBrush(Windows::UI::Colors::IndianRed()));

	// Select the smaller scale to preserve somewhat uniform skeleton scaling
	const double s_scale_w = s_mat_width / s_mat_width_default,
	             s_scale_h = s_mat_height / s_mat_height_default;

	// Move the ellipse to the appropriate point
	ellipse.Margin({
		// Left
		joint.x() * 300. * std::min(s_scale_w, s_scale_h) * s_multiply +
		s_mat_width / 2. - (s_ellipse_wh + s_ellipse_stroke) / 2.,

		// Top
		joint.y() * -300. * std::min(s_scale_w, s_scale_h) * s_multiply +
		s_mat_height / 3. - (s_ellipse_wh + s_ellipse_stroke) / 2.,

		// Not used
		0, 0
	});
	ellipse.Visibility(Visibility::Visible);
}


std::pair<HWND, hresult> GetHWNDFromWindow(const Window& window)
{
	HWND nativeWindow{nullptr};
	hresult result = window.as<IWindowNative>()->get_WindowHandle(&nativeWindow);
	return std::make_pair(nativeWindow, result);
}


bool IsCurrentWindowActive()
{
	if (k2app::shared::main::thisWindow.get() == nullptr)
		return true; // Give up k?

	if (const auto [h_handle, h_result] =
			GetHWNDFromWindow(*k2app::shared::main::thisWindow);

		h_result >= 0) // From winrt::check_hresult
		return GetActiveWindow() == h_handle;

	return true; // (else) Give up k?
}


bool IsDashboardOpen()
{
	// Check if we're running on null
	char system_name[1024];
	vr::VRSystem()->GetStringTrackedDeviceProperty(
		vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String, system_name, 1024);

	// Just return true for debug reasons
	if (strcmp(system_name, "null") == 0)
		return true;

	// Also check if we're not idle / standby
	const auto stat = vr::VRSystem()->GetTrackedDeviceActivityLevel(vr::k_unTrackedDeviceIndex_Hmd);
	if (stat != vr::k_EDeviceActivityLevel_UserInteraction &&
		stat != vr::k_EDeviceActivityLevel_UserInteraction_Timeout)
		return false; // Standby - hide

	// Check if the dashboard is open
	return vr::VROverlay()->IsDashboardVisible();
}


std::pair<bool, bool> IsJointUsedAsOverride(const uint32_t& joint)
{
	std::pair _o{false, false};

	// Scan for position overrides
	for (const auto& _j_p : k2app::K2Settings.K2TrackersVector)
		if (joint == _j_p.positionOverrideJointID)_o.first = true;

	// Scan for rotation overrides
	for (const auto& _j_r : k2app::K2Settings.K2TrackersVector)
		if (joint == _j_r.rotationOverrideJointID)_o.second = true;

	return (k2app::K2Settings.overrideDeviceID >= 0)
		       ? _o
		       : std::make_pair(false, false);
}


std::pair<bool, bool> IsJointOverriden(const uint32_t& joint)
{
	return (k2app::K2Settings.overrideDeviceID >= 0)
		       ? std::make_pair(
			       k2app::K2Settings.K2TrackersVector.at(joint).isPositionOverridden,
			       k2app::K2Settings.K2TrackersVector.at(joint).isRotationOverridden)
		       : std::make_pair(false, false);
}


bool isCurrentWindowActive_backup = false;

void Amethyst::implementation::GeneralPage::SkeletonDrawingCanvas_Loaded(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	static auto boneLines = std::array<Shapes::Line, 24>();
	static auto jointDots = std::array<Shapes::Ellipse, 25>(); // For now this is MAX

	SkeletonDrawingCanvas().Children().Clear();
	for (auto& l : boneLines)
	{
		l = Shapes::Line();
		SkeletonDrawingCanvas().Children().Append(l);
	}
	for (auto& el : jointDots)
	{
		el = Shapes::Ellipse();
		SkeletonDrawingCanvas().Children().Append(el);
	}

	auto timer = DispatcherTimer();
	timer.Interval(std::chrono::milliseconds(33));

	timer.Tick([&, this](const IInspectable& sender, const IInspectable& e)
	{
		const bool isCurrentWindowActive = IsCurrentWindowActive();

		if (isCurrentWindowActive_backup != isCurrentWindowActive &&
			k2app::shared::main::appTitleLabel.get() != nullptr)
		{
			k2app::shared::main::appTitleLabel.get()->Opacity(isCurrentWindowActive ? 1.0 : 0.5);
			isCurrentWindowActive_backup = isCurrentWindowActive;
		}

		// If we've disabled the preview
		if (!k2app::K2Settings.skeletonPreviewEnabled)
		{
			// Hide the UI, only show that viewing is disabled
			SkeletonDrawingCanvas().Opacity(0.0);
			TrackingStateLabelsPanel().Opacity(0.0);

			NotTrackedNotice().Opacity(0.0);
			NotInFocusNotice().Opacity(0.0);
			DashboardClosedNotice().Opacity(0.0);
			SkeletonHiddenNotice().Opacity(1.0);

			return; // Nothing more to do anyway
		}

		// If the preview isn't forced
		if (!k2app::K2Settings.forceSkeletonPreview)
		{
			// If the dashboard's closed
			if (k2app::K2Settings.skeletonPreviewEnabled && !IsDashboardOpen())
			{
				// Hide the UI, only show that viewing is disabled
				SkeletonDrawingCanvas().Opacity(0.0);
				TrackingStateLabelsPanel().Opacity(0.0);

				NotTrackedNotice().Opacity(0.0);
				SkeletonHiddenNotice().Opacity(0.0);
				NotInFocusNotice().Opacity(0.0);
				DashboardClosedNotice().Opacity(1.0);

				return; // Nothing more to do anyway
			}

			// If we're out of focus (skip if we're gonna do a VROverlay)
			if (k2app::K2Settings.skeletonPreviewEnabled && !isCurrentWindowActive)
			{
				// Hide the UI, only show that viewing is disabled
				SkeletonDrawingCanvas().Opacity(0.0);
				TrackingStateLabelsPanel().Opacity(0.0);

				NotTrackedNotice().Opacity(0.0);
				SkeletonHiddenNotice().Opacity(0.0);
				DashboardClosedNotice().Opacity(0.0);
				NotInFocusNotice().Opacity(1.0);

				return; // Nothing more to do anyway
			}
		}

		SkeletonHiddenNotice().Opacity(0.0); // Else hide
		NotInFocusNotice().Opacity(0.0); // Else hide
		DashboardClosedNotice().Opacity(0.0); // Else hide

		const auto& trackingDevice = TrackingDevices::getCurrentDevice();

		switch (trackingDevice.index())
		{
		case 0:
			{
				const auto& device = std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(trackingDevice);

				const auto joints = device->getJointPositions();
				const auto states = device->getTrackingStates();

				StartAutoCalibrationButton().IsEnabled(
					device->isSkeletonTracked() && !CalibrationPending && !AutoCalibration_StillPending);

				if (device->isSkeletonTracked())
				{
					// Don't waste cpu & ram, ok?

					// Show the UI
					SkeletonDrawingCanvas().Opacity(1.0);
					TrackingStateLabelsPanel().Opacity(1.0);

					NotTrackedNotice().Opacity(0.0);

					if (device->getDeviceCharacteristics() == ktvr::K2_Character_Full)
					{
						// Clear joints
						for (auto& l : jointDots)
							l.Visibility(Visibility::Collapsed);

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

						// Waist
						sk_dot(jointDots[1], joints[ktvr::Joint_SpineWaist], states[ktvr::Joint_SpineWaist],
						       IsJointOverriden(0));

						// Left Foot
						sk_dot(jointDots[2], joints[ktvr::Joint_AnkleLeft], states[ktvr::Joint_AnkleLeft],
						       IsJointOverriden(1));

						// Right Foot
						sk_dot(jointDots[3], joints[ktvr::Joint_AnkleRight], states[ktvr::Joint_AnkleRight],
						       IsJointOverriden(2));

						// Left Elbow
						sk_dot(jointDots[4], joints[ktvr::Joint_ElbowLeft], states[ktvr::Joint_ElbowLeft],
						       IsJointOverriden(3));

						// Right Elbow
						sk_dot(jointDots[5], joints[ktvr::Joint_ElbowRight], states[ktvr::Joint_ElbowRight],
						       IsJointOverriden(4));

						// Left Knee
						sk_dot(jointDots[6], joints[ktvr::Joint_KneeLeft], states[ktvr::Joint_KneeLeft],
						       IsJointOverriden(5));

						// Right Knee
						sk_dot(jointDots[7], joints[ktvr::Joint_KneeRight], states[ktvr::Joint_KneeRight],
						       IsJointOverriden(6));
					}
					else if (device->getDeviceCharacteristics() == ktvr::K2_Character_Simple)
					{
						// Clear joints
						for (auto& l : jointDots)
							l.Visibility(Visibility::Collapsed);

						// Draw the skeleton with from-to lines
						// Head
						sk_dot(jointDots[0], joints[ktvr::Joint_Head], states[ktvr::Joint_Head],
						       std::make_pair(false, false));

						// Waist
						sk_dot(jointDots[1], joints[ktvr::Joint_SpineWaist], states[ktvr::Joint_SpineWaist],
						       IsJointOverriden(0));

						// Left Foot
						sk_dot(jointDots[2], joints[ktvr::Joint_AnkleLeft], states[ktvr::Joint_AnkleLeft],
						       IsJointOverriden(1));

						// Right Foot
						sk_dot(jointDots[3], joints[ktvr::Joint_AnkleRight], states[ktvr::Joint_AnkleRight],
						       IsJointOverriden(2));

						// Left Elbow
						sk_dot(jointDots[4], joints[ktvr::Joint_ElbowLeft], states[ktvr::Joint_ElbowLeft],
						       IsJointOverriden(3));

						// Right Elbow
						sk_dot(jointDots[5], joints[ktvr::Joint_ElbowRight], states[ktvr::Joint_ElbowRight],
						       IsJointOverriden(4));

						// Left Knee
						sk_dot(jointDots[6], joints[ktvr::Joint_KneeLeft], states[ktvr::Joint_KneeLeft],
						       IsJointOverriden(5));

						// Right Knee
						sk_dot(jointDots[7], joints[ktvr::Joint_KneeRight], states[ktvr::Joint_KneeRight],
						       IsJointOverriden(6));

						// Empty lines
						boneLines[0] = Shapes::Line();
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
						boneLines[16] = Shapes::Line();
						boneLines[17] = Shapes::Line();

						// Lower left limb
						sk_line(boneLines[18], joints, states, ktvr::Joint_SpineWaist, ktvr::Joint_KneeLeft);
						sk_line(boneLines[19], joints, states, ktvr::Joint_KneeLeft, ktvr::Joint_AnkleLeft);
						sk_line(boneLines[20], joints, states, ktvr::Joint_AnkleLeft, ktvr::Joint_FootLeft);

						// Lower right limb
						sk_line(boneLines[21], joints, states, ktvr::Joint_SpineWaist, ktvr::Joint_KneeRight);
						sk_line(boneLines[22], joints, states, ktvr::Joint_KneeRight, ktvr::Joint_AnkleRight);
						sk_line(boneLines[23], joints, states, ktvr::Joint_AnkleRight, ktvr::Joint_FootRight);
					}
					else if (device->getDeviceCharacteristics() == ktvr::K2_Character_Basic)
					{
						// Clear bones
						for (auto& l : boneLines)
							l.Visibility(Visibility::Collapsed);

						// Clear joints
						for (auto& l : jointDots)
							l.Visibility(Visibility::Collapsed);

						// Draw the skeleton with from-to lines
						// Head
						sk_dot(jointDots[0], joints[ktvr::Joint_Head], states[ktvr::Joint_Head],
						       std::make_pair(false, false));

						// Waist
						sk_dot(jointDots[1], joints[ktvr::Joint_SpineWaist], states[ktvr::Joint_SpineWaist],
						       IsJointOverriden(0));

						// Left Foot
						sk_dot(jointDots[2], joints[ktvr::Joint_AnkleLeft], states[ktvr::Joint_AnkleLeft],
						       IsJointOverriden(1));

						// Right Foot
						sk_dot(jointDots[3], joints[ktvr::Joint_AnkleRight], states[ktvr::Joint_AnkleRight],
						       IsJointOverriden(2));
					}
				}
				else
				{
					SkeletonDrawingCanvas().Opacity(0.0);
					TrackingStateLabelsPanel().Opacity(0.0);

					NotTrackedNotice().Opacity(1.0);
				}
			}
			break;
		case 1:
			{
				const auto& device = std::get<ktvr::K2TrackingDeviceBase_JointsBasis*>(trackingDevice);
				auto joints = device->getTrackedJoints();

				StartAutoCalibrationButton().IsEnabled(
					device->isSkeletonTracked() && !CalibrationPending && !AutoCalibration_StillPending);

				if (device->isSkeletonTracked() && !joints.empty())
				{
					// Don't waste cpu & ram, ok?

					// Show the UI
					SkeletonDrawingCanvas().Opacity(1.0);
					TrackingStateLabelsPanel().Opacity(1.0);

					NotTrackedNotice().Opacity(0.0);

					{
						// Clear bones
						for (auto& l : boneLines)
							l.Visibility(Visibility::Collapsed);

						// Clear joints
						for (auto& l : jointDots)
							l.Visibility(Visibility::Collapsed);

						// Get joints and draw points
						for (uint32_t j = 0; j < std::min(static_cast<uint32_t>(joints.size()),
						                                  static_cast<uint32_t>(ktvr::Joint_Total)); j++)
							sk_dot(jointDots[j],
							       joints.at(j).getJointPosition(),
							       joints.at(j).getTrackingState(),
							       IsJointUsedAsOverride(j));
					}
				}
				else
				{
					SkeletonDrawingCanvas().Opacity(0.0);
					TrackingStateLabelsPanel().Opacity(0.0);

					NotTrackedNotice().Opacity(1.0);
				}
			}
			break;
		}
	});

	timer.Start();
}


void Amethyst::implementation::GeneralPage::CalibrationButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	// Capture playspace details one more time, before the calibration
	{
		const auto trackingOrigin = vr::VRSystem()->GetRawZeroPoseToStandingAbsoluteTrackingPose();

		k2app::interfacing::vrPlayspaceTranslation = EigenUtils::p_cast_type<Eigen::Vector3f>(trackingOrigin);
		k2app::interfacing::vrPlayspaceOrientationQuaternion = EigenUtils::p_cast_type<Eigen::Quaternionf>(
			trackingOrigin);

		// Get current yaw angle
		k2app::interfacing::vrPlayspaceOrientation =
			EigenUtils::RotationProjectedYaw(
				k2app::interfacing::vrPlayspaceOrientationQuaternion); // Yaw angle
	}

	// If no overrides
	if (k2app::K2Settings.overrideDeviceID < 0)
	{
		AutoCalibrationPane().Visibility(Visibility::Collapsed);
		ManualCalibrationPane().Visibility(Visibility::Collapsed);

		CalibrationSelectView().DisplayMode(Controls::SplitViewDisplayMode::Inline);
		CalibrationSelectView().IsPaneOpen(true);

		CalibrationRunningView().DisplayMode(Controls::SplitViewDisplayMode::Overlay);
		CalibrationRunningView().IsPaneOpen(false);

		AllowNavigation(false);
		k2app::interfacing::currentAppState = L"calibration";

		show_skeleton_previous = k2app::K2Settings.skeletonPreviewEnabled; // Back up
		k2app::K2Settings.skeletonPreviewEnabled = true; // Change to show
		skeleton_visibility_set_ui(true); // Change to show

		// Play a sound
		playAppSound(k2app::interfacing::sounds::AppSounds::Show);
	}
	else
	{
		ChooseDeviceFlyout().ShowAt(CalibrationButton());

		// Assume no head position providers
		AutoCalibrationButton().IsEnabled(false);
		AutoCalibrationButtonDecorations().Opacity(.5);

		AllowNavigation(true);
	}
}


void Amethyst::implementation::GeneralPage::BaseCalibration_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	ChooseDeviceFlyout().Hide();

	AutoCalibrationPane().Visibility(Visibility::Collapsed);
	ManualCalibrationPane().Visibility(Visibility::Collapsed);

	CalibrationSelectView().DisplayMode(Controls::SplitViewDisplayMode::Inline);
	CalibrationSelectView().IsPaneOpen(true);

	CalibrationRunningView().DisplayMode(Controls::SplitViewDisplayMode::Overlay);
	CalibrationRunningView().IsPaneOpen(false);

	AllowNavigation(false);
	k2app::interfacing::currentAppState = L"calibration";

	show_skeleton_previous = k2app::K2Settings.skeletonPreviewEnabled; // Back up
	k2app::K2Settings.skeletonPreviewEnabled = true; // Change to show
	skeleton_visibility_set_ui(true); // Change to show

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Show);

	// Eventually enable the auto calibration
	const auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(k2app::K2Settings.trackingDeviceID);
	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		if (std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(trackingDevice)->
			getDeviceCharacteristics() == ktvr::K2_Character_Full)
		{
			AutoCalibrationButton().IsEnabled(true);
			AutoCalibrationButtonDecorations().Opacity(1.0);
		}
	}

	// Set the current device for scripts
	general_current_calibrating_device = general_calibrating_device::K2_BaseDevice;
}


void Amethyst::implementation::GeneralPage::OverrideCalibration_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	ChooseDeviceFlyout().Hide();

	AutoCalibrationPane().Visibility(Visibility::Collapsed);
	ManualCalibrationPane().Visibility(Visibility::Collapsed);

	CalibrationSelectView().DisplayMode(Controls::SplitViewDisplayMode::Inline);
	CalibrationSelectView().IsPaneOpen(true);

	CalibrationRunningView().DisplayMode(Controls::SplitViewDisplayMode::Overlay);
	CalibrationRunningView().IsPaneOpen(false);

	AllowNavigation(false);
	k2app::interfacing::currentAppState = L"calibration";

	show_skeleton_previous = k2app::K2Settings.skeletonPreviewEnabled; // Back up
	k2app::K2Settings.skeletonPreviewEnabled = true; // Change to show
	skeleton_visibility_set_ui(true); // Change to show

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Show);

	// Eventually enable the auto calibration
	const auto& trackingDevice = TrackingDevices::TrackingDevicesVector.at(k2app::K2Settings.overrideDeviceID);
	if (trackingDevice.index() == 0)
	{
		// Kinect Basis
		if (std::get<ktvr::K2TrackingDeviceBase_SkeletonBasis*>(trackingDevice)->
			getDeviceCharacteristics() == ktvr::K2_Character_Full)
		{
			AutoCalibrationButton().IsEnabled(true);
			AutoCalibrationButtonDecorations().Opacity(1.0);
		}
	}

	// Set the current device for scripts
	general_current_calibrating_device = general_calibrating_device::K2_OverrideDevice;
}


void Amethyst::implementation::GeneralPage::ToggleTrackingButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	k2app::interfacing::isTrackingFrozen = !k2app::interfacing::isTrackingFrozen;

	toggleFreezeButton.get()->IsChecked(k2app::interfacing::isTrackingFrozen);
	toggleFreezeButton.get()->Content(
		k2app::interfacing::isTrackingFrozen
			? box_value(
				k2app::interfacing::LocalizedResourceWString(
					L"GeneralPage",
					L"Buttons/Skeleton/Unfreeze"))
			: box_value(
				k2app::interfacing::LocalizedResourceWString(
					L"GeneralPage",
					L"Buttons/Skeleton/Freeze")));

	// Play a sound
	playAppSound(k2app::interfacing::isTrackingFrozen
		             ? k2app::interfacing::sounds::AppSounds::ToggleOff
		             : k2app::interfacing::sounds::AppSounds::ToggleOn);

	// Optionally show the binding teaching tip
	if (!k2app::K2Settings.teachingTipShown_Freeze &&
		k2app::interfacing::currentPageTag == L"general")
	{
		auto _header =
			k2app::interfacing::LocalizedResourceWString(
				L"GeneralPage", L"Tips/TrackingFreeze/Header");

		// Change the tip depending on the currently connected controllers
		char _controller_model[1024];
		vr::VRSystem()->GetStringTrackedDeviceProperty(
			vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(
				vr::ETrackedControllerRole::TrackedControllerRole_LeftHand),
			vr::ETrackedDeviceProperty::Prop_ModelNumber_String,
			_controller_model, std::size(_controller_model));

		// For the ""s operator
		using namespace std::string_literals;

		if (k2app::interfacing::findStringIC(_controller_model, "knuckles") ||
			k2app::interfacing::findStringIC(_controller_model, "index"))
			k2app::interfacing::stringReplaceAll(_header, L"{0}"s,
			                                     k2app::interfacing::LocalizedResourceWString(
				                                     L"GeneralPage",
				                                     L"Tips/TrackingFreeze/Buttons/Index"));

		else if (k2app::interfacing::findStringIC(_controller_model, "vive"))
			k2app::interfacing::stringReplaceAll(_header, L"{0}"s,
			                                     k2app::interfacing::LocalizedResourceWString(
				                                     L"GeneralPage",
				                                     L"Tips/TrackingFreeze/Buttons/VIVE"));

		else if (k2app::interfacing::findStringIC(_controller_model, "mr"))
			k2app::interfacing::stringReplaceAll(_header, L"{0}"s,
			                                     k2app::interfacing::LocalizedResourceWString(
				                                     L"GeneralPage",
				                                     L"Tips/TrackingFreeze/Buttons/WMR"));

		else
			k2app::interfacing::stringReplaceAll(_header, L"{0}"s,
			                                     k2app::interfacing::LocalizedResourceWString(
				                                     L"GeneralPage",
				                                     L"Tips/TrackingFreeze/Buttons/Oculus"));

		FreezeTrackingTeachingTip().Title(_header.c_str());
		FreezeTrackingTeachingTip().Subtitle(
			k2app::interfacing::LocalizedResourceWString(
				L"GeneralPage",
				L"Tips/TrackingFreeze/Footer").c_str());

		FreezeTrackingTeachingTip().TailVisibility(Controls::TeachingTipTailVisibility::Collapsed);
		FreezeTrackingTeachingTip().IsOpen(true);

		k2app::K2Settings.teachingTipShown_Freeze = true;
		k2app::K2Settings.saveSettings();
	}
}


void Amethyst::implementation::GeneralPage::CalibrationSelectView_PaneClosing(
	const Controls::SplitView& sender,
	const Controls::SplitViewPaneClosingEventArgs& args)
{
	args.Cancel(true);
}


void Amethyst::implementation::GeneralPage::CalibrationRunningView_PaneClosing(
	const Controls::SplitView& sender,
	const Controls::SplitViewPaneClosingEventArgs& args)
{
	args.Cancel(true);
}


void Amethyst::implementation::GeneralPage::CalibrationPointsNumberBox_ValueChanged(
	const Controls::NumberBox& sender,
	const Controls::NumberBoxValueChangedEventArgs& args)
{
	// Don't react to dummy changes
	if (!general_tab_setup_finished)return;

	// Attempt automatic fixes
	if (isnan(sender.as<Controls::NumberBox>().Value()))
		sender.as<Controls::NumberBox>().Value(k2app::K2Settings.calibrationPointsNumber);

	k2app::K2Settings.calibrationPointsNumber =
		static_cast<int>(sender.as<Controls::NumberBox>().Value());
	k2app::K2Settings.saveSettings(); // Save it

	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);
}

void Amethyst::implementation::GeneralPage::ReRegisterButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	if (exists(k2app::interfacing::GetProgramLocation().parent_path() / "K2CrashHandler" / "K2CrashHandler.exe"))
	{
		std::thread([]
		{
			ShellExecute(nullptr, L"open",
			             (k2app::interfacing::GetProgramLocation().parent_path() /
				             L"K2CrashHandler" / L"K2CrashHandler.exe ").wstring().c_str(),
			             nullptr, nullptr, SW_SHOWDEFAULT);
		}).detach();
	}
	else
	{
		LOG(WARNING) << "Crash handler exe (./K2CrashHandler/K2CrashHandler.exe) not found!";

		SetErrorFlyoutText().Text(
			k2app::interfacing::LocalizedResourceWString(L"SettingsPage", L"ReRegister/Error/NotFound"));

		Controls::Primitives::FlyoutShowOptions _opt;
		_opt.Placement(Controls::Primitives::FlyoutPlacementMode::RightEdgeAlignedBottom);
		SetErrorFlyout().ShowAt(ReRegisterButton(), _opt);
	}
}


void Amethyst::implementation::GeneralPage::DismissSetErrorButton_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	SetErrorFlyout().Hide();
}


void Amethyst::implementation::GeneralPage::ToggleTrackersTeachingTip_Closed(
	const Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);

	CalibrationTeachingTip().TailVisibility(Controls::TeachingTipTailVisibility::Collapsed);
	CalibrationTeachingTip().IsOpen(true);
}


void Amethyst::implementation::GeneralPage::CalibrationTeachingTip_Closed(
	const Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);

	StatusTeachingTip().TailVisibility(Controls::TeachingTipTailVisibility::Collapsed);
	StatusTeachingTip().IsOpen(true);
}


Windows::Foundation::IAsyncAction Amethyst::implementation::GeneralPage::StatusTeachingTip_Closed(
	const Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);

	// Wait a bit
	{
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(200);
		co_await ui_thread;
	}

	// Reset the next page layout (if ever changed)
	if (k2app::shared::settings::pageMainScrollViewer)
		k2app::shared::settings::pageMainScrollViewer->ScrollToVerticalOffset(0);

	// Navigate to the settings page
	k2app::shared::main::mainNavigationView->SelectedItem(
		k2app::shared::main::mainNavigationView->MenuItems().GetAt(1));
	k2app::shared::main::NavView_Navigate(L"settings", Media::Animation::EntranceNavigationTransitionInfo());

	// Wait a bit
	{
		apartment_context ui_thread;
		co_await resume_background();
		Sleep(500);
		co_await ui_thread;
	}

	// Show the next tip
	k2app::shared::teaching_tips::settings::manageTrackersTeachingTip->TailVisibility(
		Controls::TeachingTipTailVisibility::Collapsed);
	k2app::shared::teaching_tips::settings::manageTrackersTeachingTip->IsOpen(true);
}


void Amethyst::implementation::GeneralPage::ToggleButtonFlyout_Opening(
	const Windows::Foundation::IInspectable& sender, const Windows::Foundation::IInspectable& e)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Show);
}


void Amethyst::implementation::GeneralPage::ToggleButtonFlyout_Closing(
	const Controls::Primitives::FlyoutBase& sender,
	const Controls::Primitives::FlyoutBaseClosingEventArgs& args)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Hide);
}


void Amethyst::implementation::GeneralPage::ToggleTrackersTeachingTip_ActionButtonClick(
	const Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);

	// Close the current tip
	ToggleTrackersTeachingTip().IsOpen(false);

	// Show the previous one
	k2app::shared::teaching_tips::main::initializerTeachingTip->IsOpen(true);
}


void Amethyst::implementation::GeneralPage::CalibrationTeachingTip_ActionButtonClick(
	const Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);

	// Close the current tip
	CalibrationTeachingTip().IsOpen(false);

	// Show the previous one
	ToggleTrackersTeachingTip().TailVisibility(
		Controls::TeachingTipTailVisibility::Collapsed);
	ToggleTrackersTeachingTip().IsOpen(true);
}


void Amethyst::implementation::GeneralPage::StatusTeachingTip_ActionButtonClick(
	const Controls::TeachingTip& sender, const Windows::Foundation::IInspectable& args)
{
	// Play a sound
	playAppSound(k2app::interfacing::sounds::AppSounds::Invoke);

	// Close the current tip
	StatusTeachingTip().IsOpen(false);

	// Show the previous one
	CalibrationTeachingTip().TailVisibility(
		Controls::TeachingTipTailVisibility::Collapsed);
	CalibrationTeachingTip().IsOpen(true);
}


void Amethyst::implementation::GeneralPage::FreezeOnlyLowerToggle_Click(
	const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e)
{
	k2app::K2Settings.freezeLowerOnly = FreezeOnlyLowerToggle().IsChecked();
	k2app::K2Settings.saveSettings();

	// Play a sound
	playAppSound(FreezeOnlyLowerToggle().IsChecked()
		             ? k2app::interfacing::sounds::AppSounds::ToggleOn
		             : k2app::interfacing::sounds::AppSounds::ToggleOff);
}
