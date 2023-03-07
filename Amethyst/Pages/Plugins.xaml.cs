// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Threading;
using System.Threading.Tasks;
using Windows.ApplicationModel.DataTransfer;
using Windows.System;
using Amethyst.Classes;
using Amethyst.MVVM;
using Amethyst.Utils;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Input;
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
        // TODO TeachingTips

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

    public bool LoadingData { get; set; } = false;
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
        if (string.IsNullOrEmpty(SearchTextBox.Text)) return;
        await ProcessQuery(SearchTextBox.Text); // Search using our query
    }

    private async void SearchTextBox_KeyDown(object sender, KeyRoutedEventArgs e)
    {
        if (e.Key != VirtualKey.Enter) return; // Discard non-confirms
        await ProcessQuery((sender as TextBox)?.Text);
    }

    private async Task ProcessQuery(string query)
    {
        if (query.EndsWith(".zip"))
        {
            // The provided text is a link to the plugin zip, process as drag-and-drop
            // TODO DRAGANDDROP
        }
        else if (query.StartsWith("https://github.com/") || query.StartsWith("http://github.com/"))
        {
            await ExecuteSearch(query // The provided text is a link
                .Replace("https://github.com/", string.Empty)
                .Replace("http://github.com/", string.Empty));
        }
        else
        {
            // Only a search query
            await ExecuteSearch(query); 
        }
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
                RequestShowRateExceededEvent(this, EventArgs.Empty);
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
        if (e.DataView.Contains(StandardDataFormats.WebLink))
        {
            var uri = await e.DataView.GetWebLinkAsync();
            Logger.Info($"Dropped a web link! Data: {uri}");

            // Parse the url and display the status:
            // - Check if it's actually from GitHub
            // - Search for the releases and the manifest
            // - Install the plugin and prompt to restart
        }

        if (e.DataView.Contains(StandardDataFormats.Text))
        {
            var uri = await e.DataView.GetTextAsync();
            Logger.Info($"Dropped a web link! Data: {uri}");

            // (The same as upper)
        }
        else if (e.DataView.Contains(StandardDataFormats.StorageItems))
        {
            var file = await e.DataView.GetStorageItemsAsync();
            Logger.Info($"Dropped a StorageItem! Entities: {file.Count}");

            // Parse the result and display the status:
            // - If it's a .zip file:
            //     - Unpack it to Amethyst's shared plugin directory
            //     - Search for 'plugin*.dll' and prompt to restart if OK
            // - If it's a SINGLE folder:
            //     - Copy it to Amethyst's shared plugin directory
            //     - Search for 'plugin*.dll' and prompt to restart if OK
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
}