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

public sealed partial class JointSelectorExpander : UserControl, INotifyPropertyChanged
{
    private bool _areChangesValid;
    private List<AppTracker> _trackers = [];

    public JointSelectorExpander()
    {
        InitializeComponent();
        ResubscribeListeners(); // Register for any pending changes

        Interfacing.AppSettingsRead += (_, _) => ResubscribeListeners();
        Shared.Events.RequestJointSelectorExpandersCollapseEvent += (_, _) => Task.FromResult(
            Shared.Main.DispatcherQueue.TryEnqueue(() => JointsItemsExpander.IsExpanded = false));
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

    private static List<string> GetBaseDeviceJointsList()
    {
        return AppPlugins.BaseTrackingDevice.TrackedJoints.Select(x => x.Name).ToList();
    }

    private void JointsSelectorComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        // Don't even care if we're not set up yet
        if (!((sender as ComboBox)?.IsLoaded ?? false) ||
            !Shared.Devices.DevicesJointsValid) return;

        // Block invalid requests
        if (!IsAnyTrackerEnabled)
            // Trackers.ForEach(x => x.OnPropertyChanged());
            return; // Invalidate the pending input changes

        // Either fix the selection index or give up on everything
        if (((ComboBox)sender).SelectedIndex < 0)
        {
            ((ComboBox)sender).SelectedItem = GetBaseDeviceJointsList()
                .ElementAtOrDefault((((ComboBox)sender).DataContext as AppTracker)!.SelectedBaseTrackedJointId);
        }

        // else
        else if (_areChangesValid)
        {
            if (((ComboBox)sender).SelectedIndex < 0)
                ((ComboBox)sender).SelectedItem = e.RemovedItems[0];

            if ((((ComboBox)sender).DataContext as AppTracker)!.SelectedBaseTrackedJointId ==
                ((ComboBox)sender).SelectedIndex) return; // Check if already okay

            // Signal the just-selected tracked joint
            AppPlugins.BaseTrackingDevice.SignalJoint(((ComboBox)sender).SelectedIndex);
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
        if (!IsAnyTrackerEnabled || !Shared.Devices.DevicesJointsValid ||
            Devices.DisableJointExpanderSounds) return;

        Trackers.ForEach(x => x.OnPropertyChanged());
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void JointsItemsExpander_Collapsed(Expander sender, ExpanderCollapsedEventArgs args)
    {
        // Don't even care if we're not set up yet
        if (!IsAnyTrackerEnabled || !Shared.Devices.DevicesJointsValid ||
            Devices.DisableJointExpanderSounds) return;
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