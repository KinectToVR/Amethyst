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
using Microsoft.UI.Xaml.Input;
using static Amethyst.Classes.Shared.Events;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Installer.Views;

/// <summary>
///     An empty page that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class SetupError : Page, INotifyPropertyChanged
{
    private readonly List<string> _languageList = new();

    private bool _blockHiddenSoundOnce;
    private bool _pageSetupFinished, _pageLoadedOnce;

    public SetupError()
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

    public ICustomError Error { get; set; }
    public RaisedEvent ContinueEvent { get; set; }

    public event PropertyChangedEventHandler PropertyChanged;

    private void Page_Loaded(object sender, RoutedEventArgs e)
    {
        Logger.Info($"Re/Loading page: '{GetType().FullName}'...");

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

    private async void ContinueTextBlock_Tapped(object sender, TappedRoutedEventArgs e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        MainGrid.Opacity = 0.0;
        await Task.Delay(500);

        ContinueEvent?.Invoke(this, EventArgs.Empty);
    }

    private async void ActionButton_Click(object sender, RoutedEventArgs e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        await Error.Action();
    }

    private void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }
}