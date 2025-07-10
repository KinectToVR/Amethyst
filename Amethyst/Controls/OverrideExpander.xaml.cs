using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Threading.Tasks;
using Amethyst.Classes;
using Amethyst.Pages;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

namespace Amethyst.Controls;

public sealed partial class OverrideExpander : UserControl, INotifyPropertyChanged
{
    private List<AppTracker> _trackers = [];

    public OverrideExpander()
    {
        InitializeComponent();
        ResubscribeListeners(); // Register for any pending changes

        Interfacing.AppSettingsRead += (_, _) => ResubscribeListeners();
        Shared.Events.RequestOverrideExpandersCollapseEvent += (_, _) => Task.FromResult(
            Shared.Main.DispatcherQueue.TryEnqueue(() => OverridesItemsExpander.IsExpanded = false));
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

    private bool IsAnyTrackerEnabled => AppPlugins.IsOverride(
        AppData.Settings.SelectedTrackingDeviceGuid) && Trackers.Any(x => x.IsActive);

    public event PropertyChangedEventHandler PropertyChanged;

    private void ResubscribeListeners()
    {
        // Unregister all reload events
        AppData.Settings.PropertyChanged -= OnPropertyChanged;
        AppData.Settings.TrackersVector.CollectionChanged -= OnPropertyChanged;
        Trackers.ForEach(x => x.PropertyChanged -= OnPropertyChanged);

        // Register for any pending changes
        AppData.Settings.PropertyChanged += OnPropertyChanged;
        AppData.Settings.TrackersVector.CollectionChanged += OnPropertyChanged;
        Trackers.ForEach(x => x.PropertyChanged += OnPropertyChanged);
    }

    public void OnPropertyChanged(object propName = null, object e = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(
            propName is string ? propName.ToString() : null));
    }

    private static List<string> GetManagingDeviceJointsList(string guid)
    {
        if (string.IsNullOrEmpty(guid))
            return
            [
                Interfacing.LocalizedJsonString(
                    "/DevicesPage/Placeholders/Overrides/NoOverride/PlaceholderText")
            ];

        var jointsList = AppPlugins.GetDevice(guid).Device.TrackedJoints
            .Select(x => x.Name).ToList();

        jointsList.Insert(0, Interfacing.LocalizedJsonString(
            "/DevicesPage/Placeholders/Overrides/NoOverride/PlaceholderText"));
        return jointsList;
    }

    private void OverrideTrackerComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        // Don't even care if we're not set up yet
        if (!((sender as ComboBox)?.IsLoaded ?? false) ||
            !Shared.Devices.DevicesJointsValid) return;

        // Block invalid requests
        if (!IsAnyTrackerEnabled)
            // Trackers.ForEach(x => x.OnPropertyChanged());
            return; // Invalidate the pending input changes

        // Fix the selection index if it's invalid
        if (((ComboBox)sender).SelectedIndex < 0)
            ((ComboBox)sender).SelectedItem = GetManagingDeviceJointsList(
                    AppData.Settings.SelectedTrackingDeviceGuid)
                .ElementAtOrDefault((((ComboBox)sender).DataContext as AppTracker)!
                    .SelectedOverrideJointIdForSelectedDevice);

        if ((((ComboBox)sender).DataContext as AppTracker)!.SelectedOverrideJointIdForSelectedDevice ==
            ((ComboBox)sender).SelectedIndex) return; // Check if already okay

        // Signal the just-selected tracked joint (checker copied from AppTracker.cs)
        if (((ComboBox)sender).SelectedIndex > 0)
            AppPlugins.GetDevice(AppData.Settings.SelectedTrackingDeviceGuid).Device?
                .SignalJoint(((ComboBox)sender).SelectedIndex > 0 ? ((ComboBox)sender).SelectedIndex - 1 : 0);

        (((ComboBox)sender).DataContext as AppTracker)!.SelectedOverrideJointIdForSelectedDevice =
            ((ComboBox)sender).SelectedIndex; // Set the property of the host

        // Validate the pending input changes
        Trackers.ForEach(x => x.OnPropertyChanged());
    }

    private void OverrideTrackerCombo_OnDropDownOpened(object sender, object e)
    {
        if (!Devices.DisableJointExpanderSounds)
            AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void OverrideTrackerCombo_OnDropDownClosed(object sender, object e)
    {
        if (!Devices.DisableJointExpanderSounds)
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

        // OnPropertyChanged(); // Not needed anymore
    }

    private void OverrideOrientationSwitch_Toggled(object sender, RoutedEventArgs e)
    {
        // Don't even care if we're not set up yet
        if (!((sender as ToggleSwitch)?.IsLoaded ?? false) ||
            !IsAnyTrackerEnabled || !Shared.Devices.DevicesJointsValid) return;

        AppSounds.PlayAppSound(((ToggleSwitch)sender).IsOn
            ? AppSounds.AppSoundType.ToggleOn
            : AppSounds.AppSoundType.ToggleOff);

        // OnPropertyChanged(); // Not needed anymore
    }

    private void OverridesItemsExpander_Expanding(Expander sender, ExpanderExpandingEventArgs args)
    {
        // Don't even care if we're not set up yet
        if (!IsAnyTrackerEnabled) return;

        if (!Devices.DisableJointExpanderSounds)
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

    private void OverrideSwitch_Toggled(object sender, RoutedEventArgs e)
    {
        // Don't even care if we're not set up yet
        if (!((sender as CheckBox)?.IsLoaded ?? false) ||
            !IsAnyTrackerEnabled || !Shared.Devices.DevicesJointsValid) return;

        AppSounds.PlayAppSound(((CheckBox)sender).IsChecked ?? false
            ? AppSounds.AppSoundType.ToggleOn
            : AppSounds.AppSoundType.ToggleOff);

        // OnPropertyChanged(); // Not needed anymore
    }
}