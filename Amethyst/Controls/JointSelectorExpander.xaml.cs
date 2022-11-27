// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Amethyst.Classes;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Controls;

public sealed partial class JointSelectorExpander : UserControl, INotifyPropertyChanged
{
    private List<AppTracker> _trackers = new();
    public event PropertyChangedEventHandler PropertyChanged;

    public string Header { get; set; } = "";

    public JointSelectorExpander()
    {
        InitializeComponent();

        // Register for any pending changes
        AppData.Settings.PropertyChanged += (_, _) => OnPropertyChanged();
        Trackers.ForEach(x => x.PropertyChanged += (_, _) => { OnPropertyChanged(); });
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
            OnPropertyChanged(); // Trigger a complete refresh of the user control
        }
    }

    private bool IsAnyTrackerEnabled =>
        AppData.Settings.SelectedTrackingDeviceGuid == AppData.Settings.TrackingDeviceGuid &&
        Trackers.Any(x => x.IsActive && x.IsManuallyManaged);

    private static List<string> GetBaseDeviceJointsList()
    {
        return TrackingDevices.GetTrackingDevice().TrackedJoints.Select(x => x.JointName).ToList();
    }

    private void JointsSelectorComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
    }

    private void JointTrackerCombo_OnDropDownOpened(object sender, object e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void JointTrackerCombo_OnDropDownClosed(object sender, object e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }
}