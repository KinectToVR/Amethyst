using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using System.Timers;
using Amethyst.Classes;
using Amethyst.MVVM;
using Amethyst.Pages;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Media.Animation;

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

                        await AppPlugins.CurrentServiceEndpoint.SetTrackerStates([trackerBase]);
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
            .Parent is Popup popup)
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
        if ((((((item.Parent as StackPanel)?.Parent as Grid)?.Parent as Grid)?.Parent as Grid)?
                .Parent as FlyoutPresenter)?.Parent is Popup popup) popup.IsOpen = false;

        // Hide and save
        AppData.Settings.SaveSettings();
    }

    private void ButtonFlyout_Opening(object sender, object e)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);

        if (sender is not Flyout flyout) return;
        if (flyout.Content is not Grid { DataContext: InputActionBindingEntry entry } grid) return;
        if ((((grid.Children.FirstOrDefault() as Grid)?.Children?.FirstOrDefault() as Grid)?
                .Children?.FirstOrDefault() as ScrollViewer)?.Content is not TreeView tree) return;

        var tracker = Trackers.FirstOrDefault(x => x.Role == entry.Action.Tracker);
        if (tracker is null) return; // Not related to the updated selection

        tree.RootNodes.Clear(); // Remove everything first
        AppPlugins.TrackingDevicesList.Values
            .Where(x => x.TrackedJoints.Any(y => y.SupportedInputActions.Any()))
            .OrderByDescending(x => x.Name).Select(device => new TreeViewNodeEx(device.Name,
                device.TrackedJoints.Where(joint => joint.SupportedInputActions.Any())
                    .Select(joint => new TreeViewNodeEx(joint.Name, joint.SupportedInputActions
                        .Select(action => new TreeViewNodeEx(action.Name, entry,
                            entry.Action, device.Guid, joint.Role, action)), entry)).ToList(),
                entry)).ToList().ForEach(tree.RootNodes.Add);

        tree.ItemInvoked -= Tree_ItemInvoked;
        tree.ItemInvoked += Tree_ItemInvoked;

        entry.TreeSelectedAction = null;
        InputActionBindingEntry.TreeCurrentAction = null;
    }

    private async void Tree_ItemInvoked(TreeView sender, TreeViewItemInvokedEventArgs args)
    {
        if (!sender.IsLoaded) return;
        if (args.InvokedItem is not TreeViewNodeEx { HasData: true } node)
        {
            sender.SelectionMode = TreeViewSelectionMode.None;

            if (args.InvokedItem is not
                TreeViewNodeEx { HasData: false, Entry: not null } node1) return;

            var shouldAnimate1 = node1.Entry.TreeSelectedAction != node1.Source;
            node1.Entry.TreeSelectedAction = node1.Source;

            if (!shouldAnimate1) return;
            await Tree_LaunchTransition(sender); 
            AppSounds.PlayAppSound(AppSounds.AppSoundType.Focus);

            return; // Give up now...
        }

        sender.SelectionMode = TreeViewSelectionMode.Single;
        sender.SelectedNode = node;

        var shouldAnimate = node.Entry.TreeSelectedAction != node.Source;
        node.Entry.TreeSelectedAction = node.Source;
        InputActionBindingEntry.TreeCurrentAction = node.Source;

        if (!shouldAnimate) return;
        await Tree_LaunchTransition(sender);
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
    }

    private async Task Tree_LaunchTransition(TreeView tree)
    {
        // Hide and save
        if (((tree.Parent as ScrollViewer)?.Parent as Grid)?.Parent
            is not Grid innerGrid || innerGrid.Children.Last() is not Grid previewGrid) return;

        // Action stuff reload animation
        try
        {
            // Remove the only one child of our outer main content grid
            // (What a bestiality it is to do that!!1)
            innerGrid.Children.Remove(previewGrid);
            previewGrid.Transitions.Add(
                new EntranceThemeTransition { IsStaggeringEnabled = false });

            // Sleep peacefully pretending that noting happened
            await Task.Delay(10);

            // Re-add the child for it to play our funky transition
            // (Though it's not the same as before...)
            innerGrid.Children.Add(previewGrid);

            // Remove the transition
            await Task.Delay(100);
            previewGrid.Transitions.Clear();
        }
        catch (Exception e)
        {
            Logger.Error(e);
        }
    }

    private void ButtonFlyout_Closing(FlyoutBase sender,
        FlyoutBaseClosingEventArgs args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
        InputActionBindingEntry.TreeCurrentAction = null;
    }

    private void ToggleSplitButton_OnIsCheckedChanged(
        ToggleSplitButton sender, ToggleSplitButtonIsCheckedChangedEventArgs args)
    {
        sender.IsChecked = true;
    }

    private void SplitButton_OnClick(SplitButton sender, SplitButtonClickEventArgs args)
    {
        if (!sender.IsLoaded || sender.GetValue(AttachedObject.AttachedObjectProperty)
                is not InputActionBindingEntry entry) return; // Sanity check -> give up

        var tracker = Trackers.FirstOrDefault(x => x.Role == entry.Action.Tracker);
        if (tracker is null) return; // Not related to the updated selection

        // Set up the command binding
        tracker.InputActions[AppData.Settings.ServiceEndpointGuid]
            [entry.Action] = entry.TreeSelectedAction;

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
        if (((((sender.Parent as Grid)?.Parent as Grid)?.Parent as Grid)?.Parent as FlyoutPresenter)?
            .Parent is Popup popup) popup.IsOpen = false; // Hide the flyout

        if (!AppPlugins.GetDevice(entry.TreeSelectedAction.Device, out var device)) return;
        device.SignalJoint(device.TrackedJoints.FindIndex(x => x.Role == entry.TreeSelectedAction.Tracker));
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

internal class TreeViewNodeEx : TreeViewNode
{
    public InputActionBindingEntry Entry { get; set; }
    public InputActionEndpoint Endpoint { get; set; }
    public InputActionSource Source { get; set; }

    public bool HasData => Source is not null && Endpoint is not null;

    public TreeViewNodeEx(string name, IEnumerable<TreeViewNode> children,
        InputActionBindingEntry entry = null, InputActionEndpoint endpoint = null,
        InputActionSource source = null)
    {
        Content = name;
        Endpoint = endpoint;
        Source = source;
        Entry = entry;

        children.ToList().ForEach(Children.Add);
    }

    public TreeViewNodeEx(string name, InputActionBindingEntry entry = null,
        InputActionEndpoint endpoint = null, string device = null,
        TrackedJointType? tracker = null, IKeyInputAction source = null)
    {
        Content = name;
        Endpoint = endpoint;
        Entry = entry;

        if (string.IsNullOrEmpty(device) ||
            source is null || tracker is null) return;

        Source = new InputActionSource
        {
            Device = device,
            Guid = source.Guid,
            Name = source.Name,
            Tracker = tracker.Value
        };
    }
}