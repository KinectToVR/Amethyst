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

public sealed partial class JointSelectorExpander : UserControl, INotifyPropertyChanged
{
    private bool _areChangesValid;
    private List<AppTracker> _trackers = new();

    // OnPropertyChanged listener for containers
    public EventHandler PropertyChangedEvent;

    public JointSelectorExpander()
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

    private bool IsAnyTrackerEnabled =>
        AppData.Settings.SelectedTrackingDeviceGuid == AppData.Settings.TrackingDeviceGuid &&
        Trackers.Any(x => x.IsActive && x.IsManuallyManaged);

    public event PropertyChangedEventHandler PropertyChanged;

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
        PropertyChangedEvent?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    private static List<string> GetBaseDeviceJointsList()
    {
        return TrackingDevices.BaseTrackingDevice.TrackedJoints.Select(x => x.Name).ToList();
    }

    private void JointsSelectorComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        // Don't even care if we're not set up yet
        if (!((sender as ComboBox)?.IsLoaded ?? false) ||
            !IsAnyTrackerEnabled || !Shared.Devices.DevicesJointsValid) return;

        // Either fix the selection index or give up on everything
        if (((ComboBox)sender).SelectedIndex < 0)
        {
            ((ComboBox)sender).SelectedItem = GetBaseDeviceJointsList()
                .ElementAtOrDefault((((ComboBox)sender).DataContext as AppTracker)!.SelectedBaseTrackedJointId);
        }

        //else
        else if (_areChangesValid)
        {
            if (((ComboBox)sender).SelectedIndex < 0)
                ((ComboBox)sender).SelectedItem = e.RemovedItems[0];

            // Signal the just-selected tracked joint
            TrackingDevices.BaseTrackingDevice.SignalJoint(((ComboBox)sender).SelectedIndex);
            (((ComboBox)sender).DataContext as AppTracker)!.SelectedBaseTrackedJointId =
                ((ComboBox)sender).SelectedIndex; // Update the host data (manual) binding
        }
    }

    private void JointTrackerCombo_OnDropDownOpened(object sender, object e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
        _areChangesValid = true; // Begin changes
    }

    private void JointTrackerCombo_OnDropDownClosed(object sender, object e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
        _areChangesValid = false; // End changes
    }

    private void JointsItemsExpander_Expanding(Expander sender, ExpanderExpandingEventArgs args)
    {
        // Don't even care if we're not set up yet
        if (!IsAnyTrackerEnabled || !Shared.Devices.DevicesJointsValid) return;
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void JointsItemsExpander_Collapsed(Expander sender, ExpanderCollapsedEventArgs args)
    {
        // Don't even care if we're not set up yet
        if (!IsAnyTrackerEnabled || !Shared.Devices.DevicesJointsValid) return;
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }

    private void JointsItemsRepeater_Loaded(object sender, RoutedEventArgs e)
    {
        // Don't even care if we're not set up yet
        if ((!(sender as ItemsRepeater)?.IsLoaded ?? false) ||
            !IsAnyTrackerEnabled || !Shared.Devices.DevicesJointsValid) return;

        Trackers.ForEach(x => x.OnPropertyChanged());
    }
}