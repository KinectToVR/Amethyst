// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Numerics;
using System.Threading;
using System.Threading.Tasks;
using Windows.Media.Core;
using Windows.System;
using Windows.UI.ViewManagement;
using Amethyst.Classes;
using Amethyst.MVVM;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using AmethystSupport;
using Microsoft.UI;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Media.Animation;
using Microsoft.UI.Xaml.Shapes;
using static Amethyst.Classes.Shared.TeachingTips;
using Path = System.IO.Path;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Pages;

/// <summary>
///     An empty page that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class General : Page, INotifyPropertyChanged
{
    private static string _calibratingDeviceGuid = "";
    private bool _autoCalibration_StillPending;

    private bool _calibrationPending;
    private bool _generalPageLoadedOnce;
    private bool _isCurrentWindowActiveBackup;
    private bool _offsetsPageNavigated;

    private int _previousOffsetPageIndex;
    private bool _showSkeletonPrevious = true;

    private bool _skeletonDrawingCanvassLoadedOnce;

    public General()
    {
        InitializeComponent();

        Logger.Info($"Constructing page: '{GetType().FullName}'...");

        Shared.General.ToggleTrackersButton = ToggleTrackersButton;
        Shared.General.SkeletonToggleButton = SkeletonToggleButton;
        Shared.General.ForceRenderCheckBox = ForceRenderCheckBox;
        Shared.General.OffsetsButton = OffsetsButton;
        Shared.General.CalibrationButton = CalibrationButton;
        Shared.General.ServiceSettingsButton = ServiceSettingsButton;
        Shared.General.DeviceNameLabel = SelectedDeviceNameLabel;
        Shared.General.DeviceStatusLabel = TrackingDeviceStatusLabel;
        Shared.General.ErrorWhatText = ErrorWhatText;
        Shared.General.TrackingDeviceErrorLabel = TrackingDeviceErrorLabel;
        Shared.General.ServerStatusLabel = ServerStatusLabel;
        Shared.General.ServerErrorLabel = ServerErrorLabel;
        Shared.General.ServerErrorWhatText = ServerErrorWhatText;
        Shared.General.ForceRenderText = ForceRenderText;
        Shared.General.ErrorButtonsGrid = ErrorButtonsGrid;
        Shared.General.ErrorWhatGrid = ErrorWhatGrid;
        Shared.General.ServerErrorWhatGrid = ServerErrorWhatGrid;
        Shared.General.ToggleFreezeButton = ToggleFreezeButton;
        Shared.General.AdditionalDeviceErrorsHyperlink = AdditionalDeviceErrorsHyperlink;

        GeneralPage.ToggleTrackersTeachingTip = ToggleTrackersTeachingTip;
        GeneralPage.StatusTeachingTip = StatusTeachingTip;

        Logger.Info($"Registering devices MVVM for page: '{GetType().FullName}'...");
        TrackingDeviceTreeView.ItemsSource = TrackingDevices.TrackingDevicesList.Values;

        Logger.Info($"Setting graphical resources for: '{CalibrationPreviewMediaElement.GetType().FullName}'...");
        CalibrationPreviewMediaElement.Source = MediaSource.CreateFromUri(
            new Uri(Path.Join(Interfacing.GetProgramLocation().DirectoryName, "Assets", "CalibrationDirections.mp4")));
        CalibrationPreviewMediaElement.MediaPlayer.CommandManager.IsEnabled = false;

        Logger.Info("Registering a detached binary semaphore " +
                    $"reload handler for '{GetType().FullName}'...");

        Task.Run(() =>
        {
            Shared.Events.ReloadGeneralPageEvent =
                new ManualResetEvent(false);

            while (true)
            {
                // Wait for a reload signal (blocking)
                Shared.Events.ReloadGeneralPageEvent.WaitOne();

                // Reload & restart the waiting loop
                if (_generalPageLoadedOnce && Interfacing.CurrentPageTag == "general")
                    Shared.Main.DispatcherQueue.TryEnqueue(
                        async () => { await Page_LoadedHandler(); });

                // Reset the event
                Shared.Events.ReloadGeneralPageEvent.Reset();
            }
        });
    }

    private IEnumerable<AppTracker> EnabledTrackers =>
        AppData.Settings.TrackersVector.Where(x => x.IsActive);

    private List<Line> BoneLines { get; } = new(24);
    private List<Ellipse> JointPoints { get; } = new(60);

    private string ServiceSettingsText => string.Format(Interfacing.LocalizedJsonString(
        "/SettingsPage/Buttons/ServiceSettings"), TrackingDevices.CurrentServiceEndpoint.Name);

    private string DeviceSettingsText => string.Format(Interfacing.LocalizedJsonString(
        "/GeneralPage/Buttons/DeviceSettings"), TrackingDevices.BaseTrackingDevice.Name);

    private string AutoStartTipText => string.Format(Interfacing.LocalizedJsonString(
        "/NUX/Tip2/Content"), TrackingDevices.CurrentServiceEndpoint.Name);

    private string ServiceStatusLabel => string.Format(Interfacing.LocalizedJsonString(
        "/GeneralPage/Captions/DriverStatus/Label"), TrackingDevices.CurrentServiceEndpoint.Name);

    private bool AllowCalibration => Interfacing.AppTrackersInitialized && ServiceStatusOk;

    private bool ServiceStatusError => TrackingDevices.CurrentServiceEndpoint.StatusError;

    private bool ServiceStatusOk => TrackingDevices.CurrentServiceEndpoint.StatusOk;

    private string[] ServiceStatusText
    {
        get
        {
            var message = StringUtils.SplitStatusString(TrackingDevices.CurrentServiceEndpoint.ServiceStatusString);
            return message is null || message.Length < 3
                ? new[] { "The status message was broken!", "E_FIX_YOUR_SHIT", "AAAAA" }
                : message; // If everything is all right this time
        }
    }

    public event PropertyChangedEventHandler PropertyChanged;

    private async void Page_Loaded(object sender, RoutedEventArgs e)
    {
        Logger.Info($"Re/Loading page: '{GetType().FullName}'...");
        Interfacing.CurrentAppState = "general";

        // Execute the handler
        await Page_LoadedHandler();

        // Mark as loaded
        _generalPageLoadedOnce = true;
    }

    private async Task Page_LoadedHandler()
    {
        // Start the main loop since we're done with basic setup
        if (!_generalPageLoadedOnce)
        {
            Logger.Info("Basic setup done! Starting the main loop now...");
            Shared.Events.SemSignalStartMain.Release();

            // Try auto-spawning trackers if stated so
            if (Interfacing.IsServiceEndpointPresent && // If the driver's ok
                AppData.Settings.AutoSpawnEnabledJoints) // If autospawn
            {
                if (await Interfacing.SpawnEnabledTrackers())
                {
                    ToggleTrackersButton.IsChecked = true; // Mark as spawned
                }

                // Cry about it
                else
                {
                    Interfacing.ServiceEndpointFailure = true; // WAAAAAAA
                    Interfacing.ServiceEndpointSetup(); // Refresh
                    Interfacing.ShowToast(
                        Interfacing.LocalizedJsonString("/SharedStrings/Toasts/AutoSpawnFailed/Title"),
                        Interfacing.LocalizedJsonString("/SharedStrings/Toasts/AutoSpawnFailed"),
                        true); // High priority - it's probably a server failure

                    Interfacing.ShowServiceToast(
                        Interfacing.LocalizedJsonString("/SharedStrings/Toasts/AutoSpawnFailed/Title"),
                        Interfacing.LocalizedJsonString("/SharedStrings/Toasts/AutoSpawnFailed"));
                }
            }
        }

        // Update the internal version
        VersionLabel.Text = $"v{AppData.VersionString.Display}";

        // Update things
        Interfacing.UpdateServerStatus();
        TrackingDevices.UpdateTrackingDevicesInterface();

        // Reload offset values
        Logger.Info($"Force refreshing offsets MVVM for page: '{GetType().FullName}'...");
        AppData.Settings.TrackersVector.ToList().ForEach(x => x.OnPropertyChanged());

        // Reload tracking devices
        Logger.Info($"Force refreshing devices MVVM for page: '{GetType().FullName}'...");
        TrackingDevices.TrackingDevicesList.Values.ToList().ForEach(x => x.OnPropertyChanged());

        // Notify of the setup's end
        OnPropertyChanged(); // Just everything
        Shared.General.GeneralTabSetupFinished = true;
    }

    private void NoCalibrationTeachingTip_Closed(TeachingTip sender, TeachingTipClosedEventArgs args)
    {
        Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = false;
    }

    private void CancelPaneClosing(SplitView sender, SplitViewPaneClosingEventArgs args)
    {
        args.Cancel = true;
    }

    private void DiscardOffsetsButton_Click(object sender, RoutedEventArgs e)
    {
        // Discard backend offsets' values by re-reading them from settings
        lock (Interfacing.UpdateLock)
        {
            AppData.Settings.ReadSettings();
            AppData.Settings.CheckSettings();
        }

        // Reload offset values
        Logger.Info($"Force refreshing offsets MVVM for page: '{GetType().FullName}'...");
        AppData.Settings.TrackersVector.ToList().ForEach(x => x.OnPropertyChanged());

        // Close the pane now
        OffsetsView.DisplayMode = SplitViewDisplayMode.Overlay;
        OffsetsView.IsPaneOpen = false;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.GoBack);

        AllowNavigation(true);
        Interfacing.CurrentAppState = "general";
    }

    private void SaveOffsetsButton_Click(object sender, RoutedEventArgs e)
    {
        // Close the pane now
        OffsetsView.DisplayMode = SplitViewDisplayMode.Overlay;
        OffsetsView.IsPaneOpen = false;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);

        AllowNavigation(true);
        Interfacing.CurrentAppState = "general";

        // Save backend offsets' values to settings/file
        AppData.Settings.SaveSettings();
    }

    private async void TrackingDeviceTreeView_ItemInvoked(TreeView sender, TreeViewItemInvokedEventArgs args)
    {
        // Don't even care if we're not set up yet
        if (!Shared.General.GeneralTabSetupFinished) return;

        // Block erred device selects
        if (args.InvokedItem is not TrackingDevice node || node.StatusError)
        {
            // Set the correct target
            NoCalibrationTeachingTip.Target =
                (FrameworkElement)TrackingDeviceTreeView.ContainerFromItem(args.InvokedItem);

            // Hide the tail and open the tip
            NoCalibrationTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
            NoCalibrationTeachingTip.PreferredPlacement = TeachingTipPlacementMode.Bottom;

            Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = true;
            NoCalibrationTeachingTip.IsOpen = true;

            await Task.Delay(300);
            return; // Give up
        }

        // If no actual calibration is supported
        if (TrackingDevices.CurrentServiceEndpoint.ControllerInputActions == null &&
            (node.TrackedJoints.All(x => x.Role != TrackedJointType.JointHead) ||
             TrackingDevices.CurrentServiceEndpoint.HeadsetPose == null))
        {
            // Set the correct target
            NoCalibrationTeachingTip.Target =
                (FrameworkElement)TrackingDeviceTreeView.ContainerFromItem(args.InvokedItem);

            // Hide the tail and open the tip
            NoCalibrationTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
            NoCalibrationTeachingTip.PreferredPlacement = TeachingTipPlacementMode.Bottom;

            Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = true;
            NoCalibrationTeachingTip.IsOpen = true;

            await Task.Delay(300);
            return; // Give up
        }

        // Show the calibration choose pane / calibration
        AutoCalibrationPane.Visibility = Visibility.Collapsed;
        ManualCalibrationPane.Visibility = Visibility.Collapsed;

        CalibrationModeSelectView.DisplayMode = SplitViewDisplayMode.Inline;
        CalibrationModeSelectView.IsPaneOpen = true;

        CalibrationRunningView.DisplayMode = SplitViewDisplayMode.Overlay;
        CalibrationRunningView.IsPaneOpen = false;

        AllowNavigation(false);

        // Set the app state
        Interfacing.CurrentAppState = "calibration";

        // Set the calibration device
        _calibratingDeviceGuid = node.Guid;

        _showSkeletonPrevious = AppData.Settings.SkeletonPreviewEnabled; // Back up
        AppData.Settings.SkeletonPreviewEnabled = true; // Change to show
        SetSkeletonVisibility(true); // Change to show

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);

        // If auto-calibration is not supported, proceed straight to manual
        // Test: supports if the device provides a head joint / otherwise not
        if (node.TrackedJoints.Any(x => x.Role == TrackedJointType.JointHead))
        {
            // If manual calibration is not supported, proceed straight to automatic
            if (TrackingDevices.CurrentServiceEndpoint.HeadsetPose == null) ExecuteAutomaticCalibration();
            else return; // Else open the selection pane
        }

        // Still here? the test must have failed then
        Logger.Info($"Device ({node.Name}, {node.Guid}) does not provide a {TrackedJointType.JointHead}" +
                    "and can't be calibrated with automatic calibration! Proceeding to manual now...");

        // Open the pane and start the calibration
        await ExecuteManualCalibration();
    }

    private void AutoCalibrationButton_Click(object sender, RoutedEventArgs e)
    {
        ExecuteAutomaticCalibration();
    }

    private async void ManualCalibrationButton_Click(object sender, RoutedEventArgs e)
    {
        await ExecuteManualCalibration();
    }

    private async void StartAutoCalibrationButton_Click(object sender, RoutedEventArgs e)
    {
        // Setup the calibration image : start
        CalibrationPreviewMediaElement.MediaPlayer.Play();

        // Set the [calibration pending] bool
        _calibrationPending = true;
        _autoCalibration_StillPending = true;

        // Play a nice sound - starting
        AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationStart);

        // Disable the start button and change [cancel]'s text
        StartAutoCalibrationButton.IsEnabled = false;
        CalibrationPointsNumberBox.IsEnabled = false;

        DiscardAutoCalibrationButton.Content =
            Interfacing.LocalizedJsonString("/GeneralPage/Buttons/Abort");

        // Reset the origin
        AppData.Settings.DeviceCalibrationOrigins[_calibratingDeviceGuid] = Vector3.Zero;

        // Setup helper variables
        List<Vector3> hmdPositions = new(), headPositions = new();
        await Task.Delay(1000);

        // Loop over total 3 points (by default)
        for (var point = 0; point < AppData.Settings.CalibrationPointsNumber; point++)
        {
            // Wait for the user to move
            CalibrationInstructionsLabel.Text = CalibrationPointsFormat(
                Interfacing.LocalizedJsonString("/GeneralPage/Calibration/Captions/Move"),
                point + 1, AppData.Settings.CalibrationPointsNumber);

            for (var i = 3; i >= 0; i--)
            {
                if (!_calibrationPending) break; // Check for exiting

                // Update the countdown label
                CalibrationCountdownLabel.Text = i.ToString();

                // Exit if aborted
                if (!_calibrationPending) break;

                // Play a nice sound - tick / move
                if (i > 0) // Don't play the last one!
                    AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationTick);

                await Task.Delay(1000);
                if (!_calibrationPending) break; // Check for exiting
            }

            CalibrationInstructionsLabel.Text = CalibrationPointsFormat(
                Interfacing.LocalizedJsonString("/GeneralPage/Calibration/Captions/Stand"),
                point + 1, AppData.Settings.CalibrationPointsNumber);

            for (var i = 3; i >= 0; i--)
            {
                if (!_calibrationPending) break; // Check for exiting

                // Update the countdown label
                CalibrationCountdownLabel.Text = i.ToString();

                switch (i)
                {
                    // Play a nice sound - tick / stand (w/o the last one!)
                    case > 0:
                        AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationTick);
                        break;

                    // Capture user's position at t_end-1, update the label text
                    case 0:
                        hmdPositions.Add(Interfacing.Plugins.GetHmdPose.Position);
                        headPositions.Add(Interfacing.DeviceHookJointPosition.ValueOr(_calibratingDeviceGuid).Position);

                        CalibrationInstructionsLabel.Text = Interfacing.LocalizedJsonString(
                            "/GeneralPage/Calibration/Captions/Captured");
                        break;
                }

                await Task.Delay(1000);
                if (!_calibrationPending) break; // Check for exiting
            }

            // Exit if aborted
            if (!_calibrationPending) break;

            // Play a nice sound - tick / captured
            AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationPointCaptured);

            await Task.Delay(1000);
            if (!_calibrationPending) break; // Check for exiting
        }

        // Do the actual calibration after capturing points
        if (_calibrationPending)
        {
            // Calibrate (AmethystSupport/CLI)
            var (translation, rotation) = Support.SVD(
                headPositions.Select(x => x.Projected()).ToArray(),
                hmdPositions.Select(x => x.Projected()).ToArray()).T();

            Logger.Info("Automatic calibration concluded!\n" +
                        $"Head positions: {headPositions.ToArray()}\n" +
                        $"HMD positions: {hmdPositions.ToArray()}\n" +
                        $"Recovered t: {translation}\n" +
                        $"Recovered R: {rotation}");

            AppData.Settings.DeviceCalibrationRotationMatrices[_calibratingDeviceGuid] = rotation;
            AppData.Settings.DeviceCalibrationTranslationVectors[_calibratingDeviceGuid] = translation;
            AppData.Settings.DeviceCalibrationOrigins[_calibratingDeviceGuid] = Vector3.Zero;
        }

        // Reset by re-reading the settings if aborted
        if (!_calibrationPending)
        {
            lock (Interfacing.UpdateLock)
            {
                AppData.Settings.ReadSettings();
                AppData.Settings.CheckSettings();
            }

            AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationAborted);
        }
        // Else save I guess
        else
        {
            AppData.Settings.SaveSettings();
            AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationComplete);
        }

        // Notify that we're finished
        CalibrationCountdownLabel.Text = "~";
        CalibrationInstructionsLabel.Text =
            Interfacing.LocalizedJsonString(_calibrationPending
                ? "/GeneralPage/Calibration/Captions/Done"
                : "/GeneralPage/Calibration/Captions/Aborted");

        await Task.Delay(2200);

        // Exit the pane
        CalibrationDeviceSelectView.DisplayMode = SplitViewDisplayMode.Overlay;
        CalibrationDeviceSelectView.IsPaneOpen = false;

        CalibrationModeSelectView.DisplayMode = SplitViewDisplayMode.Overlay;
        CalibrationModeSelectView.IsPaneOpen = false;

        CalibrationRunningView.DisplayMode = SplitViewDisplayMode.Overlay;
        CalibrationRunningView.IsPaneOpen = false;

        AllowNavigation(true);
        Interfacing.CurrentAppState = "general";

        NoSkeletonTextNotice.Text = Interfacing.LocalizedJsonString("/GeneralPage/Captions/Preview/NoSkeletonText");

        _calibrationPending = false; // We're finished
        _autoCalibration_StillPending = false;

        AppData.Settings.SkeletonPreviewEnabled = _showSkeletonPrevious; // Change to whatever
        SetSkeletonVisibility(_showSkeletonPrevious); // Change to whatever
    }

    private void CalibrationPointsNumberBox_ValueChanged(NumberBox sender, NumberBoxValueChangedEventArgs args)
    {
        // Don't even care if we're not set up yet
        if (!Shared.General.GeneralTabSetupFinished) return;

        // Attempt automatic fixes
        if (double.IsNaN(CalibrationPointsNumberBox.Value))
            CalibrationPointsNumberBox.Value = AppData.Settings.CalibrationPointsNumber;

        AppData.Settings.CalibrationPointsNumber = (uint)CalibrationPointsNumberBox.Value;
        AppData.Settings.SaveSettings(); // Save it

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
    }

    private void DiscardCalibrationButton_Click(object sender, RoutedEventArgs e)
    {
        // Just exit
        if (!_calibrationPending && !_autoCalibration_StillPending)
        {
            CalibrationDeviceSelectView.DisplayMode = SplitViewDisplayMode.Overlay;
            CalibrationDeviceSelectView.IsPaneOpen = false;

            CalibrationModeSelectView.DisplayMode = SplitViewDisplayMode.Overlay;
            CalibrationModeSelectView.IsPaneOpen = false;

            CalibrationRunningView.DisplayMode = SplitViewDisplayMode.Overlay;
            CalibrationRunningView.IsPaneOpen = false;

            AllowNavigation(true);

            // Play a sound
            AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
            Interfacing.CurrentAppState = "general";

            NoSkeletonTextNotice.Text = Interfacing.LocalizedJsonString("/GeneralPage/Captions/Preview/NoSkeletonText");

            AppData.Settings.SkeletonPreviewEnabled = _showSkeletonPrevious; // Change to whatever
            SetSkeletonVisibility(_showSkeletonPrevious); // Change to whatever
        }
        // Begin abort
        else
        {
            _calibrationPending = false;
        }
    }

    private void CalibrationTeachingTip_ActionButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Close the current tip
        CalibrationTeachingTip.IsOpen = false;

        // Show the previous one
        ToggleTrackersTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        ToggleTrackersTeachingTip.IsOpen = true;
    }

    private void CalibrationTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        StatusTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        StatusTeachingTip.IsOpen = true;
    }

    private async void CalibrationButton_Click(object sender, RoutedEventArgs e)
    {
        // If no overrides
        if (AppData.Settings.OverrideDevicesGuidMap.Count < 1)
        {
            // Get our current device
            var trackingDevice = TrackingDevices.BaseTrackingDevice;

            // If the status isn't OK, cry about it
            if (trackingDevice.StatusError)
            {
                // Set the correct target
                NoCalibrationTeachingTip.Target = DeviceTitleContainer;

                // Hide the tail and open the tip
                NoCalibrationTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
                NoCalibrationTeachingTip.PreferredPlacement = TeachingTipPlacementMode.Top;

                Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = true;
                NoCalibrationTeachingTip.IsOpen = true;

                await Task.Delay(300);
                return; // Give up
            }

            // Show the calibration choose pane / calibration
            AutoCalibrationPane.Visibility = Visibility.Collapsed;
            ManualCalibrationPane.Visibility = Visibility.Collapsed;

            CalibrationDeviceSelectView.DisplayMode = SplitViewDisplayMode.Overlay;
            CalibrationDeviceSelectView.IsPaneOpen = false;

            CalibrationModeSelectView.DisplayMode = SplitViewDisplayMode.Inline;
            CalibrationModeSelectView.IsPaneOpen = true;

            CalibrationRunningView.DisplayMode = SplitViewDisplayMode.Overlay;
            CalibrationRunningView.IsPaneOpen = false;

            AllowNavigation(false);

            // Set the app state
            Interfacing.CurrentAppState = "calibration";

            // Set the calibration device
            _calibratingDeviceGuid = AppData.Settings.TrackingDeviceGuid;

            _showSkeletonPrevious = AppData.Settings.SkeletonPreviewEnabled; // Back up
            AppData.Settings.SkeletonPreviewEnabled = true; // Change to show
            SetSkeletonVisibility(true); // Change to show

            // Play a sound
            AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);

            // If auto-calibration is not supported, proceed straight to manual
            // Test: supports if the device provides a head joint / otherwise not
            if (trackingDevice.TrackedJoints.Any(x => x.Role == TrackedJointType.JointHead))
            {
                // If manual calibration is not supported, proceed straight to automatic
                if (TrackingDevices.CurrentServiceEndpoint.HeadsetPose == null) ExecuteAutomaticCalibration();
                else return; // Else open the selection pane
            }

            // Still here? the test must have failed then
            Logger.Info($"Device ({trackingDevice.Name}, {trackingDevice.Guid}) " +
                        $"does not provide a {TrackedJointType.JointHead}" +
                        "and can't be calibrated with automatic calibration! Proceeding to manual now...");

            // Open the pane and start the calibration
            await ExecuteManualCalibration();
        }
        else
        {
            // Show the device selector pane
            AutoCalibrationPane.Visibility = Visibility.Collapsed;
            ManualCalibrationPane.Visibility = Visibility.Collapsed;


            // If all used devices are erred: cry about it
            if (TrackingDevices.TrackingDevicesList.Values
                .Where(plugin => plugin.IsBase || plugin.IsOverride)
                .All(device => device.StatusError))
            {
                // Set the correct target
                NoCalibrationTeachingTip.Target = DeviceTitleContainer;
                NoCalibrationTeachingTip.PreferredPlacement = TeachingTipPlacementMode.Top;

                // Hide the tail and open the tip
                NoCalibrationTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;

                Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = true;
                NoCalibrationTeachingTip.IsOpen = true;

                await Task.Delay(300);
                return; // Give up
            }

            // Else proceed to calibration device pick
            CalibrationDeviceSelectView.DisplayMode = SplitViewDisplayMode.Inline;
            CalibrationDeviceSelectView.IsPaneOpen = true;

            CalibrationModeSelectView.DisplayMode = SplitViewDisplayMode.Overlay;
            CalibrationModeSelectView.IsPaneOpen = false;

            CalibrationRunningView.DisplayMode = SplitViewDisplayMode.Overlay;
            CalibrationRunningView.IsPaneOpen = false;

            AllowNavigation(false);
            Interfacing.CurrentAppState = "calibration";

            _showSkeletonPrevious = AppData.Settings.SkeletonPreviewEnabled; // Back up
            AppData.Settings.SkeletonPreviewEnabled = true; // Change to show
            SetSkeletonVisibility(true); // Change to show

            // Play a sound
            AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
        }
    }

    private void ToggleFreezeButton_Click(object sender, RoutedEventArgs e)
    {
        Interfacing.IsTrackingFrozen = !Interfacing.IsTrackingFrozen;

        ToggleFreezeButton.IsChecked = Interfacing.IsTrackingFrozen;
        ToggleFreezeButton.Content = Interfacing.LocalizedJsonString(
            Interfacing.IsTrackingFrozen
                ? "/GeneralPage/Buttons/Skeleton/Unfreeze"
                : "/GeneralPage/Buttons/Skeleton/Freeze");

        // Play a sound
        AppSounds.PlayAppSound(Interfacing.IsTrackingFrozen
            ? AppSounds.AppSoundType.ToggleOff
            : AppSounds.AppSoundType.ToggleOn);

        // Optionally show the binding teaching tip
        if (AppData.Settings.TeachingTipShownFreeze || Interfacing.CurrentPageTag != "general" ||
            string.IsNullOrEmpty(TrackingDevices.CurrentServiceEndpoint
                .ControllerInputActions?.TrackingFreezeActionTitleString) ||
            string.IsNullOrEmpty(TrackingDevices.CurrentServiceEndpoint
                .ControllerInputActions?.TrackingFreezeActionContentString)) return;

        FreezeTrackingTeachingTip.Title =
            TrackingDevices.CurrentServiceEndpoint.ControllerInputActions?.TrackingFreezeActionTitleString;
        FreezeTrackingTeachingTip.Subtitle =
            TrackingDevices.CurrentServiceEndpoint.ControllerInputActions?.TrackingFreezeActionContentString;
        FreezeTrackingTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;

        Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = true;
        FreezeTrackingTeachingTip.IsOpen = true;

        AppData.Settings.TeachingTipShownFreeze = true;
        AppData.Settings.SaveSettings();
    }

    private void OffsetsButton_Click(object sender, RoutedEventArgs e)
    {
        // Push saved offsets' by reading them from settings
        lock (Interfacing.UpdateLock)
        {
            AppData.Settings.SaveSettings();
            AppData.Settings.ReadSettings();
            AppData.Settings.CheckSettings();
        }

        // Reload offset values
        Logger.Info($"Force refreshing offsets MVVM for page: '{GetType().FullName}'...");
        AppData.Settings.TrackersVector.ToList().ForEach(x => x.OnPropertyChanged());

        // Open the pane now
        OffsetsView.DisplayMode = SplitViewDisplayMode.Inline;
        OffsetsView.IsPaneOpen = true;
        AllowNavigation(false);

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
        Interfacing.CurrentAppState = "offsets";
    }

    private void FreezeOnlyLowerToggle_Click(object sender, RoutedEventArgs e)
    {
        AppData.Settings.FreezeLowerBodyOnly = FreezeOnlyLowerToggle.IsChecked;
        AppData.Settings.SaveSettings();

        // Play a sound
        AppSounds.PlayAppSound(FreezeOnlyLowerToggle.IsChecked
            ? AppSounds.AppSoundType.ToggleOn
            : AppSounds.AppSoundType.ToggleOff);
    }

    private async void ToggleTrackersButton_Checked(object sender, RoutedEventArgs e)
    {
        // Don't check if setup's finished since we're gonna emulate a click rather than change the state only
        ToggleTrackersButton.Content = Interfacing.LocalizedJsonString(
            "/GeneralPage/Buttons/TrackersToggle/Disconnect");

        // Optionally spawn trackers
        if (!Interfacing.AppTrackersSpawned)
            if (!await Interfacing.SpawnEnabledTrackers()) // Mark as spawned
            {
                Interfacing.ServiceEndpointFailure = true; // WAAAAAAA
                Interfacing.ServiceEndpointSetup(); // Refresh
                Interfacing.ShowToast(
                    Interfacing.LocalizedJsonString("/SharedStrings/Toasts/AutoSpawnFailed/Title"),
                    Interfacing.LocalizedJsonString("/SharedStrings/Toasts/AutoSpawnFailed"),
                    true); // High priority - it's probably a server failure

                Interfacing.ShowServiceToast(
                    Interfacing.LocalizedJsonString("/SharedStrings/Toasts/AutoSpawnFailed/Title"),
                    Interfacing.LocalizedJsonString("/SharedStrings/Toasts/AutoSpawnFailed"));
            }

        // Give up if failed
        if (!Interfacing.ServiceEndpointFailure)
        {
            // Mark trackers as active
            Interfacing.AppTrackersInitialized = true;

            // Request a check for already-added trackers
            Interfacing.AlreadyAddedTrackersScanRequested = true;

            // Play a sound
            AppSounds.PlayAppSound(AppSounds.AppSoundType.TrackersConnected);
        }

        OnPropertyChanged(); // Refresh the UI
        Interfacing.UpdateServerStatus();
        TrackingDevices.UpdateTrackingDevicesInterface();
    }

    private void ToggleTrackersButton_Unchecked(object sender, RoutedEventArgs e)
    {
        // Don't check if setup's finished since we're gonna emulate a click rather than change the state only
        ToggleTrackersButton.Content = Interfacing.LocalizedJsonString(
            "/GeneralPage/Buttons/TrackersToggle/Reconnect");

        // Mark trackers as inactive
        Interfacing.AppTrackersInitialized = false;

        // Request a check for already-added trackers
        Interfacing.AlreadyAddedTrackersScanRequested = true;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.TrackersDisconnected);

        OnPropertyChanged(); // Refresh the UI
        Interfacing.UpdateServerStatus();
        TrackingDevices.UpdateTrackingDevicesInterface();
    }

    private void ToggleTrackersTeachingTip_ActionButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Close the current tip
        ToggleTrackersTeachingTip.IsOpen = false;

        // Show the previous one
        MainPage.InitializerTeachingTip.IsOpen = true;
    }

    private void ToggleTrackersTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        CalibrationTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        CalibrationTeachingTip.IsOpen = true;
    }

    private void StatusTeachingTip_ActionButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Close the current tip
        StatusTeachingTip.IsOpen = false;

        // Show the previous one
        CalibrationTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        CalibrationTeachingTip.IsOpen = true;
    }

    private async void StatusTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
        await Task.Delay(200);

        // Reset the next page layout (if ever changed)
        Shared.Settings.PageMainScrollViewer?.ScrollToVerticalOffset(0);

        // Navigate to the settings page
        Shared.Main.MainNavigationView.SelectedItem =
            Shared.Main.MainNavigationView.MenuItems[1];

        Shared.Main.NavigateToPage("settings",
            new EntranceNavigationTransitionInfo());

        await Task.Delay(500);

        // Show the next tip
        SettingsPage.ManageTrackersTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        SettingsPage.ManageTrackersTeachingTip.IsOpen = true;
    }

    private void AdditionalDeviceErrorsHyperlink_Tapped(object sender, TappedRoutedEventArgs e)
    {
        Shared.General.AdditionalDeviceErrorsHyperlinkTappedEvent.Start();
    }

    private async void ServiceSettingsButton_Click(object sender, RoutedEventArgs e)
    {
        // Go to the bottom of the settings page to view service endpoint settings

        // Navigate to the settings page
        Shared.Main.MainNavigationView.SelectedItem =
            Shared.Main.MainNavigationView.MenuItems[1];

        Shared.Main.NavigateToPage("settings",
            new EntranceNavigationTransitionInfo());

        await Task.Delay(750);

        Shared.Settings.PageMainScrollViewer?.UpdateLayout();
        Shared.Settings.PageMainScrollViewer?.ChangeView(null,
            Shared.Settings.PageMainScrollViewer?.ExtentHeight ?? 0, null);
    }

    private async void DeviceSettingsButton_Click(object sender, RoutedEventArgs e)
    {
        // Go to the devices page to view device settings

        // Navigate to the settings page
        Shared.Main.MainNavigationView.SelectedItem =
            Shared.Main.MainNavigationView.MenuItems[2];

        Shared.Main.NavigateToPage("devices",
            new EntranceNavigationTransitionInfo());

        await Task.Delay(500);

        // Should already be init-ed after 500ms, but check anyway
        if (Shared.Devices.DevicesTreeView is null) return;
        var devicesListIndex = TrackingDevices.TrackingDevicesList.Keys.ToList()
            .IndexOf(AppData.Settings.TrackingDeviceGuid);
        var devicesListNode = Shared.Devices.DevicesTreeView.RootNodes[devicesListIndex];

        var skipAnimation = Shared.Devices.DevicesTreeView.SelectedNode == devicesListNode;
        Shared.Devices.DevicesTreeView.SelectedNode = devicesListNode;

        AppData.Settings.PreviousSelectedTrackingDeviceGuid = AppData.Settings.TrackingDeviceGuid;
        AppData.Settings.SelectedTrackingDeviceGuid = AppData.Settings.TrackingDeviceGuid;

        await Shared.Devices.ReloadSelectedDevice(skipAnimation);
        Shared.Events.ReloadDevicesPageEvent?.Set(); // Full reload
    }

    private void SkeletonDrawingCanvas_Loaded(object sender, RoutedEventArgs e)
    {
        // Don't handle reloads
        if (_skeletonDrawingCanvassLoadedOnce) return;
        SkeletonDrawingCanvas.Children.Clear();

        // Manual init is necessary (XAML something something)
        for (var i = 0; i < BoneLines.Capacity; i++)
        {
            BoneLines.Add(new Line());
            SkeletonDrawingCanvas.Children.Add(BoneLines[i]);
        }

        // Manual init is necessary (XAML something something)
        for (var i = 0; i < JointPoints.Capacity; i++)
        {
            JointPoints.Add(new Ellipse());
            SkeletonDrawingCanvas.Children.Add(JointPoints[i]);
        }

        var timer = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(30) };
        timer.Tick += (_, _) =>
        {
            if (Interfacing.IsExitingNow) return;
            var windowActive = Interfacing.IsCurrentWindowActive();

            if (_isCurrentWindowActiveBackup != windowActive &&
                Shared.Main.AppTitleLabel is not null)
            {
                Shared.Main.AppTitleLabel.Opacity = windowActive ? 1.0 : 0.5;
                _isCurrentWindowActiveBackup = windowActive;
            }

            // If we've disabled the preview
            if (!AppData.Settings.SkeletonPreviewEnabled)
            {
                // Hide the UI, only show that viewing is disabled
                (SkeletonDrawingCanvas.Opacity, TrackingStateLabelsPanel.Opacity) = (0, 0);
                (NoSkeletonNotice.Opacity, OutOfFocusNotice.Opacity) = (0, 0);
                (DashboardClosedNotice.Opacity, PreviewDisabledNotice.Opacity) = (0, 1);
                return; // Nothing more to do anyway
            }

            // If the preview isn't forced
            if (!AppData.Settings.ForceSkeletonPreview)
            {
                // If the dashboard's closed
                if (!TrackingDevices.CurrentServiceEndpoint.IsAmethystVisible)
                {
                    // Hide the UI, only show that viewing is disabled
                    (SkeletonDrawingCanvas.Opacity, TrackingStateLabelsPanel.Opacity) = (0, 0);
                    (NoSkeletonNotice.Opacity, OutOfFocusNotice.Opacity) = (0, 0);
                    (DashboardClosedNotice.Opacity, PreviewDisabledNotice.Opacity) = (1, 0);
                    return; // Nothing more to do anyway
                }

                // If we're out of focus (skip if we're gonna do a VROverlay)
                if (!windowActive)
                {
                    // Hide the UI, only show that viewing is disabled
                    (SkeletonDrawingCanvas.Opacity, TrackingStateLabelsPanel.Opacity) = (0, 0);
                    (NoSkeletonNotice.Opacity, OutOfFocusNotice.Opacity) = (0, 1);
                    (DashboardClosedNotice.Opacity, PreviewDisabledNotice.Opacity) = (0, 0);
                    return; // Nothing more to do anyway
                }
            }

            // Else hide the notices
            (DashboardClosedNotice.Opacity, PreviewDisabledNotice.Opacity,
                OutOfFocusNotice.Opacity) = (0, 0, 0); // Only these for now

            var trackingDevice = TrackingDevices.BaseTrackingDevice;
            var joints = trackingDevice.TrackedJoints;

            // Okay to do this here => the preview is forced on calibration
            StartAutoCalibrationButton.IsEnabled =
                trackingDevice.IsSkeletonTracked
                && !_calibrationPending && !_autoCalibration_StillPending;

            if (!trackingDevice.IsSkeletonTracked)
            {
                // Hide the UI, only show that viewing is disabled
                (SkeletonDrawingCanvas.Opacity, TrackingStateLabelsPanel.Opacity) = (0, 0);
                NoSkeletonNotice.Opacity = 1.0; // Say it
                return; // Nothing more to do anyway
            }

            // Show the UI
            (SkeletonDrawingCanvas.Opacity, TrackingStateLabelsPanel.Opacity) = (1, 1);
            NoSkeletonNotice.Opacity = 0.0; // Say it

            // Clear visibilities
            BoneLines.ForEach(x => x.Visibility = Visibility.Collapsed);
            JointPoints.ForEach(x => x.Visibility = Visibility.Collapsed);

            // Draw the skeleton with from-to lines
            // Head
            if (joints.Any(x => x.Role == TrackedJointType.JointNeck))
            {
                // If extended (like kinect v2)
                SkBone(BoneLines[0], JointPoints[0], ref joints,
                    TrackedJointType.JointHead, TrackedJointType.JointNeck);
                SkBone(BoneLines[1], JointPoints[1], ref joints,
                    TrackedJointType.JointNeck, TrackedJointType.JointSpineShoulder);
            }
            else
            {
                // Else skip neck and try with spine-shoulder
                SkBone(BoneLines[0], JointPoints[0], ref joints,
                    TrackedJointType.JointHead, TrackedJointType.JointSpineShoulder);
            }

            // Upper left limb
            SkBone(BoneLines[2], JointPoints[2], ref joints,
                TrackedJointType.JointSpineShoulder, TrackedJointType.JointShoulderLeft);
            SkBone(BoneLines[3], JointPoints[3], ref joints,
                TrackedJointType.JointShoulderLeft, TrackedJointType.JointElbowLeft);
            SkBone(BoneLines[4], JointPoints[4], ref joints,
                TrackedJointType.JointElbowLeft, TrackedJointType.JointWristLeft);
            SkBone(BoneLines[5], JointPoints[5], ref joints,
                TrackedJointType.JointWristLeft, TrackedJointType.JointHandLeft);
            SkBone(BoneLines[6], JointPoints[6], ref joints,
                TrackedJointType.JointHandLeft, TrackedJointType.JointHandTipLeft);
            SkBone(BoneLines[7], JointPoints[7], ref joints,
                TrackedJointType.JointHandLeft, TrackedJointType.JointThumbLeft);

            // Upper right limb
            SkBone(BoneLines[8], JointPoints[8], ref joints,
                TrackedJointType.JointSpineShoulder, TrackedJointType.JointShoulderRight);
            SkBone(BoneLines[9], JointPoints[9], ref joints,
                TrackedJointType.JointShoulderRight, TrackedJointType.JointElbowRight);
            SkBone(BoneLines[10], JointPoints[10], ref joints,
                TrackedJointType.JointElbowRight, TrackedJointType.JointWristRight);
            SkBone(BoneLines[11], JointPoints[11], ref joints,
                TrackedJointType.JointWristRight, TrackedJointType.JointHandRight);
            SkBone(BoneLines[12], JointPoints[12], ref joints,
                TrackedJointType.JointHandRight, TrackedJointType.JointHandTipRight);
            SkBone(BoneLines[13], JointPoints[13], ref joints,
                TrackedJointType.JointHandRight, TrackedJointType.JointThumbRight);

            // Spine
            SkBone(BoneLines[14], JointPoints[14], ref joints,
                TrackedJointType.JointSpineShoulder, TrackedJointType.JointSpineMiddle);
            SkBone(BoneLines[15], JointPoints[15], ref joints,
                TrackedJointType.JointSpineMiddle, TrackedJointType.JointSpineWaist);

            // Lower left limb
            SkBone(BoneLines[16], JointPoints[16], ref joints,
                TrackedJointType.JointSpineWaist, TrackedJointType.JointHipLeft);
            SkBone(BoneLines[17], JointPoints[17], ref joints,
                TrackedJointType.JointHipLeft, TrackedJointType.JointKneeLeft);
            SkBone(BoneLines[18], JointPoints[18], ref joints,
                TrackedJointType.JointKneeLeft, TrackedJointType.JointFootLeft);
            SkBone(BoneLines[19], JointPoints[19], ref joints,
                TrackedJointType.JointFootLeft, TrackedJointType.JointFootTipLeft);

            // Lower right limb
            SkBone(BoneLines[20], JointPoints[20], ref joints,
                TrackedJointType.JointSpineWaist, TrackedJointType.JointHipRight);
            SkBone(BoneLines[21], JointPoints[21], ref joints,
                TrackedJointType.JointHipRight, TrackedJointType.JointKneeRight);
            SkBone(BoneLines[22], JointPoints[22], ref joints,
                TrackedJointType.JointKneeRight, TrackedJointType.JointFootRight);
            SkBone(BoneLines[23], JointPoints[23], ref joints,
                TrackedJointType.JointFootRight, TrackedJointType.JointFootTipRight);

            // Waist
            SkPoint(JointPoints[24], joints.FirstOrDefault(x =>
                    x.Role == TrackedJointType.JointSpineWaist, null),
                AppData.Settings.TrackersVector[0].IsOverriddenPair);

            // Left Foot
            SkPoint(JointPoints[25], joints.FirstOrDefault(x =>
                    x.Role == TrackedJointType.JointFootLeft, null),
                AppData.Settings.TrackersVector[1].IsOverriddenPair);

            // Right Foot
            SkPoint(JointPoints[26], joints.FirstOrDefault(x =>
                    x.Role == TrackedJointType.JointFootRight, null),
                AppData.Settings.TrackersVector[2].IsOverriddenPair);

            // Left Elbow
            SkPoint(JointPoints[27], joints.FirstOrDefault(x =>
                    x.Role == TrackedJointType.JointElbowLeft, null),
                AppData.Settings.TrackersVector[3].IsOverriddenPair);

            // Right Elbow
            SkPoint(JointPoints[28], joints.FirstOrDefault(x =>
                    x.Role == TrackedJointType.JointElbowRight, null),
                AppData.Settings.TrackersVector[4].IsOverriddenPair);

            // Left Knee
            SkPoint(JointPoints[29], joints.FirstOrDefault(x =>
                    x.Role == TrackedJointType.JointKneeLeft, null),
                AppData.Settings.TrackersVector[5].IsOverriddenPair);

            // Right Knee
            SkPoint(JointPoints[30], joints.FirstOrDefault(x =>
                    x.Role == TrackedJointType.JointKneeRight, null),
                AppData.Settings.TrackersVector[6].IsOverriddenPair);

            // The rest
            var jointPointIndex = 31;
            foreach (var trackedJoint in joints.Where(x => x.Role == TrackedJointType.JointManual))
            {
                if (JointPoints.Count <= jointPointIndex) return; // Too much, oof!
                SkPoint(JointPoints[jointPointIndex++], trackedJoint, (false, false));
            }
        };

        timer.Start();
        _skeletonDrawingCanvassLoadedOnce = true;
    }

    private void SkeletonToggleButton_Click(SplitButton sender, SplitButtonClickEventArgs args)
    {
        // Don't even care if we're not set up yet
        if (!Shared.General.GeneralTabSetupFinished) return;

        AppData.Settings.SkeletonPreviewEnabled = SkeletonToggleButton.IsChecked;
        AppData.Settings.SaveSettings();

        SkeletonToggleButton.Content = Interfacing.LocalizedJsonString(
            AppData.Settings.SkeletonPreviewEnabled
                ? "/GeneralPage/Buttons/Skeleton/Hide"
                : "/GeneralPage/Buttons/Skeleton/Show");

        // Play a sound
        AppSounds.PlayAppSound(AppData.Settings.SkeletonPreviewEnabled
            ? AppSounds.AppSoundType.ToggleOn
            : AppSounds.AppSoundType.ToggleOff);

        ForceRenderCheckBox.IsEnabled =
            SkeletonToggleButton.IsChecked;
        ForceRenderText.Opacity =
            SkeletonToggleButton.IsChecked ? 1.0 : 0.5;
    }

    private void ToggleButtonFlyout_Opening(object sender, object e)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void ToggleButtonFlyout_Closing(FlyoutBase sender, FlyoutBaseClosingEventArgs args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }

    private void ForceRenderCheckBox_Checked(object sender, RoutedEventArgs e)
    {
        // Don't even care if we're not set up yet
        if (!Shared.General.GeneralTabSetupFinished) return;

        AppData.Settings.ForceSkeletonPreview = true;
        SetSkeletonForce(AppData.Settings.ForceSkeletonPreview);
        AppData.Settings.SaveSettings();

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.ToggleOn);
    }

    private void ForceRenderCheckBox_Unchecked(object sender, RoutedEventArgs e)
    {
        // Don't even care if we're not set up yet
        if (!Shared.General.GeneralTabSetupFinished) return;

        AppData.Settings.ForceSkeletonPreview = false;
        SetSkeletonForce(AppData.Settings.ForceSkeletonPreview);
        AppData.Settings.SaveSettings();

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.ToggleOff);
    }

    private void DismissSetErrorButton_Click(object sender, RoutedEventArgs e)
    {
        SetErrorFlyout.Hide();
    }

    private void OffsetsValueChanged(NumberBox sender, NumberBoxValueChangedEventArgs args)
    {
        // Don't even care if we're not set up yet
        if (!sender.IsLoaded) return;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Fix and reload the offset value
        if (double.IsNaN(sender.Value))
            sender.Value = 0; // Reset to 0, auto-updates

        // Round the value (floats are kinda bad...)
        sender.Value = sender.GetValue(AttachedString
                .AttachedStringProperty).ToString()![0] switch
            {
                'P' => Math.Round(sender.Value, 2),
                'O' or _ => Math.Round(sender.Value, 0)
            };

        var tracker = AppData.Settings.TrackersVector.Where(
            x => x.Role == (sender.DataContext as AppTracker)!.Role).ToList()[0];

        // Manually overwrite the offset, use the custom property
        switch (sender.GetValue(AttachedString.AttachedStringProperty))
        {
            case "OZ":
                tracker.OrientationOffset.Z = (float)sender.Value;
                break;
            case "OY":
                tracker.OrientationOffset.Y = (float)sender.Value;
                break;
            case "OX":
                tracker.OrientationOffset.X = (float)sender.Value;
                break;
            case "PX":
                tracker.PositionOffset.X = (float)sender.Value;
                break;
            case "PY":
                tracker.PositionOffset.Y = (float)sender.Value;
                break;
            case "PZ":
                tracker.PositionOffset.Z = (float)sender.Value;
                break;
        }
    }

    private void SkPoint(Ellipse ellipse, TrackedJoint joint,
        (bool Position, bool Orientation) overrideStatus)
    {
        // Give up, no such joint
        if (joint is null) return;

        const double matWidthDefault = 700, matHeightDefault = 600;
        double matWidth = SkeletonDrawingCanvas.ActualWidth,
            matHeight = SkeletonDrawingCanvas.ActualHeight;

        // Eventually fix sizes
        if (matWidth < 1) matWidth = matWidthDefault;
        if (matHeight < 1) matHeight = matHeightDefault;

        // Where to scale by 1.0 in perspective
        const double normalDistance = 3;
        const double normalEllipseSize = 12,
            normalEllipseStrokeSize = 2;

        // Compose perspective constants, make it 70%
        var multiply = .7 * (normalDistance /
                             (joint.Position.Z > 0.0 ? joint.Position.Z : 3.0));

        var a = new AcrylicBrush
        {
            TintColor = new UISettings()
                .GetColorValue(UIColorType.Accent)
        };

        ellipse.StrokeThickness = normalEllipseStrokeSize;
        ellipse.Width = normalEllipseSize;
        ellipse.Height = normalEllipseSize;

        if (joint.TrackingState != TrackedJointState.StateTracked)
        {
            ellipse.Stroke = a;
            ellipse.Fill = a;
        }
        else
        {
            ellipse.Stroke = new SolidColorBrush(Colors.White);
            ellipse.Fill = new SolidColorBrush(Colors.White);
        }

        // Change the stroke based on overrides
        ellipse.Stroke = overrideStatus.Position switch
        {
            // Both
            true when overrideStatus.Orientation => new SolidColorBrush(Colors.BlueViolet),
            // Position
            true => new SolidColorBrush(Colors.DarkOliveGreen),
            // Orientation
            false when overrideStatus.Orientation => new SolidColorBrush(Colors.IndianRed),
            // None
            _ => null
        };

        // Select the smaller scale to preserve somewhat uniform skeleton scaling
        double sScaleW = matWidth / matWidthDefault,
            sScaleH = matHeight / matHeightDefault;

        // Move the ellipse to the appropriate point
        ellipse.Margin = new Thickness(
            // Left
            joint.Position.X * 300.0 *
            Math.Min(sScaleW, sScaleH) * multiply +
            matWidth / 2.0 - (normalEllipseSize + normalEllipseStrokeSize) / 2.0,

            // Top
            joint.Position.Y * -300.0 *
            Math.Min(sScaleW, sScaleH) * multiply +
            matHeight / 3.0 - (normalEllipseSize + normalEllipseStrokeSize) / 2.0,

            // Not used
            0, 0
        );

        ellipse.Visibility = Visibility.Visible;
    }

    private bool SkLine(Line line, TrackedJoint fromJoint, TrackedJoint toJoint)
    {
        // Give up, no such bone/joints
        if (fromJoint is null || toJoint is null)
        {
            line.Visibility = Visibility.Collapsed;
            return false;
        }

        const double matWidthDefault = 700, matHeightDefault = 600;
        double matWidth = SkeletonDrawingCanvas.ActualWidth,
            matHeight = SkeletonDrawingCanvas.ActualHeight;

        // Eventually fix sizes
        if (matWidth < 1) matWidth = matWidthDefault;
        if (matHeight < 1) matHeight = matHeightDefault;

        // Where to scale by 1.0 in perspective
        const double normalDistance = 3;
        const double normalLineStrokeSize = 5;

        // Compose perspective constants, make it 70%
        var fromMultiply = .7 * (normalDistance /
                                 (fromJoint.Position.Z > 0.0 ? fromJoint.Position.Z : 3.0));
        var toMultiply = .7 * (normalDistance /
                               (toJoint.Position.Z > 0.0 ? toJoint.Position.Z : 3.0));

        line.StrokeThickness = normalLineStrokeSize;
        if (fromJoint.TrackingState != TrackedJointState.StateTracked ||
            toJoint.TrackingState != TrackedJointState.StateTracked)
            line.Stroke = new AcrylicBrush
            {
                TintColor = new UISettings()
                    .GetColorValue(UIColorType.Accent)
            };
        else
            line.Stroke = new SolidColorBrush(Colors.White);

        // Select the smaller scale to preserve somewhat uniform skeleton scaling
        double sScaleW = matWidth / matWidthDefault,
            sScaleH = matHeight / matHeightDefault;

        // Move the line to the appropriate point
        line.X1 = fromJoint.Position.X * 300.0 *
            Math.Min(sScaleW, sScaleH) * fromMultiply + matWidth / 2.0;
        line.Y1 = fromJoint.Position.Y * -300.0 *
            Math.Min(sScaleW, sScaleH) * fromMultiply + matHeight / 3.0;

        line.X2 = toJoint.Position.X * 300.0 *
            Math.Min(sScaleW, sScaleH) * toMultiply + matWidth / 2.0;
        line.Y2 = toJoint.Position.Y * -300.0 *
            Math.Min(sScaleW, sScaleH) * toMultiply + matHeight / 3.0;

        line.Visibility = Visibility.Visible;
        return true; // All good
    }

    private void SkBone(Line line, Ellipse point,
        ref List<TrackedJoint> joints,
        TrackedJointType jointTypeStart,
        TrackedJointType jointTypeEnd)
    {
        // Search for the desired joints
        var fromJoint = joints.FirstOrDefault(
            x => x.Role == jointTypeStart, null);
        var toJoint = joints.FirstOrDefault(
            x => x.Role == jointTypeEnd, null);

        // Try drawing a full bone line
        // Redirect to points if failed
        if (SkLine(line, fromJoint, toJoint)) return;

        // Auto-handles (hides) if null
        SkPoint(point, fromJoint, (false, false));
        SkPoint(point, toJoint, (false, false));
    }

    private void ExecuteAutomaticCalibration()
    {
        // Setup the calibration image : reset and stop
        CalibrationPreviewMediaElement.MediaPlayer.Position = TimeSpan.Zero;
        CalibrationPreviewMediaElement.MediaPlayer.Pause();

        AutoCalibrationPane.Visibility = Visibility.Visible;
        ManualCalibrationPane.Visibility = Visibility.Collapsed;

        CalibrationRunningView.DisplayMode = SplitViewDisplayMode.Inline;
        CalibrationRunningView.IsPaneOpen = true;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
        Interfacing.CurrentAppState = "calibration_auto";

        StartAutoCalibrationButton.IsEnabled = true;
        CalibrationPointsNumberBox.IsEnabled = true;
        CalibrationPointsNumberBox.Value = AppData.Settings.CalibrationPointsNumber;

        CalibrationInstructionsLabel.Text = Interfacing.LocalizedJsonString(
            "/GeneralPage/Calibration/Captions/Start");
        NoSkeletonTextNotice.Text = Interfacing.LocalizedJsonString(
            "/GeneralPage/Captions/Preview/NoSkeletonTextCalibrating");
        DiscardAutoCalibrationButton.Content = Interfacing.LocalizedJsonString(
            "/GeneralPage/Buttons/Cancel");

        CalibrationCountdownLabel.Text = "~";
    }

    private async Task ExecuteManualCalibration()
    {
        // Set up as default (just in case)
        LabelFineTuneVive.Visibility = Visibility.Collapsed;
        LabelFineTuneNormal.Visibility = Visibility.Visible;

        // Swap (optionally)
        if (TrackingDevices.CurrentServiceEndpoint.TrackingSystemName.Contains("knuckles",
                StringComparison.OrdinalIgnoreCase) ||
            TrackingDevices.CurrentServiceEndpoint.TrackingSystemName.Contains("index",
                StringComparison.OrdinalIgnoreCase) ||
            TrackingDevices.CurrentServiceEndpoint.TrackingSystemName.Contains("vive",
                StringComparison.OrdinalIgnoreCase))
        {
            LabelFineTuneVive.Visibility = Visibility.Visible;
            LabelFineTuneNormal.Visibility = Visibility.Collapsed;
        }

        // Set up panels
        AutoCalibrationPane.Visibility = Visibility.Collapsed;
        ManualCalibrationPane.Visibility = Visibility.Visible;

        CalibrationRunningView.DisplayMode = SplitViewDisplayMode.Inline;
        CalibrationRunningView.IsPaneOpen = true;

        Interfacing.CurrentAppState = "calibration_manual";

        // Set the [calibration pending] bool
        _calibrationPending = true;

        // Play a nice sound - starting
        AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationStart);

        // Set up (a lot of) helper variables
        var calibrationFirstTime = true;
        float tempYaw = 0, tempPitch = 0;

        // pitch, yaw, roll
        var anglesVector3 = Vector3.Zero;
        var rotationQuaternion = Quaternion.CreateFromYawPitchRoll(
            anglesVector3.Y, anglesVector3.X, anglesVector3.Z);

        // Copy the empty matrices to settings
        AppData.Settings.DeviceCalibrationRotationMatrices[_calibratingDeviceGuid] = rotationQuaternion;
        AppData.Settings.DeviceCalibrationTranslationVectors[_calibratingDeviceGuid] = Vector3.Zero;

        bool calibrationConfirm = false, calibrationModeSwap = false;

        void OnCalibrationConfirmed(object o, EventArgs eventArgs)
        {
            calibrationConfirm = true;
        }

        void OnCalibrationModeChanged(object o, EventArgs eventArgs)
        {
            calibrationModeSwap = true;
        }

        if (TrackingDevices.CurrentServiceEndpoint.ControllerInputActions?.CalibrationConfirmed is not null)
            TrackingDevices.CurrentServiceEndpoint
                .ControllerInputActions.CalibrationConfirmed += OnCalibrationConfirmed;
        if (TrackingDevices.CurrentServiceEndpoint.ControllerInputActions?.CalibrationModeChanged is not null)
            TrackingDevices.CurrentServiceEndpoint
                .ControllerInputActions.CalibrationModeChanged += OnCalibrationModeChanged;

        // Loop over until finished
        while (!calibrationConfirm)
        {
            // Wait for a mode switch
            while (!calibrationModeSwap && !calibrationConfirm)
            {
                // Apply to the global base
                AppData.Settings.DeviceCalibrationTranslationVectors[_calibratingDeviceGuid] +=
                    TrackingDevices.CurrentServiceEndpoint.ControllerInputActions?.MovePositionValues ?? Vector3.Zero;

                await Task.Delay(7);

                // Exit if aborted
                if (!_calibrationPending) break;
            }

            // Reset the swap event
            calibrationModeSwap = false;

            // Play mode swap sound
            if (_calibrationPending && !calibrationConfirm)
                AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationPointCaptured);

            // Set up the calibration origin
            if (calibrationFirstTime)
                AppData.Settings.DeviceCalibrationOrigins[_calibratingDeviceGuid] =
                    Interfacing.DeviceRelativeTransformOrigin.ValueOr(_calibratingDeviceGuid).Position;

            // Cache the calibration first_time
            calibrationFirstTime = false;
            await Task.Delay(300);

            // Wait for a mode switch
            while (!calibrationModeSwap && !calibrationConfirm)
            {
                tempYaw += TrackingDevices.CurrentServiceEndpoint.ControllerInputActions?.AdjustRotationValues
                    .Y ?? 0f; // Left X
                tempPitch += TrackingDevices.CurrentServiceEndpoint.ControllerInputActions?.AdjustRotationValues
                    .X ?? 0f; // Right Y

                anglesVector3 = new Vector3(tempPitch, tempYaw, 0f);
                rotationQuaternion = Quaternion.CreateFromYawPitchRoll(
                    anglesVector3.Y, anglesVector3.X, anglesVector3.Z);

                // Copy the empty matrices to settings
                AppData.Settings.DeviceCalibrationRotationMatrices[_calibratingDeviceGuid] = rotationQuaternion;

                await Task.Delay(7);

                // Exit if aborted
                if (!_calibrationPending) break;
            }

            // Reset the swap event
            calibrationModeSwap = false;

            await Task.Delay(300);

            // Play mode swap sound
            if (_calibrationPending && !calibrationConfirm)
                AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationPointCaptured);

            // Exit if aborted
            if (!_calibrationPending) break;
        }

        if (TrackingDevices.CurrentServiceEndpoint.ControllerInputActions?.CalibrationConfirmed is not null)
            TrackingDevices.CurrentServiceEndpoint
                .ControllerInputActions.CalibrationConfirmed -= OnCalibrationConfirmed;
        if (TrackingDevices.CurrentServiceEndpoint.ControllerInputActions?.CalibrationModeChanged is not null)
            TrackingDevices.CurrentServiceEndpoint
                .ControllerInputActions.CalibrationModeChanged -= OnCalibrationModeChanged;

        // Reset by re-reading the settings if aborted
        if (!_calibrationPending)
        {
            lock (Interfacing.UpdateLock)
            {
                AppData.Settings.ReadSettings();
                AppData.Settings.CheckSettings();
            }

            AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationAborted);
        }
        // Else save I guess
        else
        {
            AppData.Settings.SaveSettings();
            AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationComplete);
            await Task.Delay(1000);
        }

        // Exit the pane and reset
        CalibrationDeviceSelectView.DisplayMode = SplitViewDisplayMode.Overlay;
        CalibrationDeviceSelectView.IsPaneOpen = false;

        CalibrationModeSelectView.DisplayMode = SplitViewDisplayMode.Overlay;
        CalibrationModeSelectView.IsPaneOpen = false;

        CalibrationRunningView.DisplayMode = SplitViewDisplayMode.Overlay;
        CalibrationRunningView.IsPaneOpen = false;

        AllowNavigation(true);
        Interfacing.CurrentAppState = "general";

        _calibrationPending = false; // We're finished
        AppData.Settings.SkeletonPreviewEnabled = _showSkeletonPrevious;
        SetSkeletonVisibility(_showSkeletonPrevious); // Change to whatever
    }

    private void Button_ClickSound(object sender, RoutedEventArgs e)
    {
        // Don't even care if we're not set up yet
        if (!Shared.General.GeneralTabSetupFinished) return;

        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
    }

    private void SetSkeletonVisibility(bool visibility)
    {
        // Don't even care if we're not set up yet
        if (!Shared.General.GeneralTabSetupFinished) return;

        Shared.General.ForceRenderCheckBox.IsEnabled = visibility;
        Shared.General.SkeletonToggleButton.IsChecked = visibility;
        Shared.General.ForceRenderText.Opacity = visibility ? 1.0 : 0.5;
        Shared.General.SkeletonToggleButton.Content = Interfacing.LocalizedJsonString(
            visibility ? "/GeneralPage/Buttons/Skeleton/Hide" : "/GeneralPage/Buttons/Skeleton/Show");
    }

    private void SetSkeletonForce(bool visibility)
    {
        // Don't even care if we're not set up yet
        if (!Shared.General.GeneralTabSetupFinished) return;
        Shared.General.ForceRenderCheckBox.IsChecked = visibility;
    }

    private string CalibrationPointsFormat(string format, int p1, uint p2)
    {
        return format.Replace("{1}", p1.ToString())
            .Replace("{2}", p2.ToString());
    }

    private void AllowNavigation(bool allow)
    {
        Shared.Main.NavigationBlockerGrid.IsHitTestVisible = !allow;
    }

    private void OffsetsControlPivot_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        // Don't even care if we're not set up yet
        if (!((sender as Pivot)?.IsLoaded ?? false)) return;

        if (_offsetsPageNavigated)
        {
            // The last item
            if (((Pivot)sender).SelectedIndex == ((Pivot)sender).Items.Count - 1)
                AppSounds.PlayAppSound(_previousOffsetPageIndex == 0
                    ? AppSounds.AppSoundType.MovePrevious
                    : AppSounds.AppSoundType.MoveNext);

            // The first item
            else if (((Pivot)sender).SelectedIndex == 0)
                AppSounds.PlayAppSound(_previousOffsetPageIndex == ((Pivot)sender).Items.Count - 1
                    ? AppSounds.AppSoundType.MoveNext
                    : AppSounds.AppSoundType.MovePrevious);

            // Default
            else
                AppSounds.PlayAppSound(((Pivot)sender).SelectedIndex > _previousOffsetPageIndex
                    ? AppSounds.AppSoundType.MoveNext
                    : AppSounds.AppSoundType.MovePrevious);
        }

        // Cache
        _previousOffsetPageIndex = ((Pivot)sender).SelectedIndex;
        _offsetsPageNavigated = true;
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));

        // Setup boiler
        Shared.General.GeneralTabSetupFinished = false;

        // Setup the preview button
        SetSkeletonVisibility(AppData.Settings.SkeletonPreviewEnabled);
        SetSkeletonForce(AppData.Settings.ForceSkeletonPreview);

        // Setup the freeze button
        ToggleFreezeButton.IsChecked = Interfacing.IsTrackingFrozen;
        FreezeOnlyLowerToggle.IsChecked = AppData.Settings.FreezeLowerBodyOnly;
        ToggleFreezeButton.Content = Interfacing.LocalizedJsonString(
            Interfacing.IsTrackingFrozen
                ? "/GeneralPage/Buttons/Skeleton/Unfreeze"
                : "/GeneralPage/Buttons/Skeleton/Freeze");

        // Set up the co/re/disconnect button
        if (!Interfacing.AppTrackersSpawned)
        {
            ToggleTrackersButton.IsChecked = false;
            ToggleTrackersButton.Content =
                Interfacing.LocalizedJsonString("/GeneralPage/Buttons/TrackersToggle/Connect");
        }
        else
        {
            ToggleTrackersButton.IsChecked = Interfacing.AppTrackersInitialized;
            ToggleTrackersButton.Content = Interfacing.LocalizedJsonString(
                Interfacing.AppTrackersInitialized
                    ? "/GeneralPage/Buttons/TrackersToggle/Disconnect"
                    : "/GeneralPage/Buttons/TrackersToggle/Reconnect");
        }

        // Set up the skeleton toggle button
        SkeletonToggleButton.Content = Interfacing.LocalizedJsonString(
            AppData.Settings.SkeletonPreviewEnabled
                ? "/GeneralPage/Buttons/Skeleton/Hide"
                : "/GeneralPage/Buttons/Skeleton/Show");

        // Setup end
        Shared.General.GeneralTabSetupFinished = true;
    }
}

public class AttachedString : DependencyObject
{
    public static readonly DependencyProperty AttachedStringProperty =
        DependencyProperty.RegisterAttached(
            "AttachedString",
            typeof(string),
            typeof(AttachedString),
            new PropertyMetadata(false)
        );

    public static void SetAttachedString(UIElement element, string value)
    {
        element.SetValue(AttachedStringProperty, value);
    }

    public static string GetAttachedString(UIElement element)
    {
        return (string)element.GetValue(AttachedStringProperty);
    }
}