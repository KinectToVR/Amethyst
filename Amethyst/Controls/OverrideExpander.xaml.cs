// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using Amethyst.Classes;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

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
        Trackers.ForEach(x => x.PropertyChanged += (_, _) => { OnPropertyChanged(); });
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

    private bool IsAnyTrackerEnabled =>
        Trackers.Any(x => x.IsActive && x.OverrideGuid == AppData.Settings.SelectedTrackingDeviceGuid);

    public event PropertyChangedEventHandler PropertyChanged;

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
        PropertyChangedEvent?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    private static List<string> GetManagingDeviceJointsList(string guid)
    {
        var jointsList = TrackingDevices.GetDevice(guid).Device.TrackedJoints
            .Select(x => x.JointName).ToList();

        jointsList.Insert(0, Interfacing.LocalizedJsonString(""));
        return jointsList;
    }

    private void OverrideTrackerComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
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
        AppSounds.PlayAppSound((sender as ToggleSwitch).IsOn
            ? AppSounds.AppSoundType.ToggleOn
            : AppSounds.AppSoundType.ToggleOff);
    }

    private void OverrideOrientationSwitch_Toggled(object sender, RoutedEventArgs e)
    {
        AppSounds.PlayAppSound((sender as ToggleSwitch).IsOn
            ? AppSounds.AppSoundType.ToggleOn
            : AppSounds.AppSoundType.ToggleOff);
    }
}