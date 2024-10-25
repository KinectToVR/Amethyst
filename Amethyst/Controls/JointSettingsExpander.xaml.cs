// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using Amethyst.Classes;
using Amethyst.MVVM;
using Amethyst.Pages;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Controls;

public sealed partial class JointSettingsExpander : UserControl, INotifyPropertyChanged
{
    private bool _blockPropertyToggleSignals;

    public JointSettingsExpander()
    {
        InitializeComponent();

        ResubscribeListeners(); // Register for any pending changes
        Interfacing.AppSettingsRead += (_, _) => ResubscribeListeners();
    }

    public TrackerType Role { get; set; }

    public List<AppTracker> Trackers
    {
        get
        {
            // Logic: get all trackers that match the requested role or its flipped equivalent
            // Then, either group them by whether they are supported (by the current service)
            // If they appear to be mixed, return only the supported ones (see the upper line)
            // And if that fails, return only the non-supported trackers (not to pair mixed)

            var trackers = AppData.Settings.TrackersVector.Where(x => x.Role == Role || (
                AppData.Settings.UseTrackerPairs && x.Role == TypeUtils.PairedTrackerTypeDictionary[Role])).ToList();

            return trackers.All(x => x.IsSupported) || trackers.All(x => !x.IsSupported)
                ? trackers // All the same -> standard
                : trackers.Any(x => x.Role == Role && x.IsSupported)
                    ? trackers.Where(x => x.IsSupported).ToList()
                    : trackers.Where(x => !x.IsSupported).ToList();
        }
    }

    private string Header => Interfacing.LocalizedJsonString(
        $"/SharedStrings/Joints/{(TrackersCount > 1 ? "Pairs" : "Enum")}/" +
        $"{(int)(Trackers.FirstOrDefault(x => x.Role == Role)?.Role ?? TrackerType.TrackerHanded)}");

    private bool IsActive => Trackers.All(x => x.IsActive);

    private bool IsSupported => Trackers.All(x => x.IsSupported);

    private bool ShowBindingsSection => Trackers.Any(x => x.AvailableInputActions.Count > 0);

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
        get => NoPositionFilteringRequested ? -1 : Trackers.FirstOrDefault()?.PositionTrackingDisplayOption ?? -1;
        set
        {
            if (NoPositionFilteringRequested)
            {
                OnPropertyChanged();
                return; // Don't care
            }

            Trackers.ToList().ForEach(x => x.PositionTrackingDisplayOption = value);
        }
    }

    private int OrientationTrackingDisplayOption
    {
        get => Trackers.FirstOrDefault()?.OrientationTrackingDisplayOption ?? -1;
        set => Trackers.ToList().ForEach(x => x.OrientationTrackingDisplayOption = value);
    }

    private string ManagingDevicePlaceholder => Trackers.FirstOrDefault()?.ManagingDevicePlaceholder ?? "INVALID";
    private bool NoPositionFilteringRequested => Trackers.Any(x => x.NoPositionFilteringRequested);
    private bool AppOrientationSupported => Trackers.All(x => x.AppOrientationSupported);

    private bool Show
    {
        get
        {
            // Logic: get all trackers that match the requested role or its flipped equivalent
            // Then, either group them by whether they are supported (by the current service)
            // If they appear to be mixed, return only the supported ones (see the upper line)
            // And if that fails, return only the non-supported trackers (not to pair mixed)

            AppData.Settings.TrackersVector.GroupBy(x =>
                    x.IsSupported == Trackers.FirstOrDefault()?.IsSupported &&
                    (x.Role == Trackers.FirstOrDefault()?.Role ||
                     x.Role == TypeUtils.PairedTrackerTypeDictionaryReverse[
                         Trackers.FirstOrDefault()?.Role ?? TrackerType.TrackerHanded]))
                .ToDictionary(x => x.Key)
                .TryGetValue(true, out var c);

            return AppData.Settings.UseTrackerPairs
                ? TrackersCount > 1 || (Trackers.Count == 1 && (c?.Count() ?? 1) == 1)
                : TrackersCount == 1; // Either only (paired or mixed) or only the single ones
        }
    }

    private string NotSupportedText =>
        Interfacing.LocalizedJsonString("/SharedStrings/Joints/NotSupported/Tooltip")
            .Format(AppPlugins.CurrentServiceEndpoint.Name);

    private int TrackersCount =>
        Trackers.All(x => x.IsSupported) ||
        Trackers.All(x => !x.IsSupported)
            ? Trackers.Count // All the same -> standard
            : Trackers.Count(x => x.IsSupported);

    public event PropertyChangedEventHandler PropertyChanged;

    private void ResubscribeListeners()
    {
        // Unregister all reload events
        AppData.Settings.PropertyChanged -= OnPropertyChanged;
        AppData.Settings.TrackersVector.CollectionChanged -= OnPropertyChanged;
        Shared.Events.RequestJointSettingsRefreshEvent -= OnPropertyChanged;
        Trackers.ForEach(x => x.PropertyChanged -= OnPropertyChanged);

        // Register for any pending changes
        AppData.Settings.PropertyChanged += OnPropertyChanged;
        AppData.Settings.TrackersVector.CollectionChanged += OnPropertyChanged;
        Shared.Events.RequestJointSettingsRefreshEvent += OnPropertyChanged;
        Trackers.ForEach(x => x.PropertyChanged += OnPropertyChanged);
    }

    public void OnPropertyChanged(object propName = null, object e = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(
            propName is string ? propName.ToString() : null));
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
        if (!Shared.Settings.SettingsTabSetupFinished ||
            _blockPropertyToggleSignals) return;

        // Check if any trackers are enabled
        if (!(sender as ToggleSwitch)!.IsOn && !AppData.Settings.TrackersVector
                .Where(x => Trackers.All(tracker => tracker.Role != x.Role)).Any(x => x.IsActive))
        {
            Logger.Warn("All supported trackers have been disabled, force-re-enabling!");
            Trackers.ForEach(x =>
            {
                x.IsActive = true; // Force re-enable this tracker
                x.OnPropertyChanged("IsActive");
                x.OnPropertyChanged("IsActiveEnabled");
            });

            _blockPropertyToggleSignals = true;
            OnPropertyChanged(); // Refresh UI
            _blockPropertyToggleSignals = false;
        }
        // This change was valid, proceed further
        else
        {
            // Update tracker data before the next event
            // (TrackersConfigChanged) can access/reset it
            _blockPropertyToggleSignals = true;
            IsActiveEnabled = (sender as ToggleSwitch)!.IsOn;
            _blockPropertyToggleSignals = false;

            // Update states if the change is valid
            if (Interfacing.AppTrackersInitialized)
                foreach (var tracker in Trackers)
                    for (var i = 0; i < 3; i++)
                    {
                        // Try 3 times cause why not
                        // Update status in server
                        var trackerBase = tracker.GetTrackerBase();
                        trackerBase.ConnectionState = (sender as ToggleSwitch)!.IsOn;

                        await AppPlugins.CurrentServiceEndpoint.SetTrackerStates(new[] { trackerBase });
                        await Task.Delay(20);
                    }

            // Notify about our pending changes
            if (Interfacing.AppTrackersSpawned)
                AppPlugins.TrackersConfigChanged();

            if (Show) // Play a sound (if the expander is valid)
                AppSounds.PlayAppSound((sender as ToggleSwitch)!.IsOn
                    ? AppSounds.AppSoundType.ToggleOn
                    : AppSounds.AppSoundType.ToggleOff);
        }

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

    private void PositionFilterComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if ((sender as ComboBox)!.SelectedIndex < 0 && !NoPositionFilteringRequested)
            (sender as ComboBox)!.SelectedItem = e.RemovedItems[0];
    }

    private void OrientationOptionComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if ((sender as ComboBox)!.SelectedIndex < 0)
            (sender as ComboBox)!.SelectedItem = e.RemovedItems[0];
    }

    private void ActionConfigButton_OnClick(object sender, RoutedEventArgs e)
    {
        // ignored
    }

    private void InputActionToggle_OnClick(object sender, RoutedEventArgs e)
    {
        if (sender is not ToggleMenuFlyoutItem { DataContext: InputActionEntry entry, Parent: ItemsRepeater parent } item ||
            parent.GetValue(AttachedObject.AttachedObjectProperty) is not AppTracker tracker) return;

        if (!tracker.InputActions.ContainsKey(AppData.Settings.ServiceEndpointGuid))
            tracker.InputActions[AppData.Settings.ServiceEndpointGuid] = [];

        // Set up the binding placeholder now
        if (item.IsChecked)
            tracker.InputActions[AppData.Settings.ServiceEndpointGuid][entry.Action] = null;
        else
            tracker.InputActions[AppData.Settings.ServiceEndpointGuid].Remove(
                tracker.InputActions[AppData.Settings.ServiceEndpointGuid]
                    .FirstOrDefault(x => x.Key.Guid == entry.Action.Guid).Key ?? entry.Action);

        // Refresh everything
        _blockPropertyToggleSignals = true;
        OnPropertyChanged(); // Refresh UI
        Trackers.ForEach(x =>
        {
            x.OnPropertyChanged("InputActionEntries");
            x.OnPropertyChanged("InputActionBindingEntries");
        });
        _blockPropertyToggleSignals = false;

        // Hide and save
        if (((parent.Parent as StackPanel)?.Parent as FlyoutPresenter)?
            .Parent is Microsoft.UI.Xaml.Controls.Primitives.Popup popup)
            popup.IsOpen = false;

        AppData.Settings.SaveSettings();
    }

    private void BindingFlyoutItem_OnClick(object sender, RoutedEventArgs e)
    {
        if (sender is not MenuFlyoutItem { DataContext: InputActionBindingEntry entry } item ||
            item.GetValue(AttachedString.AttachedStringProperty) is not string command) return;

        var tracker = Trackers.FirstOrDefault(x => x.Role == entry.Action.Tracker);
        if (tracker is null) return; // Not related to the updated selection

        switch (command)
        {
            case "HIDE":
                // Remove the action from our bindings
                tracker.InputActions[AppData.Settings.ServiceEndpointGuid].Remove(
                    tracker.InputActions[AppData.Settings.ServiceEndpointGuid]
                        .FirstOrDefault(x => x.Key.Guid == entry.Action.Guid).Key ?? entry.Action);
                break;
            case "DISABLE":
                // Set up the command as "null" so it's shown but disabled
                tracker.InputActions[AppData.Settings.ServiceEndpointGuid][entry.Action] = null;
                break;
            default:
                return;
        }

        // Refresh everything
        _blockPropertyToggleSignals = true;
        OnPropertyChanged(); // Refresh UI
        Trackers.ForEach(x =>
        {
            x.OnPropertyChanged("InputActionEntries");
            x.OnPropertyChanged("InputActionBindingEntries");
        });
        _blockPropertyToggleSignals = false;

        // Hide and save
        AppData.Settings.SaveSettings();
    }

    private void BindingsFlyout_OnOpening(object sender, object e)
    {
        if (sender is not MenuFlyout flyout) return;
        if (flyout.Items.LastOrDefault()?.DataContext is not InputActionBindingEntry entry) return;

        var tracker = Trackers.FirstOrDefault(x => x.Role == entry.Action.Tracker);
        if (tracker is null) return; // Not related to the updated selection

        if (flyout.Items.Count > 4) // Remove the sources
            flyout.Items.Take(flyout.Items.Count - 4)
                .ToList().ForEach(x => flyout.Items.Remove(x));

        // Loop over all devices that have accessible actions and add them to the flyout
        // Group by Device>Joint>Action(Selectable)
        foreach (var device in AppPlugins.TrackingDevicesList.Values
                     .Where(x => x.TrackedJoints.Any(y => y.SupportedInputActions.Any()))
                     .OrderByDescending(x => x.Name))
        {
            var deviceItem = new MenuFlyoutSubItem { Text = device.Name };
            foreach (var joint in device.TrackedJoints
                         .Where(x => x.SupportedInputActions.Any()))
            {
                var jointItem = new MenuFlyoutSubItem { Text = joint.Name };
                foreach (var action in joint.SupportedInputActions.OrderBy(action => action.Name))
                {
                    var actionItem = new ToggleMenuFlyoutItem
                    {
                        Text = action.Name,
                        IsChecked = entry.Source?.LinkedAction?.Guid == action.Guid,
                        IsEnabled = entry.Action?.LinkedAction?.DataType == action.DataType
                    };

                    ToolTipService.SetToolTip(actionItem,
                        new ToolTip { Content = action.Description });

                    actionItem.Click += (_, _) =>
                    {
                        // Set up the command binding
                        tracker.InputActions[AppData.Settings.ServiceEndpointGuid]
                            [entry.Action] = new InputActionSource
                        {
                            Device = device.Guid,
                            Guid = action.Guid,
                            Name = action.Name,
                            Tracker = joint.Role
                        };

                        // Refresh everything
                        _blockPropertyToggleSignals = true;
                        OnPropertyChanged(); // Refresh UI
                        Trackers.ForEach(x =>
                        {
                            x.OnPropertyChanged("InputActionEntries");
                            x.OnPropertyChanged("InputActionBindingEntries");
                        });
                        _blockPropertyToggleSignals = false;

                        AppData.Settings.SaveSettings(); // Save and signal
                        device.SignalJoint(device.TrackedJoints.IndexOf(joint));
                    };
                    jointItem.Items.Add(actionItem);
                }

                deviceItem.Items.Add(jointItem);
            }

            flyout.Items.Insert(0, deviceItem);
        }
    }
}

public class AttachedObject : DependencyObject
{
    public static readonly DependencyProperty AttachedObjectProperty =
        DependencyProperty.RegisterAttached(
            "AttachedObject",
            typeof(object),
            typeof(AttachedObject),
            new PropertyMetadata(false)
        );

    public static void SetAttachedObject(UIElement element, object value)
    {
        element.SetValue(AttachedObjectProperty, value);
    }

    public static object GetAttachedObject(UIElement element)
    {
        return element.GetValue(AttachedObjectProperty);
    }
}