// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

using System.ComponentModel;
using System.Threading;
using System.Threading.Tasks;
using Amethyst.Classes;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
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
            Shared.Semaphores.ReloadInfoPageSemaphore =
                new Semaphore(0, 1);

            while (true)
            {
                // Wait for a reload signal (blocking)
                Shared.Semaphores.ReloadInfoPageSemaphore.WaitOne();

                // Reload & restart the waiting loop
                if (_infoPageLoadedOnce)
                    Shared.Main.DispatcherQueue.TryEnqueue(Page_LoadedHandler);

                Task.Delay(100); // Sleep a bit
            }
        });
    }

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

        // Reset the next page layout (if ever changed)
        Shared.Settings.PageMainScrollViewer?.ScrollToVerticalOffset(0);

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

    private void HelpTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        EndingTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        EndingTeachingTip.IsOpen = true;
    }

    private async void EndingTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Dismiss the current tip
        EndingTeachingTip.IsOpen = false;
        await Task.Delay(200);

        // Unblock the interface
        Shared.Main.InterfaceBlockerGrid.Opacity = 0.0;
        Shared.Main.InterfaceBlockerGrid.IsHitTestVisible = false;

        Interfacing.IsNuxPending = false;

        // Navigate to the general page
        Shared.Main.MainNavigationView.SelectedItem =
            Shared.Main.MainNavigationView.MenuItems[0];
        Shared.Main.NavigateToPage("general",
            new EntranceNavigationTransitionInfo());

        // We're done
        AppData.Settings.FirstTimeTourShown = true;
        AppData.Settings.SaveSettings();
    }

    private void K2DoubleTapped(object sender, DoubleTappedRoutedEventArgs e)
    {
        // Show a console-text-box popup
    }

    // MVVM stuff
    public event PropertyChangedEventHandler PropertyChanged;

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

}