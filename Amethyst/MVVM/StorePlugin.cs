using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Threading.Tasks;
using Windows.System;
using Amethyst.Classes;
using Amethyst.Schedulers;
using Amethyst.Utils;
using Newtonsoft.Json.Linq;
using RestSharp;
using Microsoft.UI.Xaml.Controls;
using System.IO;
using Windows.Storage;
using static System.Runtime.InteropServices.JavaScript.JSType;
using static System.Collections.Specialized.BitVector32;
using System.IO.Compression;

namespace Amethyst.MVVM;

public class StorePlugin : INotifyPropertyChanged
{
    private RestClient ApiClient { get; } = new("https://api.github.com");
    private RestClient GithubClient { get; } = new("https://github.com");

    public string Name { get; set; }
    public bool Official { get; set; } = false;

    public bool LoadingData { get; set; } = true;
    public bool FinishedLoadingData { get; set; }
    public bool Uninstalling => InstalledPlugin?.Uninstalling ?? false;
    public bool PluginExpanderExpanded { get; set; } = false;

    public IEnumerable<StorePluginContributor> Contributors { get; set; }
    public PluginRepository Repository { get; set; }
    public PluginRelease LatestRelease { get; set; }

    public bool WebsiteValid => !string.IsNullOrEmpty(Repository.Url);
    public bool DescriptionValid => !string.IsNullOrEmpty(Repository.Description);
    public bool HasRelease => LatestRelease is not null && !string.IsNullOrEmpty(LatestRelease.Download);
    public bool NoReleases => !HasRelease;

    public bool CanUninstall => InstalledPlugin?.CanUninstall ?? false;

    public bool InstallQueued => ShutdownController.ShutdownTasks
        .Any(x => x.Data == Name + LatestRelease.Version);

    public bool CanInstall => HasRelease && InstalledPlugin is null && !Uninstalling &&
                              !Interfacing.IsExitPending && !InstallQueued;

    private LoadAttemptedPlugin InstalledPlugin => AppPlugins.LoadAttemptedPluginsList.FirstOrDefault(x =>
        x.Website == Repository?.Url || (x.GuidValid && x.Guid == LatestRelease?.Guid), null);

    public event PropertyChangedEventHandler PropertyChanged;

    // MVVM stuff
    public string TrimString(string s, int l)
    {
        return s?[..Math.Min(s.Length, l)] +
               (s?.Length > l ? "..." : "");
    }

    public double BoolToOpacity(bool value)
    {
        return value ? 1.0 : 0.0;
    }

    public void OnPropertyChanged(string propName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
    }

    public string FormatResourceString(string resourceName)
    {
        return string.Format(Interfacing.LocalizedJsonString(resourceName), Name);
    }

    public string FormatReleaseString(string resourceName)
    {
        return string.Format(Interfacing.LocalizedJsonString(resourceName),
            LatestRelease?.DisplayName, LatestRelease?.Version);
    }

    public async void OpenPluginWebsite()
    {
        try
        {
            await Launcher.LaunchUriAsync(new Uri(Repository.Url));
        }
        catch (Exception)
        {
            // ignored
        }
    }

    public void SchedulePluginUninstall()
    {
        // Enqueue a delete startup action to uninstall this plugin
        InstalledPlugin?.EnqueuePluginUninstall();
        OnPropertyChanged(); // Refresh everything
    }

    public void CancelPluginUninstall()
    {
        // Enqueue a delete startup action to uninstall this plugin
        InstalledPlugin?.CancelPluginUninstall();
        OnPropertyChanged(); // Refresh everything
    }

    public void SchedulePluginInstall()
    {
        // Enqueue the update in the shutdown controller
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
        ShutdownController.ShutdownTasks.Add(new ShutdownTask
        {
            Name = $"Install plugin ({Name}, {LatestRelease.Guid}) v{LatestRelease.Version}",
            Priority = true,
            Data = Name + LatestRelease.Version,
            Action = ExecuteUpdates
        });

        // Show the update notice
        OnPropertyChanged(); // Refresh everything
        Shared.Main.DispatcherQueue.TryEnqueue(() =>
        {
            // Search for any pending plugin install task
            var installsPending = ShutdownController.ShutdownTasks
                .Any(x => x.Name.StartsWith("Install"));

            // Also show in MainWindow
            Shared.Main.PluginsInstallInfoBar.IsOpen = installsPending;
            Shared.Main.PluginsInstallInfoBar.Opacity = BoolToOpacity(installsPending);
            Shared.Events.RefreshMainWindowEvent?.Set();

            // Notify about updates
            OnPropertyChanged();
        });
    }

    public void CancelPluginInstall()
    {
        // Delete the uninstall startup action
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Invoke);
        ShutdownController.ShutdownTasks.Remove(ShutdownController.ShutdownTasks
            .FirstOrDefault(x => x.Data == Name + LatestRelease.Version));

        // Show the update notice
        OnPropertyChanged(); // Refresh everything
        Shared.Main.DispatcherQueue.TryEnqueue(() =>
        {
            // Search for any pending plugin install task
            var installsPending = ShutdownController.ShutdownTasks
                .Any(x => x.Name.StartsWith("Install"));

            // Also show in MainWindow
            Shared.Main.PluginsInstallInfoBar.IsOpen = installsPending;
            Shared.Main.PluginsInstallInfoBar.Opacity = BoolToOpacity(installsPending);
            Shared.Events.RefreshMainWindowEvent?.Set();

            // Notify about updates
            OnPropertyChanged();
        });
    }

    private async Task<bool> ExecuteUpdates()
    {
        // Logic:
        // - Download the zip to TempAppData\Amethyst\Plugins\$({LatestRelease}.Guid)
        // - Unzip the plugin, dispose and delete the package if worked
        // - Move from temporary AppData (tmp) to the regular one (roa)

        try
        {
            // Mark the update footer as active
            Shared.Main.PluginsUpdatePendingInfoBar.Title = string.Format(Interfacing.LocalizedJsonString(
                "/SharedStrings/Plugins/Store/Headers/Downloading"), LatestRelease.DisplayName, LatestRelease.Version);

            Shared.Main.PluginsUpdatePendingInfoBar.Message = Interfacing.LocalizedJsonString(
                "/SharedStrings/Plugins/Store/Headers/Preparing");

            Shared.Main.PluginsUpdatePendingInfoBar.IsOpen = true;
            Shared.Main.PluginsUpdatePendingInfoBar.Opacity = 1.0;

            Shared.Main.PluginsUpdatePendingProgressBar.IsIndeterminate = true;
            Shared.Main.PluginsUpdatePendingProgressBar.ShowError = false;
            Shared.Events.RefreshMainWindowEvent?.Set();

            if (string.IsNullOrEmpty(LatestRelease.Download))
            {
                // No files to download, switch to update error
                Shared.Main.PluginsUpdatePendingInfoBar.Message = string.Format(
                    Interfacing.LocalizedJsonString("/SharedStrings/Plugins/Store/Statuses/Error"),
                    LatestRelease.DisplayName);

                Shared.Main.PluginsUpdatePendingProgressBar.IsIndeterminate = true;
                Shared.Main.PluginsUpdatePendingProgressBar.ShowError = true;
                AppSounds.PlayAppSound(AppSounds.AppSoundType.Error);

                await Task.Delay(2500);
                Shared.Main.PluginsUpdatePendingInfoBar.Opacity = 0.0;
                Shared.Main.PluginsUpdatePendingInfoBar.IsOpen = false;
                Shared.Events.RefreshMainWindowEvent?.Set();

                await Task.Delay(1000);
                return false; // That's all
            }

            // Try downloading the plugin archive from the manifest
            Logger.Info($"Downloading the next {Name} plugin assuming it's under {LatestRelease.Download}");
            try
            {
                // Update the progress message
                Shared.Main.PluginsUpdatePendingInfoBar.Message = string.Format(Interfacing.LocalizedJsonString(
                        "/SharedStrings/Plugins/Store/Statuses/Downloading"), LatestRelease.DisplayName,
                    LatestRelease.Version);

                // Create a stream reader using the received Installer Uri
                await using var stream =
                    await GithubClient.DownloadStreamAsync(new RestRequest(LatestRelease.Download));

                // Search for an empty folder in AppData
                var installFolder = Interfacing.GetAppDataPluginFolderDir(
                    string.Join("_", LatestRelease.Guid.Split(
                        Path.GetInvalidFileNameChars().Append('.').ToArray())));

                // Create an empty folder in TempAppData
                var downloadFolder = Interfacing.GetTempPluginFolderDir(
                    string.Join("_", Guid.NewGuid().ToString().ToUpper()
                        .Split(Path.GetInvalidFileNameChars().Append('.').ToArray())));

                // Randomize the path if already exists
                // Delete if only a single null folder
                if (Directory.Exists(installFolder))
                {
                    if (Directory.EnumerateFileSystemEntries(installFolder).Any())
                        installFolder = Interfacing.GetAppDataPluginFolderDir(
                            string.Join("_", Guid.NewGuid().ToString().ToUpper()
                                .Split(Path.GetInvalidFileNameChars().Append('.').ToArray())));

                    // Else delete if empty
                    else Directory.Delete(installFolder, true);
                }

                // Try reserving the install folder
                if (Directory.Exists(installFolder!))
                    Directory.Delete(installFolder!, true);

                // Try creating the download folder
                if (!Directory.Exists(downloadFolder!))
                    Directory.CreateDirectory(downloadFolder!);

                // Replace or create our installer file
                var pluginArchive = await (await StorageFolder
                    .GetFolderFromPathAsync(downloadFolder)).CreateFileAsync(
                    "package.zip", CreationCollisionOption.ReplaceExisting);

                // Create an output stream and push all the available data to it
                await using var fsPluginArchive = await pluginArchive.OpenStreamForWriteAsync();
                await stream!.CopyToAsync(fsPluginArchive); // The runtime will do the rest for us

                // Close the stream
                await stream.DisposeAsync();
                await fsPluginArchive.DisposeAsync();

                // Unpack the archive now
                Logger.Info("Unpacking the new plugin from its archive...");
                ZipFile.ExtractToDirectory(pluginArchive.Path, downloadFolder, true);

                Logger.Info("Deleting the plugin installation package...");
                File.Delete(pluginArchive.Path); // Cleanup after the install

                // Rename the plugin folder if everything's fine
                Directory.Move(downloadFolder, installFolder);
            }
            catch (Exception e)
            {
                Logger.Error($"Error installing {Name}");
                Logger.Error(e); // Print everything

                // No files to download, switch to update error
                Shared.Main.PluginsUpdatePendingInfoBar.Message = string.Format(
                    Interfacing.LocalizedJsonString("/SharedStrings/Plugins/Store/Statuses/Error"),
                    LatestRelease.DisplayName);

                Shared.Main.PluginsUpdatePendingProgressBar.IsIndeterminate = true;
                Shared.Main.PluginsUpdatePendingProgressBar.ShowError = true;

                await Task.Delay(2500);
                Shared.Main.PluginsUpdatePendingInfoBar.Opacity = 0.0;
                Shared.Main.PluginsUpdatePendingInfoBar.IsOpen = false;
                Shared.Events.RefreshMainWindowEvent?.Set();

                await Task.Delay(1000);
                return false; // That's all
            }

            // Everything's fine, show the restart notice
            Shared.Main.PluginsUpdatePendingInfoBar.Message = string.Format(Interfacing.LocalizedJsonString(
                "/SharedStrings/Plugins/Store/Headers/Restart"), LatestRelease.DisplayName);

            Shared.Main.PluginsUpdatePendingProgressBar.IsIndeterminate = false;
            Shared.Main.PluginsUpdatePendingProgressBar.ShowError = false;
            AppSounds.PlayAppSound(AppSounds.AppSoundType.CalibrationComplete);

            await Task.Delay(2500);
            Shared.Main.PluginsUpdatePendingInfoBar.Opacity = 0.0;
            Shared.Main.PluginsUpdatePendingInfoBar.IsOpen = false;
            Shared.Events.RefreshMainWindowEvent?.Set();

            // Still here? We're done!
            await Task.Delay(1000);
            return true; // That's all
        }
        catch (Exception e)
        {
            Logger.Info(e);

            // No files to download, switch to update error
            Shared.Main.PluginsUpdatePendingInfoBar.Message = string.Format(Interfacing.LocalizedJsonString(
                "/SharedStrings/Plugins/Store/Statuses/Error"), LatestRelease.DisplayName);

            Shared.Main.PluginsUpdatePendingProgressBar.IsIndeterminate = true;
            Shared.Main.PluginsUpdatePendingProgressBar.ShowError = true;
            AppSounds.PlayAppSound(AppSounds.AppSoundType.Error);

            await Task.Delay(2500);
            Shared.Main.PluginsUpdatePendingInfoBar.Opacity = 0.0;
            Shared.Main.PluginsUpdatePendingInfoBar.IsOpen = false;
            Shared.Events.RefreshMainWindowEvent?.Set();

            await Task.Delay(1000);
            return false; // That's all
        }
    }

    public async void FetchPluginData()
    {
        try
        {
            PlayExpandingSound();

            // Mark as loading and trigger a partial refresh
            LoadingData = true;
            FinishedLoadingData = false;
            OnPropertyChanged("LoadingData");
            OnPropertyChanged("FinishedLoadingData");

            // Fetch contributor details
            var contributorsResponse = await ApiClient.GetAsyncAuthorized(
                new RestRequest($"repos/{Repository.FullName}/contributors"));
            if (!contributorsResponse.IsSuccessStatusCode || contributorsResponse.Content is null)
                throw new Exception("Contributors request failed!");

            var contributorsResult = JObject.Parse($"{{\"items\":{contributorsResponse.Content}}}");
            var contributors = contributorsResult["items"]?.Children().ToList()
                               ?? throw new Exception("Contributors were invalid!");

            // Add contributors to the plugin data
            Contributors = contributors.Select(x => new StorePluginContributor
            {
                Name = x["login"]?.ToString() ?? string.Empty,
                Avatar = new Uri(x["avatar_url"]?.ToString() ?? string.Empty),
                Url = new Uri(x["html_url"]?.ToString() ?? string.Empty)
            });

            // Fetch release details
            var releasesResponse = await ApiClient.GetAsyncAuthorized(
                new RestRequest($"repos/{Repository.FullName}/releases"));
            if (!releasesResponse.IsSuccessStatusCode || releasesResponse.Content is null)
                throw new Exception("Releases request failed!");

            var releasesResult = JObject.Parse($"{{\"items\":{releasesResponse.Content}}}");
            var release = releasesResult["items"]?.Children().FirstOrDefault(defaultValue: null)
                          ?? throw new Exception("Releases were invalid!");

            // Fetch manifest details and content
            var manifestUrl = release["assets"]?.Children()
                .FirstOrDefault(x => x["name"]?.ToString() == "manifest.json", null)?["browser_download_url"]
                ?.ToString() ?? throw new Exception("No manifest found!");

            var manifestResponse = await GithubClient.GetAsync(
                new RestRequest(manifestUrl.Replace("https://github.com/", "")));
            if (!manifestResponse.IsSuccessStatusCode || manifestResponse.Content is null)
                throw new Exception("Manifest request failed!");

            var manifestResult = manifestResponse.Content.TryParseJson(out var manifest);
            if (!manifestResult || manifest is null) throw new Exception("The manifest was invalid!");

            // Add everything to the plugin data
            LatestRelease = new PluginRelease
            {
                Title = release["name"]?.ToString() ?? Repository.Name,
                Date = DateTime.Parse(release["published_at"]?.ToString() ?? string.Empty).ToLocalTime(),
                Description = release["body"]?.ToString() ?? Repository.Description,

                Guid = manifest["guid"]?.ToString(),
                Version = "v" + (manifest["version"]?.ToString() ?? "1.0.0.0"),
                Changelog = manifest["changelog"]?.ToString(),
                DisplayName = manifest["display_name"]?.ToString(),

                Download = release["assets"]?.Children().FirstOrDefault(
                        x => x["name"]?.ToString() == manifest["download"]?.ToString(), null)?
                    ["browser_download_url"]?.ToString()
            };

            // Setup property watchers
            if (InstalledPlugin is not null)
            {
                InstalledPlugin.PropertyChanged -= InstalledPluginOnPropertyChanged;
                InstalledPlugin.PropertyChanged += InstalledPluginOnPropertyChanged;
            }
        }
        catch (HttpRequestException e)
        {
            // API rate exceeded, show the authorization toast
            if (e.StatusCode is HttpStatusCode.Forbidden or HttpStatusCode.Unauthorized)
                Pages.Plugins.RequestShowRateExceededEvent(this, EventArgs.Empty);
        }
        catch (Exception e)
        {
            // Show a 'fetch failed' error view
            LatestRelease = null;
            Logger.Error(e);
        }

        // Refresh everything else
        OnPropertyChanged();

        // Wait a bit and show the rest
        await Task.Delay(500);

        // Unblock the resource controls
        LoadingData = false;
        FinishedLoadingData = true;

        OnPropertyChanged("LoadingData");
        OnPropertyChanged("FinishedLoadingData");
    }

    private void InstalledPluginOnPropertyChanged(object sender, PropertyChangedEventArgs e)
    {
        Shared.Main.DispatcherQueue.TryEnqueue(() => OnPropertyChanged());
    }

    public void PlayExpandingSound()
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Show);
    }

    public void PlayCollapsingSound()
    {
        AppSounds.PlayAppSound(AppSounds.AppSoundType.Hide);
    }

    public class PluginRepository
    {
        public string Name { get; set; }
        public string FullName { get; set; }
        public string Owner { get; set; }
        public string Description { get; set; }
        public string Url { get; set; }
    }

    public class PluginRelease
    {
        public string Title { get; set; }
        public string DisplayName { get; set; }
        public string Version { get; set; }
        public DateTime Date { get; set; }
        public string Description { get; set; }
        public string Changelog { get; set; }
        public string Download { get; set; }
        public string Guid { get; set; }
    }
}

public class StorePluginContributor
{
    public string Name { get; set; }
    public Uri Avatar { get; set; }
    public Uri Url { get; set; }

    // MVVM stuff
    public async void OpenContributorWebsite()
    {
        try
        {
            await Launcher.LaunchUriAsync(Url);
        }
        catch (Exception e)
        {
            Logger.Warn(e);
        }
    }
}