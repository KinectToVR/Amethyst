using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Windows.Storage;
using Amethyst.Classes;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using WinUI.Fluent.Icons;
using Windows.ApplicationModel.DataTransfer;

namespace Amethyst.Popups;

public sealed partial class Report : Page, INotifyPropertyChanged
{
    private readonly List<string> _languageList = [];

    private bool _blockHiddenSoundOnce;
    private bool _reportPageSetupFinished, _reportPageLoadedOnce;

    [SetsRequiredMembers]
    public Report(List<AppDataFile> dataFiles)
    {
        InitializeComponent();

        Logger.Info($"Constructing page: '{GetType().FullName}'...");
        CollectedFiles = new ObservableCollection<AppDataFile>(dataFiles);

        Logger.Info("Registering a detached binary semaphore " +
                    $"reload handler for '{GetType().FullName}'...");

        Task.Run(() =>
        {
            while (true)
            {
                // Wait for a reload signal (blocking)
                Shared.Events.ReloadVendorPagesEvent.WaitOne();

                // Reload & restart the waiting loop
                if (_reportPageLoadedOnce && Interfacing.CurrentAppState == "blocked")
                    DispatcherQueue.TryEnqueue(Page_LoadedHandler);

                // Reset the event
                Shared.Events.ReloadPluginsPageEvent.Reset();
            }
        });
    }

    private bool IsElevated => FileUtils.IsCurrentProcessElevated();
    private bool IsNotElevated => !IsElevated;
    private double DragGridOpacity => IsElevated ? 0.5 : 1.0;

    private string DragCaption => IsElevated
        ? Interfacing.LocalizedJsonString("/ReportPage/Captions/Elevated")
        : Interfacing.LocalizedJsonString("/ReportPage/Captions/Drop");

    public required Host ParentWindow { get; set; }
    public required ObservableCollection<AppDataFile> CollectedFiles { get; set; }

    public event PropertyChangedEventHandler PropertyChanged;

    private void Page_Loaded(object sender, RoutedEventArgs e)
    {
        Logger.Info($"Re/Loading page: '{GetType().FullName}'...");
        Interfacing.CurrentAppState = "blocked";

        // Execute the handler
        Page_LoadedHandler();

        // Mark as loaded
        _reportPageLoadedOnce = true;
    }

    private void Page_LoadedHandler()
    {
        // Clear available languages' list
        _reportPageSetupFinished = false;
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
        _reportPageSetupFinished = true;

        // Reload
        OnPropertyChanged();
    }

    private void LanguageOptionBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        // Don't react to pre-init signals
        if (!_reportPageSetupFinished) return;
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
        Shared.Events.RequestInterfaceReload();

        // Reload this page
        Page_LoadedHandler();
        OnPropertyChanged();
    }

    private void OptionBox_DropDownOpened(object sender, object e)
    {
        // Don't react to pre-init signals
        if (!_reportPageSetupFinished) return;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void OptionBox_DropDownClosed(object sender, object e)
    {
        // Don't react to pre-init signals
        if (!_reportPageSetupFinished) return;

        // If playing is not allowed yet
        if (_blockHiddenSoundOnce)
        {
            _blockHiddenSoundOnce = false;
            return; // Don't play the sound
        }

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }

    private void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    private void FilesGrid_DragStarting(UIElement sender, DragStartingEventArgs args)
    {
        try
        {
            Logger.Info("Copying collected data files to the drag context...");
            args.Data.SetStorageItems(CollectedFiles.Select(x => x.DataFile));
        }
        catch (Exception e)
        {
            Logger.Error(e);
        }
    }

    private void CopyFilesButton_Click(object sender, RoutedEventArgs e)
    {
        try
        {
            Logger.Info("Copying collected data files to a new data package...");
            var clipboardData = new DataPackage
            {
                RequestedOperation = DataPackageOperation.Copy
            };
            clipboardData.SetStorageItems(CollectedFiles.Select(x => x.DataFile));

            Logger.Info("Copying collected data files to clipboard...");
            Clipboard.SetContent(clipboardData);
        }
        catch (Exception ex)
        {
            Logger.Error(ex);
        }
    }
}

public class AppDataFile(StorageFile file)
{
    public StorageFile DataFile { get; } = file;
    public string Name => DataFile.Name;
    public bool IsLog => Name.EndsWith(".log") || Name.EndsWith(".txt");

    public FluentSymbol Icon => IsLog
        ? FluentSymbol.DocumentText24
        : FluentSymbol.TableSettings24;
}