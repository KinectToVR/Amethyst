// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Windows.System;
using Amethyst.Classes;
using Amethyst.Plugins.Contract;
using Amethyst.Utils;
using Microsoft.AppCenter.Analytics;
using Microsoft.AppCenter.Crashes;
using Microsoft.CodeAnalysis.CSharp.Scripting;
using Microsoft.CodeAnalysis.Scripting;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media.Animation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Pages;

/// <summary>
///     An empty page that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class Info : Page, INotifyPropertyChanged
{
    private bool _commandConfirmLocked;
    private bool _infoPageLoadedOnce;

    public Info()
    {
        InitializeComponent();

        Logger.Info($"Constructing page: '{GetType().FullName}'...");
        Shared.TeachingTips.InfoPage.HelpTeachingTip = HelpTeachingTip;

        Logger.Info("Registering a detached binary semaphore " +
                    $"reload handler for '{GetType().FullName}'...");

        Task.Run(() =>
        {
            Shared.Events.ReloadInfoPageEvent =
                new ManualResetEvent(false);

            while (true)
            {
                // Wait for a reload signal (blocking)
                Shared.Events.ReloadInfoPageEvent.WaitOne();

                // Reload & restart the waiting loop
                if (_infoPageLoadedOnce && Interfacing.CurrentPageTag == "info")
                    Shared.Main.DispatcherQueue.TryEnqueue(Page_LoadedHandler);

                // Reset the event
                Shared.Events.ReloadInfoPageEvent.Reset();
            }
        });
    }

    // MVVM stuff
    public event PropertyChangedEventHandler PropertyChanged;

    private void Page_Loaded(object sender, RoutedEventArgs e)
    {
        Logger.Info($"Re/Loading page: '{GetType().FullName}'...");
        Interfacing.CurrentAppState = "info";

        // Execute the handler
        Page_LoadedHandler();

        // Mark as loaded
        _infoPageLoadedOnce = true;
    }

    private void Page_LoadedHandler()
    {
        OnPropertyChanged(); // Just everything
    }

    private async void HelpTeachingTip_ActionButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Dismiss the current tip
        HelpTeachingTip.IsOpen = false;
        await Task.Delay(400);

        // Navigate to the devices page
        Shared.Main.MainNavigationView.SelectedItem =
            Shared.Main.MainNavigationView.MenuItems[2];
        Shared.Main.NavigateToPage("devices",
            new EntranceNavigationTransitionInfo());

        // Wait a bit
        await Task.Delay(500);

        // Show the next tip
        Shared.TeachingTips.DevicesPage.DeviceControlsTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        Shared.TeachingTips.DevicesPage.DeviceControlsTeachingTip.IsOpen = true;
    }

    private async void HelpTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Navigate to the plugin page
        Shared.Main.MainNavigationView.SelectedItem =
            Shared.Main.MainNavigationView.FooterMenuItems[0];
        Shared.Main.NavigateToPage("plugins",
            new EntranceNavigationTransitionInfo());

        // Wait a bit
        await Task.Delay(500);

        // Show the next tip
        Shared.TeachingTips.PluginsPage.ManagerTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        Shared.TeachingTips.PluginsPage.ManagerTeachingTip.IsOpen = true;
    }

    private void K2DoubleTapped(object sender, DoubleTappedRoutedEventArgs e)
    {
        // Show a console-text-box popup
        CommandFlyout.ShowAt(TargetGrid);
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    private async void CommandTextBox_KeyDown(object sender, KeyRoutedEventArgs e)
    {
        if (e.Key == VirtualKey.Enter && !string.IsNullOrEmpty((sender as TextBox)?.Text))
        {
            if (_commandConfirmLocked) return;
            _commandConfirmLocked = true; // Don't re-accept now!
            AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

            // Check starts-with first : toast?
            if (((TextBox)sender).Text.ToLowerInvariant().StartsWith("toast "))
            {
                Interfacing.ShowToast(
                    (((TextBox)sender).Text.Contains('/') // Check if there's any description content preset
                        ? ((TextBox)sender).Text[
                            "toast ".Length..((TextBox)sender).Text.IndexOf("/", StringComparison.Ordinal)]
                        : ((TextBox)sender).Text["toast ".Length..])?.Trim(), // Don't show toast text if invalid
                    ((TextBox)sender).Text.Contains('/') // Check if there's any description content preset
                        ? ((TextBox)sender).Text[(((TextBox)sender).Text.IndexOf("/", StringComparison.Ordinal) + 1)..]
                        : "No toast description provided."); // Don't show toast description if invalid

                SetCommandText($"Toast with payload \"{((TextBox)sender).Text["toast ".Length..]}\" sent!");
                return; // Nothing else to do!
            }

            // Check starts-with first : crash/exit/fail?
            if (((TextBox)sender).Text.ToLowerInvariant().StartsWith("exit ") ||
                ((TextBox)sender).Text.ToLowerInvariant().StartsWith("fail "))
            {
                if (int.TryParse(((TextBox)sender).Text["exit ".Length..].Trim(), out var code))
                {
                    // Exit with a code to be parsed by the crash handler
                    Logger.Info($"Exit with code \"{((TextBox)sender).Text["exit ".Length..].Trim()}\" requested!");
                    Interfacing.IsExitHandled = true;
                    Environment.Exit(code); // Prepare and exit
                }
                else
                {
                    // Exit with a custom message to be shown by the crash handler
                    Logger.Info($"Exit with payload \"{((TextBox)sender).Text["exit ".Length..].Trim()}\" requested!");
                    Interfacing.Fail(((TextBox)sender).Text["exit ".Length..].Trim());
                }

                SetCommandText("Sending a crash signal failed for some reason!");
                return; // Nothing else to do!
            }

            // Check starts-with first : crash/exit/fail?
            if (((TextBox)sender).Text.ToLowerInvariant().StartsWith("eval "))
            {
                // Exit with a custom message to be shown by the crash handler
                Logger.Info($"Trying to evaluate expression \"{((TextBox)sender).Text["eval ".Length..].Trim()}\"...");

                try
                {
                    SetCommandText(CSharpScript.EvaluateAsync(((TextBox)sender).Text["eval ".Length..].Trim(),
                            ScriptOptions.Default.WithImports("Amethyst.Classes")
                                .WithReferences(typeof(Interfacing).Assembly).AddImports("System.Linq"))
                        .Result.ToString());
                }
                catch (Exception ex)
                {
                    SetCommandText($"Evaluation error: '{ex}'");
                }

                return; // Nothing else to do!
            }

            // Parse the passed command now
            switch (((TextBox)sender).Text.ToLowerInvariant())
            {
                case "alltrackers" or "all trackers" or "all":
                {
                    // De-spawn all supported-enabled trackers
                    Interfacing.AppTrackersInitialized = false;
                    await Task.Delay(200); // Sleep a bit

                    lock (Interfacing.UpdateLock)
                    {
                        // Add all create-able trackers
                        Enum.GetValues<TrackerType>().ToList().ForEach(role =>
                            AppData.Settings.TrackersVector.Add(new AppTracker
                            {
                                Role = role, IsActive = true,
                                Serial = TypeUtils.TrackerTypeRoleSerialDictionary[role]
                            }));

                        // Remove duplicates and save
                        ConfigCheck(); // Check all
                    }

                    SetCommandText("Trackers added, type 'respawn' to spawn them!");
                    break;
                }
                case "resettrackers" or "reset trackers" or "default":
                {
                    // De-spawn all supported-enabled trackers
                    Interfacing.AppTrackersInitialized = false;
                    await Task.Delay(200); // Sleep a bit

                    lock (Interfacing.UpdateLock)
                    {
                        // Add all create-able trackers
                        AppData.Settings.TrackersVector.Clear();
                        ConfigCheck(); // Check all
                    }

                    SetCommandText("Trackers reset, type 'spawn' to enable them!");
                    break;
                }
                case "respawn" or "spawn" or "on":
                {
                    // Try spawning all supported-enabled trackers
                    var success = await Interfacing.SpawnEnabledTrackers();

                    // Set up the co/re/disconnect button
                    Shared.General.ToggleTrackersButton.IsChecked = success;
                    Shared.General.ToggleTrackersButtonText.Text =
                        Interfacing.LocalizedJsonString(success
                            ? "/GeneralPage/Buttons/TrackersToggle/Disconnect"
                            : "/GeneralPage/Buttons/TrackersToggle/Reconnect");

                    SetCommandText("Trackers spawned, type 'despawn' to disable them!");
                    break;
                }
                case "despawn" or "off":
                {
                    // De-spawn all supported-enabled trackers
                    Interfacing.AppTrackersInitialized = false;
                    await Task.Delay(200); // Sleep a bit

                    // Set up the co/re/disconnect button
                    Shared.General.ToggleTrackersButton.IsChecked = false;
                    Shared.General.ToggleTrackersButtonText.Text =
                        Interfacing.LocalizedJsonString(Interfacing.AppTrackersSpawned
                            ? "/GeneralPage/Buttons/TrackersToggle/Reconnect"
                            : "/GeneralPage/Buttons/TrackersToggle/Connect");

                    SetCommandText("Trackers despawned, type 'spawn' to enable them!");
                    break; // That's all for now!
                }
                case "reset":
                {
                    // De-spawn all already-added trackers
                    Interfacing.AppTrackersInitialized = false;
                    await Task.Delay(200); // Sleep a bit

                    // Set up the co/re/disconnect button
                    Shared.General.ToggleTrackersButton.IsChecked = false;
                    Shared.General.ToggleTrackersButtonText.Text =
                        Interfacing.LocalizedJsonString(Interfacing.AppTrackersSpawned
                            ? "/GeneralPage/Buttons/TrackersToggle/Reconnect"
                            : "/GeneralPage/Buttons/TrackersToggle/Connect");

                    // Reset application settings
                    Logger.Info("Reset has been invoked: turning trackers off...");

                    // Play a sound
                    AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

                    // Mark trackers as inactive
                    Interfacing.AppTrackersInitialized = false;
                    Shared.General.ToggleTrackersButton.IsChecked = false;

                    Logger.Info("Reset has been invoked: clearing app settings...");

                    // Mark exiting as true
                    Interfacing.IsExitingNow = true;
                    await Task.Delay(50);

                    // Read settings after reset
                    AppData.Settings = new AppSettings(); // Reset settings
                    AppData.Settings.SaveSettings(); // Save empty settings

                    /* Restart */

                    Logger.Info("Reset invoked: trying to restart the app...");

                    // Restart and exit with code 0
                    await Interfacing.ExecuteAppRestart();

                    // Still here?
                    Logger.Fatal(new InvalidDataException(
                        "App will not be restarted due to caller process identification error."));

                    Interfacing.ShowToast(
                        Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed/Title"),
                        Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed"));
                    Interfacing.ShowServiceToast(
                        Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed/Title"),
                        Interfacing.LocalizedJsonString("/SharedStrings/Toasts/RestartFailed"));

                    SetCommandText("Settings reset, type 'spawn' to enable trackers!");
                    break; // That's all for now!
                }
                case "restartnotice" or "restartbar":
                {
                    Shared.TeachingTips.MainPage.ReloadInfoBar.IsOpen =
                        !Shared.TeachingTips.MainPage.ReloadInfoBar.IsOpen;
                    Shared.TeachingTips.MainPage.ReloadInfoBar.Opacity =
                        Shared.TeachingTips.MainPage.ReloadInfoBar.IsOpen ? 1 : 0;
                    break; // That's all for now!
                }
                case "update":
                {
                    await MainWindow.RequestUpdateEvent(this, EventArgs.Empty);
                    SetCommandText($"Sent an update signal to the '{typeof(MainWindow).FullName}' handler!");
                    break; // That's all for now!
                }
                case "uwu" or "owo" or "omo" or "umu":
                {
                    Interfacing.ShowToast(((TextBox)sender).Text
                        .ToUpperInvariant(), null, true);
                    SetCommandText("Ugh, what else...?"); // Uh, okay I guess?
                    break;
                }
                case "hello" or "hi" or "yo":
                {
                    SetCommandText("Hello!");
                    break; // Why does this exist...
                }
                default:
                {
                    SetCommandText($"Invalid command: '{((TextBox)sender).Text}'!");
                    break; // The command was invalid, give up on it for now
                }
            }

            return; // Nothing else to do!
        }

        // Reset the confirmation flag
        if (_commandConfirmLocked)
            _commandConfirmLocked = false;
    }

    private void ConfigCheck()
    {
        // Reload everything we can
        Shared.Devices.DevicesJointsValid = false;

        // Check app settings and save
        AppData.Settings.CheckSettings();
        AppData.Settings.SaveSettings();

        // Request page reloads
        Translator.Get.OnPropertyChanged();
        Shared.Events.RequestInterfaceReload();

        // Request manager reloads
        AppData.Settings.OnPropertyChanged();
        AppData.Settings.TrackersVector.ToList()
            .ForEach(x => x.OnPropertyChanged());

        // We're done with our changes now!
        Shared.Devices.DevicesJointsValid = true;
    }

    private void SetCommandText(string text)
    {
        if (CommandTextBox is null) return;
        CommandTextBox.PlaceholderText = text;
        CommandTextBox.Text = null; // Reset
    }

    private void CommandFlyout_Opening(object sender, object e)
    {
        SetCommandText("Type command:");
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private async void TelemetryToggleSwitch_Toggled(object sender, RoutedEventArgs e)
    {
        if (!_infoPageLoadedOnce) return;
        AppData.Settings.IsTelemetryEnabled = (sender as ToggleSwitch)?.IsOn ?? true;
        AppData.Settings.SaveSettings(); // Save our made changes

        await Analytics.SetEnabledAsync(AppData.Settings.IsTelemetryEnabled);
        await Crashes.SetEnabledAsync(AppData.Settings.IsTelemetryEnabled);

        AppSounds.PlayAppSound(AppData.Settings.IsTelemetryEnabled
            ? AppSounds.AppSoundType.ToggleOn
            : AppSounds.AppSoundType.ToggleOff);
    }

    private void TelemetryTextBlock_Tapped(object sender, TappedRoutedEventArgs e)
    {
        TelemetryFlyout.ShowAt(sender as TextBlock, new FlyoutShowOptions
        {
            Placement = FlyoutPlacementMode.Top,
            ShowMode = FlyoutShowMode.TransientWithDismissOnPointerMoveAway
        });
    }

    private void TelemetryFlyout_Opening(object sender, object e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void TelemetryFlyout_Closing(FlyoutBase sender, FlyoutBaseClosingEventArgs args)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }

    private void CommandFlyout_Closing(FlyoutBase sender, FlyoutBaseClosingEventArgs args)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }
}