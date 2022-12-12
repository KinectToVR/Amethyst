// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Threading.Tasks;
using Amethyst.Classes;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Controls;

public sealed partial class JointSettingsExpander : UserControl, INotifyPropertyChanged
{
    private bool _filterInteractionsBlocked;

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
        $"/SharedStrings/Joints/{(Trackers.Count > 1 ? "Pairs" : "Enum")}/" +
        $"{(int)(Trackers.FirstOrDefault(x => x.Role == Role)?.Role ?? TrackerType.TrackerHanded)}");

    private bool IsActive => Trackers.All(x => x.IsActive);

    private bool IsSupported => Trackers.All(x => x.IsSupported);

    private bool IsActiveEnabled
    {
        get => Trackers.All(x => x.IsActiveEnabled);
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
        set
        {
            if (_filterInteractionsBlocked) return;
            Trackers.ToList().ForEach(x => x.PositionTrackingDisplayOption = value);
        }
    }

    private int OrientationTrackingDisplayOption
    {
        get => Trackers.FirstOrDefault()?.OrientationTrackingDisplayOption ?? -1;
        set
        {
            if (_filterInteractionsBlocked) return;
            Trackers.ToList().ForEach(x => x.OrientationTrackingDisplayOption = value);
        }
    }

    private string ManagingDevicePlaceholder => Trackers.FirstOrDefault()?.ManagingDevicePlaceholder ?? "INVALID";
    private bool NoPositionFilteringRequested => Trackers.FirstOrDefault()?.NoPositionFilteringRequested ?? false;
    private bool AppOrientationSupported => Trackers.All(x => x.AppOrientationSupported);

    private bool Show => AppData.Settings.UseTrackerPairs
        ? Trackers.Count > 1 || AppData.Settings.TrackersVector.All(x =>
            x.Role != TypeUtils.PairedTrackerTypeDictionaryReverse[
                Trackers.FirstOrDefault()?.Role ?? TrackerType.TrackerHanded])
        : Trackers.Count == 1; // Either only (paired or mixed) or only the single ones

    private string NotSupportedText => Interfacing.LocalizedJsonString("/SharedStrings/Joints/NotSupported/Tooltip")
        .Replace("{0}", TrackingDevices.CurrentServiceEndpoint.Name);

    public event PropertyChangedEventHandler PropertyChanged;

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));

        // Block all ComboBox interactions
        _filterInteractionsBlocked = true;

        // Refresh ComboBox selected (display) items
        PositionFilterComboBox.SelectedIndex = -1;
        PositionFilterComboBox.SelectedIndex = PositionTrackingDisplayOption;
        OrientationOptionComboBox.SelectedIndex = -1;
        OrientationOptionComboBox.SelectedIndex = OrientationTrackingDisplayOption;

        // Unblock all ComboBox interactions
        _filterInteractionsBlocked = false;
    }

    private bool InvertBool(bool v)
    {
        return !v;
    }

    private Visibility InvertVisibility(bool v)
    {
        return !v ? Visibility.Visible : Visibility.Collapsed;
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
                var trackerBase = tracker.GetTrackerBase();
                trackerBase.ConnectionState = (sender as ToggleSwitch)!.IsOn;

                await TrackingDevices.CurrentServiceEndpoint.SetTrackerStates(new[] { trackerBase });
                await Task.Delay(20);
            }

        // Check if any trackers are enabled
        if (!(sender as ToggleSwitch)!.IsOn && !AppData.Settings.TrackersVector
                .Where(x => x.Role != tracker!.Role).Any(x => x.IsActive))
        {
            Logger.Warn("All supported trackers (except this one) have been disabled, force-re-enabling!");
            tracker!.IsActive = true; // Force re-enable this tracker
            tracker.OnPropertyChanged("IsActive");
            tracker.OnPropertyChanged("IsActiveEnabled");
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