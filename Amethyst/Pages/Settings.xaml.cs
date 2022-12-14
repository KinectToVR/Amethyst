// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Windows.Globalization;
using Windows.System;
using Windows.System.UserProfile;
using Amethyst.Classes;
using Amethyst.MVVM;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Media.Animation;
using WinRT;
using static Amethyst.Classes.Shared.TeachingTips;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Pages;

/// <summary>
///     An empty page that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class Settings : Page, INotifyPropertyChanged
{
    private readonly List<string> _languageList = new();
    private bool _settingsPageLoadedOnce;

    public Settings()
    {
        InitializeComponent();

        Logger.Info($"Constructing page: '{GetType().FullName}'...");

        Shared.Settings.RestartButton = RestartButton;
        Shared.Settings.FlipDropDownGrid = FlipDropDownGrid;
        Shared.Settings.PageMainScrollViewer = PageMainScrollViewer;
        Shared.Settings.FlipToggle = FlipToggle;
        Shared.Settings.FlipDropDown = FlipDropDown;
        Shared.Settings.ExternalFlipCheckBox = ExternalFlipCheckBox;
        Shared.Settings.CheckOverlapsCheckBox = CheckOverlapsCheckBox;
        Shared.Settings.SetErrorFlyoutText = SetErrorFlyoutText;
        Shared.Settings.ExternalFlipStatusLabel = ExtFlipStatusLabel;
        Shared.Settings.ExternalFlipStatusStackPanel = ExtFlipStatusStackPanel;
        Shared.Settings.FlipDropDownContainer = FlipDropDownContainer;

        SettingsPage.AutoStartTeachingTip = AutoStartTeachingTip;
        SettingsPage.ManageTrackersTeachingTip = ManageTrackersTeachingTip;

        // This 'ease in' transition will affect the added expander
        AppData.Settings.TrackersVector.ToList().ForEach(tracker =>
        {
            tracker.SettingsExpanderTransitions =
                new TransitionCollection { new RepositionThemeTransition() };
            tracker.OnPropertyChanged(); // Refresh the transition
        });

        AppData.Settings.TrackersVector.CollectionChanged += (_, _) =>
        {
            // Trackers' collection has changed, 
            Shared.Main.DispatcherQueue.TryEnqueue(async () =>
            {
                // This 'ease in' transition will affect the added expander
                AppData.Settings.TrackersVector.ToList().ForEach(tracker =>
                {
                    tracker.SettingsExpanderTransitions =
                        new TransitionCollection { new ContentThemeTransition() };
                    tracker.OnPropertyChanged(); // Refresh the transition
                });

                // Wait for the transition to end
                await Task.Delay(200);

                // This 'move' transition will affect all tracker expanders
                AppData.Settings.TrackersVector.ToList().ForEach(tracker =>
                {
                    tracker.SettingsExpanderTransitions =
                        new TransitionCollection { new RepositionThemeTransition() };
                    tracker.OnPropertyChanged(); // Refresh the transition
                });
            });
        };

        Logger.Info($"Registering settings MVVM for page: '{GetType().FullName}'...");
        DataContext = AppData.Settings; // Set this settings instance as the context

        Logger.Info("Registering a detached binary semaphore " +
                    $"reload handler for '{GetType().FullName}'...");

        Task.Run(() =>
        {
            Shared.Events.ReloadSettingsPageEvent =
                new ManualResetEvent(false);

            while (true)
            {
                // Wait for a reload signal (blocking)
                Shared.Events.ReloadSettingsPageEvent.WaitOne();

                // Reload & restart the waiting loop
                if (_settingsPageLoadedOnce && Interfacing.CurrentPageTag == "settings")
                    Shared.Main.DispatcherQueue.TryEnqueue(Page_LoadedHandler);

                // Reset the event
                Shared.Events.ReloadSettingsPageEvent.Reset();
            }
        });
    }

    public List<AppTrackerEntry> ToggleableTrackerEntries => Enum.GetValues<TrackerType>()
        // Pick all non-default (additional) trackers
        .Where(x => x is not TrackerType.TrackerWaist and not
            TrackerType.TrackerLeftFoot and not TrackerType.TrackerRightFoot and not
            TrackerType.TrackerLeftKnee and not TrackerType.TrackerRightKnee and not
            TrackerType.TrackerLeftElbow and not TrackerType.TrackerRightElbow)
        // Convert to an entry, passing the filtered tracker role
        .Select(x => new AppTrackerEntry { TrackerRole = x }).ToList();

    // MVVM stuff: service settings and its status
    private IEnumerable<Page> ServiceSettingsPage => new[]
    {
        TrackingDevices.CurrentServiceEndpoint.SettingsInterfaceRoot as Page
    };

    private IEnumerable<string> LoadedServiceNames =>
        TrackingDevices.ServiceEndpointsList.Values.Select(service => service.Name);

    private int SelectedServiceIndex =>
        TrackingDevices.ServiceEndpointsList.Values.ToList().IndexOf(TrackingDevices.CurrentServiceEndpoint);

    private bool ServiceStatusError => TrackingDevices.CurrentServiceEndpoint.StatusError;

    private bool ServiceSupportsSettings => TrackingDevices.CurrentServiceEndpoint.IsSettingsDaemonSupported &&
                                            TrackingDevices.CurrentServiceEndpoint.SettingsInterfaceRoot is Page;

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

    private bool ServiceNeedsRestart => TrackingDevices.CurrentServiceEndpoint.IsRestartOnChangesNeeded;

    // Dynamically switch between CardMiddleStyle and CardBottomStyle
    private CornerRadius ServiceControlsCornerRadius =>
        ServiceSupportsSettings ? new CornerRadius(0) : new CornerRadius(0, 0, 4, 4);

    // MVVM stuff: bound strings with placeholders
    private string RestartServiceText => Interfacing.LocalizedJsonString("/SettingsPage/Buttons/RestartService")
        .Replace("{0}", TrackingDevices.CurrentServiceEndpoint.Name);

    private string RestartServiceNoteL1 => Interfacing
        .LocalizedJsonString("/SettingsPage/Captions/TrackersRestart/Line1")
        .Replace("{0}", TrackingDevices.CurrentServiceEndpoint.Name);

    private string RestartServiceNoteL2 => Interfacing
        .LocalizedJsonString("/SettingsPage/Captions/TrackersRestart/Line2")
        .Replace("{0}", TrackingDevices.CurrentServiceEndpoint.Name);

    private string AutoStartLabelText => Interfacing.LocalizedJsonString("/SettingsPage/Captions/AutoStart")
        .Replace("{0}", TrackingDevices.CurrentServiceEndpoint.Name);

    private string AutoStartTipText => Interfacing.LocalizedJsonString("/NUX/Tip7/Title")
        .Replace("{0}", TrackingDevices.CurrentServiceEndpoint.Name);

    private string AutoStartTipContent => Interfacing.LocalizedJsonString("/NUX/Tip7/Content")
        .Replace("{0}", TrackingDevices.CurrentServiceEndpoint.Name);

    private string ManageTrackersText
    {
        get
        {
            if (TrackingDevices.CurrentServiceEndpoint.IsRestartOnChangesNeeded)
                return Interfacing.LocalizedJsonString("/SettingsLearn/Captions/ManageTrackers")
                    .Replace("{0}", TrackingDevices.CurrentServiceEndpoint.Name)
                    .Replace("[[", "").Replace("]]", "");

            // If the service doesn't need restarting
            return StringUtils.RemoveBetween(
                Interfacing.LocalizedJsonString("/SettingsLearn/Captions/ManageTrackers")
                    .Replace("{0}", TrackingDevices.CurrentServiceEndpoint.Name), "[[", "]]");
        }
    }

    // MVVM stuff
    public event PropertyChangedEventHandler PropertyChanged;


    private void Page_Loaded(object sender, RoutedEventArgs e)
    {
        Logger.Info($"Re/Loading page: '{GetType().FullName}'...");
        Interfacing.CurrentAppState = "settings";

        // Execute the handler
        Page_LoadedHandler();

        // Mark as loaded
        _settingsPageLoadedOnce = true;
    }

    private void Page_LoadedHandler()
    {
        // Notify of the setup beginning
        Shared.Settings.SettingsTabSetupFinished = false;

        // Clear available languages' list
        LanguageOptionBox.Items.Clear();

        // Push all the found languages
        if (Directory.Exists(Path.Join(Interfacing.GetProgramLocation().DirectoryName, "Assets", "Strings")))
            foreach (var entry in Directory.EnumerateFiles(
                         Path.Join(Interfacing.GetProgramLocation().DirectoryName, "Assets", "Strings")))
            {
                if (Path.GetFileNameWithoutExtension(entry) == "locales") continue;

                _languageList.Add(Path.GetFileNameWithoutExtension(entry));
                LanguageOptionBox.Items.Add(Interfacing.GetLocalizedLanguageName(
                    Path.GetFileNameWithoutExtension(entry)));

                if (Path.GetFileNameWithoutExtension(entry) == AppData.Settings.AppLanguage)
                    LanguageOptionBox.SelectedIndex = LanguageOptionBox.Items.Count - 1;
            }

        // Clear available themes' list
        AppThemeOptionBox.Items.Clear();

        // Push all the available themes
        AppThemeOptionBox.Items.Add(Interfacing.LocalizedJsonString("/SettingsPage/Themes/System"));
        AppThemeOptionBox.Items.Add(Interfacing.LocalizedJsonString("/SettingsPage/Themes/Dark"));
        AppThemeOptionBox.Items.Add(Interfacing.LocalizedJsonString("/SettingsPage/Themes/White"));

        // Select the current theme
        AppThemeOptionBox.SelectedIndex = (int)AppData.Settings.AppTheme;

        // Check for autostart
        AutoStartCheckBox.IsChecked = TrackingDevices.CurrentServiceEndpoint.AutoStartAmethyst;

        // Optionally show the foreign language grid
        if (!File.Exists(Path.Join(Interfacing.GetProgramLocation().DirectoryName, "Assets", "Strings",
                new Language(GlobalizationPreferences.Languages[0]).LanguageTag[..2] + ".json")))
            ForeignLangGrid.Visibility = Visibility.Visible;

        // Enable/Disable Ext/Flip
        TrackingDevices.CheckFlipSupport();

        // Notify of the setup's end
        OnPropertyChanged(); // Just everything
        Shared.Settings.SettingsTabSetupFinished = true;
    }

    private void LanguageOptionBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;

        if (LanguageOptionBox.SelectedIndex < 0)
            LanguageOptionBox.SelectedItem = e.RemovedItems[0];

        // Overwrite the current language code
        AppData.Settings.AppLanguage = _languageList[LanguageOptionBox.SelectedIndex];

        // Save made changes
        AppData.Settings.SaveSettings();

        // Reload language resources
        Interfacing.LoadJsonStringResourcesEnglish();
        Interfacing.LoadJsonStringResources(AppData.Settings.AppLanguage);

        // Reload plugins' language resources
        foreach (var plugin in TrackingDevices.TrackingDevicesList.Values)
            Interfacing.Plugins.SetLocalizationResourcesRoot(plugin.LocalizationResourcesRoot.Directory, plugin.Guid);

        // Reload everything we can
        Shared.Devices.DevicesJointsValid = false;

        // Request page reloads
        Translator.Get.OnPropertyChanged();
        Shared.Events.RequestInterfaceReload();

        // Request manager reloads
        AppData.Settings.OnPropertyChanged();
        AppData.Settings.TrackersVector.ToList()
            .ForEach(x => x.OnPropertyChanged());

        // We're done with our changes now!
        Shared.Devices.DevicesJointsValid = true;
    }

    private void OptionBox_DropDownOpened(object sender, object e)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void OptionBox_DropDownClosed(object sender, object e)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }

    private void AppThemeOptionBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;

        if (AppThemeOptionBox.SelectedIndex < 0)
            AppThemeOptionBox.SelectedItem = e.RemovedItems[0];

        // Overwrite the current theme
        AppData.Settings.AppTheme = (uint)AppThemeOptionBox.SelectedIndex;

        Shared.Main.MainNavigationView.XamlRoot.Content.As<Grid>().RequestedTheme =
            AppData.Settings.AppTheme switch
            {
                2 => ElementTheme.Light,
                1 => ElementTheme.Dark,
                _ => ElementTheme.Default
            };

        // Save made changes
        AppData.Settings.SaveSettings();
    }

    private void AutoStartCheckBox_Checked(object sender, RoutedEventArgs e)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;
        TrackingDevices.CurrentServiceEndpoint.AutoStartAmethyst = true;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.ToggleOn);
    }

    private void AutoStartCheckBox_Unchecked(object sender, RoutedEventArgs e)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;
        TrackingDevices.CurrentServiceEndpoint.AutoStartAmethyst = false;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.ToggleOff);
    }

    private void CheckBox_CheckedSound(object sender, RoutedEventArgs e)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.ToggleOn);
    }

    private void CheckBox_UncheckedSound(object sender, RoutedEventArgs e)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.ToggleOff);
    }

    private async void AutoStartTeachingTip_ActionButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Close the current tip
        AutoStartTeachingTip.IsOpen = false;

        PageMainScrollViewer.UpdateLayout();
        PageMainScrollViewer.ChangeView(null,
            PageMainScrollViewer.ExtentHeight / 2.0, null);

        await Task.Delay(500);

        // Show the previous one
        AddTrackersTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        AddTrackersTeachingTip.IsOpen = true;
    }

    private async void AutoStartTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        await Task.Delay(200);

        // Navigate to the devices page
        Shared.Main.MainNavigationView.SelectedItem =
            Shared.Main.MainNavigationView.MenuItems[2];

        Shared.Main.NavigateToPage("devices",
            new EntranceNavigationTransitionInfo());

        await Task.Delay(500);

        // Reset the previous page layout
        PageMainScrollViewer.ScrollToVerticalOffset(0);

        // Show the next tip
        DevicesPage.DevicesListTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        DevicesPage.DevicesListTeachingTip.IsOpen = true;
    }

    private void FlipToggle_Toggled(object sender, RoutedEventArgs e)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;

        // Optionally show the binding teaching tip
        if (!AppData.Settings.TeachingTipShownFlip && Interfacing.CurrentPageTag == "settings" &&
            !string.IsNullOrEmpty(
                TrackingDevices.CurrentServiceEndpoint.ControllerInputActions?.SkeletonFlipActionTitleString) &&
            !string.IsNullOrEmpty(
                TrackingDevices.CurrentServiceEndpoint.ControllerInputActions?.SkeletonFlipActionContentString))
        {
            ToggleFlipTeachingTip.Title =
                TrackingDevices.CurrentServiceEndpoint.ControllerInputActions?.SkeletonFlipActionTitleString;
            ToggleFlipTeachingTip.Subtitle =
                TrackingDevices.CurrentServiceEndpoint.ControllerInputActions?.SkeletonFlipActionContentString;

            ToggleFlipTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;

            Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = true;
            ToggleFlipTeachingTip.IsOpen = true;
            AppData.Settings.TeachingTipShownFlip = true;

            AppData.Settings.SaveSettings();
        }

        // Play a sound
        AppSounds.PlayAppSound(sender.As<ToggleSwitch>().IsOn
            ? AppSounds.AppSoundType.ToggleOn
            : AppSounds.AppSoundType.ToggleOff);
    }

    private void ToggleFlipTeachingTip_Closed(TeachingTip sender, TeachingTipClosedEventArgs args)
    {
        Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = false;
    }

    private void FlipDropDown_Expanding(Expander sender, ExpanderExpandingEventArgs args)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;
        TrackingDevices.CheckFlipSupport();

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void FlipDropDown_Collapsed(Expander sender, ExpanderCollapsedEventArgs args)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }

    private void MenuFlyout_Opening(object sender, object e)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void MenuFlyout_Closing(FlyoutBase sender,
        FlyoutBaseClosingEventArgs args)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }

    private void CalibrateExternalFlipMenuFlyoutItem_Click(object sender, RoutedEventArgs e)
    {
        // If the extflip is from Amethyst
        AppData.Settings.ExternalFlipCalibrationMatrix =
            AppData.Settings.TrackersVector[0].IsOrientationOverridden
                ? AppData.Settings.TrackersVector[0].Orientation
                : Interfacing.GetVrTrackerPoseCalibrated("waist").Orientation;

        Logger.Info($"Captured orientation for external flip: {AppData.Settings.ExternalFlipCalibrationMatrix}");
        AppData.Settings.SaveSettings();
    }

    private async void ManageTrackersTeachingTip_ActionButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Close the current tip
        ManageTrackersTeachingTip.IsOpen = false;

        await Task.Delay(400);

        // Navigate to the settings page
        Shared.Main.MainNavigationView.SelectedItem =
            Shared.Main.MainNavigationView.MenuItems[0];

        Shared.Main.NavigateToPage("general",
            new EntranceNavigationTransitionInfo());

        await Task.Delay(500);

        // Show the next tip
        GeneralPage.StatusTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        GeneralPage.StatusTeachingTip.IsOpen = true;
    }

    private async void ManageTrackersTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        PageMainScrollViewer.UpdateLayout();
        PageMainScrollViewer.ChangeView(null,
            PageMainScrollViewer.ExtentHeight / 2.0, null);

        await Task.Delay(500);

        AddTrackersTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        AddTrackersTeachingTip.IsOpen = true;
    }

    private void LearnAboutFiltersButton_Click(object sender, RoutedEventArgs e)
    {
        DimGrid.Opacity = 0.35;
        DimGrid.IsHitTestVisible = true;

        LearnAboutFiltersFlyout.ShowAt(LearnAboutFiltersButton, new FlyoutShowOptions
        {
            Placement = FlyoutPlacementMode.Full,
            ShowMode = FlyoutShowMode.Transient
        });
    }

    private void LearnAboutFiltersFlyout_Closed(object sender, object e)
    {
        DimGrid.Opacity = 0.0;
        DimGrid.IsHitTestVisible = false;
    }

    private void TrackerConfigButton_Click(object sender, RoutedEventArgs e)
    {
        // ignored
    }

    private async void AddTrackersTeachingTip_ActionButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Close the current tip
        AddTrackersTeachingTip.IsOpen = false;

        PageMainScrollViewer.UpdateLayout();
        PageMainScrollViewer.ChangeView(null,
            PageMainScrollViewer.ExtentHeight / 2.0, null);

        await Task.Delay(500);

        // Show the previous one
        ManageTrackersTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        ManageTrackersTeachingTip.IsOpen = true;
    }

    private async void AddTrackersTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        PageMainScrollViewer.UpdateLayout();
        PageMainScrollViewer.ChangeView(null, 0.0, null);

        await Task.Delay(500);

        AutoStartTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        AutoStartTeachingTip.IsOpen = true;
    }

    private void RestartButton_Click(object sender, RoutedEventArgs e)
    {
        TrackingDevices.CurrentServiceEndpoint.RequestServiceRestart(
            Interfacing.LocalizedJsonString("/SettingsPage/Captions/ServiceRestart")
                .Replace("{0}", TrackingDevices.CurrentServiceEndpoint.Name));

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
    }

    private async void ReNUXButton_Click(object sender, RoutedEventArgs e)
    {
        Logger.Info("Manual NUX relaunch invoked! Starting NUX...");

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Navigate to the general page
        Shared.Main.MainNavigationView.SelectedItem =
            Shared.Main.MainNavigationView.MenuItems[0];

        Shared.Main.NavigateToPage("general",
            new EntranceNavigationTransitionInfo());

        await Task.Delay(500);

        // Show the first tip
        Shared.Main.InterfaceBlockerGrid.Opacity = 0.35;
        Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = true;

        MainPage.InitializerTeachingTip.IsOpen = true;
        Interfacing.IsNuxPending = true;
    }

    private async void ResetButton_Click(object sender, RoutedEventArgs e)
    {
        Logger.Info("Reset has been invoked: turning trackers off...");

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Mark trackers as inactive
        Interfacing.AppTrackersInitialized = false;
        if (Shared.General.ToggleTrackersButton is not null)
            Shared.General.ToggleTrackersButton.IsChecked = false;

        Logger.Info("Reset has been invoked: clearing app settings...");

        // Mark exiting as true
        Interfacing.IsExitingNow = true;
        await Task.Delay(50);

        // Read settings after reset
        AppData.Settings = new AppSettings(); // Reset settings
        AppData.Settings.SaveSettings(); // Save empty settings

        /* Restart */

        Logger.Info("Reset invoked: trying to restart the app...");

        // If we've found who asked
        if (File.Exists(Interfacing.GetProgramLocation().FullName))
        {
            // Log the caller
            Logger.Info($"The current caller process is: {Interfacing.GetProgramLocation().FullName}");

            // Exit the app
            Logger.Info("Configuration has been reset, exiting in 500ms...");

            // Don't execute the exit routine
            Interfacing.IsExitHandled = true;

            // Handle a typical app exit
            await Interfacing.HandleAppExit(500);

            // Restart and exit with code 0
            Process.Start(Interfacing.GetProgramLocation()
                .FullName.Replace(".dll", ".exe"));

            // Exit without re-handling everything
            Environment.Exit(0);
        }

        // Still here?
        Logger.Error("App will not be restarted due to caller process identification error.");

        Interfacing.ShowToast(
            Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed/Title"),
            Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed"));
        Interfacing.ShowServiceToast(
            Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed/Title"),
            Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed"));
    }

    private void ViewLogsButton_Click(object sender, RoutedEventArgs e)
    {
        SystemShell.OpenFolderAndSelectItem(Logger.LogFilePath);

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
    }

    private void DismissSetErrorButton_Click(object sender, RoutedEventArgs e)
    {
        SetErrorFlyout.Hide();
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    private async void TrackerToggleMenuFlyoutItem_Click(object sender, RoutedEventArgs e)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;

        // Play a sound
        AppSounds.PlayAppSound((sender as ToggleMenuFlyoutItem)!.IsChecked
            ? AppSounds.AppSoundType.ToggleOn
            : AppSounds.AppSoundType.ToggleOff);

        // Notify of the setup end
        Shared.Settings.SettingsTabSetupFinished = false;

        // Find out who asked
        var role = ((sender as ToggleMenuFlyoutItem)!
            .DataContext as AppTrackerEntry)!.TrackerRole;

        // Create a new tracker
        if ((sender as ToggleMenuFlyoutItem)!.IsChecked)
        {
            AppData.Settings.TrackersVector.Add(new AppTracker
            {
                Role = role,
                Serial = TypeUtils.TrackerTypeRoleSerialDictionary[role]
            });
        }

        // If the tracker was unchecked
        else
        {
            // Find the removed tracker
            var removedTracker = AppData.Settings.TrackersVector
                .FirstOrDefault(x => x.Role == role, null);
            if (removedTracker is null) return; // False alarm?

            // Make actual changes
            if (removedTracker.IsActive && Interfacing.AppTrackersInitialized)
            {
                var trackerBase = removedTracker.GetTrackerBase();
                trackerBase.ConnectionState = false;

                await TrackingDevices.CurrentServiceEndpoint.SetTrackerStates(new[] { trackerBase });
            }

            await Task.Delay(20);
            AppData.Settings.TrackersVector.Remove(removedTracker);

            // If the tracker was on and then removed
            if (removedTracker.IsActive)
            {
                // Show the notifications and rebuild (boiler-ed)
                Shared.Settings.SettingsTabSetupFinished = true;
                TrackingDevices.TrackersConfigChanged();
                Shared.Settings.SettingsTabSetupFinished = false;
            }
        }

        // Notify of the setup end
        Shared.Settings.SettingsTabSetupFinished = true;
        TrackingDevices.CheckFlipSupport();

        AppData.Settings.CheckSettings();
        AppData.Settings.SaveSettings();
        AppData.Settings.OnPropertyChanged();

        // Reload pages for the new changes
        Shared.Events.RequestInterfaceReload();

        Logger.Info("Requesting a check for already-added trackers...");
        Interfacing.AlreadyAddedTrackersScanRequested = true;
        ToggleTrackersFlyout.Hide(); // Hide the flyout

        await Task.Delay(10);
        AppData.Settings.OnPropertyChanged();
        OnPropertyChanged(); // Retry 1

        await Task.Delay(100);
        AppData.Settings.OnPropertyChanged();
        OnPropertyChanged(); // Retry 2
    }

    private async void PairsToggleMenuFlyoutItem_Click(object sender, RoutedEventArgs e)
    {
        // Second copy just in case
        AppData.Settings.UseTrackerPairs =
            (sender as ToggleMenuFlyoutItem)!.IsChecked;

        AppData.Settings.CheckSettings();
        AppData.Settings.SaveSettings();

        // Reload pages for the new changes
        AppData.Settings.OnPropertyChanged();
        ToggleTrackersFlyout.Hide(); // Hide the flyout

        await Task.Delay(10);
        AppData.Settings.OnPropertyChanged();
        OnPropertyChanged(); // Retry 1

        await Task.Delay(100);
        AppData.Settings.OnPropertyChanged();
        OnPropertyChanged(); // Retry 2
    }

    private void RefreshServiceButton_Click(SplitButton sender, SplitButtonClickEventArgs args)
    {
        Logger.Info($"Now reinitializing service endpoint {TrackingDevices.CurrentServiceEndpoint.Guid}...");
        TrackingDevices.CurrentServiceEndpoint.Initialize();

        // Force refresh all the valid pages
        Shared.Events.RequestInterfaceReload(false);

        // Update other components (may be moved to MVVM)
        TrackingDevices.HandleDeviceRefresh(false);
        TrackingDevices.UpdateTrackingDevicesInterface();
        AlternativeConnectionOptionsFlyout.Hide();

        // Set up the co/re/disconnect button
        Shared.General.ToggleTrackersButton.IsChecked = false;
        Shared.General.ToggleTrackersButton.Content =
            Interfacing.LocalizedJsonString(Interfacing.K2AppTrackersSpawned
                ? "/GeneralPage/Buttons/TrackersToggle/Reconnect"
                : "/GeneralPage/Buttons/TrackersToggle/Connect");

        // Hide additional controls
        Shared.General.CalibrationButton.IsEnabled = false;

        // Mark the service as non-failed, clean
        Interfacing.ServiceEndpointFailure = false;
        Interfacing.K2AppTrackersSpawned = false;
        Interfacing.AppTrackersInitialized = false;
        Interfacing.ServiceEndpointSetup(); // Refresh

        // Reload everything we can
        Shared.Devices.DevicesJointsValid = false;

        // Request page reloads
        Translator.Get.OnPropertyChanged();
        Shared.Events.RequestInterfaceReload();

        // Request manager reloads
        AppData.Settings.OnPropertyChanged();
        AppData.Settings.TrackersVector.ToList()
            .ForEach(x => x.OnPropertyChanged());

        // We're done with our changes now!
        Shared.Devices.DevicesJointsValid = true;
    }

    private void ShutdownServiceButton_Click(object sender, RoutedEventArgs e)
    {
        try
        {
            Logger.Info($"Now shutting down service endpoint {TrackingDevices.CurrentServiceEndpoint.Guid}...");
            TrackingDevices.CurrentServiceEndpoint.Shutdown();
        }
        catch (Exception ex)
        {
            Logger.Info($"Shutting down service endpoint {TrackingDevices.CurrentServiceEndpoint.Guid} failed! " +
                        $"Exception: {ex.GetType().Name} in {ex.Source}: {ex.Message}\n{ex.StackTrace}");
        }

        // Force refresh all the valid pages
        Shared.Events.RequestInterfaceReload(false);

        // Update other components (may be moved to MVVM)
        TrackingDevices.HandleDeviceRefresh(false);
        TrackingDevices.UpdateTrackingDevicesInterface();
        AlternativeConnectionOptionsFlyout.Hide();

        // Set up the co/re/disconnect button
        Shared.General.ToggleTrackersButton.IsChecked = false;
        Shared.General.ToggleTrackersButton.Content =
            Interfacing.LocalizedJsonString(Interfacing.K2AppTrackersSpawned
                ? "/GeneralPage/Buttons/TrackersToggle/Reconnect"
                : "/GeneralPage/Buttons/TrackersToggle/Connect");

        // Hide additional controls
        Shared.General.CalibrationButton.IsEnabled = false;

        // Mark the service as non-failed, clean
        Interfacing.ServiceEndpointFailure = false;
        Interfacing.K2AppTrackersSpawned = false;
        Interfacing.AppTrackersInitialized = false;
        Interfacing.ServiceEndpointSetup(); // Refresh

        // Reload everything we can
        Shared.Devices.DevicesJointsValid = false;

        // Request page reloads
        Translator.Get.OnPropertyChanged();
        Shared.Events.RequestInterfaceReload();

        // Request manager reloads
        AppData.Settings.OnPropertyChanged();
        AppData.Settings.TrackersVector.ToList()
            .ForEach(x => x.OnPropertyChanged());

        // We're done with our changes now!
        Shared.Devices.DevicesJointsValid = true;
    }

    private async void OpenDocsButton_Click(object sender, RoutedEventArgs e)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        await Launcher.LaunchUriAsync(
            new Uri($"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/app/help/"));
    }

    private async void OpenDiscordButton_Click(object sender, RoutedEventArgs e)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        await Launcher.LaunchUriAsync(new Uri("https://discord.gg/YBQCRDG"));
    }

    private async void SelectedServiceComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if ((sender as ComboBox)!.SelectedIndex < 0)
            (sender as ComboBox)!.SelectedItem = e.RemovedItems[0];

        // Get the selected device by its vector index
        var selectedService = TrackingDevices.ServiceEndpointsList.Values
            .ElementAt((sender as ComboBox)!.SelectedIndex);

        // Check if not null
        if (selectedService is null)
        {
            Logger.Info("The newly selected service appears to be null, aborting!");
            return; // Abandon this one and don't care further
        }

        // Check if the selected service isn't already set
        if (selectedService == TrackingDevices.CurrentServiceEndpoint) return;
        Logger.Info("The selected service endpoint was requested to be changed to " +
                    $"({selectedService.Guid}, {selectedService.Name})!");

        // Check and disable previous service's provided [freeze] action handlers
        if (TrackingDevices.CurrentServiceEndpoint
                .ControllerInputActions?.TrackingFreezeToggled is not null)
            TrackingDevices.CurrentServiceEndpoint
                .ControllerInputActions.TrackingFreezeToggled -= Main.FreezeActionToggled;

        // Check and disable previous service's provided [flip] action handlers
        if (TrackingDevices.CurrentServiceEndpoint
                .ControllerInputActions?.SkeletonFlipToggled is not null)
            TrackingDevices.CurrentServiceEndpoint
                .ControllerInputActions.SkeletonFlipToggled -= Main.FlipActionToggled;

        // Set up the co/re/disconnect button
        Shared.General.ToggleTrackersButton.IsChecked = false;
        Shared.General.ToggleTrackersButton.Content =
            Interfacing.LocalizedJsonString(Interfacing.K2AppTrackersSpawned
                ? "/GeneralPage/Buttons/TrackersToggle/Reconnect"
                : "/GeneralPage/Buttons/TrackersToggle/Connect");

        // Hide additional controls
        Shared.General.CalibrationButton.IsEnabled = false;

        // Mark the service as non-failed, disable trackers
        Interfacing.ServiceEndpointFailure = false;
        Interfacing.AppTrackersInitialized = false;

        await Task.Delay(300); // Wait until disabled
        Interfacing.K2AppTrackersSpawned = false;

        try
        {
            // Try disabling the currently selected service endpoint
            Logger.Info("Shutting down the currently selected service " +
                        $"({TrackingDevices.CurrentServiceEndpoint.Guid}, " +
                        $"{TrackingDevices.CurrentServiceEndpoint.Name})...");

            TrackingDevices.CurrentServiceEndpoint.Shutdown();
        }
        catch (Exception ex)
        {
            Logger.Info($"Shutting down service endpoint {TrackingDevices.CurrentServiceEndpoint.Guid} failed! " +
                        $"Exception: {ex.GetType().Name} in {ex.Source}: {ex.Message}\n{ex.StackTrace}");
        }

        // Check and use service's provided [freeze] action handlers
        if (TrackingDevices.ServiceEndpointsList[AppData.Settings.ServiceEndpointGuid]
                .ControllerInputActions?.TrackingFreezeToggled is not null)
            TrackingDevices.ServiceEndpointsList[AppData.Settings.ServiceEndpointGuid]
                .ControllerInputActions.TrackingFreezeToggled += Main.FreezeActionToggled;

        // Check and use service's provided [flip] action handlers
        if (TrackingDevices.ServiceEndpointsList[AppData.Settings.ServiceEndpointGuid]
                .ControllerInputActions?.SkeletonFlipToggled is not null)
            TrackingDevices.ServiceEndpointsList[AppData.Settings.ServiceEndpointGuid]
                .ControllerInputActions.SkeletonFlipToggled += Main.FlipActionToggled;

        // Update the selected service in application settings
        AppData.Settings.ServiceEndpointGuid = selectedService.Guid;

        // Re-initialize just in case
        Logger.Info("Now reinitializing service endpoint " +
                    $"{TrackingDevices.CurrentServiceEndpoint.Guid}...");

        TrackingDevices.CurrentServiceEndpoint.Initialize();
        Interfacing.ServiceEndpointSetup(); // Refresh

        // Reload everything we can
        Shared.Devices.DevicesJointsValid = false;

        // Request page reloads
        Translator.Get.OnPropertyChanged();
        Shared.Events.RequestInterfaceReload();

        // Request manager reloads
        AppData.Settings.OnPropertyChanged();
        AppData.Settings.TrackersVector.ToList()
            .ForEach(x => x.OnPropertyChanged());

        // We're done with our changes now!
        Shared.Devices.DevicesJointsValid = true;
    }

    private void ServiceCombo_OnDropDownOpened(object sender, object e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void ServiceCombo_OnDropDownClosed(object sender, object e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }
}