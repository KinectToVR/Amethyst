// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Windows.System;
using Amethyst.Classes;
using Amethyst.Controls;
using Amethyst.MVVM;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Media.Animation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Pages;

/// <summary>
///     An empty page that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class Devices : Page, INotifyPropertyChanged
{
    private bool _devicesPageLoadedOnce;
    public static bool DisableJointExpanderSounds = false;

    public Devices()
    {
        InitializeComponent();

        Logger.Info($"Constructing page: '{GetType().FullName}'...");

        Shared.Devices.DeviceNameLabel = SelectedDeviceNameLabel;
        Shared.Devices.DeviceStatusLabel = TrackingDeviceStatusLabel;
        Shared.Devices.ErrorWhatText = ErrorWhatText;
        Shared.Devices.TrackingDeviceErrorLabel = TrackingDeviceErrorLabel;
        Shared.Devices.DeviceErrorGrid = DeviceErrorGrid;
        Shared.Devices.TrackingDeviceChangePanel = TrackingDeviceChangePanel;
        Shared.Devices.DevicesTreeView = TrackingDeviceTreeView;
        Shared.Devices.SetAsOverrideButton = SetAsOverrideButton;
        Shared.Devices.SetAsBaseButton = SetAsBaseButton;
        Shared.Devices.DeselectDeviceButton = DeselectDeviceButton;
        Shared.Devices.SelectedDeviceSettingsRootLayoutPanel = SelectedDeviceSettingsRootLayoutPanel;
        Shared.Devices.DevicesMainContentGridOuter = DevicesMainContentGridOuter;
        Shared.Devices.DevicesMainContentGridInner = DevicesMainContentGridInner;

        Shared.TeachingTips.DevicesPage.DeviceControlsTeachingTip = DeviceControlsTeachingTip;
        Shared.TeachingTips.DevicesPage.DevicesListTeachingTip = DevicesListTeachingTip;

        Logger.Info($"Registering devices MVVM for page: '{GetType().FullName}'...");
        TrackingDeviceTreeView.ItemsSource = AppPlugins.TrackingDevicesList.Values;

        ResubscribeListeners(); // Register for any pending changes
        Interfacing.AppSettingsRead += (_, _) => ResubscribeListeners();

        // Set currently tracking device & selected device
        Logger.Info("Overwriting the devices TreeView selected item...");
        var devicesListIndex = AppPlugins.TrackingDevicesList.Keys.ToList()
            .IndexOf(AppData.Settings.TrackingDeviceGuid);
        TrackingDeviceTreeView.SelectedNode = TrackingDeviceTreeView.RootNodes[devicesListIndex];

        AppData.Settings.SelectedTrackingDeviceGuid = AppData.Settings.TrackingDeviceGuid;
        AppData.Settings.PreviousSelectedTrackingDeviceGuid = AppData.Settings.TrackingDeviceGuid;

        Logger.Info("Registering a detached binary semaphore " +
                    $"reload handler for '{GetType().FullName}'...");

        Task.Run(() =>
        {
            Shared.Events.ReloadDevicesPageEvent =
                new ManualResetEvent(false);

            while (true)
            {
                // Wait for a reload signal (blocking)
                Shared.Events.ReloadDevicesPageEvent.WaitOne();

                // Reload & restart the waiting loop
                if (_devicesPageLoadedOnce && Interfacing.CurrentPageTag == "devices")
                    Shared.Main.DispatcherQueue.TryEnqueue(
                        async () => { await Page_LoadedHandler(); });

                // Reset the event
                Shared.Events.ReloadDevicesPageEvent.Reset();
            }
        });
    }

    // Waist, LR Foot
    public List<AppTracker> BasicTrackers => AppData.Settings.TrackersVector.ToArray()[..3].ToList();

    // LR Knee, LR Elbow
    public List<AppTracker> StandardTrackers => AppData.Settings.TrackersVector.ToArray()[3..7].ToList();

    // Everything else
    public List<AppTracker> AdditionalTrackers => AppData.Settings.TrackersVector.Count > 6
        ? AppData.Settings.TrackersVector.ToArray()[7..].ToList()
        : new List<AppTracker>(); // Dummy list with no items (at least not null)

    // MVVM stuff
    public event PropertyChangedEventHandler PropertyChanged;

    private void ResubscribeListeners()
    {
        //AppData.Settings.PropertyChanged += (_, _) => OnPropertyChanged();
        AppData.Settings.TrackersVector.CollectionChanged += (_, _) => OnPropertyChanged();
    }

    private async void Page_Loaded(object sender, RoutedEventArgs e)
    {
        Logger.Info($"Re/Loading page: '{GetType().FullName}'...");
        Interfacing.CurrentAppState = "devices";

        // Execute the handler
        await Page_LoadedHandler();

        // Mark as loaded
        _devicesPageLoadedOnce = true;
    }

    private async Task Page_LoadedHandler()
    {
        // Reconnect and update the status here
        await Shared.Devices.ReloadSelectedDevice(true);

        // Notify of the setup's end
        OnPropertyChanged(); // Just everything
        Shared.Devices.DevicesTabSetupFinished = true;

        await Task.Delay(10);
        WaistAndFeetTrackersExpander.InvalidateMeasure();
        KneesAndElbowsTrackersExpander.InvalidateMeasure();
        AdditionalTrackersExpander.InvalidateMeasure();

        WaistAndFeetOverridesExpander.InvalidateMeasure();
        KneesAndElbowsOverridesExpander.InvalidateMeasure();
        AdditionalOverridesExpander.InvalidateMeasure();
    }

    private async void DevicesListTeachingTip_ActionButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Close the current tip
        DevicesListTeachingTip.IsOpen = false;
        await Task.Delay(400);

        // Reset the next page layout (if ever changed)
        Shared.Settings.PageMainScrollViewer?.ScrollToVerticalOffset(0);

        // Navigate to the settings page
        Shared.Main.MainNavigationView.SelectedItem =
            Shared.Main.MainNavigationView.MenuItems[1];

        Shared.Main.NavigateToPage("settings",
            new EntranceNavigationTransitionInfo());

        await Task.Delay(500);

        // Check whether the previous tip can be shown
        if (AppPlugins.CurrentServiceEndpoint.CanAutoStartAmethyst)
        {
            // Show the previous tip: auto-start
            Shared.TeachingTips.SettingsPage.AutoStartTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
            Shared.TeachingTips.SettingsPage.AutoStartTeachingTip.IsOpen = true;
        }
        else
        {
            // Show the previous tip: managing trackers
            Shared.Settings.PageMainScrollViewer?.UpdateLayout();
            Shared.Settings.PageMainScrollViewer?.ChangeView(null,
                Shared.Settings.PageMainScrollViewer.ExtentHeight / 3.0, null);

            await Task.Delay(500);

            Shared.TeachingTips.SettingsPage.AddTrackersTeachingTip.TailVisibility =
                TeachingTipTailVisibility.Collapsed;
            Shared.TeachingTips.SettingsPage.AddTrackersTeachingTip.IsOpen = true;
        }
    }

    private void DevicesListTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        DeviceStatusTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        DeviceStatusTeachingTip.IsOpen = true;
    }

    private async void TrackingDeviceTreeView_ItemInvoked(TreeView sender, TreeViewItemInvokedEventArgs args)
    {
        if (!Shared.Devices.DevicesTabSetupFinished) return; // Block dummy selects
        args.Handled = true; // Tell XAML we're currently working on the TreeView
        Shared.Devices.DevicesJointsValid = false; // Don't signal on device selection

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
        var node = args.InvokedItem as TrackingDevice;

        // Collapse
        DisableJointExpanderSounds = true;
        if (node?.Guid != AppData.Settings.PreviousSelectedTrackingDeviceGuid)
        {
            // Collapse all expanders if the update was not triggered manually
            Shared.Events.RequestJointSelectorExpandersCollapseEvent?.Invoke(this, EventArgs.Empty);
            Shared.Events.RequestOverrideExpandersCollapseEvent?.Invoke(this, EventArgs.Empty);

            TrackingDeviceTreeView.IsHitTestVisible = false; // Mark as working on UI
            await Task.Delay(300); // Wait a moment until they actually collapse
            TrackingDeviceTreeView.IsHitTestVisible = true; // Mark as done
        }

        AppData.Settings.SelectedTrackingDeviceGuid = node?.Guid;
        Logger.Info($"Selected device: ({node!.Name}, {node!.Guid})");

        // Request a reload from most of the internal app components
        AppData.Settings.TrackersVector.ToList().ForEach(x => x.OnPropertyChanged());

        // Reload the tracking device UI (no animations if unchanged)
        await Shared.Devices.ReloadSelectedDevice(
            AppData.Settings.SelectedTrackingDeviceGuid == AppData.Settings.PreviousSelectedTrackingDeviceGuid);

        // Backup
        AppData.Settings.PreviousSelectedTrackingDeviceGuid = AppData.Settings.SelectedTrackingDeviceGuid;

        await Task.Delay(10);
        WaistAndFeetTrackersExpander.InvalidateMeasure();
        KneesAndElbowsTrackersExpander.InvalidateMeasure();
        AdditionalTrackersExpander.InvalidateMeasure();
        DevicesJointsSelectorStackPanel.InvalidateMeasure();

        WaistAndFeetOverridesExpander.InvalidateMeasure();
        KneesAndElbowsOverridesExpander.InvalidateMeasure();
        AdditionalOverridesExpander.InvalidateMeasure();
        DevicesOverridesSelectorStackPanel.InvalidateMeasure();

        // We're done here
        AppData.Settings.TrackersVector.ToList().ForEach(x => x.OnPropertyChanged());
        Shared.Devices.DevicesJointsValid = true;
        DisableJointExpanderSounds = false;
        args.Handled = false; // Tell XAML we've finished now
    }

    private void DeviceStatusTeachingTip_ActionButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Close the current tip
        DeviceStatusTeachingTip.IsOpen = false;

        // Show the previous one
        DevicesListTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        DevicesListTeachingTip.IsOpen = true;
    }

    private void DeviceStatusTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        DeviceControlsTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        DeviceControlsTeachingTip.IsOpen = true;
    }

    private async void ReconnectDeviceButton_Click(SplitButton sender, SplitButtonClickEventArgs args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Reload the tracking device
        await Shared.Devices.ReloadSelectedDevice(true, true);

        // Check the application config, save
        AppData.Settings.CheckSettings();
        AppData.Settings.SaveSettings();

        // Update the page UI
        OnPropertyChanged();
        await Page_LoadedHandler();
    }

    private void DeviceControlsTeachingTip_ActionButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Close the current tip
        DeviceControlsTeachingTip.IsOpen = false;

        // Show the previous one
        DeviceStatusTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        DeviceStatusTeachingTip.IsOpen = true;
    }

    private async void DeviceControlsTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
        await Task.Delay(200);

        // Navigate to the info page
        Shared.Main.MainNavigationView.SelectedItem =
            Shared.Main.MainNavigationView.MenuItems[3];

        Shared.Main.NavigateToPage("info",
            new EntranceNavigationTransitionInfo());

        await Task.Delay(500);

        // Show the next tip
        Shared.TeachingTips.InfoPage.HelpTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        Shared.TeachingTips.InfoPage.HelpTeachingTip.IsOpen = true;
    }

    private void DisconnectDeviceButton_Click(object sender, RoutedEventArgs e)
    {
        Logger.Info($"Now disconnecting tracking device {AppData.Settings.SelectedTrackingDeviceGuid}...");

        if (AppPlugins.TrackingDevicesList.TryGetValue(
                AppData.Settings.SelectedTrackingDeviceGuid, out var device))
            device.Shutdown();

        // Update the status here
        AppPlugins.HandleDeviceRefresh(false);

        // Update other statuses
        AppPlugins.UpdateTrackingDevicesInterface();
        AlternativeConnectionOptionsFlyout.Hide();

        // Update the page UI
        OnPropertyChanged();
    }

    private void SetAsBaseButton_Click(object sender, RoutedEventArgs e)
    {
        var device = AppPlugins.GetDevice(AppData.Settings.SelectedTrackingDeviceGuid).Device;
        if (device?.TrackedJoints is null || device.TrackedJoints.Count < 1)
        {
            NoJointsFlyout.ShowAt(SelectedDeviceNameLabel);
            return; // Better give up on this one
        }

        // Stop the pose composer for now
        lock (Interfacing.UpdateLock)
        {
            // Remove an override if exists and overlaps
            if (device.IsOverride) AppData.Settings.OverrideDevicesGuidMap.Remove(device.Guid);

            // Block interface interactions
            Shared.Devices.DevicesJointsValid = false;

            // Setup the new base: change the GUID
            Logger.Info($"Changing the current tracking device (Base) to: ({device.Name}, {device.Guid})");
            AppData.Settings.TrackingDeviceGuid = device.Guid;

            // Setup the new base: prepare the config
            Logger.Info("Checking the configuration before continuing...");
            AppData.Settings.CheckSettings(false, device);

            Logger.Info("Requesting internal updates from all of the linked components...");
            AppData.Settings.TrackersVector.ToList().ForEach(x => x.OnPropertyChanged());

            // Update the status here
            Logger.Info("Updating the device status and management interface...");
            Shared.Devices.ReloadSelectedDeviceSync(true);

            // Make all the devices refresh their props
            Logger.Info("Refreshing shared properties of all other devices...");
            AppPlugins.TrackingDevicesList.ToList()
                .ForEach(x => x.Value.OnPropertyChanged());

            // Check the application config, save
            Logger.Info("Saving the configuration...");
            AppData.Settings.CheckSettings();
            AppData.Settings.SaveSettings();

            // Unblock config checking from XAML
            Shared.Devices.DevicesJointsValid = true;
            Logger.Info($"Changed the current tracking device (Base) to: ({device.Name}, {device.Guid})");
        }

        SetDeviceTypeFlyout.Hide(); // Hide the flyout
        OnPropertyChanged(); // Update the page UI
    }

    private void SetAsOverrideButton_Click(object sender, RoutedEventArgs e)
    {
        var device = AppPlugins.GetDevice(AppData.Settings.SelectedTrackingDeviceGuid).Device;
        if (device?.TrackedJoints is null || device.TrackedJoints.Count < 1)
        {
            NoJointsFlyout.ShowAt(SelectedDeviceNameLabel);
            return; // Better give up on this one
        }

        // Stop the pose composer for now
        lock (Interfacing.UpdateLock)
        {
            // Block interface interactions
            Shared.Devices.DevicesJointsValid = false;

            // Setup the new override
            Logger.Info($"Setting up a new override device: ({device.Name}, {device.Guid})");
            AppData.Settings.OverrideDevicesGuidMap.Add(device.Guid);

            // Additionally check settings
            Logger.Info("Checking the configuration before continuing...");
            AppData.Settings.CheckSettings();

            // Update the status here
            Logger.Info("Updating the device status and management interface...");
            Shared.Devices.ReloadSelectedDeviceSync(true);

            // Make all the devices refresh their props
            Logger.Info("Refreshing shared properties of all other devices...");
            AppPlugins.TrackingDevicesList.ToList()
                .ForEach(x => x.Value.OnPropertyChanged());

            // Check the application config, save
            Logger.Info("Saving the configuration...");
            AppData.Settings.CheckSettings();
            AppData.Settings.SaveSettings();

            // Unblock config checking from XAML
            Shared.Devices.DevicesJointsValid = true;
            Logger.Info($"Set up a new override device: ({device.Name}, {device.Guid})");
        }

        SetDeviceTypeFlyout.Hide(); // Hide the flyout
        OnPropertyChanged(); // Update the page UI
    }

    private void DismissOverrideTipNoJointsButton_Click(object sender, RoutedEventArgs e)
    {
        NoJointsFlyout.Hide();
    }

    private void DeselectDeviceButton_Click(object sender, RoutedEventArgs e)
    {
        var device = AppPlugins.GetDevice(AppData.Settings.SelectedTrackingDeviceGuid).Device;
        if (device?.TrackedJoints is null || device.TrackedJoints.Count < 1)
        {
            NoJointsFlyout.ShowAt(SelectedDeviceNameLabel);
            return; // Better give up on this one
        }

        Logger.Info($"Deselecting tracking device ({device.Name}, {device.Guid})...");

        // Stop the pose composer for now
        lock (Interfacing.UpdateLock)
        {
            // Block interface interactions
            Shared.Devices.DevicesJointsValid = false;

            // Deselect the device, update status
            Logger.Info($"Removing ({device.Name}, {device.Guid}) from the shared override map...");
            AppData.Settings.OverrideDevicesGuidMap.Remove(device.Guid);

            // Update the status here
            Logger.Info("Updating the device status and management interface...");
            AppPlugins.HandleDeviceRefresh(false);

            // Additionally check settings
            Logger.Info("Checking the configuration before continuing...");
            AppData.Settings.CheckSettings();

            // Make all the devices refresh their props
            Logger.Info("Refreshing shared properties of all other devices...");
            AppPlugins.TrackingDevicesList.ToList()
                .ForEach(x => x.Value.OnPropertyChanged());

            // Update other statuses
            Logger.Info("Updating the device status and management interface...");
            AppPlugins.UpdateTrackingDevicesInterface();
            AlternativeConnectionOptionsFlyout.Hide();

            // Check the application config, save
            Logger.Info("Saving the configuration...");
            AppData.Settings.CheckSettings();
            AppData.Settings.SaveSettings();

            // Unblock config checking from XAML
            Shared.Devices.DevicesJointsValid = true;
            Logger.Info($"Removed an override device: ({device.Name}, {device.Guid})");
        }

        SetAsOverrideButton.IsEnabled = true;
        SetAsBaseButton.IsEnabled = true;
        DeselectDeviceButton.Visibility = Visibility.Collapsed;

        // Update the page UI
        OnPropertyChanged();
    }

    private async void OpenDocsButton_Click(object sender, RoutedEventArgs e)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        try
        {
            // Launch passed device docs
            await Launcher.LaunchUriAsync(
                AppPlugins.TrackingDevicesList[AppData.Settings.SelectedTrackingDeviceGuid].ErrorDocsUri ??
                new Uri($"https://docs.k2vr.tech/{Interfacing.DocsLanguageCode}/app/help/"));
        }
        catch (Exception ex)
        {
            Logger.Error(new InvalidOperationException($"Couldn't launch device docs! Message: {ex.Message}"));
        }
    }

    private async void OpenDiscordButton_Click(object sender, RoutedEventArgs e)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        await Launcher.LaunchUriAsync(new Uri("https://discord.gg/YBQCRDG"));
    }

    private void MenuFlyout_Opening(object sender, object e)
    {
        // Don't react to pre-init signals
        if (!Shared.Devices.DevicesTabSetupFinished) return;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void MenuFlyout_Closing(FlyoutBase sender,
        FlyoutBaseClosingEventArgs args)
    {
        // Don't react to pre-init signals
        if (!Shared.Devices.DevicesTabSetupFinished) return;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    public Visibility CombineVisibility(Visibility v1, Visibility v2, Visibility v3)
    {
        return new[] { v1, v2, v3 }.Contains(Visibility.Visible) ? Visibility.Visible : Visibility.Collapsed;
    }
}