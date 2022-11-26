// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System.Collections.Generic;
using System.IO;
using Amethyst.Classes;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using System.Threading.Tasks;
using Windows.Media.Core;
using Valve.VR;
using Windows.Globalization;
using Windows.System.UserProfile;
using WinRT;
using Microsoft.UI.Xaml.Media.Animation;
using System.Text;
using System;
using ABI.System.Numerics;
using Amethyst.Driver.Client;
using static Amethyst.Classes.Shared.TeachingTips;
using Quaternion = System.Numerics.Quaternion;
using Microsoft.UI.Xaml.Controls.Primitives;
using System.Diagnostics;
using System.Linq;
using Amethyst.Driver.API;
using System.Reflection;
using System.Threading;
using Amethyst.MVVM;
using System.ComponentModel;
using Amethyst.Plugins.Contract;
using Google.Protobuf.WellKnownTypes;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Pages;

/// <summary>
///     An empty page that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class Settings : Page, INotifyPropertyChanged
{
    private bool _settingsPageLoadedOnce = false;
    private List<string> _languageList = new();

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
        Shared.Settings.AutoStartCheckBox = AutoStartCheckBox;
        Shared.Settings.CheckOverlapsCheckBox = CheckOverlapsCheckBox;
        Shared.Settings.ExternalFlipCheckBoxLabel = ExternalFlipCheckBoxLabel;
        Shared.Settings.SetErrorFlyoutText = SetErrorFlyoutText;
        Shared.Settings.ExternalFlipStatusLabel = ExtFlipStatusLabel;
        Shared.Settings.ExternalFlipStackPanel = ExternalFlipStackPanel;
        Shared.Settings.JointExpanderHostStackPanel = JointExpanderHostStackPanel;
        Shared.Settings.ExternalFlipStatusStackPanel = ExtFlipStatusStackPanel;
        Shared.Settings.FlipDropDownContainer = FlipDropDownContainer;

        SettingsPage.AutoStartTeachingTip = AutoStartTeachingTip;
        SettingsPage.ManageTrackersTeachingTip = ManageTrackersTeachingTip;

        Logger.Info($"Registering settings MVVM for page: '{GetType().FullName}'...");
        DataContext = AppData.Settings; // Set this settings instance as the context

        Logger.Info("Registering a detached binary semaphore " +
                    $"reload handler for '{GetType().FullName}'...");

        Task.Run(() =>
        {
            Shared.Semaphores.ReloadSettingsPageSemaphore =
                new Semaphore(0, 1);

            while (true)
            {
                // Wait for a reload signal (blocking)
                Shared.Semaphores.ReloadSettingsPageSemaphore.WaitOne();

                // Reload & restart the waiting loop
                if (_settingsPageLoadedOnce)
                    Shared.Main.DispatcherQueue.TryEnqueue(Page_LoadedHandler);

                Task.Delay(100); // Sleep a bit
            }
        });
    }


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
        AutoStartCheckBox.IsChecked = OpenVR.Applications.GetApplicationAutoLaunch("K2VR.Amethyst");

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

        // Request page reloads
        Translator.Get.OnPropertyChanged();
        Shared.Semaphores.RequestInterfaceReload();
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

        Interfacing.InstallVrApplicationManifest(); // Just in case

        var appError = OpenVR.Applications.SetApplicationAutoLaunch("K2VR.Amethyst", true);

        if (appError != EVRApplicationError.None)
            Logger.Warn($"Amethyst manifest not installed! Error: {
                OpenVR.Applications.GetApplicationsErrorNameFromEnum(appError)}");

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.ToggleOn);
    }

    private void AutoStartCheckBox_Unchecked(object sender, RoutedEventArgs e)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;

        Interfacing.InstallVrApplicationManifest(); // Just in case

        var appError = OpenVR.Applications.SetApplicationAutoLaunch("K2VR.Amethyst", false);

        if (appError != EVRApplicationError.None)
            Logger.Warn($"Amethyst manifest not installed! Error: {
                OpenVR.Applications.GetApplicationsErrorNameFromEnum(appError)}");

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
        if (!AppData.Settings.TeachingTipShownFlip &&
            Interfacing.CurrentPageTag == "settings")
        {
            var header = Interfacing.LocalizedJsonString("/SettingsPage/Tips/FlipToggle/Header");

            // Change the tip depending on the currently connected controllers
            var controllerModel = new StringBuilder(1024);
            var error = ETrackedPropertyError.TrackedProp_Success;

            OpenVR.System.GetStringTrackedDeviceProperty(
                OpenVR.System.GetTrackedDeviceIndexForControllerRole(
                    ETrackedControllerRole.LeftHand),
                ETrackedDeviceProperty.Prop_ModelNumber_String,
                controllerModel, 1024, ref error);

            if (controllerModel.ToString().Contains("knuckles", StringComparison.OrdinalIgnoreCase) ||
                controllerModel.ToString().Contains("index", StringComparison.OrdinalIgnoreCase))
                header = header.Replace("{0}",
                    Interfacing.LocalizedJsonString("/SettingsPage/Tips/FlipToggle/Buttons/Index"));

            else if (controllerModel.ToString().Contains("vive", StringComparison.OrdinalIgnoreCase))
                header = header.Replace("{0}",
                    Interfacing.LocalizedJsonString("/SettingsPage/Tips/FlipToggle/Buttons/VIVE"));

            else if (controllerModel.ToString().Contains("mr", StringComparison.OrdinalIgnoreCase))
                header = header.Replace("{0}",
                    Interfacing.LocalizedJsonString("/SettingsPage/Tips/FlipToggle/Buttons/WMR"));

            else
                header = header.Replace("{0}",
                    Interfacing.LocalizedJsonString("/SettingsPage/Tips/FlipToggle/Buttons/Oculus"));

            ToggleFlipTeachingTip.Title = header;
            ToggleFlipTeachingTip.Subtitle =
                Interfacing.LocalizedJsonString("/SettingsPage/Tips/FlipToggle/Footer");

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
        if (AppData.Settings.TrackersVector[0].IsOrientationOverridden)
            AppData.Settings.ExternalFlipCalibrationMatrix =
                // Overriden tracker
                Quaternion.Inverse(Interfacing.VrPlayspaceOrientationQuaternion) * // VR space offset
                AppData.Settings.TrackersVector[0].Orientation; // Raw orientation

        // If it's from an external tracker
        else
            AppData.Settings.ExternalFlipCalibrationMatrix =
                Interfacing.GetVrTrackerPoseCalibrated("waist", true).Orientation;

        Logger.Info($"Captured orientation for external flip: {
            AppData.Settings.ExternalFlipCalibrationMatrix}");

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
        // Invalid for now

        //_trackerConfigFlyout = new MenuFlyout();

        //for (var index = 0; index <= (int)TrackerType.TrackerKeyboard; index++)
        //{
        //    var isAdditional = index is 0 or 3 or 4 or 10 or 11 or 12;
        //    var menuTrackerToggleItem = new ToggleMenuFlyoutItem
        //    {
        //        Text = Interfacing.LocalizedJsonString(
        //            $"/SharedStrings/Joints/Enum/{index}"),
        //        IsEnabled = isAdditional, IsChecked = !isAdditional
        //    };

        //    var currentTracker = (TrackerType)index;
        //    if (AppData.Settings.TrackersVector.Any(x => x.Role == currentTracker))
        //    {
        //        menuTrackerToggleItem.IsChecked = true;
        //        menuTrackerToggleItem.IsEnabled = true;
        //    }

        //    menuTrackerToggleItem.Click += async (parent, _) =>
        //    {
        //        // Notify of the setup end
        //        Shared.Settings.SettingsTabSetupFinished = false;

        //        // Create a new tracker / Remove the unchecked one
        //        if (parent.As<ToggleMenuFlyoutItem>().IsChecked)
        //            // If not checked, add a new tracker
        //            AppData.Settings.TrackersVector.Add(new AppTracker
        //            {
        //                Role = currentTracker,
        //                Serial = TypeUtils.TrackerTypeRoleSerialDictionary[currentTracker]
        //            });

        //        else // If the tracker was unchecked
        //            for (var t = 0; t < AppData.Settings.TrackersVector.Count; t++)
        //                if (AppData.Settings.TrackersVector[t].Role == currentTracker)
        //                {
        //                    // Make actual changes
        //                    if (AppData.Settings.TrackersVector[t].IsActive && Interfacing.AppTrackersInitialized)
        //                        DriverClient.UpdateTrackerStates(new List<(TrackerType Role, bool State)>
        //                        {
        //                            (AppData.Settings.TrackersVector[t].Role, false)
        //                        });

        //                    await Task.Delay(20);
        //                    AppData.Settings.TrackersVector.RemoveAt(t);

        //                    // If the tracker was on and then removed
        //                    if (AppData.Settings.TrackersVector[t].IsActive)
        //                    {
        //                        // Boiler
        //                        Shared.Settings.SettingsTabSetupFinished = true;

        //                        // Show the notifications and rebuild
        //                        TrackingDevices.TrackersConfigChanged();

        //                        // Boiler end
        //                        Shared.Settings.SettingsTabSetupFinished = false;
        //                    }

        //                    // Save settings
        //                    AppData.Settings.SaveSettings();
        //                }

        //        // TODO Rebuild the joint expander stack

        //        TrackingDevices.CheckFlipSupport();

        //        // Notify of the setup end
        //        Shared.Settings.SettingsTabSetupFinished = true;
        //        AppData.Settings.SaveSettings();

        //        // Check if any trackers are enabled
        //        if (!AppData.Settings.TrackersVector.Any(x => x.IsActive))
        //        {
        //            Logger.Warn("All trackers have been disabled, force-enabling the waist tracker!");
        //            AppData.Settings.TrackersVector[0].IsActive = true;

        //            // Save settings
        //            AppData.Settings.SaveSettings();
        //        }

        //        Logger.Info("Requesting a check for already-added trackers...");
        //        Interfacing.AlreadyAddedTrackersScanRequested = true;
        //    };

        //    // Append the item
        //    _trackerConfigFlyout.Items.Add(menuTrackerToggleItem);
        //}

        ////     var menuPairsToggleItem = new ToggleMenuFlyoutItem
        ////     {
        ////         Text = Interfacing.LocalizedJsonString("/SettingsPage/Captions/TrackerPairs"),
        ////         IsChecked = AppData.Settings.UseTrackerPairs
        ////     };

        ////     menuPairsToggleItem.Click += (_, _) =>
        ////     {
        ////Shared.Settings.SettingsTabSetupFinished = false;
        ////         AppData.Settings.UseTrackerPairs = sender.As<ToggleMenuFlyoutItem>().IsChecked;

        ////         // TODO tracker pairs

        ////TrackingDevices.CheckFlipSupport();

        ////         // Notify of the setup end
        ////         Shared.Settings.SettingsTabSetupFinished = true;
        ////         AppData.Settings.SaveSettings();
        ////         AppData.Settings.ReadSettings(); // Config check
        ////     };

        //// Append the item
        ////_trackerConfigFlyout.Items.Add(new MenuFlyoutSeparator());
        ////_trackerConfigFlyout.Items.Add(menuPairsToggleItem);

        //_trackerConfigFlyout.Placement = FlyoutPlacementMode.LeftEdgeAlignedBottom;
        //_trackerConfigFlyout.ShowMode = FlyoutShowMode.Transient;
        //_trackerConfigFlyout.ShowAt(TrackerConfigButton);

        //// Play a sound
        //AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);

        //_trackerConfigFlyout.Closing += (_, _) =>
        //{
        //    // Play a sound
        //    AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
        //};
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
        DriverClient.RequestVrRestart("SteamVR needs to be restarted to enable/disable trackers properly.");

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
        Interfacing.ShowVrToast(
            Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed/Title"),
            Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed"));
    }

    private void ReRegisterButton_Click(object sender, RoutedEventArgs e)
    {
        if (File.Exists(Path.Combine(Interfacing.GetProgramLocation().DirectoryName,
                "K2CrashHandler", "K2CrashHandler.exe")))
        {
            Process.Start(Path.Combine(Interfacing.GetProgramLocation().DirectoryName,
                "K2CrashHandler", "K2CrashHandler.exe"));
        }
        else
        {
            Logger.Warn("Crash handler exe (./K2CrashHandler/K2CrashHandler.exe) not found!");

            Shared.Settings.SetErrorFlyoutText.Text =
                Interfacing.LocalizedJsonString("/SettingsPage/ReRegister/Error/NotFound");

            SetErrorFlyout.ShowAt(ReRegisterButton, new FlyoutShowOptions
            {
                Placement = FlyoutPlacementMode.RightEdgeAlignedBottom
            });
        }

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
    }

    private void ViewLogsButton_Click(object sender, RoutedEventArgs e)
    {
        SystemShell.OpenFolderAndSelectItem(Logger.LogFilePath);

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
    }

    private void ReManifestButton_Click(object sender, RoutedEventArgs e)
    {
        switch (Interfacing.InstallVrApplicationManifest())
        {
            // Not found failure
            case 0:
            {
                Shared.Settings.SetErrorFlyoutText.Text =
                    Interfacing.LocalizedJsonString("/SettingsPage/ReManifest/Error/NotFound");

                SetErrorFlyout.ShowAt(ReManifestButton, new FlyoutShowOptions
                {
                    Placement = FlyoutPlacementMode.RightEdgeAlignedBottom
                });
                break;
            }
            // Generic success
            case 1:
                break;
            // SteamVR failure
            case 2:
            {
                Shared.Settings.SetErrorFlyoutText.Text =
                    Interfacing.LocalizedJsonString("/SettingsPage/ReManifest/Error/Other");

                SetErrorFlyout.ShowAt(ReManifestButton, new FlyoutShowOptions
                {
                    Placement = FlyoutPlacementMode.RightEdgeAlignedBottom
                });
                break;
            }
        }

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
    }

    private void DismissSetErrorButton_Click(object sender, RoutedEventArgs e)
    {
        SetErrorFlyout.Hide();
    }

    private async void TrackerToggleSwitch_Toggled(object sender, RoutedEventArgs e)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;
        var context = (sender as ToggleSwitch)!.DataContext;

        // Don't react to pre-init signals
        if (context is not AppTracker tracker) return;
        if (Interfacing.AppTrackersInitialized)
            // try 3 times cause why not
            for (var i = 0; i < 3; i++)
            {
                // Update status in server
                await DriverClient.UpdateTrackerStates(new List<(TrackerType Role, bool State)>
                    { (tracker!.Role, (sender as ToggleSwitch)!.IsOn) });
                await Task.Delay(20);
            }

        // Check if any trackers are enabled
        if (!(sender as ToggleSwitch)!.IsOn && !AppData.Settings.TrackersVector
                .Where(x => x.Role != tracker!.Role).Any(x => x.IsActive))
        {
            Logger.Warn("All trackers (except this one) have been disabled, force-re-enabling!");
            tracker!.IsActive = true; // Force re-enable this tracker
            tracker.OnPropertyChanged("IsActive");
        }

        TrackingDevices.TrackersConfigChanged();

        // Play a sound
        AppSounds.PlayAppSound((sender as ToggleSwitch)!.IsOn
            ? AppSounds.AppSoundType.ToggleOn
            : AppSounds.AppSoundType.ToggleOff);

        // Request a check for already-added trackers
        Logger.Info("Requesting a check for already-added trackers...");
        Interfacing.AlreadyAddedTrackersScanRequested = true;
        AppData.Settings.SaveSettings();
    }

    private void TrackerExpander_Expanding(Expander sender, ExpanderExpandingEventArgs args)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void TrackerExpander_Collapsed(Expander sender, ExpanderCollapsedEventArgs args)
    {
        // Don't react to pre-init signals
        if (!Shared.Settings.SettingsTabSetupFinished) return;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }

    private void TrackerPositionFilterOptionBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if ((sender as ComboBox)!.SelectedIndex < 0)
            (sender as ComboBox)!.SelectedItem = e.RemovedItems[0];
    }

    private void TrackerOrientationFilterOptionBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if ((sender as ComboBox)!.SelectedIndex < 0)
            (sender as ComboBox)!.SelectedItem = e.RemovedItems[0];
    }

    // MVVM stuff
    public event PropertyChangedEventHandler PropertyChanged;

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    public List<AppTrackerEntry> ToggleableTrackerEntries => System.Enum.GetValues<TrackerType>()
        // Pick all non-default (additional) trackers
        .Where(x => x is not TrackerType.TrackerWaist and not
            TrackerType.TrackerLeftFoot and not TrackerType.TrackerRightFoot and not
            TrackerType.TrackerLeftKnee and not TrackerType.TrackerRightKnee and not
            TrackerType.TrackerLeftElbow and not TrackerType.TrackerRightElbow)
        // Convert to an entry, passing the filtered tracker role
        .Select(x => new AppTrackerEntry { TrackerRole = x }).ToList();

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
                await DriverClient.UpdateTrackerStates(new List<(TrackerType Role, bool State)>
                        { (removedTracker.Role, false) });

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
        Shared.Semaphores.RequestInterfaceReload();

        Logger.Info("Requesting a check for already-added trackers...");
        Interfacing.AlreadyAddedTrackersScanRequested = true;
        ToggleTrackersFlyout.Hide(); // Hide the flyout
    }

    private void PairsToggleMenuFlyoutItem_Click(object sender, RoutedEventArgs e)
    {
    }
}