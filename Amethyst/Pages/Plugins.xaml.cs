// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Threading;
using System.Threading.Tasks;
using Windows.ApplicationModel.DataTransfer;
using Windows.Storage;
using Windows.System;
using Amethyst.Classes;
using Amethyst.MVVM;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media.Animation;
using Newtonsoft.Json.Linq;
using RestSharp;
using static Amethyst.Classes.Interfacing;
using static Amethyst.MVVM.StorePlugin;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Amethyst.Pages;

/// <summary>
///     An empty page that can be used on its own or navigated to within a Frame.
/// </summary>
public sealed partial class Plugins : Page, INotifyPropertyChanged
{
    public delegate void RequestShowRateExceeded(object sender, EventArgs e);

    public static RequestShowRateExceeded RequestShowRateExceededEvent;
    private bool _pluginsPageLoadedOnce;

    public Plugins()
    {
        InitializeComponent();

        Logger.Info($"Constructing page: '{GetType().FullName}'...");

        Shared.TeachingTips.PluginsPage.ManagerTeachingTip = PluginsListTeachingTip;
        Shared.TeachingTips.PluginsPage.StoreTeachingTip = PluginsStoreTeachingTip;

        RequestShowRateExceededEvent += (_, _) =>
            Shared.Main.DispatcherQueue.TryEnqueue(() =>
            {
                // API rate exceeded, show the authorization toast
                SearchPlaceholderGrid.Opacity = 0.0;
                SearchResultsGrid.Opacity = 0.0;
                SearchErrorGrid.Opacity = 1.0;
                NoResultsGrid.Opacity = 0.0;

                SearchButton.IsEnabled = false;
                SearchTextBox.IsEnabled = false;

                ResultForbiddenGrid.Visibility = Visibility.Visible;
                AppSounds.PlayAppSound(AppSounds.AppSoundType.Error);

                // Invalidate the token in settings
                AppData.Settings.GitHubToken = (false, string.Empty);
                AppData.Settings.SaveSettings();
            });

        Logger.Info("Registering a detached binary semaphore " +
                    $"reload handler for '{GetType().FullName}'...");

        Task.Run(() =>
        {
            Shared.Events.ReloadPluginsPageEvent =
                new ManualResetEvent(false);

            while (true)
            {
                // Wait for a reload signal (blocking)
                Shared.Events.ReloadPluginsPageEvent.WaitOne();

                // Reload & restart the waiting loop
                if (_pluginsPageLoadedOnce && CurrentPageTag == "plugins")
                    Shared.Main.DispatcherQueue.TryEnqueue(Page_LoadedHandler);

                // Reset the event
                Shared.Events.ReloadPluginsPageEvent.Reset();
            }
        });
    }

    public bool LoadingData { get; set; }
    public bool FinishedLoadingData { get; set; } = true;

    private RestClient ApiClient { get; } = new("https://api.github.com");

    private RestClient GithubClient { get; } = new("https://github.com");

    public ObservableCollection<StorePlugin> FoundStorePluginsList { get; set; } = new();

    // MVVM stuff
    public event PropertyChangedEventHandler PropertyChanged;

    private void Page_Loaded(object sender, RoutedEventArgs e)
    {
        Logger.Info($"Re/Loading page: '{GetType().FullName}'...");
        CurrentAppState = "plugins";

        // Execute the handler
        Page_LoadedHandler();

        // Mark as loaded
        _pluginsPageLoadedOnce = true;
    }

    private void Page_LoadedHandler()
    {
        OnPropertyChanged(); // Just everything
    }

    private void OnPropertyChanged(string propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    private async void SearchButton_Click(object sender, RoutedEventArgs e)
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
        await ProcessQuery(SearchTextBox.Text); // Search using our query
    }

    private async void SearchTextBox_KeyDown(object sender, KeyRoutedEventArgs e)
    {
        if (e.Key != VirtualKey.Enter) return; // Discard non-confirms
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
        await ProcessQuery((sender as TextBox)?.Text);
    }

    private async Task ProcessQuery(string query)
    {
        if (query.EndsWith(".zip") && Uri.TryCreate(query, UriKind.Absolute, out var uri))
            // The provided text is a link to the plugin zip, process as drag-and-drop
            await InstallRemoteZipPlugin(uri);
        else if (query.StartsWith("https://github.com/") || query.StartsWith("http://github.com/"))
            await ExecuteSearch(query // The provided text is a link
                .Replace("https://github.com/", string.Empty)
                .Replace("http://github.com/", string.Empty));
        else
            // Only a search query
            await ExecuteSearch(query);
    }

    private async Task ExecuteSearch(string query)
    {
        try
        {
            // Block the resource controls
            SearchButton.IsEnabled = false;
            SearchTextBox.IsEnabled = false;

            SearchPlaceholderGrid.Opacity = 0.0;
            SearchResultsGrid.Opacity = 0.0;
            SearchErrorGrid.Opacity = 0.0;
            NoResultsGrid.Opacity = 0.0;

            // Mark as loading and trigger a partial refresh
            LoadingData = true;
            FinishedLoadingData = false;
            OnPropertyChanged("LoadingData");
            OnPropertyChanged("FinishedLoadingData");

            // FoundStorePluginsList
            var searchResponse = await ApiClient.GetAsyncAuthorized(
                new RestRequest("search/repositories")
                    .AddQueryParameter("q", $"topic:amethyst-plugin+{query}", false));

            // Check if the results are valid
            if (searchResponse.Content is not null && searchResponse.IsSuccessStatusCode)
            {
                // Clear all the results
                FoundStorePluginsList.Clear();

                // Parse all the retrieved items
                JObject.Parse(searchResponse.Content)["items"]?
                    .Children().ToList().ForEach(x =>
                    {
                        // Add a new item to the list
                        FoundStorePluginsList.Add(new StorePlugin
                        {
                            Name = x["name"]?.ToString() ?? string.Empty,
                            Repository = new PluginRepository
                            {
                                Name = x["name"]?.ToString() ?? string.Empty,
                                Url = x["html_url"]?.ToString() ?? string.Empty,
                                FullName = x["full_name"]?.ToString() ?? string.Empty,
                                Owner = x["owner"]?["login"]?.ToString() ?? string.Empty,
                                Description = x["description"]?.ToString() ?? string.Empty
                            },
                            Official = (x["full_name"]?.ToString().StartsWith("KinectToVR/") ?? false) ||
                                       (x["full_name"]?.ToString().StartsWith("KimihikoAkayasaki/") ?? false)
                        });

                        // Also refresh the last item
                        FoundStorePluginsList.Last().OnPropertyChanged();
                    });

                // Show the status controls
                SearchPlaceholderGrid.Opacity = 0.0;
                SearchResultsGrid.Opacity = FoundStorePluginsList.Any() ? 1.0 : 0.0;
                SearchErrorGrid.Opacity = 0.0;
                NoResultsGrid.Opacity = FoundStorePluginsList.Any() ? 0.0 : 1.0;
            }
            else
            {
                // Show the status controls
                SearchPlaceholderGrid.Opacity = 0.0;
                SearchResultsGrid.Opacity = 0.0;
                SearchErrorGrid.Opacity = 1.0;
                NoResultsGrid.Opacity = 0.0;
            }
        }
        catch (HttpRequestException ex)
        {
            // API rate exceeded, show the authorization toast
            if (ex.StatusCode is HttpStatusCode.Forbidden or HttpStatusCode.Unauthorized)
            {
                RequestShowRateExceededEvent(this, EventArgs.Empty);
            }
            else
            {
                Logger.Warn(ex);

                // Show the status controls
                SearchPlaceholderGrid.Opacity = 0.0;
                SearchResultsGrid.Opacity = 0.0;
                SearchErrorGrid.Opacity = 1.0;
                NoResultsGrid.Opacity = 0.0;
            }
        }
        catch (Exception ex)
        {
            Logger.Warn(ex);

            // Show the status controls
            SearchPlaceholderGrid.Opacity = 0.0;
            SearchResultsGrid.Opacity = 0.0;
            SearchErrorGrid.Opacity = 1.0;
            NoResultsGrid.Opacity = 0.0;
        }

        // Refresh everything else
        OnPropertyChanged();

        // Wait a bit and show the rest
        // This is to avoid weird transitions
        await Task.Delay(500);

        // Unblock the resource controls
        SearchButton.IsEnabled = true;
        SearchTextBox.IsEnabled = true;

        LoadingData = false;
        FinishedLoadingData = true;

        OnPropertyChanged("LoadingData");
        OnPropertyChanged("FinishedLoadingData");
    }

    private void Grid_SizeChanged(object sender, SizeChangedEventArgs e)
    {
        if (MainSplitView is null) return; // Give up before
        MainSplitView.OpenPaneLength = e.NewSize.Width - 700;

        // Optionally close the whole thing if it doesn't fit
        MainSplitView.IsPaneOpen = MainSplitView.OpenPaneLength >= 300;

        SecondarySectionNameTextBlock.Visibility =
            MainSplitView.IsPaneOpen ? Visibility.Collapsed : Visibility.Visible;
        SecondarySectionNameTextBlock.Opacity = MainSplitView.IsPaneOpen ? 0.0 : 1.0;
    }

    private async void SearcherGrid_Drop(object sender, DragEventArgs e)
    {
        if (!SearchTextBox?.IsEnabled ?? true) return;
        if (e.DataView.Contains(StandardDataFormats.WebLink))
        {
            var uri = await e.DataView.GetWebLinkAsync();
            Logger.Info($"Dropped a web link! Data: {uri}");

            // Parse the url and display the status:
            // - Check if it's actually from GitHub
            // - Search for the releases and the manifest
            // - Install the plugin and prompt to restart

            if (uri.ToString().EndsWith(".zip"))
            {
                // The provided text is a link to the plugin zip, process as drag-and-drop
                await InstallRemoteZipPlugin(uri);
            }
            else if (uri.ToString().StartsWith("https://github.com/") ||
                     uri.ToString().StartsWith("http://github.com/"))
            {
                SearchTextBox.Text = uri.ToString();
                await ExecuteSearch(uri.ToString() // The provided text is a link
                    .Replace("https://github.com/", string.Empty)
                    .Replace("http://github.com/", string.Empty));
            }
            else
            {
                SearchTextBox.Text = uri.ToString();
                await ExecuteSearch(uri.ToString());
            }
        }
        else if (e.DataView.Contains(StandardDataFormats.Text))
        {
            var text = await e.DataView.GetTextAsync();
            Logger.Info($"Dropped a text chunk! Data: {text}");

            // (The same as upper)
            if (text.EndsWith(".zip") && Uri.TryCreate(text, UriKind.Absolute, out var uri))
            {
                // The provided text is a link to the plugin zip, process as drag-and-drop
                await InstallRemoteZipPlugin(uri);
            }
            else if (text.StartsWith("https://github.com/") ||
                     text.StartsWith("http://github.com/"))
            {
                SearchTextBox.Text = text;
                await ExecuteSearch(text // The provided text is a link
                    .Replace("https://github.com/", string.Empty)
                    .Replace("http://github.com/", string.Empty));
            }
            else
            {
                SearchTextBox.Text = text;
                await ExecuteSearch(text);
            }
        }
        else if (e.DataView.Contains(StandardDataFormats.StorageItems))
        {
            var files = await e.DataView.GetStorageItemsAsync();
            Logger.Info($"Dropped a StorageItem! Entities: {files.Count}");

            // Parse the result and display the status:
            // - If it's a .zip file:
            //     - Unpack it to Amethyst's shared plugin directory
            //     - Search for 'plugin*.dll' and prompt to restart if OK
            // - If it's a SINGLE folder:
            //     - Copy it to Amethyst's shared plugin directory
            //     - Search for 'plugin*.dll' and prompt to restart if OK
            try
            {
                switch (files.Count)
                {
                    case <= 0:
                        Logger.Info("No files passed!");
                        return;

                    case > 1:
                    {
                        Logger.Info($"Dropped an installation package: {files.Count} file/s");

                        // Block other controls
                        SearchButton.IsEnabled = false;
                        SearchTextBox.IsEnabled = false;

                        // Hide everything
                        SearchPlaceholderGrid.Opacity = 0.0;
                        SearchErrorGrid.Opacity = 0.0;
                        NoResultsGrid.Opacity = 0.0;
                        DeviceCodeGrid.Opacity = 0.0;
                        DropInstallerGrid.Opacity = 0.0;
                        SearchResultsGrid.Opacity = 0.0;

                        DropInstallerMessageTextBlock.Opacity = 0.0;

                        // Search for a plugin dll
                        if (files.Any(x => x.Name.StartsWith("plugin") && x.Name.EndsWith(".dll")))
                        {
                            // Prepare our resources
                            DropInstallerProgressRing.IsIndeterminate = true;
                            DropInstallerProgressRing.Opacity = 1.0;
                            DropInstallerErrorIcon.Opacity = 0.0;

                            DropInstallerHeaderTextBlock.Text =
                                string.Format(LocalizedJsonString("/SharedStrings/Plugins/Drop/Headers/Installing"),
                                    files.Count + " " +
                                    LocalizedJsonString("/SharedStrings/Plugins/Drop/Resources/Files"));

                            // Show the progress indicator
                            DropInstallerGrid.Opacity = 1.0;
                            AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationComplete);

                            // Search for an empty folder in AppData
                            var installFolder = await GetAppDataPluginFolder(
                                string.Join("_", Guid.NewGuid().ToString().ToUpper()
                                    .Split(Path.GetInvalidFileNameChars().Append('.').ToArray())));

                            // Create an empty folder in TempAppData
                            var downloadFolder = await GetTempPluginFolder(
                                string.Join("_", Guid.NewGuid().ToString().ToUpper()
                                    .Split(Path.GetInvalidFileNameChars().Append('.').ToArray())));

                            Logger.Info("Preparing the file system...");

                            // Randomize the path if already exists
                            // Delete if only a single null folder
                            if ((await installFolder.GetItemsAsync()).Any())
                                installFolder = await GetAppDataPluginFolder(
                                    string.Join("_", Guid.NewGuid().ToString().ToUpper()
                                        .Split(Path.GetInvalidFileNameChars().Append('.').ToArray())));

                            // Try reserving the install folder
                            await installFolder.DeleteAsync();

                            // Try creating the download folder
                            await downloadFolder.DeleteAsync();

                            // Unpack the archive now
                            Logger.Info("Unpacking the new plugin from its package...");
                            foreach (var x in files.ToList())
                                switch (x)
                                {
                                    case StorageFile file:
                                        Logger.Info($"Copying file {file.Name} to {downloadFolder}\\");
                                        await file.CopyAsync(downloadFolder, file.Name,
                                            NameCollisionOption.ReplaceExisting);
                                        break;
                                    case StorageFolder folder:
                                        Logger.Info($"Copying folder {folder.Name} to {downloadFolder}\\");
                                        new DirectoryInfo(folder.Path).CopyToFolder(downloadFolder.Path);
                                        break;
                                }

                            // Move the plugin folder if everything's fine
                            Logger.Info($"Moving temp {downloadFolder.Path} to {installFolder.Path}...");
                            Directory.Move(downloadFolder.Path, installFolder.Path);

                            // Wait a bit
                            await Task.Delay(3000);

                            // Prepare our resources
                            DropInstallerProgressRing.IsIndeterminate = false;
                            DropInstallerProgressRing.Value = 100;

                            DropInstallerHeaderTextBlock.Text = string.Format(
                                LocalizedJsonString("/SharedStrings/Plugins/Drop/Headers/Installed"),
                                files.Count + " " + LocalizedJsonString("/SharedStrings/Plugins/Drop/Resources/Files"));
                            DropInstallerMessageTextBlock.Text = string.Format(
                                LocalizedJsonString("/SharedStrings/Plugins/Drop/Statuses/Installed"),
                                files.Count + " " + LocalizedJsonString("/SharedStrings/Plugins/Drop/Resources/Files"));

                            DropInstallerMessageTextBlock.Opacity = 1.0;
                            AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationComplete);

                            // Wait a moment and hide
                            await Task.Delay(3500);

                            // Unlock other controls
                            SearchButton.IsEnabled = true;
                            SearchTextBox.IsEnabled = true;

                            // Show everything
                            SearchPlaceholderGrid.Opacity = 1.0;
                            SearchErrorGrid.Opacity = 0.0;
                            NoResultsGrid.Opacity = 0.0;
                            DeviceCodeGrid.Opacity = 0.0;
                            DropInstallerGrid.Opacity = 0.0;
                        }
                        else
                        {
                            // Prepare our resources
                            DropInstallerProgressRing.Opacity = 0.0;
                            DropInstallerErrorIcon.Opacity = 1.0;

                            DropInstallerHeaderTextBlock.Text = string.Format(
                                LocalizedJsonString("/SharedStrings/Plugins/Drop/Headers/Error/Validating"),
                                files.Count + " " + LocalizedJsonString("/SharedStrings/Plugins/Drop/Resources/Files"));
                            DropInstallerMessageTextBlock.Text = string.Format(
                                LocalizedJsonString("/SharedStrings/Plugins/Drop/Statuses/Error/NotFound/Plural"),
                                files.Count + " " + LocalizedJsonString("/SharedStrings/Plugins/Drop/Resources/Files"));

                            DropInstallerMessageTextBlock.Opacity = 1.0;
                            AppSounds.PlayAppSound(AppSounds.AppSoundType.Error);

                            // Show the result
                            DropInstallerGrid.Opacity = 1.0;

                            // Wait a moment and hide
                            await Task.Delay(2500);

                            // Unlock other controls
                            SearchButton.IsEnabled = true;
                            SearchTextBox.IsEnabled = true;

                            // Show everything
                            SearchPlaceholderGrid.Opacity = 1.0;
                            SearchErrorGrid.Opacity = 0.0;
                            NoResultsGrid.Opacity = 0.0;
                            DeviceCodeGrid.Opacity = 0.0;
                            DropInstallerGrid.Opacity = 0.0;
                        }
                    }
                        break;

                    default:
                        switch (files[0])
                        {
                            case StorageFolder:
                            {
                                var entries = Directory.EnumerateFiles(files[0].Path).ToList();
                                Logger.Info($"Dropped an installation package: {entries.Count} file/s");

                                // Block other controls
                                SearchButton.IsEnabled = false;
                                SearchTextBox.IsEnabled = false;

                                // Hide everything
                                SearchPlaceholderGrid.Opacity = 0.0;
                                SearchErrorGrid.Opacity = 0.0;
                                NoResultsGrid.Opacity = 0.0;
                                DeviceCodeGrid.Opacity = 0.0;
                                DropInstallerGrid.Opacity = 0.0;
                                SearchResultsGrid.Opacity = 0.0;

                                DropInstallerMessageTextBlock.Opacity = 0.0;

                                // Search for a plugin dll
                                if (entries.Select(x => new FileInfo(x))
                                    .Any(x => x.Name.StartsWith("plugin") && x.Name.EndsWith(".dll")))
                                {
                                    // Prepare our resources
                                    DropInstallerProgressRing.IsIndeterminate = true;
                                    DropInstallerProgressRing.Opacity = 1.0;
                                    DropInstallerErrorIcon.Opacity = 0.0;

                                    DropInstallerHeaderTextBlock.Text = string.Format(LocalizedJsonString(
                                        "/SharedStrings/Plugins/Drop/Headers/Installing"), files[0].Name);

                                    // Show the progress indicator
                                    DropInstallerGrid.Opacity = 1.0;
                                    AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationComplete);

                                    // Search for an empty folder in AppData
                                    var installFolder = await GetAppDataPluginFolder(
                                        string.Join("_", Guid.NewGuid().ToString().ToUpper()
                                            .Split(Path.GetInvalidFileNameChars().Append('.').ToArray())));

                                    // Create an empty folder in TempAppData
                                    var downloadFolder = await GetTempPluginFolder(
                                        string.Join("_", Guid.NewGuid().ToString().ToUpper()
                                            .Split(Path.GetInvalidFileNameChars().Append('.').ToArray())));

                                    Logger.Info("Preparing the file system...");

                                    // Randomize the path if already exists
                                    // Delete if only a single null folder
                                    if ((await installFolder.GetItemsAsync()).Any())
                                        installFolder = await GetAppDataPluginFolder(
                                            string.Join("_", Guid.NewGuid().ToString().ToUpper()
                                                .Split(Path.GetInvalidFileNameChars().Append('.').ToArray())));

                                    // Try reserving the install folder
                                    await installFolder.DeleteAsync();

                                    // Try creating the download folder
                                    await downloadFolder.DeleteAsync();

                                    // Unpack the archive now
                                    Logger.Info("Unpacking the new plugin from its package...");
                                    foreach (var x in await (files[0] as StorageFolder)!.GetItemsAsync())
                                        switch (x)
                                        {
                                            case StorageFile file:
                                                Logger.Info($"Copying file {file.Name} to {downloadFolder}\\");
                                                await file.CopyAsync(downloadFolder, file.Name,
                                                    NameCollisionOption.ReplaceExisting);
                                                break;
                                            case StorageFolder folder:
                                                Logger.Info($"Copying folder {folder.Name} to {downloadFolder}\\");
                                                new DirectoryInfo(folder.Path).CopyToFolder(downloadFolder.Path);
                                                break;
                                        }

                                    // Move the plugin folder if everything's fine
                                    Logger.Info($"Moving temp {downloadFolder.Path} to {installFolder.Path}...");
                                    Directory.Move(downloadFolder.Path, installFolder.Path);

                                    // Wait a bit
                                    await Task.Delay(3000);

                                    // Prepare our resources
                                    DropInstallerProgressRing.IsIndeterminate = false;
                                    DropInstallerProgressRing.Value = 100;

                                    DropInstallerHeaderTextBlock.Text = string.Format(LocalizedJsonString(
                                        "/SharedStrings/Plugins/Drop/Headers/Installed"), files[0].Name);
                                    DropInstallerMessageTextBlock.Text = string.Format(LocalizedJsonString(
                                        "/SharedStrings/Plugins/Drop/Statuses/Installed"), files[0].Name);

                                    DropInstallerMessageTextBlock.Opacity = 1.0;
                                    AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationComplete);

                                    // Wait a moment and hide
                                    await Task.Delay(3500);

                                    // Unlock other controls
                                    SearchButton.IsEnabled = true;
                                    SearchTextBox.IsEnabled = true;

                                    // Show everything
                                    SearchPlaceholderGrid.Opacity = 1.0;
                                    SearchErrorGrid.Opacity = 0.0;
                                    NoResultsGrid.Opacity = 0.0;
                                    DeviceCodeGrid.Opacity = 0.0;
                                    DropInstallerGrid.Opacity = 0.0;
                                }
                                else
                                {
                                    // Prepare our resources
                                    DropInstallerProgressRing.Opacity = 0.0;
                                    DropInstallerErrorIcon.Opacity = 1.0;

                                    DropInstallerHeaderTextBlock.Text = string.Format(LocalizedJsonString(
                                        "/SharedStrings/Plugins/Drop/Headers/Error/Validating"), files[0].Name);
                                    DropInstallerMessageTextBlock.Text = string.Format(LocalizedJsonString(
                                        "/SharedStrings/Plugins/Drop/Statuses/Error/NotFound"), files[0].Name);

                                    DropInstallerMessageTextBlock.Opacity = 1.0;
                                    AppSounds.PlayAppSound(AppSounds.AppSoundType.Error);

                                    // Show the result
                                    DropInstallerGrid.Opacity = 1.0;

                                    // Wait a moment and hide
                                    await Task.Delay(2500);

                                    // Unlock other controls
                                    SearchButton.IsEnabled = true;
                                    SearchTextBox.IsEnabled = true;

                                    // Show everything
                                    SearchPlaceholderGrid.Opacity = 1.0;
                                    SearchErrorGrid.Opacity = 0.0;
                                    NoResultsGrid.Opacity = 0.0;
                                    DeviceCodeGrid.Opacity = 0.0;
                                    DropInstallerGrid.Opacity = 0.0;
                                }
                            }
                                break;
                            case StorageFile:
                            {
                                Logger.Info($"Dropped an installation package: {files[0].Name}");

                                // Block other controls
                                SearchButton.IsEnabled = false;
                                SearchTextBox.IsEnabled = false;

                                // Hide everything
                                SearchPlaceholderGrid.Opacity = 0.0;
                                SearchErrorGrid.Opacity = 0.0;
                                NoResultsGrid.Opacity = 0.0;
                                DeviceCodeGrid.Opacity = 0.0;
                                DropInstallerGrid.Opacity = 0.0;
                                SearchResultsGrid.Opacity = 0.0;

                                DropInstallerMessageTextBlock.Opacity = 0.0;

                                // Check if the file is a zip
                                if (files[0].Name.EndsWith(".zip"))
                                {
                                    // Search for a plugin dll
                                    using var archive = ZipFile.OpenRead(files[0].Path);
                                    if (archive.Entries.Any(x =>
                                            x.Name.StartsWith("plugin") && x.Name.EndsWith(".dll")))
                                    {
                                        // Prepare our resources
                                        DropInstallerProgressRing.IsIndeterminate = true;
                                        DropInstallerProgressRing.Opacity = 1.0;
                                        DropInstallerErrorIcon.Opacity = 0.0;

                                        DropInstallerHeaderTextBlock.Text = string.Format(LocalizedJsonString(
                                            "/SharedStrings/Plugins/Drop/Headers/Installing"), files[0].Name);

                                        // Show the progress indicator
                                        DropInstallerGrid.Opacity = 1.0;
                                        AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationComplete);

                                        // Search for an empty folder in AppData
                                        var installFolder = await GetAppDataPluginFolder(
                                            string.Join("_", Guid.NewGuid().ToString().ToUpper()
                                                .Split(Path.GetInvalidFileNameChars().Append('.').ToArray())));

                                        // Create an empty folder in TempAppData
                                        var downloadFolder = await GetTempPluginFolder(
                                            string.Join("_", Guid.NewGuid().ToString().ToUpper()
                                                .Split(Path.GetInvalidFileNameChars().Append('.').ToArray())));

                                        Logger.Info("Preparing the file system...");

                                        // Randomize the path if already exists
                                        // Delete if only a single null folder
                                        if ((await installFolder.GetItemsAsync()).Any())
                                            installFolder = await GetAppDataPluginFolder(
                                                string.Join("_", Guid.NewGuid().ToString().ToUpper()
                                                    .Split(Path.GetInvalidFileNameChars().Append('.').ToArray())));

                                        // Try reserving the install folder
                                        await installFolder.DeleteAsync();

                                        // Try creating the download folder
                                        await downloadFolder.DeleteAsync();

                                        // Unpack the archive now
                                        Logger.Info("Unpacking the new plugin from its package...");
                                        archive.ExtractToDirectory(downloadFolder.Path, true);

                                        // Move the plugin folder if everything's fine
                                        Logger.Info($"Moving temp {downloadFolder.Path} to {installFolder.Path}...");
                                        Directory.Move(downloadFolder.Path, installFolder.Path);

                                        // Wait a bit
                                        await Task.Delay(3000);

                                        // Prepare our resources
                                        DropInstallerProgressRing.IsIndeterminate = false;
                                        DropInstallerProgressRing.Value = 100;

                                        DropInstallerHeaderTextBlock.Text = string.Format(LocalizedJsonString(
                                            "/SharedStrings/Plugins/Drop/Headers/Installed"), files[0].Name);
                                        DropInstallerMessageTextBlock.Text = string.Format(LocalizedJsonString(
                                            "/SharedStrings/Plugins/Drop/Statuses/Installed"), files[0].Name);

                                        DropInstallerMessageTextBlock.Opacity = 1.0;
                                        AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationComplete);

                                        // Wait a moment and hide
                                        await Task.Delay(3500);

                                        // Unlock other controls
                                        SearchButton.IsEnabled = true;
                                        SearchTextBox.IsEnabled = true;

                                        // Show everything
                                        SearchPlaceholderGrid.Opacity = 1.0;
                                        SearchErrorGrid.Opacity = 0.0;
                                        NoResultsGrid.Opacity = 0.0;
                                        DeviceCodeGrid.Opacity = 0.0;
                                        DropInstallerGrid.Opacity = 0.0;
                                    }
                                    else
                                    {
                                        // Prepare our resources
                                        DropInstallerProgressRing.Opacity = 0.0;
                                        DropInstallerErrorIcon.Opacity = 1.0;

                                        DropInstallerHeaderTextBlock.Text = string.Format(LocalizedJsonString(
                                            "/SharedStrings/Plugins/Drop/Headers/Error/Validating"), files[0].Name);
                                        DropInstallerMessageTextBlock.Text = string.Format(LocalizedJsonString(
                                            "/SharedStrings/Plugins/Drop/Statuses/Error/NotFound"), files[0].Name);

                                        DropInstallerMessageTextBlock.Opacity = 1.0;
                                        AppSounds.PlayAppSound(AppSounds.AppSoundType.Error);

                                        // Show the result
                                        DropInstallerGrid.Opacity = 1.0;

                                        // Wait a moment and hide
                                        await Task.Delay(2500);

                                        // Unlock other controls
                                        SearchButton.IsEnabled = true;
                                        SearchTextBox.IsEnabled = true;

                                        // Show everything
                                        SearchPlaceholderGrid.Opacity = 1.0;
                                        SearchErrorGrid.Opacity = 0.0;
                                        NoResultsGrid.Opacity = 0.0;
                                        DeviceCodeGrid.Opacity = 0.0;
                                        DropInstallerGrid.Opacity = 0.0;
                                    }
                                }
                                else
                                {
                                    // Prepare our resources
                                    DropInstallerProgressRing.Opacity = 0.0;
                                    DropInstallerErrorIcon.Opacity = 1.0;

                                    DropInstallerHeaderTextBlock.Text = string.Format(LocalizedJsonString(
                                        "/SharedStrings/Plugins/Drop/Headers/Error/Validating"), files[0].Name);
                                    DropInstallerMessageTextBlock.Text = string.Format(LocalizedJsonString(
                                        "/SharedStrings/Plugins/Drop/Statuses/Error/Invalid"), files[0].Name);

                                    DropInstallerMessageTextBlock.Opacity = 1.0;
                                    AppSounds.PlayAppSound(AppSounds.AppSoundType.Error);

                                    // Show the result
                                    DropInstallerGrid.Opacity = 1.0;

                                    // Wait a moment and hide
                                    await Task.Delay(2500);

                                    // Unlock other controls
                                    SearchButton.IsEnabled = true;
                                    SearchTextBox.IsEnabled = true;

                                    // Show everything
                                    SearchPlaceholderGrid.Opacity = 1.0;
                                    SearchErrorGrid.Opacity = 0.0;
                                    NoResultsGrid.Opacity = 0.0;
                                    DeviceCodeGrid.Opacity = 0.0;
                                    DropInstallerGrid.Opacity = 0.0;
                                }
                            }
                                break;
                        }

                        break;
                }
            }
            catch (Exception ex)
            {
                // Prepare our resources
                DropInstallerProgressRing.Opacity = 0.0;
                DropInstallerErrorIcon.Opacity = 1.0;

                DropInstallerHeaderTextBlock.Text = LocalizedJsonString(
                    "/SharedStrings/Plugins/Drop/Headers/Error/Exception");
                DropInstallerMessageTextBlock.Text = string.Format(LocalizedJsonString(
                    "/SharedStrings/Plugins/Drop/Statuses/Error/Exception"), ex.Message);

                DropInstallerMessageTextBlock.Opacity = 1.0;
                AppSounds.PlayAppSound(AppSounds.AppSoundType.Error);

                // Show the result
                DropInstallerGrid.Opacity = 1.0;

                // Wait a moment and hide
                await Task.Delay(2500);

                // Unlock other controls
                SearchButton.IsEnabled = true;
                SearchTextBox.IsEnabled = true;

                // Show everything
                SearchPlaceholderGrid.Opacity = 1.0;
                SearchErrorGrid.Opacity = 0.0;
                NoResultsGrid.Opacity = 0.0;
                DeviceCodeGrid.Opacity = 0.0;
                DropInstallerGrid.Opacity = 0.0;
            }
        }
    }

    private async Task InstallRemoteZipPlugin(Uri link)
    {
        Logger.Info($"Dropped an installation package: {link.AbsoluteUri}");

        // Block other controls
        SearchButton.IsEnabled = false;
        SearchTextBox.IsEnabled = false;

        // Hide everything
        SearchPlaceholderGrid.Opacity = 0.0;
        SearchErrorGrid.Opacity = 0.0;
        NoResultsGrid.Opacity = 0.0;
        DeviceCodeGrid.Opacity = 0.0;
        DropInstallerGrid.Opacity = 0.0;
        SearchResultsGrid.Opacity = 0.0;

        DropInstallerMessageTextBlock.Opacity = 0.0;

        // Check if the file is a zip
        if (link.IsAbsoluteUri)
        {
            // Try downloading the plugin archive from the link
            Logger.Info($"Downloading the plugin assuming it's under {link.AbsoluteUri}");
            try
            {
                Logger.Info("Preparing UI resources...");

                // Prepare our resources
                DropInstallerProgressRing.IsIndeterminate = true;
                DropInstallerProgressRing.Opacity = 1.0;
                DropInstallerErrorIcon.Opacity = 0.0;

                DropInstallerHeaderTextBlock.Text = string.Format(
                    LocalizedJsonString("/SharedStrings/Plugins/Drop/Headers/Downloading"),
                    link.Segments.LastOrDefault("package.zip"));

                // Show the progress indicator
                DropInstallerGrid.Opacity = 1.0;

                Logger.Info("Creating the download stream...");

                // Create a stream reader using the received Installer Uri
                await using var stream = await GithubClient.DownloadStreamAsync(new RestRequest(link));

                // Search for an empty folder in AppData
                var installFolder = await GetAppDataPluginFolder(
                    string.Join("_", Guid.NewGuid().ToString().ToUpper()
                        .Split(Path.GetInvalidFileNameChars().Append('.').ToArray())));

                // Create an empty folder in TempAppData
                var downloadFolder = await GetTempPluginFolder(
                    string.Join("_", Guid.NewGuid().ToString().ToUpper()
                        .Split(Path.GetInvalidFileNameChars().Append('.').ToArray())));

                Logger.Info("Preparing the file system...");

                // Randomize the path if already exists
                // Delete if only a single null folder
                if ((await installFolder.GetItemsAsync()).Any())
                    installFolder = await GetAppDataPluginFolder(
                        string.Join("_", Guid.NewGuid().ToString().ToUpper()
                            .Split(Path.GetInvalidFileNameChars().Append('.').ToArray())));

                // Try reserving the install folder
                await installFolder.DeleteAsync();

                // Replace or create our installer file
                var pluginArchive = await downloadFolder.CreateFileAsync(
                    link.Segments.LastOrDefault("package.zip"),
                    CreationCollisionOption.ReplaceExisting);

                // Create an output stream and push all the available data to it
                await using var fsPluginArchive = await pluginArchive.OpenStreamForWriteAsync();
                await stream!.CopyToAsync(fsPluginArchive); // The runtime will do the rest for us

                // Close the stream
                await stream.DisposeAsync();
                await fsPluginArchive.DisposeAsync();

                Logger.Info($"Validating the downloaded archive at {pluginArchive.Path}...");

                // Search for a plugin dll
                using var archive = ZipFile.OpenRead(pluginArchive.Path);
                if (archive.Entries.Any(x =>
                        x.Name.StartsWith("plugin") && x.Name.EndsWith(".dll")))
                {
                    // Prepare our resources
                    DropInstallerHeaderTextBlock.Text = string.Format(LocalizedJsonString(
                        "/SharedStrings/Plugins/Drop/Headers/Installing"), pluginArchive.Name);

                    Logger.Info($"Unpacking the archive at {pluginArchive.Path}...");
                    AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationComplete);

                    // Unpack the archive now
                    Logger.Info("Unpacking the new plugin from its package...");
                    archive.ExtractToDirectory(downloadFolder.Path, true);

                    archive.Dispose(); // Close the archive file, dispose
                    Logger.Info("Deleting the plugin installation package...");
                    File.Delete(pluginArchive.Path); // Cleanup after the install

                    // Move the plugin folder if everything's fine
                    Logger.Info($"Moving temp {downloadFolder.Path} to {installFolder.Path}...");
                    Directory.Move(downloadFolder.Path, installFolder.Path);

                    // Wait a bit
                    await Task.Delay(3000);

                    // Prepare our resources
                    DropInstallerProgressRing.IsIndeterminate = false;
                    DropInstallerProgressRing.Value = 100;

                    DropInstallerHeaderTextBlock.Text = string.Format(LocalizedJsonString(
                        "/SharedStrings/Plugins/Drop/Headers/Installed"), pluginArchive.Name);
                    DropInstallerMessageTextBlock.Text = string.Format(LocalizedJsonString(
                        "/SharedStrings/Plugins/Drop/Statuses/Installed"), pluginArchive.Name);

                    DropInstallerMessageTextBlock.Opacity = 1.0;
                    AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationComplete);

                    // Wait a moment and hide
                    await Task.Delay(3500);

                    // Unlock other controls
                    SearchButton.IsEnabled = true;
                    SearchTextBox.IsEnabled = true;

                    // Show everything
                    SearchPlaceholderGrid.Opacity = 1.0;
                    SearchErrorGrid.Opacity = 0.0;
                    NoResultsGrid.Opacity = 0.0;
                    DeviceCodeGrid.Opacity = 0.0;
                    DropInstallerGrid.Opacity = 0.0;
                }
                else
                {
                    // Prepare our resources
                    DropInstallerProgressRing.Opacity = 0.0;
                    DropInstallerErrorIcon.Opacity = 1.0;

                    DropInstallerHeaderTextBlock.Text = string.Format(LocalizedJsonString(
                        "/SharedStrings/Plugins/Drop/Headers/Error/Validating"), pluginArchive.Name);
                    DropInstallerMessageTextBlock.Text = string.Format(LocalizedJsonString(
                        "/SharedStrings/Plugins/Drop/Statuses/Error/NotFound"), pluginArchive.Name);

                    DropInstallerMessageTextBlock.Opacity = 1.0;
                    AppSounds.PlayAppSound(AppSounds.AppSoundType.Error);

                    // Show the result
                    DropInstallerGrid.Opacity = 1.0;

                    // Wait a moment and hide
                    await Task.Delay(2500);

                    // Unlock other controls
                    SearchButton.IsEnabled = true;
                    SearchTextBox.IsEnabled = true;

                    // Show everything
                    SearchPlaceholderGrid.Opacity = 1.0;
                    SearchErrorGrid.Opacity = 0.0;
                    NoResultsGrid.Opacity = 0.0;
                    DeviceCodeGrid.Opacity = 0.0;
                    DropInstallerGrid.Opacity = 0.0;
                }
            }
            catch (Exception e)
            {
                Logger.Error(new Exception($"Error downloading the plugin! Message: {e.Message}"));

                // Prepare our resources
                DropInstallerProgressRing.Opacity = 0.0;
                DropInstallerErrorIcon.Opacity = 1.0;

                DropInstallerHeaderTextBlock.Text = string.Format(LocalizedJsonString(
                        "/SharedStrings/Plugins/Drop/Headers/Error/Installing"),
                    link.Segments.LastOrDefault("package.zip"));

                DropInstallerMessageTextBlock.Text = string.Format(LocalizedJsonString(
                    "/SharedStrings/Plugins/Drop/Statuses/Error/Exception"), e.Message.TrimEnd('.'));

                DropInstallerMessageTextBlock.Opacity = 1.0;
                AppSounds.PlayAppSound(AppSounds.AppSoundType.Error);

                // Show the result
                DropInstallerGrid.Opacity = 1.0;

                // Wait a moment and hide
                await Task.Delay(2500);

                // Unlock other controls
                SearchButton.IsEnabled = true;
                SearchTextBox.IsEnabled = true;

                // Show everything
                SearchPlaceholderGrid.Opacity = 1.0;
                SearchErrorGrid.Opacity = 0.0;
                NoResultsGrid.Opacity = 0.0;
                DeviceCodeGrid.Opacity = 0.0;
                DropInstallerGrid.Opacity = 0.0;
            }
        }
        else
        {
            // Prepare our resources
            DropInstallerProgressRing.Opacity = 0.0;
            DropInstallerErrorIcon.Opacity = 1.0;

            DropInstallerHeaderTextBlock.Text = string.Format(LocalizedJsonString(
                "/SharedStrings/Plugins/Drop/Headers/Error/Validating"), link);
            DropInstallerMessageTextBlock.Text = string.Format(LocalizedJsonString(
                "/SharedStrings/Plugins/Drop/Statuses/Error/Invalid"), link);

            DropInstallerMessageTextBlock.Opacity = 1.0;
            AppSounds.PlayAppSound(AppSounds.AppSoundType.Error);

            // Show the result
            DropInstallerGrid.Opacity = 1.0;

            // Wait a moment and hide
            await Task.Delay(2500);

            // Unlock other controls
            SearchButton.IsEnabled = true;
            SearchTextBox.IsEnabled = true;

            // Show everything
            SearchPlaceholderGrid.Opacity = 1.0;
            SearchErrorGrid.Opacity = 0.0;
            NoResultsGrid.Opacity = 0.0;
            DeviceCodeGrid.Opacity = 0.0;
            DropInstallerGrid.Opacity = 0.0;
        }
    }

    private void SearcherGrid_DragEnter(object sender, DragEventArgs e)
    {
        if (e.DataView.Contains(StandardDataFormats.WebLink) ||
            e.DataView.Contains(StandardDataFormats.Text) ||
            e.DataView.Contains(StandardDataFormats.StorageItems))
            e.AcceptedOperation = DataPackageOperation.Copy;
    }

    private void SearchTextBox_TextChanged(object sender, TextChangedEventArgs e)
    {
        if (!string.IsNullOrEmpty((sender as TextBox)?.Text)) return;
        SearchPlaceholderGrid.Opacity = 1.0;
        SearchResultsGrid.Opacity = 0.0;
        SearchErrorGrid.Opacity = 0.0;
        NoResultsGrid.Opacity = 0.0;
    }

    private async void LoginTextBlock_Tapped(object sender, TappedRoutedEventArgs e)
    {
        // Hide everything else
        SearchPlaceholderGrid.Opacity = 0.0;
        SearchResultsGrid.Opacity = 0.0;
        SearchErrorGrid.Opacity = 0.0;
        NoResultsGrid.Opacity = 0.0;

        try
        {
            // Request the device code
            var codeResponse = await GithubClient.PostAsync(new RestRequest("login/device/code")
                .AddQueryParameter("client_id", AppData.ApiToken, false)
                .AddHeader("Accept", "application/json"));

            if (!codeResponse.IsSuccessStatusCode || codeResponse.Content is null)
                throw new Exception(codeResponse.ErrorMessage);

            var codeResult = codeResponse.Content.TryParseJson(out var code);
            if (!codeResult || code is null)
                throw new Exception("The code result response was invalid!");

            // Update the app view with the received code
            DeviceCodeTextBlock.Text = code["user_code"]?.ToString() ?? "ERROR";
            DeviceCodeGrid.Opacity = 1.0;

            // Open the default web browser to paste the auth code
            await Launcher.LaunchUriAsync(
                new Uri(code["verification_uri"]?.ToString() ?? "https://github.com/login/device"));

            // Wait for the user to authorize the OAuth application
            var timer = new Stopwatch();
            timer.Start(); // Start the timer

            var intervalValid = int.TryParse(code["interval"]?.ToString(), out var interval);
            while (timer.Elapsed < TimeSpan.FromMinutes(13))
            {
                var authResponse = await GithubClient.PostAsync(new RestRequest("login/oauth/access_token")
                    .AddQueryParameter("client_id", AppData.ApiToken, false)
                    .AddQueryParameter("device_code", code["device_code"]?.ToString(), false)
                    .AddQueryParameter("grant_type", "urn:ietf:params:oauth:grant-type:device_code", false)
                    .AddHeader("Accept", "application/json"));

                // Check if the response was valid
                if (authResponse.IsSuccessStatusCode && authResponse.Content is not null)
                    if (authResponse.Content.TryParseJson(out var auth) && auth is not null)
                    {
                        // Search for any errors
                        if (auth["error"]?.ToString() == "access_denied")
                        {
                            Logger.Warn("Access denied, please reauthorize using GitHub.");
                            break; // Don't even care anymore
                        }

                        // Search for any errors
                        if (auth["error"]?.ToString() == "incorrect_device_code")
                        {
                            Logger.Warn("Invalid device code, please reauthorize using GitHub.");
                            break; // Don't even care anymore
                        }

                        // Search for any errors
                        if (auth["error"]?.ToString() == "incorrect_client_credentials")
                        {
                            Logger.Warn("Our fault, please reauthorize using GitHub.");
                            break; // Don't even care anymore
                        }

                        // Check the result
                        if (!string.IsNullOrEmpty(auth["access_token"]?.ToString()))
                        {
                            Logger.Info($"[{auth["token_type"]}] Token acquired!");
                            AppData.Settings.GitHubToken = (true,
                                auth["access_token"]?.ToString().Encrypt());

                            // Save the token to settings
                            AppData.Settings.SaveSettings();

                            // Unblock the resource controls
                            SearchPlaceholderGrid.Opacity = 1.0;
                            SearchResultsGrid.Opacity = 0.0;
                            SearchErrorGrid.Opacity = 0.0;
                            DeviceCodeGrid.Opacity = 0.0;
                            NoResultsGrid.Opacity = 0.0;

                            ResultForbiddenGrid.Visibility = Visibility.Collapsed;
                            SearchButton.IsEnabled = true;
                            SearchTextBox.IsEnabled = true;

                            // Refresh everything else
                            OnPropertyChanged();
                            return; // Deal with it
                        }
                    }

                // Wait 5 seconds before the next request
                await Task.Delay(TimeSpan.FromSeconds(intervalValid ? interval : 5));
            }
        }
        catch (Exception ex)
        {
            Logger.Error(ex);
        }

        // Show the resource controls
        SearchPlaceholderGrid.Opacity = 0.0;
        SearchResultsGrid.Opacity = 0.0;
        SearchErrorGrid.Opacity = 1.0;
        DeviceCodeGrid.Opacity = 0.0;
        NoResultsGrid.Opacity = 0.0;
    }

    public double BoolToOpacity(bool value)
    {
        return value ? 1.0 : 0.0;
    }

    private async void PluginsListTeachingTip_ActionButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
        await Task.Delay(200);

        // Navigate to the info page
        Shared.Main.MainNavigationView.SelectedItem =
            Shared.Main.MainNavigationView.MenuItems[3];

        Shared.Main.NavigateToPage("info",
            new EntranceNavigationTransitionInfo());

        await Task.Delay(500);

        // Show the next tip
        Shared.TeachingTips.InfoPage.HelpTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        Shared.TeachingTips.InfoPage.HelpTeachingTip.IsOpen = true;
    }

    private void PluginsListTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Close the current tip
        PluginsListTeachingTip.IsOpen = false;

        // Show the previous one
        PluginsStoreTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        PluginsStoreTeachingTip.IsOpen = true;
    }

    private void PluginsStoreTeachingTip_ActionButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);

        // Close the current tip
        PluginsStoreTeachingTip.IsOpen = false;

        // Show the previous one
        PluginsListTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        PluginsListTeachingTip.IsOpen = true;
    }

    private async void PluginsStoreTeachingTip_CloseButtonClick(TeachingTip sender, object args)
    {
        // Play a sound
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
        await Task.Delay(200);

        // Navigate to the info page
        Shared.Main.MainNavigationView.SelectedItem =
            Shared.Main.MainNavigationView.MenuItems[0];

        Shared.Main.NavigateToPage("general",
            new EntranceNavigationTransitionInfo());

        await Task.Delay(500);

        // Show the next tip
        Shared.TeachingTips.MainPage.EndingTeachingTip.TailVisibility = TeachingTipTailVisibility.Collapsed;
        Shared.TeachingTips.MainPage.EndingTeachingTip.IsOpen = true;
    }
}