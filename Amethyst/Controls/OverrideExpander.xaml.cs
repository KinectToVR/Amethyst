// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Threading.Tasks;
using Amethyst.Classes;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Newtonsoft.Json.Linq;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Controls;

public sealed partial class OverrideExpander : UserControl, INotifyPropertyChanged
{
    private List<AppTracker> _trackers = new();

    // OnPropertyChanged listener for containers
    public EventHandler PropertyChangedEvent;

    public OverrideExpander()
    {
        InitializeComponent();

        // Register for any pending changes
        AppData.Settings.PropertyChanged += (_, _) => OnPropertyChanged();
        AppData.Settings.TrackersVector.CollectionChanged += (_, _) => OnPropertyChanged();
        Trackers.ForEach(x => x.PropertyChanged += (_, _) => OnPropertyChanged());
    }

    public string Header { get; set; } = "";

    public List<AppTracker> Trackers
    {
        get => _trackers;
        set
        {
            _trackers = value;
            OnPropertyChanged(); // Trigger a complete refresh of the user control
        }
    }

    private bool IsAnyTrackerEnabled => TrackingDevices.IsOverride(
        AppData.Settings.SelectedTrackingDeviceGuid) && Trackers.Any(x => x.IsActive);

    public event PropertyChangedEventHandler PropertyChanged;

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
        PropertyChangedEvent?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    private static List<string> GetManagingDeviceJointsList(string guid)
    {
        if (string.IsNullOrEmpty(guid))
            return new List<string>
            {
                Interfacing.LocalizedJsonString(
                    "/DevicesPage/Placeholders/Overrides/NoOverride/PlaceholderText")
            };

        var jointsList = TrackingDevices.GetDevice(guid).Device.TrackedJoints
            .Select(x => x.Name).ToList();

        jointsList.Insert(0, Interfacing.LocalizedJsonString(
            "/DevicesPage/Placeholders/Overrides/NoOverride/PlaceholderText"));
        return jointsList;
    }

    private void OverrideTrackerComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        // Validate the pending input changes
        Trackers.ForEach(x => x.OnPropertyChanged());

        // Don't even care if we're not set up yet
        if (!((sender as ComboBox)?.IsLoaded ?? false) || 
            !IsAnyTrackerEnabled || !Shared.Devices.DevicesJointsValid) return;

        // Either fix the selection index or give up on everything
        if (((ComboBox)sender).SelectedIndex < 0)
            ((ComboBox)sender).SelectedItem = GetManagingDeviceJointsList(
                    AppData.Settings.SelectedTrackingDeviceGuid)
                .ElementAtOrDefault((((ComboBox)sender).DataContext as AppTracker)!
                    .SelectedOverrideJointIdForSelectedDevice);

        //else
        {
            // Signal the just-selected tracked joint (checker copied from AppTracker.cs)
            TrackingDevices.GetDevice(AppData.Settings.SelectedTrackingDeviceGuid).Device?
                .SignalJoint(((ComboBox)sender).SelectedIndex > 0 ? ((ComboBox)sender).SelectedIndex - 1 : 0);

            (((ComboBox)sender).DataContext as AppTracker)!.SelectedOverrideJointIdForSelectedDevice =
                ((ComboBox)sender).SelectedIndex; // Set the property of the host
        }
    }

    private void OverrideTrackerCombo_OnDropDownOpened(object sender, object e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void OverrideTrackerCombo_OnDropDownClosed(object sender, object e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }

    private void OverridePositionSwitch_Toggled(object sender, RoutedEventArgs e)
    {
        // Don't even care if we're not set up yet
        if (!((sender as ToggleSwitch)?.IsLoaded ?? false) || 
            !IsAnyTrackerEnabled || !Shared.Devices.DevicesJointsValid) return;

        AppSounds.PlayAppSound(((ToggleSwitch)sender).IsOn
            ? AppSounds.AppSoundType.ToggleOn
            : AppSounds.AppSoundType.ToggleOff);

        OnPropertyChanged();
    }

    private void OverrideOrientationSwitch_Toggled(object sender, RoutedEventArgs e)
    {
        // Don't even care if we're not set up yet
        if (!((sender as ToggleSwitch)?.IsLoaded ?? false) || 
            !IsAnyTrackerEnabled || !Shared.Devices.DevicesJointsValid) return;

        AppSounds.PlayAppSound(((ToggleSwitch)sender).IsOn
            ? AppSounds.AppSoundType.ToggleOn
            : AppSounds.AppSoundType.ToggleOff);

        OnPropertyChanged();
    }

    private void OverridesItemsExpander_Expanding(Expander sender, ExpanderExpandingEventArgs args)
    {
        // Don't even care if we're not set up yet
        if (!IsAnyTrackerEnabled) return;
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);

        Trackers.ForEach(x => x.OnPropertyChanged());
        OnPropertyChanged(); // Refresh everything upon expanding
    }

    private void JointsItemsRepeater_Loaded(object sender, RoutedEventArgs e)
    {
        // Don't even care if we're not set up yet
        if ((!(sender as ItemsRepeater)?.IsLoaded ?? false) || 
            !IsAnyTrackerEnabled || !Shared.Devices.DevicesJointsValid) return;

        Trackers.ForEach(x => x.OnPropertyChanged());
        OnPropertyChanged(); // Refresh everything upon expanding
    }
}