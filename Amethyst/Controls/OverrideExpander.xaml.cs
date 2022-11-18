// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
using Amethyst.Classes;
using System.ComponentModel;
using Amethyst.Utils;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Controls;

public sealed partial class OverrideExpander : UserControl, INotifyPropertyChanged
{
    private List<AppTracker> _trackers = new();
    public event PropertyChangedEventHandler PropertyChanged;

    public string Header { get; set; } = "";

    public OverrideExpander()
    {
        InitializeComponent();
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
        PropertyChangedEvent?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    // OnPropertyChanged listener for containers
    public EventHandler PropertyChangedEvent;

    public List<AppTracker> Trackers
    {
        get => _trackers;
        set
        {
            _trackers = value;
            Trackers.ForEach(x => x.PropertyChangedEvent += (_, _) => { OnPropertyChanged(); });
            OnPropertyChanged(); // Trigger a complete refresh of the user control
        }
    }

    private bool IsAnyTrackerEnabled =>
        Trackers.Any(x => x.IsActive && x.OverrideGuid == Shared.Devices.SelectedTrackingDeviceGuid);

    private static List<string> GetManagingDeviceJointsList(string guid)
    {
        var jointsList = TrackingDevices.GetDevice(guid).Device.TrackedJoints
            .Select(x => x.Joint.JointName).ToList();

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

    private string GetResourceString(string key)
    {
        return Interfacing.LocalizedJsonString(key);
    }
}