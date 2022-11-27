// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Threading.Tasks;
using Amethyst.Classes;
using Amethyst.Driver.API;
using Amethyst.Driver.Client;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Controls;

public sealed partial class JointSettingsExpander : UserControl, INotifyPropertyChanged
{
    private ObservableCollection<AppTracker> _trackers = new();

    // OnPropertyChanged listener for containers
    public EventHandler PropertyChangedEvent;

    public JointSettingsExpander()
    {
        InitializeComponent();

        // Register for any pending changes
        AppData.Settings.PropertyChanged += (_, _) => OnPropertyChanged();
    }

    public TrackerType Role { get; set; }

    public List<AppTracker> Trackers => AppData.Settings.TrackersVector.Where(x => x.Role == Role || (
        AppData.Settings.UseTrackerPairs && x.Role == TypeUtils.PairedTrackerTypeDictionary[Role])).ToList();

    private string Header => Interfacing.LocalizedJsonString(
        $"/SharedStrings/Joints/{(Trackers.Count > 1 ? "Pairs" : "Enum")}/{
            (int)(Trackers.FirstOrDefault()?.Role ?? TrackerType.TrackerHanded)}");

    private bool IsActive
    {
        get => Trackers.All(x => x.IsActive);
        set
        {
            Trackers.ToList().ForEach(x => x.IsActive = value);
            OnPropertyChanged(); // All
        }
    }

    private bool IsTrackerExpanderOpen
    {
        get => Trackers.FirstOrDefault()?.IsTrackerExpanderOpen ?? false;
        set => Trackers.FirstOrDefault(new AppTracker()).IsTrackerExpanderOpen = value;
    }

    private int PositionTrackingDisplayOption
    {
        get => Trackers.FirstOrDefault()?.PositionTrackingDisplayOption ?? -1;
        set => Trackers.ToList().ForEach(x => x.PositionTrackingDisplayOption = value);
    }

    private int OrientationTrackingDisplayOption
    {
        get => Trackers.FirstOrDefault()?.OrientationTrackingDisplayOption ?? -1;
        set => Trackers.ToList().ForEach(x => x.OrientationTrackingDisplayOption = value);
    }

    private string ManagingDevicePlaceholder => Trackers.FirstOrDefault()?.ManagingDevicePlaceholder ?? "INVALID";
    private bool NoPositionFilteringRequested => Trackers.FirstOrDefault()?.NoPositionFilteringRequested ?? false;
    private bool AppOrientationSupported => Trackers.All(x => x.AppOrientationSupported);

    private bool Show => AppData.Settings.UseTrackerPairs
        ? Trackers.Count > 1 || AppData.Settings.TrackersVector.All(x =>
            x.Role != TypeUtils.PairedTrackerTypeDictionaryReverse[
                Trackers.FirstOrDefault()?.Role ?? TrackerType.TrackerHanded])
        : Trackers.Count == 1; // Either only (paired or mixed) or only the single ones

    public event PropertyChangedEventHandler PropertyChanged;

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
        PropertyChangedEvent?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    private bool InvertBool(bool v)
    {
        return !v;
    }

    private void TrackerCombo_OnDropDownOpened(object sender, object e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void TrackerCombo_OnDropDownClosed(object sender, object e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
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

    private void CheckComboBoxSelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if ((sender as ComboBox)!.SelectedIndex < 0)
            (sender as ComboBox)!.SelectedItem = e.RemovedItems[0];
    }
}