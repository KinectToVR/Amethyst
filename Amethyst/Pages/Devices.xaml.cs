// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;
using Amethyst.Classes;
using Amethyst.Utils;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Media.Animation;
using Windows.System;
using Microsoft.UI.Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Pages;

/// <summary>
///     An empty page that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class Devices : Page, INotifyPropertyChanged
{
    private bool _devicesPageLoadedOnce = false;

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
        Shared.Devices.DevicesMainContentGridOuter = DevicesMainContentGridOuter;
        Shared.Devices.DevicesMainContentGridInner = DevicesMainContentGridInner;
        Shared.Devices.SelectedDeviceSettingsHostContainer = SelectedDeviceSettingsHostContainer;
        Shared.Devices.DevicesTreeView = TrackingDeviceTreeView;
        Shared.Devices.PluginsItemsRepeater = PluginsItemsRepeater;
        Shared.Devices.NoJointsFlyout = NoJointsFlyout;
        Shared.Devices.SetDeviceTypeFlyout = SetDeviceTypeFlyout;
        Shared.Devices.SetAsOverrideButton = SetAsOverrideButton;
        Shared.Devices.SetAsBaseButton = SetAsBaseButton;
        Shared.Devices.DeselectDeviceButton = DeselectDeviceButton;
        Shared.Devices.DevicesMainContentScrollViewer = DevicesMainContentScrollViewer;
        Shared.Devices.DevicesOverridesSelectorStackPanel = DevicesOverridesSelectorStackPanel;
        Shared.Devices.DevicesJointSelectorStackPanel = DevicesJointsSelectorStackPanel;
        Shared.Devices.JointExpanderHostStackPanel = JointsExpanderHostStackPanel;
        Shared.Devices.SelectedDeviceSettingsRootLayoutPanel = SelectedDeviceSettingsRootLayoutPanel;
        Shared.Devices.OverridesExpanderHostStackPanel = OverridesExpanderHostStackPanel;

        Shared.TeachingTips.DevicesPage.DeviceControlsTeachingTip = DeviceControlsTeachingTip;
        Shared.TeachingTips.DevicesPage.DevicesListTeachingTip = DevicesListTeachingTip;

        Logger.Info($"Registering devices MVVM for page: '{GetType().FullName}'...");
        TrackingDeviceTreeView.ItemsSource = TrackingDevices.TrackingDevicesList.Values;

        Logger.Info($"Registering plugins MVVM for page: '{GetType().FullName}'...");
        PluginsItemsRepeater.ItemsSource = TrackingDevices.LoadAttemptedTrackingDevicesVector;

        //Logger.Info($"Registering joint selectors MVVM for page: '{GetType().FullName}'...");
        //WaistAndFeetTrackersExpander.PropertyChangedEvent += (_, _) => { OnPropertyChanged(); };
        //KneesAndElbowsTrackersExpander.PropertyChangedEvent += (_, _) => { OnPropertyChanged(); };
        //AdditionalTrackersExpander.PropertyChangedEvent += (_, _) => { OnPropertyChanged(); };

        //Logger.Info($"Registering override selectors MVVM for page: '{GetType().FullName}'...");
        //WaistAndFeetOverridesExpander.PropertyChangedEvent += (_, _) => { OnPropertyChanged(); };
        //KneesAndElbowsOverridesExpander.PropertyChangedEvent += (_, _) => { OnPropertyChanged(); };
        //AdditionalOverridesExpander.PropertyChangedEvent += (_, _) => { OnPropertyChanged(); };

        // Set currently tracking device & selected device
        Logger.Info("Overwriting the devices TreeView selected item...");
        var devicesListIndex = TrackingDevices.TrackingDevicesList.Keys.ToList()
            .IndexOf(AppData.Settings.TrackingDeviceGuid);
        TrackingDeviceTreeView.SelectedNode = TrackingDeviceTreeView.RootNodes[devicesListIndex];

        AppData.Settings.SelectedTrackingDeviceGuid = AppData.Settings.TrackingDeviceGuid;
        AppData.Settings.PreviousSelectedTrackingDeviceGuid = AppData.Settings.TrackingDeviceGuid;

        Logger.Info("Registering a detached binary semaphore " +
                    $"reload handler for '{GetType().FullName}'...");

        Task.Run(() =>
        {
            Shared.Semaphores.ReloadDevicesPageSemaphore =
                new Semaphore(0, 1);

            while (true)
            {
                // Wait for a reload signal (blocking)
                Shared.Semaphores.ReloadDevicesPageSemaphore.WaitOne();

                // Reload & restart the waiting loop
                if (_devicesPageLoadedOnce)
                    Shared.Main.DispatcherQueue.TryEnqueue(
                        async () => { await Page_LoadedHandler(); });

                Task.Delay(100); // Sleep a bit
            }
        });
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

        // Show the next tip
        Shared.TeachingTips.SettingsPage.AutoStartTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        Shared.TeachingTips.SettingsPage.AutoStartTeachingTip.IsOpen = true;
    }

    private void DevicesListTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        DeviceStatusTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        DeviceStatusTeachingTip.IsOpen = true;
    }

    private void PluginManagerFlyout_Opening(object sender, object e)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);

        Shared.Main.InterfaceBlockerGrid.Opacity = 0.35;
        Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = true;

        Interfacing.IsNuxPending = true;
    }

    private void PluginManagerFlyout_Closing(FlyoutBase sender, FlyoutBaseClosingEventArgs args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }

    private void PluginManagerFlyout_Closed(object sender, object e)
    {
        Shared.Main.InterfaceBlockerGrid.Opacity = 0.0;
        Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = false;

        Interfacing.IsNuxPending = false;
    }

    private async void TrackingDeviceTreeView_ItemInvoked(TreeView sender, TreeViewItemInvokedEventArgs args)
    {
        if (!Shared.Devices.DevicesTabSetupFinished) return; // Block dummy selects
        Shared.Devices.DevicesSignalJoints = false; // Don't signal on device selection

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        var node = args.InvokedItem as MVVM.TrackingDevice;
        AppData.Settings.SelectedTrackingDeviceGuid = node?.Guid;

        Logger.Info($"Selected device: ({node!.Name}, {node!.Guid})");

        // Reload the tracking device UI (no animations if unchanged)
        await Shared.Devices.ReloadSelectedDevice(
            AppData.Settings.SelectedTrackingDeviceGuid == AppData.Settings.PreviousSelectedTrackingDeviceGuid);

        // Backup
        AppData.Settings.PreviousSelectedTrackingDeviceGuid = AppData.Settings.SelectedTrackingDeviceGuid;
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
        await Shared.Devices.ReloadSelectedDevice(true);

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

        if (TrackingDevices.TrackingDevicesList.TryGetValue(
                AppData.Settings.SelectedTrackingDeviceGuid, out var device))
            device.Shutdown();

        // Update the status here
        TrackingDevices.HandleDeviceRefresh(false);

        // Update other statuses
        TrackingDevices.UpdateTrackingDevicesInterface();
        AlternativeConnectionOptionsFlyout.Hide();

        // Update the page UI
        OnPropertyChanged();
    }

    private async void SetAsBaseButton_Click(object sender, RoutedEventArgs e)
    {
        var device = TrackingDevices.GetDevice(AppData.Settings.SelectedTrackingDeviceGuid).Device;
        if (device?.TrackedJoints is null || device.TrackedJoints.Count < 1)
        {
            NoJointsFlyout.ShowAt(SelectedDeviceNameLabel);
            return; // Better give up on this one
        }

        // Setup the new base
        AppData.Settings.TrackingDeviceGuid = device.Guid;

        // Remove an override if exists and overlaps
        if (device.IsOverride) AppData.Settings.OverrideDevicesGuidMap.Remove(device.Guid);
        Logger.Info($"Changed the current tracking device (Base) to: ({device.Name}, {device.Guid})");

        // Update the status here
        await Shared.Devices.ReloadSelectedDevice(true);

        // Check the application config, save
        AppData.Settings.CheckSettings();
        AppData.Settings.SaveSettings();

        SetDeviceTypeFlyout.Hide(); // Hide the flyout
        OnPropertyChanged(); // Update the page UI
    }

    private async void SetAsOverrideButton_Click(object sender, RoutedEventArgs e)
    {
        var device = TrackingDevices.GetDevice(AppData.Settings.SelectedTrackingDeviceGuid).Device;
        if (device?.TrackedJoints is null || device.TrackedJoints.Count < 1)
        {
            NoJointsFlyout.ShowAt(SelectedDeviceNameLabel);
            return; // Better give up on this one
        }

        // Setup the new override
        AppData.Settings.OverrideDevicesGuidMap.Add(device.Guid);
        Logger.Info($"Changed the current tracking device (Base) to: ({device.Name}, {device.Guid})");

        // Update the status here
        await Shared.Devices.ReloadSelectedDevice(true);

        // Check the application config, save
        AppData.Settings.CheckSettings();
        AppData.Settings.SaveSettings();

        SetDeviceTypeFlyout.Hide(); // Hide the flyout
        OnPropertyChanged(); // Update the page UI
    }

    private void DismissOverrideTipNoJointsButton_Click(object sender, RoutedEventArgs e)
    {
        NoJointsFlyout.Hide();
    }

    private void DeselectDeviceButton_Click(object sender, RoutedEventArgs e)
    {
        Logger.Info($"Now disconnecting tracking device {AppData.Settings.SelectedTrackingDeviceGuid}...");

        SetAsOverrideButton.IsEnabled = true;
        SetAsBaseButton.IsEnabled = true;
        DeselectDeviceButton.Visibility = Visibility.Collapsed;

        // Deselect the device
        AppData.Settings.OverrideDevicesGuidMap.Remove(AppData.Settings.SelectedTrackingDeviceGuid);

        // Update the status here
        TrackingDevices.HandleDeviceRefresh(false);

        // Update other statuses
        TrackingDevices.UpdateTrackingDevicesInterface();
        AlternativeConnectionOptionsFlyout.Hide();

        // Check the application config, save
        AppData.Settings.CheckSettings();
        AppData.Settings.SaveSettings();

        // Update the page UI
        OnPropertyChanged();
    }

    private async void OpenDocsButton_Click(object sender, RoutedEventArgs e)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        var deviceErrorString = TrackingDeviceErrorLabel.Text;
        var deviceName = SelectedDeviceNameLabel.Text;

        switch (deviceName)
        {
            case "Xbox 360 Kinect":
                await Launcher.LaunchUriAsync(new Uri(deviceErrorString switch
                {
                    "E_NUI_NOTPOWERED" =>
                        $"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/360/troubleshooting/notpowered/",
                    "E_NUI_NOTREADY" =>
                        $"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/360/troubleshooting/notready/",
                    "E_NUI_NOTGENUINE" =>
                        $"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/360/troubleshooting/notgenuine/",
                    "E_NUI_INSUFFICIENTBANDWIDTH" =>
                        $"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/360/troubleshooting/insufficientbandwidth",
                    _ => $"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/app/help/"
                }));
                break;

            case "Xbox One Kinect":
                await Launcher.LaunchUriAsync(new Uri(deviceErrorString switch
                {
                    "E_NOTAVAILABLE" => $"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/one/troubleshooting/",
                    _ => $"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/app/help/"
                }));
                break;

            case "PSMove Service":
                await Launcher.LaunchUriAsync(new Uri(deviceErrorString switch
                {
                    "E_PSMS_NOT_RUNNING" =>
                        $"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/psmove/troubleshooting/",
                    "E_PSMS_NO_JOINTS" =>
                        $"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/psmove/troubleshooting/",
                    _ => $"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/app/help/"
                }));
                break;

            default:
                await Launcher.LaunchUriAsync(
                    new Uri($"https://docs.k2vr.tech/{AppData.Settings.AppLanguage}/app/help/"));
                break;
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

    private void PluginManagerFlyout_OnOpened(object sender, object e)
    {
        Shared.Devices.PluginsPageLoadedOnce = true;
    }

    private void JointsSelectorComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
    }

    // MVVM stuff
    public event PropertyChangedEventHandler PropertyChanged;

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    // Waist, LR Foot
    public List<AppTracker> BasicTrackers => AppData.Settings.TrackersVector.ToArray()[..3].ToList();

    // LR Knee, LR Elbow
    public List<AppTracker> StandardTrackers => AppData.Settings.TrackersVector.ToArray()[3..7].ToList();

    // Everything else
    public List<AppTracker> AdditionalTrackers => AppData.Settings.TrackersVector.Count > 6
        ? AppData.Settings.TrackersVector.ToArray()[7..].ToList()
        : new List<AppTracker>(); // Dummy list with no items (at least not null)

    public Visibility CombineVisibility(Visibility v1, Visibility v2, Visibility v3)
    {
        return new[] { v1, v2, v3 }.Contains(Visibility.Visible) ? Visibility.Visible : Visibility.Collapsed;
    }
}