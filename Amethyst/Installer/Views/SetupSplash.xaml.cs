using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Threading.Tasks;
using Amethyst.Classes;
using Amethyst.Installer.ViewModels;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using static Amethyst.Classes.Shared.Events;

namespace Amethyst.Installer.Views;

public sealed partial class SetupSplash : Page, INotifyPropertyChanged
{
    private readonly List<string> _languageList = [];

    private bool _blockHiddenSoundOnce;
    private bool _pageSetupFinished, _pageLoadedOnce;

    public bool AnimateEnding { get; set; } = true;
    public TimeSpan? ShowWait { get; set; } = null;

    public SetupSplash()
    {
        InitializeComponent();

        Logger.Info($"Constructing page: '{GetType().FullName}'...");
        Logger.Info("Registering a detached binary semaphore " +
                    $"reload handler for '{GetType().FullName}'...");

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

    public ICustomSplash Splash { get; set; }

    public event PropertyChangedEventHandler PropertyChanged;

    private async void Page_Loaded(object sender, RoutedEventArgs e)
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

        if (!_pageLoadedOnce && (Splash?.ShowVideo ?? false))
        {
            BackgroundVideoElement.MediaPlayer.IsLoopingEnabled = true;
            BackgroundVideoElement.MediaPlayer.Play();
        }

        // Execute the handler
        Page_LoadedHandler();

        // Mark as loaded
        _pageLoadedOnce = true;

        if (ShowWait.HasValue) await Task.Delay(ShowWait.Value);
        MainGrid.Opacity = 1.0; // Show the splash now
    }

    private void Page_LoadedHandler()
    {
        // Clear available languages' list
        _pageSetupFinished = false;
        LanguageOptionBox.Items.Clear();
        _languageList.Clear();

        // Push all the found languages
        if (Interfacing.GetAvailableResourceLanguages(entry =>
            {
                _languageList.Add(Path.GetFileNameWithoutExtension(entry));
                LanguageOptionBox.Items.Add(Interfacing.GetLocalizedLanguageName(
                    Path.GetFileNameWithoutExtension(entry)));

                if (Path.GetFileNameWithoutExtension(entry) == AppData.Settings.AppLanguage)
                    LanguageOptionBox.SelectedIndex = LanguageOptionBox.Items.Count - 1;
            }).Count <= 0) return;

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

    private async void ActionButton_Click(object sender, RoutedEventArgs e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        if (AnimateEnding)
        {
            MainGrid.Opacity = 0.0;
            await Task.Delay(500);
        }

        await Splash.Action();
    }

    private async void BottomTextBlock_Tapped(object sender, Microsoft.UI.Xaml.Input.TappedRoutedEventArgs e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
        await Splash.BottomTextAction();
    }

    private void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }
}