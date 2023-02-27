// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Net;
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
using RestSharp;
using Newtonsoft.Json.Linq;
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
    private bool _pluginsPageLoadedOnce;

    public Plugins()
    {
        InitializeComponent();

        Logger.Info($"Constructing page: '{GetType().FullName}'...");
        // TODO TeachingTips

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

    private RestClient ApiClient { get; } = new("https://api.github.com");

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
        await ExecuteSearch(SearchTextBox.Text); // Search using our query
    }

    private async void SearchTextBox_KeyDown(object sender, KeyRoutedEventArgs e)
    {
        if (e.Key != VirtualKey.Enter || string.IsNullOrEmpty((sender as TextBox)?.Text)) return;
        await ExecuteSearch(((TextBox)sender).Text); // Search using our query
    }

    private async Task ExecuteSearch(string query)
    {
        try
        {
            // Block the resource controls
            SearchButton.IsEnabled = false;
            SearchTextBox.IsEnabled = false;

            SearchPlaceholderGrid.Opacity = 1.0;
            SearchResultsGrid.Opacity = 0.0;
            SearchErrorGrid.Opacity = 0.0;

            // FoundStorePluginsList
            var searchResponse = await ApiClient.GetAsync(
                new RestRequest("search/repositories")
                    .AddQueryParameter("q", $"topic:amethyst-plugin+{query}", false));

            // Check if the results are valid
            if (searchResponse.Content is null && searchResponse.StatusCode is not
                    HttpStatusCode.Accepted and not HttpStatusCode.Forbidden)
            {
                SearchPlaceholderGrid.Opacity = 0.0;
                SearchResultsGrid.Opacity = 0.0;
                SearchErrorGrid.Opacity = 1.0;
                return; // Show the status controls
            }

            // API rate exceeded, show the authorization toast
            if (searchResponse.StatusCode is HttpStatusCode.Forbidden)
            {
                SearchPlaceholderGrid.Opacity = 0.0;
                SearchResultsGrid.Opacity = 0.0;
                SearchErrorGrid.Opacity = 1.0;

                ResultForbiddenGrid.Visibility = Visibility.Visible;
                return; // Show the status controls
            }

            // Clear all the results
            FoundStorePluginsList.Clear();

            // Parse all the retrieved items
            JObject.Parse(searchResponse.Content)["items"]?
                .Children().ToList().ForEach(x =>
                    FoundStorePluginsList.Add(new StorePlugin
                    {
                        Name = x["name"]?.ToString() ?? string.Empty,
                        Repository = new PluginRepository
                        {
                            Name = x["name"]?.ToString() ?? string.Empty,
                            FullName = x["full_name"]?.ToString() ?? string.Empty,
                            Owner = x["owner"]?["login"]?.ToString() ?? string.Empty,
                            Description = x["description"]?.ToString() ?? string.Empty
                        },
                        Official = (x["full_name"]?.ToString().StartsWith("KinectToVR/") ?? false) ||
                                   (x["full_name"]?.ToString().StartsWith("KimihikoAkayasaki/") ?? false)
                    }));

            // Show the status controls
            SearchPlaceholderGrid.Opacity = 0.0;
            SearchResultsGrid.Opacity = 1.0;
            SearchErrorGrid.Opacity = 0.0;
        }
        catch (Exception ex)
        {
            Logger.Warn(ex);

            // Show the status controls
            SearchPlaceholderGrid.Opacity = 0.0;
            SearchResultsGrid.Opacity = 0.0;
            SearchErrorGrid.Opacity = 1.0;
        }

        // Unblock the resource controls
        SearchButton.IsEnabled = true;
        SearchTextBox.IsEnabled = true;

        // Refresh everything else
        OnPropertyChanged();
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
}