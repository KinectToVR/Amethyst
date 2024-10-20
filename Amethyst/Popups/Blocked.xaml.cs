using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Amethyst.Classes;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media.Imaging;
using WinUI.Fluent.Icons;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Popups;

/// <summary>
///     An empty page that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class Blocked : Page, INotifyPropertyChanged
{
    private readonly List<string> _languageList = new();
    private bool _blockedPageSetupFinished, _blockedPageLoadedOnce;

    private bool _blockHiddenSoundOnce;

    [SetsRequiredMembers]
    public Blocked(string blockedPluginName, List<BlockingProcess> blockers)
    {
        InitializeComponent();

        Logger.Info($"Constructing page: '{GetType().FullName}'...");

        BlockedPluginName = blockedPluginName;
        Blockers = new ObservableCollection<BlockingProcess>(blockers);

        Logger.Info("Registering a detached binary semaphore " +
                    $"reload handler for '{GetType().FullName}'...");

        Task.Run(() =>
        {
            while (true)
            {
                // Wait for a reload signal (blocking)
                Shared.Events.ReloadVendorPagesEvent.WaitOne();

                // Reload & restart the waiting loop
                if (_blockedPageLoadedOnce && Interfacing.CurrentAppState == "blocked")
                    DispatcherQueue.TryEnqueue(Page_LoadedHandler);

                // Reset the event
                Shared.Events.ReloadPluginsPageEvent.Reset();
            }
        });

        Blockers.ToList().ForEach(x =>
            Task.Run(async () =>
            {
                while (true)
                {
                    if (x.Process.HasExited)
                    {
                        DispatcherQueue.TryEnqueue(() => ProcessOnExited(this, new DoWorkEventArgs(x)));
                        return; // That's all, exit this thread to save system resources
                    }

                    // Wait for process shutdown
                    await Task.Delay(1000);
                }
            }));
    }

    public required string BlockedPluginName { get; set; }
    public required ObservableCollection<BlockingProcess> Blockers { get; set; }
    public required Host ParentWindow { get; set; }

    private Process IndexProcess { get; set; }

    private string SkipUpdatingText => Interfacing.LocalizedJsonString(
        "/BlockedPage/Buttons/Skip").Format(BlockedPluginName);

    private string BlockedHeaderText => Interfacing.LocalizedJsonString(
        "/BlockedPage/Headers/Blocked").Format(BlockedPluginName);

    private string BlockedMessageText => Interfacing.LocalizedJsonString(
        "/BlockedPage/Contents/Blocked").Format(BlockedPluginName);

    public event PropertyChangedEventHandler PropertyChanged;

    private void ProcessOnExited(object sender, DoWorkEventArgs e)
    {
        // Remove this process from the waiting queue
        Blockers.Remove(e.Argument as BlockingProcess);
        OnPropertyChanged(); // Refresh this data view

        // Check if all processes exited, continue
        if (Blockers.Any()) return;

        // Set success, close the parent window
        ParentWindow.Result = true;
        ParentWindow.Close();
    }

    private void Page_Loaded(object sender, RoutedEventArgs e)
    {
        Logger.Info($"Re/Loading page: '{GetType().FullName}'...");
        Interfacing.CurrentAppState = "blocked";

        // Execute the handler
        Page_LoadedHandler();

        // Mark as loaded
        _blockedPageLoadedOnce = true;
    }

    private void Page_LoadedHandler()
    {
        // Clear available languages' list
        _blockedPageSetupFinished = false;
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
        _blockedPageSetupFinished = true;

        // Reload
        OnPropertyChanged();
    }

    private void LanguageOptionBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        // Don't react to pre-init signals
        if (!_blockedPageSetupFinished) return;
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
        if (!_blockedPageSetupFinished) return;

        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    private void OptionBox_DropDownClosed(object sender, object e)
    {
        // Don't react to pre-init signals
        if (!_blockedPageSetupFinished) return;

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

    private void KillProcessButton_Click(object sender, RoutedEventArgs e)
    {
        if ((sender as Button)?.DataContext is not BlockingProcess process) return;

        // If the blocking process is running as administrator, show an additional dialog
        if (process.IsElevated)
        {
            IndexProcess = process.Process; // Reference the process for children views
            PermissionsFlyout_Open.ShowAt(((Button)sender).Parent as FrameworkElement);
        }

        // Else try to kill it
        else
        {
            try
            {
                // If we want to kill server, shut down monitor first
                if (process.Process.ProcessName == "vrserver")
                    Process.GetProcessesByName("vrmonitor").FirstOrDefault()?.Kill();

                process.Process.Kill(); // Now kill the actual process
            }
            catch (Exception ex)
            {
                Logger.Error(ex);
            }
        }
    }

    private void CancelElevationButton_Click(object sender, RoutedEventArgs e)
    {
        // Hide the flyout, cancelled by user
        PermissionsFlyout_Open.Hide();
    }

    private void ElevatedKillButton_Click(object sender, RoutedEventArgs e)
    {
        Logger.Info("Restart requested: trying to restart the app...");

        // If we've found who asked
        if (!File.Exists(Interfacing.ProgramLocation.FullName)) return;

        // Log the caller
        Logger.Info($"The current caller process is: {Interfacing.ProgramLocation.FullName}");

        // Start amethyst process kill slave
        Process.Start(new ProcessStartInfo
        {
            // Pass same args
            FileName = Interfacing.ProgramLocation.FullName.Replace(".dll", ".exe"),
            Arguments = $"Kill {IndexProcess.Id}",

            UseShellExecute = true,
            Verb = "runas" // Force UAC prompt
        });
    }

    private void CancelUpdateButton_Click(object sender, RoutedEventArgs e)
    {
        // Close the parent window
        ParentWindow.Close();
    }

    private void FluentSymbolIcon_Tapped(object sender, TappedRoutedEventArgs e)
    {
        PermissionsFlyout_Info.ShowAt(sender as FluentSymbolIcon);
    }
}

public class BlockingProcess : INotifyPropertyChanged
{
    public FileInfo ProcessPath { get; set; }
    public Process Process { get; set; }
    public bool IsElevated { get; set; }

    public BitmapImage ProcessImage
    {
        get
        {
            if (!ProcessPath.Exists) return null;

            // Stolen right away from PowerToys FileLocksmith
            var bitmap = Icon.ExtractAssociatedIcon(ProcessPath.FullName)?.ToBitmap();
            if (bitmap is null) return new BitmapImage();

            var bitmapImage = new BitmapImage();
            using var stream = new MemoryStream();
            bitmap.Save(stream, ImageFormat.Png);
            stream.Position = 0;
            bitmapImage.SetSource(stream.AsRandomAccessStream());

            return bitmapImage;
        }
    }

    public bool ProcessInvalid => !ProcessPath.Exists;
    public bool IsNotElevated => !IsElevated;
    public event PropertyChangedEventHandler PropertyChanged;

    protected virtual void OnPropertyChanged(string propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}