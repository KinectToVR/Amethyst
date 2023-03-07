using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Net;
using Windows.System;
using Amethyst.Classes;
using Amethyst.Utils;
using RestSharp;
using Newtonsoft.Json.Linq;
using Microsoft.UI.Xaml;
using System.Net.Http;
using System.Numerics;
using Amethyst.Schedulers;
using System.Threading.Tasks;

namespace Amethyst.MVVM;

public class StorePlugin : INotifyPropertyChanged
{
    private RestClient ApiClient { get; } = new("https://api.github.com");
    private RestClient GithubClient { get; } = new("https://github.com");

    public string Name { get; set; }
    public bool Official { get; set; } = false;

    public bool Installing { get; set; } = false;
    public bool LoadingData { get; set; } = true;
    public bool FinishedLoadingData { get; set; } = false;
    public bool Uninstalling => InstalledPlugin?.Uninstalling ?? false;
    public bool InstallSuccess { get; set; } = false;
    public bool InstallError { get; set; } = false;
    public bool PluginExpanderExpanded { get; set; } = false;

    public IEnumerable<StorePluginContributor> Contributors { get; set; }
    public PluginRepository Repository { get; set; }
    public PluginRelease LatestRelease { get; set; }

    public bool WebsiteValid => !string.IsNullOrEmpty(Repository.Url);
    public bool DescriptionValid => !string.IsNullOrEmpty(Repository.Description);
    public bool HasRelease => LatestRelease is not null && !string.IsNullOrEmpty(LatestRelease.Download);
    public bool NoReleases => !HasRelease;

    public bool CanUninstall => InstalledPlugin?.CanUninstall ?? false;
    public bool CanInstall => HasRelease && InstalledPlugin is null && !Uninstalling;

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

    public async void InstallPlugin()
    {
        // TODO download action etc
        OnPropertyChanged(); // Refresh everything
    }

    public async void FetchPluginData()
    {
        try
        {
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
                Version = manifest["version"]?.ToString(),
                Changelog = manifest["changelog"]?.ToString(),
                DisplayName = manifest["display_name"]?.ToString(),

                Download = release["assets"]?.Children().FirstOrDefault(
                        x => x["name"]?.ToString().EndsWith(".zip") ?? false, null)?
                    ["browser_download_url"]?.ToString()
            };
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