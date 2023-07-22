using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Amethyst.Classes;
using Amethyst.Installer.ViewModels;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media.Animation;
using Newtonsoft.Json;
using WinRT;
using static Amethyst.Classes.Shared.Events;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Installer.Views;

/// <summary>
///     An empty page that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class SetupDevices : Page, INotifyPropertyChanged
{
    private readonly List<string> _languageList = new();

    private bool _blockHiddenSoundOnce, _blockSelectionOnce;
    private bool _pageSetupFinished, _pageLoadedOnce;

    public SetupDevices()
    {
        InitializeComponent();

        Logger.Info($"Constructing page: '{GetType().FullName}'...");
        Logger.Info("Registering a detached binary semaphore " +
                    $"reload handler for '{GetType().FullName}'...");

        NextButton = new Button
        {
            Content = "Next",
            Margin = new Thickness { Right = 10 },
            HorizontalAlignment = HorizontalAlignment.Left,
            VerticalAlignment = VerticalAlignment.Bottom,
            OpacityTransition = new ScalarTransition(),
            Style = Application.Current.Resources["AccentButtonStyle"].As<Style>()
        };

        NextButton.Click += SetupData.LimitedSetup ? NextButtonNextPageOnClick : NextButton_Click;

        Task.Run(() =>
        {
            while (true)
            {
                // Wait for a reload signal (blocking)
                ReloadVendorPagesEvent.WaitOne();

                // Reload & restart the waiting loop
                if (_pageLoadedOnce)
                    DispatcherQueue.TryEnqueue(Page_LoadedHandler);

                // Reset the event
                ReloadPluginsPageEvent.Reset();
            }
        });
    }

    private Button NextButton { get; }
    public RaisedEvent ContinueEvent { get; set; }
    public List<SetupPlugin> Devices { get; set; }

    private List<(GridViewItem Container, SetupPlugin Item)> SelectedDevices { get; set; }
    private List<IDependency> DependenciesToInstall { get; set; }
    private SemaphoreSlim NextButtonClickedSemaphore { get; } = new(0);

    public List<SetupPluginGroup> GroupedDevices =>
        Devices.GroupBy(
            device => device.CoreSetupData.GroupName ?? string.Empty,
            device => device,
            (groupName, plugins) =>
                new SetupPluginGroup
                {
                    Name = groupName,
                    Plugins = plugins.ToList()
                }
        ).ToList();

    public event PropertyChangedEventHandler PropertyChanged;

    private void Page_Loaded(object sender, RoutedEventArgs e)
    {
        Logger.Info($"Re/Loading page: '{GetType().FullName}'...");

        if (!_pageLoadedOnce)
            Shared.Main.Window.Activated += (_, args) =>
            {
                if (_pageLoadedOnce)
                    DispatcherQueue.TryEnqueue(() =>
                    {
                        AppTitleLabel.Opacity = args.WindowActivationState is not
                            WindowActivationState.Deactivated
                            ? 1.0
                            : 0.5;
                    });
            };

        // Execute the handler
        Page_LoadedHandler();

        // Mark as loaded
        _pageLoadedOnce = true;
        MainGrid.Opacity = 1.0;
    }

    private void Page_LoadedHandler()
    {
        // Clear available languages' list
        _pageSetupFinished = false;
        LanguageOptionBox.Items.Clear();

        // Push all the found languages
        if (!Directory.Exists(Path.Join(Interfacing.ProgramLocation.DirectoryName, "Assets", "Strings"))) return;
        foreach (var entry in Directory.EnumerateFiles(
                     Path.Join(Interfacing.ProgramLocation.DirectoryName, "Assets", "Strings")))
        {
            if (Path.GetFileNameWithoutExtension(entry) == "locales") continue;

            _languageList.Add(Path.GetFileNameWithoutExtension(entry));
            LanguageOptionBox.Items.Add(Interfacing.GetLocalizedLanguageName(
                Path.GetFileNameWithoutExtension(entry)));

            if (Path.GetFileNameWithoutExtension(entry) == AppData.Settings.AppLanguage)
                LanguageOptionBox.SelectedIndex = LanguageOptionBox.Items.Count - 1;
        }

        // Mark as ready to go
        LanguageComboFlyout.Hide();
        _blockHiddenSoundOnce = true;
        _pageSetupFinished = true;

        // Reload
        OnPropertyChanged();
    }

    private void LanguageOptionBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        // Don't react to pre-init signals
        if (!_pageSetupFinished) return;
        if (LanguageOptionBox.SelectedIndex < 0)
            LanguageOptionBox.SelectedItem = e.RemovedItems[0];

        // Overwrite the current language code
        AppData.Settings.AppLanguage = _languageList[LanguageOptionBox.SelectedIndex];

        // Save made changes
        AppData.Settings.SaveSettings();

        // Reload language resources
        Interfacing.LoadJsonStringResourcesEnglish();
        Interfacing.LoadJsonStringResources(AppData.Settings.AppLanguage);

        // Request page reloads
        Translator.Get.OnPropertyChanged();
        RequestInterfaceReload();

        // Reload this page
        Page_LoadedHandler();
        OnPropertyChanged();
    }

    private void OptionBox_DropDownOpened(object sender, object e)
    {
        // Don't react to pre-init signals
        if (!_pageSetupFinished) return;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void OptionBox_DropDownClosed(object sender, object e)
    {
        // Don't react to pre-init signals
        if (!_pageSetupFinished) return;

        // If playing is not allowed yet
        if (_blockHiddenSoundOnce)
        {
            _blockHiddenSoundOnce = false;
            return; // Don't play the sound
        }

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }

    private void ContextTextBlock_Tapped(object sender, TappedRoutedEventArgs e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
        ContextTeachingTip.IsOpen = true;
    }

    private void ContextTeachingTip_Closing(TeachingTip sender, TeachingTipClosingEventArgs args)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }

    private async void PluginGridView_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (_blockSelectionOnce && e.AddedItems.Any() && (sender as GridView)?
            .SelectionMode is ListViewSelectionMode.Single)
        {
            ((GridView)sender).SelectedItem = null;
            _blockSelectionOnce = false;
            return; // Nothing else to do right now
        }

        // Process the change - update the list of selected devices
        SelectedDevices = (sender as GridView)?.SelectedItems
            .Select(x => ((sender as GridView)?.ContainerFromItem(x)
                as GridViewItem, x as SetupPlugin)).ToList();

        // Process the change - hide/show the 'next' button
        if (SelectedDevices?.Any() ?? false)
        {
            if (NextButtonContainer.Children.Any()) return;
            NextButtonContainer.Children.Add(NextButton);
            NextButton.Opacity = 1.0;
        }
        else
        {
            NextButton.Opacity = 0.0;
            await Task.Delay(500); // Wait for the button to hide
            NextButtonContainer.Children.Remove(NextButton);
        }
    }

    private async void NextButton_Click(object sender, RoutedEventArgs e)
    {
        // Omit this handler during setup
        if (InterfaceBlockerGrid.IsHitTestVisible) return;

        // List all setup-able devices
        var devicesWithAvailableDependencies =
            SelectedDevices.Where(x => x.Item.DependencyInstaller?.ListDependencies()
                .Any(y => !y.IsInstalled) ?? false).ToList();

        if (!devicesWithAvailableDependencies.Any())
        {
            NextButtonNextPageOnClick(sender, e);
            return; // Don't care anymore
        }

        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        InterfaceBlockerGrid.Opacity = 0.35;
        InterfaceBlockerGrid.IsHitTestVisible = true;
        NextButton.IsEnabled = false; // Disable
        await Task.Delay(200); // Wait a bit

        // Loop over all the selected devices that are applicable to any changes
        foreach (var device in devicesWithAvailableDependencies)
        {
            /* Forward animation */

            var animationGuid = Guid.NewGuid().ToString();
            var backAnimationGuid = Guid.NewGuid().ToString();

            MainPluginScrollViewer.ScrollToElement(device.Container, false);
            await Task.Delay(500); // Wait a bit here

            // Prepare for the animation
            var animation = ConnectedAnimationService.GetForCurrentView()
                .PrepareToAnimate(animationGuid, device.Container);

            device.Item.InstallHandler.HideProgress = true;
            device.Item.InstallHandler.NoProgress = true;
            device.Item.InstallHandler.ProgressIndeterminate = false;
            device.Item.InstallHandler.CirclePending = true;
            device.Item.InstallHandler.StageName =
                Interfacing.LocalizedJsonString("/Installer/Views/Setup/Dep/Select");
            device.Item.InstallHandler.OnPropertyChanged();

            animation.Configuration = new BasicConnectedAnimationConfiguration();

            device.Container.Opacity = 0.0;
            ConnectedAnimationService.GetForCurrentView()
                .GetAnimation(animationGuid)?.TryStart(DeviceSetupGrid);

            DeviceSetupGrid.Visibility = Visibility.Visible;
            DeviceSetupGrid.Opacity = 1.0;
            SetupItems.ItemsSource = new List<SetupPlugin> { device.Item };

            /* Device setup */

            DependenciesToInstall = device.Item.DependencyInstaller?.ListDependencies()
                .Where(x => x.IsMandatory && !x.IsInstalled).ToList();

            // Enable the 'next button and wait
            NextButton.Click += NextButtonOnClick;
            NextButton.IsEnabled = true;

            await NextButtonClickedSemaphore.WaitAsync();
            NextButton.Click -= NextButtonOnClick;
            NextButton.IsEnabled = false;

            // Block user input
            device.Item.DependencySetupPending = true;
            device.Item.OnPropertyChanged();

            // Loop over all dependencies and install them
            foreach (var dependency in DependenciesToInstall ?? new List<IDependency>())
            {
                eula:
                if (!string.IsNullOrEmpty(dependency.InstallerEula))
                {
                    // Set the progress indicator and title
                    device.Item.InstallHandler.NoProgress = true;
                    device.Item.InstallHandler.HideProgress = true;
                    device.Item.InstallHandler.ProgressIndeterminate = true;

                    device.Item.InstallHandler.StageName =
                        Interfacing.LocalizedJsonString("/Installer/Views/Setup/Dep/Eula");
                    device.Item.InstallHandler.OnPropertyChanged();
                    await Task.Delay(2500);

                    // Set up EULA values and open the flyout
                    EulaHeader.Text = "{0} EULA".Format(dependency.Name);
                    EulaText.Text = dependency.InstallerEula;
                    EulaFlyout.ShowAt(MainGrid);

                    // Wait for the eula flyout to be closed
                    await Shared.Main.EulaFlyoutClosed.WaitAsync();

                    // Validate the result and try again
                    if (!Shared.Main.EulaFlyoutResult) goto eula;
                }

                // The EULA must have already been accepted, install now
                device.Item.InstallHandler.NoProgress = false;
                device.Item.InstallHandler.StageName = " ";
                await device.Item.PerformDependencyInstallation(dependency);
                device.Item.InstallHandler.NoProgress = true;
            }

            /* Back animation */

            // Mark as done, wait a bit and go back
            device.Item.InstallHandler.HideProgress = true;
            device.Item.InstallHandler.NoProgress = true;
            device.Item.InstallHandler.ProgressIndeterminate = false;
            device.Item.InstallHandler.CirclePending = false;

            device.Item.InstallHandler.StageName = Interfacing
                .LocalizedJsonString("/Installer/Views/Setup/Dep/Success")
                .Format(device.Item.Name);
            device.Item.InstallHandler.OnPropertyChanged();
            await Task.Delay(2500);

            // Prepare for the animation
            var backAnimation = ConnectedAnimationService.GetForCurrentView()
                .PrepareToAnimate(backAnimationGuid, DeviceSetupGrid);

            backAnimation.Configuration = new BasicConnectedAnimationConfiguration();

            DeviceSetupGrid.Visibility = Visibility.Collapsed;
            DeviceSetupGrid.Opacity = 0.0;
            ConnectedAnimationService.GetForCurrentView()
                .GetAnimation(backAnimationGuid)?.TryStart(device.Container);

            device.Container.Opacity = 1.0;
            await Task.Delay(500); // Wait a bit here
        }

        NextButton.Click -= NextButton_Click;
        NextButton.Click += NextButtonNextPageOnClick;

        InterfaceBlockerGrid.Opacity = 0.0;
        NextButton.IsEnabled = true;
    }

    private void NextButtonOnClick(object sender, RoutedEventArgs e)
    {
        // Save selected devices to the default configuration
        try
        {
            // Overwrite the data - make Kinect devices a priority, then PSMS
            DefaultSettings.TrackingDevice = SelectedDevices
                .FirstOrDefault(x => x.Item.Guid.Contains("KINECT"), SelectedDevices
                    .FirstOrDefault(x => x.Item.Guid.Contains("PSMOVE"), SelectedDevices.First())).Item
                .Guid.Replace(":SETUP", string.Empty);
        }
        catch (Exception ex)
        {
            Logger.Error(ex);
        }

        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
        NextButtonClickedSemaphore.Release();
    }

    private async void NextButtonNextPageOnClick(object sender, RoutedEventArgs e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        MainGrid.Opacity = 0.0;
        await Task.Delay(500);

        ContinueEvent?.Invoke(this, EventArgs.Empty);
    }

    private void DependencyCheckBox_Toggled(object sender, RoutedEventArgs e)
    {
        if (((sender as CheckBox)?.DataContext as IDependency) is null) return;
        AppSounds.PlayAppSound(((CheckBox)sender).IsChecked ?? false
            ? AppSounds.AppSoundType.ToggleOn
            : AppSounds.AppSoundType.ToggleOff);

        // Find the dependency that's been enabled or disabled
        var dependency = ((CheckBox)sender).DataContext as IDependency;

        // Add or remove it, depending on the checkbox value
        if (((CheckBox)sender).IsChecked ?? false)
            DependenciesToInstall.Add(dependency);
        else DependenciesToInstall.Remove(dependency);
    }

    private void PluginGridView_ItemClick(object sender, ItemClickEventArgs e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke); // Also play a sound
        if ((sender as GridView)?.SelectionMode is ListViewSelectionMode.Single &&
            e.ClickedItem != ((GridView)sender).SelectedItem) return;

        _blockSelectionOnce = true;
        ((GridView)sender).SelectedItem = null;
    }

    private void EulaFlyout_Opening(object sender, object e)
    {
        InterfaceBlockerGrid.Opacity = 0.35;
        InterfaceBlockerGrid.IsHitTestVisible = true;
        Shared.Main.EulaFlyoutResult = false;
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void EulaFlyout_Closed(object sender, object e)
    {
        InterfaceBlockerGrid.Opacity = 0.0;
        InterfaceBlockerGrid.IsHitTestVisible = false;
        Shared.Main.EulaFlyoutClosed?.Release();
    }

    private void EulaFlyout_Closing(object sender, object e)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }

    private void AcceptEulaButton_Click(object sender, RoutedEventArgs e)
    {
        // Set the result
        Shared.Main.EulaFlyoutResult = true;

        // Close the EULA flyout
        EulaFlyout.Hide();
    }

    private void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }
}